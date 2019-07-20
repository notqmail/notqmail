int
main(int argc, char **argv)
{
	unsigned int i;

	__builtin_add_overflow(1, 2, &i);
}
