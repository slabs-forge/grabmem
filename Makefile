
CC=gcc
LDFLAGS=-static

all: grabmem grabmore Dockerfile
	docker build -t sebetln/grabmem .

grabmem: grabmem.o grabcommon.o
	$(CC) $(LDFLAGS) $+ -o $@

grabmore: grabmore.o grabcommon.o
	$(CC) $(LDFLAGS) $+ -o $@


grabmem.o: grabmem.c grabcommon.h
	$(CC) -c $< -o $@

grabmore.o: grabmore.c grabcommon.h
	$(CC) -c $< -o $@

grabcommon.o: grabcommon.c grabcommon.h
	$(CC) -c $< -o $@
