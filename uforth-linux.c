#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include "uforth.h"
#include "uforth-ext.h"

FILE *OUTFP;

#define CONFIG_IMAGE_FILE "uforth.img"


/*
#define INITIAL_DICT {DICT_VERSION, MAX_DICT_CELLS, 0, 0, 1, {0}}
struct dict ramdict = INITIAL_DICT;
*/
struct dict *dict;
struct timeval start_tv;


uint8_t rxc(void) {
  return getchar();
}


 	
/* A static variable for holding the line. */
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char *
rl_gets ()
{
  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  line_read = readline ("");

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);
  return (line_read);
}

void rxgetline(char* str) {
  fgets(str,128,stdin);
}

void txc(uint8_t c) {
  fputc(c, OUTFP);
  fflush(OUTFP);
}

void txs(char* s, int cnt) {
  fwrite(s,cnt,1,OUTFP);
  fflush(OUTFP);
}
#define txs0(s) txs(s,strlen(s))

static FILE *cfp;
bool config_open_w(char* f) {
  cfp = fopen(f, "w+");
  return (cfp != NULL);
}

bool config_open_r(char* f) {
  cfp = fopen(f, "r");
  return (cfp != NULL);
}

bool config_write(char *src, uint16_t size) {
  fwrite((char*)src, size, 1, cfp);
  return 1;
}

bool config_read(void *dest) {
  uint16_t size;
  fscanf(cfp,"%d\n",(int*)&size);
  fread((char*)dest, size, 1, cfp);
  return 1;
}
bool config_close(void) {
  fclose(cfp);
  return 1;
}

void interpret_from(FILE *fp);


uforth_stat c_handle(void) {
  DCELL r2, r1 = dpop();
  FILE *fp;
  static char buf[80*2];

  switch(r1) {
  case UF_MS:		/* milliseconds */
    {
      struct timeval tv;
      gettimeofday(&tv,0);
      tv.tv_sec -= start_tv.tv_sec;
      tv.tv_usec -= start_tv.tv_usec;
      r2 = (tv.tv_sec * 1000) + (tv.tv_usec/1000);
      dpush(r2);
    }
    break;
  case UF_EMIT:			/* emit */
    txc(dpop()&0xff);
    break;
  case UF_KEY:			/* key */
    dpush((CELL)rxc());
    break;
  case UF_SAVE_IMAGE:			/* save image */
    {
      int dict_size= (dict_here())*sizeof(CELL);
      char *s = uforth_next_word();
      strncpy(buf, s, uforth_iram->tibwordlen+1);
      buf[(uforth_iram->tibwordlen)+1] = '\0';
      printf("Saving raw dictionary into %s\n", buf);
      fp = fopen(buf, "w+");
      fprintf(fp,"%ld\n",(int)(dict_here())*sizeof(CELL));
      fwrite(dict, dict_size,1,fp);
      fclose(fp);

      char *hfile = malloc(strlen(buf) + 3);
      strcpy(hfile,buf);
      strcat(hfile,".h");
      printf("Saving raw dictionary into %s\n", hfile);
      fp = fopen(hfile,"w");
      free(hfile);
      fprintf(fp,"struct dict flashdict = {%d,%d,%d,%d,%d,{\n",
	      dict->version,dict->max_cells,dict->here,dict->last_word_idx,
	      dict->varidx);
      int i;
      for(i = 0; i < dict_size-1; i++) {
	fprintf(fp, "0x%0X,",dict->d[i]);
      }
      fprintf(fp, "%0X",dict->d[dict_size-1]);
      fprintf(fp,"\n}};\n");
      fclose(fp);
    }
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
  case UF_CLOSE:
    dpush(close(dpop()));
    break;
  case UF_OPEN:
    {
      char *s;
      r1 = dpop();
      r2 = dpop();
      s = (char*)&uforth_ram[r2+1];
      strncpy(buf,s, uforth_ram[r2]);
      buf[uforth_ram[r2]] = '\0';
      dpush(open(buf, r1));
    }
    break;
  case UF_INCLUDE:			/* include */
    {
      char *s = uforth_next_word();
      strncpy(buf,s, uforth_iram->tibwordlen+1);
      buf[uforth_iram->tibwordlen] = '\0';
      printf("   Loading %s\n",buf);
      fp = fopen(buf, "r");
      if (fp != NULL) {
	interpret_from(fp);
	fclose(fp);
      } else {
	printf("File not found: <%s>\n", buf);
      }
    }  
    break;

  default:
    return c_ext_handle_cmds(r1);
  }
  return OK;
}


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

#include <sys/types.h>
#include <sys/stat.h>

#define MAX_DICT_CELLS (65535)

int main(int argc, char* argv[]) {

  dict = malloc(sizeof(CELL)*MAX_DICT_CELLS) + sizeof(struct dict);
  dict->version = DICT_VERSION;
  dict->max_cells = MAX_DICT_CELLS;
  dict->here =  0;
  dict->last_word_idx = 0;
  dict->varidx = 1;

  gettimeofday(&start_tv,0);

  uforth_init();

  OUTFP = stdout;
  if (argc < 2) {
    FILE *fp;
    uforth_load_prims();
    printf("   Loading ./init.f\n");
    fp = fopen("./init.f", "r");
    if (fp != NULL) 
      interpret_from(fp);
    fclose(fp);
  } else {
    if (config_open_r(argv[1])) {
      if (!config_read(dict))
	exit(1);
      config_close();
    }
  }

  c_ext_init();
  uforth_interpret("init");
  interpret_from(stdin);

  return 0;
}

