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

/* Functions used by the simulation algorithms */

#include <stdio.h>

/**************************************************************
Calculates x^y

Input: x, y
Output: x^y
Side effects: None
**************************************************************/

power(x, y)
int x, y;

{  int pwr = 1;

   while (y>0) {
     pwr *= x;
     --y;
   }
   return(pwr);
}


/******************************************************************
  From "Advanced C: tips and techniques", Paul and Gail Anderson

Creates a two dimensional array dynamically.

Input: Row and Column dimensions
Output: A pointer to the two dimensional array space
Side effects: None
******************************************************************/

unsigned **idim2(row, col)
int row, col;

{
   int i;
   register unsigned **prow, *pdata;

   pdata = (unsigned *) calloc (row*col, sizeof (int));
   if (pdata == (unsigned *) NULL) {
     fprintf (stderr, "Problem alocating space\n");
     exit (1);
   }
   prow = (unsigned **) calloc (row, sizeof (int *));
   if (prow == (unsigned **) NULL) {
     fprintf (stderr, "Problem alocating space\n");
     exit (1);
   }
   for (i=0;i<row;i++) {
     prow [i] = pdata;
     pdata += col;
   }

   return(prow);
}



struct hash_table {
        int addr;
        int grptime;
	int prty;
        struct hash_table *nxt;
 };

/***************************************************************
Free list handlers
***************************************************************/

static struct hash_table *head_free_list;

UHT_Add_to_free_list (free_ptr)
struct hash_table *free_ptr;

{
   free_ptr->nxt = head_free_list;
   head_free_list = free_ptr;
}

struct hash_table
*UHT_Get_from_free_list ()

{  struct hash_table *free_ptr;

   if (head_free_list == NULL)
     return(NULL);
   else {
     free_ptr = head_free_list;
     head_free_list = head_free_list->nxt;
     return (free_ptr);
   }
}




 struct tree_node {
        int addr;
	int grpno;
	int prty;
        int rtwt;
        struct tree_node *lft, *rt;
 };  



/******************************************************************
A left rotation. Adapted from the Sleator and Tarjan paper on Splay trees
Makes use of p_stack, setup during the lookup.

Input: Index to entry at which rotation is to be done in p_stack.
Output: None
Side effects: Does a left rotation at the entry
******************************************************************/

rotate_left(y, p_stack)
int y;
struct tree_node **p_stack;

{   int x,z;

    
    z = y-1;
    x = y+1;
    if (z > 0)
         if (p_stack[z]->lft == p_stack[y])
              p_stack[z]->lft = p_stack[x];
         else
              p_stack[z]->rt = p_stack[x];
    p_stack[y]->rt = p_stack[x]->lft;
    p_stack[y]->rtwt -= p_stack[x]->rtwt + 1;
    p_stack[x]->lft = p_stack[y];
    p_stack[y] = p_stack[x];
    p_stack[x] = p_stack[x+1];
}


/******************************************************************
A right rotation. Adapted from the Sleator and Tarjan paper on Splay trees
Makes use of p_stack, setup during the lookup.

Input: Index to entry at which rotation is to be done in p_stack.
Output: None
Side effects: Does a right rotation at the entry
******************************************************************/

rotate_right(y, p_stack)
int y;
struct tree_node **p_stack;

{   int x,z;
    register struct tree_node *t1, *t2, *t3;

    z = y-1;
    x = y+1;
    t1 = p_stack[x];
    t2 = p_stack[y];
    t3 = p_stack[z];
    if (z>0)
         if (t3->lft == t2)
              t3->lft = t1;
         else
              t3->rt = t1;
    t2->lft = t1->rt;
    t1->rt = t2;
    t1->rtwt += t2->rtwt + 1;
    p_stack[y] = t1;
    p_stack[x] = p_stack[x+1];
}


/*********************************************************************
Adapted from the Sleator and Tarjan paper on Splay trees.
Splay the input entry to the top of the stack.
Makes use of p_stack setup during the lookup.

Input: Index to entry to be splayed to root in p_stack.
Output: None
Side effects: Does a spay on the tree
*********************************************************************/


splay(at, p_stack)
int at;
struct tree_node **p_stack;

{   int x, px, gx;
  
    x = at;
    px = at-1;
    gx = at-2;
 
 /* 'at' is a left child */

    if (p_stack[x] == p_stack[px]->lft)
         if (gx == 0)   /* zig */
              rotate_right(1, p_stack);
         else if (p_stack[px] == p_stack[gx]->lft){   /* zig-zig */
                   rotate_right(gx, p_stack);
                   rotate_right(gx, p_stack);
              }
              else{                               /* zig-zag */
                   rotate_right(px, p_stack);
                   rotate_left(gx, p_stack);
              }

 /* 'at' is a right child */

      else if (gx == 0)                              /* zig */
                rotate_left(1, p_stack);
      else if (p_stack[px] == p_stack[gx]->rt){         /* zig-zig */
               rotate_left(gx, p_stack);
               rotate_left(gx, p_stack);
          }
          else{                                   /* zig-zag */
               rotate_left(px, p_stack);
               rotate_right(gx, p_stack);
          }
}        
 
