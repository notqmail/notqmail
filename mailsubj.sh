subject="$1"
shift
( echo Subject: "$subject"
  echo To: ${1+"$@"}
  echo ''
  cat
) | QMAIL/bin/qmail-inject
