#!/bin/sh

# Update temporary RSA and DH keys
# Frederik Vermeulen 2004-05-31 GPL

umask 0077 || exit 0

export PATH="$PATH:/usr/local/bin/ssl:/usr/sbin"

openssl genrsa -out QMAIL/control/rsa512.new 512 &&
chmod 600 QMAIL/control/rsa512.new &&
chown UGQMAILD QMAIL/control/rsa512.new &&
mv -f QMAIL/control/rsa512.new QMAIL/control/rsa512.pem
echo

openssl dhparam -2 -out QMAIL/control/dh512.new 512 &&
chmod 600 QMAIL/control/dh512.new &&
chown UGQMAILD QMAIL/control/dh512.new &&
mv -f QMAIL/control/dh512.new QMAIL/control/dh512.pem
echo

openssl dhparam -2 -out QMAIL/control/dh1024.new 1024 &&
chmod 600 QMAIL/control/dh1024.new &&
chown UGQMAILD QMAIL/control/dh1024.new &&
mv -f QMAIL/control/dh1024.new QMAIL/control/dh1024.pem
