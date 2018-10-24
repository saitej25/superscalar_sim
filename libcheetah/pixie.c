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

/* Preprocessor to convert trace in pixie format to foramt *
 * accepted by simulator.                                  */


#include <stdio.h>
#include "trace.h"

#define HASHNO 7211
#define MAX_BKTS 32
#define IN_BUFFER_SZ 1000
#define OUT_BUFFER_SZ IN_BUFFER_SZ*4


extern int trace_type;

extern FILE *in;



pixie (ret_buffer)
unsigned **ret_buffer;

{  struct ref_struct addr_str;
   static unsigned cur_bb;
   static int bb_i = -1;
   static unsigned buffer_out [1024];
   struct ref_struct buffer_in[64];   /* Max pixie compression ratio is 16 */
   int nr, l, indx;


   indx = 0;
   *ret_buffer = buffer_out;

 loop:
   nr = fread((struct ref_struct *) buffer_in, 4, 64, in);

   for (l=0; l < nr; l++) {
     addr_str = *(buffer_in +l);
     bb_i = addr_str.cnt;
     if (addr_str.type == BASIC_BLOCK) {
       --bb_i;
       cur_bb = addr_str.addr << 2;
       while (bb_i >= 0) {
	 if (trace_type & 0x1) {
	   buffer_out [indx] = cur_bb;
	   indx++;
	 }
	 cur_bb += 4;
	 --bb_i;
       }
     }
     else {
       if (trace_type & 0x1) {
	 buffer_out [indx] = cur_bb;
	 indx++;
       }
       if (IsRead (addr_str.type)) {
	 if (trace_type & 0x2) {
	   buffer_out[indx] = addr_str.addr;
	   indx++;
	 }
       }
       else {
	 if (trace_type & 0x4) {
	   buffer_out[indx] = addr_str.addr;
	   indx++;
	 }
       }
       cur_bb += 4;
       --bb_i;
       while (bb_i >= 0) {
	 if (trace_type & 0x1) {
	   buffer_out [indx] = cur_bb;
	   indx++;
	 }
	 cur_bb += 4;
	 --bb_i;
       }
     }
   } /*for*/
   if ((indx == 0) && (nr > 0))
     goto loop;
   return (indx);
 }









