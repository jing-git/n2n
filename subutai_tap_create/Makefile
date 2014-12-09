CC=gcc
CFLAGS=-Wall -g -O2

tap_create: subutai_tap_create.c libancillary.a
	$(CC) -o $@ $(CFLAGS) -L. subutai_tap_create.c -lancillary 
