#ifndef FAKE_UIDS_H
#define FAKE_UIDS_H

/*
 * Included only by hier.c, which is linked only into ./install
 * These values don't matter there, as it doesn't try to set permissions
 * Permissions are set by ./install-destdir (linked with hier_destdir.c)
 */

#define auto_uido -79
#define auto_uida -78
#define auto_uidd -77
#define auto_uidl -76
#define auto_uidp -75
#define auto_uidq -74
#define auto_uidr -73
#define auto_uids -72

#define auto_gidq -71
#define auto_gidn -70

#endif
