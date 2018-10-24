/*
 * regs.h - architected register state interfaces
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
 * $Id: regs.h,v 1.4 1997/03/11 01:19:41 taustin Exp taustin $
 *
 * $Log: regs.h,v $
 * Revision 1.4  1997/03/11  01:19:41  taustin
 * updated copyright
 * long/int tweaks made for ALPHA target support
 *
 * Revision 1.3  1997/01/06  16:02:48  taustin
 * \comments updated
 *
 * Revision 1.1  1996/12/05  18:50:23  taustin
 * Initial revision
 *
 *
 */

#ifndef REGS_H
#define REGS_H

#include "ss.h"

/*
 * This module implements the SimpleScalar architected register state, which
 * includes integer and floating point registers and miscellaneous registers.
 * The architected register state is as follows:
 *
 * Integer Register File:                  Miscellaneous Registers:
 * (aka general-purpose registers, GPR's)
 *
 *   +------------------+                  +------------------+
 *   | $r0 (src/sink 0) |                  | PC               | Program Counter
 *   +------------------+                  +------------------+
 *   | $r1              |                  | HI               | Mult/Div HI val
 *   +------------------+                  +------------------+
 *   |  .               |                  | LO               | Mult/Div LO val
 *   |  .               |                  +------------------+
 *   |  .               |
 *   +------------------+
 *   | $r31             |
 *   +------------------+
 *
 * Floating point Register File:
 *    single-precision:  double-precision:
 *   +------------------+------------------+  +------------------+
 *   | $f0              | $f1 (for double) |  | FCC              | FP codes
 *   +------------------+------------------+  +------------------+
 *   | $f1              |
 *   +------------------+
 *   |  .               |
 *   |  .               |
 *   |  .               |
 *   +------------------+------------------+
 *   | $f30             | $f31 (for double)|
 *   +------------------+------------------+
 *   | $f31             |
 *   +------------------+
 *
 * The floating point register file can be viewed as either 32 single-precision
 * (32-bit IEEE format) floating point values $f0 to $f31, or as 16
 * double-precision (64-bit IEEE format) floating point values $f0 to $f30.
 */

/* (signed) integer register file */
extern SS_WORD_TYPE regs_R[SS_NUM_REGS];

/* floating point register file format */
union regs_FP {
    SS_WORD_TYPE l[SS_NUM_REGS];		/* integer word view */
    SS_FLOAT_TYPE f[SS_NUM_REGS];		/* single-precision FP view */
    SS_DOUBLE_TYPE d[SS_NUM_REGS/2];		/* double-precision FP view */
};

/* floating point register file */
extern union regs_FP regs_F;

/* (signed) hi register, holds mult/div results */
extern SS_WORD_TYPE regs_HI;

/* (signed) lo register, holds mult/div results */
extern SS_WORD_TYPE regs_LO;

/* floating point condition codes */
extern int regs_FCC;

/* program counter */
extern SS_ADDR_TYPE regs_PC;

/* initialize architected register state */
void regs_init(void);

/* dump all architected register state values to output stream STREAM */
void
regs_dump(FILE *stream);	/* output stream */

#endif /* REGS_H */
