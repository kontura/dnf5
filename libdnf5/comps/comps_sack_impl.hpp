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

#ifndef LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP
#define LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP

#include "libdnf5/base/base.hpp"
#include "libdnf5/comps/comps_sack.hpp"
#include "libdnf5/comps/environment/query.hpp"
#include "libdnf5/comps/group/query.hpp"

namespace libdnf5::comps {

class GroupSack;
using GroupSackWeakPtr = WeakPtr<GroupSack, false>;

class GroupSack : public sack::Sack<Group> {
public:
    //TODO(amatej): move definitios
    void add_group(std::unique_ptr<Group> && item) {
        add_item(std::move(item));
    }

    std::vector<std::unique_ptr<Group>> & get_data() {
        return sack::Sack<Group>::get_data();
    }

    GroupSackWeakPtr get_weak_ptr() {
        return {this, &sack_guard};
    }

    // Number of solvables in the comps pool when the
    // sack was initialized.
    int cached_solvables_size{0};
private:
    friend comps::GroupQuery;
    WeakPtrGuard<GroupSack, false> sack_guard;

};

class CompsSack::Impl {
public:
    explicit Impl(const BaseWeakPtr & base) : base(base) {}

    /// Sets excluded groups and environments according to the configuration.
    ///
    /// Uses the `disable_excludes`, `excludegroups` and `excludeenvironments` configuration options
    /// to calculate the `config_group_excludes` and `config_environment_excludes` sets.
    void load_config_excludes();

    const std::set<std::string> get_config_environment_excludes();
    const std::set<std::string> get_config_group_excludes();

    const std::set<std::string> get_user_environment_excludes();
    void add_user_environment_excludes(const std::set<std::string> & excludes);
    void add_user_environment_excludes(const EnvironmentQuery & excludes);
    void remove_user_environment_excludes(const std::set<std::string> & excludes);
    void remove_user_environment_excludes(const EnvironmentQuery & excludes);
    void set_user_environment_excludes(const std::set<std::string> & excludes);
    void set_user_environment_excludes(const EnvironmentQuery & excludes);
    void clear_user_environment_excludes();

    const std::set<std::string> get_user_group_excludes();
    void add_user_group_excludes(const std::set<std::string> & excludes);
    void add_user_group_excludes(const GroupQuery & excludes);
    void remove_user_group_excludes(const std::set<std::string> & excludes);
    void remove_user_group_excludes(const GroupQuery & excludes);
    void set_user_group_excludes(const std::set<std::string> & excludes);
    void set_user_group_excludes(const GroupQuery & excludes);
    void clear_user_group_excludes();

    void init_group_sack();
    //TODO(amatej): move this
    GroupSackWeakPtr get_group_sack();

private:
    friend comps::CompsSack;

    BaseWeakPtr base;
    WeakPtrGuard<comps::CompsSack, false> sack_guard;

    std::set<std::string> config_environment_excludes;  // environments explicitly excluded by config
    std::set<std::string> config_group_excludes;        // groups explicitly excluded by config
    std::set<std::string> user_environment_excludes;    // environments explicitly excluded by API user
    std::set<std::string> user_group_excludes;          // groups explicitly excluded by API user

    //TODO(amatej): check if we can move the excludes/includes into the group/env sack.
    //              If not we don't need this to a be sack at all, we can just have:
    //              std::vector<std::unique_ptr<T>> data;
    GroupSack group_sack;
};


}  // namespace libdnf5::comps


#endif  // LIBDNF5_COMPS_COMPS_SACK_IMPL_HPP
