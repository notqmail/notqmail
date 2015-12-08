#!/bin/sh

# Update temporary RSA and DH keys
# Frederik Vermeulen 2004-05-31 GPL

umask 0077 || exit 0

export PATH="$PATH:/usr/local/bin/ssl:/usr/sbin"

openssl genrsa -out QMAIL/control/rsa2048.new 2048 &&
chmod 600 QMAIL/control/rsa2048.new &&
chown UGQMAILD QMAIL/control/rsa2048.new &&
mv -f QMAIL/control/rsa2048.new QMAIL/control/rsa2048.pem
echo

openssl dhparam -2 -out QMAIL/control/dh2048.new 2048 &&
chmod 600 QMAIL/control/dh2048.new &&
chown UGQMAILD QMAIL/control/dh2048.new &&
mv -f QMAIL/control/dh2048.new QMAIL/control/dh2048.pem
