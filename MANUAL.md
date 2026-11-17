# BS9 Reference Manual

BS9 (Bit Shift Assembler) is a cross-assembler for the Motorola **6809** and
the Hitachi **6309** CPU. It runs on any host with a standard C compiler
(Linux, macOS, Windows, ...) and produces binary images, Motorola S-records
or Sparrow files for machines such as the Thomson MO/TO series,
Dragon 32/64, Tandy CoCo or Commodore SuperPET.

This manual describes the assembler as implemented in `bs9.c`.

---

## Contents

1. [Building and installing](#1-building-and-installing)
2. [Running the assembler](#2-running-the-assembler)
3. [Source format](#3-source-format)
4. [Symbols and labels](#4-symbols-and-labels)
5. [Constants and expressions](#5-constants-and-expressions)
6. [Instructions and addressing modes](#6-instructions-and-addressing-modes)
7. [Directives (pseudo ops)](#7-directives-pseudo-ops)
8. [Data definition](#8-data-definition)
9. [Conditional assembly](#9-conditional-assembly)
10. [Macros](#10-macros)
11. [Modules and local symbols](#11-modules-and-local-symbols)
12. [Output files](#12-output-files)
13. [Listing format](#13-listing-format)
14. [Optimisation and passes](#14-optimisation-and-passes)
15. [Errors](#15-errors)
16. [Limits](#16-limits)
17. [Differences to other assemblers](#17-differences-to-other-assemblers)

---

## 1. Building and installing

```
gcc -o bs9 bs9.c -lm
```

or, with GNU make:

```
make                 # builds bs9 and form9
sudo make install    # installs to /usr/local/bin (PREFIX=... to change)
make syntax          # installs the vim syntax file 6809.vim
make WIN=y           # cross-compiles a Windows executable with MinGW
```

---

## 2. Running the assembler

```
bs9 [options] <source>
```

If `<source>` has no extension of the form `.xxx`, the extension `.as9`
is appended: `bs9 hello` reads `hello.as9`.

### Options

| Option | Meaning |
|--------|---------|
| `-D name` / `-Dname` | Define symbol `name` with value 1 |
| `-D name=value` / `-Dname=value` | Define symbol `name` with `value` (any expression) |
| `-I path` / `-Ipath` | Include path, searched if an `INCLUDE` file is not found (only one) |
| `-i` | Ignore case of symbols (same as `CASE -` at the start) |
| `-l value` / `-lvalue` | Preset all memory with a byte value (`255`, `$FF` or `0xFF`) |
| `-m` | Motorola/Thomson code style: a blank ends the operand, the rest of the line is a comment |
| `-n` | Print source line numbers in the listing |
| `-o` | Do not optimise branches and jumps (see [section 14](#14-optimisation-and-passes)) |
| `-p` | Write the preprocessed source (macros expanded, conditionals resolved) to `<name>.pp` |
| `-q` | Quiet: print only errors |
| `-d` | Write debug information to `Debug.lst` |
| `-x` | Assemble a listing file: skip the hex columns in front of each line |
| `-h` | Print the option summary |

Symbols defined with `-D` can be tested with `IF`, `IFDEF` and `IFNDEF`.
Defining the same symbol again in the source with a different value, or
as a label, is an error.

### Files

| File | Content |
|------|---------|
| `<name>.as9` | Source |
| `<name>.ls9` | Listing with cross reference (always written) |
| `<name>.sym` | Symbol file |
| `<name>.pp`  | Preprocessed source (`-p`) |
| `Debug.lst`  | Debug output (`-d`) |

Binary output is not written automatically. It is requested in the source
with `STORE` (see [section 12](#12-output-files)).

The exit code is the number of errors (0 = success).

---

## 3. Source format

A source line has the form

```
[label[:]]  [mnemonic | directive | macro call]  [operand]  [; comment]
```

* Mnemonics, directives and register names are **not** case sensitive.
* Symbols are case sensitive unless `-i` or `CASE -` is used.
* A label may be indented. Every word at the start of a line that is not a
  mnemonic, directive, keyword or macro name is taken as a label. A
  misspelled mnemonic therefore becomes a label without an error message;
  check the symbol table for unused symbols.
* A colon after a label is optional and ignored.
* The maximum line length is 255 characters.

### Comments

| Form | Meaning |
|------|---------|
| `; text` | Comment to the end of the line (also after an instruction) |
| `*` at line start | Comment line, unless it is `* = address` |
| empty line | Ignored |
| line starting with `/*` or `\*` | Comment; also switches to `-m` code style |

Without `-m`, text after a complete instruction that is not preceded by
`;` is an error (`NOP 5`, `LDA $1234 junk`). With `-m` everything after the
first blank of the operand field is a comment.

---

## 4. Symbols and labels

Symbol names start with a letter, `_` or `.` and may contain letters,
digits, `_`, `.` and `$`. Names starting with `.` are local to the current
module (see [section 11](#11-modules-and-local-symbols)).

Mnemonics and directive names cannot be used as symbols.

### Defining symbols

```
LABEL     LDX  #0          ; LABEL = current program counter
TXTPTR    =    $21B8       ; constant
OLDPTR    EQU  $21BA       ; constant (same as =)
COUNT     SET  5           ; variable, may be assigned again with SET
RED       ENUM 1           ; enumeration: RED = 1
GREEN     ENUM             ; last ENUM value + 1 = 2
BLUE      ENUM             ; 3
CURSOR    BSS  2           ; CURSOR = BSS counter, BSS counter += 2
```

Rules:

* A label, `EQU`/`=`/`ENUM` constant or `BSS` label can be defined only
  once. A second definition at another place is reported together with
  the place of the first definition.
* `EQU`/`=` may repeat a definition only with the **same** value.
* `SET` variables may be reassigned at any time.
* An undefined symbol used in an expression is reported at the end of
  assembly; using it in code is an error.

### Special symbols

| Symbol | Value |
|--------|-------|
| `*` | Current program counter (in an operand position) |
| `&` | Current BSS counter |
| `?name` | Number of bytes of the data line labelled `name` (see below) |

`?name` gives the length of the `BYTE`, `FCB`, `FCC`, `WORD`, `FDB` or
`LONG` line at the address of `name`:

```
Msg     BYTE "Hello"
        LDB  #?Msg          ; LDB #5
```

---

## 5. Constants and expressions

All values are 32 bit signed integers. The value `$5A5AA5A5` is reserved
internally as the marker for undefined values and cannot be used.

### Constants

| Form | Example | Value |
|------|---------|-------|
| decimal | `100` | 100 |
| hexadecimal | `$FFD2` | $FFD2 |
| hexadecimal (suffix) | `0FFh`, `1234H` | max. 5 digits, must start with a digit |
| binary | `%10100101` | $A5 |
| binary with blanks | `%1010 0101` | $A5 |
| binary with symbols | `%..*.*.**` | `*` = 1, `.` = 0 |
| character | `'A'` | $41 |
| character escape | `'\n'` `'\r'` `'\a'` `'\e'` `'\0'` | 10, 13, 7, 27, 0 |
| apostrophe / backslash | `'\''` / `'\'` | $27 / $5C |
| multi character | `"AB"` | $4142 (1 to 4 characters, big endian) |

A decimal constant with a trailing `A`-`F` is an error ("leading $ for hex
missing").

### Operators

Unary operators:

| Operator | Meaning |
|----------|---------|
| `+` `-` | sign |
| `!` | logical NOT (result 0 or 1) |
| `~` | bitwise NOT |
| `<` | force direct / 8 bit mode (does **not** select the low byte) |
| `>` | force extended / 16 bit mode |
| `( )` `[ ]` | grouping (both kinds of brackets) |

Binary operators, highest priority first (same priority: left to right):

| Priority | Operators | Meaning |
|----------|-----------|---------|
| 11 | `*` `/` | multiplication, integer division |
| 10 | `+` `-` | addition, subtraction |
| 9  | `<<` `>>` | shift left, arithmetic shift right |
| 8  | `<` `<=` `>` `>=` | comparison (result 0 or 1) |
| 7  | `==` `!=` | equal, not equal |
| 6  | `&` | bitwise AND |
| 5  | `^` | bitwise XOR |
| 4  | `\|` | bitwise OR |
| 3  | `&&` | logical AND |
| 2  | `\|\|` | logical OR |

The priorities are the same as in C. Overflow of the 32 bit range,
division by zero and shifts by more than 31 bits are errors.

```
        LDX  #Table+2*[LEN-1]
        LDA  #(MASK & $F0) >> 4
        FDB  *-Start
```

---

## 6. Instructions and addressing modes

### CPU selection

```
CPU = 6309          ; full 6309 instruction set (default)
CPU = 6809          ; 6809 only: 6309 instructions and registers are errors
```

The CPU setting is reset to 6309 at the start of every pass.

### Addressing modes

| Mode | Syntax | Example |
|------|--------|---------|
| Inherent | – | `NOP`, `CLRA`, `PSHSW` |
| Immediate | `#value` | `LDA #$20`, `LDD #1000`, `LDQ #$12345678` |
| Direct | `address` (high byte = DP) | `LDA $12` |
| Extended | `address` | `LDA $1234` |
| Forced direct | `<address` | `LDA <$12` |
| Forced extended | `>address` | `LDA >$0012` |
| Extended indirect | `[address]` | `JMP [$FFFE]` |
| Relative | `label` | `BNE Loop` |
| Register | `reg,reg` | `TFR A,B` |
| Register list | `reg,reg,...` | `PSHS A,B,X` |
| Indexed | see below | `LDA 5,X` |

**Direct or extended** is chosen automatically: an address whose high
byte equals the value set with `SETDP` (default 0) uses direct mode,
otherwise extended mode. A forward reference uses extended mode until the
value is known.

**Immediate values** are checked: 8 bit registers accept -128..255,
16 bit registers -32768..65535. `LDQ #` takes a 32 bit value or a real
constant (`LDQ #1.5`, see `REAL`).

### Indexed addressing

Index registers: `X`, `Y`, `U`, `S`; on the 6309 also `W` (with
restrictions, see table).

| Syntax | Meaning |
|--------|---------|
| `,R` | no offset |
| `n,R` | constant offset (5, 8 or 16 bit chosen automatically) |
| `<n,R` | forced 8 bit offset (-128..127, otherwise error) |
| `>n,R` | forced 16 bit offset |
| `A,R` `B,R` `D,R` | accumulator offset |
| `E,R` `F,R` `W,R` | accumulator offset (6309) |
| `,R+` `,R++` | post increment by 1 / 2 |
| `,-R` `,--R` | pre decrement by 1 / 2 |
| `label,PCR` | program counter relative: offset = label - next instruction |
| `n,PC` | program counter with constant offset `n` |
| `[...]` | indirect form of all the above except `,R+`, `,-R` and 5 bit offsets |
| `,W` `,W++` `,--W` `n,W` | W register (6309), also indirect `[,W]` etc. |

An offset of 0 (`0,X`) is assembled as `,X`. The W register has only a
16 bit offset; `<n,W` is an error.

### Register to register

`TFR`, `EXG` and the 6309 instructions `ADDR`, `ADCR`, `SUBR`, `SBCR`,
`ANDR`, `ORR`, `EORR`, `CMPR` take two registers:

```
D X Y U S PC W V A B CC DP 0 E F      (6309)
D X Y U S PC     A B CC DP            (6809)
```

The sizes of the registers are not checked.

### Push and pull

```
PSHS A,B,X,PC
PULU CC,DP,Y
PSHS ALL              ; all registers ($FF)
```

`PSHS`/`PULS` cannot push `S`, `PSHU`/`PULU` cannot push `U`.

### 6309 block transfer

```
TFM X+,Y+             ; increment source and destination
TFM X-,Y-             ; decrement both
TFM X+,Y              ; increment source only
TFM X,Y+              ; increment destination only
```

Registers: `D`, `X`, `Y`, `U`, `S`.

### 6309 memory immediate (AIM, OIM, EIM, TIM)

```
AIM #$FE,$40          ; direct
AIM #$FE,$1234        ; extended
AIM #$FE,5,X          ; indexed
AIM #$FE,[,X]         ; indirect
```

### 6309 bit operations

`BAND`, `BIAND`, `BOR`, `BIOR`, `BEOR`, `BIEOR`, `LDBT`, `STBT`:

```
BAND  A.1,$40.3       ; register A bit 1, memory bit 3 at direct address $40
LDBT  CC.0,$12.7
```

Registers: `CC`, `A`, `B`. The address must be in the range 0..255
(direct page offset).

### Extra instructions

These pseudo instructions are available with `CPU = 6309`:

| Instruction | Code generated |
|-------------|----------------|
| `CLRX` | `LDX #0` |
| `CLRY` | `SUBR Y,Y` |
| `DECX` / `INCX` | `LEAX -1,X` / `LEAX 1,X` |
| `DECY` / `INCY` | `LEAY -1,Y` / `LEAY 1,Y` |

---

## 7. Directives (pseudo ops)

### Program counter and memory layout

| Directive | Meaning |
|-----------|---------|
| `ORG addr` | Set program counter (0..$10000) |
| `* = addr` | Same as `ORG` |
| `& = addr` | Set BSS counter |
| `label BSS n` | `label` = BSS counter, BSS counter += n (1..32767) |
| `BSS n` | BSS counter += n |
| `RMB n` / `DS n` | Reserve n bytes (program counter += n, memory unchanged) |
| `ALIGN n` | Align program counter to a multiple of n, fill with `NOP` ($12) |
| `ALIGN n (v)` / `ALIGN * n (v)` | Same, fill with byte v |
| `ALIGN & n` / `ALIGN & n (v)` | Align BSS counter, fill with the `-l` value or v |
| `SECT LOC=addr` | Set program counter |
| `SETDP n` | Assume direct page register = n (0..255, or an address $nn00) |
| `CPU = 6809` / `CPU = 6309` | Select instruction set |

The program counter must be set before the first code or data byte.

### Files

| Directive | Meaning |
|-----------|---------|
| `INCLUDE "file"` | Assemble another source file (nesting up to 99) |
| `LOAD addr,"file"` | Load a binary file to `addr` (program counter unchanged) |
| `LOAD "file"` | Load a binary file to the program counter and advance it |
| `STORE start,length,"file"[,format[,entry]]` | Write a binary file, see [section 12](#12-output-files) |

### Listing and assembly control

| Directive | Meaning |
|-----------|---------|
| `LIST +` / `LIST -` | Switch listing on / off |
| `MACLIST ON` / `MACLIST OFF` | List / do not list expanded macro lines (also `1` / `0`) |
| `CASE +` / `CASE -` | Symbols case sensitive / not case sensitive |
| `SIZE` | Print the size of the current module in the listing |
| `END` | Stop assembly (in an include file: end of that file) |
| `#error text` | Stop with an error message (normally inside `IF`) |
| `TTL`, `EXTERN`, `INTERN` | Ignored |
| `FORMLN n` | Lines per page (inactive) |

---

## 8. Data definition

| Directive | Meaning |
|-----------|---------|
| `BYTE`  | Bytes, strings; values outside -128..255 are stored as 16 bit words |
| `FCB` / `DB` | Bytes and strings; every value is one byte (-128..255) |
| `FCC` | Strings (same rules as `BYTE`) |
| `BHEX` | Like `BYTE`, but numbers are hexadecimal without `$` |
| `WORD` / `FDB` / `DW` | 16 bit words, big endian (-32768..65535) |
| `LONG` | 32 bit values, big endian |
| `REAL` | 32 bit floating point values |
| `FILL n (v)` | n bytes with value v |
| `BITS` | One byte from 8 bit symbols |
| `C5TO3` | Packed 5 bit characters |
| `CMAP` | One byte from 8 bit symbols (character maps) |

```
Text    BYTE  "Hello",13,0           ; 48 65 6C 6C 6F 0D 00
        BYTE  1,$1234                ; 01 12 34
        FCB   $20,"Example",0        ; 20 45 78 61 6D 70 6C 65 00
        FCB   300                    ; error: out of range
Table   WORD  Start,Stop,$0200       ; 16 bit addresses
        LONG  1000000                ; 00 0F 42 40
        FILL  16 ($FF)               ; 16 x $FF
        FILL  $A000-* (0)            ; fill up to $9FFF
        BHEX  20,FF,0D               ; 20 FF 0D
```

### Strings

Strings are enclosed in `"`. Escape sequences:

| Escape | Value | Escape | Value |
|--------|-------|--------|-------|
| `\r` | 13 (CR) | `\e` | 27 (ESC) |
| `\n` | 10 (LF) | `\0` | 0 |
| `\t` | 9 (TAB) | `\b` | 29 (bold colour) |
| `\f` | 12 (FF) | `\s` | 28 (standard colour) |
| `\a` | 7 (BEL) | `\x` | x for any other character |

A `^` after the closing quote sets bit 7 of the last character:
`FCC "AB"^` gives `41 C2`.

Special items in `BYTE`, `FCB` and `FCC`:

| Item | Meaning |
|------|---------|
| `$DATE` | Assembly date as text, e.g. `17-Nov-2026` |
| `#"ABC"` | Hashed ASCII: 3 characters `@`..`Z` packed into a 16 bit word (base 27) |

### REAL

`REAL` stores a 4 byte floating point value: one exponent byte (excess
$80) and a 24 bit mantissa with the sign in bit 7 of the first mantissa
byte (Microsoft BASIC format).

```
        REAL  1.0                    ; 81 00 00 00
        REAL  -0.5                   ; 80 80 00 00
        REAL  cosd(60)               ; cosine of 60 degrees
        REAL  $81490FDB              ; hex bytes
```

### BITS, CMAP

Eight symbols, `*` for 1 and `.` for 0, separated by blanks:

```
        BITS  * . * . * . * .        ; $AA
```

### C5TO3

Packs 4 characters into 20 bits and a type letter into bits 20..22,
giving 3 bytes. Allowed characters: `?`, blank, `A`..`Z`, `2`, `3`.
Type letters: `C`=1, `D`=2, `R`=3, `W`=4, others 0.

```
        C5TO3 "ABCDC"                ; 12 90 62
```

---

## 9. Conditional assembly

```
IF expression        ; true if expression is defined and not 0
IFDEF symbol         ; true if symbol is defined
IFNDEF symbol        ; true if symbol is not defined
ELSE
ENDIF
```

* Conditions can be nested up to 9 levels.
* An undefined symbol in `IF` counts as false.
* The old syntax `#if`, `#ifdef`, `#else`, `#endif` is accepted.
* `ELSE` without `IF` and a missing `ENDIF` are errors.

```
IFDEF MO5
        STA  $A7C0
ELSE
        STA  $E7C0
ENDIF

IF (MAXLEN & $FF00)
        #error This code is 8 bit only, MAXLEN too large!
ENDIF
```

Symbols for conditional assembly can be defined on the command line:
`bs9 -DMO5 os` or `bs9 -DVERSION=3 os`.

---

## 10. Macros

### Definition

```
MACRO Name(Arg1, Arg2, ...)
        ...
ENDM

MACRO Name                     ; macro without arguments
        ...
ENDM
```

### Call

```
        Name(value1, value2)
Label   Name(value1, value2)   ; label before a macro call
```

Example:

```
MACRO PrintString(Message)
        LDX   #Message
        LDB   #?Message
        JSR   PrintN
ENDM

OK      BYTE "\nOK\n"
        PrintString(OK)
```

expands to

```
        LDX   #OK
        LDB   #4
        JSR   PrintN
```

Rules:

* Up to 10 arguments. The number of arguments in a call must match the
  definition.
* Arguments may contain balanced parentheses, strings and characters in
  quotes, and commas inside them.
* An argument name is replaced where it appears as a whole word (not
  part of a longer name of letters, digits, `_`, `.`, `$`). Replacement
  also happens in strings and comments.
* Macros may call other macros (nesting up to 9 levels), but not
  themselves. A macro definition cannot contain another definition.
* Macro names must not be mnemonics, directives or keywords.
* The character `` ` `` cannot be used inside a macro definition.
* An expanded line must not be longer than 255 characters.
* A label inside a macro body is defined at every call: a macro with a
  label can be called only once. Use local labels in different modules
  for macros that are called several times.

`MACLIST OFF` suppresses the expanded lines in the listing.

---

## 11. Modules and local symbols

```
MODULE Delay              ; or SUBROUTINE Delay
.loop   LEAX -1,X
        BNE  .loop
        RTS
ENDMOD                    ; or ENDSUB
```

`MODULE name` defines the label `name` at the current address and opens
a scope. Symbols starting with `.` inside the scope get the module name
as prefix: `.loop` becomes `Delay.loop`. The same local name can be used
in other modules. From outside a local symbol is accessed with its full
name (`Delay.loop`).

Local symbols can be used only inside a module.

`ENDMOD` prints the size of the module in the listing.

---

## 12. Output files

```
STORE start,length,"file"[,format[,entry]]
```

| Format | Output |
|--------|--------|
| `BIN` (default) | Raw binary. With `entry` given: a 2 byte big endian load address (`start`) precedes the data |
| `S19` / `SREC` | Motorola S-records: S0 header, S1 data records (32 bytes), S5 record count, S9 start address if `entry` is given. Lines end with CR LF. Without an extension in the file name `.S19` is appended |
| `SPARROW` | 8 byte header (`S`, 3 byte start address, 2 byte length, 2 byte checksum) followed by the data |

```
STORE $E000,$2000,"basic.rom"            ; 8 KB binary
STORE Start,End-Start,"prog.bin",bin,1   ; binary with load address
STORE Start,End-Start,"prog.s19",s19,Main ; S-records, start address Main
```

Up to 20 `STORE` commands are possible. The files are written after
successful assembly; the range must lie inside 64 KB.

### Symbol file

`<name>.sym` contains the number of symbols, a checksum line
(16 bit sum of the bytes $8000..$FFFF) and one line per symbol:

```
    9
C 0000 Checksum
  0001 FOO
B 0300 X
M 2002 Mod
  2007 Later
```

The first column is the type: `B` = BSS label, `M` = module,
`D` = defined with `-D`, blank = other.

---

## 13. Listing format

```
  27 9ff6   b6    fe30         LDA     IO_SDCARD
  28 9ff9 1034 8e              ANDR    A,E       ; test IO_INCD#
  29 9ffc   27      06         BEQ     +
  ^  ^    ^    ^  ^            ^       ^         ^
  |  |    |    |  |            |       |         \- comment
  |  |    |    |  |            |       \----------- operand
  |  |    |    |  |            \------------------- mnemonic
  |  |    |    |  \----- address/value operand (word or byte)
  |  |    |    \-------- postbyte
  |  |    \------------- opcode (byte or word)
  |  \------------------ program address
  \--------------------- line number (option -n)
```

Other lines show the value of the definition (constants, `BSS`, `SETDP`,
`CPU`), `TRUE`/`FALSE` for conditions and `SKIP` for lines in false
conditional blocks.

At the end the listing contains the symbol table three times:

1. all symbols sorted by value,
2. symbols with values $0000..$00FF sorted by number of references,
3. symbols with values $0000..$4000 sorted by number of references.

Each entry shows name, value, `L` for constants (EQU, SET, ENUM) and the
line numbers of the definition (marked `D`) and of all references.

With `-x` a listing file can be assembled again: the hex columns in front
of the source text are skipped.

---

## 14. Optimisation and passes

BS9 assembles in up to 10 passes until no label changes its value, then
generates code in a final pass.

By default branches and jumps are optimised:

* Long branches (`LBRA`, `LBEQ`, ...) are made short if the target is in
  range.
* Short branches out of range are made long (`BRA` → `LBRA`,
  `BEQ` → `LBEQ`, ...).
* `JMP` and `JSR` with a target in short branch range become `BRA` and
  `BSR`. `JMP >label` and `JSR >label` are never changed.

With `-o` long branches stay long and `JMP`/`JSR` stay unchanged. Short
branches out of range are still made long.

---

## 15. Errors

Most errors stop the assembly immediately. The message shows the file,
the line number, the source line and a marker at the position of the
error:

```
*** Error in file test.as9 line 12:
Junk after operand
 lda $1234 junk
```

Undefined symbols are listed at the end and counted as errors. The exit
code of `bs9` is the number of errors.

Typical errors:

| Message | Cause |
|---------|-------|
| `Label [X] already defined in file F line N` | Second definition of a label or constant |
| `Multiple assignments for label [X]` | `EQU` with a different value |
| `Phase error label [X]` | Label value differs in the final pass |
| `Use of an undefined label` | Undefined symbol in code |
| `Immediate value out of range` | Value does not fit the register |
| `8 bit offset out of range` | `<n,R` with n outside -128..127 |
| `FCB value ... out of range` | `FCB` value outside -128..255 (use `BYTE`) |
| `Junk after operand` | Text after a complete operand without `;` |
| `6309 instruction not allowed with CPU = 6809` | 6309 code in 6809 mode |
| `Undefined program counter (missing ORG)` | Code before the first `ORG` |

---

## 16. Limits

| Item | Limit |
|------|-------|
| Line length | 255 characters (also for expanded macro lines) |
| Symbols | 8000 |
| Macros | 200 |
| Macro arguments | 10 |
| Macro nesting | 9 levels |
| Conditional nesting | 9 levels |
| Include nesting | 99 levels |
| `STORE` commands | 20 |
| Passes | 10 |
| Address space | 64 KB |

---

## 17. Differences to other assemblers

Compared with lwasm and the Motorola assembler:

* `BYTE` stores values outside the byte range as 16 bit words. `FCB`
  stores single bytes but reports values outside -128..255 as an error
  (lwasm truncates them silently).
* Strings are allowed in `FCB`; `FCB "AB"` stores two bytes.
* `FCC` supports escape sequences and additional values, but not arbitrary
  delimiters (`FCC /text/` is not supported).
* Short branches out of range are extended automatically.
* Bit operations use the syntax `BAND A.1,$40.3` (lwasm: `BAND A,3,1,$40`)
  and expect an address 0..255.
* Additional operators `<<`, `>>`, `!`, `==`, `!=`, `<`, `<=`, `>`, `>=`,
  `&&`, `||`.
* Additional features: macros with arguments, `MODULE` scopes, `ENUM`,
  `BSS`, `?name`, `PSHS ALL`, `BHEX`, `C5TO3`, `REAL`, `$DATE`,
  hashed ASCII.
