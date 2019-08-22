%undefine _missing_build_ids_terminate_build
%global _unpackaged_files_terminate_build 1
%global debug_package %{nil}

%define nq_docdir          %{_docdir}/%{name}-doc
%if %{defined _project}
# define build_on_obs if building on openSUSE build service
%global build_on_obs       1
%else
%define _project           local
%global build_on_obs       0
%global _hardened_build    1
%endif
%global qmaildir           /var/qmail
#log directory if we use multilog in supervise scripts
%global logdir             /var/log/svc
%global see_base           For a description of notqmail visit https://notqmail.org/
%if 0%{?suse_version}
%global noperms            1
%else
%global noperms            0
%endif

Name: notqmail
Version: 1.07
Release: 1.0%{?dist}
Summary: A community driven fork of qmail
License: CC-PDDC
URL: https://notqmail.org
Source0: notqmail-1.07.tar.xz
%if %noperms == 0
%if 0%{?suse_version} >= 1120
Source1:%{name}-permissions.easy
Source2:%{name}-permissions.secure
Source3:%{name}-permissions.paranoid
%endif
%endif
Source5:system-users-qmail.conf
%if %{undefined suse_version} && %{undefined sles_version}
Group: System Environment/Base
%else
Group: Productivity/Networking/Email/Servers
%endif
BuildRequires: rpm gcc make binutils coreutils grep
BuildRequires: glibc glibc-devel procps
BuildRequires: diffutils

%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version}
Requires(pre): shadow-utils
Requires(postun): shadow-utils
%endif
%if 0%{?suse_version} || 0%{?sles_version}
Requires(pre): pwdutils
Requires(postun): pwdutils
%endif
%if %{undefined fedora_version} && %{undefined centos_version} && %{undefined rhel_version} && %{undefined sles_version} && %{undefined suse_version}
BuildRequires: sysuser-tools
%endif
%if 0%{?suse_version} >= 1500 || 0%{?sles_version} >= 15
BuildRequires: sysuser-tools
%endif


%if %build_on_obs == 1
##################################### OBS ####################################
%if 0%{?suse_version}
BuildRequires: -post-build-checks  
#!BuildIgnore: post-build-checks  
%endif
##############################################################################
%endif

Requires: /usr/sbin/useradd /usr/sbin/userdel /usr/sbin/groupadd /usr/sbin/groupdel
Requires: /sbin/chkconfig procps /usr/bin/awk /usr/bin/which
Requires: coreutils grep /bin/sh glibc
Requires: binutils sed findutils
%if "%{?_unitdir}" == ""
Requires: initscripts
%endif
%if %noperms == 0
%if 0%{?suse_version} >= 1120
PreReq: permissions
%endif
%endif

Provides: user(alias)       > 999
Provides: user(qmaild)      > 999
Provides: user(qmaill)      > 999
Provides: user(qmailp)      > 999
Provides: user(qmailq)      > 999
Provides: user(qmailr)      > 999
Provides: user(qmails)      > 999
Provides: group(nofiles)    > 999
Provides: group(qmail)      > 999
Provides: smtp_daemon
%if %{undefined fedora_version} && %{undefined centos_version} && %{undefined rhel_version} && %{undefined sles_version} && %{undefined suse_version}
%sysusers_requires
%endif
%if 0%{?suse_version} >= 1500 || 0%{?sles_version} >= 15
%sysusers_requires
%endif

%if %build_on_obs == 1
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXXX)
%endif

%description
notqmail is a community-driven fork of qmail, beginning where netqmail
left off: providing stable, compatible, small releases to which existing
qmail users can safely update.
notqmail also aims higher: developing an extensible, easily packaged, and
increasingly useful modern mail server.

%package      doc
Summary:        Documentations for the %{name} package
%if %{undefined suse_version} && %{undefined sles_version}
Group: System Environment/Base
%else
Group: Productivity/Networking/Email/Servers
%endif
BuildArch:      noarch

%description doc
notqmail is a community-driven fork of qmail, beginning where netqmail
left off: providing stable, compatible, small releases to which existing
qmail users can safely update.
notqmail also aims higher: developing an extensible, easily packaged, and
increasingly useful modern mail server.
This package contains the documentation for %{name}

%{see_base}

%prep
%setup

%build
%if %{undefined fedora_version} && %{undefined centos_version} && %{undefined rhel_version} && %{undefined sles_version} && %{undefined suse_version}
%sysusers_generate_pre %{S:5} notqmail
%endif
%if 0%{?suse_version} >= 1500 || 0%{?sles_version} >= 15
%sysusers_generate_pre %{S:5} notqmail
%endif
make -s it man NROFF=true

%install
env DESTDIR=%{buildroot} ./instpackage
%{__rm} -rf %{buildroot}%{qmaildir}/man/man3 %{buildroot}%{qmaildir}/man/cat*
mkdir -p %{buildroot}%{_mandir}
mv %{buildroot}%{qmaildir}/man/man? %{buildroot}%{_mandir}
mkdir -p %{buildroot}%{nq_docdir}
mv %{buildroot}%{qmaildir}/doc/* %{buildroot}%{nq_docdir}
%{__cp} COPYRIGHT %{buildroot}%{nq_docdir}
%if %noperms == 0
%if 0%{?suse_version} >= 1120
mkdir -p %{buildroot}%{_sysconfdir}/permissions.d/
install -m 644 %{S:1} %{buildroot}%{_sysconfdir}/permissions.d/%{name}-permissions
install -m 644 %{S:2} %{buildroot}%{_sysconfdir}/permissions.d/%{name}-permissions.secure
%endif
%endif
%if "%{?_unitdir}" != ""
  install -D -m 644 qmail-send.service %{buildroot}%{_unitdir}/%{name}.service
%endif

%files
%defattr(-, root, root,-)
%dir %attr(755,root,qmail)        %{qmaildir}
%dir %attr(755,root,qmail)        %{qmaildir}/bin
%dir %attr(755,root,qmail)        %{qmaildir}/boot
%dir %attr(755,root,qmail)        %{qmaildir}/control
%dir %attr(755,root,qmail)        %{qmaildir}/users
%dir %attr(2755,alias,qmail)      %{qmaildir}/alias
%dir %attr(750,qmailq,qmail)      %{qmaildir}/queue
%dir %attr(700,qmails,qmail)      %{qmaildir}/queue/bounce
     %attr(700,qmails,qmail)      %{qmaildir}/queue/info
%dir %attr(700,qmailq,qmail)      %{qmaildir}/queue/intd
     %attr(700,qmails,qmail)      %{qmaildir}/queue/local
%dir %attr(750,qmailq,qmail)      %{qmaildir}/queue/lock
%attr(600,qmails,qmail)           %{qmaildir}/queue/lock/sendmutex
%attr(644,qmailr,qmail)           %{qmaildir}/queue/lock/tcpto
%attr(622,qmails,qmail)           %{qmaildir}/queue/lock/trigger
     %attr(750,qmailq,qmail)      %{qmaildir}/queue/mess
%dir %attr(700,qmailq,qmail)      %{qmaildir}/queue/pid
     %attr(700,qmails,qmail)      %{qmaildir}/queue/remote
%dir %attr(750,qmailq,qmail)      %{qmaildir}/queue/todo
%attr(0700,root,qmail)            %{qmaildir}/bin/qmail-lspawn
%attr(0711,root,qmail)            %{qmaildir}/bin/splogger
%attr(0755,root,qmail)            %{qmaildir}/bin/preline
%attr(0700,root,qmail)            %{qmaildir}/bin/qmail-newu
%attr(0755,root,qmail)            %{qmaildir}/bin/qsmhook
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-popup
%attr(0755,root,qmail)            %{qmaildir}/bin/qail
%attr(0755,root,qmail)            %{qmaildir}/bin/qreceipt
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-showctl
%attr(0755,root,qmail)            %{qmaildir}/bin/maildir2mbox
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-qread
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-getpw
%attr(0755,root,qmail)            %{qmaildir}/bin/bouncesaying
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-smtpd
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-inject
%attr(0755,root,qmail)            %{qmaildir}/bin/sendmail
%attr(0755,root,qmail)            %{qmaildir}/bin/except
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-pop3d
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-clean
%attr(0755,root,qmail)            %{qmaildir}/bin/qbiff
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-qstat
%attr(0755,root,qmail)            %{qmaildir}/bin/mailsubj
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-tcpok
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-tcpto
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-qmtpd
%attr(0700,root,qmail)            %{qmaildir}/bin/qmail-start
%attr(4711,qmailq,qmail)          %{qmaildir}/bin/qmail-queue
%attr(0755,root,qmail)            %{qmaildir}/bin/maildirmake
%attr(0755,root,qmail)            %{qmaildir}/bin/maildirwatch
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-qmqpd
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-pw2u
%attr(0755,root,qmail)            %{qmaildir}/bin/datemail
%attr(0700,root,qmail)            %{qmaildir}/bin/qmail-newmrh
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-local
%attr(0755,root,qmail)            %{qmaildir}/bin/predate
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-send
%attr(0755,root,qmail)            %{qmaildir}/bin/pinq
%attr(0755,root,qmail)            %{qmaildir}/bin/forward
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-rspawn
%attr(0755,root,qmail)            %{qmaildir}/bin/qmail-qmqpc
%attr(0755,root,qmail)            %{qmaildir}/bin/condredirect
%attr(0755,root,qmail)            %{qmaildir}/bin/tcp-env
%attr(0711,root,qmail)            %{qmaildir}/bin/qmail-remote
%attr(0755,root,qmail)            %{qmaildir}/bin/elq

%attr(0755,root,qmail)            %{qmaildir}/boot/proc+df
%attr(0755,root,qmail)            %{qmaildir}/boot/proc
%attr(0755,root,qmail)            %{qmaildir}/boot/home
%attr(0755,root,qmail)            %{qmaildir}/boot/binm2+df
%attr(0755,root,qmail)            %{qmaildir}/boot/binm3
%attr(0755,root,qmail)            %{qmaildir}/boot/home+df
%attr(0755,root,qmail)            %{qmaildir}/boot/binm1
%attr(0755,root,qmail)            %{qmaildir}/boot/binm2
%attr(0755,root,qmail)            %{qmaildir}/boot/binm1+df
%attr(0755,root,qmail)            %{qmaildir}/boot/binm3+df
%if "%{?_unitdir}" != ""
                                  %{_unitdir}/%{name}.service
%endif

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%attr(644,root,root)                               %{_sysconfdir}/permissions.d/%{name}-permissions
%attr(644,root,root)                               %{_sysconfdir}/permissions.d/%{name}-permissions.secure
%endif
%endif
%if %noperms == 0
%if 0%{?suse_version} >= 1120
%verify (not user group mode) %attr(6551, qmailq, qmail)   %{qmaildir}/bin/qmail-queue
%verify (not user group mode) %attr(2555, alias, qmail)    %{qmaildir}/alias
%endif
%endif

%files doc
%{_mandir}/man?/*%{?ext_man}
%{nq_docdir}

### SCRIPTLETS ###############################################################################
%verifyscript
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%verify_permissions -e %{qmaildir}/sbin/qmail-queue
%verify_permissions -e %{qmaildir}/alias
%endif
%endif

%pretrans
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if test -f %{_sysconfdir}/systemd/system/multi-user.target.wants/notqmail.service
then
  echo "Giving %{name} exactly 5 seconds to exit nicely"
  /bin/systemctl stop notqmail > /dev/null 2>&1
fi
sleep 5

%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?suse_version} < 1500 || 0%{?sles_version < 15}
%pre
/usr/bin/getent group nofiles   > /dev/null || /usr/sbin/groupadd nofiles
/usr/bin/getent group qmail     > /dev/null || /usr/sbin/groupadd qmail
/usr/bin/getent passwd alias    > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{qmaildir}/alias  -s /sbin/nologin alias
/usr/bin/getent passwd qmaild   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{qmaildir}        -s /sbin/nologin qmaild
/usr/bin/getent passwd qmaill   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{logdir}          -s /sbin/nologin qmaill
/usr/bin/getent passwd qmailp   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{qmaildir}        -s /sbin/nologin qmailp
/usr/bin/getent passwd qmailq   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{qmaildir}        -s /sbin/nologin qmailq
/usr/bin/getent passwd qmailr   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{qmaildir}        -s /sbin/nologin qmailr
/usr/bin/getent passwd qmails   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{qmaildir}        -s /sbin/nologin qmails
for i in alias qmaild qmaill qmailp qmailq qmailr qmails qscand
do
  %{__rm} -f /var/spool/mail/$i
done
%else
%pre -f notqmail.pre
%endif

%service_add_pre %{name}.service

### SCRIPTLET ###############################################################################
%post
%if %noperms == 0
%if 0%{?suse_version} >= 1120
%if 0%{?set_permissions:1} > 0
  if [ ! -f /tmp/no_permissions ] ; then
    %set_permissions %{name}
  fi
%else
  if [ ! -f /tmp/no_permissions ] ; then
    %run_permissions
  fi
%endif
%endif
%endif
#%{qmaildir}/bin/instchown
%service_add_post %{name}.service

### SCRIPTLET ###############################################################################
%preun
argv1=$1
if [ -z "$argv1" ] ; then
  argv1=0
fi
# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
  exit 0
fi
%service_del_preun %{name}.service

### SCRIPTLET ###############################################################################
%postun
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if [ -z "$argv1" ] ; then
  argv1=0
fi
# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
  exit 0
fi
%service_del_postun %{name}.service
(
# remove users / groups
for i in alias qmaild qmaill qmailp qmailq qmailr qmails
do
  echo "Removing user $i"
  /usr/bin/getent passwd $i > /dev/null && /usr/sbin/userdel $i >/dev/null || true
done
for i in nofiles qmail
do
  echo "Removing group $i"
  /usr/bin/getent group $i > /dev/null && /usr/sbin/groupdel $i  >/dev/null || true
done

for i in postmaster mailer-daemon root
do
  %{__rm} -f %{qmaildir}/alias/.qmail-"$i"
done
%{__rm} -rf %{qmaildir}/alias/Maildir
) >> /tmp/%{name}-setup.log 2>&1

### SCRIPTLET ###############################################################################
%posttrans
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi

%changelog
* Sun Aug 11 2019 17:35:00 +0530 mbhangui@gmail.com 1.07-1.0
1. First release
