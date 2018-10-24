/*
 * sim-bpred.c - sample branch predictor simulator implementation
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
 * $Id$
 *
 * $Log$
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "misc.h"
#include "ss.h"
#include "regs.h"
#include "memory.h"
#include "loader.h"
#include "syscall.h"
#include "dlite.h"
#include "options.h"
#include "stats.h"
#include "bpred.h"
#include "sim.h"

/*
 * This file implements a branch predictor analyzer.
 */

/* branch predictor type {nottaken|taken|perfect|bimod|2lev} */
static char *pred_type;

/* bimodal predictor config (<table_size>) */
static int bimod_nelt = 1;
static int bimod_config[1] =
  { /* bimod tbl size */2048 };

/* 2-level predictor config (<l1size> <l2size> <hist_size> <xor>) */
static int twolev_nelt = 4;
static int twolev_config[4] =
  { /* l1size */1, /* l2size */1024, /* hist */8, /* xor */FALSE};

/* combining predictor config (<meta_table_size> */
static int comb_nelt = 1;
static int comb_config[1] =
  { /* meta_table_size */1024 };

/* return address stack (RAS) size */
static int ras_size = 8;

/* BTB predictor config (<num_sets> <associativity>) */
static int btb_nelt = 2;
static int btb_config[2] =
  { /* nsets */512, /* assoc */4 };

/* branch predictor */
static struct bpred *pred;

/* track number of insn and refs */
static SS_COUNTER_TYPE sim_num_insn = 0;
static SS_COUNTER_TYPE sim_num_refs = 0;

/* total number of branches executed */
static SS_COUNTER_TYPE sim_num_branches = 0;


/* register simulator-specific options */
void
sim_reg_options(struct opt_odb_t *odb)
{
  opt_reg_header(odb, 
"sim-bpred: This simulator implements a branch predictor analyzer.\n"
		 );

  /* branch predictor options */
  opt_reg_note(odb,
"  Branch predictor configuration examples for 2-level predictor:\n"
"    Configurations:   N, M, W, X\n"
"      N   # entries in first level (# of shift register(s))\n"
"      W   width of shift register(s)\n"
"      M   # entries in 2nd level (# of counters, or other FSM)\n"
"      X   (yes-1/no-0) xor history and address for 2nd level index\n"
"    Sample predictors:\n"
"      GAg     : 1, W, 2^W, 0\n"
"      GAp     : 1, W, M (M > 2^W), 0\n"
"      PAg     : N, W, 2^W, 0\n"
"      PAp     : N, W, M (M == 2^(N+W)), 0\n"
"      gshare  : 1, W, 2^W, 1\n"
"  Predictor `comb' combines a bimodal and a 2-level predictor.\n"
               );

  opt_reg_string(odb, "-bpred",
		 "branch predictor type {nottaken|taken|bimod|2lev|comb}",
                 &pred_type, /* default */"bimod",
                 /* print */TRUE, /* format */NULL);

  opt_reg_int_list(odb, "-bpred:bimod",
		   "bimodal predictor config (<table size>)",
		   bimod_config, bimod_nelt, &bimod_nelt,
		   /* default */bimod_config,
		   /* print */TRUE, /* format */NULL, /* !accrue */FALSE);

  opt_reg_int_list(odb, "-bpred:2lev",
                   "2-level predictor config "
		   "(<l1size> <l2size> <hist_size> <xor>)",
                   twolev_config, twolev_nelt, &twolev_nelt,
		   /* default */twolev_config,
                   /* print */TRUE, /* format */NULL, /* !accrue */FALSE);

  opt_reg_int_list(odb, "-bpred:comb",
		   "combining predictor config (<meta_table_size>)",
		   comb_config, comb_nelt, &comb_nelt,
		   /* default */comb_config,
		   /* print */TRUE, /* format */NULL, /* !accrue */FALSE);

  opt_reg_int(odb, "-bpred:ras",
              "return address stack size (0 for no return stack)",
              &ras_size, /* default */ras_size,
              /* print */TRUE, /* format */NULL);

  opt_reg_int_list(odb, "-bpred:btb",
		   "BTB config (<num_sets> <associativity>)",
		   btb_config, btb_nelt, &btb_nelt,
		   /* default */btb_config,
		   /* print */TRUE, /* format */NULL, /* !accrue */FALSE);
}

/* check simulator-specific option values */
void
sim_check_options(struct opt_odb_t *odb, int argc, char **argv)
{
  if (!mystricmp(pred_type, "taken"))
    {
      /* static predictor, not taken */
      pred = bpred_create(BPredTaken, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
  else if (!mystricmp(pred_type, "nottaken"))
    {
      /* static predictor, taken */
      pred = bpred_create(BPredNotTaken, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
  else if (!mystricmp(pred_type, "bimod"))
    {
      if (bimod_nelt != 1)
	fatal("bad bimod predictor config (<table_size>)");
      if (btb_nelt != 2)
	fatal("bad btb config (<num_sets> <associativity>)");

      /* bimodal predictor, bpred_create() checks BTB_SIZE */
      pred = bpred_create(BPred2bit,
			  /* bimod table size */bimod_config[0],
			  /* 2lev l1 size */0,
			  /* 2lev l2 size */0,
			  /* meta table size */0,
			  /* history reg size */0,
			  /* history xor address */0,
			  /* btb sets */btb_config[0],
			  /* btb assoc */btb_config[1],
			  /* ret-addr stack size */ras_size);
    }
  else if (!mystricmp(pred_type, "2lev"))
    {
      /* 2-level adaptive predictor, bpred_create() checks args */
      if (twolev_nelt != 4)
	fatal("bad 2-level pred config (<l1size> <l2size> <hist_size> <xor>)");
      if (btb_nelt != 2)
	fatal("bad btb config (<num_sets> <associativity>)");

      pred = bpred_create(BPred2Level,
			  /* bimod table size */0,
			  /* 2lev l1 size */twolev_config[0],
			  /* 2lev l2 size */twolev_config[1],
			  /* meta table size */0,
			  /* history reg size */twolev_config[2],
			  /* history xor address */twolev_config[3],
			  /* btb sets */btb_config[0],
			  /* btb assoc */btb_config[1],
			  /* ret-addr stack size */ras_size);
    }
  else if (!mystricmp(pred_type, "comb"))
    {
      /* combining predictor, bpred_create() checks args */
      if (twolev_nelt != 4)
	fatal("bad 2-level pred config (<l1size> <l2size> <hist_size> <xor>)");
      if (bimod_nelt != 1)
	fatal("bad bimod predictor config (<table_size>)");
      if (comb_nelt != 1)
	fatal("bad combining predictor config (<meta_table_size>)");
      if (btb_nelt != 2)
	fatal("bad btb config (<num_sets> <associativity>)");

      pred = bpred_create(BPredComb,
			  /* bimod table size */bimod_config[0],
			  /* l1 size */twolev_config[0],
			  /* l2 size */twolev_config[1],
			  /* meta table size */comb_config[0],
			  /* history reg size */twolev_config[2],
			  /* history xor address */twolev_config[3],
			  /* btb sets */btb_config[0],
			  /* btb assoc */btb_config[1],
			  /* ret-addr stack size */ras_size);
    }
  else
    fatal("cannot parse predictor type `%s'", pred_type);
}

/* register simulator-specific statistics */
void
sim_reg_stats(struct stat_sdb_t *sdb)
{
  stat_reg_counter(sdb, "sim_num_insn",
		   "total number of instructions executed",
		   &sim_num_insn, 0, NULL);
  stat_reg_counter(sdb, "sim_num_refs",
		   "total number of loads and stores executed",
		   &sim_num_refs, 0, NULL);
  stat_reg_int(sdb, "sim_elapsed_time",
	       "total simulation time in seconds",
	       (int *)&sim_elapsed_time, 0, NULL);
  stat_reg_formula(sdb, "sim_inst_rate",
		   "simulation speed (in insts/sec)",
		   "sim_num_insn / sim_elapsed_time", NULL);

  stat_reg_counter(sdb, "sim_num_branches",
                   "total number of branches executed",
                   &sim_num_branches, /* initial value */0, /* format */NULL);
  stat_reg_formula(sdb, "sim_IPB",
                   "instruction per branch",
                   "sim_num_insn / sim_num_branches", /* format */NULL);

  /* register predictor stats */
  if (pred)
    bpred_reg_stats(pred, sdb);
}

/* initialize the simulator */
void
sim_init(void)
{
  SS_INST_TYPE inst;

  sim_num_insn = 0;
  sim_num_refs = 0;

  regs_PC = ld_prog_entry;

  /* decode all instructions */
  {
    SS_ADDR_TYPE addr;

    if (OP_MAX > 255)
      fatal("cannot perform fast decoding, too many opcodes");

    debug("sim: decoding text segment...");
    for (addr=ld_text_base;
	 addr < (ld_text_base+ld_text_size);
	 addr += SS_INST_SIZE)
      {
	inst = __UNCHK_MEM_ACCESS(SS_INST_TYPE, addr);
	inst.a = SWAP_WORD(inst.a);
	inst.b = SWAP_WORD(inst.b);
	inst.a = (inst.a & ~0xff) | (unsigned int)SS_OP_ENUM(SS_OPCODE(inst));
	__UNCHK_MEM_ACCESS(SS_INST_TYPE, addr) = inst;
      }
  }

  /* initialize the DLite debugger */
  dlite_init(dlite_reg_obj, dlite_mem_obj, dlite_mstate_obj);
}


/* print simulator-specific configuration information */
void
sim_aux_config(FILE *stream)		/* output stream */
{
  /* nothing currently */
}

/* dump simulator-specific auxiliary simulator statistics */
void
sim_aux_stats(FILE *stream)		/* output stream */
{
  /* nada */
}

/* un-initialize simulator-specific state */
void
sim_uninit(void)
{
  /* nada */
}


/*
 * configure the execution engine
 */

/*
 * precise architected register accessors
 */

/* next program counter */
#define SET_NPC(EXPR)		(next_PC = (EXPR))

/* target program counter */
#undef  SET_TPC
#define SET_TPC(EXPR)		(target_PC = (EXPR))

/* current program counter */
#define CPC			(regs_PC)

/* general purpose registers */
#define GPR(N)			(regs_R[N])
#define SET_GPR(N,EXPR)		(regs_R[N] = (EXPR))

/* floating point registers, L->word, F->single-prec, D->double-prec */
#define FPR_L(N)		(regs_F.l[(N)])
#define SET_FPR_L(N,EXPR)	(regs_F.l[(N)] = (EXPR))
#define FPR_F(N)		(regs_F.f[(N)])
#define SET_FPR_F(N,EXPR)	(regs_F.f[(N)] = (EXPR))
#define FPR_D(N)		(regs_F.d[(N) >> 1])
#define SET_FPR_D(N,EXPR)	(regs_F.d[(N) >> 1] = (EXPR))

/* miscellaneous register accessors */
#define SET_HI(EXPR)		(regs_HI = (EXPR))
#define HI			(regs_HI)
#define SET_LO(EXPR)		(regs_LO = (EXPR))
#define LO			(regs_LO)
#define FCC			(regs_FCC)
#define SET_FCC(EXPR)		(regs_FCC = (EXPR))

/* precise architected memory state help functions */
#define __READ_WORD(DST_T, SRC_T, SRC)					\
  ((unsigned int)((DST_T)(SRC_T)MEM_READ_WORD(addr = (SRC))))

#define __READ_HALF(DST_T, SRC_T, SRC)					\
  ((unsigned int)((DST_T)(SRC_T)MEM_READ_HALF(addr = (SRC))))

#define __READ_BYTE(DST_T, SRC_T, SRC)					\
  ((unsigned int)((DST_T)(SRC_T)MEM_READ_BYTE(addr = (SRC))))

/* precise architected memory state accessor macros */
#define READ_WORD(SRC)							\
  __READ_WORD(unsigned int, unsigned int, (SRC))

#define READ_UNSIGNED_HALF(SRC)						\
  __READ_HALF(unsigned int, unsigned short, (SRC))

#define READ_SIGNED_HALF(SRC)						\
  __READ_HALF(signed int, signed short, (SRC))

#define READ_UNSIGNED_BYTE(SRC)						\
  __READ_BYTE(unsigned int, unsigned char, (SRC))

#define READ_SIGNED_BYTE(SRC)						\
  __READ_BYTE(signed int, signed char, (SRC))

#define WRITE_WORD(SRC, DST)						\
  (MEM_WRITE_WORD(addr = (DST), (unsigned int)(SRC)))

#define WRITE_HALF(SRC, DST)						\
  (MEM_WRITE_HALF(addr = (DST), (unsigned short)(unsigned int)(SRC)))

#define WRITE_BYTE(SRC, DST)						\
  (MEM_WRITE_BYTE(addr = (DST), (unsigned char)(unsigned int)(SRC)))

/* system call handler macro */
#define SYSCALL(INST)		(ss_syscall(mem_access, INST))

/* instantiate the helper functions in the '.def' file */
#define DEFINST(OP,MSK,NAME,OPFORM,RES,CLASS,O1,O2,I1,I2,I3,EXPR)
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)
#define CONNECT(OP)
#define IMPL
#include "ss.def"
#undef DEFINST
#undef DEFLINK
#undef CONNECT
#undef IMPL

/* start simulation, program loaded, processor precise state initialized */
void
sim_main(void)
{
  SS_INST_TYPE inst;
  register SS_ADDR_TYPE next_PC, target_PC;
  register SS_ADDR_TYPE addr;
  enum ss_opcode op;
  register int is_write;
  int stack_idx;

  fprintf(stderr, "sim: ** starting functional simulation **\n");

  /* set up initial default next PC */
  next_PC = regs_PC + SS_INST_SIZE;

  /* check for DLite debugger entry condition */
  if (dlite_check_break(regs_PC, /* no access */0, /* addr */0, 0, 0))
    dlite_main(regs_PC - SS_INST_SIZE, regs_PC, sim_num_insn);

  while (TRUE)
    {
      /* maintain $r0 semantics */
      regs_R[0] = 0;

      /* keep an instruction count */
      sim_num_insn++;

      /* get the next instruction to execute */
      inst = __UNCHK_MEM_ACCESS(SS_INST_TYPE, regs_PC);

      /* set default reference address and access mode */
      addr = 0; is_write = FALSE;

      /* decode the instruction */
      op = SS_OPCODE(inst);
      switch (op)
	{
#define DEFINST(OP,MSK,NAME,OPFORM,RES,FLAGS,O1,O2,I1,I2,I3,EXPR)	\
	case OP:                                                        \
          EXPR;                                                         \
          break;
#define DEFLINK(OP,MSK,NAME,MASK,SHIFT)                                 \
        case OP:                                                        \
          panic("attempted to execute a linking opcode");
#define CONNECT(OP)
#include "ss.def"
#undef DEFINST
#undef DEFLINK
#undef CONNECT
	default:
	  panic("bogus opcode");
      }

      if (SS_OP_FLAGS(op) & F_MEM)
	{
	  sim_num_refs++;
	  if (SS_OP_FLAGS(op) & F_STORE)
	    is_write = TRUE;
	}

      if (SS_OP_FLAGS(op) & F_CTRL)
	{
	  SS_ADDR_TYPE pred_PC;
	  struct bpred_update update_rec;

	  sim_num_branches++;

	  if (pred)
	    {
	      /* get the next predicted fetch address */
	      pred_PC = bpred_lookup(pred,
				     /* branch addr */regs_PC,
				     /* target */target_PC,
				     /* opcode */op,
				     /* jump through R31? */(RS) == 31,
				     /* stash an update ptr */&update_rec,
				     /* stash return stack ptr */&stack_idx);

	      /* valid address returned from branch predictor? */
	      if (!pred_PC)
		{
		  /* no predicted taken target, attempt not taken target */
		  pred_PC = regs_PC + sizeof(SS_INST_TYPE);
		}

	      bpred_update(pred,
			   /* branch addr */regs_PC,
			   /* resolved branch target */next_PC,
			   /* taken? */next_PC != (regs_PC +
						   sizeof(SS_INST_TYPE)),
			   /* pred taken? */pred_PC != (regs_PC +
							sizeof(SS_INST_TYPE)),
			   /* correct pred? */pred_PC == next_PC,
			   /* opcode */op,
			   /* jump through R31? */(RS) == 31,
			   /* predictor update pointer */&update_rec);
	    }
	}

      /* check for DLite debugger entry condition */
      if (dlite_check_break(next_PC,
			    is_write ? ACCESS_WRITE : ACCESS_READ,
			    addr, sim_num_insn, sim_num_insn))
	dlite_main(regs_PC, next_PC, sim_num_insn);

      /* go to the next instruction */
      regs_PC = next_PC;
      next_PC += SS_INST_SIZE;
    }
}
