This file points out some reasons that you might want to switch from
mbox format to a new format, maildir.


1. The trouble with mbox

The mbox format---the format of ~user/Mailbox, understood by BSD Mail
and lots of other MUAs---is inherently unreliable.

Think about it: what happens if the system crashes while a program is
appending a new message to ~user/Mailbox? The message will be truncated.
Even worse, if it was truncated in the middle of a line, it will end up
being merged with the next message! Sure, the mailer understands that it
wasn't successful, so it'll try delivering the message again later, but
it can't fix your corrupted mbox.

Other formats, such as mh folders, are just as unreliable.

qmail supports maildir, a crashproof format for incoming mail messages.
maildir is fast and easy for MUAs to use. Even better, maildir works
wonders over NFS---see below.

I don't want to cram maildir down people's throats, so it's not the
default. Nevertheless, I encourage you to start asking for maildir
versions of your favorite MUAs, and to switch over to maildir as soon as
you can.


2. Sun's Network F_ail_u_re System

Anyone who tells you that mail can be safely delivered in mbox format
over NFS is pulling your leg---as explained above, mbox format is
inherently unreliable even on a single machine.

Anyway, NFS is the most unreliable computing environment ever invented,
and qmail doesn't even pretend to support mbox over NFS.

You should switch to maildir, which works fine over NFS without any
locking. You can safely read your mail over NFS if it's in maildir
format. Any number of machines can deliver mail to you at the same time.
(On the other hand, for efficiency, it's better to get NFS out of the
picture---your mail should be delivered on the server that contains your
home directory.)

Here's how to set up qmail to use maildir for your incoming mail:

   % maildirmake $HOME/Maildir
   % echo ./Maildir/ > ~/.qmail

Make sure you include the trailing slash on Maildir/.

The system administrator can set up Maildir as the default for everybody
by creating a maildir in the new-user template directory and replacing
./Mailbox with ./Maildir/ in /var/qmail/rc.

Until your MUA supports maildir, you'll probably want to convert maildir
format to (gaaack) mbox format. I've supplied a maildir2mbox utility
that does the trick, along with some tiny qail and elq and pinq wrappers
that call maildir2mbox before calling Mail or elm or pine.
