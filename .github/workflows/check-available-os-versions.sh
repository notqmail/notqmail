#!/bin/sh

set -e

SURPRISE_COUNT=0
[ -z "${HTML2TEXT}" ] && HTML2TEXT=html2text

fetch_webpage_as_markdown() {
	curl -sS "$1" | ${HTML2TEXT}
}

fetch_alpine_upstream_latest_stable() {
	fetch_webpage_as_markdown 'https://alpinelinux.org/downloads/' \
		| grep 'Current Alpine Version' \
		| awk '{print $4}' \
		| sed -e 's|\*||g'
}

fetch_github_runner_list() {
	fetch_webpage_as_markdown 'https://github.com/actions/runner-images' \
		| perl -ne 'next unless /YAML Label/; print <>;' \
		| perl -ne 'next if /^---/; print;' \
		| perl -ne 'last if /Label scheme/; print;' \
		| grep -v ' beta ' \
		| grep -v ' deprecated ' \
		| grep -v 'windows-' \
		| cut -d'|' -f2 \
		| sed -e 's| or |\n|g' -e 's|, |\n|g' -e 's|^ ||' -e 's|`||g' -e 's| $||' \
		| grep -v -- '-latest' \
		| grep -v -- '-large' \
		| grep -v -- '-xlarge' \
		| grep -v '^\s*$'
}

fetch_github_runner_os_versions() {
	osname="$1"
	osname_lower="$(echo ${osname} | tr '[:upper:]' '[:lower:]')"
	fetch_github_runner_list \
		| grep "^${osname_lower}-" \
		| sed -e "s|^${osname_lower}-||" \
		| tr '\n' ' ' \
		| sed -e 's| $||'

}

fetch_vmactions_os_versions() {
	osname="$1"
	osname_lower="$(echo ${osname} | tr '[:upper:]' '[:lower:]')"
	fetch_webpage_as_markdown "https://github.com/vmactions/${osname_lower}-vm" \
		| grep 'All the supported' \
		| sed -e "s|.*${osname} ||i" -e 's|, test\.releases.*||g' -e 's|,||g' \
		| tr ' ' '\n' \
		| sort -rn \
		| tr '\n' ' ' \
		| sed -e 's| $||'
}

fetch_os_versions() {
	osname="$1"
	case ${osname} in
	Alpine)
		fetch_alpine_upstream_latest_stable
		;;
	Fedora)
		echo rawhide
		;;
	*BSD|Solaris)
		fetch_vmactions_os_versions $1
		;;
	macOS|Ubuntu)
		fetch_github_runner_os_versions $1
		;;
	*)
		echo unknown
	esac
}

check_os_versions() {
	osname="$1"; shift
	known="$@"
	available="$(fetch_os_versions ${osname})"
	[ "${known}" = "${available}" ] || {
		SURPRISE_COUNT=$(expr 1 + ${SURPRISE_COUNT})
		echo "==> known to us: ${known}"
		echo "==> now existing: ${available}"
	}
}

get_workflow_platforms() {
	fullpath="$(dirname "$0")/platform-builders.yml"
	cat "${fullpath}" | yq -r '.jobs.[].name' | grep -v 'OS version check' | grep -v '^$' | awk '{print $1}'
}

get_workflow_platform_versions() {
	osname="$1"; shift
	osname_lower="$(echo ${osname} | tr '[:upper:]' '[:lower:]')"
	fullpath="$(dirname "$0")/platform-builders.yml"

	case ${osname} in
	Fedora)
		# UUOC
		cat "${fullpath}" \
			| yq -r ".jobs.${osname_lower}.container" \
			| cut -d':' -f2 \
			| tr '\n' ' ' \
			| sed -e 's| $||'
		;;
	*)
		cat "${fullpath}" \
			| yq -r ".jobs.${osname_lower}.strategy.matrix.osversion.[]" \
			| tr '\n' ' ' \
			| sed -e 's| $||'
	esac
}

get_known_platform_versions() {
	osname="$1"; shift
	case ${osname} in
	Alpine) echo 3.20.0 ;;
	DragonFlyBSD) echo 6.4.0 ;;
	Fedora) echo rawhide ;;
	FreeBSD) echo 14.0 13.3 13.2 12.4 ;;
	macOS) echo 14 13 12 ;;
	NetBSD) echo 10.0 9.4 9.3 9.2 9.1 9.0 ;;
	OpenBSD) echo 7.5 7.4 7.3 7.2 ;;
	Solaris) echo 11.4-gcc 11.4 ;;
	Ubuntu) echo 22.04 20.04 ;;
	*) echo unknown ;;
	esac
}


main() {
	for i in $(get_workflow_platforms); do
		echo "$i (we build for $(get_workflow_platform_versions $i))"
		check_os_versions $i $(get_known_platform_versions $i)
	done

	return ${SURPRISE_COUNT}
}

main "$@"
exit $?
