Here's how to remove binmail from your system. Don't do this if you have
configured qmail to use binmail for local delivery.


1. Find the binmail binary on your system: /usr/libexec/mail.local if
   that exists, otherwise /bin/mail.

2. Remove permissions from the binmail binary:
      # chmod 0 /usr/libexec/mail.local

3. If the binmail binary was /bin/mail, make sure that ``mail'' still
   invokes a usable mailer. Under SVR4 you may want to link mail to
   mailx.

4. Comment out the comsat line in /etc/inetd.conf, and kill -HUP your
   inetd.
