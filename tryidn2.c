#ifdef SMTPUTF8
#include <idn2.h>
#endif
int
main(int argc, char **argv)
{
#ifdef SMTPUTF8
	char           *ascii;
	idn2_lookup_u8((const uint8_t *) argv[1], (uint8_t **) &ascii, IDN2_NFC_INPUT);
#else
	:
#endif
}
