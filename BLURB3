Here are some of qmail's features. 

Setup:
*  automatic adaptation to your UNIX variant---no configuration needed
*  AIX, BSD/OS, FreeBSD, HP/UX, Irix, Linux, OSF/1, SunOS, Solaris, and more
*  automatic per-host configuration (config, config-fast)
*  quick installation---no big list of decisions to make

Security:
*  clear separation between addresses, files, and programs
*  minimization of setuid code (qmail-queue)
*  minimization of root code (qmail-start, qmail-lspawn)
*  five-way trust partitioning---security in depth
*  optional logging of one-way hashes, entire contents, etc. (QUEUE_EXTRA)

Message construction (qmail-inject):
*  RFC 822, RFC 1123
*  full support for address groups
*  automatic conversion of old-style address lists to RFC 822 format
*  sendmail hook for compatibility with current user agents
*  header line length limited only by memory
*  host masquerading (control/defaulthost)
*  user masquerading ($MAILUSER, $MAILHOST)
*  automatic Mail-Followup-To creation ($QMAILMFTFILE)

SMTP service (qmail-smtpd):
*  RFC 821, RFC 1123, RFC 1651, RFC 1652, RFC 1854
*  8-bit clean
*  931/1413/ident/TAP callback (tcp-env)
*  relay control---stop unauthorized relaying by outsiders (control/rcpthosts)
*  no interference between relay control and forwarding
*  tcpd hook---reject SMTP connections from known abusers
*  automatic recognition of local IP addresses
*  per-buffer timeouts
*  hop counting

Queue management (qmail-send):
*  instant handling of messages added to queue
*  parallelism limit (control/concurrencyremote, control/concurrencylocal)
*  split queue directory---no slowdown when queue gets big
*  quadratic retry schedule---old messages tried less often
*  independent message retry schedules
*  automatic safe queueing---no loss of mail if system crashes
*  automatic per-recipient checkpointing
*  automatic queue cleanups (qmail-clean)
*  queue viewing (qmail-qread)
*  detailed delivery statistics (qmailanalog, available separately)

Bounces (qmail-send):
*  QSBMF bounce messages---both machine-readable and human-readable
*  HCMSSC support---language-independent RFC 1893 error codes
*  double bounces sent to postmaster

Routing by domain (qmail-send):
*  any number of names for local host (control/locals)
*  any number of virtual domains (control/virtualdomains)
*  domain wildcards (control/virtualdomains)
*  configurable percent hack support (control/percenthack)
*  UUCP hook

SMTP delivery (qmail-remote):
*  RFC 821, RFC 974, RFC 1123
*  8-bit clean
*  automatic downed host backoffs
*  artificial routing---smarthost, localnet, mailertable (control/smtproutes)
*  per-buffer timeouts
*  passive SMTP queue---perfect for SLIP/PPP (serialmail, available separately)

Forwarding and mailing lists (qmail-local):
*  address wildcards (.qmail-default, .qmail-foo-default, etc.)
*  sendmail .forward compatibility (dot-forward, available separately)
*  fast forwarding databases (fastforward, available separately)
*  sendmail /etc/aliases compatibility (fastforward/newaliases)
*  mailing list owners---automatically divert bounces and vacation messages
*  VERPs---automatic recipient identification for mailing list bounces
*  Delivered-To---automatic loop prevention, even across hosts
*  automatic mailing list management (ezmlm, available separately)

Local delivery (qmail-local):
*  user-controlled address hierarchy---fred controls fred-anything
*  mbox delivery
*  reliable NFS delivery (maildir)
*  user-controlled program delivery: procmail etc. (qmail-command)
*  optional new-mail notification (qbiff)
*  optional NRUDT return receipts (qreceipt)
*  conditional filtering (condredirect, bouncesaying)

POP3 service (qmail-popup, qmail-pop3d):
*  RFC 1939
*  UIDL support
*  TOP support
*  APOP hook
*  modular password checking (checkpassword, available separately)
