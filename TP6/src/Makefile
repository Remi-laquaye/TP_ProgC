CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -g
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lm

.PHONY: all clean

all: serveur client

serveur: serveur.o couleur.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

client: client.o bmp.o couleur.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f serveur client *.o pie_chart.svg
