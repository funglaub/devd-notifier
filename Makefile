CC = gcc -Wall -pedantic -std=c99  `pkg-config --cflags --libs libnotify`
PREFIX = /usr/local

all: devd-notifier

devd-notifier:
	${CC} -o devd-notifier devd-notifier.c

install: devd-notifier
	/usr/bin/install  -g operator -m 0755 -o root devd-notifier ${PREFIX}/bin

clean: 
	rm -f devd-notifier

.PHONY: all install clean
