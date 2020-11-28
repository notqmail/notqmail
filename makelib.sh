main=$1; shift
rm -f "$main"
$AR cr "$main" ${1:+"$@"}
$RANLIB "$main"
