1. Overview

Here's the data flow in the qmail suite:

 qmail-smtpd --- qmail-queue --- qmail-send --- qmail-rspawn --- qmail-remote
               /                     |      \
qmail-inject _/                 qmail-clean  \_ qmail-lspawn --- qmail-local

Every message is added to a central queue directory by qmail-queue.
qmail-queue is invoked as needed, usually by qmail-inject for locally
generated messages, qmail-smtpd for messages received through SMTP,
qmail-local for forwarded messages, or qmail-send for bounce messages.

Every message is then delivered by qmail-send, in cooperation with
qmail-lspawn and qmail-rspawn, and cleaned up by qmail-clean. These four
programs are long-running daemons.

The queue is designed to be crashproof, provided that the underlying
filesystem is crashproof. All cleanups are handled by qmail-send and
qmail-clean without human intervention. See section 6 for more details.


2. Queue structure

Each message in the queue is identified by a unique number, let's say
457. The queue is organized into several directories, each of which may
contain files related to message 457:

   mess/457: the message
   todo/457: the envelope: where the message came from, where it's going
   intd/457: the envelope, under construction by qmail-queue
   info/457: the envelope sender address, after preprocessing
   local/457: local envelope recipient addresses, after preprocessing
   remote/457: remote envelope recipient addresses, after preprocessing
   bounce/457: permanent delivery errors

Here are all possible states for a message. + means a file exists; -
means it does not exist; ? means it may or may not exist.

   S1. -mess -intd -todo -info -local -remote -bounce
   S2. +mess -intd -todo -info -local -remote -bounce
   S3. +mess +intd -todo -info -local -remote -bounce
   S4. +mess ?intd +todo ?info ?local ?remote -bounce (queued)
   S5. +mess -intd -todo +info ?local ?remote ?bounce (preprocessed)

Guarantee: If mess/457 exists, it has inode number 457.


3. How messages enter the queue

To add a message to the queue, qmail-queue first creates a file in a
separate directory, pid/, with a unique name. The filesystem assigns
that file a unique inode number. qmail-queue looks at that number, say
457. By the guarantee above, message 457 must be in state S1.

qmail-queue renames pid/whatever as mess/457, moving to S2. It writes
the message to mess/457. It then creates intd/457, moving to S3, and
writes the envelope information to intd/457.

Finally qmail-queue creates a new link, todo/457, for intd/457, moving
to S4. At that instant the message has been successfully queued, and
qmail-queue leaves it for further handling by qmail-send.

qmail-queue starts a 24-hour timer before touching any files, and
commits suicide if the timer expires.


4. How queued messages are preprocessed

Once a message has been queued, qmail-send must decide which recipients
are local and which recipients are remote. It may also rewrite some
recipient addresses.

When qmail-send notices todo/457, it knows that message 457 is in S4. It
removes info/457, local/457, and remote/457 if they exist. Then it reads
through todo/457. It creates info/457, possibly local/457, and possibly
remote/457. When it is done, it removes intd/457. The message is still
in S4 at this point. Finally qmail-send removes todo/457, moving to S5.
At that instant the message has been successfully preprocessed.


5. How preprocessed messages are delivered

Messages at S5 are handled as follows. Each address in local/457 and
remote/457 is marked either NOT DONE or DONE.

   DONE: The message was successfully delivered, or the last delivery
         attempt met with permanent failure. Either way, qmail-send
	 should not attempt further delivery to this address.

   NOT DONE: If there have been any delivery attempts, they have all
             met with temporary failure. Either way, qmail-send should
             try delivery in the future.

qmail-send may at its leisure try to deliver a message to a NOT DONE
address. If the message is successfully delivered, qmail-send marks the
address as DONE. If the delivery attempt meets with permanent failure,
qmail-send first appends a note to bounce/457, creating bounce/457 if
necessary; then it marks the address as DONE. Note that bounce/457 is
not crashproof.

qmail-send may handle bounce/457 at any time, as follows: it (1) injects
a new bounce message, created from bounce/457 and mess/457; (2) deletes
bounce/457.

When all addresses in local/457 are DONE, qmail-send deletes local/457.
Same for remote/457. 

When local/457 and remote/457 are gone, qmail-send eliminates the
message, as follows. First, if bounce/457 exists, qmail-send handles it
as described above. Once bounce/457 is definitely gone, qmail-send
deletes info/457, moving to S2, and finally mess/457, moving to S1.


6. Cleanups

If the computer crashes while qmail-queue is trying to queue a message,
or while qmail-send is eliminating a message, the message may be left in
state S2 or S3.

When qmail-send sees a message in state S2 or S3---other than one
it is currently eliminating!---where mess/457 is more than 36 hours old,
it deletes intd/457 if that exists, then deletes mess/457. Note that any
qmail-queue handling the message must be dead.

Similarly, when qmail-send sees a file in the pid/ directory that is
more than 36 hours old, it deletes it.

Cleanups are not necessary if the computer crashes while qmail-send is
delivering a message. At worst a message may be delivered twice. (There
is no way for a distributed mail system to eliminate the possibility of
duplication. What if an SMTP connection is broken just before the server
acknowledges successful receipt of the message? The client must assume
the worst and send the message again. Similarly, if the computer crashes
just before qmail-send marks a message as DONE, the new qmail-send must
assume the worst and send the message again. The usual solutions in the
database literature---e.g., keeping log files---amount to saying that
it's the recipient's computer's job to discard duplicate messages.)


7. Further notes

Currently info/457 serves two purposes: first, it records the envelope
sender; second, its modification time is used to decide when a message
has been in the queue too long. In the future info/457 may store more
information. Any non-backwards-compatible changes will be identified by
version numbers.

When qmail-queue has successfully placed a message into the queue, it
pulls a trigger offered by qmail-send. Here is the current triggering
mechanism: lock/trigger is a named pipe. Before scanning todo/,
qmail-send opens lock/trigger O_NDELAY for reading. It then selects for
readability on lock/trigger. qmail-queue pulls the trigger by writing a
byte O_NDELAY to lock/trigger. This makes lock/trigger readable and
wakes up qmail-send. Before scanning todo/ again, qmail-send closes and
reopens lock/trigger.
