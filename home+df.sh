#!/bin/sh

# Using splogger to send the log through syslog.
# Using dot-forward to support sendmail-style ~/.forward files.
# Using qmail-local to deliver messages to ~/Mailbox by default.

exec env - PATH="QMAIL/bin:$PATH" \
qmail-start '|dot-forward .forward
./Mailbox' splogger qmail
