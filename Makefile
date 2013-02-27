CC=gcc
CFLAGS=-Wall -g -DRAM_DICT -DPC_SIM -lm
LDFLAGS= -g -lm
# CFLAGS=-Wall -O2 -DRAM_DICT -DPC_SIM -lm
# LDFLAGS= -O2 -lm

SRCS=	uforth.c  uforth-ext.c  utils.c 
HDRS= uforth.h uforth-ext.h
OBJS= $(SRCS:.c=.o)

TARGET=uforth-linux

uforth: $(OBJS) $(TARGET).o  ext.f
	gcc $(CFLAGS) -o uforth $(OBJS) $(TARGET).o -lreadline -lm
	echo "save-image uforth.img" | ./uforth 

ext.f: uforth-ext.h
	awk -f make_ext_words.awk uforth-ext.h > ext.f

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

clean:
	-rm -f *.o *.exe *~ *.stackdump *.aft-TOC uforth
