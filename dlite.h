/*
 * dlite.h - DLite, the lite debugger, interfaces
 *
 * This file is a part of the SimpleScalar tool suite written by
 * Todd M. Austin as a part of the Multiscalar Research Project.
 *  
 * The tool suite is currently maintained by Doug Burger and Todd M. Austin.
 * 
 * Copyright (C) 1994, 1995, 1996, 1997 by Todd M. Austin
 *
 * This source file is distributed "as is" in the hope that it will be
 * useful.  The tool set comes with no warranty, and no author or
 * distributor accepts any responsibility for the consequences of its
 * use. 
 * 
 * Everyone is granted permission to copy, modify and redistribute
 * this tool set under the following conditions:
 * 
 *    This source code is distributed for non-commercial use only. 
 *    Please contact the maintainer for restrictions applying to 
 *    commercial use.
 *
 *    Permission is granted to anyone to make or distribute copies
 *    of this source code, either as received or modified, in any
 *    medium, provided that all copyright notices, permission and
 *    nonwarranty notices are preserved, and that the distributor
 *    grants the recipient permission for further redistribution as
 *    permitted by this document.
 *
 *    Permission is granted to distribute this file in compiled
 *    or executable form under the same conditions that apply for
 *    source code, provided that either:
 *
 *    A. it is accompanied by the corresponding machine-readable
 *       source code,
 *    B. it is accompanied by a written offer, with no time limit,
 *       to give anyone a machine-readable copy of the corresponding
 *       source code in return for reimbursement of the cost of
 *       distribution.  This written offer must permit verbatim
 *       duplication by anyone, or
 *    C. it is distributed by someone who received only the
 *       executable form, and is accompanied by a copy of the
 *       written offer of source code that they received concurrently.
 *
 * In other words, you are welcome to use, share and improve this
 * source file.  You are forbidden to forbid anyone else to use, share
 * and improve what you give them.
 *
 * INTERNET: dburger@cs.wisc.edu
 * US Mail:  1210 W. Dayton Street, Madison, WI 53706
 *
 * $Id: dlite.h,v 1.1 1997/03/11 01:30:26 taustin Exp taustin $
 *
 * $Log: dlite.h,v $
 * Revision 1.1  1997/03/11  01:30:26  taustin
 * Initial revision
 *
 *
 */

/*
 * This module implements DLite, the lite debugger.  DLite is a very light
 * weight semi-symbolic debbugger that can interface to any simulator with
 * only a few function calls.  See sim-safe.c for an example of how to
 * interface DLite to a simulator.
 *
 * The following commands are supported by DLite: 
 *

 *
 * help			 - print command reference
 * version		 - print DLite version information
 * terminate		 - terminate the simulation with statistics
 * quit			 - exit the simulator
 * cont {<addr>}	 - continue program execution (optionally at <addr>)
 * step			 - step program one instruction
 * next			 - step program one instruction in current procedure
 * print <expr>		 - print the value of <expr>
 * regs			 - print register contents
 * mstate		 - print machine specific state (simulator dependent)
 * display/<mods> <addr> - display the value at <addr> using format <modifiers>
 * dump {<addr>} {<cnt>} - dump memory at <addr> (optionally for <cnt> words)
 * dis <addr> {<cnt>}	 - disassemble instructions at <addr> (for <cnt> insts)
 * break <addr>		 - set breakpoint at <addr>, returns <id> of breakpoint
 * dbreak <addr> {r|w|x} - set data breakpoint at <addr> (for (r)ead, (w)rite,
 *			   and/or e(x)ecute, returns <id> of breakpoint
 * breaks		 - list active code and data breakpoints
 * delete <id>		 - delete breakpoint <id>
 * clear		 - clear all breakpoints (code and data)
 *
 * ** command args <addr>, <cnt>, <expr>, and <id> are any legal expression:
 *
 * <expr>		<- <factor> +|- <expr>
 * <factor>		<- <term> *|/ <factor>
 * <term>		<- ( <expr> )
 *			   | - <term>
 *			   | <const>
 *			   | <symbol>
 *			   | <file:loc>
 *
 * ** command modifiers <mods> are any of the following:
 *
 * b - print a byte
 * h - print a half (short)
 * w - print a word
 * t - print in decimal format
 * u - print in unsigned decimal format
 * o - print in octal format
 * x - print in hex format
 * 1 - print in binary format
 * f - print a float
 * d - print a double
 * c - print a character
 * s - print a string
 */

#ifndef DLITE_H
#define DLITE_H

#include <stdio.h>
#include "misc.h"
#include "ss.h"
#include "regs.h"

/* state access commands */
enum dlite_access_t {
  at_read,		/* read register/memory value */
  at_write,		/* write register/memory value */
  at_NUM
};

/* register file object definitions */

/* register bank specifier */
enum dlite_reg_t {
  rt_gpr,		/* general purpose register */
  rt_lpr,		/* integer-precision floating pointer register */
  rt_fpr,		/* single-precision floating pointer register */
  rt_dpr,		/* double-precision floating pointer register */
  rt_hi,		/* HI register */
  rt_lo,		/* LO register */
  rt_FCC,		/* float pointer condition codes register */
  rt_PC,		/* program counter */
  rt_NUM
};

/* register bank value */
union dlite_reg_val_t {
  SS_WORD_TYPE as_word;		/* for rt_gpr, rt_lpr, rt_hi, rt_lo */
  SS_FLOAT_TYPE as_float;	/* for rt_fpr */
  SS_FLOAT_TYPE as_double;	/* for rt_dpr */
  SS_ADDR_TYPE as_address;	/* for rt_PC */
  int as_condition;		/* for rt_FCC */
};

/* DLite register access function, the debugger uses this function to access
   simulator register state */
typedef char *			  	   /* error str, NULL for no err */
(*dlite_reg_obj_t)(enum dlite_access_t at, /* access type */
		   enum dlite_reg_t rt,	   /* reg bank to access */
		   int reg,		   /* register number */
		   union dlite_reg_val_t *val);	/* input, output */

/* DLite memory access function, the debugger uses this function to access
   simulator memory state */
typedef char *			  	   /* error str, NULL for no err */
(*dlite_mem_obj_t)(enum dlite_access_t at, /* access type */
		   SS_ADDR_TYPE addr,	   /* address to access */
		   char *p,		   /* input/output buffer */
		   int nbytes);		   /* size of access */

/* DLite memory access function, the debugger uses this function to display
   the state of machine-specific state */
typedef char *			  	   /* error str, NULL for no err */
(*dlite_mstate_obj_t)(FILE *stream,	   /* output stream */
		      char *cmd);	   /* optional command string */

/* initialize the DLite debugger */
void
dlite_init(dlite_reg_obj_t reg_obj,		/* register state object */
	   dlite_mem_obj_t mem_obj,		/* memory state object */
	   dlite_mstate_obj_t mstate_obj);	/* machine state object */

/*
 * default architected/machine state accessors
 */

/* default architected register state accessor */
char *						/* err str, NULL for no err */
dlite_reg_obj(enum dlite_access_t at,		/* access type */
	      enum dlite_reg_t rt,		/* reg bank to probe */
	      int reg,				/* register number */
	      union dlite_reg_val_t *val);	/* input, output */

/* default architected memory state accessor */
char *						/* err str, NULL for no err */
dlite_mem_obj(enum dlite_access_t at,		/* access type */
	      SS_ADDR_TYPE addr,		/* address to access */
	      char *p,				/* input, output */
	      int nbytes);			/* size of access */

/* default architected machine-specific state accessor */
char *						/* err str, NULL for no err */
dlite_mstate_obj(FILE *stream,			/* output stream */
		 char *cmd);			/* optional command string */

/* state access masks */
#define ACCESS_READ	0x01			/* read access allowed */
#define ACCESS_WRITE	0x02			/* write access allowed */
#define ACCESS_EXEC	0x04			/* execute access allowed */

/* non-zero iff one breakpoint is set, for fast break check */
extern SS_ADDR_TYPE dlite_fastbreak /* = 0 */;

/* set non-zero to enter DLite after next instruction */
extern int dlite_active /* = FALSE */;

/* non-zero to force a check for a break */
extern int dlite_check /* = FALSE */;

/* internal break check interface */
int						/* non-zero if brkpt hit */
__check_break(SS_ADDR_TYPE next_PC,		/* address of next inst */
	      int access,			/* mem access of last inst */
	      SS_ADDR_TYPE addr,		/* mem addr of last inst */
	      SS_COUNTER_TYPE icount,		/* instruction count */
	      SS_COUNTER_TYPE cycle);		/* cycle count */

/* check for a break condition */
#define dlite_check_break(NPC, ACCESS, ADDR, ICNT, CYCLE)		\
  ((dlite_check || dlite_active)					\
   ? __check_break((NPC), (ACCESS), (ADDR), (ICNT), (CYCLE))		\
   : FALSE)

/* DLite debugger main loop */
void
dlite_main(SS_ADDR_TYPE regs_PC,		/* addr of last inst to exec */
	   SS_ADDR_TYPE next_PC,		/* addr of next inst to exec */
	   SS_COUNTER_TYPE cycle);		/* current cycle */

#endif /* DLITE_H */
