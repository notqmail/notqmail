# Open Build Service Steps before release

This document is mostly for the notqmail maintainers, but can also be of use to anyone interested in OBS packaging or building their own.

1. update version number, release number in conf-version, conf-release
2. Run make which will create notqmail.spec, notqmail.dsc, \_service, debian.tar.gz.
3. Transfer the following files to open build service repo
   debian.tar.gz
   notqmail.changes
   notqmail.dsc
   notqmail-permissions.easy
   notqmail-permissions.paranoid
   notqmail-permissions.secure
   notqmail-rpmlintrc
   notqmail.spec
   \_service
   system-users-qmail.conf

# Creating rpm locally

1. Ensure you have packages listed in BuildRequires in notqmail.spec installed on the system.
   These will be rpm-build, gcc, coreutils, glibc, glibc-devel
   on opensuse systems install sysuser-tools additionally
2. Run create\_rpm script

# Creating rpm locally

1. Ensure you have following packages installed on the system.
   These will be build-essential, cdbs, debhelper
2. Run create\_debian script
