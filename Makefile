# Radix Tree Makefile
# Josh Allmann <joshua.allmann@gmail.com>

all: test test-ipv6

COMMON=radix.c
test: $(COMMON) test.c
	gcc -g -DLSB_FIRST radix.c test.c -o $@

test-ipv6: $(COMMON)  test-ipv6.c
	gcc -g -DLSB_FIRST radix.c test-ipv6.c -o $@

check: test-ipv6
	./test-ipv6
