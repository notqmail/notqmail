qmail's modular, lightweight design and sensible queue management make
it the fastest available message transfer agent. Here's how it stacks up
against the competition in five different speed measurements.

* Scheduling: I sent a message to 8192 ``trash'' recipients on my home
machine. All the deliveries were done in a mere 78 seconds---a rate of
over 9 million deliveries a day! Compare this to the speed advertised
for Zmailer's scheduling: 1.1 million deliveries a day on a
SparcStation-10/50. (My home machine is a 16MB Pentium-100 under BSD/OS,
with the default qmail configuration. qmail's logs were piped through
accustamp and written to disk as usual.)

* Local mailing lists: When qmail is delivering a message to a mailbox,
it physically writes the message to disk before it announces success---
that way, mail doesn't get lost if the power goes out. I tried sending a
message to 1024 local mailboxes on the same disk on my home machine; all
the deliveries were done in 25.5 seconds. That's more than 3.4 million
deliveries a day! Sending 1024 copies to a _single_ mailbox was just as
fast. Compare these figures to Zmailer's advertised rate for throwing
recipients away without even delivering the message---only 0.48 million
per day on the SparcStation.

* Mailing lists with remote recipients: qmail uses the same delivery
strategy that makes LSOFT's LSMTP so fast for outgoing mailing lists---
you choose how many parallel SMTP connections you want to run, and qmail
runs exactly that many. Of course, performance varies depending on how
far away your recipients are. The advantage of qmail over other packages
is its smallness: for example, one Linux user is running 60 simultaneous
connections, without swapping, on a machine with just 16MB of memory!

* Separate local messages: What LSOFT doesn't tell you about LSMTP is
how many _separate_ messages it can handle in a day. Does it get bogged
down as the queue fills up? On my home machine, I disabled qmail's
deliveries and then sent 5000 separate messages to one recipient. The
messages were all safely written to the queue disk in 23 minutes, with
no slowdown as the queue filled up. After I reenabled deliveries, all
the messages were delivered to the recipient's mailbox in under 12
minutes. End-to-end rate: more than 200000 individual messages a day!

* Overall performance: What really matters is how well qmail performs
with your mail load. Red Hat Software found one day that their mail hub,
a 48MB Pentium running sendmail 8.7, was running out of steam at 70000
messages a day. They shifted the load to qmail---on a _smaller_ machine,
a 16MB 486/66---and now they're doing fine.
