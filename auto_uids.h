#ifndef AUTO_UIDS_H
#define AUTO_UIDS_H

#define ID_OWNER   0
#define ID_ALIAS   1
#define ID_DAEMON  2
#define ID_LOG     3
#define ID_PASSWD  4
#define ID_QUEUE   5
#define ID_REMOTE  6
#define ID_SEND    7
#define ID_QMAIL   8
#define ID_NOFILES 9

#define auto_uido qmail_id_lookup(ID_OWNER)
#define auto_uida qmail_id_lookup(ID_ALIAS)
#define auto_uidd qmail_id_lookup(ID_DAEMON)
#define auto_uidl qmail_id_lookup(ID_LOG)
#define auto_uidp qmail_id_lookup(ID_PASSWD)
#define auto_uidq qmail_id_lookup(ID_QUEUE)
#define auto_uidr qmail_id_lookup(ID_REMOTE)
#define auto_uids qmail_id_lookup(ID_SEND)

#define auto_gidq qmail_id_lookup(ID_QMAIL)
#define auto_gidn qmail_id_lookup(ID_NOFILES)

extern int qmail_id_lookup();

#endif
