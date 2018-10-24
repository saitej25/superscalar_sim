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

/* Preprocessor to convert traces in DIN format to format accepted      *
 * by simulator.  (Not very efficient)                                  */

#include <stdio.h>

extern int trace_type;
extern FILE *in;

din (ret_buffer)
unsigned **ret_buffer;

{  int l, indx, type;
   unsigned addr;
   char s[256], *status;
   static unsigned buffer [1000];

   indx = 0;
   *ret_buffer = buffer;
   for (l=0; l<1000; l++) {
     if (status = fgets (s, 256, in)) {
       sscanf (s, "%d%x", &type, &addr);
       switch (type) {
       case 0:
	 if (trace_type & 0x2) {
	   buffer[indx] = addr;
	   indx++;
	 }
	 break;
       case 1:
	 if (trace_type & 0x4) {
	   buffer[indx] = addr;
	   indx++;
	 }
	 break;
       case 2:
	 if (trace_type & 0x1) {
	   buffer[indx] = addr;
	   indx++;
	 }
	 break;
       case 3:
       case 4:
	 break;
       default:
	 fprintf (stderr, "Unknown access type %d\n", type);
       }
     }
     else {
       break;
     }
   }
   return (indx);
 }
