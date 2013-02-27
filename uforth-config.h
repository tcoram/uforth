#ifndef UFORTH_CONFIG_H
#define UFORTH_CONFIG_H
#include <stdint.h>
#include <stdlib.h>

#define INLINE inline
/*
  Be careful with this...
*/
typedef uint16_t CELL;

/* 
   The Dictionary
*/
#define MAX_DICT_CELLS 		(65535)

typedef int32_t DCELL;

/*
  The dictionary is stored as 16 bit ints, so endianess matters.
*/
#define USE_LITTLE_ENDIAN

/*
 Note: A Dictionary CELL is 2 bytes.
*/

// Define this if you want to allow floating point notation (which will
// be converted to FIXED point.
//
#define SUPPORT_FLOAT_FIXED
// This is how we convert our fixed point notation into floating point reality
// and vice versa.
//
#define FIXED_PT_DIVISOR	((double)(1000000.0))
#define FIXED_PT_PLACES		6
#ifdef PC_SIM
#define FIXED_PT_MULT(x)	 round(x*(1000000))
#else
// This is a silly little adjustment for IAR to prevent rounding errors.
//
// #define FIXED_PT_MULT(x)	 ((x+0.0000009)*(1000000))
#define FIXED_PT_MULT(x)	 (x*(1000000))
#endif

/*
 Total user RAM
*/
#define TOTAL_RAM_CELLS		(1024+512) /* 6KB */
#define PAD_SIZE		160  /* bytes */
#define TIB_SIZE  PAD_SIZE
/*
  Task RAM --
*/
#define TASK0_DS_CELLS 		150
#define TASK0_RS_CELLS 		100
#define TASK0_URAM_CELLS	500

/*
 All dictionary writing/updating is captured here.
 If using RAM, then the below macros are probably all you need.
 If using ROM/Flash, then you should either replace with your
 own functions.
*/
#ifdef RAM_DICT
# define dict_start_def()
# define dict_here() dict->here
# define dict_set_last_word(cell) dict->last_word_idx=cell
# define dict_incr_varidx(n) (dict->varidx += n)
# define dict_incr_here(n) dict->here += n
# define dict_append(cell) uforth_dict[dict->here] = cell, dict_incr_here(1)
# define dict_write(idx,cell) uforth_dict[idx] = cell
# define dict_append_string(src,len) { \
    strncpy((char*)((uforth_dict )+(dict->here)),src,len);			\
    dict->here += (len/BYTES_PER_CELL) + (len % BYTES_PER_CELL); \
}
# define dict_end_def()
#endif	/* RAM_DICT */

#endif	/* UFORTH_CONFIG_H */

