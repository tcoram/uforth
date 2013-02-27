#include <string.h>
#include "uforth.h"
#include "uforth-config.h"
#include "uforth-ext.h"
#include <time.h>

/* 
   Convenience function to convert 32 bit integers to strings.
*/
char* i32toa(int32_t value, char* str, int32_t base);

void c_ext_init(void) { 
}

/* No additional commands to register */
void c_ext_create_cmds(void) {
}

/*
  All extended commands are handled here.  This is a really long
  switch statement. It could be made shorter by making the contents function
  calls but (outside of a good optimizer) this would incur additional overhead.
  We want to be as fast as possible.

  If you are reading this, but haven't looked at uForth scripts, you are
  missing a lot of context. This is just a bunch of utility calls that will
  make more sense when browsed within the context of a calling script!
  
*/
uforth_stat c_ext_handle_cmds(CELL n) {
  CELL r1, r2,r3;		/* NOTE: THESE ARE 16 bit values! */
  char *str1;

  switch (n) {
  case UF_INTERP:		/* recursively call the uforth interpreter */
    r1 = dpop();
    str1 = uforth_count_str(r1,&r1);
    str1[r1] = '\0';
    dpush(uforth_interpret(str1));
    break;
  case UF_SUBSTR:		/* return a substring of the uForth string */
    r1 = dpop();		/* addr */
    r2 = dpop();		/* length */
    r3 = dpop();		/* start */
    str1 = uforth_count_str(r1,&r1);
    if (r1 < r2) r2 = r1;
    PAD_STRLEN = r2;
    memcpy(PAD_STR, str1 + r3, r2);
    dpush(PAD_ADDR);
    break;
  case UF_NUM_TO_STR:			/* 32bit to string */
    {
      char num[12];
      i32toa(dpop32(),num,uforth_uram->base);
      PAD_STRLEN=strlen(num);
      memcpy(PAD_STR, num, PAD_SIZE);
      dpush(PAD_ADDR);
    }
    break;
  default:
    return E_ABORT;
  }
  return OK;
}
