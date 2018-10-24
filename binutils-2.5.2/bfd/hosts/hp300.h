#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <sys/file.h>
#ifndef	O_ACCMODE
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif
#define SEEK_SET 0
#define SEEK_CUR 1

#define HOST_HP300HPUX

#define USE_UTIME

/* Some things that need to be defined in order to make code written for
   BSD Unix compile under System V Unix.  */

#include <memory.h>
#define bcmp(b1,b2,len)		memcmp(b1,b2,len)
#define bcopy(src,dst,len)	memcpy(dst,src,len)
#define bzero(s,n)		memset(s,0,n)

#if 0
static int
rename(from, to)
{
  unlink(to);
  return(link(from, to));
}
#endif

#include "fopen-same.h"