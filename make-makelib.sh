echo 'main="$1"; shift'
echo 'rm -f "$main"'
echo "$AR" cr '"$main" ${1+"$@"}'
echo "$RANLIB" '"$main"'
