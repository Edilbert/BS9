// code formatter for as9 assembler source
// version 1.0  12-APR-2023

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define ADMODES 8

int optl,optu,opto,optp;
int mcol =  8; // new start column for mnemonics
int ocol = 16; // new start column for operands
int ecol = 16; // new start column for equates
int ccol = 32; // new start column for comments on code lines

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

#define DIMOP (sizeof(Mat) / sizeof(struct MatStruct))

char *PseudoTab[] =
{
   "ALIGN"     ,
   "BITS"      ,
   "BSS"       ,
   "BYTE"      ,
   "C5TO3"     ,
   "CASE"      ,
   "CMAP"      ,
   "CPU"       ,
   "END"       ,
   "ENDSUB"    ,
   "EXTERN"    ,
   "FCB"       ,
   "FCC"       ,
   "FDB"       ,
   "FILL"      ,
   "FORMLN"    ,
   "INCLUDE"   ,
   "INTERN"    ,
   "LIST"      ,
   "LOAD"      ,
   "LONG"      ,
   "ORG"       ,
   "RMB"       ,
   "REAL"      ,
   "SECT"      ,
   "SETDP"     ,
   "SIZE"      ,
   "STORE"     ,
   "SUBROUTINE",
   "TTL"       ,
   "WORD"
};

#define PSEUDOS (int)(sizeof(PseudoTab) / sizeof(char *))

char Line[256];

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

int IsEquate(void)
{
   char *e;
   char *p;
   e = strchr(Line,'=');
   if (e)
   {
      p = strchr(Line,';');
      if (p && p < e) return -1; // '=' after comment character
      p = strchr(Line,'*');
      if (p && p < e) return -1; // ignore * = syntax for PC setting
      return e-Line;
   }
   return -1; // no equate character found
}


int IsInstruction(int *l)
{
   int i,j,ll;
   char c;

   if (Line[0] == '*') return -1;
   if (Line[0] == ';') return -1;
   ll = strlen(Line);
   if (ll < 4) return -1;
   *l = 0;
   for (j=0 ; j < ll-3 ; ++j)
   {
      if (Line[j] == ';') return -1;
      if (Line[j] == '"') return -1;
      if (Line[j] ==  39) return -1;
      if (j > 0)
      {
         c = Line[j-1];
         if (c != ' ') continue;
      }
      for (i = 0 ; i < DIMOP ; ++i)
      {
         *l = strlen(Mat[i].Mne);
         if (strlen(Line+j) >= *l)
         {
            if (!StrNCaseCmp(Line+j,Mat[i].Mne,*l)
            && Line[j+ *l] <= ' ') return j;
         }
      }
   }
   return -1; // No mnemonic
}

int IsPseudo(int *l)
{
   int i,j,ll;
   char c;

   if (Line[0] == '*') return -1;
   if (Line[0] == ';') return -1;
   ll = strlen(Line);
   if (ll < 4) return -1;
   *l = 0;
   for (j=0 ; j < ll-3 ; ++j)
   {
      if (Line[j] == ';') return -1;
      if (Line[j] == '"') return -1;
      if (Line[j] ==  39) return -1;
      if (j > 0)
      {
         c = Line[j-1];
         if (c != ' ') continue;
      }
      for (i = 0 ; i < PSEUDOS ; ++i)
      {
         *l = strlen(PseudoTab[i]);
         if (strlen(Line+j) >= *l)
         {
            if (!StrNCaseCmp(Line+j,PseudoTab[i],*l)
            && Line[j+ *l] <= ' ') return j;
         }
      }
   }
   return -1; // No pseudo
}

void Usage(void)
{
   fprintf(stderr,"Usage: form9 [options] <in >out\n");
   fprintf(stderr,"Options:\n");
   fprintf(stderr,"   -l print mnemonics in lower case\n");
   fprintf(stderr,"   -u print mnemonics in upper case\n");
   fprintf(stderr,"   -o print pseudos   in lower case\n");
   fprintf(stderr,"   -p print pseudos   in upper case\n");
   fprintf(stderr,"   -m col   mnemonic/pseudo  column ( 8)\n");
   fprintf(stderr,"   -a col   argument/operand column (16)\n");
   fprintf(stderr,"   -e col   equate column           (16)\n");
   fprintf(stderr,"   -c col  code line comment column (32)\n");
   exit(1);
}

int MoveLine(int i, int col)
{
   int l;
   if (col >= 0)
   {
      l = strlen(Line)-i+1;
      if (col > i)
      {
         memmove(Line+col,Line+i,l);
         memset(Line+i,' ',col-i);
         return col; // moved to this column
      }
      while (col < i)
      {
         if (i > 0 && Line[i-1] != ' ') break;
         if (i > 1 && Line[i-2] != ' ') break;
         --i;
         memmove(Line+i,Line+i+1,l);
      }
      return i; // moved to this column
   }
   return i;
}

void MoveOperand(int i)
{
   int l;
   if (ocol >= 0)
   {
      while (Line[i] == ' ') ++i;
      l = strlen(Line)-i+1;
      if (ocol > i)
      {
         memmove(Line+ocol,Line+i,l);
         memset(Line+i,' ',ocol-i);
      }
      while (ocol < i)
      {
         if (Line[i-1] != ' ') break;
         if (i > 1 && Line[i-2] != ' ') break;
         --i;
         memmove(Line+i,Line+i+1,l);
      }
   }
}

void MoveComment(int i)
{
   char *p;
   int j;

   p = strchr(Line+i,';');
   if (!p) p = strchr(Line+i,'*');
   if (p)
   {
      j = p - Line;
      if (p[1] != ' ') MoveLine(j+1,j+2);
      MoveLine(j,ccol);
   }
}

void Convert(void)
{
   int i,j,l,ll;
   fgets(Line,sizeof(Line),stdin);
   while (!feof(stdin))
   {
      ll = strlen(Line);
      if (ll > 3)
      {
         if ((i = IsInstruction(&l)) >= 0)
         {
            if (optl) for (j=i ; j < i+l ; ++j) Line[j] = tolower(Line[j]);
            if (optu) for (j=i ; j < i+l ; ++j) Line[j] = toupper(Line[j]);
            if (mcol > 0)
            {
                i = l + MoveLine(i,mcol);
                MoveOperand(i);
            }
            else i += l;
            if (ccol >= 0) MoveComment(i);
         }
         else if ((i = IsPseudo(&l)) >= 0)
         {
            if (opto) for (j=i ; j < i+l ; ++j) Line[j] = tolower(Line[j]);
            if (optp) for (j=i ; j < i+l ; ++j) Line[j] = toupper(Line[j]);
            if (mcol >= 0)
            {
                i = l + MoveLine(i,mcol);
                MoveOperand(i);
            }
            else i += l;
            if (ccol >= 0) MoveComment(i);
         }
         else if ((i = IsEquate()) >= 0)
         {
            if (ecol >= 0)
            {
                MoveLine(i,ecol);
            }
         }
      }
      fputs(Line,stdout);
      fgets(Line,sizeof(Line),stdin);
   }
}

int main(int argc, char *argv[])
{
   int ic;
   if (argc < 1) Usage();
   for (ic=1 ; ic < argc ; ++ic)
   {
           if (!strcmp(argv[ic],"-l")) optl = 1;
      else if (!strcmp(argv[ic],"-u")) optu = 1;
      else if (!strcmp(argv[ic],"-o")) opto = 1;
      else if (!strcmp(argv[ic],"-p")) optp = 1;
      else if (!strcmp(argv[ic],"-m"))
      {
         mcol = atoi(argv[++ic]);
         if (mcol < -1 || mcol > 32)
         {
            fprintf(stderr,"*** wrong mnemonic column ***\n");
            exit(1);
         }
      }
      else if (!strcmp(argv[ic],"-c"))
      {
         ccol = atoi(argv[++ic]);
         if (ccol > 72)
         {
            fprintf(stderr,"*** wrong comment column ***\n");
            exit(1);
         }
      }
      else if (!strcmp(argv[ic],"-a"))
      {
         ocol = atoi(argv[++ic]);
         if (ocol != -1 && (ocol > 72 || ocol < mcol+5))
         {
            fprintf(stderr,"*** wrong operand column ***\n");
            exit(1);
         }
      }
      else if (!strcmp(argv[ic],"-e"))
      {
         ecol = atoi(argv[++ic]);
         if (ecol != -1 && ecol > 72)
         {
            fprintf(stderr,"*** wrong equates column ***\n");
            exit(1);
         }
      }
      else Usage();
   }

   if (optl && optu || opto && optp)
   {
      fprintf(stderr,"*** option conflict ***\n");
      exit(1);
   }

   Convert();
}

