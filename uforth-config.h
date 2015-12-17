#ifndef UFORTH_CONFIG_H
#define UFORTH_CONFIG_H
#include <stdint.h>
#include <stdlib.h>

#define INLINE

/* 
   The Dictionary: Max is 64K words (64KB * 2)
   Note: A Dictionary CELL is 2 bytes.
*/
#define MAX_DICT_CELLS 		(16384)

/*
  The dictionary is stored as 16 bit ints, so endianess matters.
*/
#define USE_LITTLE_ENDIAN


// Define this if you want to allow floating point notation (which will
// be converted to FIXED point.
//
// #define SUPPORT_FLOAT_FIXED
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
 Total user RAM (each ram cell is 4 bytes)
*/
#define TOTAL_RAM_CELLS		(2048) /* 8KB */
#define PAD_SIZE		160  /* bytes */
#define TIB_SIZE  PAD_SIZE
/*
  Task RAM -- Each cell is 4 bytes. This is a subset of TOTAL_RAM_CELLS
*/
#define TASK0_DS_CELLS 		150
#define TASK0_RS_CELLS 		100
#define TASK0_URAM_CELLS	500


#endif	/* UFORTH_CONFIG_H */

