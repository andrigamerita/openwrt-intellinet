include ../../../make.def

CFLAGS = $(MODE) $(MODEL)

all:
	/opt/buildroot-gdb/bin/mipsel-linux-uclibc-gcc $(CFLAGS) ozerocdoff.c -o ozerocdoff -I../../libusb-0.1.12 ../../libusb-0.1.12/.libs/libusb.a
