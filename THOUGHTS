Please note that this file is not called ``Internet Mail For Dummies.''
It _records_ my thoughts on various issues. It does not _explain_ them.
Paragraphs are not organized except by section. The required background
varies wildly from one paragraph to the next.

In this file, ``sendmail'' means Allman's creation; ``sendmail-clone''
means the program in this package.


1. Security

There are lots of interesting remote denial-of-service attacks on any
mail system. A long-term solution is to insist on prepayment for
unauthorized resource use. The tricky technical problem is to make the
prepayment enforcement mechanism cheaper than the expected cost of the
attacks. (For local denial-of-service attacks it's enough to be able to
figure out which user is responsible.)

qmail-send's log was originally designed for profiling. It subsequently
sprouted some tracing features. However, there's no way to verify
securely that a particular message came from a particular local user;
how do you know the recipient is telling you the truth about the
contents of the message? With QUEUE_EXTRA it'd be possible to record a
one-way hash of each outgoing message, but a user who wants to send
``bad'' mail can avoid qmail entirely.

I originally decided on security grounds not to put qmail advertisements
into SMTP responses: advertisements often act as version identifiers.
But this problem went away when I found a stable qmail URL.

As qmail grows in popularity, the mere knowledge that rcpthosts is so
easily available will deter people from setting up unauthorized MXs.
(I've never seen an unauthorized MX, but I can imagine that it would be
rather annoying.) Note that, unlike the bat book checkcompat() kludge,
rcpthosts doesn't interfere with mailing lists.

qmail-start doesn't bother with tty dissociation. On some old machines
this means that random people can send tty signals to the qmail daemons.
That's a security flaw in the job control subsystem, not in qmail.

The resolver library isn't too bloated (before 4.9.4, at least), but it
uses stdio, which _is_ bloated. Reading /etc/resolv.conf costs lots of
memory in each qmail-remote process. So it's tempting to incorporate a
smaller resolver library into qmail. (Bonus: I'd avoid system-specific
problems with old resolvers.) The problem is that I'd then be writing a
fundamentally insecure library. I'd no longer be able to blame the BIND
authors and vendors for the fact that attackers can easily use DNS to
steal mail. Solution: insist that the resolver run on the same host; the
kernel can guarantee the security of low-numbered 127.0.0.1 UDP ports.

NFS is the primary enemy of security partitioning under UNIX. Here's the
story. Sun knew from the start that NFS was completely insecure. It
tried to hide that fact by disallowing root access over NFS. Intruders
nevertheless broke into system after system, first obtaining bin access
and then obtaining root access. Various people thus decided to compound
Sun's error and build a wall between root and all other users: if all
system files are owned by root, and if there are no security holes other
than NFS, someone who breaks in via NFS won't be able to wipe out the
operating system---he'll merely be able to wipe out all user files. This
clueless policy means that, for example, all the qmail users have to be
replaced by root. See what I mean by ``enemy''? ... Basic NFS comments:
Aside from the cryptographic problem of having hosts communicate
securely, it's obvious that there's an administrative problem of mapping
client uids to server uids. If a host is secure and under your control,
you shouldn't have to map anything. If a host is under someone else's
control, you'll want to map his uids to one local account; it's his
client's job to decide which of his users get to talk NFS in the first
place. Sun's original map---root to nobody, everyone else left alone---
is, as far as I can tell, always wrong.


2. Injecting mail locally (qmail-inject, sendmail-clone)

RFC 822 section 3.4.9 prohibits certain visual effects in headers, and
the 822bis draft prohibits even more. qmail-inject could enforce these
absurd restrictions, but why waste the time? If you will suffer from
someone sending you ``flash mail,'' go find a better mail reader.

qmail-inject's ``Cc: recipient list not shown: ;'' successfully stops
sendmail from adding Apparently-To. Unfortunately, old versions of
sendmail will append a host name. This wasn't fixed until sendmail 8.7.
How many years has it been since RFC 822 came out?

sendmail discards duplicate addresses. This has probably resulted in
more lost and stolen mail over the years than the entire Chicago branch
of the United States Postal Service. The qmail system delivers messages
exactly as it's told to do. Along the same lines: qmail-inject is both
unable and unwilling to support anything like sendmail's (default)
nometoo option. Of course, a list manager could support nometoo.

There should be a mechanism in qmail-inject that does for envelope
recipients what Return-Path does for the envelope sender. Then
qmail-inject -n could print the recipients.

Should qmail-inject bounce messages with no recipients? Should there be
an option for this? If it stays as is (accept the message), qmail-inject
could at least avoid invoking qmail-queue.

It is possible to extract non-unique Message-IDs out of qmail-inject.
Here's how: stop qmail-inject before it gets to the third line of
main(), then wait until the pids wrap around, then restart qmail-inject
and blast the message through, then start another qmail-inject with the
same pid in the same second. I'm not sure how to fix this without
system-supplied sequence numbers. (Of course, the user could just type
in his own non-unique Message-IDs.)

The bat book says: ``Rules that hide hosts in a domain should be applied
only to sender addresses.'' Recipient masquerading works fine with
qmail. None of sendmail's pitfalls apply, basically because qmail has a
straight paper path.

I predicted that I would receive some pressure to make up for the
failings of MUA writers who don't understand the concept of reliability.
(``Like, duh, you mean I'm supposed to check the sendmail exit code?'')
I was right.


3. Receiving mail from the network (tcp-env, qmail-smtpd)

qmail-smtpd doesn't allow privacy-invading commands like VRFY and EXPN.
If you really want to publish such information, use a mechanism that
legitimate users actually know about, such as fingerd or httpd.

RFC 1123 says that VRFY and EXPN are important to track down cross-host
mailing list loops. With Delivered-To, mailing list loops do no damage,
_and_ one of the list administrators gets a bounce message that shows
exactly how the loop occurred. Solve the problem, not the symptom.

Should dns.c make special allowances for 127.0.0.1/localhost?

badmailfrom (like 8BITMIME) is a waste of code space.

In theory a MAIL or RCPT argument can contain unquoted LFs. In practice
there are a huge number of clients that terminate commands with just LF,
even if they use CR properly inside DATA.


4. Adding messages to the queue (qmail-queue)

Should qmail-queue try to make sure enough disk space is free in
advance? When qmail-queue is invoked by qmail-local or (with ESMTP)
qmail-smtpd or qmail-qmtpd or qmail-qmqpd, it could be told a size in
advance. I wish UNIX had an atomic allocate-disk-space routine... 

The qmail.h interface (reflecting the qmail-queue interface, which in
turn reflects the current queue file structure) is constitutionally
incapable of handling an address that contains a 0 byte. I can't imagine
that this will be a problem.

Should qmail-queue not bother queueing a message with no recipients?


5. Handling queued mail (qmail-send, qmail-clean)

The queue directory must be local. Mounting it over NFS is extremely
dangerous---not that this stops people from running sendmail that way!
Diskless hosts should use mini-qmail instead.

Queue reliability demands that single-byte writes be atomic. This is
true for a fixed-block filesystem such as UFS, and for a logging
filesystem such as LFS.

qmail-send uses 8 bytes of memory per queued message. Double that for
reallocation. (Fix: use a small forest of heaps; i.e., keep several
prioqs.) Double again for buddy malloc()s. (Fix: be clever about the
heap sizes.) 32 bytes is worrisome, but not devastating. Even on my
disk-heavy memory-light machine, I'd run out of inodes long before
running out of memory.

Some mail systems organize the queue by host. This is pointless as a
means of splitting up the queue directory. The real issue is what to do
when you suddenly find out that a host is up. For local SLIP/PPP links
you know in advance which hosts need this treatment, so you can handle
them with virtualdomains and serialmail.

For the old queue structure I implemented recipient list compression:
if mail goes out to a giant mailing list, and most of the recipients are
delivered, make a new, compressed, todo list. But this really isn't
worth the effort: it saves only a tiny bit of CPU time.

qmail-send doesn't have any notions of precedence, priority, fairness,
importance, etc. It handles the queue in first-seen-first-served order.
One could put a lot of work into doing something different, but that
work would be a waste: given the triggering mechanism and qmail's
deferral strategy, it is exceedingly rare for the queue to contain more
than one deliverable message at any given moment.

Exception: Even with all the concurrency tricks, qmail-send can end up
spending a few minutes on a mailing list with thousands of remote
entries. A user might send a new message to a remote address in the
meantime. The simplest way to handle this would be to put big messages
on a separate channel.

qmail-send will never start a pass for a job that it already has. This
means that, if one delivery takes longer than the retry interval, the
next pass will be delayed. I implemented the opposite strategy for the
old queue structure. Some hassles: mark() had to understand how job
input was buffered; every new delivery had to check whether the same
mpos in the same message was already being done.

Some things that qmail-send does synchronously: queueing a bounce
message; doing a cleanup via qmail-clean; classifying and rewriting all
the addresses in a new message. As usual, making these asynchronous
would require some housekeeping, but could speed things up a bit.
(I'm willing to assume POSIX waitpid() for asynchronous bounces; putting
an unbounded buffer into wait_pid() for the sake of NeXTSTEP 3 is not
worthwhile.)

Disk I/O is a bottleneck; UFS is reliable but it isn't fast. A good
logging filesystem offers much better performance, but logging
filesystems aren't widely available. Solution: Keep a journal, separate
from the queue, adequate to rebuild the queue (with at worst some
duplicate deliveries). Compress the journal. This would dramatically
reduce total disk I/O.

Bounce aggregation is a dubious feature. Bounce records aren't
crashproof; there can be a huge delay between a failure and a bounce;
the resulting bounce format is unnecessarily complicated. I'm tempted to
scrap the bounce directory and send one bounce for each failing
recipient, with appropriate modifications in the accompanying text.

qmail-stop implementation: setuid to UID_SEND; kill -TERM -1. Or run
qmail-start under an external service controller, such as supervise;
that's why it runs in the foreground.

The readdir() interface hides I/O errors. Lower-level interfaces would
lead me into a thicket of portability problems. I'm really not sure what
to do about this. Of course, a hard I/O error means that mail is toast,
but a soft I/O error shouldn't cause any trouble.

job_open() or pass_dochan() could be paranoid about the same id,channel
already being open; but, since messdone() is so paranoid, the worst
possible effect of a bug along these lines would be double delivery.

Mathematical amusement: The optimal retry schedule is essentially,
though not exactly, independent of the actual distribution of message
delay times. What really matters is how much cost you assign to retries
and to particular increases in latency. qmail's current quadratic retry
schedule says that an hour-long delay in a day-old message is worth the
same as a ten-minute delay in an hour-old message; this doesn't seem so
unreasonable.

Insider information: AOL retries their messages every five minutes for
three days straight. Hmmm.


6. Sending mail through the network (qmail-rspawn, qmail-remote)

Are there any hosts, anywhere, whose mailers are bogged down by huge
messages to multiple recipients at a single host? For typical hosts,
multiple RCPTs per SMTP aren't an ``efficiency feature''; they're a
_slowness_ feature. Separate SMTP transactions have much lower latency.

I've heard three complaints about bandwidth use from masochists sending
messages through a modem through a smarthost to thousands of users---
without sublists! They can get much better performance with QMQP.

In the opposite direction: It's tempting to remove the @host part of the
qmail-remote recip argument. Or at least avoid double-dns_cname.

There are lots of reasons that qmail-rspawn should take a more active
role in qmail-remote's activities. It should call separate programs to
do (1) MX lookups, (2) SMTP connections, (3) QMTP connections. (But this
wouldn't be so important if the DNS library didn't burn so much memory.)

I bounce ambiguous MXs. (An ``ambiguous MX'' is a best-preference MX
record sending me mail for a host that I don't recognize as local.)
Automatically treating ambiguous MXs as local is incompatible with my
design decision to keep local delivery working when the network goes
down. It puts more faith in DNS than DNS deserves. Much better: Have
your MX records generated automatically from control/locals.

If I successfully connect to an MX host but it temporarily refuses to
accept the message, I give up and put the message back into the queue.
But several documents seem to suggest that I should try further MX
records. What are they thinking? My approach deals properly with downed
hosts, hosts that are unreachable through a firewall, and load
balancing; what else do people use multiple MX records for?

Currently qmail-remote sends data in 1024-byte buffers. Perhaps it
should try to take account of the MTU.

Perhaps qmail-remote should allocate a fixed amount of DNS/connect()
time across any number of MXs; this idea is due to Mark Delany.

RFC 821 doesn't say what it means by ``text.'' qmail-remote assumes that
the server's reply text doesn't contain bare LFs.

RFC 821 and RFC 1123 prohibit host names in MAIL FROM and RCPT TO from
being aliases. qmail-remote, like sendmail, rewrites aliases in RCPT;
people who don't list aliases in control/locals or sendmail's Cw are
implicitly relying on this conversion. It is course quite silly for an
internal DNS detail to have such an effect on mail delivery, but that's
how the Internet works. On the other hand, the compatibility arguments
do not apply to MAIL FROM. qmail-remote no longer bothers with CNAME
lookups for the envelope sender host.


7. Delivering mail locally (qmail-lspawn, qmail-local)

qmail-local doesn't support comsat. comsat is a pointless abomination.
Use qbiff if you want that kind of notification.

The getpwnam() interface hides I/O errors. Solution: qmail-pw2u.


8. sendmail V8's new features

sendmail-8.8.0/doc/op/op.me includes a list of big improvements of
sendmail 8.8.0 over sendmail 5.67. Here's how qmail stacks up against
each of those improvements. (Of course, qmail has its own improvements,
but that's not the point of this list.)

Connection caching, MX piggybacking: Nope. (Profile. Don't speculate.)

Response to RCPT command is fast: Yup.

IP addresses show up in Received lines: Yup.

Self domain literal is properly handled: Yup.

Different timeouts for QUIT, RCPT, etc.: No, just a single timeout.

Proper <> handling, route-address pruning: Yes, but not configurable.

ESMTP support: Yup. (Server-side, including PIPELINING.)

8-bit clean: Yup. (Including server-side 8BITMIME support; same as
sendmail with the 8 option.)

Configurable user database: Yup.

BIND support: Yup.

Keyed files: Yes, in fastforward.

931/1413/Ident/TAP: Yup.

Correct 822 address list parsing: Yup. (Note that sendmail still has
some major problems with quoting.)

List-owner handling: Yup.

Dynamic header allocation: Yup.

Minimum number of disk blocks: Yes, via tunefs -m. (Or quotas; the right
setup has qmailq with a small quota, qmails with a larger quota, so that
qmail-send always has room to work.)

Checkpointing: Yes, but not configurable---qmail always checkpoints.

Error message configuration: Nope.

GECOS matching: Not directly, but easy to hook in.

Hop limit configuration: No. (qmail's limit is 100 hops. qmail offers
automatic loop protection much more advanced than hop counting.)

MIME error messages: No. (qmail uses QSBMF error messages, which are
much easier to parse.)

Forward file path: Yes, via /etc/passwd.

Incoming SMTP configuration: Yes, via inetd or tcpserver.

Privacy options: Yes, but they're not options.

Best-MX mangling: Nope. See section 6 for further discussion.

7-bit mangling: Nope. qmail always uses 8 bits.

Support for up to 20 MX records: Yes, and more. qmail has no limits
other than memory.

Correct quoting of name-and-address headers: Yup.

VRFY and EXPN now different: Nope. qmail always hides this information.

Multi-word classes, deferred macro expansion, separate envelope/header
$g processing, separate per-mailer envelope and header processing, new
command line flags, new configuration lines, new mailer flags, new
macros: These are sendmail-specific; they wouldn't even make sense for
qmail. For example, _of course_ qmail handles envelopes and headers
separately; they're almost entirely different objects!


9. Miscellany

sendmail-clone and qsmhook are too bletcherous to be documented. (The
official replacement for qsmhook is preline, together with the
qmail-command environment variables.)

I've considered making install atomic, but this is very difficult to do
right, and pointless if it isn't done right.

RN suggests automatically putting together a reasonable set of lines for
/etc/passwd. I perceive this as getting into the adduser business, which
is worrisome: I'll be lynched the first time I screw up somebody's
passwd file. This should be left to OS-specific installation scripts.

The BSD 4.2 inetd didn't allow a username. I think I can safely forget
about this. (DS notes that the username works under Ultrix even though
it's undocumented.)

I should clean up the bput/put choices.

Some of the stralloc_0()s indicate that certain lower-level routines
should grok stralloc.

qmail assumes that all times are positive; that pid_t, time_t and ino_t
fit into unsigned long; that gid_t fits into int; that the character set
is ASCII; and that all pointers are interchangeable. Do I care?

The bat book justifies sendmail's insane line-splitting mechanism by
pointing out that it might be useful for ``a 40-character braille
print-driving program.'' C'mon, guys, is that your best excuse?

qmail's mascot is a dolphin.
