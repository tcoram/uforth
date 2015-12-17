{ Automatically generated from uforth-ext.h. Do not EDIT!!! }

: emit ( c -- )   1 cf ;
: key ( -- c )   2 cf ;
: n>str ( n -- addr )   3 cf ;
: save-image ( -- )   4 cf ;
: interp ( addr -- )   5 cf ;
: substr ( addr start len -- addr )   6 cf ;
: include ( <filename> -- )   7 cf ;
: openfile ( <filename> mode -- fd )   8 cf ;
: closefile ( fd -- )   9 cf ;
: inb ( fd -- c )   10 cf ;
: outb ( c fd -- )   11 cf ;
: ms ( -- milliseconds )   12 cf ;
: dlsym ( <word> -- addr)   13 cf ;
: dlffi ( p1..p4 addr -- res)   14 cf ;
