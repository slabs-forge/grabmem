
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "grabcommon.h"

long delta = 0;

void usage() {
	fprintf(stderr,"usage: grabmore [memory increment]\n");
}

void docleanup() {
	unlinkat(0,SOCKET_PATH,0);
}

void dowrite(int sd,const char* buf) {
	ssize_t cnt = strlen(buf);

	for (;;) {
		ssize_t cr = write(sd,buf,cnt);
		if (cr <0) {
			fprintf(stderr,"Can't communicate with server: %s\n",strerror(errno));
			exit(3);
		}

		if (cr == cnt) break;

		buf += cr;
		cnt -= cr;
	}
}

/**
 * Server notification
 */
void donotify(long delta) {
	char buffer[128];

	struct sockaddr_un sa;
	int cr;
	int sd;

	sd = socket(AF_UNIX,SOCK_STREAM,0);
	if (sd == 0) {
		fprintf(stderr,"Can't create socket: %s\n",strerror(errno));
		exit(3);
	}

	memset(&sa,0,sizeof(sa));
	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path,SOCKET_PATH);

	cr = connect(sd,(struct sockaddr*)&sa,sizeof(sa));
	if (cr < 0) {
		fprintf(stderr,"Can't connect on socket: %s\n",strerror(errno));
		exit(3);
	}

	sprintf(buffer,"%ld",delta);

	dowrite(sd,buffer);

	close(sd);
}

/**
 * Read Param & Env
 */
int main(int argc, char* argv[]) {
	int cr;

	if (argc == 2) {
		cr = readValue(argv[1],&delta);	
		if (cr < 0) {
			usage();
			exit(1);
		}

	} else {  
		usage();
		exit(1);
	} 

	donotify(delta);
}
