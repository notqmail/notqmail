fqdn="$1"
echo Your fully qualified host name is "$fqdn".

echo Putting "$fqdn" into control/me...
echo "$fqdn" > QMAIL/control/me
chmod 644 QMAIL/control/me

( echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' | (
  read ddom
  echo Putting "$ddom" into control/defaultdomain...
  echo "$ddom" > QMAIL/control/defaultdomain
  chmod 644 QMAIL/control/defaultdomain
) )

( echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' | (
  read pdom
  echo Putting "$pdom" into control/plusdomain...
  echo "$pdom" > QMAIL/control/plusdomain
  chmod 644 QMAIL/control/plusdomain
) )

echo Putting "$fqdn" into control/locals...
echo "$fqdn" >> QMAIL/control/locals
chmod 644 QMAIL/control/locals

echo Putting "$fqdn" into control/rcpthosts...
echo "$fqdn" >> QMAIL/control/rcpthosts
chmod 644 QMAIL/control/rcpthosts
echo "Now qmail will refuse to accept SMTP messages except to $fqdn."
echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
