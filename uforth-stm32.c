#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_conf.h"
#include "utils.h"

#include "uforth.h"
#include "uforth-ext.h"

#include "uforth.img.h"


void init();

void uforth_main();

int main(void) {
  init();
  uforth_main();
  return 0;
}


/*
 * Called from systick handler
 */
volatile uint32_t time_var1, time_var2;
void timing_handler() {
	if (time_var1) {
		time_var1--;
	}

	time_var2++;
}

void init() {
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;

	// ---------- SysTick timer -------- //
	if (SysTick_Config(SystemCoreClock / 1000)) {
		// Capture error
		while (1){};
	}

	// GPIOD Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Configure PD12, PD13, PD14 and PD15 in output pushpull mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	// ------ UART ------ //

	// Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

	// IO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	// Conf
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART3, &USART_InitStructure);

	// Enable
	USART_Cmd(USART3, ENABLE);


	// ---------- DAC ---------- //

	// Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Configuration
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	// IO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Enable DAC Channel1
	DAC_Cmd(DAC_Channel_1, ENABLE);

	// Set DAC Channel1 DHR12L register
	DAC_SetChannel1Data(DAC_Align_12b_R, 0);
}

/*
 * Dummy function to avoid compiler error
 */
void _init() {

}

 	
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
void repl() {
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
  DCELL r1 = dpop();

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
    }
    break;
  case UF_WRITEB:
    {
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
  repl();
}

