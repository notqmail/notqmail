SHELL=/bin/sh

# Files are edited in the installation directory, then copied.
# There are 40 arguments to idedit after the filename,
# showing the positions of each byte in the following ten ints:
# uida, uidd, uidl, uido, uidp, uidq, uidr, uids, gidq, gidn.
# Normal little-endian positions are n n+1 n+2 ... n+39 for some n.
# Normal big-endian positions are n+3 n+2 n+1 n n+7 ... n+36 for some n.

setup:
	mkdir /var/qmail
	./idedit install-big XXX
	./idedit qmail-lspawn XXX
	./idedit qmail-queue XXX
	./idedit qmail-rspawn XXX
	./idedit qmail-showctl XXX
	./idedit qmail-start XXX
	./install-big
	cp /var/qmail/boot/binm1+df /var/qmail/rc
	chmod 755 /var/qmail/rc
	echo '|fastforward -d /etc/aliases.cdb' > /var/qmail/alias/.qmail-default
	chmod 644 /var/qmail/alias/.qmail-default
	hostname | grep -q '\.'
	./config-fast `hostname`
