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

/* Algorithm for simulating fully associative caches of a fixed line 	*
*  size, and a range of sizes with LRU replacement.			*/


#include <stdio.h>

/* Macros */
#define toggle(i) if(i==1)i=0;else{i=1;}
#define min(a,b) ((a<b) ? a : b)

#define ONE 1
#define HASHNO 7211    /* number of slots in hash table */
#define INPUT_BUFFER_SIZE 1000
#define MAX_PHYSICAL_MEM 2097152  /* Physical mem available on machine */
#define BYTES_PER_LINE 35  /* Storage requirement per line */

struct hash_table {
        unsigned addr;
        int inum;
        struct hash_table *nxt;
};

struct tree_node {
        unsigned inum;
	unsigned addr;
	int dummy_grpno; /* For compatibility with facopt tree_node structure */
        int rtwt;
        struct tree_node *lft, *rt;
};  

static struct hash_table slot[HASHNO]; /* Hash table */
static struct tree_node *root,	/* Root of splay tree */
	      **p_stack;	/* Stack used for tree operations */

static unsigned *out_stack;	/* Stack depth hits array */

#ifdef PERF
double comp=0.0, no_splay_steps=0.0;
#endif
static unsigned tot_addrs,	/* Count of distinct line */
                t_entries;	/* Count of addresses processed */
static short size_exceeded=0;	/* Flag */


extern int L,		/* Line size */
           T;		/* Max addresses processed */
extern unsigned MAX_CACHE_SIZE;	/* Maximum cache size of interest */
extern unsigned MAX_LINES;	/* Calculated from max cache size and line size */
extern int MISS_RATIO_INTERVAL,	/* Output interval */
           SAVE_INTERVAL, /* Intervals at which output should be saved */
           P_INTERVAL;  /* Intervals at which progress output is done */

void outpr_faclru(FILE *fd);

/****************************************************************
Main simulation routine. Reads in addresses from trace, does hash and
tree lookups as required.

Input: None
Output: None
Side effects: Updates the tree and hash table and other globals as the
              simulation progresses.
****************************************************************/

extern FILE *reportfile;

static unsigned long next_save_time;

void
ptc(unsigned long addr)
{ 
  unsigned i_inum;		/* Key lookup */
  struct tree_node *nnode;	/* New tree node */
  struct tree_node *delete ();	/* Delete funtion */
  
  if (tot_addrs > MAX_LINES)
    if (size_exceeded == 0) {
      toggle (size_exceeded);
      fprintf(stderr, "libcheetah: distinct entries limit exceeded\n");
      fprintf(stderr, "libcheetah: addresses processed %d\n", t_entries);
    }
  if (t_entries > next_save_time) {
    outpr_faclru(stderr);
    next_save_time += SAVE_INTERVAL;
  }
    
  ++t_entries;
  if ((t_entries % P_INTERVAL) == 0)
    fprintf(stderr, "libcheetah: addresses processed %d\n", t_entries);
    
  addr >>=L;
  if (size_exceeded == 0)
    if ((i_inum = hash_lookup_add (addr)) != 0)
      ref_tree(i_inum, addr); 
    else {
      nnode = (struct tree_node *) malloc(sizeof(struct tree_node));
      if (nnode == NULL) {
	fprintf(stderr, "libcheetah: problem allocating space for tree\n");
	fprintf(stderr, "libcheetah: addrs processed: %d  distinct addrs: %d\n",
		t_entries, tot_addrs);
	exit (1);
      }
      nnode->inum = t_entries;
      nnode->addr = addr;
      nnode->lft = root;
      nnode->rt = NULL;
      nnode->rtwt = 0;
      root = nnode;
    }
  else
    if ((i_inum = hash_lookup_add(addr)) != 0)
      ref_tree(i_inum, addr);
    else {
      nnode = delete ();
      hash_del (nnode->addr);
      nnode->inum = t_entries;
      nnode->addr = addr;
      nnode->lft = root;
      nnode->rt = NULL;
      nnode->rtwt = 0;
      root = nnode;
    }
}



/***********************************************************************
Lookup 'addr' in the hash table. Adds 'addr' to the hash table if it is
found. The hashing scheme is 'modulo a prime'. Collision resolution is
by chaining.

Input: Address to be looked up
Output: Previous arrival time of address if found
        Zero if not.
Side effects: Adds the address to the hash table if it is not found.
              Updates the previous time of arrival of address.
***********************************************************************/

hash_lookup_add (addr)
unsigned addr;

{  int old_inum;		/* Scratch variables */
   int loc;
   struct hash_table *ptr, *oldptr;


   loc = addr % HASHNO;
   if (slot[loc].inum == 0){
        ++tot_addrs;
        slot[loc].addr = addr;
        slot[loc].inum = t_entries;
        slot[loc].nxt = NULL;
        return(0);
   }


   else{
        ptr = &slot[loc];
        while (ptr) {
            oldptr = ptr;
            if (ptr->addr == addr)
                 break;
            ptr = ptr->nxt;
        }
        if (ptr){
             old_inum = ptr->inum;
             ptr->inum = t_entries;
             return(old_inum);
        }
        else{
             ++tot_addrs;
             oldptr->nxt = (struct hash_table *) malloc(sizeof(struct hash_table));
	     if (oldptr->nxt == NULL) {
	       fprintf (stderr, "libcheetah: no memory for hash table\n");
	       fprintf (stderr,
			"libcheetah: addrs processed: %d distinct addrs: %d\n",
			t_entries, tot_addrs);
	       exit (1);
	     }
             oldptr->nxt->addr = addr;
             oldptr->nxt->nxt = NULL;
             oldptr->nxt->inum = t_entries;
             return(0);
        }
    }
}



/*****************************************************************
Deletes the input address from the hash table. The entry is freed
if possible.

Input: Address to be deleted
Output : None
Side effects: Deletes address from the hash table
*****************************************************************/

hash_del (addr)
unsigned addr;

{  int loc;		/* Scratch variables */
   struct hash_table *ptr, *oldptr;


   loc = addr % HASHNO;
   if (slot[loc].inum == 0){
     fprintf(stderr, "libcheetah: addr not found in hash_table\n");
   }
   else if (slot[loc].addr == addr) {
     if (slot[loc].nxt == NULL) {
       slot[loc].inum = 0;
     }
     else {
       slot[loc].addr = slot[loc].nxt->addr;
       slot[loc].inum = slot[loc].nxt->inum;
       ptr = slot[loc].nxt;
       slot[loc].nxt = slot[loc].nxt->nxt;
       free(ptr);
     }
   }
   else{
        ptr = &slot[loc];
        while (ptr) {
          if (ptr->addr == addr)
            break;
          oldptr = ptr;
          ptr = ptr->nxt;
        }
        if (ptr){
	  oldptr->nxt = ptr->nxt;
	  free (ptr);
        }
        else {
     	  fprintf (stderr, "libcheetah: addr not found in hash_table\n");
        }
    }
}



/******************************************************************
Looks up the key i_inum in the splay tree, deletes the node
and reinserts it at the top. Also splays the previous entry in the
stack to the root.

Input: Key to be looked up (i_inum) and current address.
Output: None
Side effects: Updates tree as described above.
******************************************************************/

ref_tree(i_inum, addr)
unsigned i_inum;
unsigned addr;


{  struct tree_node *ptr;
   int top, addr_above, pos, lstlft, at;


#ifdef PERF
   comp += 1.0;
#endif
   if (root->inum == i_inum){
        ++out_stack[1];
        root->inum = t_entries;
   }
   else{
        top = addr_above = lstlft = 0;
        ptr = root;
        while (ptr){
#ifdef PERF
           comp += 1.0;
#endif
           ++top;
           p_stack[top] = ptr;
           if (ptr->inum > i_inum){
                addr_above += ptr->rtwt + 1;
                lstlft = top;
                ptr = ptr->lft;
           }
           else{
                if (ptr->inum == i_inum){
                     addr_above += ptr->rtwt;
                     ++out_stack[addr_above + 1];
                     pos = top;
		     if (ptr->addr != addr)
		       fprintf(stderr,
			       "libcheetah: inconsistency w/ inum & addr'\n");
		     ptr->rtwt -= 1;
		     ptr = ptr->rt;
		     while (ptr) {
			++top;
			p_stack[top] = ptr;
			ptr = ptr->lft;
		     }
		     break;
                }
                ptr->rtwt -= 1;
                ptr = ptr->rt;
           }
        }                                                            

        if (pos == top){
             if (p_stack[top-1]->lft == p_stack[top])
                  p_stack[top-1]->lft = p_stack[top]->lft;
             else
                  p_stack[top-1]->rt = p_stack[top]->lft;
             at = lstlft;
        }
        else{
             if (p_stack[top-1]->lft == p_stack[top])
                  p_stack[top-1]->lft = p_stack[top]->rt;
             else
                  p_stack[top-1]->rt = p_stack[top]->rt;
        
	     p_stack[pos]->addr = p_stack[top]->addr;
             p_stack[pos]->inum = p_stack[top]->inum;
             at = top-1;
        }
        while (at > 1){
#ifdef PERF
	  no_splay_steps += 1.0;   /* Counts the number of basic operations */
#endif
            splay(at, p_stack);
            at = at - 2;
        }
        root = p_stack[1];

        p_stack[top]->lft = root;
        p_stack[top]->rt = NULL;
        p_stack[top]->inum = t_entries;
	p_stack[top]->addr = addr;
        p_stack[top]->rtwt = 0;
        root = p_stack[top];
      /*  traverse(root);  */
   }
}        

/******************************************************************
Deletes the last node in the stack from the tree.

Input: None
Output: Deleted node
Side effects: Deletes last node from tree.
******************************************************************/

struct tree_node
*delete()

{   struct tree_node *ptr, *free_ptr;
    static struct tree_node **delete_stack;
    static int top;
    static short first_time=1;
    static int last_inum;

 
    if ((first_time) || (top < 5) ||
		(delete_stack[top]->inum != last_inum) ||
                (delete_stack[top-1]->lft != delete_stack[top])) {
      if (first_time) {
        first_time = 0;
	delete_stack = (struct tree_node **) calloc (MAX_LINES, sizeof (struct tree_node *));
	if (delete_stack == NULL)
	  fprintf(stderr, "libcheetah: problem allocating space in delete()\n");
      }
      top = -1;
      ptr = root;
      while (ptr->lft != NULL){
	++top;
	delete_stack [top] = ptr;
        ptr = ptr->lft;
      }
      delete_stack[top]->lft = ptr->rt;
      free_ptr = ptr;

      ptr = delete_stack[top]->lft;
      while (ptr) {
	++top;
        delete_stack [top] = ptr;
        ptr = ptr->lft;
      }
      last_inum = delete_stack[top]->inum;
    }
    else {

      if (delete_stack[top]->lft != NULL)
        fprintf(stderr, "libcheetah: lft ptr of last entry not NULL\n");

      free_ptr = delete_stack[top];
      delete_stack[top-1]->lft = delete_stack[top]->rt;
      --top;
      ptr = delete_stack[top]->lft;
      while (ptr) {
        ++top;
        delete_stack [top] = ptr;
        ptr = ptr->lft;
      }
      last_inum = delete_stack[top]->inum;
    }

    return (free_ptr);
}




/*********************************************************************
Initialization routine. Creates a root for the splay tree
and allocates space for the arrays.

Input: None
Output: None
Side effects: Creates a root for the tree. Adds the address to the splay tree.
*********************************************************************/

init_faclru ()

{   unsigned addr;

    next_save_time = SAVE_INTERVAL;

      out_stack = (unsigned *) calloc ((MAX_LINES+INPUT_BUFFER_SIZE), sizeof (unsigned));
			/* Stack is not cut off precisely at MAX_LINES */
      p_stack = (struct tree_node **) calloc (MAX_LINES, sizeof (struct tree_node *));

      if ((out_stack == NULL) || (p_stack == NULL))
	goto error;

    addr = 1; /* Assumes that the first address is 1. */
    ++t_entries; /* Kludge to avoid a warning in hash_del */
    hash_lookup_add(addr);
    t_entries = 0;
    tot_addrs = 0; /* Resets tot_addrs which is incremented in hash_lookup_add */
    root = (struct tree_node *) malloc(sizeof(struct tree_node));
    if (root == NULL)
      goto error;
    root->inum = t_entries;
    root->addr = addr;
    root->rtwt = 0;
    root->lft = root->rt = NULL;
    return;
error:
    fprintf (stderr, "libcheetah: problem allocating space (in init)\n");
    exit (1);
}



/********************************************************************
Output routine.

Input: None
Output: None
Side effects: None
********************************************************************/

void
outpr_faclru(FILE *fd)
{  int i;
   int stack_end;
   unsigned sum=0;

   fprintf(fd, "Addresses processed : %d\n", t_entries);
   fprintf(fd, "Line size : %d bytes \n", (ONE << L));
   fprintf(fd, "Number of distinct lines  %d\n", tot_addrs); 
#ifdef PERF
   fprintf(fd, "Number of Comparisons %lf\n", comp);
   fprintf(fd, "Number of balance steps %lf\n", no_splay_steps);
#endif
   fprintf(fd, "Cache size (bytes)\tMiss Ratio\n");
   stack_end = min (tot_addrs, MAX_LINES);
   
   for (i = 1; i <= stack_end; i++){
	sum += out_stack[i];
	if ((i % MISS_RATIO_INTERVAL) == 0)
          fprintf(fd, "%d\t\t\t%1.6lf\n", (i * (ONE << L)),
			(1.0 - ((double)sum/(double)t_entries)));
   }
   if (stack_end == tot_addrs)
     fprintf(fd, "Miss ratio is %lf for all bigger caches\n",
	     (1.0 - ((double)sum/(double)t_entries)));
   fprintf(fd, "\n");
}
