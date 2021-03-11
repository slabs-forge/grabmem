
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "grabcommon.h"

#define BUFFER_SIZE 128

long claim = 0;

size_t ps;

size_t nAlloc= 0;
char*  pAlloc= 0;

void usage() {
	fprintf(stderr,"usage: grabmem [mem quantity]\n");
}

void docleanup() {
	unlinkat(0,SOCKET_PATH,0);
}

void doallocate() {
	// Round page up
	size_t n = ((claim + ps - 1) & ~ (ps-1));

	if (pAlloc == 0 || nAlloc != n) {
		printf("Allocating %ld pages (%lu kB)\n",n / ps,n / 1024);
		pAlloc = realloc(pAlloc,n);
		
		printf("Initializing\n");
		if (pAlloc && (n > nAlloc)) {
			for (size_t i = nAlloc ; i < n ; i += ps) {
				pAlloc[i]=1;
			}
		}
		printf("Initializing done\n");

		nAlloc = n;
	}
}

/**
 * Server setup
 */
void dograb() {
	char buffer[BUFFER_SIZE];

	struct sockaddr_un sa;
	int cr;
	int ssd;
	int csd;

	long delta;

	ssd = socket(AF_UNIX,SOCK_STREAM,0);
	if (ssd == 0) {
		fprintf(stderr,"Can't create socket: %s\n",strerror(errno));
		exit(3);
	}

	memset(&sa,0,sizeof(sa));
	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path,SOCKET_PATH);

	cr = unlinkat(0,SOCKET_PATH,0);
	if (cr < 0 && errno != ENOENT) {
		fprintf(stderr,"Can't bind socket: %s\n",strerror(errno));
		exit(3);
	}

	cr = bind(ssd,(struct sockaddr*)&sa,sizeof(sa));
	if (cr <0) {
		fprintf(stderr,"Can't bind socket: %s\n",strerror(errno));
		exit(3);
	}
	atexit(docleanup);

	cr = listen(ssd,1);
	if (cr < 0) {
		fprintf(stderr,"Can't listen on socket: %s\n",strerror(errno));
		exit(3);
	}

	for (;;) {
		doallocate();

		csd = accept(ssd,0,0);

		ssize_t c = read(csd,buffer,BUFFER_SIZE-1);
		if (c > 0) {
			buffer[c]=0;

			cr = readValue(buffer,&delta);
			if (cr == 0) {
				claim = claim + delta;
				if (claim <0) claim = 0;
			}
		}

		close(csd);
	}

}

/**
 * Read Param & Env
 */
int main(int argc, char* argv[]) {
	int cr;

	const char *strval;

	ps = (size_t)  sysconf(_SC_PAGESIZE);

	if (argc == 2) {
		strval = argv[1];
	} else if (argc != 1) {
		usage();
		exit(1);
	} else {
		strval = getenv("GRABMEM");
		if (strval == 0) {
			strval = "0";
		}
        }

	cr = readValue(strval,&claim);	
	if (cr<0) {
		usage();
		exit(1);
	}

	if (claim < 0)  {
		fprintf(stderr,"initial claimed memory must be positive\n");
		exit(2);
	}

	dograb();
}
