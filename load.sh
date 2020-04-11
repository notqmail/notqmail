main="$1"; shift
exec $LD -o "$main" "$main".o ${1:+"$@"}
