CC=gcc
CFLAGS=-Wall -g -O2

tap_create: subutai_tap_create.c ../subutai_n2n/libancillary.a
	$(CC) -o $@ $(CFLAGS) -L../subutai_n2n subutai_tap_create.c -lancillary
