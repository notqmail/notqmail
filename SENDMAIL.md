This document explains what you, as a user, will notice when the system
switches from sendmail to qmail.

This is a global document, part of the qmail package, not reflecting the
decisions made by your system administrator. For details on

   * which local delivery agent qmail is configured to use,
   * whether qmail is configured to use dot-forward,
   * whether ezmlm is installed,
   * whether fastforward is installed, and
   * all other local configuration features,

see your local sendmail-qmail upgrade announcement (which your system
administrator may have placed into /var/qmail/doc/ANNOUNCE).


--- Mailbox location

If your system administrator has configured qmail to use binmail for
local deliveries, your mailbox will be in /var/spool/mail/you, just as
it was under sendmail.

If your system administrator has configured qmail to use qmail-local for
local deliveries, your mailbox will be moved to ~you/Mailbox. There is a
symbolic link from /var/spool/mail/you to ~you/Mailbox, so your mail
reader will find the mailbox at its new location.


--- Loop control

qmail-local automatically adds a Delivered-To field at the top of every
delivered message. It uses Delivered-To to prevent mail forwarding
loops, including cross-host mailing-list loops.


--- Outgoing messages

qmail lets you use environment variables to control the appearance of
your outgoing mail, supplementing the features offered by your MUA. For
example, qmail-inject will set up Mail-Followup-To for you automatically
if you tell it which mailing lists you are subscribed to. See
qmail-inject(8) for a complete list of features.

If you're at (say) sun.ee.movie.edu, qmail lets you type joe@mac for
joe@mac.ee.movie.edu, and joe@mac+ for joe@mac.movie.edu without the ee.
sendmail has a different interpretation of hostnames without dots.


--- Forwarding and mailing lists

qmail gives you the power to set up your own mailing lists without
pestering your system administrator.

Under qmail, you are in charge of all addresses of the form
you-anything. The delivery of you-anything is controlled by
~you/.qmail-anything, a file in your home directory.

For example, if you want to set up a bug-of-the-month-club mailing list,
you can put a list of addresses into ~you/.qmail-botmc. Any mail to
you-botmc will be forwarded to all of those addresses. Mail directly to
you is controlled by ~you/.qmail. You can even set up a catch-all,
~you/.qmail-default, to handle unknown you- addresses.

See dot-qmail(5) for the complete story. Beware that the syntax of
.qmail is different from the syntax of sendmail's .forward file.

If your system administrator has configured qmail to use the dot-forward
compatibility tool, you can put forwarding addresses (and programs) into
.forward the same way you did with sendmail.

If your system administrator has installed ezmlm, you can use ezmlm-make
to instantly set up a professional-quality mailing list, handling
subscriptions and archives automatically.

If your system administrator has installed fastforward, you can easily
manage a large database of forwarding addresses.
