ALL = unittest
CC = gcc
CFLAGS = -Wall -O2 -MMD
LDFLAGS =
SOURCES = $(wildcard ./*.c)
DEPENDS = $(SOURCES:.c=.d)

all: $(ALL)

unittest: unittest.o url_parser.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	-rm -f *.o
	-rm -f *.d
	-rm -f $(ALL)

-include $(DEPENDS)
