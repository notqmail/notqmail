int main()
{
 unsigned int a = 0x80000000, b = 0x80000000, c;
 return !__builtin_add_overflow(a, b, &c) && !__builtin_mul_overflow(a, b, &c);
}
