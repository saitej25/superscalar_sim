/*
 * bpred.h - branch predictor interfaces
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
 * $Id: bpred.h,v 1.1.1.1 1997/05/22 18:04:05 aklauser Exp $
 *
 * $Log: bpred.h,v $
 * Revision 1.1.1.1  1997/05/22 18:04:05  aklauser
 *
 * Revision 1.8  1997/05/01 20:23:06  skadron
 * BTB bug fixes; jumps no longer update direction state; non-taken
 *    branches non longer update BTB
 *
 * Revision 1.7  1997/05/01 00:05:51  skadron
 * Separated BTB from direction-predictor
 *
 * Revision 1.6  1997/04/29  23:50:44  skadron
 * Added r31 info to distinguish between return-JRs and other JRs for bpred
 *
 * Revision 1.5  1997/04/29  22:53:10  skadron
 * Hopefully bpred is now right: bpred now allocates entries only for
 *    branches; on a BTB miss it still returns a direction; and it uses a
 *    return-address stack.  Returns are not yet distinguished among JR's
 *
 * Revision 1.4  1997/04/28  17:37:09  skadron
 * Bpred now allocates entries for any instruction instead of only
 *    branches; also added return-address stack
 *
 * Revision 1.3  1997/04/24  16:57:27  skadron
 * Bpred used to return no prediction if the indexing branch didn't match
 *    in the BTB.  Now it can predict a direction even on a BTB address
 *    conflict
 *
 * Revision 1.2  1997/03/25 16:17:21  skadron
 * Added function called after priming
 *
 * Revision 1.1  1997/02/16  22:23:54  skadron
 * Initial revision
 *
 *
 */

#ifndef BPRED_H
#define BPRED_H

#define dassert(a) assert(a)

#include <stdio.h>
#include "misc.h"
#include "ss.h"
#include "stats.h"

/*
 * This module implements a number of branch predictor mechanisms.  The
 * following predictors are supported:
 *
 *	BPred2Level:  two level adaptive branch predictor
 *
 *		It can simulate many prediction mechanisms that have up to
 *		two levels of tables. Parameters are:
 *		     N   # entries in first level (# of shift register(s))
 *		     W   width of shift register(s)
 *		     M   # entries in 2nd level (# of counters, or other FSM)
 *		One BTB entry per level-2 counter.
 *
 *		Configurations:   N, W, M
 *
 *		    counter based: 1, 0, M
 *
 *		    GAg          : 1, W, 2^W
 *		    GAp          : 1, W, M (M > 2^W)
 *		    PAg          : N, W, 2^W
 *		    PAp          : N, W, M (M == 2^(N+W))
 *
 *	BPred2bit:  a simple direct mapped bimodal predictor
 *
 *		This predictor has a table of two bit saturating counters.
 *		Where counter states 0 & 1 are predict not taken and
 *		counter states 2 & 3 are predict taken, the per-branch counters
 *		are incremented on taken branches and decremented on
 *		no taken branches.  One BTB entry per counter.
 *
 *	BPredTaken:  static predict branch taken
 *
 *	BPredNotTaken:  static predict branch not taken
 *
 */

/* branch predictor types */
enum bpred_class {
  BPredComb,                    /* combined predictor (McFarling) */
  BPred2Level,			/* 2-level correlating pred w/2-bit counters */
  BPred2bit,			/* 2-bit saturating cntr pred (dir mapped) */
  BPredTaken,			/* static predict taken */
  BPredNotTaken,		/* static predict not taken */
  BPred_NUM
};

/* an entry in a BTB */
struct bpred_btb_ent {
  SS_ADDR_TYPE addr;		/* address of branch being tracked */
  enum ss_opcode op;		/* opcode of branch corresp. to addr */
  SS_ADDR_TYPE target;		/* last destination of branch when taken */
  struct bpred_btb_ent *prev, *next; /* lru chaining pointers */
};

/* direction predictor def */
struct bpred_dir {
  enum bpred_class class;	/* type of predictor */
  union {
    struct {
      unsigned int size;	/* number of entries in direct-mapped table */
      unsigned char *table;	/* prediction state table */
    } bimod;
    struct {
      int l1size;		/* level-1 size, number of history regs */
      int l2size;		/* level-2 size, number of pred states */
      int shift_width;		/* amount of history in level-1 shift regs */
      int xor;			/* history xor address flag */
      int *shiftregs;		/* level-1 history table */
      unsigned char *l2table;	/* level-2 prediction state table */
    } two;
  } config;
};

/* branch predictor def */
struct bpred {
  enum bpred_class class;	/* type of predictor */
  struct {
    struct bpred_dir *bimod;     /* first direction predictor */
    struct bpred_dir *twolev;    /* second direction predictor */
    struct bpred_dir *meta;      /* meta predictor */
  } dirpred;

  struct {
    int sets;			/* num BTB sets */
    int assoc;			/* BTB associativity */
    struct bpred_btb_ent *btb_data; /* BTB addr-prediction table */
  } btb;

  struct {
    int size;			/* return-address stack size */
    int tos;			/* top-of-stack */
    struct bpred_btb_ent *stack; /* return-address stack */
  } retstack;

  /* stats */
  SS_COUNTER_TYPE addr_hits;	/* num correct addr-predictions */
  SS_COUNTER_TYPE dir_hits;	/* num correct dir-predictions (incl addr) */
  SS_COUNTER_TYPE used_bimod;	/* num bimodal predictions used (BPredComb) */
  SS_COUNTER_TYPE used_2lev;	/* num 2-level predictions used (BPredComb) */
  SS_COUNTER_TYPE jr_hits;	/* num correct addr-predictions for JR's */
  SS_COUNTER_TYPE jr_seen;	/* num JR's seen */
  SS_COUNTER_TYPE misses;	/* num incorrect predictions */

  SS_COUNTER_TYPE lookups;	/* num lookups */
  SS_COUNTER_TYPE retstack_pops;   /* number of times a value was popped */
  SS_COUNTER_TYPE retstack_pushes; /* number of times a value was pushed */
};

/* branch predictor update information */
struct bpred_update {
  char *pdir1;         /* direction-1 predictor counter */
  char *pdir2;         /* direction-2 predictor counter */
  char *pmeta;         /* meta predictor counter */
  struct {             /* predicted directions */
    uint bimod  : 1;    /* bimodal predictor */
    uint twolev : 1;    /* 2-level predictor */
    uint meta   : 1;    /* meta predictor (0..bimod / 1..2lev) */
  } dir;
};

/* create a branch predictor */
struct bpred *				/* branch predictory instance */
bpred_create(enum bpred_class class,	/* type of predictor to create */
	     unsigned int bimod_size,	/* bimod table size */
	     unsigned int l1size,	/* level-1 table size */
	     unsigned int l2size,	/* level-2 table size */
	     unsigned int meta_size,	/* meta predictor table size */
	     unsigned int shift_width,	/* history register width */
	     unsigned int xor,		/* history xor address flag */
	     unsigned int btb_sets,	/* number of sets in BTB */ 
	     unsigned int btb_assoc,	/* BTB associativity */
	     unsigned int retstack_size);/* num entries in ret-addr stack */

/* create a branch direction predictor */
struct bpred_dir *		/* branch direction predictor instance */
bpred_dir_create (
  enum bpred_class class,	/* type of predictor to create */
  unsigned int l1size,		/* level-1 table size */
  unsigned int l2size,		/* level-2 table size (if relevant) */
  unsigned int shift_width,	/* history register width */
  unsigned int  xor);   	/* history xor address flag */

/* print branch predictor configuration */
void
bpred_config(struct bpred *pred,	/* branch predictor instance */
	     FILE *stream);		/* output stream */

/* print predictor stats */
void
bpred_stats(struct bpred *pred,		/* branch predictor instance */
	    FILE *stream);		/* output stream */

/* register branch predictor stats */
void
bpred_reg_stats(struct bpred *pred,	/* branch predictor instance */
		struct stat_sdb_t *sdb);/* stats database */

/* reset stats after priming, if appropriate */
void bpred_after_priming(struct bpred *bpred);

/* probe a predictor for a next fetch address, the predictor is probed
   with branch address BADDR, the branch target is BTARGET (used for
   static predictors), and OP is the instruction opcode (used to simulate
   predecode bits; a pointer to the predictor state entry (or null for jumps)
   is returned in *DIR_UPDATE_PTR (used for updating predictor state),
   and the non-speculative top-of-stack is returned in stack_recover_idx 
   (used for recovering ret-addr stack after mis-predict).  */
SS_ADDR_TYPE				/* predicted branch target addr */
bpred_lookup(struct bpred *pred,	/* branch predictor instance */
	     SS_ADDR_TYPE baddr,	/* branch address */
	     SS_ADDR_TYPE btarget,	/* branch target if taken */
	     enum ss_opcode op,		/* opcode of instruction */
	     int r31p,			/* is this using r31? */
	     struct bpred_update *dir_update_ptr,/* predictor state pointer */
	     int *stack_recover_idx);	/* Non-speculative top-of-stack;
					 * used on mispredict recovery */

/* Speculative execution can corrupt the ret-addr stack.  So for each
 * lookup we return the top-of-stack (TOS) at that point; a mispredicted
 * branch, as part of its recovery, restores the TOS using this value --
 * hopefully this uncorrupts the stack. */
void
bpred_recover(struct bpred *pred,	/* branch predictor instance */
	      SS_ADDR_TYPE baddr,	/* branch address */
	      int stack_recover_idx);	/* Non-speculative top-of-stack;
					 * used on mispredict recovery */

/* update the branch predictor, only useful for stateful predictors; updates
   entry for instruction type OP at address BADDR.  BTB only gets updated
   for branches which are taken.  Inst was determined to jump to
   address BTARGET and was taken if TAKEN is non-zero.  Predictor 
   statistics are updated with result of prediction, indicated by CORRECT and 
   PRED_TAKEN, predictor state to be updated is indicated by *DIR_UPDATE_PTR 
   (may be NULL for jumps, which shouldn't modify state bits).  Note if
   bpred_update is done speculatively, branch-prediction may get polluted. */
void
bpred_update(struct bpred *pred,	/* branch predictor instance */
	     SS_ADDR_TYPE baddr,	/* branch address */
	     SS_ADDR_TYPE btarget,	/* resolved branch target */
	     int taken,			/* non-zero if branch was taken */
	     int pred_taken,		/* non-zero if branch was pred taken */
	     int correct,		/* was earlier prediction correct? */
	     enum ss_opcode op,		/* opcode of instruction */
	     int r31p,			/* is this using r31? */
	     struct bpred_update *dir_update_ptr);/* predictor state pointer */


#ifdef foo0
/* OBSOLETE */
/* dump branch predictor state (for debug) */
void
bpred_dump(struct bpred *pred,		/* branch predictor instance */
	   FILE *stream);		/* output stream */
#endif

#endif /* BPRED_H */
