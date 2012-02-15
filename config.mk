VERSION = 0.1

# Customize below to fit your system

# paths

PREFIX=/usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = `pkg-config --cflags libnotify`
LIBS = `pkg-config --libs libnotify`	

CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS = -g ${LIBS} 

CC = cc
