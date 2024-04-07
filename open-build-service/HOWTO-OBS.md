# Open Build Service Steps before release

This document is mostly for the notqmail maintainers, but can also be of use to anyone interested in OBS packaging or building their own.

1. update version number, release number in conf-version, conf-release
2. Run make which will create notqmail.spec, notqmail.dsc, \_service, debian.tar.gz.
3. Transfer the following files to open build service repo
   debian.tar.gz
   notqmail.changes
   notqmail.dsc
   notqmail-permissions.easy
   notqmail-permissions.secure
   notqmail-rpmlintrc
   notqmail.spec
   \_service
   system-users-qmail.conf

# Creating rpm locally

1. Ensure you have packages listed in BuildRequires in notqmail.spec installed on the system.
   These will be rpm, rpm-build, rpmdevtools, gcc, gcc-c++, coreutils, glibc, glibc-devel, git
   on opensuse systems install sysuser-tools additionally
2. Run create\_rpm script

# Creating debian locally

1. Ensure you have following packages installed on the system.
   These will be build-essential, cdbs, debhelper, git
2. Run create\_debian script

# Using podman or docker to build packages

You can use podman or docker to build binary packages locally. Using podman is better because it doesn't require a daemon running as root. Given below are two examples (one on Fedora and one on Ubuntu 23.04) demonstrating building the binary package

1. Pull an image for your distribution. e.g. ubuntu
   ```
   # for Ubuntu 23.04
   $ podman pull ubuntu:23.04

   # for Rocky Linux 9
   $ podman pull rockylinux:9
   ```

2. Get the IMAGE id for the image pulled. Here you can see the id for almalinux 9 is 7407e03f7ffc and id for Ubuntu 23.04 is f4cdeba72b99
   ```
   $ podman images
   REPOSITORY                          TAG         IMAGE ID      CREATED       SIZE
   docker.io/library/debian            bookworm    c978d997d5fe  3 weeks ago   121 MB
   docker.io/library/rockylinux        9           b72d2d915008  4 months ago  181 MB
   docker.io/library/almalinux         9           7407e03f7ffc  4 months ago  190 MB
   docker.io/library/ubuntu            23.04       f4cdeba72b99  4 months ago  72.8 MB
   ```

3. Run the container image

   <b>Example for debian package generation using podman</b>

   ```
   # run podman command to start ubuntu 23.04 in a container
   $ podman run --rm --name notqmail -h notqmail.org -ti f4cdeba72b99 bash

   # run apt-get update followed by apt-get install
   root@notqmail:/# apt-get update
   root@notqmail:/# apt-get install build-essential cdbs debhelper git

   # clone notqmail repository
   root@notqmail:/# cd /usr/local/src/
   root@notqmail:/usr/local/src# git clone https://github.com/notqmail/notqmail.git
   Cloning into 'notqmail'...
   remote: Enumerating objects: 4422, done.
   remote: Counting objects: 100% (1249/1249), done.
   remote: Compressing objects: 100% (419/419), done.
   remote: Total 4422 (delta 903), reused 997 (delta 827), pack-reused 3173
   Receiving objects: 100% (4422/4422), 1.66 MiB | 488.00 KiB/s, done.
   Resolving deltas: 100% (2802/2802), done.

   # create the debian package using create_debian script
   root@notqmail:/usr/local/src# cd notqmail/open-build-service
   root@notqmail:/usr/local/src/notqmail/open-build-service# ./create_debian
   dpkg-buildpackage: info: binary-only upload (no source included)
   total 1032
   -rw-r--r-- 1 root root   2330 Apr  7 13:20 notqmail_1.08-1.1_amd64.changes
   -rw-r--r-- 1 root root   4805 Apr  7 13:20 notqmail_1.08-1.1_amd64.buildinfo
   -rw-r--r-- 1 root root 217452 Apr  7 13:20 notqmail_1.08-1.1_amd64.deb
   -rw-r--r-- 1 root root 821736 Apr  7 13:20 notqmail-dbgsym_1.08-1.1_amd64.ddeb

   # You can now copy /root/stage/notqmail_1.08-1.1_amd64.deb to any host for installation
   ```

   <b>Example for RPM package generation using podman</b>

   ```
   # run podman command to start ubuntu 23.04 in a container
   podman run --rm --name notqmail -h notqmail .org -ti b72d2d915008 bash

   # run dnf command to install essential packages
   [root@notqmail /]# dnf -y install gcc gcc-c++ rpm rpm-build rpmdevtools git

   # clone notqmail repository
   [root@notqmail /]# cd /usr/local/src
   [root@notqmail src]# git clone https://github.com/notqmail/notqmail.git
   Cloning into 'notqmail'...
   remote: Enumerating objects: 4422, done.
   remote: Counting objects: 100% (1249/1249), done.
   remote: Compressing objects: 100% (419/419), done.
   remote: Total 4422 (delta 903), reused 997 (delta 827), pack-reused 3173
   Receiving objects: 100% (4422/4422), 1.66 MiB | 1.02 MiB/s, done.
   Resolving deltas: 100% (2802/2802), done.

   # create the RPM package using create_rpm script
   root@notqmail:/usr/local/src# cd notqmail/open-build-service
   [root@notqmail open-build-service]# rpmdev-setuptree
   [root@notqmail open-build-service]# sed -i -e 's{rocky_version{rocky_ver{g' notqmail.spec.in
   [root@notqmail open-build-service]# git config tar.tar.xz.command "xz -c"
   [root@notqmail open-build-service]# ./create_rpm
   Checking for unpackaged file(s): /usr/lib/rpm/check-files /root/rpmbuild/BUILDROOT/notqmail-1.08-1.1.el9.x86_64
   Wrote: /root/rpmbuild/RPMS/x86_64/notqmail-debugsource-1.08-1.1.el9.x86_64.rpm
   Wrote: /root/rpmbuild/RPMS/noarch/notqmail-doc-1.08-1.1.el9.noarch.rpm
   Wrote: /root/rpmbuild/RPMS/x86_64/notqmail-1.08-1.1.el9.x86_64.rpm
   Wrote: /root/rpmbuild/RPMS/x86_64/notqmail-debuginfo-1.08-1.1.el9.x86_64.rpm
   Executing(%clean): /bin/sh -e /var/tmp/rpm-tmp.55TdJ1
   + umask 022
   + cd /root/rpmbuild/BUILD
   + cd notqmail-1.08
   + /usr/bin/rm -rf /root/rpmbuild/BUILDROOT/notqmail-1.08-1.1.el9.x86_64
   + RPM_EC=0
   ++ jobs -p
   + exit 0
   [root@notqmail open-build-service]# ls -l /root/rpmbuild/RPMS/x86_64/
   total 804
   -rw-r--r-- 1 root root 244201 Apr  7 13:44 notqmail-1.08-1.1.el9.x86_64.rpm
   -rw-r--r-- 1 root root 459340 Apr  7 13:44 notqmail-debuginfo-1.08-1.1.el9.x86_64.rpm
   -rw-r--r-- 1 root root 111086 Apr  7 13:44 notqmail-debugsource-1.08-1.1.el9.x86_64.rpm

   # You can now copy /root/rpmbuild/RPMS/x86_64/notqmail-1.08-1.1.el9.x86_64.rpm to any host for installation
   ```
