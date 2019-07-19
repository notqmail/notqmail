You can do several tests of messages entering the qmail system:

1. SMTP server test: Forge some mail locally via SMTP. Replace ``me''
   with your username and ``domain'' with your host's name.
       % telnet 127.0.0.1 25
       Trying 127.0.0.1...
       Connected to 127.0.0.1.
       Escape character is '^]'.
       220 domain ESMTP
       helo dude
       250 domain
       mail <me@domain>
       250 ok
       rcpt <me@domain>
       250 ok
       data
       354 go ahead
       Subject: testing
       
       This is a test.
       .
       250 ok 812345679 qp 12345
       quit
       221 domain
       Connection closed by foreign host.
       %
   Look for the message in your mailbox. (Note for programmers: Most
   SMTP servers need more text after MAIL and RCPT. See RFC 821.)

2. Remote-local test: Send yourself some mail from another machine.
   Look for the message in your mailbox.

3. Remote-error test: Send some mail from another machine to
   nonexistent@domain. Look for a bounce message in the remote mailbox.

4. UA test: Try sending mail, first to a local account, then to a
   remote account, with your normal user agent.

5. Remote-postmaster test: Send mail from another machine to
   PoStMaStEr@domain. Look for the message in the alias mailbox,
   normally ~alias/Mailbox.
