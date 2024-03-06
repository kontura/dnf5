/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "system-upgrade.hpp"

#include "../offline/offline.hpp"

#include "libdnf5/utils/bgettext/bgettext-lib.h"

#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_path.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>
#include <sys/wait.h>
#include <systemd/sd-journal.h>

#include <iostream>
#include <string>

using namespace libdnf5::cli;

namespace dnf5 {

void SystemUpgradeCommand::pre_configure() {
    throw_missing_command();
}

void SystemUpgradeCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("subcommands").register_argument(arg_parser_this_cmd);
}

void SystemUpgradeCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Prepare system for upgrade to a new release"));
}

void SystemUpgradeCommand::register_subcommands() {
    register_subcommand(std::make_unique<SystemUpgradeDownloadCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineCleanCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineRebootCommand>(get_context()));
    register_subcommand(std::make_unique<OfflineLogCommand>(get_context()));
}

void SystemUpgradeDownloadCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(_("Downloads everything needed to upgrade to a new release"));

    download_dir = dynamic_cast<libdnf5::OptionPath *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionPath>(dnf5::offline::DEFAULT_DATADIR)));

    auto * download_dir_arg = parser.add_new_named_arg("downloaddir");
    download_dir_arg->set_long_name("downloaddir");
    download_dir_arg->set_has_value(true);
    download_dir_arg->set_description(_("Redirect download of packages to provided <path>"));
    download_dir_arg->link_value(download_dir);
    cmd.register_named_arg(download_dir_arg);

    no_downgrade =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(true)));

    auto * no_downgrade_arg = parser.add_new_named_arg("no-downgrade");
    no_downgrade_arg->set_long_name("no-downgrade");
    no_downgrade_arg->set_description(
        _("Do not install packages from the new release if they are older than what is currently installed"));
    no_downgrade_arg->link_value(no_downgrade);
    cmd.register_named_arg(no_downgrade_arg);
}

void SystemUpgradeDownloadCommand::configure() {
    auto & ctx = get_context();

    const std::filesystem::path installroot{ctx.base.get_config().get_installroot_option().get_value()};

    system_releasever = *libdnf5::Vars::detect_release(ctx.base.get_weak_ptr(), installroot);
    target_releasever = ctx.base.get_vars()->get_value("releasever");

    // Check --releasever
    if (target_releasever == system_releasever) {
        throw libdnf5::cli::CommandExitError(1, M_("Need a --releasever greater than the current system version."));
    }

    ctx.set_load_system_repo(true);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    // Specifically for system-upgrade, the `downloaddir` argument should take
    // priority over the `cachedir` specified by the user. We also prepend the
    // installroot here since it won't be done in Base::setup if the option
    // priority is greater than Priority::COMMANDLINE.
    if (download_dir->get_priority() < libdnf5::Option::Priority::COMMANDLINE) {
        ctx.base.get_config().get_cachedir_option().set(
            libdnf5::Option::Priority::RUNTIME, installroot / download_dir->get_value());
    } else {
        ctx.base.get_config().get_cachedir_option().set(libdnf5::Option::Priority::RUNTIME, download_dir->get_value());
    }
}

void SystemUpgradeDownloadCommand::run() {
    auto & ctx = get_context();

    const auto & goal = std::make_unique<libdnf5::Goal>(ctx.base);

    if (no_downgrade->get_value()) {
        goal->add_rpm_upgrade();
    } else {
        goal->add_rpm_distro_sync();
    }

    auto transaction = goal->resolve();
    if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw libdnf5::cli::GoalResolveError(transaction);
    }

    if (transaction.get_transaction_packages_count() == 0) {
        throw libdnf5::cli::CommandExitError(
            1, M_("The system-upgrade transaction is empty; your system is already up-to-date."));
    }

    ctx.set_should_store_offline(true);
    ctx.download_and_run(transaction);

    std::cout << _("Download complete! Use `dnf5 system-upgrade reboot` to start the upgrade.\n"
                   "To cancel the upgrade and delete the downloaded upgrade files, use `dnf5 system-upgrade clean`.")
              << std::endl;

    dnf5::offline::log_status(
        "Download finished.", dnf5::offline::DOWNLOAD_FINISHED_ID, system_releasever, target_releasever);
}

}  // namespace dnf5