#ifndef SNAKE_COMPAT_H
#define SNAKE_COMPAT_H

/* Compatibility header */

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#include <unistd.h>		/* getuid, stat, getopt...   */
#else
int usleep (unsigned long);
#endif


#endif
