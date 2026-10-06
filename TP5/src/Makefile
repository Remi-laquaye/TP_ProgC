CC ?= gcc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -g

.PHONY: all clean

all: serveur client repertoire

serveur: serveur.o
	$(CC) $(LDFLAGS) -o $@ $^

client: client.o
	$(CC) $(LDFLAGS) -o $@ $^

repertoire: repertoire.o
	$(CC) $(LDFLAGS) -o $@ $^

client.o: client.c client.h
serveur.o: serveur.c serveur.h
repertoire.o: repertoire.c repertoire.h

clean:
	rm -f serveur client repertoire client.o serveur.o repertoire.o
