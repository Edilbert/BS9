/*

*******************
Bit Shift Assembler
*******************

Version: 19-Nov-2023

The assembler was developed and tested on a MAC with macOS Catalina.
Using no specific options of the host system, it should run on any
computer with a standard C-compiler, e.g. Linux, Windows, Amiga OS, etc.

This assembler is a Cross-Assembler, it is run on a (modern) host system,
but produces code for target machines running a Motorola 6809
or a Hitachi 6309 CPU, e.g. Thomson MO and TO series,
Dragon 32/64, Commodore Super PET and Tandy CoCo.


Compiling
=========
If your compiler is named "gcc" for example, compile with:

gcc -o bs9 bs9.c

If you have GNU make and sudo installed, you may also use these lines
to install the binary to /usr/local/bin:

make
sudo make install

Running
=======
If you have a source code named "hello.as9", run the assembler with:

bs9 hello

It will read "hello.as9" as input file and write the
listing file with cross reference "hello.lst".
Binary output is controlled within the source file by means
of the pseudo op "STORE" (see below for syntax):

Case sensitivity
================
mnemonics, and pseudo opcodes are insensitive to case:

LDA lda Lda are all equivalent (Load Accumulator A)

FCB fcb Fcb are all equivalent (define byte data)

Label and named constants are case sensitive by default!
The option "-i" switches off the case sensitivity for symbols.
Also the pseudo op "CASE +/-" may be used to switch sensitivity.

LDA #Cr  and LDA #CR  use different constants!
JMP Lab_10 and JMP LAB_10  jump to different targets!

Directives
==========
CPU = 6809                     allow code for 6809 only
CPU = 6309                     allow full 6309 instruction set (default)

Labels and Constants
====================

LABEL   LDX  #Value            define LABEL for current PC
TXTPTR  = $21b8                define constant TXTPTR
OLDPTR  EQU $21ba              define constant OLDPTR
CURRENT SET 5                  define variable CURRENT

Modules (Subroutines)
=====================

The pseudo instructions

MODULE
...
ENDMOD

or the aliases

SUBROUTINE
...
ENDSUB

define a namespace for local variables.
Variables starting with a '.' (dot) have a scope limited to code
between MODULE and ENDMOD.
Example:

MODULE Delay
.loop  LEAX -1,X
       BNE  .loop
       RTS
ENDMOD

MODULE Strout
.loop  LDA  ,X+
       BEQ  .ret
       JSR  Chrout
       BRA  .loop
.ret   RTS
ENDMOD

There is no conflict in using the label ".loop" twice, because they
are used in separate modules. Internally the assembler generates the names:

Delay.loop
Strout.loop
Strout.ret

for these labels.

Assign addresses to symbols
===========================
LABEL   ENUM value             define label with value
LABEL   ENUM                   use last ENUM value + 1

& = value                      set start value for BSS segment
TXTPTR  BSS 2                  assign TXTPTR = &, & += 2
CURSOR  BSS 1                  assign CURSOR = &, & += 1

Labels and constants can have only one value.
Variables, which get their value assigned with "SET",
may change their values.
Labels that are defined by their current position must start
at the first column.

Examples of pseudo opcodes (directives):
========================================
ORG  $E000                     set program counter
STORE START,$2000,"basic.rom" write binary image file "basic.rom"
STORE START,$2000,"basic.rom",bin,1 write binary image, headed by load address
STORE START,$2000,"basic.s19",s19 write binary file in Motorola S-Record format
STORE START,$2000,"basic.s19",s19,Main write binary and provide execution start address
LOAD  START,"image.bin"        load binary file to START and following addresses
LOAD  "image.bin"              load binary file starting at current address
LIST +                         switch on  assembler listing
LIST -                         switch off assembler listing
BITS . . * . * . . .           stores a byte from 8 bit symbols
BYTE $20,"Example",0           stores a series of byte data
WORD LAB_10, WriteTape,$0200   stores a series of word data
LONG 1000000                   stores 32 bit long data
REAL  3.1415926                stores a 32 bit real
FILL  N ($EA)                  fill memory with N bytes containing $EA
FILL  $A000 - * (0)            fill memory from pc(*) upto $9FFF
INCLUDE "filename"             includes specified file
END                            stops assembly
CASE -                         symbols are not case sensitive
SIZE                           print code size info
TXTTAB BSS 2                   define TXTTAB and increase address pointer by 2
  * = $E000                    set program counter
  & = $033A                    set BSS address pointer
ORG   $E000                    set program counter
SETDP $20                      assume content of direct page register
FCB   $20,"Example",0          stores a series of byte data
FDB   LAB_10, WriteTape,$0200  stores a series of word data
FCC   "Example\n"              store ASCII string

Examples of Operands
====================
    6      = decimal constant
 $A12      = hex constant
MURX       = label or constant
"hello\r"  = ASCII string with CR at end
Table_Offset + 2 * [LEN-1] = address

Constants
=========
'A'         char constant
%1111 0000  bytet constant
?           length of BYTE data line
$ffd2       hex constant

Unary  operators in address calculations
========================================
<    low byte
>    extended address (override DP mode)
(    parenthesis
)    parenthesis
+    positive sign
-    negative sign
!    logical NOT
~    bitwise NOT

Binary operators in address calculations
========================================
+     addition
-     subtraction
*     multiplication or program counter (context sensitive)
/     division
&     bitwise and
|     bitwise or
^     bitwise xor

Relational operators
====================
==    equal
!=    not equal
>     greater than
<     less than
>=    greater than or equal to
<=    less than or equal to
<<    left shift
>>    right shift
&&    and
||    or

Relational operators return the integer 0 (false) or 1 (true).

User macros
===========
Example:

MACRO PrintString(Message)
   LDX   #Message    ; address of message
   LDB   #?Message   ; length of message
   JMP   [SWI1PT]    ; jump through vector
ENDM

defines a MACRO for loading a 16bit word into X and Y

Call:

OK     .BYTE "\nOK\n"

PrintString(OK)

Generated Code:

   LDX #OK
   LDB #4
   JMP   [SWI1PT]

Macros accept up to 10 parameter and may have any length.

Conditional assembly
====================
Example: Assemble first part if C64 has a non zero value

if MO5
   STA $D000
else
   STA $9000
endif

Example: Assemble first part if MO5 is defined ($0000 - $ffff)
(undefined symbols are set to UNDEF ($00ff0000)

ifdef MO5
   STA $D000
else
   STA $9000
endif

assembles the first statement if MO5 is not zero and the second if zero.

Example: Assemble if symbol is undefined

ifndef TO9
   STA $D000 ; Code for MO5 and TO8
endif

Another example:

if MO5 | TO9          ; true if either MO5 or TO9 is true (not zero)
   LDA #MASK
if MO5
   STA ICR_REG
else
   STA TO9_ICR_REG
endif                   ; finishes inner if
endif                   ; finishes outer if

Example: check and force error

if (MAXLEN & $ff00)
   #error This code is 8 bit only, MAXLEN too large!
endif

The maximum nesting depth is 10

For more examples see the complete operating system for the
Thomson MO5 available from the user "Bit Shifter" e.g. at
forum64 or the forum of the VzEkC.

Listing
=======

The program listings lists the original source code preceded by the
generated code in form of hexadecimal bigendian word or byte values.
For example:

  27 9ff6   b6    fe30         LDA     IO_SDCARD
  28 9ff9 1034 8e              ANDR    A,E       ; test IO_INCD#
  29 9ffc   27      06         BEQ     +
  30 9ffe   8e    a096         LDX     #Msg_NOT  ; " not"
  ^  ^    ^    ^  ^            ^       ^         ^
  |  |    |    |  |            |       |         \- Comment
  |  |    |    |  |            |       \----------- Operand
  |  |    |    |  |            \------------------- Mnemonic
  |  |    |    |  \----- Address/Value Operand (Word or Byte)
  |  |    |    \-------- Postbyte (Byte)
  |  |    \------------- Opcode (Word or Byte)
  |  \------------------ Program address
  \--------------------- Line number
*/

// switch off windows warnings for string functions

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

int CPU = 6309; // default: Hitachi 6309

#define MAX_STR 1024

// The array ROM receives the assembled code.
// The size is one page more than 64K because program counter
// overflows are detected after using the new value.
// So references to pc + n do no harm if pc is near the boundary

unsigned char ROM[0x10100]; // binary

// Used to detect overwrite attempts

unsigned char LOCK[0x10100]; // binary

// A two pass assembler must set the instruction length in phase 1
// These values are stored in ADL, in order to avoid phase errors
//  0: no code generated for this address
// >0: instruction length locked
// <0: data byte or not start byte of instruction

int ErrNum;          // error count
int ListOn = 1;      // listing control

FILE *sf; // source       file
FILE *lf; // listing      file
FILE *df; // debug        file
FILE *pf; // preprocessed file
FILE *of; // object       file

// forward references

void AddLabel(char *p);
void ErrorLine(char *p);
void ErrorMsg(const char *format, ...);
char *ExtractOpText(char *);
char *EvalOperand(char *, int *, int);
char *ExtractValue(char *, int *);

// store code or data into ROM array

void Put(int i, int v, char *p)
{
   if (df) fprintf(df,"LOCK[%4.4x]=%x  ROM[%4.4x]=%x  v=%4.4x\n",
                      i,LOCK[i],i,ROM[i],v);
   v &= 0xff;
   if (LOCK[i] && ROM[i] != v)
   {
      ++ErrNum;
      if (p) ErrorLine(p);
      ErrorMsg("Tried to overwrite address %4.4x\n",i);
      exit(1);
   }
   ROM[i]  = v;
   LOCK[i] = 1;
}

// **********
// StrCaseStr
// **********

char *StrCaseStr(char *s1, const char *s2)
{
   char h1[MAX_STR];
   char h2[MAX_STR];
   char *r;
   unsigned int i;

   memset(h1,0,sizeof(h1));
   memset(h2,0,sizeof(h2));

    for (i=0 ; i < strlen(s1) && i < sizeof(h1)-1 ; ++i)
        h1[i] = toupper(s1[i]);
    for (i=0 ; i < strlen(s2) && i < sizeof(h2)-1 ; ++i)
        h2[i] = toupper(s2[i]);

    r = strstr(h1,h2);
    if (r) r = s1 + (r - h1);
    return r;
}

// ***********
// AssertAlloc
// ***********

void *AssertAlloc(void *p)
{
   if (p) return p;
   fprintf(stderr, "Allocation of memory failed.\n");
   exit(1);
}

// ***********
// MallocOrDie
// ***********

void *MallocOrDie(size_t size)
{
   return AssertAlloc(calloc(size,1));
}

// ************
// ReallocOrDie
// ************

void *ReallocOrDie(void *p, size_t size)
{
   return AssertAlloc(realloc(p, size));
}

// ************
// AssertFileOp
// ************

FILE *AssertFileOp(FILE *p, const char *msg)
{
   if (p) return p;
   perror(msg);
   exit(1);
}

// *******
// StrNDup
// *******

char *StrNDup(void *src, unsigned int n)
{
   char *dst;
   if (n > MAX_STR)
   {
      fprintf(stderr,"*** tried to allocate %d bytes for string\n",n);
      fprintf(stderr,"*** current maximum length is %d\n",MAX_STR);
      exit(1);
   }
   dst = (char *)MallocOrDie(n+1);
   memmove(dst,src,n);
   return dst;
 }

#define ADMODES 8

enum Addressing_Mode
{
   AM_None        , // 0
   AM_Inherent    , // 1
   AM_Register    , // 2
   AM_Relative    , // 3
   AM_Immediate   , // 4
   AM_Direct      , // 5
   AM_Indexed     , // 6
   AM_Extended      // 7
};

struct MatStruct
{
   char Mne[6];       // Mnemonic
   int  Opc[ADMODES]; // Opcodes
} Mat[] =
{
//             0       1      2      3      4      5      6      7
//   Mnem    CPU   Inher    Reg   Rela    Imm Direct    Ind    Ext
// ---------------------------------------------------------------
   {"NEG"     ,{0,    -1,    -1,    -1,    -1,  0x00,  0x60,  0x70}},
   {"COM"     ,{0,    -1,    -1,    -1,    -1,  0x03,  0x63,  0x73}},
   {"LSR"     ,{0,    -1,    -1,    -1,    -1,  0x04,  0x64,  0x74}},
   {"ROR"     ,{0,    -1,    -1,    -1,    -1,  0x06,  0x66,  0x76}},
   {"ASR"     ,{0,    -1,    -1,    -1,    -1,  0x07,  0x67,  0x77}},
   {"ASL"     ,{0,    -1,    -1,    -1,    -1,  0x08,  0x68,  0x78}},
   {"LSL"     ,{0,    -1,    -1,    -1,    -1,  0x08,  0x68,  0x78}},
   {"ROL"     ,{0,    -1,    -1,    -1,    -1,  0x09,  0x69,  0x79}},
   {"DEC"     ,{0,    -1,    -1,    -1,    -1,  0x0a,  0x6a,  0x7a}},
   {"INC"     ,{0,    -1,    -1,    -1,    -1,  0x0c,  0x6c,  0x7c}},
   {"TST"     ,{0,    -1,    -1,    -1,    -1,  0x0d,  0x6d,  0x7d}},
   {"JMP"     ,{0,    -1,    -1,    -1,    -1,  0x0e,  0x6e,  0x7e}},
   {"CLR"     ,{0,    -1,    -1,    -1,    -1,  0x0f,  0x6f,  0x7f}},
   {"NOP"     ,{0,  0x12,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"SYNC"    ,{0,  0x13,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LBRA"    ,{0,    -1,    -1,  0x16,    -1,    -1,    -1,    -1}},
   {"LBSR"    ,{0,    -1,    -1,  0x17,    -1,    -1,    -1,    -1}},
   {"DAA"     ,{0,  0x19,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ORCC"    ,{0,    -1,    -1,    -1,  0x1a,    -1,    -1,    -1}},
   {"ANDCC"   ,{0,    -1,    -1,    -1,  0x1c,    -1,    -1,    -1}},
   {"SEX"     ,{0,  0x1d,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"EXG"     ,{0,    -1,  0x1e,    -1,    -1,    -1,    -1,    -1}},
   {"TFR"     ,{0,    -1,  0x1f,    -1,    -1,    -1,    -1,    -1}},
   {"BRA"     ,{0,    -1,    -1,  0x20,    -1,    -1,    -1,    -1}},
   {"BRN"     ,{0,    -1,    -1,  0x21,    -1,    -1,    -1,    -1}},
   {"BHI"     ,{0,    -1,    -1,  0x22,    -1,    -1,    -1,    -1}},
   {"BLS"     ,{0,    -1,    -1,  0x23,    -1,    -1,    -1,    -1}},
   {"BCC"     ,{0,    -1,    -1,  0x24,    -1,    -1,    -1,    -1}},
   {"BHS"     ,{0,    -1,    -1,  0x24,    -1,    -1,    -1,    -1}},
   {"BCS"     ,{0,    -1,    -1,  0x25,    -1,    -1,    -1,    -1}},
   {"BLO"     ,{0,    -1,    -1,  0x25,    -1,    -1,    -1,    -1}},
   {"BNE"     ,{0,    -1,    -1,  0x26,    -1,    -1,    -1,    -1}},
   {"BEQ"     ,{0,    -1,    -1,  0x27,    -1,    -1,    -1,    -1}},
   {"BVC"     ,{0,    -1,    -1,  0x28,    -1,    -1,    -1,    -1}},
   {"BVS"     ,{0,    -1,    -1,  0x29,    -1,    -1,    -1,    -1}},
   {"BPL"     ,{0,    -1,    -1,  0x2a,    -1,    -1,    -1,    -1}},
   {"BMI"     ,{0,    -1,    -1,  0x2b,    -1,    -1,    -1,    -1}},
   {"BGE"     ,{0,    -1,    -1,  0x2c,    -1,    -1,    -1,    -1}},
   {"BLT"     ,{0,    -1,    -1,  0x2d,    -1,    -1,    -1,    -1}},
   {"BGT"     ,{0,    -1,    -1,  0x2e,    -1,    -1,    -1,    -1}},
   {"BLE"     ,{0,    -1,    -1,  0x2f,    -1,    -1,    -1,    -1}},
   {"LEAX"    ,{0,    -1,    -1,    -1,    -1,    -1,  0x30,    -1}},
   {"LEAY"    ,{0,    -1,    -1,    -1,    -1,    -1,  0x31,    -1}},
   {"LEAS"    ,{0,    -1,    -1,    -1,    -1,    -1,  0x32,    -1}},
   {"LEAU"    ,{0,    -1,    -1,    -1,    -1,    -1,  0x33,    -1}},
   {"PSHS"    ,{0,    -1,  0x34,    -1,    -1,    -1,    -1,    -1}},
   {"PULS"    ,{0,    -1,  0x35,    -1,    -1,    -1,    -1,    -1}},
   {"PSHU"    ,{0,    -1,  0x36,    -1,    -1,    -1,    -1,    -1}},
   {"PULU"    ,{0,    -1,  0x37,    -1,    -1,    -1,    -1,    -1}},
   {"RTS"     ,{0,  0x39,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ABX"     ,{0,  0x3a,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"RTI"     ,{0,  0x3b,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CWAI"    ,{0,    -1,    -1,    -1,  0x3c,    -1,    -1,    -1}},
   {"MUL"     ,{0,  0x3d,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"RESET"   ,{0,  0x3e,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"SWI"     ,{0,  0x3f,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"NEGA"    ,{0,  0x40,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"COMA"    ,{0,  0x43,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LSRA"    ,{0,  0x44,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"RORA"    ,{0,  0x46,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ASRA"    ,{0,  0x47,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ASLA"    ,{0,  0x48,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LSLA"    ,{0,  0x48,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ROLA"    ,{0,  0x49,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"DECA"    ,{0,  0x4a,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"INCA"    ,{0,  0x4c,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"TSTA"    ,{0,  0x4d,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CLRA"    ,{0,  0x4f,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"NEGB"    ,{0,  0x50,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"COMB"    ,{0,  0x53,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LSRB"    ,{0,  0x54,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"RORB"    ,{0,  0x56,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ASRB"    ,{0,  0x57,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ASLB"    ,{0,  0x58,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LSLB"    ,{0,  0x58,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ROLB"    ,{0,  0x59,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"DECB"    ,{0,  0x5a,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"INCB"    ,{0,  0x5c,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"TSTB"    ,{0,  0x5d,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CLRB"    ,{0,  0x5f,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"SUBA"    ,{0,    -1,    -1,    -1,  0x80,  0x90,  0xa0,  0xb0}},
   {"CMPA"    ,{0,    -1,    -1,    -1,  0x81,  0x91,  0xa1,  0xb1}},
   {"SBCA"    ,{0,    -1,    -1,    -1,  0x82,  0x92,  0xa2,  0xb2}},
   {"SUBD"    ,{0,    -1,    -1,    -1,  0x83,  0x93,  0xa3,  0xb3}},
   {"ANDA"    ,{0,    -1,    -1,    -1,  0x84,  0x94,  0xa4,  0xb4}},
   {"BITA"    ,{0,    -1,    -1,    -1,  0x85,  0x95,  0xa5,  0xb5}},
   {"LDA"     ,{0,    -1,    -1,    -1,  0x86,  0x96,  0xa6,  0xb6}},
   {"EORA"    ,{0,    -1,    -1,    -1,  0x88,  0x98,  0xa8,  0xb8}},
   {"ADCA"    ,{0,    -1,    -1,    -1,  0x89,  0x99,  0xa9,  0xb9}},
   {"ORA"     ,{0,    -1,    -1,    -1,  0x8a,  0x9a,  0xaa,  0xba}},
   {"ADDA"    ,{0,    -1,    -1,    -1,  0x8b,  0x9b,  0xab,  0xbb}},
   {"CMPX"    ,{0,    -1,    -1,    -1,  0x8c,  0x9c,  0xac,  0xbc}},
   {"BSR"     ,{0,    -1,    -1,  0x8d,    -1,    -1,    -1,    -1}},
   {"LDX"     ,{0,    -1,    -1,    -1,  0x8e,  0x9e,  0xae,  0xbe}},
   {"STA"     ,{0,    -1,    -1,    -1,    -1,  0x97,  0xa7,  0xb7}},
   {"JSR"     ,{0,    -1,    -1,    -1,    -1,  0x9d,  0xad,  0xbd}},
   {"STX"     ,{0,    -1,    -1,    -1,    -1,  0x9f,  0xaf,  0xbf}},
   {"SUBB"    ,{0,    -1,    -1,    -1,  0xc0,  0xd0,  0xe0,  0xf0}},
   {"CMPB"    ,{0,    -1,    -1,    -1,  0xc1,  0xd1,  0xe1,  0xf1}},
   {"SBCB"    ,{0,    -1,    -1,    -1,  0xc2,  0xd2,  0xe2,  0xf2}},
   {"ADDD"    ,{0,    -1,    -1,    -1,  0xc3,  0xd3,  0xe3,  0xf3}},
   {"ANDB"    ,{0,    -1,    -1,    -1,  0xc4,  0xd4,  0xe4,  0xf4}},
   {"BITB"    ,{0,    -1,    -1,    -1,  0xc5,  0xd5,  0xe5,  0xf5}},
   {"LDB"     ,{0,    -1,    -1,    -1,  0xc6,  0xd6,  0xe6,  0xf6}},
   {"EORB"    ,{0,    -1,    -1,    -1,  0xc8,  0xd8,  0xe8,  0xf8}},
   {"ADCB"    ,{0,    -1,    -1,    -1,  0xc9,  0xd9,  0xe9,  0xf9}},
   {"ORB"     ,{0,    -1,    -1,    -1,  0xca,  0xda,  0xea,  0xfa}},
   {"ADDB"    ,{0,    -1,    -1,    -1,  0xcb,  0xdb,  0xeb,  0xfb}},
   {"LDD"     ,{0,    -1,    -1,    -1,  0xcc,  0xdc,  0xec,  0xfc}},
   {"LDU"     ,{0,    -1,    -1,    -1,  0xce,  0xde,  0xee,  0xfe}},
   {"STB"     ,{0,    -1,    -1,    -1,    -1,  0xd7,  0xe7,  0xf7}},
   {"STD"     ,{0,    -1,    -1,    -1,    -1,  0xdd,  0xed,  0xfd}},
   {"STU"     ,{0,    -1,    -1,    -1,    -1,  0xdf,  0xef,  0xff}},
   {"LBRN"    ,{0,    -1,    -1,0x1021,    -1,    -1,    -1,    -1}},
   {"LBHI"    ,{0,    -1,    -1,0x1022,    -1,    -1,    -1,    -1}},
   {"LBLS"    ,{0,    -1,    -1,0x1023,    -1,    -1,    -1,    -1}},
   {"LBCC"    ,{0,    -1,    -1,0x1024,    -1,    -1,    -1,    -1}},
   {"LBHS"    ,{0,    -1,    -1,0x1024,    -1,    -1,    -1,    -1}},
   {"LBCS"    ,{0,    -1,    -1,0x1025,    -1,    -1,    -1,    -1}},
   {"LBLO"    ,{0,    -1,    -1,0x1025,    -1,    -1,    -1,    -1}},
   {"LBNE"    ,{0,    -1,    -1,0x1026,    -1,    -1,    -1,    -1}},
   {"LBEQ"    ,{0,    -1,    -1,0x1027,    -1,    -1,    -1,    -1}},
   {"LBVC"    ,{0,    -1,    -1,0x1028,    -1,    -1,    -1,    -1}},
   {"LBVS"    ,{0,    -1,    -1,0x1029,    -1,    -1,    -1,    -1}},
   {"LBPL"    ,{0,    -1,    -1,0x102a,    -1,    -1,    -1,    -1}},
   {"LBMI"    ,{0,    -1,    -1,0x102b,    -1,    -1,    -1,    -1}},
   {"LBGE"    ,{0,    -1,    -1,0x102c,    -1,    -1,    -1,    -1}},
   {"LBLT"    ,{0,    -1,    -1,0x102d,    -1,    -1,    -1,    -1}},
   {"LBGT"    ,{0,    -1,    -1,0x102e,    -1,    -1,    -1,    -1}},
   {"LBLE"    ,{0,    -1,    -1,0x102f,    -1,    -1,    -1,    -1}},
   {"SWI2"    ,{0,0x103f,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CMPD"    ,{0,    -1,    -1,    -1,0x1083,0x1093,0x10a3,0x10b3}},
   {"CMPY"    ,{0,    -1,    -1,    -1,0x108c,0x109c,0x10ac,0x10bc}},
   {"LDY"     ,{0,    -1,    -1,    -1,0x108e,0x109e,0x10ae,0x10be}},
   {"STY"     ,{0,    -1,    -1,    -1,    -1,0x109f,0x10af,0x10bf}},
   {"LDS"     ,{0,    -1,    -1,    -1,0x10ce,0x10de,0x10ee,0x10fe}},
   {"STS"     ,{0,    -1,    -1,    -1,    -1,0x10df,0x10ef,0x10ff}},
   {"SWI3"    ,{0,0x113f,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CMPU"    ,{0,    -1,    -1,    -1,0x1183,0x1193,0x11a3,0x11b3}},
   {"CMPS"    ,{0,    -1,    -1,    -1,0x118c,0x119c,0x11ac,0x11bc}},

//   6309      0       1      2      3      4      5      6      7
//   Mnem    CPU   Inher    Reg   Rela    Imm Direct    Ind    Ext
// ---------------------------------------------------------------
   {"SEXW"    ,{1,  0x14,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ADDR"    ,{1,    -1,0x1030,    -1,    -1,    -1,    -1,    -1}},
   {"ADCR"    ,{1,    -1,0x1031,    -1,    -1,    -1,    -1,    -1}},
   {"SUBR"    ,{1,    -1,0x1032,    -1,    -1,    -1,    -1,    -1}},
   {"SBCR"    ,{1,    -1,0x1033,    -1,    -1,    -1,    -1,    -1}},
   {"ANDR"    ,{1,    -1,0x1034,    -1,    -1,    -1,    -1,    -1}},
   {"ORR"     ,{1,    -1,0x1035,    -1,    -1,    -1,    -1,    -1}},
   {"EORR"    ,{1,    -1,0x1036,    -1,    -1,    -1,    -1,    -1}},
   {"CMPR"    ,{1,    -1,0x1037,    -1,    -1,    -1,    -1,    -1}},
   {"TFM"     ,{1,    -1,0x1138,    -1,    -1,    -1,    -1,    -1}},
   {"BITMD"   ,{1,    -1,    -1,    -1,0x113c,    -1,    -1,    -1}},
   {"LDMD"    ,{1,    -1,    -1,    -1,0x113d,    -1,    -1,    -1}},
   {"PSHSW"   ,{1,0x1038,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"PULSW"   ,{1,0x1039,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"PSHUW"   ,{1,0x103A,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"PULUW"   ,{1,0x103B,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"NEGD"    ,{1,0x1040,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"COMD"    ,{1,0x1043,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LSRD"    ,{1,0x1044,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"RORD"    ,{1,0x1046,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ASRD"    ,{1,0x1047,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ASLD"    ,{1,0x1048,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LSLD"    ,{1,0x1048,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ROLD"    ,{1,0x1049,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"DECD"    ,{1,0x104A,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"INCD"    ,{1,0x104C,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"TSTD"    ,{1,0x104D,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CLRD"    ,{1,0x104F,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"COMW"    ,{1,0x1053,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"LSRW"    ,{1,0x1054,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"RORW"    ,{1,0x1056,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"ROLW"    ,{1,0x1059,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"DECW"    ,{1,0x105A,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"INCW"    ,{1,0x105C,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"TSTW"    ,{1,0x105D,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CLRW"    ,{1,0x105F,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"COME"    ,{1,0x1143,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"DECE"    ,{1,0x114A,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"INCE"    ,{1,0x114C,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"TSTE"    ,{1,0x114D,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CLRE"    ,{1,0x114F,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"COMF"    ,{1,0x1153,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"DECF"    ,{1,0x115A,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"INCF"    ,{1,0x115C,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"TSTF"    ,{1,0x115D,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"CLRF"    ,{1,0x115F,    -1,    -1,    -1,    -1,    -1,    -1}},
   {"OIM"     ,{1,    -1,    -1,    -1,    -1,  0x01,  0x61,  0x71}},
   {"AIM"     ,{1,    -1,    -1,    -1,    -1,  0x02,  0x62,  0x72}},
   {"EIM"     ,{1,    -1,    -1,    -1,    -1,  0x05,  0x65,  0x75}},
   {"TIM"     ,{1,    -1,    -1,    -1,    -1,  0x0b,  0x6b,  0x7b}},
   {"STW"     ,{1,    -1,    -1,    -1,    -1,0x1097,0x10a7,0x10b7}},
   {"STQ"     ,{1,    -1,    -1,    -1,    -1,0x10dd,0x10ed,0x10fd}},
   {"STE"     ,{1,    -1,    -1,    -1,    -1,0x1197,0x11a7,0x11b7}},
   {"STF"     ,{1,    -1,    -1,    -1,    -1,0x11d7,0x11e7,0x11f7}},
   {"LDQ"     ,{1,    -1,    -1,    -1,  0xcd,0x10dc,0x10ec,0x10fc}},
   {"SUBW"    ,{1,    -1,    -1,    -1,0x1080,0x1090,0x10a0,0x10b0}},
   {"CMPW"    ,{1,    -1,    -1,    -1,0x1081,0x1091,0x10a1,0x10b1}},
   {"SBCD"    ,{1,    -1,    -1,    -1,0x1082,0x1092,0x10a2,0x10b2}},
   {"ANDD"    ,{1,    -1,    -1,    -1,0x1084,0x1094,0x10a4,0x10b4}},
   {"BITD"    ,{1,    -1,    -1,    -1,0x1085,0x1095,0x10a5,0x10b5}},
   {"LDW"     ,{1,    -1,    -1,    -1,0x1086,0x1096,0x10a6,0x10b6}},
   {"EORD"    ,{1,    -1,    -1,    -1,0x1088,0x1098,0x10a8,0x10b8}},
   {"ADCD"    ,{1,    -1,    -1,    -1,0x1089,0x1099,0x10a9,0x10b9}},
   {"ORD"     ,{1,    -1,    -1,    -1,0x108a,0x109a,0x10aa,0x10ba}},
   {"ADDW"    ,{1,    -1,    -1,    -1,0x108b,0x109b,0x10ab,0x10bb}},
   {"SUBE"    ,{1,    -1,    -1,    -1,0x1180,0x1190,0x11a0,0x11b0}},
   {"CMPE"    ,{1,    -1,    -1,    -1,0x1181,0x1191,0x11a1,0x11b1}},
   {"LDE"     ,{1,    -1,    -1,    -1,0x1186,0x1196,0x11a6,0x11b6}},
   {"ADDE"    ,{1,    -1,    -1,    -1,0x118b,0x119b,0x11ab,0x11bb}},
   {"DIVD"    ,{1,    -1,    -1,    -1,0x118d,0x119d,0x11ad,0x11bd}},
   {"DIVQ"    ,{1,    -1,    -1,    -1,0x118e,0x119e,0x11ae,0x11be}},
   {"MULD"    ,{1,    -1,    -1,    -1,0x118f,0x119f,0x11af,0x11bf}},
   {"SUBF"    ,{1,    -1,    -1,    -1,0x11c0,0x11d0,0x11e0,0x11f0}},
   {"CMPF"    ,{1,    -1,    -1,    -1,0x11c1,0x11d1,0x11e1,0x11f1}},
   {"LDF"     ,{1,    -1,    -1,    -1,0x11c6,0x11d6,0x11e6,0x11f6}},
   {"ADDF"    ,{1,    -1,    -1,    -1,0x11cb,0x11db,0x11eb,0x11fb}},
   {"BAND"    ,{1,    -1,    -1,    -1,    -1,0x1130,    -1,    -1}},
   {"BIAND"   ,{1,    -1,    -1,    -1,    -1,0x1131,    -1,    -1}},
   {"BOR"     ,{1,    -1,    -1,    -1,    -1,0x1132,    -1,    -1}},
   {"BIOR"    ,{1,    -1,    -1,    -1,    -1,0x1133,    -1,    -1}},
   {"BEOR"    ,{1,    -1,    -1,    -1,    -1,0x1134,    -1,    -1}},
   {"BIEOR"   ,{1,    -1,    -1,    -1,    -1,0x1135,    -1,    -1}},
   {"LDBT"    ,{1,    -1,    -1,    -1,    -1,0x1136,    -1,    -1}},
   {"STBT"    ,{1,    -1,    -1,    -1,    -1,0x1137,    -1,    -1}}
 };

#define DIMOP_6809 139
#define DIMOP_6309 (sizeof(Mat) / sizeof(struct MatStruct))

int DimOp = DIMOP_6309;

// register Q is not included, it appears never as operand
// and is only part of the mnemonic

const char *Register_6309[] =
{
//  0   1   2   3   4   5    6   7   8   9    A    B   C   D   E   F
   "D","X","Y","U","S","PC","W","V","A","B","CC","DP","*","0","E","F"
};

const char *Register_6809[] =
{
//  0   1   2   3   4   5    6   7   8   9    A    B   C   D   E   F
   "D","X","Y","U","S","PC","-","-","A","B","CC","DP","*","*","-","-"
};

const char **RegisterNames = Register_6309;

struct PushStruct
{
   char Reg[3];
   int  Val;
} PushList[10] =
{
   {"CC" , 0x01}, // bit 0
   {"A"  , 0x02}, // bit 1
   {"B"  , 0x04}, // bit 2
   {"D"  , 0x06}, // bit 1 and 2
   {"DP" , 0x08}, // bit 3
   {"X"  , 0x10}, // bit 4
   {"Y"  , 0x20}, // bit 5
   {"S"  , 0x40}, // bit 6
   {"U"  , 0x40}, // bit 6
   {"PC" , 0x80}  // bit 7
};


#define UNDEF (int) 0xff0000

int SkipHex    =  0; // switch on with -x
int Debug      =  0; // switch on with -d
int LiNo       =  0; // line number of current file
int WithLiNo   =  0; // print line numbers in listing if set
int TotalLiNo  =  0; // total line number
int Preprocess =  0; // print preprocessed source file <file.pp>
int Quiet      =  0; // switch for quiet mode
int ERRMAX     = 10; // stop assemby after ERRMAX errors
int EnumValue  = -1; // last used ENUM value
int MacLev;          // macro nesting level
int ModuleStart;     // address of a module
int Optimize;        // branch and jump omtimization
int FormLn;          // lines per page [inactive]
int DP;              // current direct page
int CodeStyle;       // 1: operand has no spaces (old form)
int MneIndex;        // current mnemonic

int oc;              // op code
int pb;              // post byte
int am;              // address mode
int il;              // instruction length
int ol;              // opcode length
int pl;              // postbyte length
int ql;              // operand length
int pc = -1;         // program counter
int bss;             // bss counter
int nops;            // snychronisation nops

int Phase;           // phase or pass of 2-pass assembler
int IfLevel;         // if nesting level
int Skipping;        // inside 'false' branch
int SkipLine[10];    // skipping value for each nesting level
int ForcedEnd;       // Triggered by END command
int IgnoreCase;      // 1: Ignore case for symbols
int ForcedMode;      // -1: direct page, +1: extended
int optc;            // count optimization messages
int Preset;          // value for initialisation

// local labels

#define PLUMAX 200

int minlab[11];
int plucnt[11];
int plulab[11][PLUMAX];

// Filenames

#define FNSIZE 256

char *Src;           // source file
char  Lst[FNSIZE];   // list file
char  Pre[FNSIZE];   // preprocessed file
char  Opt[FNSIZE];   // optimzation hints

int GenStart = 0x10000 ; //  Lowest assemble address
int GenEnd   =       0 ; // Highest assemble address

// These arrays hold the parameter for storage files

#define SFMAX 20
int SFA[SFMAX];      // start address of data block
int SFL[SFMAX];      // length of data block
char *SFF[SFMAX];    // filename
int SFE[SFMAX];      // execution start address
int SFR[SFMAX];      // number of records in a S19 file
int SFT[SFMAX];      // file format

int StoreCount = 0;  // number of segments to store

enum OutfileFormat { BINARY, SRECORD };

signed char ADL[0x10000];

// organize nesting of include files

struct IncludeStackStruct
{
   FILE *fp;
   int   LiNo;
   char *Src;
} IncludeStack[100];

int IncludeLevel;

#define ML 256

int ArgPtr[10];              // macro argument pointer
char Line[ML];               // source line
char Label[ML];              // current label
char MacArgs[ML];            // macro arguments
unsigned char Operand[ML];   // binary operand
char OpText[ML];             // operand source
char Comment[ML];            // comment source
char Hint[ML];               // optimization hints
char Scope[ML];              // for local symbols
char datebuffer [80];

// state of label definition
// defined or BSS or defined by position

#define LDEF 1
#define LBSS 2
#define LPOS 3

#define MAXLAB 8000

struct LabelStruct
{
   char *Name;     // Label name - case sensitive
   int   Address;  // Range 0 - 65536
   int   Bytes;    // Length of object (string for example)
   int   Locked;   // Cannot change value
   int   NumRef;   // # of references
   int  *Ref;      // list of references
   int  *Att;      // list of attributes
} lab[MAXLAB];

int Labels;        // number of labels

// maximum number of macros

#define MAXMAC 200

// special character in macros used to mark argument

#define CHAMAC '`'

struct MacroStruct
{
   char *Name;  // MACRO Name(arg,arg,...) (up to 10 arguments)
   char *Body;  // "Line1\nLine2\n ... LastLine\n"
   int  Narg;   // # of macro arguments (0-10)
   int  Cola;   // column of macro definition (for pretty printing)
   int  Type;   // 0: name(arg1,arg2))  1: name arg1,arg2
} Mac[MAXMAC];

char *MacPtr[MAXMAC]; // pointer inside macro body
int Macros;           // total number of macros

char Cstat[26];

void MneStat(void)
{
   unsigned int i,j;

   for (i=0 ; i < DIMOP_6309 ; ++i)
   for (j=0 ; j < strlen(Mat[i].Mne) ; ++j)
      ++Cstat[Mat[i].Mne[j]-'A'];

   for (i=0 ; i < 26 ; ++i)
      printf("%c:%3d\n",'A'+i,Cstat[i]);
}

// *********
// SkipSpace
// *********

char *SkipSpace(char *p)
{
   if (*p) while (isspace(*p)) ++p;
   return p;
}

char *StrMatch(char *s, const char *m)
{
   int i,j,k,l;

   k = strlen(s);
   l = strlen(m);

   for (j = 0 ; j <= k-l ; ++j)
   {
      for (i = 0 ; i < l ; ++i)
      {
         if (toupper(s[j+i]) != toupper(m[i])) break;
      }
      if (i == l) return (s + j);
   }
   return NULL;
}

// Check if keyword (m) is in string (s)
// Make sure, it's not part of a larger word
// by checking the characters before and after

// ******
// StrKey
// ******

char *StrKey(char *s, const char *m)
{
   char *r = StrMatch(s,m);
   if (r) // match was true
   {
      if ((r > s && isalnum(*(r-1))) ||       // check before
         (isalnum(*(r+strlen(m))))) r = NULL; // check after
   }
   return r;
}

// ****
// isym
// ****

int isym(char c)
{
   return (c == '.' || c == '$' || c == '_' || isalnum(c));
}


// *********
// GetSymbol
// *********

char *GetSymbol(char *p, char *s)
{
   if (*p == '.') // expand local symbol
   {
      if (Scope[0])
      {
         strcpy(s,Scope);
         s += strlen(s);
         *s++ = *p++;
      }
   }
   if (*p == '_' || isalpha(*p)) while (isym(*p)) *s++ = *p++;
   *s = 0;
   return p;
}

// ***********
// GetMacroArg
// ***********

char *GetMacroArg(char *p, char *s)
{
   p = SkipSpace(p);
   while (*p && *p != ',' && *p != ')') *s++ = *p++;
   *s = 0;
   return p;
}


char *NextSymbol(char *p, char *s)
{
   p = SkipSpace(p);
   p = GetSymbol(p,s);
   return p;
}

// Compare two strings ignoring case

int StrCaseCmp(const char *a, const char *b)
{
   unsigned int i,l,m;

   l = strlen(a);
   m = strlen(b);

   for (i=0 ; i <= l && i <= m ; ++i)
   {
      if (toupper(a[i]) < toupper(b[i])) return -1;
      if (toupper(a[i]) > toupper(b[i])) return  1;
   }
   return 0;
}

// Compare two strings ignoring case of max. length n

int StrNCaseCmp(const char *a, const char *b, unsigned int n)
{
   unsigned int i,l,m;

   l = strlen(a);
   m = strlen(b);

   for (i=0 ; i < n && i <= l && i <= m ; ++i)
   {
      if (toupper(a[i]) < toupper(b[i])) return -1;
      if (toupper(a[i]) > toupper(b[i])) return  1;
   }
   return 0;
}

// Check if string s2 is a word in s1
// and search string is followed by a white space or symbol

int strcmpword(const char *s1, const char *s2)
{
   int l = strlen(s2);
   int r = StrNCaseCmp(s1,s2,l);
   if (r == 0 && isym(s1[l])) r = 1;
   return r;
}

char *SkipHexCode(char *p)
{
   int l;

   l = strlen(p);
   if (l > 20 && isdigit(p[4]) && isspace(p[5]) &&
       isxdigit(p[6]) && isxdigit(p[7]) &&
       isxdigit(p[8]) && isxdigit(p[9]) &&
       p[0] != ';')
   {
      if (SkipHex) memmove(Line,Line+20,l-19);
      else return (p+20);
   }
   return p;
}


void ErrorLine(char *p)
{
   int i,ep;
   printf("%s\n",Line);
   ep = p - Line;
   if (ep >= 0 && ep < 80)
   {
      for (i=0 ; i < ep ; ++i) printf(" ");
      printf("^\n");
      return;
   }
   ep = p - OpText;
   if (ep >= 0 && ep < 80 && *OpText)
   {
      printf("Operand: %s\n",OpText);
      for (i=0 ; i < ep+9 ; ++i) printf(" ");
      printf("^\n");
      return;
   }

}

// forward declaration

void ListSymbols(FILE *lf, int n, int lb, int ub);


#define SIZE_ERRMSG 1024

void ErrorMsg(const char *format, ...) {
   va_list args;
   char *buf;

   buf = (char *)MallocOrDie(SIZE_ERRMSG);
   snprintf(buf, SIZE_ERRMSG, "\n*** Error in file %s line %d:\n",
         IncludeStack[IncludeLevel].Src, LiNo);
   va_start(args,format);
   vsnprintf(buf+strlen(buf), SIZE_ERRMSG-strlen(buf), format, args);
   va_end(args);
   fputs(Line, stdout);
   fputs(Line, lf);
   fputs(buf, stdout);
   fputs(buf, lf);
   if (df)
   {
      fputs(Line, df);
      fputs(buf, df);
      ListSymbols(df,Labels,0,0xffff);
   }
   free(buf);
}


// *********
// PrintLiNo
// *********

void PrintLiNo(void)
{
   if (ListOn && WithLiNo && Phase == 2) fprintf(lf,"%5d ",LiNo);
}


// *******
// PrintPC
// *******

void PrintPC(void)
{
   if (ListOn && Phase == 2)
   {
      PrintLiNo();
      fprintf(lf,"%4.4x",pc);
   }
}


// *******
// PrintOC
// *******

void PrintOC(int v)
{

   if (!ListOn) return;
   // special format for 32 bit load

   if (oc == 0xcd) // LDQ immediate
   {
      fprintf(lf," cd %4.4x %4.4x",(v>>16)&0xffff,v&0xffff);
      return;
   }

   // opcode value is 16 or 8 bit

   if (oc > 255) fprintf(lf," %4.4x",oc);
   else          fprintf(lf,"   %2.2x",oc);

   // postbyte

   if (pb >= 0)  fprintf(lf," %2.2x",pb);
   else          fprintf(lf,"   ");

   // address or value 16 bit, 8 bit or none

        if (nops == 2 && ql == 0) fprintf(lf," 1212");
   else if (nops == 1 && ql == 0) fprintf(lf," 12  ");
   else if (nops == 1 && ql == 1) fprintf(lf," %2.2x12",v&0xff);
   else if (  ql == 2) fprintf(lf," %4.4x",v&0xffff);
   else if (  ql == 1) fprintf(lf,"   %2.2x",v&0xff);
   else                fprintf(lf,"     ");
}

// *********
// PrintLine
// *********

void PrintLine(void)
{
   if (!ListOn || Phase < 2) return;
   PrintLiNo();
   fprintf(lf,"                  %s\n",Line);
}

void PrintPCLine(void)
{
   if (!ListOn || Phase < 2) return;
   PrintPC();
   fprintf(lf,"              %s\n",Line);
}

void PrintByteLine(int b)
{
   if (!ListOn || Phase < 2) return;
   PrintLiNo();
   fprintf(lf,"       %2.2x",b);
   fprintf(lf,"         %s\n",Line);
}

void PrintWordLine(int w)
{
   if (!ListOn || Phase < 2) return;
   PrintLiNo();
   fprintf(lf,"%4.4x",w);
   fprintf(lf,"              %s\n",Line);
}



int IsInstruction(char *p)
{
   int i,l;

   for (i = 0 ; i < DimOp ; ++i)
   {
      l = strlen(Mat[i].Mne);
      if (!StrNCaseCmp(p,Mat[i].Mne,l) && !isym(p[l]))
      {
         return i;
      }
   }
   return -1; // No mnemonic
}

char *NeedChar(char *p, char c)
{
   p = SkipSpace(p);
   if (*p == c) return p;
   return NULL;
}



char *ParseCaseData(char *p)
{
   p = SkipSpace(p);
        if (*p == '+') IgnoreCase = 0;
   else if (*p == '-') IgnoreCase = 1;
   else
   {
      ++ErrNum;
      ErrorMsg("Missing '+' or '-' after CASE\n");
      exit(1);
   }
   PrintLine();
   return p+1;
}

char *ParseListOption(char *p)
{
   p = SkipSpace(p);
        if (*p == '+') ListOn = 1;
   else if (*p == '-') ListOn = 0;
   else
   {
      ++ErrNum;
      ErrorMsg("Missing '+' or '-' after LIST\n");
      exit(1);
   }
   PrintLine();
   return p+1;
}

char *SetBSS(char *p)
{
   p = NeedChar(p,'=');
   if (!p)
   {
      ++ErrNum;
      ErrorMsg("Missing '=' in set BSS & instruction\n");
      exit(1);
   }
   p = ExtractValue(p+1,&bss);
   if (ListOn && Phase == 2)
   {
      PrintLiNo();
      fprintf(lf,"%4.4x              %s\n",bss,Line);
   }
   return p;
}

int StrCmp(const char *s1, const char *s2)
{
   if (IgnoreCase) return StrCaseCmp(s1,s2);
   else            return strcmp(s1,s2);
}

int StrnCmp(const char *s1, const char *s2, size_t n)
{
   if (IgnoreCase) return StrNCaseCmp(s1,s2,n);
   else            return strncmp(s1,s2,n);
}

int LabelIndex(char *p)
{
   int i;

   for (i = 0 ; i < Labels ; ++i)
   {
      if (!StrCmp(p,lab[i].Name)) return i;
   }
   return -1;
}


int AddressIndex(int a)
{
   int i;

   for (i = 0 ; i < Labels ; ++i)
   {
      if (lab[i].Address == a) return i;
   }
   return -1;
}


int MacroIndex(char *p)
{
   int i,l;

   for (i = 0 ; i < Macros ; ++i)
   {
      l = strlen(Mac[i].Name);
      if (!StrnCmp(p,Mac[i].Name,l) && !isym(p[l])) return i;
   }
   return -1;
}


// *************
// ExtractOpText
// *************

char *ExtractOpText(char *p)
{
   int l,inquo,inapo;

   l          = 0; // length of trimmed operand
   inquo      = 0; // inside quotes
   inapo      = 0; // inside apostrophes
   ForcedMode = 0; // forced direct page (-1) or extended (+1)
   OpText[0]  = 0; // empty operand text

   p = SkipSpace(p);  // text after mnemonic or pseudo op
   if (!*p) return p; // end of line

   if (*p == '<' || *p == '>')
   {
      ForcedMode = *p - 0x3d; // -1 for '<', +1 for '>'
      ++p;
   }

   // Extract operand

   if (CodeStyle == 1) // Thomson style
   {
      while (*p && *p != ' ' && l < ML) OpText[l++] = *p++;
   }
   else
   {
      while (*p && l < ML)
      {
         if (*p == '"'  && inapo == 0) inquo = !inquo;
         if (*p == '\'' && inquo == 0) inapo = !inapo;
         if (*p == ';'  && inquo == 0 && inapo == 0) break;
         OpText[l++] = *p++;
      }
      while (l && isspace(OpText[l-1])) OpText[--l] = 0;
   }
   OpText[l] = 0; // end marker
   if (df) fprintf(df,"OpText = [%s]\n",OpText);
   return p;      // points to comment start or EOL
}

#define LABTYPES 4
struct LabelDefStruct
{
   char Name[5];
   int  Length;
   int  Type;
} LabDef[LABTYPES] =
{
   {"="   ,1, 0},
   {"EQU" ,3, 0},
   {"SET" ,3,-1},
   {"ENUM",4, 1}
};


// ***********
// DefineLabel
// ***********

char *DefineLabel(char *p, int *val, int Locked)
{
   int i,j,l,v;
   char *rop;

   *val = UNDEF; // preset

   if (Labels > MAXLAB -2)
   {
      ++ErrNum;
      ErrorMsg("Too many labels (> %d)\n",MAXLAB);
      exit(1);
   }
// if (df) fprintf(df,"GetSymbol(%s)\n",p);
   p = GetSymbol(p,Label);
   if (*p == ':') ++p; // Ignore colon after label
   l = strlen(Label);
   p = SkipSpace(p);

   // parse definition of constants or variables

   for (i=0 ; i < LABTYPES ; ++i)
   {
      if (*p == '=' || !strcmpword(p,LabDef[i].Name)) break;
   }

   if (i < LABTYPES) // label definition
   {
      if (df) fprintf(df,"LABVAL:%s:\n",p);
      if (df) fprintf(df,"Length:%d Index:%d\n",LabDef[i].Length,i);
      p += LabDef[i].Length;   // add keyword length
      if (df) fprintf(df,"---VAL:%s:\n",p);
      j = LabelIndex(Label);
      if (j < 0)
      {
         j = Labels;
         lab[j].Name = (char *)StrNDup(Label,l);
         lab[j].Address = UNDEF;
         lab[j].Ref = (int *)MallocOrDie(sizeof(int));
         lab[j].Att = (int *)MallocOrDie(sizeof(int));
         Labels++;
      }
      lab[j].Ref[0] = LiNo;
      lab[j].Att[0] = LDEF;

      ExtractOpText(p);
      if (OpText[0])
      {
         p += strlen(p);
         rop = EvalOperand(OpText,&v,0);
         if (*rop)
         {
            ErrorLine(rop);
            ErrorMsg("Extra text after label assignment\n");
            exit(1);
         }
         if (lab[j].Address == UNDEF || LabDef[i].Type == 0)
             lab[j].Address = v;
         else if (lab[j].Address != v && !lab[j].Locked)
         {
            ++ErrNum;
            ErrorLine(p);
            ErrorMsg("*Multiple assignments for label [%s]\n"
                     "1st. value = $%4.4x   2nd. value = $%4.4x\n",
                     Label, lab[j].Address, v);
            exit(1);
         }
         *val = v;
         if (LabDef[i].Type > 0) EnumValue = v;
         if (Locked) lab[j].Locked = Locked;
      }
      else if (LabDef[i].Type > 0) // ENUM
      {
         *val = ++EnumValue;
         if (lab[j].Address == UNDEF) lab[j].Address = *val;
         else if (lab[j].Address != *val)
         {
            ++ErrNum;
            ErrorLine(p);
            ErrorMsg("ENUM phase error\n");
            exit(1);
         }
      }
      else
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Missing operand\n");
         exit(1);
      }
   }
   else if (!strcmpword(p,"BSS"))
   {
      p = ExtractValue(p+4,&v);
      j = LabelIndex(Label);
      if (j < 0)
      {
         j = Labels;
         lab[j].Name = (char *)StrNDup(Label,l);
         lab[j].Address = UNDEF;
         lab[j].Ref = (int *)MallocOrDie(sizeof(int));
         lab[j].Att = (int *)MallocOrDie(sizeof(int));
         Labels++;
      }
      lab[j].Ref[0] = LiNo;
      lab[j].Att[0] = LBSS;
      if (lab[j].Address >= UNDEF) lab[j].Address = bss;
      else if (lab[j].Address != bss)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Multiple assignments for BSS label [%s]\n"
                  "1st. value = $%4.4x   2nd. value = $%4.4x\n",
                  Label,lab[j].Address,bss);
         exit(1);
      }
      *val = bss;
      bss += v;
   }
   else
   {
      j = LabelIndex(Label);
      if (j < 0)
      {
         j = Labels;
         lab[j].Name = (char *)StrNDup(Label,l);
         lab[j].Address = pc;
         lab[j].Ref = (int *)MallocOrDie(sizeof(int));
         lab[j].Att = (int *)MallocOrDie(sizeof(int));
         Labels++;
      }
      else if (lab[j].Address == UNDEF) lab[j].Address = pc;
      else if (lab[j].Address != pc && !lab[j].Locked)
      {
         ++ErrNum;
         if (Phase == 1)
         {
            ErrorMsg("Multiple label definition [%s]"
                     " value 1: %4.4x   value 2: %4.4x\n",
                     Label, lab[j].Address,pc);
         }
         else
         {
            ErrorMsg("Phase error label [%s]"
                     " phase 1: %4.4x   phase 2: %4.4x\n",
                     Label,lab[j].Address,pc);
         }
         exit(1);
      }
      if (!lab[j].Locked) *val = pc;
      lab[j].Ref[0] = LiNo;
      lab[j].Att[0] = LPOS;
   }
   return p;
}


void SymRefs(int i)
{
   int n;

   if (Phase != 2) return;
   n = ++lab[i].NumRef;
   lab[i].Ref = (int *)ReallocOrDie(lab[i].Ref,(n+1)*sizeof(int));
   lab[i].Ref[n] = LiNo;
   lab[i].Att = (int *)ReallocOrDie(lab[i].Att,(n+1)*sizeof(int));
   lab[i].Att[n] = am;
}


char *EvalSymValue(char *p, int *v)
{
   int i;
   char Sym[ML];

   p = GetSymbol(p,Sym);
   for (i=0 ; i < Labels ; ++i)
   {
      if (!StrCmp(Sym,lab[i].Name))
      {
         *v = lab[i].Address;
         SymRefs(i);
         return p;
      }
   }
   AddLabel(Sym);
   *v = UNDEF;
   return p;
}


char *EvalSymBytes(char *p, int *v)
{
   int i;
   char Sym[ML];

   p = GetSymbol(p,Sym);
   for (i=0 ; i < Labels ; ++i)
   {
      if (!StrCmp(Sym,lab[i].Name))
      {
         *v = lab[i].Bytes;
         SymRefs(i);
         return p;
      }
   }
   AddLabel(Sym);
   *v = UNDEF;
   return p;
}

int Nib2Byte(char nib)
{
   if (nib >= '0' && nib <= '9') return nib - '0';
   if (nib >= 'A' && nib <= 'F') return nib - 'A' + 10;
   if (nib >= 'a' && nib <= 'f') return nib - 'a' + 10;
   return -1;
}

int Hex2Byte(char hex[])
{
   int h,l;
   h = Nib2Byte(hex[0]);
   l = Nib2Byte(hex[1]);
   if (h >= 0 && l >= 0) return (h << 4) | l;
   return -1;
}

char *ParseRealData(char *p)
{

   int v;
   int i,mansize;
   int Sign,Exponent;
   double d;

   mansize = 3; // default mantissa size

   p = SkipSpace(p);
   memset(Operand,0,sizeof(Operand));

   if (*p == '$')
   {
      ++p;
      for (i=0 ; i <= mansize ; ++i, p+=2)
      {
          v = Hex2Byte(p);
          if (v < 0) break;
          Operand[i] = v;
      }
   }
   else
   {
      d = strtod(p,NULL);
      if (d != 0)
      {
         Sign = 0;
         if (d < 0)
         {
            Sign = 0x80;
            d = -d;
         }
         d = frexp(d,&Exponent);
         Exponent += 0x80;
         if (Exponent < 1 || Exponent > 255)
         {
            ErrorMsg("Exponent %d out of range\n",Exponent);
            ++ErrNum;
            return p+strlen(p);;
         }
         Operand[0] = Exponent;
         d *= 256;
         v = d;
         Operand[1] = (v & 127) | Sign;
         d -= v;

         for (i=2 ; i < 6 ; ++i)
         {
            d *= 256;
            Operand[i] = v = d;
            d -= v;
         }
      }
   }

   // Round

   if (Operand[mansize+1] & 0x80) // rounding bit set
   {
      for (i=mansize ; i > 1 ; --i)
      {
         if (++Operand[i]) break; // no carry
      }
      if (i == 1) // carry was propagated
      {
         if (Operand[1] == 0x7f) // highest positive value
         {
            ++Operand[0];        // Increment exponent
              Operand[1] = 0;
         }
         else if (Operand[1] == 0xff)
         {
            ++Operand[0];        // Increment exponent
              Operand[1] = 0x80;
         }
         else ++Operand[1];
      }
   }

   if (Phase == 2)
   {
      for (i=0 ; i < mansize+1 ; ++i) Put(pc+i,Operand[i],p);
      if (ListOn)
      {
         PrintPC();
         fprintf(lf," %2.2x %2.2x%2.2x%2.2x  ",
            Operand[0],Operand[1],Operand[2],Operand[3]);
         fprintf(lf," %s\n",Line);
      }
   }
   pc += mansize+1;
   return p + strlen(p);;
}

char *EvalHexValue(char *p, int *v)
{
   char *EndPtr;
   long long lol;
   lol = strtoll(p,&EndPtr,16);
   *v = lol & 0xffffffff;
   return EndPtr;
}

char *EvalDecValue(char *p, int *v)
{
   int i;

   // check for xxxxH or xxxxh hex syntax;

   for (i=0 ; i < 5 ; ++i) // allow max. 5 hex digits
      if (!isxdigit(p[i])) break;
   if (p[i] == 'H' || p[i] == 'h')
   {
      p = EvalHexValue(p,v);
      return p+1;
   }

   // check for decimal value

   *v = atoi(p);
   while (isdigit(*p)) ++p;
   if (!isalpha(*p)) return p;
   if ((*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f'))
      ErrorMsg("Wrong decimal constant or leading $ for hex missing\n");
   else
      ErrorMsg("Illegal character in decimal constant\n");
   ++ErrNum;
   ErrorLine(p);
   exit(1);
}


char *EvalCharValue(char *p, int *v)
{
   if (*p == '\\')
   {
      ++p;
           if (*p == 'r') *v = 13;
      else if (*p == 'n') *v = 10;
      else if (*p == 'a') *v =  7;
      else if (*p == 'e') *v = 27;
      else if (*p == '0') *v =  0;
      else                *v = *p;
      if (*p) ++p;
   }
   else *v = *p++;
   if (*p != '\'' && *p != 0)
   {
      ++ErrNum;
      ErrorMsg("Missing ' delimiter after character operand\n");
      exit(1);
   }
   if (*p) ++p;
   return p;
}

char *EvalMultiCharValue(char *p, int *v)
{
   int i;

   *v = 0;                     // preset with zero
   for (i=0 ; i < 4 ; ++i)
   {
      if (*p == '\"') break;   // end quote
      *v = (*v << 8) | *p++;
   }
   if (*p == '\"') ++p;        // end quote
   else ErrorMsg("Multi character operand too long ( > 4 )\n");
   return p;
}



char *EvalBinValue(char *p, int *v)
{
   int r;
   r = 0;
   while(*p == ' ' || *p == '1' || *p == '0' || *p == '*' || *p == '.')
   {
      if (*p == '1' || *p == '*') r = (r << 1) + 1;
      if (*p == '0' || *p == '.') r <<= 1;
      ++p;
   }
   *v = r;
   return p;
}


char *SkipToComma(char *p)
{
   while (*p && *p != ',' && *p != ';') ++p;
   return p;
}


char *op_par(char *p, int *v)
{
   char c = (*p == '[') ? ']' : ')'; // closing char
   p = EvalOperand(p+1,v,0);
   p = NeedChar(p,c);
   if (!p)
   {
      ErrorLine(p);
      ErrorMsg("Missing closing %c\n",c);
      exit(1);
   }
   return p+1;
}

// functions parsing unary operators or constants

char *op_plu(char *p, int *v) { p = EvalOperand(p+1,v,12)               ; return p; }
char *op_min(char *p, int *v) { p = EvalOperand(p+1,v,12);*v = -(*v)    ; return p; }
char *op_lno(char *p, int *v) { p = EvalOperand(p+1,v,12);*v = !(*v)    ; return p; }
char *op_bno(char *p, int *v) { p = EvalOperand(p+1,v,12);*v = ~(*v)    ; return p; }

char *op_low(char *p, int *v) { p = EvalOperand(p+1,v,12);ForcedMode=-1 ; return p; }
char *op_hig(char *p, int *v) { p = EvalOperand(p+1,v,12);ForcedMode= 1 ; return p; }

char *op_prc(char *p, int *v) { *v = pc; return p+1;}
char *op_hex(char *p, int *v) { return EvalHexValue(p+1,v) ;}
char *op_cha(char *p, int *v) { return EvalCharValue(p+1,v);}
char *op_muc(char *p, int *v) { return EvalMultiCharValue(p+1,v);}
char *op_bin(char *p, int *v) { return EvalBinValue(p+1,v) ;}
char *op_len(char *p, int *v) { return EvalSymBytes(p+1,v) ;}

struct unaop_struct
{
   char op;
   char *(*foo)(char*,int*);
};


// table of unary operators in C style

struct unaop_struct unaop[] =
{
   {'<',&op_low}, // low byte
   {'>',&op_hig}, // extended
   {'[',&op_par}, // bracket
   {'(',&op_par}, // parenthesis
   {'+',&op_plu}, // positive sign
   {'-',&op_min}, // negative sign
   {'!',&op_lno}, // logical NOT
   {'~',&op_bno}, // bitwise NOT
   {'*',&op_prc}, // program counter
   {'$',&op_hex}, // hex constant
   { 39,&op_cha}, // char constant
   { 34,&op_muc}, // multi char constant
   {'%',&op_bin}, // binary constant
   {'?',&op_len}  // length of .BYTE data line
};

#define UNAOPS (int)(sizeof(unaop) / sizeof(struct unaop_struct))

int op_mul(int l, int r) { return l *  r; }
int op_div(int l, int r) { if (r) return l / r; else return UNDEF; }
int op_add(int l, int r) { return l +  r; }
int op_sub(int l, int r) { return l -  r; }
int op_asl(int l, int r) { return l << r; }
int op_lsr(int l, int r) { return l >> r; }
int op_cle(int l, int r) { return l <= r; }
int op_clt(int l, int r) { return l <  r; }
int op_cge(int l, int r) { return l >= r; }
int op_cgt(int l, int r) { return l >  r; }
int op_ceq(int l, int r) { return l == r; }
int op_cne(int l, int r) { return l != r; }
int op_xor(int l, int r) { return l ^  r; }
int op_and(int l, int r) { return l && r; }
int op_bnd(int l, int r) { return l &  r; }
int op_lor(int l, int r) { return l || r; }
int op_bor(int l, int r) { return l |  r; }


struct binop_struct
{
   const char *op;
   int prio;
   int (*foo)(int,int);
};

#define BINOPS 17

// table of binary operators in C style and priority

struct binop_struct binop[BINOPS] =
{
   {"*" ,11,&op_mul}, //  Multiplication
   {"/" ,11,&op_div}, //  Division
   {"+" ,10,&op_add}, //  Addition
   {"-" ,10,&op_sub}, //  Subtraction
   {"<<", 9,&op_asl}, //  bitwise left  shift
   {">>", 9,&op_lsr}, //  bitwise right shift
   {"<=", 8,&op_cle}, //  less than or equal to
   {"<" , 8,&op_clt}, //  less than
   {">=", 8,&op_cge}, //  greater than or equal to
   {">" , 8,&op_cgt}, //  greater than
   {"==", 7,&op_ceq}, //  equal to
   {"!=", 7,&op_cne}, //  not equal to
   {"^" , 5,&op_xor}, //  bitwise XOR
   {"&&", 3,&op_and}, //  logical AND
   {"&" , 6,&op_bnd}, //  bitwise AND
   {"||", 2,&op_lor}, //  logical OR
   {"|" , 4,&op_bor}  //  bitwise OR
};

// ***********
// EvalOperand
// ***********

char *EvalOperand(char *p, int *v, int prio)
{
   int  i;    // loop index
   int  l;    // length of string
   int  o;    // priority of operator
   int  r;    // return value
   int  w;    // value of right operand
   char c;    // current character

   r = UNDEF; // preset result to UNDEF

   p = SkipSpace(p);
   c = *p;
   if (df) fprintf(df,"EvalOperand <%s>\n",p);

   if (c == ',' )  return p; // comma separator

   // Start parsing unary operators
   // PC represents the current program counter

   if (*p && strchr("[(+-!~<>*$'\"%?",*p))
   {
      for (i=0 ; i < UNAOPS ; ++i)
      if (*p == unaop[i].op)
      {
          p = unaop[i].foo(p,&r);
          break;
      }
   }
   else if (isdigit(c)) p = EvalDecValue(p,&r);    // decimal constant
   else if (isym(c))    p = EvalSymValue(p,&r);    // symbol or label
   else
   {
      ErrorLine(p);
      ErrorMsg("Illegal operand\n");
      exit(1);
   }

   // Left operand has been parsed successfully
   // now look for binary operators
   //  r is left  operand
   //  w is right operand

   // the left or right operand may be UNDEFINED in phase 1
   // in that case do not perform the operation, just parse

   if (CodeStyle == 1 && *p == ' ')
   {
      *v = r;
      p += strlen(p);
      if (df) fprintf(df,"Result: %4x %d\n",r,r);
      return p;
   }
   p = SkipSpace(p);

   while (*p && strchr("*/+-<>=!&^|",*p))
   {
      // loop through all binary operators

      for (i=0 ; i < BINOPS ; ++i)
      {
         l = strlen((binop[i].op));
         if (!strncmp(p,binop[i].op,l))
         {
            if ((o = binop[i].prio) <= prio)
            {
               *v = r;
               if (CodeStyle == 1 && *p == ' ') p += strlen(p);
               if (df) fprintf(df,"Result: %4x %d\n",r,r);
               return p;
            }
            p = EvalOperand(p+l,&w,o);
            if (w == UNDEF) r = UNDEF;
            else r = binop[i].foo(r,w);
            break;
         }
      }
      if (i == BINOPS)  // no valid operator
      {
         ErrorMsg("Syntax error: binary operator expected\n");
         ErrorLine(p);
         exit(1);
      }
   }
   *v = r;
   if (CodeStyle == 1 && *p == ' ') p += strlen(p);
   if (df) fprintf(df,"Result: %4x %d\n",r,r);
   if (df) fprintf(df,"Rest  : %s\n",p);
   return p;
}

// ************
// ExtractValue
// ************

char *ExtractValue(char *p, int *v)
{
   char *r; // pointer to trailing text after value (should be none)

   p = ExtractOpText(p);        // separate value string from comment
   if (OpText[0] == 0)
   {
      ErrorLine(p);
      ErrorMsg("Empty operand\n");
      exit(1);
   }
   r = EvalOperand(OpText,v,0); // evaluate integer value
   if (*r)                      // check for trailing text
   {
      ErrorLine(r);
      ErrorMsg("Extra text after operand\n");
      exit(1);
   }
   return p;                    // points to comment or EOL
}

// *************
// ParseWordData
// *************

char *ParseWordData(char *p)
{
   int i,j,l,v;
   unsigned char ByteBuffer[ML];

   l = 0;
   p = SkipSpace(p);
   while (*p && *p != ';') // Parse data line
   {
      p = EvalOperand(p,&v,0);
      ByteBuffer[l++] = v >> 8;
      ByteBuffer[l++] = v;
      p = SkipToComma(p);
      if (*p == ',') ++p;
   }
   if (l < 1)
   {
      ErrorMsg("Missing WORD data\n");
      ErrorLine(p);
      exit(1);
   }
   j = AddressIndex(pc);
   if (j >= 0 && df) fprintf(df,"Byte label [%s] $%4.4x $%4.4x %d bytes\n",
                   lab[j].Name,lab[j].Address,pc,l);
   if (j >= 0)
   for ( ; j < Labels ; ++j) // There may be multiple lables on this address
   {
       if (lab[j].Address == pc) lab[j].Bytes = l;
   }
   if (Phase == 2)
   {
      for (i=0 ; i < l ; ++i)
      {
         Put(pc+i,ByteBuffer[i],p);
         if (ListOn && (i == 0 || i == 2))
            fprintf(lf," %2.2x%2.2x",ByteBuffer[i],ByteBuffer[i+1]);
      }
      if (ListOn)
      {
         if (l == 2) fprintf(lf,"        ");
         else        fprintf(lf,"   ");
         fprintf(lf," %s\n",Line);
      }
   }
   pc += l;

   return p;
}


char *ParseLongData(char *p)
{
   int i,j,l,v;
   unsigned char ByteBuffer[ML];

   l = 0;
   p = SkipSpace(p);
   while (*p && *p != ';') // Parse data line
   {
      p = EvalOperand(p,&v,0);
      ByteBuffer[l++] = v >> 24;
      ByteBuffer[l++] = v >> 16;
      ByteBuffer[l++] = v >>  8;
      ByteBuffer[l++] = v;
      p = SkipToComma(p);
      if (*p == ',') ++p;
   }
   if (l < 4)
   {
      ErrorMsg("Missing LONG data\n");
      ErrorLine(p);
      exit(1);
   }
   j = AddressIndex(pc);
   if (j >= 0 && df) fprintf(df,"LONG label [%s] $%4.4x $%4.4x %d bytes\n",
                   lab[j].Name,lab[j].Address,pc,l);
   if (j >= 0)
   for ( ; j < Labels ; ++j) // There may be multiple lables on this address
   {
       if (lab[j].Address == pc) lab[j].Bytes = l;
   }
   if (Phase == 2)
   {
      for (i=0 ; i < l ; ++i)
      {
         Put(pc+i,ByteBuffer[i],p);
         if (ListOn && (i == 0 || i == 2))
            fprintf(lf," %2.2x%2.2x",ByteBuffer[i],ByteBuffer[i+1]);
      }
      if (ListOn)
      {
         fprintf(lf,"   ");
         fprintf(lf," %s\n",Line);
      }
   }
   pc += l;

   return p;
}


char *ParseFillData(char *p)
{
   int i,m,v;

   p = EvalOperand(p,&m,0);
   if (m < 0 || m > 0xffff)
   {
      ErrorMsg("Illegal FILL multiplier %d\n",m);
      exit(1);
   }
   p = NeedChar(p,'(');
   if (!p)
   {
      ErrorMsg("Missing '(' before FILL value\n");
      exit(1);
   }
   p = EvalOperand(p+1,&v,0);
   v &= 0xff;
   if (Phase == 2)
   {
      for (i=0 ; i < m ; ++i) Put(pc+i,v,p);
      if (ListOn)
      {
         PrintPC();
         if (m > 0) fprintf(lf," %2.2x",v);
         else       fprintf(lf,"   ");
         if (m > 1) fprintf(lf," %2.2x",v);
         else       fprintf(lf,"   ");
         if (m > 2) fprintf(lf," %2.2x",v);
         else       fprintf(lf,"   ");
         fprintf(lf," %s ; %d bytes\n",Line,m);
      }
   }
   pc += m;
   p += strlen(p);
   return p;
}

char *ListSizeInfo(char *p)
{
   int i;
   p += strlen(p);

   if (!ModuleStart) return p;
   if (ListOn && Phase == 2)
   {
      i = AddressIndex(ModuleStart);
      if (i >= 0)
      {
         fprintf(lf,"              %s",Line);
         fprintf(lf," ;%5d [%s]",pc-ModuleStart,lab[i].Name);
         ModuleStart = 0;
      }
      fprintf(lf,"\n");
   }
   return p;
}


char *IncludeFile(char *p)
{
   char FileName[256];
   char *fp;
   p = NeedChar(p,'"');
   if (!p)
   {
      ErrorMsg("Missing quoted filename after INCLUDE\n");
      exit(1);
   }
   fp = FileName;
   ++p;
   while (*p != 0 && *p != '"') *fp++ = *p++;
   *fp = 0;
   if (df) fprintf(df,"fopen %s\n",FileName);
   if (IncludeLevel >= 99)
   {
      ErrorMsg("Too many includes nested ( >= 99)\n");
      exit(1);
   }
   sf = fopen(FileName,"r");
   if (!sf)
   {
      printf("Could not open include file <%s>\n",FileName);
      exit(1);
   }
   IncludeStack[IncludeLevel].LiNo = LiNo;
   IncludeStack[++IncludeLevel].fp = sf;
   IncludeStack[IncludeLevel].Src = (char *)StrNDup(FileName,strlen(FileName));
   PrintLine();
   LiNo = 0;
   return p+1; // skip quote after filename
}

char *ParseStoreData(char *p)
{
   int Start,Length,FileFormat,Entry;
   char *Filename,*EndPtr;

   if (Phase < 2) return p + strlen(p);
   p = EvalOperand(p,&Start,0);
   if (Start < 0 || Start > 0xffff)
   {
      ErrorMsg("Illegal start address for STORE %d\n",Start);
      exit(1);
   }
   p = NeedChar(p,',');
   if (!p)
   {
      ErrorMsg("Missing ',' after start address\n");
      exit(1);
   }
   p = EvalOperand(p+1,&Length,0);
   if (Length < 0 || Length > 0x10000)
   {
      ErrorMsg("Illegal length for STORE %d\n",Length);
      exit(1);
   }
   p = NeedChar(p,',');
   if (!p)
   {
      ErrorMsg("Missing ',' after length\n");
      exit(1);
   }
   p = NeedChar(p+1,'"');
   if (!p)
   {
      ErrorMsg("Missing quote for filename\n");
      exit(1);
   }
   EndPtr = ++p;
   while (*EndPtr != '\0' && *EndPtr != '"') ++EndPtr;
   Filename = (char *)StrNDup(p, EndPtr - p);
   FileFormat = BINARY;
   Entry = -1;
   p = NeedChar(EndPtr,',');
   if (p)
   {
      ++p;
           if (StrMatch(p, "BIN"))  FileFormat = BINARY;
      else if (StrMatch(p, "SREC")) FileFormat = SRECORD;
      else if (StrMatch(p, "S19"))  FileFormat = SRECORD;
      else
      {
         ErrorMsg("Unknown output file format\n");
         exit(1);
      }
      p = NeedChar(p,',');
      if (p)
      {
         p = EvalOperand(p+1,&Entry,0);
         if (Entry < 0 || Entry > 0xffff)
         {
            ErrorMsg("Illegal execution start address for STORE %d\n",Entry);
            exit(1);
         }
      } else p = EndPtr;
   } else p = EndPtr;

   SFA[StoreCount] = Start;
   SFL[StoreCount] = Length;
   SFE[StoreCount] = Entry;
   SFF[StoreCount] = Filename;
   SFT[StoreCount] = FileFormat;
   if (df) fprintf(df,"Storing %4.4x - %4.4x <%s>, %s format\n",
           Start,Start+Length-1,Filename,FileFormat ? "S19" : "binary");
   if (StoreCount < SFMAX) ++StoreCount;
   else
   {
      ErrorMsg("number of storage files exceeds %d\n",SFMAX);
      exit(1);
   }
   PrintLine();
   p += strlen(p);
   return p;
}


char *ParseLoadData(char *p)
{
   int i,Start,Size,Advance;
   char *Filename,*EndPtr;
   FILE *lp;

   p = SkipSpace(p);

   Advance = 1;
   if (*p == '\"') Start = pc;
   else
   {
      Advance = 0;
      p = EvalOperand(p,&Start,0);

      if (Start < 0 || Start > 0xffff)
      {
         ErrorMsg("Illegal start address for LOAD %d\n",Start);
         exit(1);
      }
      p = NeedChar(p,',');
      if (!p)
      {
         ErrorMsg("Missing ',' after start address\n");
         exit(1);
      }
      p = NeedChar(p+1,'"');
      if (!p)
      {
         ErrorMsg("Missing quote for filename\n");
         exit(1);
      }
   }
   EndPtr = ++p;
   while (*EndPtr != '\0' && *EndPtr != '"') ++EndPtr;
   Filename = (char *)StrNDup(p, EndPtr - p);
   if (df) fprintf(df,"Loading %4.4x <%s>\n",Start,Filename);
   PrintLine();
   lp = fopen(Filename,"rb");
   AssertFileOp(lp,"Could not LOAD <%s>\n");
   fseek(lp,0,SEEK_END);
   Size = ftell(lp);
   rewind(lp);
   if (Start + Size > 0x10000)
   {
      ErrorMsg("LOADING %4.4x to %4.4x violates 64K size\n",
         Start,Start+Size);
      exit(1);
   }

   if (Phase == 2)
   for (i=Start; i < Start+Size ; ++i)
   {
      if (LOCK[i])
      {
         ErrorMsg("LOAD would overwrite defined values\n");
         ErrorLine(p);
         exit(1);
      }
      LOCK[i] = 1;
   }
   fread(ROM+Start,Size,1,lp);
   fclose(lp);
   if (Advance) pc += Size;
   p += strlen(p);
   return p;
}


char *ParseBSSData(char *p)
{
   int m;

   p = ExtractValue(p,&m);
   if (m < 1 || m > 32767)
   {
      ErrorMsg("Illegal BSS size %d\n",m);
      exit(1);
   }
   if (ListOn && Phase == 2)
      fprintf(lf,"%4.4x             %s\n",bss,Line);
   bss += m;
   return p;
}


char *ParseCPUData(char *p)
{
   p = SkipSpace(p);
   if (*p == '=') ++p;
   p = EvalOperand(p,&CPU,0);
   if (CPU == 6809)
   {
      DimOp = DIMOP_6809;
      RegisterNames = Register_6809;
   }
   else if (CPU == 6309)
   {
      DimOp = DIMOP_6309;
      RegisterNames = Register_6309;
   }
   else
   {
      ErrorMsg("Unknown CPU %d - use 6809 or 6309\n",CPU);
      exit(1);
   }
   if (ListOn && Phase == 2)
   {
      PrintLiNo();
      fprintf(lf,"%4d              %s\n",CPU,Line);
   }
   return p;
}


char *ParseBitData(char *p)
{
   int i,v;

   v = 0;
   for (i=0 ; i < 8 ; ++i)
   {
      v <<= 1;
      p = SkipSpace(p+1);
      if (*p == '*') v |= 1;
      else if (*p != '.')
      {
         ErrorMsg("use only '*' for 1 and '.' for 0 in BITS statement\n");
         exit(1);
      }
   }
   if (ListOn && Phase == 2)
   {
      PrintPC();
      Put(pc,v,p);
      fprintf(lf," %2.2x           ",v);
      fprintf(lf,"%s\n",Line);
   }
   ++pc;
   return p + strlen(p);
}

char *ParseCmapData(char *p)
{
   int i,v,scanline;

   v = 0;
   if (*p >= '0' && *p < '8') scanline = *p - '0';
   else                       scanline = -1;

   for (i=0 ; i < 8 ; ++i)
   {
      v <<= 1;
      p = SkipSpace(p+1);
      if (*p == '*') v |= 1;
      else if (*p != '.')
      {
         ErrorMsg("use only '*' for 1 and '.' for 0 in CMAP statement\n");
         exit(1);
      }
   }
   if (Phase == 2)
   {
      if (ListOn) PrintPC();
      if (scanline < 0) Put(pc,v,p);
      else Put(pc+2*scanline-7,v,p);
      if (ListOn)
      {
         fprintf(lf," %2.2x       ",v);
         fprintf(lf,"%s\n",Line);
      }
   }
   ++pc;
   return p + strlen(p);
}

char *ParseASCII(char *p, unsigned char b[], int *l)
{
   char Delimiter;

   Delimiter = *p++;
   while (*p && *p != Delimiter && *l < ML-1)
   {
      if (*p == '\\') // special character CR, LF, NULL
      {
         ++p;
              if (*p == 'r') b[*l] = 13; // return
         else if (*p == 'f') b[*l] = 12; // form feed
         else if (*p == 'n') b[*l] = 10; // new line
         else if (*p == 't') b[*l] =  9; // tab
         else if (*p == 'a') b[*l] =  7; // alert
         else if (*p == '0') b[*l] =  0; // zero
         else if (*p == 'b') b[*l] = 29; // bold colour
         else if (*p == 's') b[*l] = 28; // standard colour
         else b[*l] = *p;
         ++(*l);
         ++p;
      }
      else
      {
         b[(*l)++] = *p++;
      }
   }
   if (*p == Delimiter) ++p;
   if (*p == '^')
   {
      b[(*l)-1] |= 0x80; // Set bit 7 of last char
      ++p;
   }
   return p;
}

// *************
// ParseByteData
// *************

char *ParseByteData(char *p)
{
   int i,j,l,v;
   unsigned char ByteBuffer[ML];
   char Delimiter;

   p = SkipSpace(p);
   l = 0;
   while (*p && *p != ';') // Parse data line
   {
      if (CodeStyle == 1 && *p == ' ') break;
      else p = SkipSpace(p);
      Delimiter = *p;
      if (!strncmp(p,"$DATE",5))
      {
         strcpy((char *)ByteBuffer+l,datebuffer);
         l += strlen(datebuffer);
         p += 5;
      }
      else if (Delimiter == '"' || Delimiter == '\'')
      {
         i = l; // remember start of string
         p = ParseASCII(p,ByteBuffer,&l);
         if (df)
         {
            fprintf(df,"String $%4.4x:<",pc);
            for (j=i ; j < l ; ++j) fprintf(df,"%c",ByteBuffer[j]&0x7f);
            fprintf(df,"> [%d]\n",l-i);
         }
      }
      else
      {
         v = UNDEF;
         p = EvalOperand(p,&v,0);
         if (v == UNDEF && Phase == 2)
         {
            ErrorMsg("Undefined symbol in BYTE data\n");
            ErrorLine(p);
            exit(1);
         }
         if (v > 255 || v < -127) ByteBuffer[l++] = v >> 8;
         ByteBuffer[l++] = v & 0xff;
         if (df)
         {
            fprintf(df,"BYTE   $%4.4x: %2.2x\n",pc,v);
         }
      }
      if (CodeStyle == 1 && *p == ' ') break;
      p = SkipToComma(p);
      if (*p == ',') ++p;
   }
   if (l < 1)
   {
      ErrorMsg("Missing byte data\n");
      ErrorLine(p);
      exit(1);
   }
   j = AddressIndex(pc);
   if (j >= 0)
   for ( ; j < Labels ; ++j) // There may be multiple lables on this address
   {
       if (lab[j].Address == pc) lab[j].Bytes = l;
   }
   if (j >= 0 && df) fprintf(df,"Byte label [%s] $%4.4x $%4.4x %d bytes\n",
                   lab[j].Name,lab[j].Address,pc,l);
   if (Phase == 2)
   {
      for (i=0 ; i < l ; ++i)
      {
         Put(pc+i,ByteBuffer[i],p);
         if (ListOn && i < 4) fprintf(lf," %2.2x",ByteBuffer[i]);
      }
      if (ListOn)
      {
         for (i=l ; i < 4 ; ++i) fprintf(lf,"   ");
         fprintf(lf,"  %s\n",Line);
      }
   }
   pc += l;
   return p;
}


// *************
// Parsebit5Data
// *************

char *Parsebit5Data(char *p)
{
   char c;
   int i,v;

   p = SkipSpace(p);
   if (strlen(p) < 7 || p[0] != '"' || p[6] != '"')
   {
      ErrorMsg("Need 5 character string\n");
      ErrorLine(p);
      exit(1);
   }

   // pack 5 characters into 20 bit

   // '?' ->  0
   // ' ' ->  1
   // 'A' ->  2
   // 'Z' -> 27
   // '2' -> 28
   // '3' -> 29

   v = 0;
   for (i=4 ; i > 0 ; --i)
   {
      c = p[i];
      if (c == ' ') c = 0x40;
      if (c == '2') c = 'Z'+1;
      if (c == '3') c = 'Z'+2;
      if (c < '?' || c > 'Z'+2)
      {
         ErrorMsg("illegal character\n");
         ErrorLine(p);
         exit(1);
      }
      v = (v << 5) | (c - '?');
   }
        if (p[5] == 'C') v |= (1 << 20);
   else if (p[5] == 'D') v |= (2 << 20);
   else if (p[5] == 'R') v |= (3 << 20);
   else if (p[5] == 'W') v |= (4 << 20);

   if (Phase == 2)
   {
      if (ListOn) fprintf(lf," %6.6x       %s\n",v,Line);
      for (i=2 ; i >= 0 ; --i)
      {
         Put(pc+i,v & 0xff,p);
         v >>= 8;
      }
   }
   pc += 3;
   return p+7;
}

// ***************
// ParseSubroutine
// ***************

char *ParseSubroutine(char *p)
{
   p = SkipSpace(p);
   DefineLabel(p,&ModuleStart,0);
   strcpy(Scope,Label);
   if (df) fprintf(df,"SCOPE: [%s]\n",Scope);
   if (Phase == 2 && ListOn)
   {
      fprintf(lf,"              %s\n",Line);
   }
   return p;
}

// ******
// EndSub
// ******

char *EndSub(char *p)
{
   if (Phase == 2 && ListOn)
   {
      PrintPC();
      p = ListSizeInfo(p);
   }
   Scope[0] = 0;
   ModuleStart = 0;
   return p;
}

// Functions for pseudo ops

// ###

char *ps_bit5(char *p)   { PrintPC(); return Parsebit5Data(p);}
char *ps_bits(char *p)   {            return ParseBitData(p); }
char *ps_bss(char *p)    { PrintPC(); return ParseBSSData(p); }
char *ps_byte(char *p)   { PrintPC(); return ParseByteData(p); }
char *ps_case(char *p)   { PrintPC(); return ParseCaseData(p); }
char *ps_cmap(char *p)   { PrintPC(); return ParseCmapData(p); }
char *ps_cpu(char *p)    {            return ParseCPUData(p); }
char *ps_end(char *p)    { PrintLine(); ForcedEnd = 1; return p; }
char *ps_endsub(char *p) {            return EndSub(p); }
char *ps_fill(char *p)   { PrintPC(); return ParseFillData(p); }
char *ps_formln(char *p) { FormLn = atoi(p); PrintByteLine(FormLn); return p; }
char *ps_ignore(char *p) { PrintLine(); return p; }
char *ps_include(char *p){ PrintPC(); return IncludeFile(p); }
char *ps_list(char *p)   { PrintPC(); return ParseListOption(p); }
char *ps_load(char *p)   { PrintPC(); return ParseLoadData(p); }
char *ps_long(char *p)   { PrintPC(); return ParseLongData(p); }
char *ps_real(char *p)   { PrintPC(); return ParseRealData(p); }
char *ps_size(char *p)   { PrintPC(); return ListSizeInfo(p); }
char *ps_store(char *p)  {            return ParseStoreData(p); }
char *ps_string(char *p) { PrintPC(); return ParseByteData(p); }
char *ps_subr(char *p)   { PrintPC(); return ParseSubroutine(p); }
char *ps_word(char *p)   { PrintPC(); return ParseWordData(p); }

char *ps_align(char *p)
{
   int a;
   p = ExtractValue(p,&a);
   if (a > 0 && a <= 0x1000)
   {
      pc += (a - pc % a) % a;
   }
   PrintPCLine();
   return p;
}

// ******
// ps_org
// ******

char *ps_org(char *p)
{
   p = ExtractValue(p,&pc);
   PrintPCLine();
   return p;
}

// *****
// SetPC
// *****

char *SetPC(char *p)
{
   p = NeedChar(p,'=');
   if (p) return ps_org(p+1);

   ++ErrNum;
   ErrorMsg("Setting PC with \"* = address\" syntax error\n");
   exit(1);
}

char *ps_rmb(char *p)
{
   int size;
   p = ExtractValue(p,&size);
   if (size < 0)
   {
      ErrorMsg("Only theoretical physicists are allowed to reserve "
               "a negative amount of space: %d bytes\n", size);
      exit(1);
   }
   PrintPCLine();
   pc+=size;
   p += strlen(p);
   return p;
}

char *ps_sect(char *p)
{
   char *q;

   q = StrMatch(p,"LOC=");
   if (q) q = EvalOperand(q+4,&pc,0);
   PrintPCLine();
   return p;
}

char *ps_setdp(char *p)
{
   p = ExtractValue(p,&DP);
   if (DP > 255) DP >>= 8;      // alternate DP assignment
   PrintByteLine(DP);
   return p;
}

struct PseudoStruct
{
   const char *keyword;
   char *(*foo)(char *);
};

struct PseudoStruct PseudoTab[] =
{
   {"ALIGN"     , &ps_align  },
   {"BITS"      , &ps_bits   },
   {"BSS"       , &ps_bss    },
   {"BYTE"      , &ps_byte   },
   {"C5TO3"     , &ps_bit5   },
   {"CASE"      , &ps_case   },
   {"CMAP"      , &ps_cmap   },
   {"CPU"       , &ps_cpu    },
   {"END"       , &ps_end    },
   {"ENDMOD"    , &ps_endsub }, // alias to ENDSUB
   {"ENDSUB"    , &ps_endsub },
   {"EXTERN"    , &ps_ignore },
   {"FCB"       , &ps_byte   },
   {"FCC"       , &ps_string },
   {"FDB"       , &ps_word   },
   {"FILL"      , &ps_fill   },
   {"FORMLN"    , &ps_formln },
   {"INCLUDE"   , &ps_include},
   {"INTERN"    , &ps_ignore },
   {"LIST"      , &ps_list   },
   {"LOAD"      , &ps_load   },
   {"LONG"      , &ps_long   },
   {"MODULE"    , &ps_subr   }, // alias to SUBROUTINE
   {"ORG"       , &ps_org    },
   {"RMB"       , &ps_rmb    },
   {"REAL"      , &ps_real   },
   {"SECT"      , &ps_sect   },
   {"SETDP"     , &ps_setdp  },
   {"SIZE"      , &ps_size   },
   {"STORE"     , &ps_store  },
   {"SUBROUTINE", &ps_subr   },
   {"TTL"       , &ps_ignore },
   {"WORD"      , &ps_word   }
};

#define PSEUDOS (int)(sizeof(PseudoTab) / sizeof(struct PseudoStruct))

char *CheckPseudo(char *p)
{
   int i;

   p = SkipSpace(p);

   for (i=0 ; i < PSEUDOS ; ++i)
   if (!strcmpword(p,PseudoTab[i].keyword))
   {
      p = PseudoTab[i].foo(p+strlen(PseudoTab[i].keyword));
      if (pc > 0x10000)
      {
         ErrorMsg("Program counter overflow\n");
         ErrorLine(p);
         exit(1);
      }
      return NULL; // flag for pseudo op processed
   }
   return p;
}


void AddLabel(char *p)
{
   int i,l;

   if (Labels > MAXLAB -2)
   {
      ++ErrNum;
      ErrorMsg("Too many labels (> %d)\n",MAXLAB);
      exit(1);
   }
   l = strlen(p);

   // test for mnemonic

   for (i=0 ; i < DimOp ; ++i)
   {
      if (!StrCaseCmp(p,Mat[i].Mne))
      {
         ErrorMsg("Use of reserved mnemonic <%s> as label or operand\n",p);
         exit(1);
      }
   }

   // test for pseudos

   for (i=0 ; i < PSEUDOS ; ++i)
   {
      if (!StrCaseCmp(p,PseudoTab[i].keyword))
      {
         ErrorMsg("Use of reserved keyword <%s> as label or operand\n",p);
         exit(1);
      }
   }

   lab[Labels].Address = UNDEF;
   lab[Labels].Name = (char *)StrNDup(p,l);
   lab[Labels].Ref = (int *)MallocOrDie(sizeof(int));
   lab[Labels].Att = (int *)MallocOrDie(sizeof(int));
   lab[Labels].Ref[0] = LiNo;
   lab[Labels].Att[0] = 0;
   Labels++;
}


void SetInstructionLength(char *p)
{
   int i;

   // Store opcode

   if (oc >= 0)
   {
      if (oc < 256)  Put(pc,oc,p);
      else
      {
         if (df) fprintf(df,"Put ROM[%4.4x] = %4.4x\n",pc,oc);
         Put(pc  ,oc >> 8  ,p);
         Put(pc+1,oc & 0xff,p);
      }
   }

   // Lock instruction length

   if (pc >= 0 && pc < 0x10000)
   {
      if (ADL[pc] != 0 && ADL[pc] != il)
      {
         ErrorMsg("Phase error\n");
         ErrorLine(p);
         exit(1);
      }
      ADL[pc] = il;
      for (i=1 ; i < il ; ++i) ADL[pc+i] = -1;
   }
   if (df) fprintf(df,"lock oc = %4.2x il = %d ol = %d\n",oc,il,ol);
}


void Synchronize(void)
{
   nops = ADL[pc] - il;
   if (df) fprintf(df,"oc = %4.2x ol=%d ql=%d il=%d\n",oc,ol,ql,il);
   if (df && nops) fprintf(df,"Add %d NOP's\n",nops);
   il = ADL[pc];
   if (df) fprintf(df,"SYnc lock[%4.4x] = %d\n",pc,LOCK[pc]);
   if (nops) LOCK[pc] = 0;
}


void CheckSkip(void)
{
   int i;

   Skipping = 0;
   for (i=1 ; i <= IfLevel ; ++i) Skipping |= SkipLine[i];
}

int CheckCondition(char *p)
{
   int r,v,Ifdef,Ifndef,Ifval;
   r = 0;
   if (df) fprintf(df,"Check <%s>\n",p);
   if (*p == '#') ++p; // old syntax #if, #endif, etc.
   if (!strcmpword(p,"error") && (Phase == 1))
   {
      CheckSkip();
      if (Skipping) return 0;     // Include line in listing
      ErrorMsg("%s\n",p+6);
      exit(1);
   }
   Ifdef  = !strcmpword(p,"ifdef");
   Ifndef = !strcmpword(p,"ifndef");
   Ifval  = !strcmpword(p,"if");
   if (Ifdef || Ifndef || Ifval)
   {
      r = 1;
      IfLevel++;
      if (IfLevel > 9)
      {
         ++ErrNum;
         ErrorMsg("More than 10  IF or IFDEF conditions nested\n");
         exit(1);
      }
      if (Ifdef)
      {
         p = EvalOperand(p+6,&v,0);
         SkipLine[IfLevel] = v == UNDEF;
      }
      else if (Ifndef)
      {
         p = EvalOperand(p+7,&v,0);
         SkipLine[IfLevel] = v != UNDEF;
      }
      else // if (Ifval)
      {
         p = EvalOperand(p+3,&v,0);
         SkipLine[IfLevel] = v == UNDEF || v == 0;
      }
      CheckSkip();
      if (ListOn && Phase == 2)
      {
         PrintLiNo();
         if (SkipLine[IfLevel])
            fprintf(lf,"%4.4x FALSE    %s\n",SkipLine[IfLevel],Line);
         else
            fprintf(lf,"0000 TRUE     %s\n",Line);
      }
      if (df) fprintf(df,"%5d %4.4x          %s\n",LiNo,SkipLine[IfLevel],Line);
   }
   else if (!strcmpword(p,"else"))
   {
      r = 1;
      SkipLine[IfLevel] = !SkipLine[IfLevel];
      CheckSkip();
      PrintLiNo();
      if (ListOn && Phase == 2) fprintf(lf,"              %s\n",Line);
   }
   if (!strcmpword(p,"endif"))
   {
   if (df) fprintf(df,"inside Check endif\n");
      r = 1;
      IfLevel--;
      PrintLiNo();
      if (ListOn && Phase == 2) fprintf(lf,"              %s\n",Line);
      if (IfLevel < 0)
      {
         ++ErrNum;
         ErrorMsg("endif without if\n");
         exit(1);
      }
      CheckSkip();
      if (df) fprintf(df,"ENDIF SkipLevel[%d]=%d\n",IfLevel,SkipLine[IfLevel]);
   }
   return r;
}


int RegisterSize(int n)
{
   char r;

   if (!StrNCaseCmp(Mat[n].Mne,"LDMD",4)) return 1;
   if (!StrNCaseCmp(Mat[n].Mne,"CWAI",4)) return 1;

   r = Mat[n].Mne[strlen(Mat[n].Mne)-1];

   if (r == 'A' || r == 'B' || r == 'C' || r == 'E' || r == 'F') return 1;
   if (r == 'D' || r == 'X' || r == 'Y' || r == 'W') return 2;
   if (r == 'S' || r == 'U') return 2;
   if (r == 'Q') return 4;

   ErrorMsg("Illegal register name [%c]\n",r);
   exit(1);
}


char *ScanRegister(char *p, int *v)
{
   int i;
   char *q;

   for (i=15 ; i >= 0 ; --i)
   {
      if (!StrNCaseCmp(RegisterNames[i],p,strlen(RegisterNames[i]))) break;
   }
   if (i < 0)
   {
      ErrorLine(p);
      ErrorMsg("Unknown register name or wrong CPU set\n");
      exit(1);
   }
   *v = i;
   q = p + strlen(RegisterNames[i]);
   q = SkipSpace(q);
   if (*q == ',') ++q;
   return q;
}


char *TFMRegister(char *p, int *v)
{
   int i;

   for (i=4 ; i >= 0 ; --i)
   {
      if (RegisterNames[i][0] == toupper(*p)) break;
   }
   if (i < 0)
   {
      ErrorLine(p);
      ErrorMsg("Illegal register name for TFM or wrong CPU set\n");
      exit(1);
   }
   *v = i;
   return p+1;
}


void OperandError(char *p)
{
   ++ErrNum;
   ErrorLine(p);
   ErrorMsg("Syntax error in operand\n");
   exit(1);
}


int PostIndex(int reg,char *p)
{
   switch(*p)
   {
      case 'x':
      case 'X': if (reg < 0) reg = 0x00; else OperandError(p); break;
      case 'y':
      case 'Y': if (reg < 0) reg = 0x20; else OperandError(p); break;
      case 'u':
      case 'U': if (reg < 0) reg = 0x40; else OperandError(p); break;
      case 's':
      case 'S': if (reg < 0) reg = 0x60; else OperandError(p); break;
      default : OperandError(p);
   }
   return reg;
}


// ************
// PostIndexReg
// ************

int PostIndexReg(int reg,char *p)
{
   reg = PostIndex(reg,p);
   if (*(++p)) OperandError(p);
   return reg;
}

/* special postbyte codes for W register operand

   8F     ,W
   90    [,W]
   AF   ea,W
   B0  [ea,W]
   CF     ,W++
   D0    [,W++]
   EF   ,--W
   F0  [,--W]
*/

// **********
// PostIndexW
// **********

int PostIndexW(int reg,char *p)
{
   if (df) fprintf(df,"PostIndexW [%c]\n",*p);
   if (*p == 'w' || *p == 'W') return 0xf;
   return PostIndex(reg,p);
}

int SetPostByte(char *p, int *v)
{
   int inc,dec,reg,amo,off,ind,opl;

   ind =  0;  // indirect bit
   inc =  0;  // auto increment counter
   dec =  0;  // auto decrement counter
   reg = -1;  // index  register
   amo = -1;  // address mode bits
   off =  0;  // constant offset
   opl = strlen(p);

   // indirect

   if (df) fprintf(df,"indirect check %c %c\n",p[0],p[opl-1]);
   if (p[0] == '[' && p[opl-1] == ']')
   {
      ind = 0x10;
      p[opl-1] = 0;
      opl-=2;
      ++p;
      if (df) fprintf(df,"is indirect <%s>\n",p);
   }

   if (df && strlen(p) > 2)
      fprintf(df,"Check R,R: %c%c%c\n",toupper(p[0]),p[1],toupper(p[2]));

   // A,R

   if (toupper(p[0]) == 'A' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p+2);
      ql = 0;
      return (0x80 | reg | ind | 0x06);
   }

   // B,R

   if (toupper(p[0]) == 'B' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p+2);
      ql = 0;
      return (0x80 | reg | ind | 0x05);
   }

   // D,R

   if (toupper(p[0]) == 'D' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p+2);
      ql = 0;
      return (0x80 | reg | ind | 0x0b);
   }

   // E,R

   if (toupper(p[0]) == 'E' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p+2);
      ql = 0;
      return (0x80 | reg | ind | 0x07);
   }

   // F,R

   if (toupper(p[0]) == 'F' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p+2);
      ql = 0;
      return (0x80 | reg | ind | 0x0a);
   }

   // W,R

   if (toupper(p[0]) == 'W' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p+2);
      ql = 0;
      return (0x80 | reg | ind | 0x0e);
   }

   // PC relative

   if (df) fprintf(df,"check PC relative %d [%s],<%s>\n",opl,p,p+opl-3);
   if ((opl > 4 && StrNCaseCmp(p+opl-4,",PCR",4) == 0) ||
       (opl > 3 && StrNCaseCmp(p+opl-3,",PC" ,3) == 0))
   {
      if (df) fprintf(df,"check PC relative %s\n",p);
      p = EvalOperand(p,&off,0);
      off -= pc+3;
      if (ForcedMode < 0 || (off >= -128 && off < 128 && ROM[pc] != 0x8d))
      {
         ql = 1;
         *v = off;
         return (0x8c | ind);
      }
      else
      {
         ql = 2;
         *v = off-1;
         return (0x8d | ind);
      }
   }

   // offset

   if (*p != ',')
   {
      p = EvalOperand(p,&off,0);
   }

   // zero offset

   if (*p == ',' && off == 0)
   {
      while (*(++p) == '-') ++dec;
      reg = PostIndexW(reg,p);
      if (df) fprintf(df,"zero offset reg=%2.2x\n",reg);
      while (*(++p) == '+') ++inc;
      if (reg <  0) OperandError(p);
           if (inc == 1 && dec == 0) amo = 0x00;
      else if (inc == 2 && dec == 0) amo = 0x01;
      else if (inc == 0 && dec == 1) amo = 0x02;
      else if (inc == 0 && dec == 2) amo = 0x03;
      else if (inc == 0 && dec == 0) amo = 0x04;
      else OperandError(p);
      ql = 0; // no address
      if (reg == 0xf) // W register
      {
              if (amo == 4) reg = 0x8f; // ,W
         else if (amo == 1) reg = 0xcf; // ,W++
         else if (amo == 3) reg = 0xef; // ,--W
         else OperandError(p);
         if (ind) reg +=1;
         if (df) fprintf(df,"W pb = %2.2x ind = %2.2x\n",reg,ind);
         return reg;
      }
      return (0x80 | reg | ind | amo);
   }

   // constant offset

   if (*p == ',')
   {
      if (df) fprintf(df,"constant off = %x\n",off);
      *v = off;
      reg = PostIndexW(reg,++p);

      if (reg == 0xf) // W register
      {
         ql = 2;
         if (ind) return 0xb0;
         else     return 0xaf;
      }
                                             // 5 bit offset
      if (ForcedMode <= 0 && off >= -16 && off < 16 && ind == 0)
      {
         ql = 0; // no following bytes
         return (reg | (off & 0x1f));
      }                                      // 8 bit offset

      if (ForcedMode <  0 || (off >= -128 && off < 128))
      {
         ql = 1; // one following byte
         return (0x80 | reg | ind | 0x08);
      }

      // 16 bit offset

      ql = 2; // two following bytes
      return (0x80 | reg | ind | 0x09);
   }

   OperandError(p);
   return -1;
}


int ScanPushList(char *p)
{
   int i,l,v;
   char *Reg;

   if (!strcmpword(p,"ALL")) return 0xff;
   v = 0;
   while (*p)
   {
      for (i=9 ; i >= 0 ; --i) // Scan DP before D
      {
         Reg = PushList[i].Reg;
         l = strlen(Reg);
         if (df) fprintf(df,"push list [%s] <%s>\n",p,Reg);
         if (!strcmpword(p,Reg)) break;
      }
      if (i < 0) OperandError(p);
      v |= PushList[i].Val;
      p = SkipSpace(p+l);
      if (*p != ',' && *p != 0) OperandError(p);
      if (*p == ',') ++p;
      p = SkipSpace(p);
   }
   return v;
}


char *GenerateCode(char *p)
{
   int ibi = 0; // instruction byte index
   int i,l,v,rd;
   int r1,r2,qc,XIM;
   char *q;
   char *rop;   // rest of operand
   char p1,p2;  // post increment

   // initialize

   v   = UNDEF;
   pb  = -1;
   ql  =  0;

   p = SkipSpace(p);

   if (pc < 0)
   {
      ErrorLine(p);
      ErrorMsg("Undefined program counter (PC)\n");
      exit(1);
   }

   // immediate data to memory

   XIM = (Mat[MneIndex].Mne[1] == 'I' && Mat[MneIndex].Mne[2] == 'M');
   if (XIM)
   {
      if (*p == '#') ++p;
      else
      {
         ErrorLine(p);
         ErrorMsg("Immediate operand must start with '#'\n");
         exit(1);
      }
      p   = EvalOperand(p,&v,0);

      // make pseudo 16 bit opcode with embedded immediate value

      XIM = ((Mat[MneIndex].Opc[AM_Extended] << 8) | (v & 0xff));
      ol  = 2;
      if (*p == ',') ++p;
      else
      {
         ErrorLine(p);
         ErrorMsg("Immediate value must be followed by comma\n");
         exit(1);
      }
      v = UNDEF;
      oc = XIM;
   }

   // inherent instruction (no operand)

   if ((oc = Mat[MneIndex].Opc[AM_Inherent]) >= 0)
   {
      ol = il = 1 + (oc > 255); // instruction length
      p += strlen(p) ;          // ignore rest
   }

   // check for missing operand

   else if (OpText[0] == 0)
   {
      ++ErrNum;
      ErrorLine(p);
      ErrorMsg("Missing operand\n");
      exit(1);
   }

   // illegal operand conditions

   else if (OpText[0] == 0x27) // apostrophe
   {
      ErrorLine(p);
      ErrorMsg("Operand cannot start with apostrophe\n");
      exit(1);
   }

   // register address mode

   else if ((oc = Mat[MneIndex].Opc[AM_Register]) >= 0)
   {

      // push pull

      if (Mat[MneIndex].Mne[0] == 'P')
      {
         ol = 1 + (oc > 255); // instruction length
         il = ol + 1;
         pb  = ScanPushList(p);
         p += strlen(p) ;          // ignore rest
      }

      // TFM

      else if (!StrnCmp(Mat[MneIndex].Mne,"TFM",3))
      {
         p1 = p2 = 0;
         ol = 1 + (oc > 255); // instruction length
         il = ol + 1;
         q = TFMRegister(OpText,&r1);
         if (*q == '+' || *q == '-') p1 = *q++;
         if (*q == ',') q++;
         else
         {
            ErrorLine(p);
            ErrorMsg("Missing comma\n");
            exit(1);
         }
         q = TFMRegister(q     ,&r2);
         if (*q == '+' || *q == '-') p2 = *q++;
         pb = (r1 << 4) | r2;
         p += strlen(p) ;          // ignore rest

               if (p1 == '+' && p2 == '+') oc = 0x1138;
         else  if (p1 == '-' && p2 == '-') oc = 0x1139;
         else  if (p1 == '+' && p2 ==  0 ) oc = 0x113a;
         else  if (p1 ==  0  && p2 == '+') oc = 0x113b;
         else
         {
            ErrorLine(p);
            ErrorMsg("Illegal increment/decrement combination\n");
            exit(1);
         }
      }

      // register register

      else
      {
         ol = 1 + (oc > 255); // instruction length
         il = ol + 1;
         q = ScanRegister(OpText,&r1);
         q = ScanRegister(q     ,&r2);
         if (r1 != 13 && r2 != 13 && ((r1 < 8 && r2 > 7) || (r1 > 7 && r2 < 8)))
         {
            ErrorLine(p);
            ErrorMsg("mixing register of different sizes\n"
                     "register %-2.2s is %2d bit\n"
                     "register %-2.2s is %2d bit\n",
                     RegisterNames[r1],8 + 8 * (r1 < 8),
                     RegisterNames[r2],8 + 8 * (r2 < 8));
            exit(1);
         }
         pb = (r1 << 4) | r2;
         p += strlen(p) ;          // ignore rest
      }
   }

   // relative address mode

   else if ((oc = Mat[MneIndex].Opc[AM_Relative]) >= 0)
   {
      ol = 1 + (oc > 255); // operand length = opcode length
      ql = 1 + (Mat[MneIndex].Mne[0] == 'L');
      il = ol + ql;
      if (OpText[0] == '-') // local backward label
      {
         l = strlen(OpText);
         for (i=0 ; i < l ; ++i)
         if (OpText[i] != '-') break;
         if (i == l) v = minlab[l];
         else
         {
            rop = EvalOperand(OpText,&v,0);
            if (*rop)
            {
               ErrorLine(rop);
               ErrorMsg("Extra text after branch operand\n");
               exit(1);
            }
         }
      }
      else if (OpText[0] == '+') // local forward label
      {
         l = strlen(OpText);
         for (i=0 ; i < l ; ++i)
         if (OpText[i] != '+') break;
         if (i == l)
         {
            v = UNDEF;
            i = plucnt[l];
            while (--i >= 0 && plulab[l][i] > pc) v = plulab[l][i];
         }
         else EvalOperand(OpText,&v,0);
      }
      else EvalOperand(OpText,&v,0);
      if (v != UNDEF) v  -= (pc + il);
      if (Phase == 2 && v == UNDEF)
      {
         ErrorLine(p);
         ErrorMsg("Branch to undefined label\n");
         exit(1);
      }

      if (Optimize)
      {
         // fix short branch to long branch

         if (v < -128 && oc >= 0x20 && oc < 0x30)
         {
            if (Phase == 1 || (Phase == 2 && ADL[pc] >= 3))
            {
               if (oc == 0x20) // BRA
               {
                  oc = 0x16;   // LBRA
                  ol = 1;
               }
               else
               {
                  oc |= 0x1000; // short branch -> long branch
                  ol  = 2;
               }
               ql = 2;
               il = ol + ql;
            }
         }

         // optimize long branch to short branch

         if (v >= -128 && v < 0 && oc > 0x1020 && oc < 0x1030)
         {
            if (Phase == 1 || (Phase == 2 && ADL[pc] == 2))
            {
               oc &= 0xff; // long branch -> short branch
               ol  = 1;
               ql  = 1;
               il  = 2;
               if (Phase == 2)
               {
                  optc++;
                  fprintf(of,"%4s %4.4x   -->   %3s %2.2x:%5d %s\n",
                  Mat[MneIndex].Mne,v,Mat[MneIndex].Mne+1,v,LiNo,Line);
                  strcpy(Hint," ; ");
                  strcat(Hint,Mat[MneIndex].Mne+1);
               }
            }
         }

         // optimize LBRA to BRA

         if (v >= -128 && v < 0 && oc == 0x16)
         {
            if (Phase == 1 || (Phase == 2 && ADL[pc] == 2))
            {
               oc  = 0x20; // BRA
               ol  = 1;
               ql  = 1;
               il  = 2;
            }
         }
      }

      if (Phase == 2 && ql == 1 && (v < -128 || v > 127))
      {
         ErrorLine(p);
         ErrorMsg("Short Branch out of range (%d)\n",v);
         exit(1);
      }
      if (df) fprintf(df,"branch %4.4x -> %4.4x : %4.4x\n",pc,v,v-pc-il);

      if (Optimize)
      {
         if (Phase == 2 && ql == 2 && v >= -128 && v < 128)
         {
            optc++;
            fprintf(of,"%4s %4.4x   ***   %3s %2.2x:%5d %s\n",
               Mat[MneIndex].Mne,v,Mat[MneIndex].Mne+1,v,LiNo,Line);
         }
      }
      v &= 0xffff;
      p += strlen(p) ;          // ignore rest
   }

   // immediate address mode

   else if (*p == '#')
   {
      oc = Mat[MneIndex].Opc[AM_Immediate];
      if (oc < 0)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal immediate instruction %s %s\n",Mat[MneIndex].Mne,OpText);
         exit(1);
      }
      rop = EvalOperand(OpText+1,&v,0);
      if (*rop)
      {
         ErrorLine(rop);
         ErrorMsg("Extra text after operand\n");
         exit(1);
      }
      ol = 1 + (oc > 255);
      ql = RegisterSize(MneIndex);
      if (ql == 4 && oc != 0xcd) ql = 2; // only LDQ immediate has 32 bit value
      il = ol + ql;
      if (Phase == 2 && v == UNDEF)
      {
         ErrorLine(p);
         ErrorMsg("Undefined immediate value\n");
         exit(1);
      }
      if (ql == 1 && Phase == 2 && (v < -128 || v > 255))
      {
         ErrorLine(p);
         ErrorMsg("Immediate value out of range (%d)\n",v);
         exit(1);
      }
      if (ql == 2 && Phase == 2 && (v < -32768 || v > 0xffff))
      {
         ErrorLine(p);
         ErrorMsg("Immediate value out of range (%d)\n",v);
         exit(1);
      }
      p += strlen(p) ;          // ignore rest
   }

   // indirect indexed

   else if (OpText[0] == '[')
   {
      l = strlen(OpText);
      if (OpText[l-1] != ']')
      {
         ErrorLine(p);
         ErrorMsg("Missing closing bracket ]\n");
         exit(1);
      }
      oc = Mat[MneIndex].Opc[AM_Indexed];
      if (oc < 0)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal instruction %s %s\n",Mat[MneIndex].Mne,OpText);
         exit(1);
      }
      if (!strchr(OpText+1,',')) // indirect address
      {
         p = EvalOperand(OpText+1,&v,0);
         pb = 0x9f;
         ql = 2;
         il = ol + 3;
      }
      else
      {
         pb = SetPostByte(OpText,&v) | 0x10;
         ol = 1 + (oc > 255); // opcode length
         il = ol + 1 + ql;    // opcode + postbyte + address
      }
   }

   // register - direct page bit operation

   else if (strchr(p,',') && strchr(p,'.'))
   {
      if (df) fprintf(df,"Check bit op <%s>\n",p);
      oc = Mat[MneIndex].Opc[AM_Direct];
      if (oc < 0)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal bit operation %s %s\n",Mat[MneIndex].Mne,OpText);
         exit(1);
      }
      pb = 0xc0; // invalid
      if (!StrNCaseCmp(p,"CC.",3))
      {
         pb = 0x00;
         p += 3;
      }
      else if (!StrNCaseCmp(p,"A.",2))
      {
         pb = 0x40;
         p += 2;
      }
      else if (!StrNCaseCmp(p,"B.",2))
      {
         pb = 0x80;
         p += 2;
      }
      else
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal register in bit operation %s %s\n",Mat[MneIndex].Mne,OpText);
         exit(1);
      }
      i = *p++ - '0';
      if (i < 0 || i > 7)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal bit# %d\n",i);
         exit(1);
      }
      pb |= i; // add target bit
      p = strchr(p,','); // skip after comma
      q = strrchr(p,'.'); // search bit field
      if (!p || !q)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal syntax in bit operand\n");
         exit(1);
      }
      *q = 0; // separate bit number from address
      p = EvalOperand(p+1,&v,0);
      if (v != UNDEF && (v < 0 || v > 255))
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal address %d\n",v);
         exit(1);
      }
      i = q[1] - '0';
      if (i < 0 || i > 7)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal bit# %d\n",i);
         exit(1);
      }
      pb |= (i<<3); // add source bit
      ol = 2;
      ql = 1;
      il = 4;
   }

   // indexed address mode

   else if (strchr(p,','))
   {
      if (XIM) oc = XIM & 0xefff; // extended -> indexed
      else oc = Mat[MneIndex].Opc[AM_Indexed];
      if (df) fprintf(df,"indexed am oc = %4.4x\n",oc);
      if (oc < 0)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal indexed instruction %s %s\n",Mat[MneIndex].Mne,OpText);
         exit(1);
      }
      pb = SetPostByte(p,&v);

      if (XIM) ol = 2;
      else     ol = 1 + (oc > 255); // opcode length
      il = ol + 1 + ql;    // opcode + postbyte + address
   }

   // all other

   else
   {
      p = EvalOperand(p,&v,0);

      if (XIM)
      {
         ol = 2;
         if (v > 255)
         {
            oc = XIM;
            ql = 2;
         }
         else
         {
            oc = XIM & 0xfff; // extended -> direct
            ql = 1;
         }
         il = ol + ql;
         if (df) fprintf(df,"XIM oc = %4.4x  v = %4.4x il = %d\n",oc,v,il);
      }

      if (Phase == 2) // opcode and instruction length is set in phase 1
      {
         if (XIM)
         {
            oc = (ROM[pc] << 8) + ROM[pc+1];
            il = ADL[pc];
            ql = il - ol;
            if (df) fprintf(df,"ROM oc = %4.4x  v = %4.4x\n",oc,v);
         }
         else
         {
            oc = ROM[pc];
            if (df) fprintf(df,"ROM oc = %4.4x  v = %4.4x\n",oc,v);
            ol = 1 + (oc == 0x10 || oc == 0x11);
            if (ol == 2) oc = (oc << 8) | ROM[pc+1];
            il = ADL[pc];
            ql = il - ol;
            if (ForcedMode < 0 || ql == 1) v &= 0xff;
         }
      }
      else
      {
         // extended address mode

         if (!XIM) oc = Mat[MneIndex].Opc[AM_Extended];
         if (oc >= 0)
         {
            // assume extended mode

            ol = 1 + (oc > 255); // opcode  length
            ql = 2;              // operand length
            il = ol + 2;         // instruction length

            // direct address mode

            if (ForcedMode <= 0) // not forced extended
            {
               if (XIM) qc = oc & 0xfff;
               else     qc = Mat[MneIndex].Opc[AM_Direct];

               if (qc >= 0 && (ForcedMode < 0 ||
                  (v != UNDEF && (v >> 8) == DP)))
               {
                  oc = qc;
                  v &= 0xff;
                  ql = 1;
                  il = ol + 1;
      if (XIM && df) fprintf(df,"XIM3 oc = %4.4x  v = %4.4x il = %d\n",oc,v,il);
               }
            }
         }
         else
         {
            ++ErrNum;
            ErrorLine(p);
            ErrorMsg("Illegal instruction %s %s\n",Mat[MneIndex].Mne,OpText);
            exit(1);
         }
      }

      if (XIM && df) fprintf(df,"XIM2 oc = %4.4x  v = %4.4x il = %d\n",oc,v,il);
      if (Optimize)
      {
          // optimise JSR to BSR

          rd = v - pc - 3;
          if (Phase == 2 && oc == 0xbd && rd >= -128 && rd < 128)
          {
             optc++;
             fprintf(of," JSR %4.4x   ***   BSR %2.2x:%5d %s\n",
                     v,rd&0xff,LiNo,Line);
          }

         // optimize JMP to BRA

         rd = v - pc - 3;
         if (rd >= -128 && rd < 0)
         {
            if (Phase == 1 && oc == 0x7e)
            {
               oc = 0x20; // BRA
               ol =  1;
               ql =  1;
               il =  2;
               v  = rd;
            }
            if (Phase == 2 && oc == 0x20)
            {
               optc++;
               fprintf(of," JMP %4.4x   -->   BRA %2.2x:%5d %s\n",
                       v,rd&0xff,LiNo,Line);
               strcpy(Hint," ; BRA");
               ol =  1;
               ql =  1;
               il =  2;
               v  = rd;
            }
         }
      }
   }

   if (Phase == 1) SetInstructionLength(p);

   if (Phase == 2)
   {
      Synchronize();
      if (v == UNDEF && ql > 0)
      {
         ErrorLine(p);
         ErrorMsg("Use of an undefined label\n");
         exit(1);
      }

      // insert binary code

      if (df) fprintf(df,"PUT OC = %4.4x\n",oc);
      if (oc > 255) // two byte opcode
      {
         Put(pc,oc >> 8,p);
         Put(pc+1,oc,p);
         ibi = 2;
      }
      else
      {
         Put(pc,oc,p);
         ibi = 1;
      }

      if (pb >= 0) // post byte
      {
         Put(pc+ibi++,pb,p);
      }

      if (ql == 4) // 32 bit value
      {
         Put(pc+ibi++,v >> 24,p);
         Put(pc+ibi++,v >> 16,p);
         Put(pc+ibi++,v >>  8,p);
         Put(pc+ibi++,v      ,p);;
      }

      if (ql == 2) // 16 bit value
      {
         if (v > 0xffff || v < -32768)
         {
            ErrorLine(p);
            ErrorMsg("16 bit address/value out of range\n");
            exit(1);
         }
         Put(pc+ibi++,v >> 8,p);
         Put(pc+ibi++,v     ,p);
      }

      if (ql == 1) //  8 bit value
      {
         if (v >= 0xff00 && v <= 0xffff) v &= 0xff;
         if ((v-(DP<<8)) < 256 && (v-(DP<<8)) >= -128) v -= DP<<8;
         if (v > 255 || v < -128)
         {
            printf("v = %x  DP = %x\n",v,DP);
            ErrorLine(p);
            ErrorMsg("8 bit address/value out of range\n");
            exit(1);
         }
         Put(pc+ibi++,v,p);
      }

      for (i=0 ; i < nops ; ++i) Put(pc+ibi++,0x12,p); // NOP

      if (ListOn)
      {
         PrintPC();
         PrintOC(v);
         fprintf(lf," %s",Line);
         if (Hint[0])
         {
            fprintf(lf,"%s",Hint);
            Hint[0] = 0;
         }

         if (nops && df) fprintf(df,"Added %d NOP's\n",nops);
         if (nops >  1 && lf) fprintf(lf," ; added %d NOP's",nops);
         if (nops == 1 && lf) fprintf(lf," ; added a NOP");
      }
   }

   if (il < 1 || il > 5)
   {
      ++ErrNum;
      ErrorMsg("Wrong instruction length = %d\n",il);
      il = 1;
   }

   if (pc+il > 0xffff)
   {
      if (Phase > 1)
      {
         ++ErrNum;
         ErrorMsg("Program counter exceeds 64 KB\n");
      }
   }
   else pc += il;
   return p;
}

#define MAXARGS 10

// *************
// ScanArguments
// *************

// for nargs == MAXARGS this is a recording call
// otherwise it's an expansion call

int ScanArguments(char *p, char *args, int ptr[], int nargs)
{
   int l,n;
   char sym[ML];

   if (df) fprintf(df,"Scan Args %d <%s>\n",nargs,p);
   n = 0;
   ptr[0] = 0;
   while (*p && n < nargs)
   {
      if (df) fprintf(df,"Arg #%d <%s>\n",n,p);
      p = SkipSpace(p);
      if (*p == ')') break; // end of list
      if (nargs == MAXARGS) p = GetSymbol(p,sym);
      else                  p = GetMacroArg(p,sym);
      l = strlen(sym);
      if (l) memmove(args+ptr[n],sym,l+1);
      else   args[ptr[n]] = 0;
      ++n;
      ptr[n] = ptr[n-1] + l + 1;
      p = SkipSpace(p);
      if (*p == ')') break; // end of list
      if (*p != ',' && n < nargs-1 && nargs < MAXARGS)
      {
         ++ErrNum;
         ErrorMsg("Syntax error in macro definition '%c'\n",*p);
         exit(1);
      }
      if (*p == ',') ++p;
   }
   return n;
}


// Called after name MCRO returns # of args

int ScanArgs(char *p, char *args, int ptr[])
{
   int l,n;
   char sym[ML];

   n = 0;
   ptr[0] = 0;
   // if (df) fprintf(df,"Inside ScanArgs: <%s>\n",p);
   while (*p && n < 10)
   {
      p = SkipSpace(p);
      if (*p == ';') break; // comment
      p = NextSymbol(p,sym);
      l = strlen(sym);
      // if (df) fprintf(df,"ScanSym:<%s>[%d]\n",sym,l);
      if (l) memmove(args+ptr[n],sym,l+1);
      else   args[ptr[n]] = 0;
      ++n;
      ptr[n] = ptr[n-1] + l + 1;
      p = SkipSpace(p);
      if (*p != ',') break; // end of list
      ++p; // skip comma
   }
   return n;
}

void MacroInfo(int n)
{
   fprintf(df,"-----------------\n");
   fprintf(df,"Name: %s\n",Mac[n].Name);
   fprintf(df,"Args: %d\n",Mac[n].Narg);
   fprintf(df,"Cola: %d\n",Mac[n].Cola);
   fprintf(df,"Type: %d\n",Mac[n].Type);
   fprintf(df,"Body: <<<%s>>>\n",Mac[n].Body);
   fprintf(df,"-----------------\n");
}

void RecordMacro(char *p)
{
   char Macro[ML];
   int i,j,l,al,an,bl,mf;
   int ap[10];
   char args[ML];
   char Buf[ML];
   char *b;
   char *at;

   if (Macros > MAXMAC -2)
   {
      ++ErrNum;
      ErrorMsg("Too many macros (> %d)\n",MAXMAC);
      exit(1);
   }

   bl = 1;
   mf = strcmpword(p,"MACRO") != 0; // 1 : name MACRO
   if (!mf) p += 5;                 // 0 : MACRO name
   if (df) fprintf(df,"macro type = %d\n",mf);

   p = NextSymbol(p,Macro);
   l = strlen(Macro);
   if (mf) p = StrMatch(p,"MACRO") + 5;
   p = SkipSpace(p);

   if (df)
   {
      fprintf(df,"Macro name: <%s>\n",Macro);
      fprintf(df,"Arglist: <%s>\n",p);
   }

   if (*p == '(') ++p;
   if (mf) an = ScanArgs(p,args,ap);
   else    an = ScanArguments(p,args,ap,MAXARGS);
   if (df)
   {
      fprintf(df,"RecordMacro: %s(",Macro);
      for (i=0 ; i < an ; ++i)
      {
         at = args + ap[i];
         al = strlen(at);
         fprintf(df,"%s[%d]",at,al);
         if (i < an-1) fprintf(df,",");
      }
      fprintf(df,")\n");
   }
   j = MacroIndex(Macro);
   if (j < 0)  // create new entry in macro table
   {
      j = Macros;
      Mac[j].Name = (char *)StrNDup(Macro,l);
      Mac[j].Narg = an;
      Mac[j].Type = mf;
      fgets(Line,sizeof(Line),sf);
      while (!feof(sf) && !StrCaseStr(Line,"ENDM"))
      {
         ++LiNo;
         l = strlen(Line);
         if (l && Line[l-1] == 10) Line[--l] = 0; // Remove linefeed
         if (l && Line[l-1] == 13) Line[--l] = 0; // Remove return

         // parse line and substitute arguments

         p = Line;
         b = Buf;
         while (*p)
         {
            for (i=0 ; i < an ; ++i)
            {
               at = args + ap[i];
               al = strlen(at);
               if (al && !StrnCmp(p,at,al))
               {
                  *b++ = CHAMAC;
                  *b++ = '0' + i;
                  p += al;
                  break;
               }
            }
            if (i == an) *b++ = *p++;
         }
         *b++ = '\n';
         *b = 0;

         if (df)
         {
            fprintf(df,"MAC line  :%s\n",Line);
            fprintf(df,"MAC parsed:%s\n",Buf);
         }
         l = strlen(Buf);
         if (bl == 1)
         {
            bl = l+1;
            Mac[j].Body = (char *)StrNDup(Buf,l);
         }
         else
         {
            bl += l;
            Mac[j].Body = (char *)ReallocOrDie(Mac[j].Body,bl);
            strcat(Mac[j].Body,Buf);
         }
         fgets(Line,sizeof(Line),sf);
      }
      Macros++;
      if (df) fprintf(df,"finished macro %d\n",Macros);
   }
   else if (Phase == 2) // List macro
   {
      PrintLiNo();
      ++LiNo;
      if (ListOn) fprintf(lf,"            %s\n",Line);
      do
      {
         fgets(Line,sizeof(Line),sf);
         PrintLiNo();
         ++LiNo;
         if (ListOn) fprintf(lf,"            %s",Line);
         if (pf) fprintf(pf,"%s",Line);
      } while (!feof(sf) && !StrCaseStr(Line,"ENDM"));
      LiNo-=2;
   }
   else if (Phase == 1)
   {
      ++ErrNum;
      ErrorMsg("Duplicate macro [%s]\n",Macro);
      exit(1);
   }
   if (df) MacroInfo(j);
   ++LiNo;
}


int ExpandMacro(char *m)
{
   int j,an;
   char *p;
   char Macro[ML];

   j = MacroIndex(m);
   if (j < 0) return j;
   if (df) fprintf(df,"\nExpanding [%s] phase %d\n",Mac[j].Name,Phase);

   p = NextSymbol(m,Macro);
   p = SkipSpace(p);
   if (*p == '(') ++p;  // parenthesis are optional

   ExtractOpText(p);    // separate arguments from comment
   p  = OpText;
   an = ScanArguments(p,MacArgs,ArgPtr,Mac[j].Narg);

   if (an != Mac[j].Narg)
   {
      ++ErrNum;
      ErrorMsg("Wrong # of arguments in [%s] called (%d) defined (%d)\n",
            Macro,an,Mac[j].Narg);
      exit(1);
   }
   ++MacLev;
   MacPtr[MacLev] = Mac[j].Body;
   if (df) fprintf(df,"Macro Level:%d\n",MacLev);
   if (df) fprintf(df,"Macro Body :<<<%s>>>\n",Mac[j].Body);

   if (Phase == 2)
   {
      Mac[j].Cola = m - Line;
      PrintLine();
   }
   return j;
}


void NextMacLine(char *w)
{
   int i;
   char *r;

   if (df) fprintf(df,"Next Macro Line:%s\n",w);

   // do not count macro expansion lines

   --LiNo;

   // check for end of macro body

   while (MacLev > 0 && *MacPtr[MacLev] == 0) --MacLev;

   if (df) fprintf(df,"MacPtr[%d] = {{%s}}\n",MacLev,MacPtr[MacLev]);

   if (MacPtr[MacLev] && *MacPtr[MacLev])
   {
      while (*MacPtr[MacLev] &&
             *MacPtr[MacLev] != '\n')
      {
         if (*MacPtr[MacLev] == CHAMAC)
         {
            i = *(++MacPtr[MacLev]) - '0';
            r = MacArgs + ArgPtr[i];
            while (*r) *w++ = *r++;
            ++MacPtr[MacLev];
         }
         else *w++ = *MacPtr[MacLev]++;
      }
      if (*MacPtr[MacLev] == '\n') ++MacPtr[MacLev];
   }
   *w = 0;
}


void ParseLine(char *cp)
{
   int i,l,v,m;
   char *start;

   am = -1;
   oc = -1;
   Label[0] = 0;
   OpText[0] = 0;
   Comment[0] = 0;
   cp = SkipHexCode(cp);        // Skip disassembly
   cp = SkipSpace(cp);          // Skip leading blanks
   start = cp;                  // Remember start of line
   if (df) fprintf(df,"%5d %4.4x Parse[%d]:%s\n",LiNo,pc&0xffff,Phase,cp);
   if (CheckCondition(cp)) return;
   if (Skipping)
   {
      PrintLiNo();
      if (ListOn && Phase == 2) fprintf(lf,"SKIP          %s\n",Line);
      if (df)         fprintf(df,"%5d SKIP          %s\n",LiNo,Line);
      return;
   }
   if (pf && Phase == 2 && !MacLev)
   {
       fprintf(pf,"%s\n",Line); // write to preprocessed file
   }
   if (strncmp(cp,"/*",2) == 0 || strncmp(cp,"\\*",2) == 0) // SDDRIVE comment style
   {
      CodeStyle = 1;
      if (Phase == 2) PrintLine();
      return;
   }
   if (*cp == 0)  // Empty
   {
      if (Phase == 2)
      {
          PrintLiNo();
          fputc('\n',lf);
      }
      return;
   }
   if (*cp == ';')  // comment only
   {
      PrintLine();
      return;
   }

   if (*cp == '*')  // comment or setting of PC ?
   {
      if (!NeedChar(cp+1,'='))
      {
         PrintLine();  // no "* =" syntax
         return;
      }
   }

   // set local backward label

   if (*cp == '-')
   {
      l = strlen(cp);
      i = 0;
      while (*cp++ == '-' && i < 10 && i < l) ++i;
      minlab[i] = pc;
   }

   // set local forward label

   if (*cp == '+')
   {
      l = strlen(cp);
      i = 0;
      while (*cp++ == '+' && i < 10 && i < l) ++i;
      if (Phase == 1)
      {
         plulab[i][plucnt[i]] = pc;
         if (++plucnt[i] > PLUMAX-2)
         {
            ++ErrNum;
            ErrorMsg("too many local labels\n");
            exit(1);
         }
      }
   }

   cp = CheckPseudo(cp);
   if (!cp) return;      // Pseudo Op successfull processed
   if (*cp == '.' || *cp == '_' || isalpha(*cp)) // Macro, Label or mnemonic
   {
      if (StrMatch(cp,"MACRO"))
      {
         RecordMacro(cp);
         return;
      }
      if ((MneIndex = IsInstruction(cp)) < 0)
      {
         m = ExpandMacro(cp);
         if (m < 0) // not a macro
         {
            if (df) fprintf(df,"LABEL:%s:\n",cp);
            if (df) fprintf(df,"start:%s:\n",start);
            if (cp == start) cp = DefineLabel(cp,&v,0);
            else
            {
               if (StrKey(cp,"SET") || StrKey(cp,"ENUM") ||
                   StrKey(cp,"EQU") || strchr(cp,'='))
                  cp = DefineLabel(cp,&v,0);
            }
            cp = SkipSpace(cp);         // Skip leading blanks
            if (*cp) m = ExpandMacro(cp);   // Macro after label
            if (m >= 0) cp += strlen(cp);         // advance to EOL
         }
         else cp += strlen(cp);         // advance to EOL
         if (m < 0 && (*cp == 0 || *cp == ';')) // no code or data
         {
            PrintLiNo();
            if (ListOn && Phase == 2)
               fprintf(lf,"%4.4x              %s\n",v&0xffff,Line);
            return;
         }
      }
   }
   if (ForcedEnd)  return;
   if (*cp ==  0 ) return;             // No code
   if (*cp == ';') return;             // No code
   if (*cp == '&') { cp = SetBSS(cp+1); return; }    // Set BSS counter
   if (*cp == '*') { cp = SetPC(cp+1) ; return; }    // Set PC
   cp = CheckPseudo(cp);
   if (!cp) return;      // Pseudo Op successfull processed
   if (MneIndex < 0) MneIndex = IsInstruction(cp); // Check for mnemonic after label
   if (MneIndex >= 0)
   {
      ExtractOpText(cp+strlen(Mat[MneIndex].Mne));
      cp += strlen(cp);
      GenerateCode(OpText);
   }
   if (ListOn && Phase == 2) fprintf(lf,"\n");
   if (*cp == 0 || *cp == ';' || *cp == '*') return; // end of code

   printf("<%s>\n",cp);
   ++ErrNum;
   ErrorLine(cp);
   ErrorMsg("Syntax error\n");
   exit(1);
}

void Phase1Listing(void)
{
   fprintf(df,"%5d %4.4x",LiNo,pc);
   if (Label[0]) fprintf(df,"  Label:[%s]{%4.4x}",Label,lab[LabelIndex(Label)].Address);
   if (OpText[0]) fprintf(df,"  %s",OpText);
   if (Comment[0]) fprintf(df,"  %s",Comment);
   fprintf(df,"\n");
}


int CloseInclude(void)
{
   const char *msg = "Close INCLUDE file";

   PrintLiNo();
   if (Phase == 2)
   {
      if (ListOn) fprintf(lf,";                       closed INCLUDE file %s\n",
            IncludeStack[IncludeLevel].Src);
      if (ferror(lf)) AssertFileOp(NULL, msg);
   }
   if (fclose(sf)) AssertFileOp(NULL, msg);
   free(IncludeStack[IncludeLevel].Src);
   sf = IncludeStack[--IncludeLevel].fp;
   LiNo = IncludeStack[IncludeLevel].LiNo;
   fgets(Line,sizeof(Line),sf);
   ForcedEnd = 0;
   return feof(sf);
}

void Phase1(void)
{
    int i,l,Eof;

   Phase = 1;
   ForcedEnd = 0;
   for (i=0 ; i < 11 ; ++i) minlab[i] = UNDEF;

   fgets(Line,sizeof(Line),sf);
   Eof = feof(sf);
   while (!Eof || IncludeLevel > 0)
   {
      ++LiNo; ++TotalLiNo;
      l = strlen(Line);
      if (l && Line[l-1] == 10) Line[--l] = 0; // Remove linefeed
      if (l && Line[l-1] == 13) Line[--l] = 0; // Remove return
      ParseLine(Line);
      if (MacLev)
      {
         NextMacLine(Line);
         if (df) fprintf(df,"Macro: %s\n",Line);
      }
      else
      {
         fgets(Line,sizeof(Line),sf);
      }
      Eof = feof(sf) || ForcedEnd;;
      if (Eof && IncludeLevel > 0) Eof = CloseInclude();
   }
}


void Phase2(void)
{
   int i,l,Eof;

   Phase     =    2;
   pc        =   -1;
   EnumValue =   -1;
   ForcedEnd =    0;
   ListOn    =    1;
   CPU       = 6309;
   Scope[0]  =    0;
   ModuleStart =  0;

   for (i=0 ; i < 11 ; ++i) minlab[i] = UNDEF;

   if (IfLevel)
   {
      printf("\n*** Error in conditional assembly ***\n");
      if (IfLevel == 1)
         printf("*** an #endif statement is missing\n");
      else
         printf("*** %d #endif statements are missing\n",IfLevel);
      exit(1);
   }
   rewind(sf);
   LiNo = 0; TotalLiNo = 0;
   fgets(Line,sizeof(Line),sf);
   Eof = feof(sf);
   while (!Eof || IncludeLevel > 0)
   {
      ++LiNo; ++TotalLiNo;
      l = strlen(Line);
      if (l && Line[l-1] == 10) Line[--l] = 0; // Remove linefeed
      if (l && Line[l-1] == 13) Line[--l] = 0; // Remove return
      ParseLine(Line);
      if (MacLev) NextMacLine(Line);
      else fgets(Line,sizeof(Line),sf);
      Eof = feof(sf) || ForcedEnd;
      if (Eof && IncludeLevel > 0) Eof = CloseInclude();
      if (GenEnd < pc) GenEnd = pc; // Remember highest assenble address
      if (ErrNum >= ERRMAX)
      {
         printf("\n*** Error count reached maximum of %d ***\n",ErrNum);
         printf("Assembly stopped\n");
         return;
      }
   }
}


void ListSymbols(FILE *lf, int n, int lb, int ub)
{
   int i,j,l;
   char A;

   if (!ListOn) return;
   for (i=0 ; i < n && i < Labels; ++i)
   if (lab[i].Address >= lb && lab[i].Address <= ub)
   {
      fprintf(lf,"%-30.30s $%4.4x",lab[i].Name,lab[i].Address);
      for (j=0 ; j <= lab[i].NumRef ; ++j)
      {
         if (j > 0 && (j % 5) == 0)
         fprintf(lf,"\n                                    ");
         fprintf(lf,"%6d",lab[i].Ref[j]);
         l = lab[i].Att[j];
         if (l == LDEF || l == LBSS || l == LPOS) A = 'D';
         else  A = ' ';
         if ((A != ' ' || (j % 5) != 4) && j != lab[i].NumRef)
            fprintf(lf,"%c",A);
      }
      fprintf(lf,"\n");
   }
}


void ListUndefinedSymbols(void)
{
   int i;

   for (i=0 ; i < Labels ; ++i)
   {
      if (lab[i].Address == UNDEF)
      {
         printf("* Undefined   : %-25.25s *\n",lab[i].Name);
         ++ErrNum;
      }
   }
}

int CmpAddress( const void *arg1, const void *arg2 )
{
   struct LabelStruct *Label1;
   struct LabelStruct *Label2;

   Label1 = (struct LabelStruct *) arg1;
   Label2 = (struct LabelStruct *) arg2;

   if (Label1->Address == Label2->Address) return 0;
   if (Label1->Address  > Label2->Address) return 1;
   return -1;
}


int CmpRefs( const void *arg1, const void *arg2 )
{
   struct LabelStruct *Label1;
   struct LabelStruct *Label2;

   Label1 = (struct LabelStruct *) arg1;
   Label2 = (struct LabelStruct *) arg2;

   if (Label1->NumRef == Label2->NumRef)
   {
      if (Label1->Address < Label2->Address) return  1;
      if (Label1->Address > Label2->Address) return -1;
      return 0;
   }
   if (Label1->NumRef  < Label2->NumRef) return 1;
   return -1;
}

void WriteBinaryFormat(int i)
{
    unsigned char lo,hi;
    FILE *bf;
    const char *msg = "Write binary";

    if (df) fprintf(df,"Storing $%4.4x - $%4.4x <%s>\n",
                    SFA[i],SFA[i]+SFL[i],SFF[i]);
    bf = AssertFileOp(fopen(SFF[i],"wb"), msg);
    if (SFE[i] > -1)
    {
       lo = SFA[i] & 0xff;
       hi = SFA[i]  >>  8;
       if (fwrite(&hi,1,1,bf) < 1) AssertFileOp(NULL, msg);
       if (fwrite(&lo,1,1,bf) < 1) AssertFileOp(NULL, msg);
    }
    if (fwrite(ROM+SFA[i],1,SFL[i],bf) < (size_t)SFL[i]) AssertFileOp(NULL, msg);
    if (fclose(bf)) AssertFileOp(NULL, msg);
}

void WriteS19Line(FILE *bf, const char *RecordType, int PayloadSize, int Address, unsigned char *Data)
{
   int i,Checksum;
   const char *msg = "Write S19 record";

   fprintf(bf, "%s", RecordType);
   // 3 extra bytes for address and checksum
   Checksum = PayloadSize + 3 + (Address & 0xff) + (Address >> 8);
   fprintf(bf, "%02X%04X", PayloadSize + 3, Address);
   if (ferror(bf)) AssertFileOp(NULL, msg);
   for (i=0; i < PayloadSize; ++i)
   {
      fprintf(bf, "%02X", Data[i]);
      if (ferror(bf)) AssertFileOp(NULL, msg);
      Checksum += Data[i];
   }
   // Force CR LF line endings for ancient EPROM programmers
   fprintf(bf, "%02X\r\n", ~Checksum & 0xff);
   if (ferror(bf)) AssertFileOp(NULL, msg);
}

void WriteS19Format(int i)
{
    unsigned char buf[80];
    FILE *bf;
    char *filename, *ExtPtr;
    int UnwrittenBytes,Addr,BytesInThisLine;
    const char *msg = "Write S19 file";

    filename = (char *)StrNDup(SFF[i],strlen(SFF[i]) + 4);
    ExtPtr = strrchr(filename, '.');
    if (!ExtPtr)
    {
       ExtPtr = filename + strlen(filename);
       memmove(ExtPtr, ".S19",5);
    }
    if (df)
    {
       fprintf(df,"Storing $%4.4x - $%4.4x <%s>\n",
                    SFA[i],SFA[i]+SFL[i],filename);
       if (ferror(df)) AssertFileOp(NULL, msg);
    }
    bf = AssertFileOp(fopen(filename, "wb"), msg);
    free(filename);

    // Write a S0 header which the TTL pseudo op should define
    memmove((char *)buf,"Bit Shift Assembler",20);
    WriteS19Line(bf, "S0", strlen((char *)buf), 0, buf);

    SFR[i] = 0;
    UnwrittenBytes = SFL[i];
    Addr = SFA[i];
    BytesInThisLine = 32;
    if (UnwrittenBytes < 32) BytesInThisLine = UnwrittenBytes;
    while (UnwrittenBytes > 0)
    {
       WriteS19Line(bf, "S1", BytesInThisLine, Addr, &ROM[Addr]);
       ++SFR[i];
       Addr += BytesInThisLine;
       UnwrittenBytes -= BytesInThisLine;
    }

    WriteS19Line(bf, "S5", 0, SFR[i], NULL);

    if (SFE[i] > -1) WriteS19Line(bf, "S9", 0, SFE[i], NULL);
    if (fclose(bf)) AssertFileOp(NULL, msg);
}

void WriteBinaries(void)
{
   int i;

   for (i=0 ; i < StoreCount ; ++i)
   {
      if (SFT[i] == SRECORD) WriteS19Format(i);
      else                   WriteBinaryFormat(i);
   }
}

const char *StatOn  = " * ";
const char *StatOff = "   ";

const char *Stat(int o)
{
   if (o) return StatOn ;
   else   return StatOff;
}

void usage(void)
{
   printf("Usage: bs9 [options] <source>\n");
   printf("Options:\n");
   printf("   -d print details in file <Debug.lst>\n");
   printf("   -D Define symbols\n");
   printf("   -i ignore case in symbols\n");
   printf("   -h display this usage\n");
   printf("   -l preset value for memory\n");
   printf("   -m Motorola codestyle: blank = field separator\n");
   printf("   -n include line numbers in listing\n");
   printf("   -o optimize long branches and jumps\n");
   printf("   -p print preprocessed source\n");
   printf("   -p quiet mode\n");
   printf("   -x assemble listing file - skip hex in front\n");
   exit(1);
}

int main(int argc, char *argv[])
{
   int ic,l,v;
   char *EndPtr;
   char *argsrc = NULL; // argument filename;
   time_t rawtime;
   struct tm * timeinfo;

   time (&rawtime);
   timeinfo = localtime (&rawtime);
   strftime (datebuffer,80,"%e-%b-%Y",timeinfo);

   for (ic=1 ; ic < argc ; ++ic)
   {
           if (!strcmp(argv[ic],"-x")) SkipHex    = 1;
      else if (!strcmp(argv[ic],"-d")) Debug      = 1;
      else if (!strcmp(argv[ic],"-i")) IgnoreCase = 1;
      else if (!strcmp(argv[ic],"-m")) CodeStyle  = 1;
      else if (!strcmp(argv[ic],"-n")) WithLiNo   = 1;
      else if (!strcmp(argv[ic],"-o")) Optimize   = 1;
      else if (!strcmp(argv[ic],"-p")) Preprocess = 1;
      else if (!strcmp(argv[ic],"-q")) Quiet      = 1;
      else if (!strncmp(argv[ic],"-D",2)) DefineLabel(argv[ic]+2,&v,1);
      else if (!strncmp(argv[ic],"-l",2))
      {
         if (++ic == argc)
         {
            fprintf(stderr, "Missing value for -l\n");
            exit(1);
         }
         errno = 0;
         Preset = strtol(argv[ic], &EndPtr,0);
         if (errno != 0 || *EndPtr != '\0' || Preset < 0 || Preset > 0xff)
         {
            fprintf(stderr, "Illegal value '%s' for -l\n",argv[ic]);
            exit(1);
         }
         memset(ROM,Preset,sizeof(ROM));
      }
      else if (argsrc == NULL && (argv[ic][0] >= '0' || argv[ic][0] == '.'))
      {
         argsrc = argv[ic];
      }
      else
      {
         usage();
      }
   }
   if (!argsrc)
   {
      printf("*** missing filename for assembler source file ***\n");
      usage();
   }

   // default file names if only source file specified:
   // prog.as9   prog.pp   prog.lst   prog.opt

   l = strlen(argsrc);
   if (l > FNSIZE - 4)
   {
      fprintf(stderr,"\n*** filename too long ***\n");
      exit(1);
   }

   // check extension of form ".xxx"

   if (l > 4 && argsrc[l-4] == '.')
   {
      Src = (char *)StrNDup(argsrc,l);
      l -= 4; // length of basename
   }
   else
   {
      Src = StrNDup(argsrc,l+4);
      memmove(Src+l,".as9",4); // add default extension
   }

   // set static filenames

   memmove(Pre,Src,l);
   memmove(Lst,Src,l);
   memmove(Opt,Src,l);

   // add extensions

   memmove(Pre+l,".pp" ,3);
   memmove(Lst+l,".lst",4);
   memmove(Opt+l,".opt",4);

   if (!Quiet)
   {
      printf("\n");
      printf("*******************************************\n");
      printf("* Bit Shift Assembler 26-Nov-2023         *\n");
      printf("* Today is            %s         *\n",datebuffer);
      printf("* --------------------------------------- *\n");
      printf("* Source: %-31.31s *\n",Src);
      printf("* List  : %-31.31s *\n",Lst);
   }

   sf = fopen(Src,"r");
   if (!sf)
   {
      printf("Could not open <%s>\n",Src);
      exit(1);
   }
   IncludeStack[0].fp = sf;
   IncludeStack[0].Src = Src;
   lf = AssertFileOp(fopen(Lst,"w"), "Open list file");
   if (Debug) df = AssertFileOp(fopen("Debug.lst","w"), "Open Debug file");
   if (Preprocess) pf = AssertFileOp(fopen(Pre,"w"), "Open preprocessor file");
   if (Optimize) of = AssertFileOp(fopen(Opt,"w"), "Open hint file");

   Phase1();
   Phase2();
   WriteBinaries();
   ListUndefinedSymbols();
   qsort(lab,Labels,sizeof(struct LabelStruct),CmpAddress);
   fprintf(lf,"\n\n%5d Symbols\n",Labels);
   fprintf(lf,"-------------\n");
   ListSymbols(lf,Labels,0,0xffff);
   qsort(lab,Labels,sizeof(struct LabelStruct),CmpRefs);
   ListSymbols(lf,Labels,0,0xff);
   ListSymbols(lf,Labels,0,0x4000);
   if (fclose(sf)) AssertFileOp(NULL, "Close source file");
   if (fclose(lf)) AssertFileOp(NULL, "Close list file");

   if (df) if (fclose(df)) AssertFileOp(NULL, "Close debug file");
   if (Optimize)
   {
      if (fclose(of)) AssertFileOp(NULL, "Close hint file");
      if (optc == 0) remove(Opt);
      if (optc) printf("* Opt   : %-31.31s *\n",Opt);
   }
   if (!Quiet)
   {
      printf("* -d:%s  -i:%s  -n:%s  -o:%s  -x:%s  *\n",
            Stat(Debug),Stat(IgnoreCase),Stat(WithLiNo),
            Stat(Optimize),Stat(SkipHex));
      printf("*******************************************\n");
      printf("* Source Lines: %6d                    *\n",TotalLiNo);
      printf("* Symbols     : %6d                    *\n",Labels);
      printf("* Macros      : %6d                    *\n",Macros);
      if (Preset)
      printf("* Preset      : %6d                    *\n",Preset);
      if (optc)
      printf("* Hints       : %6d for optimization   *\n",optc);
      printf("*******************************************\n");
   }
   if (ErrNum)
      printf("* %3d ERROR%s occured%s                      *\n",
             ErrNum, ErrNum == 1 ? "" : "S", ErrNum == 1 ? " " : "");
   else if (!Quiet) printf("* OK, no errors                           *\n");
   if (!Quiet) printf("*******************************************\n\n");
   // MneStat();
   return ErrNum;
}
