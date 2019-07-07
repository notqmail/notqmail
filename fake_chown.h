#ifndef FAKE_CHOWN_H
#define FAKE_CHOWN_H

/*
 * Included only by install.c, which we don't want setting permissions
 * Permissions will be set by ./install-destdir
 */

int chown(const char *p, unsigned int o, unsigned int g) { return 0; }

#endif
