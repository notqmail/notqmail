echo 'main="$1"; shift'
echo 'rm -f "$main"'
echo 'ar cr "$main" ${1+"$@"}'
echo 'ranlib "$main"'
