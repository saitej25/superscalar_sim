/************************************************************************
* Copyright (C) 1989, 1990, 1991, 1992, 1993                    	*
*                   Rabin A. Sugumar and Santosh G. Abraham		*
*									*
* This software is distributed absolutely without warranty. You are	*
* free to use and modify the software as you wish.  You are also free	*
* to distribute the software as long as it is not for commercial gain,	*
* you retain the above copyright notice, and you make clear what your	*
* modifications were.							*
*									*
* Send comments and bug reports to rabin@eecs.umich.edu			*
*									*
************************************************************************/

/* Converts ASCII trace files into the binary format for the simulators */

#include <stdio.h>

main()

{int addr;

   while (fscanf(stdin, "%d", &addr) != EOF)
	  fwrite((char *) &addr, 4, 1, stdout);

   return (0);
}
