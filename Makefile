CFLAGS=-Wall -Wextra -std=c99
PREFIX=/usr

%.gz: %
	gzip -k $<
all: prez prez.1.gz
clean:
	rm -f prez prez.1.gz
install: all
	install -Dm755 -s prez -t $(PREFIX)/bin/
	install -Dm644 prez.1.gz -t $(PREFIX)/share/man/man1/
uninstall:
	rm -rf $(PREFIX)/bin/prez
	rm -rf $(PREFIX)/share/man/man1/prez.1.gz
