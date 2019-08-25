fqdn="$1"
echo Your fully qualified host name is "$fqdn".

echo Putting "$fqdn" into control/me...
echo "$fqdn" > QMAILCONTROL/me
chmod 644 QMAILCONTROL/me

( echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' | (
  read ddom
  echo Putting "$ddom" into control/defaultdomain...
  echo "$ddom" > QMAILCONTROL/defaultdomain
  chmod 644 QMAILCONTROL/defaultdomain
) )

( echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' | (
  read pdom
  echo Putting "$pdom" into control/plusdomain...
  echo "$pdom" > QMAILCONTROL/plusdomain
  chmod 644 QMAILCONTROL/plusdomain
) )

echo Putting "$fqdn" into control/locals...
echo "$fqdn" >> QMAILCONTROL/locals
chmod 644 QMAILCONTROL/locals

echo Putting "$fqdn" into control/rcpthosts...
echo "$fqdn" >> QMAILCONTROL/rcpthosts
chmod 644 QMAILCONTROL/rcpthosts
echo "Now qmail will refuse to accept SMTP messages except to $fqdn."
echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
