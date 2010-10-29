# Radix Tree Makefile
# Josh Allmann <joshua.allmann@gmail.com>

default:
	gcc -g -DLSB_FIRST radix.c test.c
