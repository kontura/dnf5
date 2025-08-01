..
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

.. _remove_command_ref-label:

###############
 Remove Command
###############

Synopsis
========

``dnf5 remove [options] <package-spec-NF>|@<group-spec>|@<environment-spec>...``


Description
===========

The ``remove`` command in ``DNF5`` is used for removing installed packages, groups or
environments from the system.

If you want to keep the dependencies that were installed together with the given package,
set the ``clean_requirements_on_remove`` configuration option to ``False``.


Options
=======

``--no-autoremove``
    | Disable removal of dependencies that are no longer used.

``--offline``
    | Store the transaction to be performed offline. See :manpage:`dnf5-offline(8)`, :ref:`Offline command <offline_command_ref-label>`.


Examples
========

``dnf5 remove tito``
    | Remove the ``tito`` package.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
