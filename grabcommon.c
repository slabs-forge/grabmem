
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "grabcommon.h"

int readValue(const char* str,long* val) {
	char* end = 0;

	long q = strtol(str,&end,10);
	long m = 1;

	if (end == 0 || errno !=0 ) {
		return -1;
	} else if (*end != 0) {
		if (end[1] != 0) {
			return -1;
		}

		switch(end[0]) {
		case 'k':
		case 'K':
			m=1024;
			break;
		case 'M':
		case 'm':
			m=1024*1024;
			break;
		case 'G':
		case 'g':
			m=1024*1024*1024;
			break;
		default:
			return -1;
		}
	}

	*val = q * m;

	return 0;
}

