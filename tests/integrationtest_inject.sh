#!/bin/sh

set -e

test_inject_qualification() {
	result=$(echo "To: djb" | ./testable/qmail-inject $@ 2>&1 || true)

	if [ "${result}" = "qmail-inject: fatal: BUG" ]; then
		echo "bug: '$@'"
		BUGCOUNT=$(expr 1 + $BUGCOUNT)
	elif [ "${result}" = "qmail-inject: fatal: OK" ]; then
		:
	else
		echo "unexpected result from ${result}"
		exit 77
	fi
}

main() {
	BUGCOUNT=0
	QMAILQUEUE=./inject-qualification-queue; export QMAILQUEUE

	echo "Running integration: notqmail qmail-inject qualification"

	test_inject_qualification "-a -- djb"
	test_inject_qualification "-H --"

	return $BUGCOUNT
}

main "$@"
exit $?
