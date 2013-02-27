#ifndef UFORTH_H
#define UFORTH_H
/*
  uForth - A tiny ROMable 16/32-bit FORTH-like scripting language
          for microcontrollers.
	  http://maplefish.com/todd/uforth.html
	  Version 1.1

  License for uforth 0.1 and later versions

  Copyright © 2009,2010 Todd Coram, todd@maplefish.com, USA.
  
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "uforth-config.h"
#define UFORTH_VERSION "1.2"
#define DICT_VERSION 0006

typedef char bool;
typedef enum { OK=0, E_NOT_A_WORD, E_STACK_UNDERFLOW, E_RSTACK_OVERFLOW,
	       E_DSTACK_OVERFLOW, E_ABORT, E_EXIT } uforth_stat;

#define BYTES_PER_CELL sizeof(CELL)
#define MAX_CELL_NUM 32767

/*
 The following structures are overlays on pre-allocated buffers.
*/

/*
  iram. 
*/
struct uforth_iram {
  DCELL compiling;		/* 0=interpreting, 1=colondef, 2=compiling */
  DCELL total_ram;		/* Total ram available */
  DCELL tibidx;
  DCELL tibclen;
  DCELL tibwordidx;		/* point to current word in inbufptr */
  DCELL tibwordlen;	/* length of current word in inbufptr */
  char tib[TIB_SIZE];		/* input buffer for interpreter */
};

struct uforth_uram {
  DCELL len;		/* size of URAM */
  DCELL base;			/* numeric base for I/O */
  DCELL didx;			/* data stack index */
  DCELL ridx;			/* return stack index */
  DCELL dsize;			/* size of data stack */
  DCELL rsize;			/* size of return stack */
  DCELL ds[];		/* data & return stack */
};


struct dict {
  CELL version;			/* dictionary version number */
  CELL max_cells;		/* MAX_DICT_CELLS */
  CELL here;			/* top of dictionary */
  CELL last_word_idx;		/* top word's code token (used for searches) */
  CELL varidx;			/* keep track of next variable slot (neg #) */
  CELL d[0];	/* dictionary */
};
#define DICT_HEADER_WORDS	5 /* version + max_cells + here + .. */
#define DICT_INFO_SIZE_BYTES	(sizeof(CELL)*DICT_HEADER_WORDS)


/*
 Pointers to the overlayed structures.
*/
extern CELL *uforth_dict;
extern struct uforth_iram *uforth_iram;
extern struct uforth_uram *uforth_uram;

/*
 Abort reasons.
*/
typedef enum { NO_ABORT=0, ABORT_CTRL_C=1, ABORT_NAW=2,
	       ABORT_ILLEGAL=3, ABORT_WORD=4, ABORT_STACKOVER=5 } abort_t;
extern abort_t _uforth_abort_request;

#define uforth_abort_request(why) _uforth_abort_request = why
#define uforth_aborting() (_uforth_abort_request != NO_ABORT)
#define uforth_abort_reason() _uforth_abort_request
#define uforth_abort_clr() (_uforth_abort_request = NO_ABORT)

char* uforth_count_str(CELL addr,CELL* new_addr);

/*
 RAM dictionaries don't need to do anything special for dictionary
 entry creation.
*/
#ifndef RAM_DICT
CELL dict_here(void);
void dict_load_cache(void);
void dict_start_def(void);
void dict_end_def(void);
void dict_write(CELL idx, CELL cell);
void dict_set_last_word(CELL cell);
CELL dict_incr_varidx(CELL n);
CELL dict_incr_here(CELL cnt);
void dict_append(CELL cell);
void dict_append_string(char* src,CELL len);
#endif


INLINE void dpush(const DCELL w);
INLINE DCELL dpop(void);
INLINE DCELL dpick(const DCELL n);
INLINE uint32_t dpop32(void);
INLINE void dpush32(const uint32_t w2);
INLINE void rpush(const DCELL w);
INLINE DCELL rpop(void);
INLINE DCELL rpick(const DCELL n);

/*
 Convenient short-cuts.
*/
#define dtop() uforth_uram->ds[uforth_uram->didx]
#define rtop() uforth_iram->rs[uforth_uram->ridx]

extern void uforth_init(void);
extern void uforth_load_prims(void);
extern void uforth_abort(void);
extern uforth_stat uforth_interpret(char*);
extern uforth_stat c_handle(void);
char* uforth_next_word (void);

#define IRAM_BYTES (DCELL)(sizeof(struct uforth_iram))/sizeof(DCELL)
#define URAM_HDR_BYTES (DCELL)(sizeof(struct uforth_uram))/sizeof(DCELL)
#define VAR_ALLOT(n) (IRAM_BYTES+URAM_HDR_BYTES+dict_incr_varidx(n))
#define VAR_ALLOT_1() (IRAM_BYTES+URAM_HDR_BYTES+dict_incr_varidx(1))
#define PAD_ADDR (uforth_iram->total_ram - PAD_SIZE)
#define PAD_STR (char*)&uforth_ram[PAD_ADDR+1]
#define PAD_STRLEN uforth_ram[PAD_ADDR]

extern DCELL uforth_ram[];
extern struct dict  *dict;

#ifdef USE_LITTLE_ENDIAN
# define BYTEPACK_FIRST(b) b
# define BYTEPACK_SECOND(b) ((CELL)b)<<8
#else
# define BYTEPACK_FIRST(b) ((CELL)b)<<8
# define BYTEPACK_SECOND(b) b
#endif


/*
 C extension hooks.
*/
void c_ext_init(void);
void c_ext_create_cmds(void);
uforth_stat c_ext_handle_cmds(CELL n);

#endif	/* UFORTH_H */
