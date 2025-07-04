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

#include "libdnf5/comps/comps_sack.hpp"

#include "comps_sack_impl.hpp"
#include "solv/pool.hpp"

#include "libdnf5/comps/group/query.hpp"

namespace libdnf5::comps {

void CompsSack::Impl::load_config_excludes() {
    const auto & main_config = base->get_config();

    const auto & disable_excludes = main_config.get_disable_excludes_option().get_value();
    if (std::find(disable_excludes.begin(), disable_excludes.end(), "*") != disable_excludes.end()) {
        return;
    }
    if (std::find(disable_excludes.begin(), disable_excludes.end(), "main") != disable_excludes.end()) {
        return;
    }

    for (const auto & name : main_config.get_excludeenvs_option().get_value()) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(name, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            config_environment_excludes.insert(environment.get_environmentid());
        }
    }
    for (const auto & name : main_config.get_excludegroups_option().get_value()) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(name, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            config_group_excludes.insert(group->get_groupid());
        }
    }
}

const std::set<std::string> CompsSack::Impl::get_config_environment_excludes() {
    return config_environment_excludes;
}

const std::set<std::string> CompsSack::Impl::get_config_group_excludes() {
    return config_group_excludes;
}

const std::set<std::string> CompsSack::Impl::get_user_environment_excludes() {
    return user_environment_excludes;
}

void CompsSack::Impl::add_user_environment_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            user_environment_excludes.insert(environment.get_environmentid());
        }
    }
}

void CompsSack::Impl::add_user_environment_excludes(const EnvironmentQuery & excludes) {
    for (const auto & environment : excludes) {
        user_environment_excludes.insert(environment.get_environmentid());
    }
}

void CompsSack::Impl::remove_user_environment_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            user_environment_excludes.erase(environment.get_environmentid());
        }
    }
}

void CompsSack::Impl::remove_user_environment_excludes(const EnvironmentQuery & excludes) {
    for (const auto & environment : excludes) {
        user_environment_excludes.erase(environment.get_environmentid());
    }
}

void CompsSack::Impl::set_user_environment_excludes(const std::set<std::string> & excludes) {
    user_environment_excludes = std::set<std::string>();
    for (const auto & exclude : excludes) {
        EnvironmentQuery query(base, EnvironmentQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_environmentid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & environment : query.list()) {
            user_environment_excludes.insert(environment.get_environmentid());
        }
    }
}

void CompsSack::Impl::set_user_environment_excludes(const EnvironmentQuery & excludes) {
    user_environment_excludes = std::set<std::string>();
    for (const auto & environment : excludes) {
        user_environment_excludes.insert(environment.get_environmentid());
    }
}

void CompsSack::Impl::clear_user_environment_excludes() {
    user_environment_excludes = std::set<std::string>();
}

const std::set<std::string> CompsSack::Impl::get_user_group_excludes() {
    return user_group_excludes;
}

void CompsSack::Impl::add_user_group_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            user_group_excludes.insert(group->get_groupid());
        }
    }
}

void CompsSack::Impl::add_user_group_excludes(const GroupQuery & excludes) {
    for (const auto & group : excludes) {
        user_group_excludes.insert(group->get_groupid());
    }
}

void CompsSack::Impl::remove_user_group_excludes(const std::set<std::string> & excludes) {
    for (const auto & exclude : excludes) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            user_group_excludes.erase(group->get_groupid());
        }
    }
}

void CompsSack::Impl::remove_user_group_excludes(const GroupQuery & excludes) {
    for (const auto & group : excludes) {
        user_group_excludes.erase(group->get_groupid());
    }
}

void CompsSack::Impl::set_user_group_excludes(const std::set<std::string> & excludes) {
    user_group_excludes = std::set<std::string>();
    for (const auto & exclude : excludes) {
        GroupQuery query(base, GroupQuery::ExcludeFlags::IGNORE_EXCLUDES);
        query.filter_groupid(exclude, libdnf5::sack::QueryCmp::GLOB);
        for (const auto & group : query.list()) {
            user_group_excludes.insert(group->get_groupid());
        }
    }
}

void CompsSack::Impl::set_user_group_excludes(const GroupQuery & excludes) {
    user_group_excludes = std::set<std::string>();
    for (const auto & group : excludes) {
        user_group_excludes.insert(group->get_groupid());
    }
}

void CompsSack::Impl::clear_user_group_excludes() {
    user_group_excludes = std::set<std::string>();
}

CompsSackWeakPtr CompsSack::get_weak_ptr() {
    return CompsSackWeakPtr(this, &p_impl->sack_guard);
}

BaseWeakPtr CompsSack::get_base() const {
    return p_impl->base->get_weak_ptr();
}

CompsSack::CompsSack(const BaseWeakPtr & base) : p_impl{new Impl(base)} {}

CompsSack::CompsSack(libdnf5::Base & base) : CompsSack(base.get_weak_ptr()) {}

CompsSack::~CompsSack() = default;

void CompsSack::load_config_excludes() {
    p_impl->load_config_excludes();
}

const std::set<std::string> CompsSack::get_config_environment_excludes() {
    return p_impl->get_config_environment_excludes();
}

const std::set<std::string> CompsSack::get_config_group_excludes() {
    return p_impl->get_config_group_excludes();
}

const std::set<std::string> CompsSack::get_user_environment_excludes() {
    return p_impl->get_user_environment_excludes();
}

void CompsSack::add_user_environment_excludes(const EnvironmentQuery & excludes) {
    p_impl->add_user_environment_excludes(excludes);
}

void CompsSack::remove_user_environment_excludes(const EnvironmentQuery & excludes) {
    p_impl->remove_user_environment_excludes(excludes);
}

void CompsSack::set_user_environment_excludes(const EnvironmentQuery & excludes) {
    p_impl->set_user_environment_excludes(excludes);
}

void CompsSack::clear_user_environment_excludes() {
    p_impl->clear_user_environment_excludes();
}

const std::set<std::string> CompsSack::get_user_group_excludes() {
    return p_impl->get_user_group_excludes();
}

void CompsSack::add_user_group_excludes(const GroupQuery & excludes) {
    p_impl->add_user_group_excludes(excludes);
}

void CompsSack::remove_user_group_excludes(const GroupQuery & excludes) {
    p_impl->remove_user_group_excludes(excludes);
}

void CompsSack::set_user_group_excludes(const GroupQuery & excludes) {
    p_impl->set_user_group_excludes(excludes);
}

void CompsSack::clear_user_group_excludes() {
    p_impl->clear_user_group_excludes();
}

void CompsSack::Impl::init_group_sack() {
    libdnf5::solv::CompsPool & pool = get_comps_pool(base);

    if (group_sack.cached_solvables_size == pool.get_nsolvables()) {
        return;
    }

    // Map of available groups:
    //     For each groupid (SOLVABLE_NAME) have a vector of (repoid, solvable_id) pairs.
    //     Each pair consists of one solvable_id that represents one definition of the group
    //     and repoid of its originating repository.
    std::map<std::string, std::vector<std::pair<std::string_view, Id>>> available_map;
    Id solvable_id;
    Solvable * solvable;
    std::pair<std::string, std::string> solvable_name_pair;
    std::string_view repoid;

    std::set<std::string> config_excludes = get_config_group_excludes();
    std::set<std::string> user_excludes = get_user_group_excludes();

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        solvable = pool.id2solvable(solvable_id);

        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "group"
        solvable_name_pair = solv::CompsPool::split_solvable_name(pool.lookup_str(solvable_id, SOLVABLE_NAME));
        if (solvable_name_pair.first != "group") {
            continue;
        }

        repoid = solvable->repo->name;

        // Add installed groups directly, because there is only one solvable for each
        if (repoid == "@System") {
            std::unique_ptr<Group> group(new Group(base));
            group->add_group_id(GroupId(solvable_id));
            group_sack.add_group(std::move(group));
        } else {
            // Create map of available groups:
            // for each groupid (SOLVABLE_NAME), list all corresponding solvable_ids with repoids
            available_map[solvable_name_pair.second].insert(
                available_map[solvable_name_pair.second].end(), std::make_pair(repoid, solvable_id));
        }
    }

    // Create groups based on the available_map
    for (auto & item : available_map) {
        std::unique_ptr<Group> group(new Group(base));
        // Sort the vector of (repoid, solvable_id) pairs by repoid
        std::sort(item.second.begin(), item.second.end(), std::greater<>());
        // Create group_ids vector from the sorted solvable_ids
        for (const auto & solvableid_repoid_pair : item.second) {
            group->add_group_id(GroupId(solvableid_repoid_pair.second));
        }
        group_sack.add_group(std::move(group));
    }

    group_sack.cached_solvables_size = pool.get_nsolvables();
}

GroupSackWeakPtr CompsSack::Impl::get_group_sack() {
    return group_sack.get_weak_ptr();
};

}  // namespace libdnf5::comps
