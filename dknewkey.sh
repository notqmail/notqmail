#
# $Id: dknewkey.sh,v 1.17 2023-02-11 23:01:57+05:30 Cprogrammer Exp mbhangui $
#

usage()
{
	echo "Usage: dknewkey [options] keyfile"
	echo "options"
	echo "       [-p | --print]          : print DKIM public keys"
	echo "       [-r | --remove]         : remove DKIM keys"
	echo "       [-d | --domain domain]  : domain name"
	echo "       [-b | --bits   size]    : DKIM private key size"
	echo "       [-t | --type   type]    : Key type (RSA or ED25519)"
	echo "       [-e | --enforce]        : Key is not in testing"
	echo "       [-f]                    : force DKIM private key creation"
	exit $1
}

# split a long string into multiple strings
# of length 255 or less.
# Credits: J
# See https://notes.sagredo.eu/en/qmail-notes-185/configuring-dkim-for-qmail-92.html#comment2961
split_str()
{
	local INPUT="$1"
	local LEN=$(echo "$INPUT" | wc -c)
	local COUNT=$((LEN / 255))
	for i in $(seq 0 $COUNT)
	do
		LINE=$(echo "$INPUT" | cut -c $((i * 255 + 1))-$(((i + 1) * 255)))
		if [ $i -ne 0 ]; then
			printf " "
		fi
		printf "\"%s\"" "${LINE}"
	done
}

write_pub_key()
{
	local selector=$(basename $1)
	local domain=$2
	local pubkey=$3
	if [ $testing -eq 1 ] ; then
		t=" t=y;"
	else
		t=""
	fi
	if [ $bits -lt 2048 -o "$ktype" = "ed25519" ] ; then
		printf "%s._domainkey.%s. IN TXT (\"v=DKIM1; k=%s;%s p=%s\")\n" "$selector" "$domain" "$ktype" "$t" "$pubkey"
	else
		printf "%s._domainkey.%s. IN TXT (" "$selector" "$domain"
		split_str "v=DKIM1; k=$ktype;$t p=$pubkey"
		printf ")\n"
	fi
}

print_key()
{
	local domain=$1
	local selector=$2
	if [ -n "$selector" ] ; then
		if [ -f $selector ] ; then
			echo "DKIM Private key for $domain file $dir/$selector"
			cat $selector
			ls -l $dir/$selector
			echo "------------------------------------------------------"
		else
			echo "DKIM Private key for $domain does not exist in $dir/$selector"
			exit 2
		fi
		if [ -f $selector.pub ] ; then
			echo "DKIM TXT record for $domain with selector=$selector file $dir/$selector.pub"
			cat $selector.pub
			ls -l $dir/$selector.pub
			echo "------------------------------------------------------"
		else
			echo "DKIM TXT record for $domain with selector=$selector does not exist in $dir/$selector.pub"
			exit 2
		fi
	else
		for i in $(find . -name '*'.pub -print)
		do
			selector=$(basename $i | cut -d. -f1)
			t=$(echo $i | cut -c3-)
			echo "DKIM TXT record for $domain with selector=$selector file $dir/$t"
			cat $i
			ls -l $dir/$t
			t=$(echo $t|sed 's{.pub{{')
			ls -l $dir/$t
			echo "------------------------------------------------------"
		done
		exit 0
	fi
}

controldir=@qsysconfdir@/control
SYSTEM=$(uname -s)
if [ "$SYSTEM" = "FreeBSD" ] ; then
	options=$(getopt eprfd:b:t: "$@")
else
	options=$(getopt -a -n dknewkey -o "eprfd:b:t:e" -l enforce,print,remove,force,domain:,bits:,type: -- "$@")
fi
if [ $? != 0 ]; then
	usage 100
fi
if [ $(id -u) -ne 0 ] ; then
	echo "dknewkey is not meant to be run by mere mortals. Use sudo to get superpowers"
	exit 100
fi

do_print=0
remove=0
testing=1
force=0
bits=2048
ktype="rsa"
domain=""
eval set -- "$options"
while :; do
	case "$1" in
	-f | --force)
	force=1
	shift 1
	;;
	-p | --print)
	do_print=1
	shift 1
	;;
	-r | --remove)
	remove=1
	shift 1
	;;
	-d | --domain)
	if [ -f $controldir/bouncehost ] ; then
		grep -w "$2" $controldir/rcpthosts $controldir/bouncehost >/dev/null 2>&1
	else
		grep -w "$2" $controldir/rcpthosts $controldir/me >/dev/null 2>&1
	fi
	if [ $? -ne 0 ] ; then
		echo "$domain not in rcpthosts or bouncehost/me" 1>&2
		exit 1
	fi
	domain="$2"
	shift 2
	;;
	-b | --bits)
	bits="$2"
	shift 2
	;;
	-t | --ktype)
	ktype=$(echo "$2" | tr '[:upper:]' '[:lower:]')
	if [ "$ktype" != "rsa" -a "$ktype" != "ed25519" ] ; then
		echo "Key type must be rsa or ed25519" 1>&2
		exit 1
	fi
	shift 2
	;;
	-e | --enforce)
	testing=0
	shift 1
	;;
	--) # end of options
	shift
	break
	;;
	*)
	echo "Unexpected option: $1 - this should not happen."
	usage 100
	;;
	esac
done
if [ $do_print -eq 0 -a $# -ne 1 ] ; then
	usage 100
else
	selector=$1
fi
if [ -z "$domain" ] ; then
	dir=$controldir/domainkeys
	if [ -f $controldir/defaultdomain ] ; then
		domain=$(cat $controldir/defaultdomain)
	else
		domain=$(echo $([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n) | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./')
	fi
else
	dir=$controldir/domainkeys/$domain
fi

if [ $do_print -eq 1 ] ; then
	cd $dir
	if [ $? -ne 0 ] ; then
		exit 2
	fi
	print_key "$domain" "$selector"
elif [ $remove -eq 1 ] ; then
	cd $dir
	if [ $? -ne 0 ] ; then
		exit 2
	fi
	echo "Removing DKIM Keys $selector, $selector.pub"
	/bin/rm -f $selector $selector.pub
	files=$(ls)
	cd ..
	if [ -d $domain -a -z "$files" ] ; then
		echo "Removing empty directory $domain"
		rmdir --ignore-fail-on-non-empty $dir
	fi
else
	t=$(dirname $selector)
	if [ "$t" != "$dir" -a "$t" != "." ] ; then
		echo "WARNING!!!. Generating DKIM keys outside $dir"
		dir=$t
		selector=$(basename $selector)
	fi
	if [ ! -d $dir ] ; then
		if (! mkdir -p $dir || ! chown root:qmail $dir || ! chmod 755 $dir) ; then
			exit 1
		fi
	fi
	cd $dir
	if [ $? -ne 0 ] ; then
		exit 2
	fi
	if [ -f $selector -a $force -eq 0 ] ; then
		echo "DKIM private key $selector exists. Skipping private key generation" 1>&2
		exit 1
	fi
	if [ -f $selector -a $force -eq 0 ] ; then
		echo "DKIM public  key $selector.pub exists. Skipping public  key generation" 1>&2
		exit 1
	fi
	err=$(mktemp -t dkkeyXXXXXXXXXX)
	if [ $? -ne 0 ] ; then
		echo "Unable to create temp files" 1>&2
		exit 1
	fi
	exec 3>&2 # save stderr in fd 3
	exec 2>$err
	exec 0<$err
	/bin/rm -f $err
	echo "Generating $ktype DKIM private key keysize=$bits, file $dir/$selector"
	if [ "$ktype" = "rsa" ] ; then
		/usr/bin/openssl genrsa -out $selector $bits
	else
		/usr/bin/openssl genpkey -algorithm ed25519 -out $selector
	fi
	if [ $? -ne 0 ] ; then
		/bin/cat
		exit 1
	fi

	if [ "$ktype" = "rsa" ] ; then
		echo "Generating $ktype DKIM public  key for $selector.domainkey.$domain, file $dir/$selector.pub, keysize=$bits"
		pubkey=$(/usr/bin/openssl  rsa -in $selector -pubout -outform PEM | grep -v '^--' | tr -d '\n')
	else
		echo "Generating $ktype DKIM public  key for $selector.domainkey.$domain, file $dir/$selector.pub"
		pubkey=$(/usr/bin/openssl pkey -pubout -in $selector | /usr/bin/openssl asn1parse -offset 12 -noout -out /dev/stdout | /usr/bin/openssl base64)
	fi
	if [ $? -ne 0 ] ; then
		/bin/cat
		exit 1
	fi
	write_pub_key "$selector" "$domain" "$pubkey" > $selector.pub
	if [ $? -ne 0 ] ; then
		/bin/cat
		exit 1
	fi

	exec 2>&3 # restore stderr
	if ( ! chown root:qmail $selector $selector.pub || ! chmod 640 $selector || ! chmod 644 $selector.pub) ; then
		exit 1
	fi
	print_key "$domain" "$selector"
fi
exit 0

#
# $Log: dknewkey.sh,v $
# Revision 1.17  2023-02-11 23:01:57+05:30  Cprogrammer
# generate ed25519 public key without ASN.1 structure (skip first 12 bytes)
#
# Revision 1.16  2023-02-05 22:53:15+05:30  Cprogrammer
# made key time argument case insenstive
# added -e, --enforce option to disable dkim key test mode
#
# Revision 1.15  2023-01-26 22:29:12+05:30  Cprogrammer
# added option to generate ed25519 DKIM keys
#
# Revision 1.14  2022-11-27 09:32:51+05:30  Cprogrammer
# list public, private key using ls
#
# Revision 1.13  2022-10-02 21:55:50+05:30  Cprogrammer
# refactored code
#
# Revision 1.12  2022-03-06 18:49:17+05:30  Cprogrammer
# fix for FreeBSD (getopt usage).
#
# Revision 1.11  2021-09-11 18:55:36+05:30  Cprogrammer
# changed owner/permissions of dkim private/public key pairs
#
# Revision 1.10  2021-08-24 11:29:21+05:30  Cprogrammer
# check if domain exists in rcpthosts
#
# Revision 1.9  2021-08-23 17:34:19+05:30  Cprogrammer
# fixed default_domain variable
#
# Revision 1.8  2021-08-21 21:33:14+05:30  Cprogrammer
# fixed syntax error
#
# Revision 1.7  2021-08-19 19:54:39+05:30  Cprogrammer
# added options to print, remove, generate DKIM keys
#
# Revision 1.6  2019-02-15 17:11:18+05:30  Cprogrammer
# fixed checking of argument count
#
# Revision 1.5  2017-03-09 16:38:24+05:30  Cprogrammer
# use full path of openssl
#
# Revision 1.4  2010-05-16 19:59:48+05:30  Cprogrammer
# fix for Mac OS X
#
# Revision 1.3  2004-11-02 20:48:31+05:30  Cprogrammer
# fixed error when dknewkey was called without arguments
#
# Revision 1.2  2004-10-21 21:54:25+05:30  Cprogrammer
# create public key file
#
# Revision 1.1  2004-10-20 20:40:56+05:30  Cprogrammer
# Initial revision
