You can do several tests of qmail delivery without setting up qmail to
accept messages through SMTP or through /usr/lib/sendmail:

1. After you start qmail, look for a
           qmail: status: local 0/10 remote 0/20
   line in syslog. qmail-send always prints either ``cannot start'' or
   ``status''. (The big number is a splogger timestamp.)

2. Do a ps and look for the qmail daemons. There should be four of
   them, all idle: qmail-send, running as qmails; qmail-lspawn, running
   as root; qmail-rspawn, running as qmailr; and qmail-clean, running
   as qmailq. You will also see splogger, running as qmaill.

3. Local-local test: Send yourself an empty message. (Replace ``me''
   with your username. Make sure to include the ``to:'' colon.)
      % echo to: me | /var/qmail/bin/qmail-inject
   The message will show up immediately in your mailbox, and syslog
   will show something like this:
           qmail: new msg 53 
           qmail: info msg 53: bytes 246 from <me@domain> qp 20345 uid 666
           qmail: starting delivery 1: msg 53 to local me@domain 
           qmail: status: local 1/10 remote 0/20
           qmail: delivery 1: success: did_1+0+0/
           qmail: status: local 0/10 remote 0/20
           qmail: end msg 53 
   (53 is an inode number; 20345 is a process ID; your numbers will
   probably be different.)

4. Local-error test: Send a message to a nonexistent local address.
      % echo to: nonexistent | /var/qmail/bin/qmail-inject
           qmail: new msg 53 
           qmail: info msg 53: bytes 246 from <me@domain> qp 20351 uid 666
           qmail: starting delivery 2: msg 53 to local nonexistent@domain
           qmail: status: local 1/10 remote 0/20
           qmail: delivery 2: failure: No_such_address.__#5.1.1_/
           qmail: status: local 0/10 remote 0/20
           qmail: bounce msg 53 qp 20357
           qmail: end msg 53 
           qmail: new msg 54 
           qmail: info msg 54: bytes 743 from <> qp 20357 uid 666
           qmail: starting delivery 3: msg 54 to local me@domain
           qmail: status: local 1/10 remote 0/20
           qmail: delivery 3: success: did_1+0+0/
           qmail: status: local 0/10 remote 0/20
           qmail: end msg 54 
   You will now have a bounce message in your mailbox.

5. Local-remote test: Send an empty message to your account on another
   machine.
      % echo to: me@wherever | /var/qmail/bin/qmail-inject
           qmail: new msg 53 
           qmail: info msg 53: bytes 246 from <me@domain> qp 20372 uid 666
           qmail: starting delivery 4: msg 53 to remote me@wherever
           qmail: status: local 0/10 remote 1/20
           qmail: delivery 4: success: 1.2.3.4_accepted_message./...
           qmail: status: local 0/10 remote 0/20
           qmail: end msg 53 
   There will be a pause between ``starting delivery'' and ``success'';
   SMTP is slow. Check that the message is in your mailbox on the other
   machine.

6. Local-postmaster test: Send mail to postmaster, any capitalization.
      % echo to: POSTmaster | /var/qmail/bin/qmail-inject
   Look for the message in the alias mailbox, normally ~alias/Mailbox.

7. Double-bounce test: Send a message with a completely bad envelope.
      % /var/qmail/bin/qmail-inject -f nonexistent
      To: unknownuser
      Subject: testing

      This is a test. This is only a test.
      %
   (Use end-of-file, not dot, to end the message.) Look for the double
   bounce in the alias mailbox.

8. Group membership test:
      % cat > ~me/.qmail-groups
      |groups >> MYGROUPS; exit 0
      % /var/qmail/bin/qmail-inject me-groups < /dev/null
      % cat ~me/MYGROUPS
   MYGROUPS will show your normal gid and nothing else. (Under Solaris,
   make sure to use /usr/ucb/groups; /usr/bin/groups is broken.)
