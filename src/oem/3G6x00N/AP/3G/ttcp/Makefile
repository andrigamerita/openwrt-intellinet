include ../../../make.def

SRCS=epi_ttcp.c
OBJS=$(SRCS:%.c=%.o)
#CC=gcc
CFLAGS=-fsigned-char -DEMBEDDED -DNO_CGI
LDFLAGS=-Wl
LD=$(CC)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

ttcp: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(OBJS) ttcp

