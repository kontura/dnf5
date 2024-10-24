/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "transaction_sr.hpp"

#include "utils/string.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <json.h>
#include <libdnf5/comps/environment/query.hpp>
#include <libdnf5/comps/group/package.hpp>


namespace libdnf5::transaction {

// The version of the stored transaction.
//
// MAJOR version denotes backwards incompatible changes (old dnf won't work with
// new transaction JSON).
//
// MINOR version denotes extending the format without breaking backwards
// compatibility (old dnf can work with new transaction JSON). Forwards
// compatibility needs to be handled by being able to process the old format as
// well as the new one.
constexpr const char * VERSION_MAJOR = "1";
constexpr const char * VERSION_MINOR = "0";

class TransactionReplayError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "TransactionReplayError"; }
};

TransactionReplay parse_transaction_replay(const std::string & json_serialized_transaction) {
    if (json_serialized_transaction.empty()) {
        throw TransactionReplayError(M_("Transaction replay JSON serialized transaction input is empty"));
    }

    TransactionReplay transaction_replay;

    auto * data = json_tokener_parse(json_serialized_transaction.c_str());
    if (data == nullptr) {
        throw TransactionReplayError(
            M_("Error during transaction replay JSON parsing : {}"), std::string(json_util_get_last_err()));
    }

    // parse json
    struct json_object * value = nullptr;
    if (json_object_object_get_ex(data, "version", &value) != 0) {
        std::string version = json_object_get_string(value);
        auto versions = libdnf5::utils::string::split(version, ".", 2);
        if (versions[0] != std::string(VERSION_MAJOR)) {
            throw TransactionReplayError(
                M_("Incompatible major version: \"{}\", supported major version is \"{}\""),
                versions[0],
                std::string(VERSION_MAJOR));
        }
    }


    // PARSE ENVIRONMENTS
    struct json_object * json_environments = nullptr;
    if (json_object_object_get_ex(data, "environments", &json_environments) != 0) {
        std::string action;
        std::string environment_id;
        std::string repo_id;

        for (std::size_t i = 0; i < json_object_array_length(json_environments); ++i) {
            struct json_object * environment = json_object_array_get_idx(json_environments, i);
            if (json_object_object_get_ex(environment, "action", &value) != 0) {
                action = json_object_get_string(value);
            }
            if (json_object_object_get_ex(environment, "id", &value) != 0) {
                environment_id = json_object_get_string(value);
            }
            if (json_object_object_get_ex(environment, "repo_id", &value) != 0) {
                repo_id = json_object_get_string(value);
            }

            transaction_replay.environments.push_back(
                {transaction_item_action_from_string(action), environment_id, repo_id});
        }
    }


    // PARSE GROUPS
    struct json_object * json_groups = nullptr;
    if (json_object_object_get_ex(data, "groups", &json_groups) != 0) {
        std::string action;
        std::string group_id;
        std::string reason;
        std::string repo_id;

        for (std::size_t i = 0; i < json_object_array_length(json_groups); ++i) {
            struct json_object * group = json_object_array_get_idx(json_groups, i);
            if (json_object_object_get_ex(group, "action", &value) != 0) {
                action = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "reason", &value) != 0) {
                reason = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "id", &value) != 0) {
                group_id = json_object_get_string(value);
            }
            if (json_object_object_get_ex(group, "repo_id", &value) != 0) {
                repo_id = json_object_get_string(value);
            }

            transaction_replay.groups.push_back(
                {transaction_item_action_from_string(action),
                 transaction_item_reason_from_string(reason),
                 group_id,
                 repo_id});
        }
    }


    // PARSE PACKAGES
    struct json_object * json_packages = nullptr;
    if (json_object_object_get_ex(data, "rpms", &json_packages) != 0) {
        std::string action;
        std::string nevra;
        std::string reason;
        std::string repo_id;
        std::string group_id;
        std::string package_path;

        for (std::size_t i = 0; i < json_object_array_length(json_packages); ++i) {
            struct json_object * package = json_object_array_get_idx(json_packages, i);
            if (json_object_object_get_ex(package, "action", &value) != 0) {
                action = json_object_get_string(value);
            }
            if (json_object_object_get_ex(package, "reason", &value) != 0) {
                reason = json_object_get_string(value);
            }
            if (json_object_object_get_ex(package, "group_id", &value) != 0) {
                group_id = json_object_get_string(value);
            }
            if (json_object_object_get_ex(package, "nevra", &value) != 0) {
                nevra = json_object_get_string(value);
            }
            if (json_object_object_get_ex(package, "package_path", &value) != 0) {
                package_path = json_object_get_string(value);
            }
            if (json_object_object_get_ex(package, "repo_id", &value) != 0) {
                repo_id = json_object_get_string(value);
            }

            transaction_replay.packages.push_back(
                {transaction_item_action_from_string(action),
                 transaction_item_reason_from_string(reason),
                 group_id,
                 nevra,
                 package_path,
                 repo_id});
        }
    }

    json_object_put(data);

    return transaction_replay;
}

std::string json_serialize(const TransactionReplay & transaction_replay) {
    json_object * root = json_object_new_object();

    size_t count = transaction_replay.packages.size();
    if (!std::in_range<int>(count)) {
        libdnf_throw_assertion("Cannot serialize transaction with {} packages", count);
    }
    if (count > 0) {
        json_object * json_packages = json_object_new_array_ext(static_cast<int>(count));
        for (const auto & pkg : transaction_replay.packages) {
            json_object * json_package = json_object_new_object();
            json_object_object_add(json_package, "nevra", json_object_new_string(pkg.nevra.c_str()));
            json_object_object_add(
                json_package, "action", json_object_new_string(transaction_item_action_to_string(pkg.action).c_str()));
            json_object_object_add(
                json_package, "reason", json_object_new_string(transaction_item_reason_to_string(pkg.reason).c_str()));
            json_object_object_add(json_package, "repo_id", json_object_new_string(pkg.repo_id.c_str()));
            if (!pkg.package_path.empty()) {
                json_object_object_add(json_package, "package_path", json_object_new_string(pkg.package_path.c_str()));
            }
            if (!pkg.group_id.empty()) {
                json_object_object_add(json_package, "group_id", json_object_new_string(pkg.group_id.c_str()));
            }

            json_object_array_add(json_packages, json_package);
        }
        json_object_object_add(root, "rpms", json_packages);
    }

    count = transaction_replay.groups.size();
    if (!std::in_range<int>(count)) {
        libdnf_throw_assertion("Cannot serialize transaction with {} groups", count);
    }
    if (count > 0) {
        json_object * json_groups = json_object_new_array_ext(static_cast<int>(count));
        for (const auto & group : transaction_replay.groups) {
            json_object * json_group = json_object_new_object();
            json_object_object_add(json_group, "id", json_object_new_string(group.group_id.c_str()));
            json_object_object_add(
                json_group, "action", json_object_new_string(transaction_item_action_to_string(group.action).c_str()));
            json_object_object_add(
                json_group, "reason", json_object_new_string(transaction_item_reason_to_string(group.reason).c_str()));
            json_object_object_add(json_group, "repo_id", json_object_new_string(group.repo_id.c_str()));
            json_object_array_add(json_groups, json_group);
        }
        json_object_object_add(root, "groups", json_groups);
    }

    count = transaction_replay.environments.size();
    if (!std::in_range<int>(count)) {
        libdnf_throw_assertion("Cannot serialize transaction with {} environments", count);
    }
    if (count > 0) {
        json_object * json_environments = json_object_new_array_ext(static_cast<int>(count));
        for (const auto & environment : transaction_replay.environments) {
            json_object * json_environment = json_object_new_object();
            json_object_object_add(json_environment, "id", json_object_new_string(environment.environment_id.c_str()));
            json_object_object_add(
                json_environment,
                "action",
                json_object_new_string(transaction_item_action_to_string(environment.action).c_str()));
            json_object_object_add(json_environment, "repo_id", json_object_new_string(environment.repo_id.c_str()));

            json_object_array_add(json_environments, json_environment);
        }
        json_object_object_add(root, "environments", json_environments);
    }

    ////TODO(amatej): Allow using local sources (downloaded packages, groups..)
    ////TODO(amatej): potentially add modules

    std::string version = std::string(VERSION_MAJOR) + "." + std::string(VERSION_MINOR);
    json_object_object_add(root, "version", json_object_new_string(version.c_str()));

    auto json = std::string(json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY));

    // clean up
    json_object_put(root);

    return json;
}

}  // namespace libdnf5::transaction
