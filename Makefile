VERSION = 0.1

include config.mk
SRC = devd-notifier.c
OBJ = ${SRC:.c=.o}

all: options devd-notifier

options:
	@echo build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC		   = ${CC}"

.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

devd-notifier: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

dist: clean
	@echo creating dist tarball
	@mkdir -p devd-notifier-${VERSION}
	@cp -R LICENSE Makefile config.mk config.h README \
				 devd-notifier.1 ${SRC}	devd-notifier.h \
				 devd-notifier-${VERSION}
	@tar czf devd-notifier-${VERSION}.tar.gz devd-notifier-${VERSION}
	@rm -rf devd-notifier-${VERSION}

man: devd-notifier.1.in
	gzip devd-notifier.1.gz

clean:
	@echo cleaning
	@rm -f devd-notifier ${OBJ} devd-notifier.1.gz devd-notifier-${VERSION}.tar.gz

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@chmod 0755 ${DESTDIR}${PREFIX}/bin
	@install -m 0755 devd-notifier ${DESTDIR}${PREFIX}/bin
	@echo install manual page to ${DESTDIR}${MANPREFIX}/man1/devd-notifier.1.gz
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@chmod 0755 ${DESTDIR}${MANPREFIX}/man1
	@install -m 0644 devd-notifier.1.gz ${DESTDIR}${MANPREFIX}/man1/

uninstall:
	@echo removing system executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/devd-notifier
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/devd-notifier.1

.PHONY: all options clean install uninstall
