UNIX has traditionally delivered mail into a central spool directory,
/var/spool/mail. (The original name was /usr/spool/mail; some systems
now use /var/mail.) There are two basic problems with /var/spool/mail:

   * It's slow. On systems with thousands of users, /var/spool/mail has
     thousands of entries. A few UNIX systems support fast operations on
     large directories, but most don't.

   * It's insecure. Writing code that works safely in a world-writable
     directory is not easy. See, for example, CERT advisory 95:02.

These may not be problems at your site, so you may want to leave your
mailboxes in /var/spool/mail.

This file explains several ways that you can configure qmail to use
existing /var/spool/mail delivery tools. Please note that I do not vouch
for the security or reliability of any of those tools.


1. What to configure

The qmail system is started from /var/qmail/rc with

   qmail-start ./Mailbox splogger qmail

The first argument to qmail-start, ./Mailbox, is the default delivery
instruction. You can change it to run a program such as binmail or
procmail. (See dot-qmail.0 for the format of delivery instructions.)


2. Using procmail

You may already have installed procmail for mail filtering. procmail
delivers to /var/spool/mail by default.

To set up qmail to use procmail, simply copy /var/qmail/boot/proc to
/var/qmail/rc.

Note that procmail must be in your system's boot PATH; if it isn't, you
will have edit /var/qmail/rc to include the full path.


3. Using sendmail's delivery agent

sendmail uses binmail to deliver to /var/spool/mail. binmail is shipped
with the operating system as /bin/mail or /usr/libexec/mail.local.

There is some variation in binmail syntax among systems. The most common
interfaces are shown in /var/qmail/boot/binm1, /var/qmail/boot/binm2,
and /var/qmail/boot/binm3.
