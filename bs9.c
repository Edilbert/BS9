/*

*******************
Bit Shift Assembler
*******************

Version: 30-Sep-2019

The assembler was developed and tested on a MAC with macOS Mojave.
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

Running
=======
If you have a source code named "hello.as9", run the assembler with:

bs9 hello

It will read "hello.as9" as input file and write the
listing file with cross reference "hello.lst".
Binary output is controlled within the source file by means
of the pseudo op "!STORE" (see below for syntax):

Case sensitivity
================
mnemonics, and pseudo opcodes are insensitive to case:

LDA lda Lda are all equivalent (Load Accumulator A)

FCB fcb Fcb are all equivalent (define byte data)

Label and named constants are case sensitive by default!
The option "-i" switches off the case sensitivity for symbols.
Also the pseudo op "!CASE +/-" may be used to switch sensitivity.

LDA #Cr  and LDA #CR  use different constants!
JMP Lab_10 and JMP LAB_10  jump to different targets!

Directives
==========
CPU_6809 = 1                   allow code for 6809 only
CPU_6309 = 1                   allow full 6309 instruction set

Examples of pseudo opcodes (directives):
========================================
!ORG  $E000                    set program counter
!LOAD $0401                    precede binary with a CBM load address
!STORE BASIC_ROM,$2000,"basic.rom" write binary image file "basic.rom"
!BITS . . * . * . . .          stores a byte from 8 bit symbols
!BYTE $20,"Example",0          stores a series of byte data
!WORD LAB_10, WriteTape,$0200  stores a series of word data
!QUAD 100000                   stores a 32 bit integer
!REAL  3.1415926               stores a 32 bit real
!FILL  N ($EA)                 fill memory with N bytes containing $EA
!FILL  $A000 - * (0)           fill memory from pc(*) upto $9FFF
!INCLUDE "filename"            includes specified file
!END                           stops assembly
!CASE -                        symbols are not case sensitive
!SIZE                          print code size info
TXTTAB .BSS 2                  define TXTTAB and increase address pointer by 2
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

#if MO5
   STA $D000
#else
   STA $9000
#endif

Example: Assemble first part if MO5 is defined ($0000 - $ffff)
(undefined symbols are set to UNDEF ($ffff0000)

#ifdef MO5
   STA $D000
#else
   STA $9000
#endif

assembles the first statement if MO5 is not zero and the second if zero.

Another example:

#if MO5 | TO9          ; true if either MO5 or TO9 is true (not zero)
   LDA #MASK
#if MO5
   STA ICR_REG
#else
   STA TO9_ICR_REG
#endif                   ; finishes inner if
#endif                   ; finishes outer if

Example: check and force error

#if (MAXLEN & $ff00)
   #error This code is 8 bit only, MAXLEN too large!
#endif

The maximum nesting depth is 10

For more examples see the complete operating system for the
Thomson MO5 available from the user "Bit Shifter" e.g. at
forum64 or the forum of the VzEkC.

*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

char *Strcasestr(const char *s1, const char *s2)
{
   char h1[256];
   char h2[256];
   char *r;

    int i;

    for (i=0 ; i < strlen(s1)+1 ; ++i) h1[i] = tolower(s1[i]);
    for (i=0 ; i < strlen(s2)+1 ; ++i) h2[i] = tolower(s2[i]);

    r = strstr(h1,h2);
    if (r)
    {
       i = r - h1;
       r += i;
    }
    return r;
}

void *AssertAlloc(void *p)
{
   if (p != NULL) return p;
   fprintf(stderr, "Allocation of memory failed.\n");
   exit(1);
}

void *MallocOrDie(size_t size)
{
   return AssertAlloc(malloc(size));
}

void *ReallocOrDie(void *p, size_t size)
{
   return AssertAlloc(realloc(p, size));
}

#define  CPU_6809 0
#define  CPU_6309 1

int CPU_Type = CPU_6809; // default

const char *CPU_Name[] =
{
   "6809"   , // Thomson
   "6309"     // Hitachi
};

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
   char Mne[6];
   int  Opc[ADMODES];
} Mat[] =
{
//             0      1      2      3      4      5      6      7
//   Mnem    CPU  Inher    Reg   Rela    Imm Direct    Ind    Ext
// ---------------------------------------------------------------
   {"NEG"     ,0,    -1,    -1,    -1,    -1,  0x00,  0x60,  0x70},
   {"COM"     ,0,    -1,    -1,    -1,    -1,  0x03,  0x63,  0x73},
   {"LSR"     ,0,    -1,    -1,    -1,    -1,  0x04,  0x64,  0x74},
   {"ROR"     ,0,    -1,    -1,    -1,    -1,  0x06,  0x66,  0x76},
   {"ASR"     ,0,    -1,    -1,    -1,    -1,  0x07,  0x67,  0x77},
   {"ASL"     ,0,    -1,    -1,    -1,    -1,  0x08,  0x68,  0x78},
   {"LSL"     ,0,    -1,    -1,    -1,    -1,  0x08,  0x68,  0x78},
   {"ROL"     ,0,    -1,    -1,    -1,    -1,  0x09,  0x69,  0x79},
   {"DEC"     ,0,    -1,    -1,    -1,    -1,  0x0a,  0x6a,  0x7a},
   {"INC"     ,0,    -1,    -1,    -1,    -1,  0x0c,  0x6c,  0x7c},
   {"TST"     ,0,    -1,    -1,    -1,    -1,  0x0d,  0x6d,  0x7d},
   {"JMP"     ,0,    -1,    -1,    -1,    -1,  0x0e,  0x6e,  0x7e},
   {"CLR"     ,0,    -1,    -1,    -1,    -1,  0x0f,  0x6f,  0x7f},
   {"NOP"     ,0,  0x12,    -1,    -1,    -1,    -1,    -1,    -1},
   {"SYNC"    ,0,  0x13,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LBRA"    ,0,    -1,    -1,  0x16,    -1,    -1,    -1,    -1},
   {"LBSR"    ,0,    -1,    -1,  0x17,    -1,    -1,    -1,    -1},
   {"DAA"     ,0,  0x19,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ORCC"    ,0,    -1,    -1,    -1,  0x1a,    -1,    -1,    -1},
   {"ANDCC"   ,0,    -1,    -1,    -1,  0x1c,    -1,    -1,    -1},
   {"SEX"     ,0,  0x1d,    -1,    -1,    -1,    -1,    -1,    -1},
   {"EXG"     ,0,    -1,  0x1e,    -1,    -1,    -1,    -1,    -1},
   {"TFR"     ,0,    -1,  0x1f,    -1,    -1,    -1,    -1,    -1},
   {"BRA"     ,0,    -1,    -1,  0x20,    -1,    -1,    -1,    -1},
   {"BRN"     ,0,    -1,    -1,  0x21,    -1,    -1,    -1,    -1},
   {"BHI"     ,0,    -1,    -1,  0x22,    -1,    -1,    -1,    -1},
   {"BLS"     ,0,    -1,    -1,  0x23,    -1,    -1,    -1,    -1},
   {"BCC"     ,0,    -1,    -1,  0x24,    -1,    -1,    -1,    -1},
   {"BHS"     ,0,    -1,    -1,  0x24,    -1,    -1,    -1,    -1},
   {"BCS"     ,0,    -1,    -1,  0x25,    -1,    -1,    -1,    -1},
   {"BLO"     ,0,    -1,    -1,  0x25,    -1,    -1,    -1,    -1},
   {"BNE"     ,0,    -1,    -1,  0x26,    -1,    -1,    -1,    -1},
   {"BEQ"     ,0,    -1,    -1,  0x27,    -1,    -1,    -1,    -1},
   {"BVC"     ,0,    -1,    -1,  0x28,    -1,    -1,    -1,    -1},
   {"BVS"     ,0,    -1,    -1,  0x29,    -1,    -1,    -1,    -1},
   {"BPL"     ,0,    -1,    -1,  0x2a,    -1,    -1,    -1,    -1},
   {"BMI"     ,0,    -1,    -1,  0x2b,    -1,    -1,    -1,    -1},
   {"BGE"     ,0,    -1,    -1,  0x2c,    -1,    -1,    -1,    -1},
   {"BLT"     ,0,    -1,    -1,  0x2d,    -1,    -1,    -1,    -1},
   {"BGT"     ,0,    -1,    -1,  0x2e,    -1,    -1,    -1,    -1},
   {"BLE"     ,0,    -1,    -1,  0x2f,    -1,    -1,    -1,    -1},
   {"LEAX"    ,0,    -1,    -1,    -1,    -1,    -1,  0x30,    -1},
   {"LEAY"    ,0,    -1,    -1,    -1,    -1,    -1,  0x31,    -1},
   {"LEAS"    ,0,    -1,    -1,    -1,    -1,    -1,  0x32,    -1},
   {"LEAU"    ,0,    -1,    -1,    -1,    -1,    -1,  0x33,    -1},
   {"PSHS"    ,0,    -1,  0x34,    -1,    -1,    -1,    -1,    -1},
   {"PULS"    ,0,    -1,  0x35,    -1,    -1,    -1,    -1,    -1},
   {"PSHU"    ,0,    -1,  0x36,    -1,    -1,    -1,    -1,    -1},
   {"PULU"    ,0,    -1,  0x37,    -1,    -1,    -1,    -1,    -1},
   {"RTS"     ,0,  0x39,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ABX"     ,0,  0x3a,    -1,    -1,    -1,    -1,    -1,    -1},
   {"RTI"     ,0,  0x3b,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CWAI"    ,0,    -1,    -1,    -1,  0x3c,    -1,    -1,    -1},
   {"MUL"     ,0,  0x3d,    -1,    -1,    -1,    -1,    -1,    -1},
   {"RESET"   ,0,  0x3e,    -1,    -1,    -1,    -1,    -1,    -1},
   {"SWI"     ,0,  0x3f,    -1,    -1,    -1,    -1,    -1,    -1},
   {"NEGA"    ,0,  0x40,    -1,    -1,    -1,    -1,    -1,    -1},
   {"COMA"    ,0,  0x43,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LSRA"    ,0,  0x44,    -1,    -1,    -1,    -1,    -1,    -1},
   {"RORA"    ,0,  0x46,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ASRA"    ,0,  0x47,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ASLA"    ,0,  0x48,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LSLA"    ,0,  0x48,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ROLA"    ,0,  0x49,    -1,    -1,    -1,    -1,    -1,    -1},
   {"DECA"    ,0,  0x4a,    -1,    -1,    -1,    -1,    -1,    -1},
   {"INCA"    ,0,  0x4c,    -1,    -1,    -1,    -1,    -1,    -1},
   {"TSTA"    ,0,  0x4d,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CLRA"    ,0,  0x4f,    -1,    -1,    -1,    -1,    -1,    -1},
   {"NEGB"    ,0,  0x50,    -1,    -1,    -1,    -1,    -1,    -1},
   {"COMB"    ,0,  0x53,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LSRB"    ,0,  0x54,    -1,    -1,    -1,    -1,    -1,    -1},
   {"RORB"    ,0,  0x56,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ASRB"    ,0,  0x57,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ASLB"    ,0,  0x58,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LSLB"    ,0,  0x58,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ROLB"    ,0,  0x59,    -1,    -1,    -1,    -1,    -1,    -1},
   {"DECB"    ,0,  0x5a,    -1,    -1,    -1,    -1,    -1,    -1},
   {"INCB"    ,0,  0x5c,    -1,    -1,    -1,    -1,    -1,    -1},
   {"TSTB"    ,0,  0x5d,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CLRB"    ,0,  0x5f,    -1,    -1,    -1,    -1,    -1,    -1},
   {"SUBA"    ,0,    -1,    -1,    -1,  0x80,  0x90,  0xa0,  0xb0},
   {"CMPA"    ,0,    -1,    -1,    -1,  0x81,  0x91,  0xa1,  0xb1},
   {"SBCA"    ,0,    -1,    -1,    -1,  0x82,  0x92,  0xa2,  0xb2},
   {"SUBD"    ,0,    -1,    -1,    -1,  0x83,  0x93,  0xa3,  0xb3},
   {"ANDA"    ,0,    -1,    -1,    -1,  0x84,  0x94,  0xa4,  0xb4},
   {"BITA"    ,0,    -1,    -1,    -1,  0x85,  0x95,  0xa5,  0xb5},
   {"LDA"     ,0,    -1,    -1,    -1,  0x86,  0x96,  0xa6,  0xb6},
   {"EORA"    ,0,    -1,    -1,    -1,  0x88,  0x98,  0xa8,  0xb8},
   {"ADCA"    ,0,    -1,    -1,    -1,  0x89,  0x99,  0xa9,  0xb9},
   {"ORA"     ,0,    -1,    -1,    -1,  0x8a,  0x9a,  0xaa,  0xba},
   {"ADDA"    ,0,    -1,    -1,    -1,  0x8b,  0x9b,  0xab,  0xbb},
   {"CMPX"    ,0,    -1,    -1,    -1,  0x8c,  0x9c,  0xac,  0xbc},
   {"BSR"     ,0,    -1,    -1,  0x8d,    -1,    -1,    -1,    -1},
   {"LDX"     ,0,    -1,    -1,    -1,  0x8e,  0x9e,  0xae,  0xbe},
   {"STA"     ,0,    -1,    -1,    -1,    -1,  0x97,  0xa7,  0xb7},
   {"JSR"     ,0,    -1,    -1,    -1,    -1,  0x9d,  0xad,  0xbd},
   {"STX"     ,0,    -1,    -1,    -1,    -1,  0x9f,  0xaf,  0xbf},
   {"SUBB"    ,0,    -1,    -1,    -1,  0xc0,  0xd0,  0xe0,  0xf0},
   {"CMPB"    ,0,    -1,    -1,    -1,  0xc1,  0xd1,  0xe1,  0xf1},
   {"SBCB"    ,0,    -1,    -1,    -1,  0xc2,  0xd2,  0xe2,  0xf2},
   {"ADDD"    ,0,    -1,    -1,    -1,  0xc3,  0xd3,  0xe3,  0xf3},
   {"ANDB"    ,0,    -1,    -1,    -1,  0xc4,  0xd4,  0xe4,  0xf4},
   {"BITB"    ,0,    -1,    -1,    -1,  0xc5,  0xd5,  0xe5,  0xf5},
   {"LDB"     ,0,    -1,    -1,    -1,  0xc6,  0xd6,  0xe6,  0xf6},
   {"EORB"    ,0,    -1,    -1,    -1,  0xc8,  0xd8,  0xe8,  0xf8},
   {"ADCB"    ,0,    -1,    -1,    -1,  0xc9,  0xd9,  0xe9,  0xf9},
   {"ORB"     ,0,    -1,    -1,    -1,  0xca,  0xda,  0xea,  0xfa},
   {"ADDB"    ,0,    -1,    -1,    -1,  0xcb,  0xdb,  0xeb,  0xfb},
   {"LDD"     ,0,    -1,    -1,    -1,  0xcc,  0xdc,  0xec,  0xfc},
   {"LDU"     ,0,    -1,    -1,    -1,  0xce,  0xde,  0xee,  0xfe},
   {"STB"     ,0,    -1,    -1,    -1,    -1,  0xd7,  0xe7,  0xf7},
   {"STD"     ,0,    -1,    -1,    -1,    -1,  0xdd,  0xed,  0xfd},
   {"STU"     ,0,    -1,    -1,    -1,    -1,  0xdf,  0xef,  0xff},
   {"LBRN"    ,0,    -1,    -1,0x1021,    -1,    -1,    -1,    -1},
   {"LBHI"    ,0,    -1,    -1,0x1022,    -1,    -1,    -1,    -1},
   {"LBLS"    ,0,    -1,    -1,0x1023,    -1,    -1,    -1,    -1},
   {"LBCC"    ,0,    -1,    -1,0x1024,    -1,    -1,    -1,    -1},
   {"LBHS"    ,0,    -1,    -1,0x1024,    -1,    -1,    -1,    -1},
   {"LBCS"    ,0,    -1,    -1,0x1025,    -1,    -1,    -1,    -1},
   {"LBLO"    ,0,    -1,    -1,0x1025,    -1,    -1,    -1,    -1},
   {"LBNE"    ,0,    -1,    -1,0x1026,    -1,    -1,    -1,    -1},
   {"LBEQ"    ,0,    -1,    -1,0x1027,    -1,    -1,    -1,    -1},
   {"LBVC"    ,0,    -1,    -1,0x1028,    -1,    -1,    -1,    -1},
   {"LBVS"    ,0,    -1,    -1,0x1029,    -1,    -1,    -1,    -1},
   {"LBPL"    ,0,    -1,    -1,0x102a,    -1,    -1,    -1,    -1},
   {"LBMI"    ,0,    -1,    -1,0x102b,    -1,    -1,    -1,    -1},
   {"LBGE"    ,0,    -1,    -1,0x102c,    -1,    -1,    -1,    -1},
   {"LBLT"    ,0,    -1,    -1,0x102d,    -1,    -1,    -1,    -1},
   {"LBGT"    ,0,    -1,    -1,0x102e,    -1,    -1,    -1,    -1},
   {"LBLE"    ,0,    -1,    -1,0x102f,    -1,    -1,    -1,    -1},
   {"SWI2"    ,0,0x103f,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CMPD"    ,0,    -1,    -1,    -1,0x1083,0x1093,0x10a3,0x10b3},
   {"CMPY"    ,0,    -1,    -1,    -1,0x108c,0x109c,0x10ac,0x10bc},
   {"LDY"     ,0,    -1,    -1,    -1,0x108e,0x109e,0x10ae,0x10be},
   {"STY"     ,0,    -1,    -1,    -1,    -1,0x109f,0x10af,0x10bf},
   {"LDS"     ,0,    -1,    -1,    -1,0x10ce,0x10de,0x10ee,0x10fe},
   {"STS"     ,0,    -1,    -1,    -1,    -1,0x10df,0x10ef,0x10ff},
   {"SWI3"    ,0,0x113f,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CMPU"    ,0,    -1,    -1,    -1,0x1183,0x1193,0x11a3,0x11b3},
   {"CMPS"    ,0,    -1,    -1,    -1,0x118c,0x119c,0x11ac,0x11bc},

//   6309      0      1      2      3      4      5      6      7
//   Mnem    CPU  Inher    Reg   Rela    Imm Direct    Ind    Ext
// ---------------------------------------------------------------
   {"SEXW"    ,1,  0x14,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ADDR"    ,1,    -1,0x1030,    -1,    -1,    -1,    -1,    -1},
   {"ADCR"    ,1,    -1,0x1031,    -1,    -1,    -1,    -1,    -1},
   {"SUBR"    ,1,    -1,0x1032,    -1,    -1,    -1,    -1,    -1},
   {"SBCR"    ,1,    -1,0x1033,    -1,    -1,    -1,    -1,    -1},
   {"ANDR"    ,1,    -1,0x1034,    -1,    -1,    -1,    -1,    -1},
   {"ORR"     ,1,    -1,0x1035,    -1,    -1,    -1,    -1,    -1},
   {"EORR"    ,1,    -1,0x1036,    -1,    -1,    -1,    -1,    -1},
   {"CMPR"    ,1,    -1,0x1037,    -1,    -1,    -1,    -1,    -1},
   {"TFM"     ,1,    -1,0x1138,    -1,    -1,    -1,    -1,    -1},
   {"BITMD"   ,1,    -1,0x113c,    -1,    -1,    -1,    -1,    -1},
   {"LDMD"    ,1,    -1,    -1,    -1,0x113d,    -1,    -1,    -1},
   {"PSHSW"   ,1,0x1038,    -1,    -1,    -1,    -1,    -1,    -1},
   {"PULSW"   ,1,0x1039,    -1,    -1,    -1,    -1,    -1,    -1},
   {"PSHUW"   ,1,0x103A,    -1,    -1,    -1,    -1,    -1,    -1},
   {"PULUW"   ,1,0x103B,    -1,    -1,    -1,    -1,    -1,    -1},
   {"NEGD"    ,1,0x1040,    -1,    -1,    -1,    -1,    -1,    -1},
   {"COMD"    ,1,0x1043,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LSRD"    ,1,0x1044,    -1,    -1,    -1,    -1,    -1,    -1},
   {"RORD"    ,1,0x1046,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ASRD"    ,1,0x1047,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ASLD"    ,1,0x1048,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LSLD"    ,1,0x1048,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ROLD"    ,1,0x1049,    -1,    -1,    -1,    -1,    -1,    -1},
   {"DECD"    ,1,0x104A,    -1,    -1,    -1,    -1,    -1,    -1},
   {"INCD"    ,1,0x104C,    -1,    -1,    -1,    -1,    -1,    -1},
   {"TSTD"    ,1,0x104D,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CLRD"    ,1,0x104F,    -1,    -1,    -1,    -1,    -1,    -1},
   {"COMW"    ,1,0x1053,    -1,    -1,    -1,    -1,    -1,    -1},
   {"LSRW"    ,1,0x1054,    -1,    -1,    -1,    -1,    -1,    -1},
   {"RORW"    ,1,0x1056,    -1,    -1,    -1,    -1,    -1,    -1},
   {"ROLW"    ,1,0x1059,    -1,    -1,    -1,    -1,    -1,    -1},
   {"DECW"    ,1,0x105A,    -1,    -1,    -1,    -1,    -1,    -1},
   {"INCW"    ,1,0x105C,    -1,    -1,    -1,    -1,    -1,    -1},
   {"TSTW"    ,1,0x105D,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CLRW"    ,1,0x105F,    -1,    -1,    -1,    -1,    -1,    -1},
   {"COME"    ,1,0x1143,    -1,    -1,    -1,    -1,    -1,    -1},
   {"DECE"    ,1,0x114A,    -1,    -1,    -1,    -1,    -1,    -1},
   {"INCE"    ,1,0x114C,    -1,    -1,    -1,    -1,    -1,    -1},
   {"TSTE"    ,1,0x114D,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CLRE"    ,1,0x114F,    -1,    -1,    -1,    -1,    -1,    -1},
   {"COMF"    ,1,0x1153,    -1,    -1,    -1,    -1,    -1,    -1},
   {"DECF"    ,1,0x115A,    -1,    -1,    -1,    -1,    -1,    -1},
   {"INCF"    ,1,0x115C,    -1,    -1,    -1,    -1,    -1,    -1},
   {"TSTF"    ,1,0x115D,    -1,    -1,    -1,    -1,    -1,    -1},
   {"CLRF"    ,1,0x115F,    -1,    -1,    -1,    -1,    -1,    -1},
   {"OIM"     ,1,    -1,    -1,    -1,    -1,  0x01,  0x61,  0x71},
   {"AIM"     ,1,    -1,    -1,    -1,    -1,  0x02,  0x62,  0x72},
   {"EIM"     ,1,    -1,    -1,    -1,    -1,  0x05,  0x65,  0x75},
   {"TIM"     ,1,    -1,    -1,    -1,    -1,  0x0b,  0x6b,  0x7b},
   {"STW"     ,1,    -1,    -1,    -1,    -1,0x1097,0x10a7,0x10b7},
   {"STQ"     ,1,    -1,    -1,    -1,    -1,0x10dd,0x10ed,0x10fd},
   {"STE"     ,1,    -1,    -1,    -1,    -1,0x1197,0x11a7,0x11b7},
   {"STF"     ,1,    -1,    -1,    -1,    -1,0x11d7,0x11e7,0x11f7},
   {"LDQ"     ,1,    -1,    -1,    -1,  0xcd,0x10dc,0x10ec,0x10fc},
   {"SUBW"    ,1,    -1,    -1,    -1,0x1080,0x1090,0x10a0,0x10b0},
   {"CMPW"    ,1,    -1,    -1,    -1,0x1081,0x1091,0x10a1,0x10b1},
   {"SBCD"    ,1,    -1,    -1,    -1,0x1082,0x1092,0x10a2,0x10b2},
   {"ANDD"    ,1,    -1,    -1,    -1,0x1084,0x1094,0x10a4,0x10b4},
   {"BITD"    ,1,    -1,    -1,    -1,0x1085,0x1095,0x10a5,0x10b5},
   {"LDW"     ,1,    -1,    -1,    -1,0x1086,0x1096,0x10a6,0x10b6},
   {"EORD"    ,1,    -1,    -1,    -1,0x1088,0x1098,0x10a8,0x10b8},
   {"ADCD"    ,1,    -1,    -1,    -1,0x1089,0x1099,0x10a9,0x10b9},
   {"ORD"     ,1,    -1,    -1,    -1,0x108a,0x109a,0x10aa,0x10ba},
   {"ADDW"    ,1,    -1,    -1,    -1,0x108b,0x109b,0x10ab,0x10bb},
   {"SUBE"    ,1,    -1,    -1,    -1,0x1180,0x1190,0x11a0,0x11b0},
   {"CMPE"    ,1,    -1,    -1,    -1,0x1181,0x1191,0x11a1,0x11b1},
   {"LDE"     ,1,    -1,    -1,    -1,0x1186,0x1196,0x11a6,0x11b6},
   {"ADDE"    ,1,    -1,    -1,    -1,0x118b,0x119b,0x11ab,0x11bb},
   {"DIVD"    ,1,    -1,    -1,    -1,0x118d,0x119d,0x11ad,0x11bd},
   {"DIVQ"    ,1,    -1,    -1,    -1,0x118e,0x119e,0x11ae,0x11be},
   {"MULD"    ,1,    -1,    -1,    -1,0x118f,0x119f,0x11af,0x11bf},
   {"SUBF"    ,1,    -1,    -1,    -1,0x11c0,0x11d0,0x11e0,0x11f0},
   {"CMPF"    ,1,    -1,    -1,    -1,0x11c1,0x11d1,0x11e1,0x11f1},
   {"LDF"     ,1,    -1,    -1,    -1,0x11c6,0x11d6,0x11e6,0x11f6},
   {"ADDF"    ,1,    -1,    -1,    -1,0x11cb,0x11db,0x11eb,0x11fb},
   {"BAND"    ,1,    -1,    -1,    -1,    -1,0x1130,    -1,    -1},
   {"BIAND"   ,1,    -1,    -1,    -1,    -1,0x1131,    -1,    -1},
   {"BOR"     ,1,    -1,    -1,    -1,    -1,0x1132,    -1,    -1},
   {"BIOR"    ,1,    -1,    -1,    -1,    -1,0x1133,    -1,    -1},
   {"BEOR"    ,1,    -1,    -1,    -1,    -1,0x1134,    -1,    -1},
   {"BIEOR"   ,1,    -1,    -1,    -1,    -1,0x1135,    -1,    -1},
   {"LDBT"    ,1,    -1,    -1,    -1,    -1,0x1136,    -1,    -1},
   {"STBT"    ,1,    -1,    -1,    -1,    -1,0x1137,    -1,    -1},
};

#define DIMOP_6809 139
#define DIMOP_6309 (sizeof(Mat) / sizeof(struct MatStruct))

int DimOp = DIMOP_6309;


const char *RegisterNames[] =
{
//  0   1   2   3   4   5    6   7   8   9    A    B   C   D   E   F
   "D","X","Y","U","S","PC","W","V","A","B","CC","DP","*","*","E","F"
};


struct PushStruct
{
   char Reg[3];
   int  Val;
} PushList[10] =
{
   {"CC", 0x01}, // bit 0
   {"A" , 0x02}, // bit 1
   {"B" , 0x04}, // bit 2
   {"D" , 0x06}, // bit 1 and 2
   {"DP", 0x08}, // bit 3
   {"X" , 0x10}, // bit 4
   {"Y" , 0x20}, // bit 5
   {"S" , 0x40}, // bit 6
   {"U" , 0x40}, // bit 6
   {"PC", 0x80}  // bit 7
};


#define UNDEF 0xffff0000

int SkipHex = 0;    // Switch on with -x
int Debug = 0;      // Switch on with -d
int LiNo  = 0;      // Line number of current file
int WithLiNo = 0;   // Print line numbers in listing if set
int TotalLiNo  = 0; // Total line number
int Preprocess = 0; // Print preprocessed source file <file.pp>
int ERRMAX = 10;    // Stop assemby after ERRMAX errors
int ErrNum;
int LoadAddress = UNDEF;
int WriteLoadAddress = 0;
int MacroStopped;
int InsideMacro;
int CurrentMacro;
int ModuleStart;       // address of a module
int ModuleTrigger;     // start of module
int FormLn;            // lines per page [inactive]
int DP;                // current direct page
int CodeStyle;         // 1: Operand has no spaces

char TTL[256];         // current title
char *MacroPointer;

int MneIndex;          // Current mnemonic

int oc;      // op code
int pb;      // post byte
int am;      // address mode
int il;      // instruction length
int ol;      // opcode length
int pl;      // postbyte length
int ql;      // operand length
int pc = -1; // program counter
int ex;      // force extended mode
int bss;     // bss counter
int Phase;
int IfLevel;
int Skipping;
int SkipLine[10];
int ForcedEnd;    // Triggered by .END command
int IgnoreCase;   // 1: Ignore case for symbols

// Filenames

char *Src;
char  Lst[256];
char  Pre[256];
char  Opt[256];

int GenStart = 0x10000 ; // Lowest assemble address
int GenEnd   =       0 ; //Highest assemble address

// These arrays hold the parameter for storage files

#define SFMAX 20
int SFA[SFMAX];
int SFL[SFMAX];
char SFF[SFMAX][80];

int StoreCount = 0;


// The size is one page more than 64K because program, counter
// overflows are detected after using the new value.
// So references to pc + n do no harm if pc is near the boundary

unsigned char ROM[0x10100]; // binary


FILE *sf;
FILE *lf;
FILE *df;
FILE *pf;
FILE *of;

struct IncludeStackStruct
{
   FILE *fp;
   int   LiNo;
   char *Src;
} IncludeStack[100];

int IncludeLevel;

#define ML 256

int ArgPtr[10];
char Line[ML];               // source line
char Label[ML];
char MacArgs[ML];
unsigned char Operand[ML];   // binary operand
char OpText[ML];             // operand source
char Comment[ML];            // comment source
char ModuleName[ML];

#define LDEF 1
#define LBSS 2
#define LPOS 3

#define MAXLAB 8000

struct LabelStruct
{
   char *Name;     // Label name - case sensitive
   int   Address;  // Range 0 - 65536
   int   Bytes;    // Length of object (string for example)
   int   Locked;   // Defined from command line argument
   int   NumRef;   // # of references
   int  *Ref;      // list of references
   int  *Att;      // list of attributes
} lab[MAXLAB];

int Labels;

#define MAXMAC 64

struct MacroStruct
{
   char *Name;  // MACRO Name(arg,arg,...) (up to 10 arguments)
   char *Body;  // "Line1\nLine2\n ... LastLine\n"
   int  Narg;   // # of macro arguments (0-10)
   int  Cola;   // column of macro definition (for pretty printing)
   int  Type;   // 0: name(arg1,arg2))  1: name arg1,arg2
} Mac[MAXMAC];

int Macros;


char *SkipSpace(char *p)
{
   if (*p) while (isspace(*p)) ++p;
   return p;
}

char *StrMatch(char *s, char *m)
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

int isym(char c)
{
   return (c == '.' || c == '$' || c == '_' || isalnum(c));
}


char *GetSymbol(char *p, char *s)
{
   if (isalpha(*p)) while (isym(*p)) *s++ = *p++;
   *s = 0;
   return p;
}


char *GetMacroArg(char *p, char *s)
{
   p = SkipSpace(p);
   while (*p && *p != ' ' && *p != ',') *s++ = *p++;
   *s = 0;
   return p;
}


char *NextSymbol(char *p, char *s)
{
   p = SkipSpace(p);
   p = GetSymbol(p,s);
   return p;
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
      if (SkipHex) memmove(Line,Line+20,l-20);
      else return (p+20);
   }
   return p;
}


void ErrorLine(char *p)
{
   int i,ep;
   printf("%s\n",Line);
   ep = p - Line;
   if (ep < 80)
   {
      for (i=0 ; i < ep ; ++i) printf(" ");
      printf("^\n");
      return;
   }
   ep = p - OpText;
   if (ep < 80 && *OpText)
   {
      printf("Operand: %s\n",OpText);
      for (i=0 ; i < ep+9 ; ++i) printf(" ");
      printf("^\n");
      return;
   }

}

void ListSymbols(FILE *lf, int n, int lb, int ub);


#define SIZE_ERRMSG 1024

void ErrorMsg(const char *format, ...) {
   va_list args;
   char *buf;

   buf = MallocOrDie(SIZE_ERRMSG);
   snprintf(buf, SIZE_ERRMSG, "\n*** Error in file %s line %d:\n",
         IncludeStack[IncludeLevel].Src, LiNo);
   va_start(args,format);
   vsnprintf(buf+strlen(buf), SIZE_ERRMSG-strlen(buf), format, args);
   va_end(args);
   fputs(buf, stdout);
   fputs(buf, lf);
   if (df)
   {
      fputs(buf, df);
      ListSymbols(df,Labels,0,0xffff);
   }
   free(buf);
}

void PrintLiNo(int Blank)
{
   if (Phase < 2) return;
   if (WithLiNo)
   {
      fprintf(lf,"%5d",LiNo);
      if (Blank ==  1) fprintf(lf," ");
   }
   if (Blank == -1) fprintf(lf,"\n");
}

void PrintPC(void)
{
   if (Phase < 2) return;
   if (WithLiNo) PrintLiNo(1);
   fprintf(lf,"%4.4x",pc);
}

void PrintOC(int v)
{
   if (oc == 0xcd) // LDQ immediate
   {
      fprintf(lf," cd %4.4x %4.4x",v>>16,v&0xffff);
      return;
   }
   if (oc > 255) fprintf(lf," %4.4x",oc);
   else          fprintf(lf,"   %2.2x",oc);
   if (pb >= 0)  fprintf(lf," %2.2x",pb);
   else          fprintf(lf,"   ");

        if (ql == 2) fprintf(lf," %4.4x",v&0xffff);
   else if (ql == 1) fprintf(lf,"   %2.2x",v&0xff);
   else              fprintf(lf,"     ");
}

void PrintLine(void)
{
   if (Phase < 2) return;
   PrintLiNo(1);
   fprintf(lf,"                  %s\n",Line);
}

void PrintPCLine(void)
{
   if (Phase < 2) return;
   PrintPC();
   fprintf(lf,"          %s\n",Line);
}

void PrintByteLine(int b)
{
   if (Phase < 2) return;
   if (WithLiNo) PrintLiNo(1);
   fprintf(lf,"  %2.2x",b);
   fprintf(lf,"          %s\n",Line);
}

void PrintWordLine(int w)
{
   if (Phase < 2) return;
   if (WithLiNo) PrintLiNo(1);
   fprintf(lf,"%4.4x",w);
   fprintf(lf,"              %s\n",Line);
}



int IsInstruction(char *p)
{
   int i,l;

   for (i = 0 ; i < DimOp ; ++i)
   {
      l = strlen(Mat[i].Mne);
      if (!strncasecmp(p,Mat[i].Mne,l) && !isym(p[l]))
      {
         return i;
      }
   }
   return -1; // No mnemonic
}

char *NeedChar(char *p, char c)
{
   return strchr(p,c);
}


char *EvalAddress(char *p, int *a)
{
   p=SkipSpace(p);
   if (*p == '$')
   {
      sscanf(++p,"%x",a);
      while (isxdigit(*p)) ++p;
      p = SkipSpace(p);
   }
   else
   {
      sscanf(p,"%x",a);
      while (isxdigit(*p)) ++p;
      if (*p == 'H' || *p == 'h') ++p;
      else
      {
         ErrorMsg("Address %5.5s is neither $xxxx nor xxxxH\n",p);
         exit(1);
      }
   }
   if (*a < 0 || *a > 0xffff)
   {
      ErrorMsg("Address %x out of range\n",*a);
   }
   return p;
}

char *EvalOperand(char *, int *, int);

char *ParseCaseData(char *p)
{
   p = SkipSpace(p);
        if (*p == '+') IgnoreCase = 0;
   else if (*p == '-') IgnoreCase = 1;
   else
   {
      ++ErrNum;
      ErrorMsg("Missing '+' or '-' after .CASE\n");
      exit(1);
   }
   PrintLine();
   return p+1;
}

char *SetPC(char *p)
{
   if (*p == '*')
   {
      p = NeedChar(p,'=');
      if (!p)
      {
         ++ErrNum;
         ErrorMsg("Missing '=' in set pc * instruction\n");
         exit(1);
      }
   }
   else p += 3; // .ORG syntax
   p = EvalOperand(p+1,&pc,0);
   if (LoadAddress < 0) LoadAddress = pc;
   PrintPCLine();
   if (GenStart > pc) GenStart = pc; // remember lowest pc value
   return p;
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
   p = EvalOperand(p+1,&bss,0);
   if (df) fprintf(df,"BSS = %4.4x\n",bss);
   if (Phase == 2)
   {
      PrintLiNo(1);
      fprintf(lf,"%4.4x          %s\n",bss,Line);
   }
   return p;
}

int StrCmp(const char *s1, const char *s2)
{
   if (IgnoreCase) return strcasecmp(s1,s2);
   else            return strcmp(s1,s2);
}

int StrnCmp(const char *s1, const char *s2, size_t n)
{
   if (IgnoreCase) return strncasecmp(s1,s2,n);
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


void ExtractOpText(char *p)
{
   int l,inquo,inapo;

   l       =       0; // length of trimmed operand
   inquo   =       0; // inside quotes
   inapo   =       0; // inside apostrophes

   OpText[0] = 0;     // empty operand
   p = SkipSpace(p);  // text after mnemonic or pseudo op
   if (!*p) return;   // end of line

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

   // Extract Comment
}


char *DefineLabel(char *p, int *val, int Locked)
{
   int j,l,v;

   if (Labels > MAXLAB -2)
   {
      ++ErrNum;
      ErrorMsg("Too many labels (> %d)\n",MAXLAB);
      exit(1);
   }
   p = GetSymbol(p,Label);
   if (*p == ':') ++p; // Ignore colon after label
   l = strlen(Label);
   p = SkipSpace(p);
   if (*p == '=' || strncmp(p,"EQU ",4) == 0)
   {
      if (*p == '=') p++;
      else           p+=4;
      j = LabelIndex(Label);
      if (j < 0)
      {
         j = Labels;
         lab[j].Name = MallocOrDie(l+1);
         strcpy(lab[j].Name,Label);
         lab[j].Address = UNDEF;
         lab[j].Ref = MallocOrDie(sizeof(int));
         lab[j].Att = MallocOrDie(sizeof(int));
         Labels++;
      }
      lab[j].Ref[0] = LiNo;
      lab[j].Att[0] = LDEF;
      ExtractOpText(p);
      p += strlen(p);
      EvalOperand(OpText,&v,0);
      if (lab[j].Address == UNDEF) lab[j].Address = v;
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
      if (Locked) lab[j].Locked = Locked;
   }
   else if (!strncasecmp(p,".BSS",4))
   {
      p = EvalOperand(p+4,&v,0);
      j = LabelIndex(Label);
      if (j < 0)
      {
         j = Labels;
         lab[j].Name = MallocOrDie(l+1);
         strcpy(lab[j].Name,Label);
         lab[j].Address = UNDEF;
         lab[j].Ref = MallocOrDie(sizeof(int));
         lab[j].Att = MallocOrDie(sizeof(int));
         Labels++;
      }
      lab[j].Ref[0] = LiNo;
      lab[j].Att[0] = LBSS;
      if (lab[j].Address == UNDEF) lab[j].Address = bss;
      else if (lab[j].Address != bss)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Multiple assignments for label [%s]\n"
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
         lab[j].Name = MallocOrDie(l+1);
         strcpy(lab[j].Name,Label);
         lab[j].Address = pc;
         lab[j].Ref = MallocOrDie(sizeof(int));
         lab[j].Att = MallocOrDie(sizeof(int));
         Labels++;
      }
      else if (lab[j].Address < 0) lab[j].Address = pc;
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


void AddLabel(char *p)
{
   int l;

   if (Labels > MAXLAB -2)
   {
      ++ErrNum;
      ErrorMsg("Too many labels (> %d)\n",MAXLAB);
      exit(1);
   }
   l = strlen(p);
   lab[Labels].Address = UNDEF;
   lab[Labels].Name = MallocOrDie(l+1);
   strcpy(lab[Labels].Name,p);
   lab[Labels].Ref = MallocOrDie(sizeof(int));
   lab[Labels].Att = MallocOrDie(sizeof(int));
   lab[Labels].Ref[0] = LiNo;
   lab[Labels].Att[0] = 0;
   Labels++;
}


void SymRefs(int i)
{
   int n;

   if (Phase != 2) return;
   n = ++lab[i].NumRef;
   lab[i].Ref = ReallocOrDie(lab[i].Ref,(n+1)*sizeof(int));
   lab[i].Ref[n] = LiNo;
   lab[i].Att = ReallocOrDie(lab[i].Att,(n+1)*sizeof(int));
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


char *ParseRealData(char *p)
{
   int i,v,mansize;
   int Sign,Exponent;
   union udb
   {
     double d;
     unsigned char b[8];
   } db;

   mansize = 3; // default mantissa size

   p = SkipSpace(p);

   for (i=0 ; i < 8 ; ++i) Operand[i] = 0;

   if (*p == '$')
   {
      ++p;
      for (i=0 ; i < mansize+1 ; ++i, p+=2)
      {
          if (!isxdigit(*p)) break;
          sscanf(p,"%2x",&v);
          Operand[i] = v;
      }
   }
   else
   {
      db.d = atof(p);
      Sign = db.b[7] & 0x80;
      Exponent = (((db.b[7] & 0x7f) << 4) | (db.b[6] >> 4)) - 0x3ff + 0x81;

      Operand[0] = Exponent;
      Operand[1] = ((db.b[6] & 0x0f) << 3) | (db.b[5] >> 5) | Sign;
      Operand[2] = ((db.b[5] & 0x1f) << 3) | (db.b[4] >> 5);
      Operand[3] = ((db.b[4] & 0x1f) << 3) | (db.b[3] >> 5);
      Operand[4] = ((db.b[3] & 0x1f) << 3) | (db.b[2] >> 5);
      Operand[5] = ((db.b[2] & 0x1f) << 3) | (db.b[1] >> 5);

      if (db.d == 0.0) for (i=0 ; i < 8 ; ++i) Operand[i] = 0;
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
      for (i=0 ; i < mansize+1 ; ++i) ROM[pc+i] = Operand[i];
      PrintPC();
      fprintf(lf," %2.2x %2.2x%2.2x%2.2x  ",
         Operand[0],Operand[1],Operand[2],Operand[3]);
      fprintf(lf," %s\n",Line);
   }
   pc += mansize+1;
   return p + strlen(p);;
}


char *EvalDecValue(char *p, int *v)
{
   int i;

   // check for xxxxH or xxxxh hex syntax;

   for (i=0 ; i < 5 ; ++i) // allow max. 4 hex digits
      if (!isxdigit(p[i])) break;
   if (p[i] == 'H' || p[i] == 'h')
   {
      sscanf(p,"%x",v);
      return p+i+1;
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
   *v = *p++;
   if (*p == '\'') ++p; // optional closing apostrophe
   return p;
}


char *EvalHexValue(char *p, int *v)
{
   sscanf(p,"%x",v);
   while (isxdigit(*p)) ++p;
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

char *op_low(char *p, int *v) { p = EvalOperand(p+1,v,12);*v &= 0xff    ; return p; }
char *op_hig(char *p, int *v) { p = EvalOperand(p+1,v,12);ex = 1        ; return p; }

char *op_prc(char *p, int *v) { *v = pc; return p+1;}
char *op_hex(char *p, int *v) { return EvalHexValue(p+1,v) ;}
char *op_cha(char *p, int *v) { return EvalCharValue(p+1,v);}
char *op_bin(char *p, int *v) { return EvalBinValue(p+1,v) ;}
char *op_len(char *p, int *v) { return EvalSymBytes(p+1,v) ;}

struct unaop_struct
{
   char op;
   char *(*foo)(char*,int*);
};

#define UNAOPS 13

// table of unary operators in C style

struct unaop_struct unaop[UNAOPS] =
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
   {'%',&op_bin}, // binary constant
   {'?',&op_len}  // length of .BYTE data line
};

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

   if (*p && strchr("[(+-!~<>*$'%?",*p))
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
      return p;
   }
   p = SkipSpace(p);

   // Unary operands are not allowed here

   if (*p && strchr("[!~<>$'%?",*p))
   {
      ErrorMsg("Syntax error: need binary operator or end of expression\n");
      ErrorLine(p);
      exit(1);
   }

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
               return p;
            }
            p = EvalOperand(p+l,&w,o);
            if (w == UNDEF) r = UNDEF;
            else r = binop[i].foo(r,w);
            break;
         }
      }
   }
   *v = r;
   if (CodeStyle == 1 && *p == ' ') p += strlen(p);
   return p;
}


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
      ErrorMsg("Missing FDB data\n");
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
         ROM[pc+i] = ByteBuffer[i];
         if (i == 0 || i == 2) fprintf(lf," %2.2x%2.2x",ByteBuffer[i],ByteBuffer[i+1]);
      }
      if (l == 2) fprintf(lf,"        ");
      else        fprintf(lf,"   ");
      fprintf(lf," %s\n",Line);
   }
   pc += l;

   return p;
}


char *ParseFillData(char *p)
{
   int i,m,v;

   p = EvalOperand(p,&m,0);
   if (m < 0 || m > 32767)
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
      for (i=0 ; i < m ; ++i) ROM[pc+i] = v;
      PrintPC();
      if (m > 0) fprintf(lf," %2.2x",v);
      else       fprintf(lf,"   ");
      if (m > 1) fprintf(lf," %2.2x",v);
      else       fprintf(lf,"   ");
      if (m > 2) fprintf(lf," %2.2x",v);
      else       fprintf(lf,"   ");
      fprintf(lf," %s ; %d bytes\n",Line,m);
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
   if (Phase == 2)
   {
      PrintPC();
      fprintf(lf,"              %s",Line);
      i = AddressIndex(ModuleStart);
      if (i >= 0)
      {
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
      ErrorMsg("Missing quoted filename after .INCLUDE\n");
      exit(1);
   }
   fp = FileName;
   ++p;
   while (*p != 0 && *p != '"') *fp++ = *p++;
   *fp = 0;
   // printf("fopen %s\n",FileName);
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
   IncludeStack[IncludeLevel].Src = MallocOrDie(strlen(FileName + 1));
   strcpy(IncludeStack[IncludeLevel].Src, FileName);
   PrintLine();
   LiNo = 0;
   return p;
}

char *ParseStoreData(char *p)
{
   int Start,Length,i;
   char Filename[80];

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
   ++p;
   i = 0;
   while (*p && *p != '"' && i < 80) Filename[i++] = *p++;
   Filename[i] = 0;
   SFA[StoreCount] = Start;
   SFL[StoreCount] = Length;
   strcpy(SFF[StoreCount],Filename);
   if (df) fprintf(df,"Storing %4.4x - %4.4x <%s>\n",Start,Start+Length-1,Filename);
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


char *ParseBSSData(char *p)
{
   int m;

   p = EvalOperand(p,&m,0);
   if (m < 1 || m > 32767)
   {
      ErrorMsg("Illegal BSS size %d\n",m);
      exit(1);
   }
   if (Phase == 2)
   {
      PrintLiNo(1);
      fprintf(lf,"%4.4x             ",bss);
      fprintf(lf,"%s\n",Line);
   }
   bss += m;
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
   if (Phase == 2)
   {
      PrintPC();
      ROM[pc] = v;
      fprintf(lf," %2.2x       ",v);
      fprintf(lf,"%s\n",Line);
   }
   ++pc;
   return p + strlen(p);
}

char *ParseCharData(char *p)
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
         ErrorMsg("use only '*' for 1 and '.' for 0 in CHAR statement\n");
         exit(1);
      }
   }
   if (Phase == 2)
   {
      PrintPC();
      if (scanline < 0) ROM[pc] = v;
      else ROM[pc+2*scanline-7] = v;
      fprintf(lf," %2.2x       ",v);
      fprintf(lf,"%s\n",Line);
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
      if (Delimiter == '"' || Delimiter == '\'')
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
         ROM[pc+i] = ByteBuffer[i];
         if (i < 4) fprintf(lf," %2.2x",ByteBuffer[i]);
      }
      for (i=l ; i < 4 ; ++i) fprintf(lf,"   ");
      fprintf(lf,"  %s\n",Line);
   }
   pc += l;
   return p;
}


char *ParseStringData(char *p)
{
   int i,j,l,v;
   unsigned char ByteBuffer[ML];

   p = SkipSpace(p);
   l = 0;
   p = ParseASCII(p,ByteBuffer,&l);
   if (df)
   {
      fprintf(df,"FCC String $%4.4x:<",pc);
      for (j=0 ; j < l ; ++j) fprintf(df,"%c",ByteBuffer[j]&0x7f);
      fprintf(df,"> [%d]\n",l);
   }

   if (l < 1)
   {
      ErrorMsg("Missing FCC data\n");
      ErrorLine(p);
      exit(1);
   }

   j = AddressIndex(pc);
   if (j >= 0)
   for ( ; j < Labels ; ++j) // There may be multiple lables on this address
   {
       if (lab[j].Address == pc) lab[j].Bytes = l;
   }
   if (j >= 0 && df) fprintf(df,"FCC label [%s] $%4.4x $%4.4x %d bytes\n",
                   lab[j].Name,lab[j].Address,pc,l);
   if (Phase == 2)
   {
      for (i=0 ; i < l ; ++i)
      {
         ROM[pc+i] = ByteBuffer[i];
         if (i < 4) fprintf(lf," %2.2x",ByteBuffer[i]);
      }
      for (i=l ; i < 4 ; ++i) fprintf(lf,"   ");
      fprintf(lf,"  %s\n",Line);
   }
   pc += l;
   return p;
}


char *CheckPseudo(char *p)
{
   char *q;

   p = SkipSpace(p);
   if (!strncasecmp(p,"ORG ",4))
   {
      ExtractOpText(p+4);
      EvalOperand(OpText,&pc,0);
      PrintPCLine();
      p += strlen(p);
      if (df) fprintf(df,"PC = $%4.4x\n",pc);
   }
   if (!strncasecmp(p,"FORMLN",6))
   {
      FormLn = atoi(p+6);
      PrintByteLine(FormLn);
      p += strlen(p);
   }
   else if (!strncasecmp(p,"SETDP",5))
   {
      ExtractOpText(p+5);
      EvalOperand(OpText,&DP,0);
      PrintByteLine(DP);
      p += strlen(p);
   }
   else if (!strncasecmp(p,"SECT ",5))
   {
      q = StrMatch(p+4,"LOC=");
      if (q) q = EvalOperand(q+4,&pc,0);
      PrintPCLine();
      p += strlen(p);
   }
   else if (!strncasecmp(p,"INTERN",6))
   {
      PrintLine();
      p += strlen(p);
   }
   else if (!strncasecmp(p,"EXTERN",6))
   {
      PrintLine();
      p += strlen(p);
   }
   else if (!strncasecmp(p,"TTL",3))
   {
      PrintLine();
      // CodeStyle = 1;
      p = SkipSpace(p);
      strcpy(TTL,p);
      p += strlen(p);
   }
   else if (!strncasecmp(p,"FDB ",4))
   {
      PrintPC();
      p = ParseWordData(p+4);
      p += strlen(p);
   }
   else if (!strncasecmp(p,"FCB ",4))
   {
      PrintPC();
      p = ParseByteData(p+4);
      p += strlen(p);
   }
   else if (!strncasecmp(p,"FCC ",4))
   {
      PrintPC();
      p = ParseStringData(p+4);
      p += strlen(p);
   }
   else if (!strncasecmp(p,"INCLUDE",7))
   {
      PrintPC();
      p = IncludeFile(p+7);
   }

   if (pc > 0x10000)
   {
      ErrorMsg("Program counter overflow\n");
      ErrorLine(p);
      exit(1);
   }
   return p;
}




void AdjustOpcode(char *p)
{
}


void CheckSkip(void)
{
   int i;

   Skipping = 0;
   for (i=1 ; i <= IfLevel ; ++i) Skipping |= SkipLine[i];
}

int CheckCondition(char *p)
{
   int r,v,Ifdef,Ifval;
   r = 0;
   if (*p != '#') return 0; // No preprocessing
   p = SkipSpace(p+1);
   if (!strncasecmp(p,"error", 5) && (Phase == 1))
   {
      CheckSkip();
      if (Skipping)
         return 0;          // Include line in listing
      char *msg = MallocOrDie(strlen(p+6) + 2);
      strcpy(msg, p+6);
      strcat(msg, "\n");
      ErrorMsg(msg);
      free(msg);
      exit(1);
   }
   Ifdef = !strncasecmp(p,"ifdef ",6);
   Ifval = !strncasecmp(p,"if "   ,3);
   if (Ifdef || Ifval)
   {
      r = 1;
      IfLevel++;
      if (IfLevel > 9)
      {
         ++ErrNum;
         ErrorMsg("More than 10  #IF or #IFDEF conditions nested\n");
         exit(1);
      }
      if (Ifdef)
      {
         p = EvalOperand(p+6,&v,0);
         SkipLine[IfLevel] = v == UNDEF;
      }
      else // if (Ifval)
      {
         p = EvalOperand(p+3,&v,0);
         SkipLine[IfLevel] = v == UNDEF || v == 0;
      }
      CheckSkip();
      if (Phase == 2)
      {
         PrintLiNo(1);
         if (SkipLine[IfLevel])
            fprintf(lf,"%4.4x FALSE    %s\n",SkipLine[IfLevel],Line);
         else
            fprintf(lf,"0000 TRUE     %s\n",Line);
      }
      if (df) fprintf(df,"%5d %4.4x          %s\n",LiNo,SkipLine[IfLevel],Line);
   }
   else if (!strncasecmp(p,"else",4) && (p[4] == 0 || isspace(p[4])))
   {
      r = 1;
      SkipLine[IfLevel] = !SkipLine[IfLevel];
      CheckSkip();
      PrintLiNo(1);
      if (Phase == 2) fprintf(lf,"              %s\n",Line);
   }
   if (!strncasecmp(p,"endif",5) && (p[5] == 0 || isspace(p[5])))
   {
      r = 1;
      IfLevel--;
      PrintLiNo(1);
      if (Phase == 2) fprintf(lf,"              %s\n",Line);
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

   if (!strncasecmp(Mat[n].Mne,"LDMD",4)) return 1;

   r = Mat[n].Mne[strlen(Mat[n].Mne)-1];

   if (r == 'A' || r == 'B' || r == 'C' || r == 'E' || r == 'F') return 1;
   if (r == 'D' || r == 'X' || r == 'Y' || r == 'W') return 2;
   if (r == 'S' || r == 'U'            ) return 2;
   if (r == 'Q') return 4;

   ErrorMsg("Illegal register name [%c]",r);
   exit(1);
}


char *ScanRegister(char *p, int *v)
{
   int i;
   char *q;

   for (i=15 ; i >= 0 ; --i)
   {
      if (!strncasecmp(RegisterNames[i],p,strlen(RegisterNames[i]))) break;
   }
   if (i < 0)
   {
      ErrorLine(p);
      ErrorMsg("Unknown register name\n");
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
      ErrorMsg("Illegal register name for TFM\n");
      exit(1);
   }
   *v = i;
   return p+1;
}


void OperandError(void)
{
   ++ErrNum;
   ErrorMsg("Syntax error in operand\n");
   exit(1);
}


int PostIndexReg(int reg,char c)
{
   switch(c)
   {
      case 'X': if (reg < 0) reg = 0x00; else OperandError(); break;
      case 'Y': if (reg < 0) reg = 0x20; else OperandError(); break;
      case 'U': if (reg < 0) reg = 0x40; else OperandError(); break;
      case 'S': if (reg < 0) reg = 0x60; else OperandError(); break;
      default : OperandError();
   }
   return reg;
}

int SetPostByte(char *p, int *v)
{
   int inc,dec,reg,ofr,amo,off,ind,opl;

   ind =  0;  // indirect bit
   inc =  0;  // auto increment counter
   dec =  0;  // auto decrement counter
   reg = -1;  // index  register
   ofr = -1;  // offset register
   amo = -1;  // address mode bits
   off =  0;  // constant offset
   opl = strlen(p);

   // indirect

   if (p[0] == '[' && p[opl-1] == ']')
   {
      ind = 0x10;
      p[opl-1] = 0;
      --opl;
      ++p;
   }
   if ((ind = 0x10 * (*p == '['))) ++p;

   // A,R

   if (p[0] == 'A' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p[2]);
      ql = 0;
      return (0x80 | reg | ind | 0x06);
   }

   // B,R

   if (p[0] == 'B' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p[2]);
      ql = 0;
      return (0x80 | reg | ind | 0x05);
   }

   // D,R

   if (p[0] == 'D' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p[2]);
      ql = 0;
      return (0x80 | reg | ind | 0x0b);
   }

   // E,R

   if (p[0] == 'E' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p[2]);
      ql = 0;
      return (0x80 | reg | ind | 0x07);
   }

   // F,R

   if (p[0] == 'F' && p[1] == ',')
   {
      reg = PostIndexReg(reg,p[2]);
      ql = 0;
      return (0x80 | reg | ind | 0x0a);
   }

   // PC relative

   if ((opl > 4 && strcmp(p+opl-4,",PCR") == 0) ||
       (opl > 3 && strcmp(p+opl-3,",PC" ) == 0))
   {
      p = EvalOperand(p,&off,0);
      off -= pc+3;
      if (off >= -128 && off < 128 && ROM[pc] != 0x8d)
      {
         ql = 1;
         *v = off;
         return (0x8c | ind);
      }
      else
      {
         ql = 2;
         *v = off-1;
         ROM[pc] = 0x8d;
         return (0x8d | ind);
      }
   }

   // constant offset

   if (*p != ',')
   {
      p = EvalOperand(p,&off,0);
   }

   // zero offset

   if (*p == ',' && off == 0)
   {
      while (*(++p) && *p != ' ')
      {
         switch (*p)
         {
            case '+': ++inc; if (reg <  0) OperandError(); break;
            case '-': ++dec; if (reg >= 0) OperandError(); break;
            default : reg = PostIndexReg(reg,*p);
         }
              if (inc == 1 && dec == 0) amo = 0x00;
         else if (inc == 2 && dec == 0) amo = 0x01;
         else if (inc == 0 && dec == 1) amo = 0x02;
         else if (inc == 0 && dec == 2) amo = 0x03;
         else if (inc == 0 && dec == 0) amo = 0x04;
         else OperandError();
      }
      ql = 0; // no address
      return (0x80 | reg | amo);
   }

   // constant offset

   if (*p == ',')
   {
      *v = off;
      reg = PostIndexReg(reg,*(++p));
      if (off >= -16 && off < 16 && ind == 0) // 5 bit offset
      {
         ql = 0; // no following bytes
         return (reg | (off & 0x1f));
      }
      else if (off >= -128 && off < 128)     // 8 bit offset
      {
         ql = 1; // one following byte
         return (0x80 | reg | ind | 0x08);
      }
      else                                  // 16 bit offset
      {
         ql = 2; // two following bytes
         return (0x80 | reg | ind | 0x09);
      }
   }


   // accumulator offset
   // auto increment
   // auto decrement

   OperandError();
   return -1;
}


int ScanPushList(char *p)
{
   int i,l,v;
   char *Reg;

   v = 0;
   while (*p && *p != ' ')
   {
      for (i=9 ; i >= 0 ; --i) // Scan DP before D
      {
         Reg = PushList[i].Reg;
         l = strlen(Reg);
         if (!strncasecmp(p,Reg,l)) break;
      }
      if (i < 0) OperandError();
      v |= PushList[i].Val;
      p += l;
      if (*p != ',' && *p != 0) OperandError();
      if (*p == ',') ++p;
   }
   return v;
}


char *GenerateCode(char *p)
{
   int ibi = 0; // instruction byte index
   int l,v,rd;
   int r1,r2,qc,XIM;
   char *q;
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
      XIM = ((Mat[MneIndex].Opc[AM_Extended] << 8) | (v & 0xff));
      ol  = 2;
      if (*p == ',') ++p;
      else
      {
         ErrorLine(p);
         ErrorMsg("Immediate value must be followed by comma\n");
         exit(1);
      }
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
      EvalOperand(OpText,&v,0);
      if (v != UNDEF) v  -= (pc + il);
      if (Phase == 2 && v == UNDEF)
      {
         ErrorLine(p);
         ErrorMsg("Branch to undefined label\n");
         exit(1);
      }
      if (Phase == 2 && ql == 1 && (v < -128 || v > 127))
      {
         ErrorLine(p);
         ErrorMsg("Short Branch out of range (%d)\n",v);
         exit(1);
      }
      if (df) fprintf(df,"branch %4.4x -> %4.4x : %4.4x\n",pc,v,v-pc-il);

      // optimise ?

      if (Phase == 2 && ql == 2 && v >= -128 && v < 128)
      {
         fprintf(of,"%4s to %3s range %4d on line %5d\n",
            Mat[MneIndex].Mne,Mat[MneIndex].Mne+1,v,LiNo);
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
      EvalOperand(OpText+1,&v,0);
      ol = 1 + (oc > 255);
      ql = RegisterSize(MneIndex);
      il = ol + ql;
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
      if (OpText[l-1] == ']') OpText[l-1] = 0;
      else
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

   // indexed address mode

   else if (strchr(p,','))
   {
      oc = Mat[MneIndex].Opc[AM_Indexed];
      if (oc < 0)
      {
         ++ErrNum;
         ErrorLine(p);
         ErrorMsg("Illegal indexed instruction %s %s\n",Mat[MneIndex].Mne,OpText);
         exit(1);
      }
      pb = SetPostByte(OpText,&v);
      ol = 1 + (oc > 255); // opcode length
      il = ol + 1 + ql;    // opcode + postbyte + address
   }

   // all other

   else
   {
      p = EvalOperand(p,&v,0);

      // extended address mode

      if (XIM) oc = XIM;
      else     oc = Mat[MneIndex].Opc[AM_Extended];
      if (oc >= 0)
      {
         ol = 1 + (oc > 255); // opcode length
         ql = 2;
         il = ol + 2;

         // direct address mode

         if (XIM) qc = oc & 0xfff;
         else     qc = Mat[MneIndex].Opc[AM_Direct];
         if (*p == '<' ||
            (ex == 0 && *p != '>' && qc >= 0 && v != UNDEF && (v >> 8) == DP))
         {
            oc = qc;
            v &= 0xff;
            ql = 1;
            il = ol + 1;
         }
      }

      // optimise JSR to BSR

      rd = v - pc - 3;
      if (Phase == 2 && oc == 0xbd && rd >= -128 && rd < 128)
      {
         fprintf(of," JSR to BSR range %4d on line %5d %s\n",
            rd,LiNo,Line);
      }

      // optimise JMP to BRA

      rd = v - pc - 3;
      if (Phase == 2 && oc == 0x7e && rd >= -128 && rd < 128)
      {
         fprintf(of," JMP to BRA range %4d on line %5d %s\n",
            rd,LiNo,Line);
      }
   }

   AdjustOpcode(p);

   if (Phase == 2)
   {
      if (v == UNDEF && ql > 0)
      {
         ErrorLine(p);
         ErrorMsg("Use of an undefined label\n");
         exit(1);
      }

      // insert binary code

      if (oc > 255) // two byte opcode
      {
         ROM[pc  ] = oc >> 8;
         ROM[pc+1] = oc;
         ibi = 2;
      }
      else
      {
         ROM[pc] = oc;
         ibi = 1;
      }

      if (pb >= 0) // post byte
      {
         ROM[pc+ibi++] = pb;
      }

      if (ql == 4) // 32 bit value
      {
         ROM[pc+ibi++] = v >> 24;
         ROM[pc+ibi++] = v >> 16;
         ROM[pc+ibi++] = v >>  8;
         ROM[pc+ibi  ] = v;
      }

      if (ql == 2) // 16 bit value
      {
         ROM[pc+ibi++] = v >> 8;
         ROM[pc+ibi  ] = v;
      }

      if (ql == 1) //  8 bit value
      {
         ROM[pc+ibi  ] = v;
      }

      PrintPC();
      PrintOC(v);
      fprintf(lf," %s",Line);
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
      p = GetMacroArg(p,sym);
      l = strlen(sym);
      if (l) strcpy(args+ptr[n],sym);
      else   args[ptr[n]] = 0;
      ++n;
      ptr[n] = ptr[n-1] + l + 1;
      p = SkipSpace(p);
      if (*p == ')') break; // end of list
      if (*p != ',' && n < nargs-1)
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
   while (*p && n < 10)
   {
      p = SkipSpace(p);
      if (strncasecmp(p,"ARG",3)) break; // end of list
      p = NextSymbol(p,sym);
      l = strlen(sym);
      if (l) strcpy(args+ptr[n],sym);
      else   args[ptr[n]] = 0;
      ++n;
      ptr[n] = ptr[n-1] + l + 1;
      p = SkipSpace(p);
      if (*p != ',') break; // end of list
      ++p; // skip comma
   }
   return n;
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
   mf = strncasecmp(p,"MACRO",5); // 1 : name MACRO
   if (!mf) p += 5;               // 0 : MACRO name

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
   else    an = ScanArguments(p+1,args,ap,10);
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
   if (j < 0)
   {
      j = Macros;
      Mac[j].Name = MallocOrDie(l+1);
      strcpy(Mac[j].Name,Macro);
      Mac[j].Narg = an;
      Mac[j].Type = mf;
      fgets(Line,sizeof(Line),sf);
      while (!feof(sf) && !Strcasestr(Line,"ENDM"))
      {
         ++LiNo;
         l = strlen(Line);
         if (l && Line[l-1] == 10) Line[--l] = 0; // Remove linefeed
         if (l && Line[l-1] == 13) Line[--l] = 0; // Remove return
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
                  *b++ = '&';
                  *b++ = '0' + i;
                  p += al;
                  break;
               }
            }
            if (i == an) *b++ = *p++;
         }
         *b++ = '\n';
         *b = 0;
         l = strlen(Buf);
         if (bl == 1)
         {
            bl = l+1;
            Mac[j].Body = MallocOrDie(bl);
            strcpy(Mac[j].Body,Buf);
         }
         else
         {
            bl += l;
            Mac[j].Body = ReallocOrDie(Mac[j].Body,bl);
            strcat(Mac[j].Body,Buf);
         }
         fgets(Line,sizeof(Line),sf);
      }
      Macros++;
   }
   else if (Phase == 2) // List macro
   {
      PrintLiNo(1);
      ++LiNo;
      fprintf(lf,"            %s\n",Line);
      do
      {
         fgets(Line,sizeof(Line),sf);
         PrintLiNo(1);
         ++LiNo;
         fprintf(lf,"            %s",Line);
         if (pf) fprintf(pf,"%s",Line);
      } while (!feof(sf) && !Strcasestr(Line,"ENDM"));
      LiNo-=2;
   }
   else if (Phase == 1)
   {
      ++ErrNum;
      ErrorMsg("Duplicate macro [%s]\n",Macro);
      exit(1);
   }
   if (df) fprintf(df,"Macro [%s] = %s\n",Mac[j].Name,Mac[j].Body);
   ++LiNo;
}


int ExpandMacro(char *m)
{
   int j,an;
   char *p;
   char Macro[ML];

   j = MacroIndex(m);
   if (j < 0) return j;
   if (df) fprintf(df,"Expanding [%s] phase %d\n",Mac[j].Name,Phase);

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
   CurrentMacro = j;
   ++InsideMacro;
   MacroPointer = Mac[j].Body;

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

   if (!WithLiNo) --LiNo; // -n : count macro lines
   if (*MacroPointer)
   {
      while (*MacroPointer && *MacroPointer != '\n')
      {
         if (*MacroPointer == '&')
         {
            i = *(++MacroPointer) - '0';
            r = MacArgs + ArgPtr[i];
            while (*r) *w++ = *r++;
            ++MacroPointer;
         }
         else *w++ = *MacroPointer++;
      }
      if (*MacroPointer == '\n') ++MacroPointer;
   }
   else
   {
      CurrentMacro = 0;
      --InsideMacro;
      MacroPointer = NULL;
      MacroStopped = 1;
   }
   *w = 0;
}


char *IsData(char *p)
{
   p = SkipSpace(p+1);
   if (pc < 0 && strncasecmp(p,"ORG",3) && strncasecmp(p,"BSS",3) &&
       strncasecmp(p,"STORE",5))
   {
      ErrorLine(p);
      ErrorMsg("Undefined program counter (PC)\n");
      exit(1);
   }
        if (!strncasecmp(p,"WORD",4))    p = ParseWordData(p+4);
   else if (!strncasecmp(p,"BYTE",4))    p = ParseByteData(p+4);
   else if (!strncasecmp(p,"BITS",4))    p = ParseBitData(p+4);
   else if (!strncasecmp(p,"CHA" ,3))    p = ParseCharData(p+3);
   else if (!strncasecmp(p,"REAL",4))    p = ParseRealData(p+4);
   else if (!strncasecmp(p,"FILL",4))    p = ParseFillData(p+4);
   else if (!strncasecmp(p,"BSS",3))     p = ParseBSSData(p+4);
   else if (!strncasecmp(p,"STORE",5))   p = ParseStoreData(p+5);
   else if (!strncasecmp(p,"CASE",4))    p = ParseCaseData(p+4);
   else if (!strncasecmp(p,"ORG",3))     p = SetPC(p);
   else if (!strncasecmp(p,"INCLUDE",7)) p = IncludeFile(p+7);
   else if (!strncasecmp(p,"SIZE",4))    p = ListSizeInfo(p);
   else if (!strncasecmp(p,"END",3))
   {
      p += 3; ForcedEnd = 1;
      PrintLine();
   }
   else
   {
      ErrorMsg("Unknown pseudo op\n");
      ErrorLine(p);
      exit(1);
   }
   if (pc > 0x10000)
   {
      ErrorMsg("Program counter overflow\n");
      ErrorLine(p);
      exit(1);
   }
   return SkipSpace(p);
}

void ParseLine(char *cp)
{
   int i,v,m;

   ex =  0;
   am = -1;
   oc = -1;
   Label[0] = 0;
   OpText[0] = 0;
   Comment[0] = 0;
   cp = SkipHexCode(cp);        // Skip disassembly
   cp = SkipSpace(cp);          // Skip leading blanks
   if (!strncmp(cp,"****",4)) ModuleTrigger = LiNo;
   if (CheckCondition(cp)) return;
   if (Skipping)
   {
      PrintLiNo(1);
      if (Phase == 2) fprintf(lf,"SKIP          %s\n",Line);
      if (df)         fprintf(df,"%5d SKIP          %s\n",LiNo,Line);
      return;
   }
   if (pf && Phase == 2 && !InsideMacro)
   {
       if (MacroStopped) MacroStopped = 0;
       else fprintf(pf,"%s\n",Line); // write to preprocessed file
   }
   if (strncmp(cp,"/*",2) == 0 || strncmp(cp,"\\*",2) == 0) // SDDRIVE comment style
   {
      CodeStyle = 1;
      if (Phase == 2) PrintLine();
      return;
   }
   if (*cp == 0 || *cp == '*' || *cp == ';')  // Empty or comment only
   {
      if (Phase == 2)
      {
          if (*cp == ';' || *cp == '*') PrintLine();
          else            PrintLiNo(-1);
      }
      return;
   }
   cp = CheckPseudo(cp);        // Pseudo Ops
   if (isalpha(*cp))            // Macro, Label or mnemonic
   {
      if (StrMatch(cp,"MACRO"))
      {
         RecordMacro(cp);
         return;
      }
      if ((MneIndex = IsInstruction(cp)) < 0)
      {
         m = ExpandMacro(cp);
         if (m < 0)
         {
            cp = DefineLabel(cp,&v,0);
            if (ModuleTrigger == LiNo-1) ModuleStart = v;
            cp = SkipSpace(cp);         // Skip leading blanks
            if (*cp) m = ExpandMacro(cp);   // Macro after label
            if (m >= 0) cp += strlen(cp);         // advance to EOL
         }
         else cp += strlen(cp);         // advance to EOL
         if (m < 0 && (*cp == 0 || *cp == ';')) // no code or data
         {
            PrintLiNo(1);
            if (Phase == 2)
               fprintf(lf,"%4.4x              %s\n",v&0xffff,Line);
            return;
         }
      }
   }
   if (*cp == '!') cp = IsData(cp);    // BS9 enhancements
   if (*cp ==  0 ) return;             // No code
   if (*cp == ';') return;             // No code
   if (*cp == '&') cp = SetBSS(cp);    // Set BSS counter
   if (ForcedEnd) return;
   cp = CheckPseudo(cp);        // Pseudo Ops
   if (MneIndex < 0) MneIndex = IsInstruction(cp); // Check for mnemonic after label
   if (MneIndex >= 0)
   {
      ExtractOpText(cp+strlen(Mat[MneIndex].Mne));
      cp += strlen(cp);
      GenerateCode(OpText);
      if ((Phase == 2 && ModuleStart != UNDEF && ModuleStart != 0) &&
          (oc == 0x39 || (oc == 0x35 && (ROM[pc-1] & 0x80))))
      {       //  RTS           PULS                   PC
         i = AddressIndex(ModuleStart);
         if (i >= 0)
         {
            fprintf(lf,"   ; Size%5d [%s]",pc-ModuleStart,lab[i].Name);
            ModuleStart = 0;
         }
      }
   }
   if (Phase == 2) fprintf(lf,"\n");
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
   PrintLiNo(1);
   if (Phase == 2)
   {
      fprintf(lf,";                       closed INCLUDE file %s\n",
            IncludeStack[IncludeLevel].Src);
   }
   fclose(sf);
   free(IncludeStack[IncludeLevel].Src);
   sf = IncludeStack[--IncludeLevel].fp;
   LiNo = IncludeStack[IncludeLevel].LiNo;
   fgets(Line,sizeof(Line),sf);
   ForcedEnd = 0;
   return feof(sf);
}

void Phase1(void)
{
    int l,Eof;

   Phase = 1;
   ForcedEnd = 0;
   fgets(Line,sizeof(Line),sf);
   Eof = feof(sf);
   while (!Eof || IncludeLevel > 0)
   {
      if (df) fprintf(df,"Phase 1: %4.4x %s",pc,Line);
      ++LiNo; ++TotalLiNo;
      l = strlen(Line);
      if (l && Line[l-1] == 10) Line[--l] = 0; // Remove linefeed
      if (l && Line[l-1] == 13) Line[--l] = 0; // Remove return
      ParseLine(Line);
      if (InsideMacro)
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
    int l,Eof;

   Phase =  2;
   pc    = -1;
   ForcedEnd = 0;
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
      if (InsideMacro) NextMacLine(Line);
      else fgets(Line,sizeof(Line),sf);
      Eof = feof(sf) || ForcedEnd;
      if (Eof && IncludeLevel > 0) Eof = CloseInclude();
      if (df) fprintf(df,"Phase 2:[%s] EOF=%d\n",Line,Eof);
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
         printf("* Undefined   : %-25.25s *\n",lab[i].Name);
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

void WriteBinaries(void)
{
   int i;
   unsigned char lo,hi;
   FILE *bf;

   for (i=0 ; i < StoreCount ; ++i)
   {
      if (df) fprintf(df,"Storing $%4.4x - $%4.4x <%s>\n",
                      SFA[i],SFA[i]+SFL[i],SFF[i]);
      bf = fopen(SFF[i],"wb");
      if (WriteLoadAddress)
      {
         lo = SFA[i] & 0xff;
         hi = SFA[i]  >>  8;
         fwrite(&lo,1,1,bf);
         fwrite(&hi,1,1,bf);
      }
      fwrite(ROM+SFA[i],1,SFL[i],bf);
      fclose(bf);
   }
}

const char *StatOn  = "On ";
const char *StatOff = "Off";

const char *Stat(int o)
{
   if (o) return StatOn ;
   else   return StatOff;
}

int main(int argc, char *argv[])
{
   int ic,v;

   for (ic=1 ; ic < argc ; ++ic)
   {
      if (!strcmp(argv[ic],"-x")) SkipHex = 1;
      else if (!strcmp(argv[ic],"-d")) Debug = 1;
      else if (!strcmp(argv[ic],"-i")) IgnoreCase = 1;
      else if (!strcmp(argv[ic],"-n")) WithLiNo = 1;
      else if (!strcmp(argv[ic],"-p")) Preprocess = 1;
      else if (!strncmp(argv[ic],"-D",2)) DefineLabel(argv[ic]+2,&v,1);
      else if (argv[ic][0] >= '0' || argv[ic][0] == '.')
      {
         if (!Src)
         {
              Src = MallocOrDie(strlen(argv[ic]) + 4 + 1);
              strcpy(Src,argv[ic]);
         }
         else if (!Lst[0]) strcpy(Lst,argv[ic]);
      }
      else
      {
         printf("\nUsage: bs9 [-d -D -i -x] <source> <list>\n");
         exit(1);
      }
   }
   if (!Src)
   {
      printf("*** missing filename for assembler source file ***\n");
      printf("\nUsage: bs9 [-d -D -i -n -x] <source> [<list>]\n");
      printf("   -d print details in file <Debug.lst>\n");
      printf("   -D Define symbols\n");
      printf("   -i ignore case in symbols\n");
      printf("   -n include line numbers in listing\n");
      printf("   -p print preprocessed source\n");
      printf("   -x assemble listing file - skip hex in front\n");
      exit(1);
   }

   // default file names if only source file specified:
   // prog.as9   prog   prog.lst   prog.opt

   strcpy(Pre,Src);
   strcpy(Opt,Src);
   strcat(Pre,".pp");
   if (!Lst[0]) strcpy(Lst,Src);
   if (!(strlen(Src) > 4 && !strcasecmp(Src+strlen(Src)-4,".as9")))
       strcat(Src,".as9");
   if ( (strlen(Lst) > 4 && !strcasecmp(Lst+strlen(Lst)-4,".as9")))
       Lst[strlen(Lst)-4] = 0;
   if (!(strlen(Lst) > 4 && !strcasecmp(Lst+strlen(Lst)-4,".lst")))
       strcat(Lst,".lst");
   strcat(Opt,".opt");

   printf("\n");
   printf("*******************************************\n");
   printf("* Bit Shift Assembler 30-Oct-2019         *\n");
   printf("* --------------------------------------- *\n");
   printf("* Source: %-31.31s *\n",Src);
   printf("* List  : %-31.31s *\n",Lst);
   printf("* -d:%s     -i:%s     -n:%s     -x:%s *\n",
         Stat(Debug),Stat(IgnoreCase),Stat(WithLiNo),Stat(SkipHex));
   printf("*******************************************\n");

   sf = fopen(Src,"r");
   if (!sf)
   {
      printf("Could not open <%s>\n",Src);
      exit(1);
   }
   IncludeStack[0].fp = sf;
   IncludeStack[0].Src = Src;
   lf = fopen(Lst,"w");  // Listing
   if (Debug) df = fopen("Debug.lst","w");
   if (Preprocess) pf = fopen(Pre,"w");
   of = fopen(Opt,"w");

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
   fclose(sf);
   fclose(lf);
   if (df) fclose(df);
   fclose(of);
   printf("* Source Lines: %6d                    *\n",TotalLiNo);
   printf("* Symbols     : %6d                    *\n",Labels);
   printf("* Macros      : %6d                    *\n",Macros);
   printf("*******************************************\n");
   if (ErrNum)
      printf("* %3d error%s occured%s                      *\n",
             ErrNum, ErrNum == 1 ? "" : "s", ErrNum == 1 ? " " : "");
   else printf("* OK, no errors                           *\n");
   printf("*******************************************\n");
   printf("\n");
   return 0;
}