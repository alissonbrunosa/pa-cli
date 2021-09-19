CC = gcc
CFLAGS = -I. -Wall

LIBS = -lpulse -lm

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

MAIN = pa-cli

.PHONY: clean

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LFLAGS) $(LIBS)

clean:
	$(RM) *.o *~ $(MAIN)

install: $(MAIN)
	install -Dm755 pa-cli $(DESTDIR)/usr/bin/pa-cli

uninstall:
	$(RM) $(DESTDIR)/usr/bin/pa-cli
