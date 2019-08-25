#!/bin/sh

# Using splogger to send the log through syslog.
# Using procmail to deliver messages to /var/spool/mail/$USER by default.

exec env - PATH="QMAILBIN:$PATH" \
qmail-start '|preline procmail' splogger qmail
