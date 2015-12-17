#ifndef UF_UFORTH_EXT_H
#define UF_UFORTH_EXT_H

#define UF_EMIT			1	// emit ( c -- )
#define UF_KEY			2	// key  ( -- c )
#define UF_NUM_TO_STR		3	// n>str ( n -- addr )
#define UF_SAVE_IMAGE		4	// save-image ( -- )
#define UF_INTERP		5	// interp ( addr -- )
#define UF_SUBSTR		6	// substr ( addr start len -- addr )
#define UF_INCLUDE		7	// include ( <filename> -- )
#define UF_OPEN			8	// openfile ( <filename> mode -- fd )
#define UF_CLOSE		9	// closefile ( fd -- )
#define UF_READB		10	// inb ( fd -- c )
#define UF_WRITEB		11	// outb ( c fd -- )
#define UF_MS		        12	// ms ( -- milliseconds )
#define UF_DLSYM		13	// dlsym ( <word> -- addr)
#define UF_DLFFI		14	// dlffi ( p1..p4 addr -- res)
#endif	/* UF_UFORTH_EXT_H */
