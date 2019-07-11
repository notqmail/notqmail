./hostname | tr '[A-Z]' '[a-z]' | (
  if read host
  then
    echo Your hostname is "$host".
    ./dnsfq "$host" | tr '[A-Z]' '[a-z]' | (
      if read fqdn
      then
        echo Your host\'s fully qualified name in DNS is "$fqdn".
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
	echo ' '
	echo Checking local IP addresses:
	: > QMAILCONTROL/locals
	chmod 644 QMAILCONTROL/locals
	( ./dnsip "$fqdn"
	  ./ipmeprint ) | sort -u | \
	(
	  while read localip
	  do
	    echo "$localip: " | tr -d '\012'
	    ./dnsptr "$localip" 2>/dev/null | (
	      if read local
	      then
		echo Adding "$local" to control/locals...
		echo "$local" >> QMAILCONTROL/locals
	      else
		echo PTR lookup failed. I assume this address has no DNS name.
	      fi
	    )
	  done
	)
	echo ' '
	echo If there are any other domain names that point to you,
	echo you will have to add them to QMAILCONTROL/locals.
	echo You don\'t have to worry about aliases, i.e., domains with CNAME records.
	echo ' '
	echo Copying QMAILCONTROL/locals to QMAILCONTROL/rcpthosts...
	cp QMAILCONTROL/locals QMAILCONTROL/rcpthosts
	chmod 644 QMAILCONTROL/rcpthosts
	echo 'Now qmail will refuse to accept SMTP messages except to those hosts.'
	echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
      else
        echo Sorry, I couldn\'t find your host\'s canonical name in DNS.
        echo You will have to set up control/me yourself.
      fi
    )
  else
    echo Sorry, I couldn\'t find your hostname.
    echo You will have to set up control/me yourself.
  fi
)
