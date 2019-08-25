subject="$1"
shift
( echo Subject: "$subject"
  echo To: ${1+"$@"}
  echo ''
  cat
) | QMAILBIN/qmail-inject
