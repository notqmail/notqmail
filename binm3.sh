#!/bin/sh

# Using splogger to send the log through syslog.
# Using binmail to deliver messages to /var/spool/mail/$USER by default.
# Using V7 binmail interface: /bin/mail -f

exec env - PATH="QMAIL/bin:$PATH" \
qmail-start \
'|preline -f /bin/mail -f "${SENDER:-MAILER-DAEMON}" -d "$USER"' \
splogger qmail
