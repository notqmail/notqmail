Before building and running the tests, install [Check](https://libcheck.github.io/check/)
(and [pkg-config](https://pkg-config.freedesktop.org/) or
[pkgconf](https://github.com/pkgconf/pkgconf) to find Check's includes and libs). These
are dependencies for tests and only for tests.

To build and run the tests from the top-level Makefile, simply run:

```shell
make test
```

Verified to work with recent BSD and GNU make.

To add more ```stralloc``` tests, copy the ```test_stralloc_thingy``` example in
```tests/unittest_stralloc.c```. To add other tests, copy the
```unittest_stralloc``` example in ```tests/Makefile```.
