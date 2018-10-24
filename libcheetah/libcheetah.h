
#ifndef LIBCHEETAH_H
#define LIBCHEETAH_H

#include <stdio.h>

/* initialize libcheetah */
void
cheetah_init(int argc, char **argv);	/* cheetah arguments */

/* output the analysis configuration */
void
cheetah_config(FILE *fd);		/* output stream */

/* pass a memory address to libcheetah */
void
cheetah_access(unsigned long addr);	/* address of access */

/* output the trace statistics */
void
cheetah_stats(FILE *fd,			/* output stream */
	      int mid);			/* intermediate stats? */

/* set-associative statistics
     - ok to access after calling cheetah_stats()
     - arranged as:
	 sac_hits[set size: min-sets..max-sets][assoc: min-depth..max-depth]
*/
extern unsigned **sac_hits;

#endif /* LIBCHEETAH_H */
