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

man: devd-notifier.1.in
	@sed -e "s/VERSION/${VERSION}/g" < devd-notifier.1.in | gzip > devd-notifier.1.gz

clean:
	@echo cleaning
	@rm -f devd-notifier ${OBJ} devd-notifier.1.gz

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

