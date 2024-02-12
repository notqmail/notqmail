/*
 * $Log: socket_v4mappedprefix.c,v $
 * Revision 1.1  2005-06-15 22:12:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
unsigned char V4mappedprefix[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
#endif
