qmail lets each user control all addresses of the form user-anything.
Addresses that don't start with a username are controlled by a special
user, alias. Delivery instructions for foo go into ~alias/.qmail-foo;
delivery instructions for user-foo go into ~user/.qmail-foo. See
dot-qmail.0 for the full story.

qmail doesn't have any built-in support for /etc/aliases. If you have a
big /etc/aliases and you'd like to keep it, install the fastforward
package, available separately. /etc/aliases should already include the
aliases discussed below---Postmaster, MAILER-DAEMON, and root.

If you don't have a big /etc/aliases, you'll find it easier to use
qmail's native alias mechanism. Here's a checklist of aliases you should
set up right now.

* Postmaster. You're not an Internet citizen if this address doesn't
work. Simply touch (and chmod 644) ~alias/.qmail-postmaster; any mail
for Postmaster will be delivered to ~alias/Mailbox.

* MAILER-DAEMON. Not required, but users sometimes respond to bounce
messages. Touch (and chmod 644) ~alias/.qmail-mailer-daemon.

* root. Under qmail, root never receives mail. Your system may generate
mail messages to root every night; if you don't have an alias for root,
those messages will bounce. (They'll end up double-bouncing to the
postmaster.) Set up an alias for root in ~alias/.qmail-root. .qmail
files are similar to .forward files, but beware that they are strictly
line-oriented---see dot-qmail.0 for details.

* Other non-user accounts. Under qmail, non-user accounts don't get
mail; ``user'' means a non-root account that owns ~account. Set up
aliases for any non-user accounts that normally receive mail.

Note that special accounts such as ftp, www, and uucp should always have
home directories owned by root.

* Default. If you want, you can touch ~alias/.qmail-default to catch
everything else. Beware: this will also catch typos and other addresses
that should probably be bounced instead. It won't catch addresses that
start with a user name---the user can set up his own ~/.qmail-default.
