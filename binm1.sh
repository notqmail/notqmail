#!/bin/sh

# Using splogger to send the log through syslog.
# Using binmail to deliver messages to /var/spool/mail/$USER by default.
# Using BSD 4.4 binmail interface: /usr/libexec/mail.local -r

exec env - PATH="QMAIL/bin:$PATH" \
qmail-start \
'|preline -f /usr/libexec/mail.local -r "${SENDER:-MAILER-DAEMON}" -d "$USER"' \
splogger qmail
