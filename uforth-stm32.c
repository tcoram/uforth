#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include "stm32f4xx_conf.h"
#include "uforth.h"
#include "uforth-ext.h"

#include "uforth.img.h"


 	
/* A static variable for holding the line. */
static char line_read[128];


char rxc(){
  while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
  return (USART_ReceiveData(USART3));
}

void txc(uint8_t c) {
  USART_SendData(USART3, c);
  while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}
void txs(char* s, int cnt) {
  int i;
  
  for(i = 0;i < cnt;i++) {
    txc((uint8_t)s[i]);
  }
}
#define txs0(s) txs(s,strlen(s))

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char * rl_gets () {
  int i = 0;
  char c;
  while (i < 128) {
    c = rxc();
    txc(c);
    switch(c) {
    case 3:
      txs0(" ^C\n");
      line_read[0] = '\0';
      return line_read;
      break;
    case '\r':
      line_read[i] = '\0';
      txc('\n');
      return line_read;
    case 8:
      if (i > 0) i--;
      break;
    default:
      line_read[i++] = c;
      break;
    }
  }
  line_read[127] = 0;
  return line_read;
}


char *line;
void interpret_from() {
  int stat;
  int16_t lineno = 0;
  while (1) {
    ++lineno;
    txs0(" ok\r\n");
    line=rl_gets(); // if (line==NULL) exit(0);
    if (line[0] == '\r' || line[0] == '\0') continue;
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

struct dict  *dict = &flashdict;

void uforth_main() {
  uforth_init();

  c_ext_init();
  uforth_interpret("init");
  uforth_interpret("cr memory cr");
  interpret_from(stdin);
}

