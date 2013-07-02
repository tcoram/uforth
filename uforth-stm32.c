#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include "uforth.h"
#include "uforth-ext.h"

#include "uforth.img.h"
struct timeval start_tv;


 	
/* A static variable for holding the line. */
static char line_read[128];

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char * rl_gets () {
  fgets(line_read,128,stdin);
  return line_read;
}

void txc(uint8_t c) {
  fputc(c, stdout);
  fflush(stdout);
}

void txs(char* s, int cnt) {
  fwrite(s,cnt,1,stdout);
  fflush(stdout);
}
#define txs0(s) txs(s,strlen(s))

static char linebuf[128];
char *line;
void interpret_from(FILE *fp) {
  int stat;
  int16_t lineno = 0;
  while (!feof(fp)) {
    ++lineno;
    if (fp == stdin) txs0(" ok\r\n");
    if (fp == stdin) {
      line=rl_gets(); if (line==NULL) exit(0);
    } else {
      if (fgets(linebuf,128,fp) == NULL) break;
      line = linebuf;
    }
    if (line[0] == '\n' || line[0] == '\0') continue;
    stat = uforth_interpret(line);
    switch(stat) {
    case E_NOT_A_WORD:
      fprintf(stdout," line: %d: ", lineno);
      txs0("Huh? >>> ");
      txs(&uforth_iram->tib[uforth_iram->tibwordidx],uforth_iram->tibwordlen);
      txs0(" <<< ");
      txs(&uforth_iram->tib[uforth_iram->tibwordidx + uforth_iram->tibwordlen],
	  uforth_iram->tibclen - 
	  (uforth_iram->tibwordidx + uforth_iram->tibwordlen));
      txs0("\r\n");
      break;
    case E_ABORT:
      txs0("Abort!:<"); txs0(line); txs0(">\n");
      break;
    case E_STACK_UNDERFLOW:
      txs0("Stack underflow!\n");
      break;
    case E_DSTACK_OVERFLOW:
      txs0("Stack overflow!\n");
      break;
    case E_RSTACK_OVERFLOW:
      txs0("Return Stack overflow!\n");
      break;
    case OK:
      break;
    default:
      txs0("Ugh\n");
      break;
    }
  }
}

uforth_stat c_handle(void) {
  DCELL r2, r1 = dpop();

  switch(r1) {
  case UF_MS:		/* milliseconds */
    {
      dpush(0);
    }
    break;
  case UF_EMIT:			/* emit */
    txc(dpop()&0xff);
    break;
  case UF_KEY:			/* key */
    dpush((CELL)rxc());
    break;
  case UF_READB:
    {
      char b;
      r2=dpop();
      read(r2,&b,1);
      dpush(b);
    }
    break;
  case UF_WRITEB:
    {
      char b;
      r2=dpop();
      b=dpop();
      dpush(write(r2,&b,1));
    }
    break;

  default:
    return c_ext_handle_cmds(r1);
  }
  return OK;
}

const struct dict  *dict = &flashdict;

void main() {
  uforth_init();

  c_ext_init();
  uforth_interpret("init");
  interpret_from(stdin);
}

