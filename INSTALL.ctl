As you've seen, qmail has essentially no pre-compilation configuration.
You should never have to recompile it unless you want to change the
qmail home directory, usernames, or uids.

qmail does allow quite a bit of easy post-installation configuration. If
you care how your machine greets other machines via SMTP, for example,
you can put an appropriate line into /var/qmail/control/smtpgreeting.

But this is all optional---if control/smtpgreeting doesn't exist, qmail
will do something reasonable by default. You shouldn't worry much about
configuration right now. You can always come back and tune things later.

There's one big exception. You MUST tell qmail your hostname. Just run
the config-fast script:

   # ./config-fast your.full.host.name

config-fast puts your.full.host.name into control/me. It also puts it
into control/locals and control/rcpthosts, so that qmail will accept
mail for your.full.host.name.

You can instead use the config script, which looks up your host name in
DNS:

   # ./config

config also looks up your local IP addresses in DNS to decide which
hosts to accept mail for.

(Why doesn't qmail do these lookups on the fly? This was a deliberate
design decision. qmail does all its local functions---header rewriting,
checking if a recipient is local, etc.---without talking to the network.
The point is that qmail can continue accepting and delivering local mail
even if your network connection goes down.)

Next, read through FAQ for information on setting up optional features
like masquerading. If you really want to learn right now what all the
configuration possibilities are, see qmail-control.0.
