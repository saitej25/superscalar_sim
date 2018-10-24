
/* +-----------------------------------------+ */
/* | Copyright (c) 1990 Stanford University. | */
/* | All Rights Reserved.                    | */
/* |                                         | */
/* | Permission is given to modify this code | */
/* | as long as this notice is not removed.  | */
/* +-----------------------------------------+ */

/* Header file for pixie trace conversion.                       *
*                                                                *
* Adapted and used with the permission of the author, Mike Smith */


#define WORD_READ 0
#define DOUBLE_READ 1
#define WORD_WRITE 2
#define DOUBLE_WRITE 3
#define BYTE_WRITE 4
#define HALF_WRITE 5
#define SWR 6
#define SWL 7
#define LWC1 8
#define LDC1 9
#define SWC1 10
#define SDC1 11
#define BASIC_BLOCK 12
#define ANNUL 13
#define REFTYPES 14

#define IsRead(_x) (((_x)&6)==0)
#define IsWrite(_x) (((_x)&6)!=0)
#define IsFp(_x) (((_x)&8)!=0)
#define IsDouble(_x) (((_x)&5)==1)
#define IsSubword(_x) (((_x)&4)!=0)

struct  ref_struct {
         unsigned   addr : 24;
	 unsigned   type : 4;
	 unsigned   cnt  : 4;
         };

