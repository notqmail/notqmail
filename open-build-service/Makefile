all: notqmail.spec notqmail.dsc debian.tar.gz _service \
notqmail.changes

_service: conf-version conf-release
	sed -e s}@version\@}"`head -1 conf-version`"}g \
	-e s}@release\@}"`head -1 conf-release`"}g \
	_service.in > _service

notqmail.spec: notqmail.spec.in ../CHANGES.md conf-version conf-release conf-email
	(sed -e s}@version\@}"`head -1 conf-version`"}g \
	-e s}@release\@}"`head -1 conf-release`"}g \
	-e s}@email\@}"`head -1 conf-email`"}g \
	notqmail.spec.in;./catChangeLog) > notqmail.spec

notqmail.dsc: notqmail.dsc.in conf-version conf-release conf-email
	sed -e s}@version\@}"`head -1 conf-version`"}g \
	-e s}@release\@}"`head -1 conf-release`"}g \
	-e s}@email\@}"`head -1 conf-email`"}g \
	notqmail.dsc.in > notqmail.dsc

notqmail.changes: ../CHANGES.md conf-email
	./catChangeLog --changes > notqmail.changes

debian.tar.gz: debian/changelog debian/compat debian/control \
debian/copyright debian/docs debian/notqmail.install \
debian/postinst debian/postrm debian/preinst \
debian/rules debian/source ../CHANGES.md
	tar cf - debian | gzip -c > debian.tar.gz

debian/changelog: ../CHANGES.md conf-version conf-release conf-email
	./catChangeLog --debian --name=notqmail --state=stable --urgency=low > debian/changelog
