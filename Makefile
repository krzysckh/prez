CFLAGS=-Wall -Wextra -std=c99
PREFIX=/usr

.SUFFIXES: .1 .1.gz

.1.1.gz: prez.1
	gzip -k $<
all: prez prez.1.gz
clean:
	rm -f prez prez.1.gz
install: all
	cp prez $(PREFIX)/bin/
	cp prez.1.gz $(PREFIX)/share/man/man1/
uninstall:
	rm -rf $(PREFIX)/bin/prez
	rm -rf $(PREFIX)/share/man/man1/prez.1.gz
