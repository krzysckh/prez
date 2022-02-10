CFLAGS=-Wall -Wextra -ggdb -std=c99

all: prez
clean:
	rm -f prez
test: all
	./prez -c prez.cfg test.prez
