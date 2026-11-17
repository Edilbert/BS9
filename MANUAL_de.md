# BS9 Referenzhandbuch

*English version: [MANUAL.md](MANUAL.md)*

BS9 (Bit Shift Assembler) ist ein Cross-Assembler für die CPUs
Motorola **6809** und Hitachi **6309**. Er läuft auf jedem Rechner mit
einem Standard-C-Compiler (Linux, macOS, Windows, ...) und erzeugt
Binärdateien, Motorola-S-Records oder Sparrow-Dateien für Rechner wie die
Thomson-MO/TO-Serie, Dragon 32/64, Tandy CoCo oder Commodore SuperPET.

Dieses Handbuch beschreibt den Assembler so, wie er in `bs9.c`
implementiert ist.

---

## Inhalt

1. [Übersetzen und Installieren](#1-übersetzen-und-installieren)
2. [Aufruf](#2-aufruf)
3. [Aufbau des Quelltexts](#3-aufbau-des-quelltexts)
4. [Symbole und Labels](#4-symbole-und-labels)
5. [Konstanten und Ausdrücke](#5-konstanten-und-ausdrücke)
6. [Befehle und Adressierungsarten](#6-befehle-und-adressierungsarten)
7. [Direktiven (Pseudo-Ops)](#7-direktiven-pseudo-ops)
8. [Datendefinition](#8-datendefinition)
9. [Bedingte Assemblierung](#9-bedingte-assemblierung)
10. [Makros](#10-makros)
11. [Module und lokale Symbole](#11-module-und-lokale-symbole)
12. [Ausgabedateien](#12-ausgabedateien)
13. [Aufbau des Listings](#13-aufbau-des-listings)
14. [Optimierung und Durchläufe](#14-optimierung-und-durchläufe)
15. [Fehler](#15-fehler)
16. [Grenzwerte](#16-grenzwerte)
17. [Unterschiede zu anderen Assemblern](#17-unterschiede-zu-anderen-assemblern)

---

## 1. Übersetzen und Installieren

```
gcc -o bs9 bs9.c -lm
```

oder mit GNU make:

```
make                 # übersetzt bs9 und form9
sudo make install    # installiert nach /usr/local/bin (anderes Ziel mit PREFIX=...)
make syntax          # installiert die vim-Syntaxdatei 6809.vim
make WIN=y           # erzeugt mit MinGW ein Windows-Programm
```

---

## 2. Aufruf

```
bs9 [Optionen] <Quelltext>
```

Hat `<Quelltext>` keine Endung der Form `.xxx`, wird `.as9` angehängt:
`bs9 hello` liest `hello.as9`.

### Optionen

| Option | Bedeutung |
|--------|-----------|
| `-D name` / `-Dname` | Symbol `name` mit dem Wert 1 definieren |
| `-D name=wert` / `-Dname=wert` | Symbol `name` mit `wert` definieren (beliebiger Ausdruck) |
| `-I pfad` / `-Ipfad` | Suchpfad für `INCLUDE`-Dateien, die nicht gefunden werden (nur einer) |
| `-i` | Groß-/Kleinschreibung bei Symbolen ignorieren (wie `CASE -` am Anfang) |
| `-l wert` / `-lwert` | Gesamten Speicher mit einem Bytewert vorbelegen (`255`, `$FF` oder `0xFF`) |
| `-m` | Motorola-/Thomson-Stil: ein Leerzeichen beendet den Operanden, der Rest der Zeile ist Kommentar |
| `-n` | Zeilennummern im Listing ausgeben |
| `-o` | Sprünge und Verzweigungen nicht optimieren (siehe [Kapitel 14](#14-optimierung-und-durchläufe)) |
| `-p` | Vorverarbeiteten Quelltext (Makros expandiert, Bedingungen aufgelöst) nach `<name>.pp` schreiben |
| `-q` | Ruhig: nur Fehler ausgeben |
| `-d` | Debug-Informationen nach `Debug.lst` schreiben |
| `-x` | Eine Listing-Datei assemblieren: die Hex-Spalten vor jeder Zeile überspringen |
| `-h` | Übersicht der Optionen ausgeben |

Mit `-D` definierte Symbole können mit `IF`, `IFDEF` und `IFNDEF`
abgefragt werden. Wird dasselbe Symbol im Quelltext mit einem anderen
Wert oder als Label noch einmal definiert, ist das ein Fehler.

### Dateien

| Datei | Inhalt |
|-------|--------|
| `<name>.as9` | Quelltext |
| `<name>.ls9` | Listing mit Querverweisen (wird immer geschrieben) |
| `<name>.sym` | Symboldatei |
| `<name>.pp`  | Vorverarbeiteter Quelltext (`-p`) |
| `Debug.lst`  | Debug-Ausgabe (`-d`) |

Binärdateien werden nicht automatisch geschrieben, sondern im Quelltext
mit `STORE` angefordert (siehe [Kapitel 12](#12-ausgabedateien)).

Der Rückgabewert ist die Anzahl der Fehler (0 = Erfolg).

---

## 3. Aufbau des Quelltexts

Eine Quelltextzeile hat die Form

```
[label[:]]  [mnemonic | direktive | makroaufruf]  [operand]  [; kommentar]
```

* Bei Mnemonics, Direktiven und Registernamen spielt Groß-/Kleinschreibung
  **keine** Rolle.
* Bei Symbolen wird Groß-/Kleinschreibung unterschieden, außer mit `-i`
  oder `CASE -`.
* Ein Label darf eingerückt sein. Jedes Wort am Zeilenanfang, das kein
  Mnemonic, keine Direktive, kein Schlüsselwort und kein Makroname ist,
  wird als Label genommen. Ein falsch geschriebenes Mnemonic wird deshalb
  ohne Fehlermeldung zum Label; prüfe die Symboltabelle auf unbenutzte
  Symbole.
* Ein Doppelpunkt nach einem Label ist erlaubt und wird ignoriert.
* Eine Zeile darf höchstens 255 Zeichen lang sein.

### Kommentare

| Form | Bedeutung |
|------|-----------|
| `; text` | Kommentar bis zum Zeilenende (auch nach einem Befehl) |
| `*` am Zeilenanfang | Kommentarzeile, außer bei `* = adresse` |
| Leerzeile | wird ignoriert |
| Zeile beginnt mit `/*` oder `\*` | Kommentar; schaltet außerdem auf den Stil von `-m` um |

Ohne `-m` ist Text nach einem vollständigen Befehl ohne vorangestelltes
`;` ein Fehler (`NOP 5`, `LDA $1234 junk`). Mit `-m` ist alles nach dem
ersten Leerzeichen im Operandenfeld Kommentar.

---

## 4. Symbole und Labels

Symbolnamen beginnen mit einem Buchstaben, `_` oder `.` und dürfen
Buchstaben, Ziffern, `_`, `.` und `$` enthalten. Namen, die mit `.`
beginnen, sind lokal zum aktuellen Modul (siehe
[Kapitel 11](#11-module-und-lokale-symbole)).

Mnemonics und Direktiven können nicht als Symbole verwendet werden.

### Symbole definieren

```
LABEL     LDX  #0          ; LABEL = aktueller Programmzähler
TXTPTR    =    $21B8       ; Konstante
OLDPTR    EQU  $21BA       ; Konstante (wie =)
COUNT     SET  5           ; Variable, kann mit SET neu zugewiesen werden
RED       ENUM 1           ; Aufzählung: RED = 1
GREEN     ENUM             ; letzter ENUM-Wert + 1 = 2
BLUE      ENUM             ; 3
CURSOR    BSS  2           ; CURSOR = BSS-Zähler, BSS-Zähler += 2
```

Regeln:

* Ein Label, eine Konstante (`EQU`/`=`/`ENUM`) oder ein `BSS`-Label darf
  nur einmal definiert werden. Eine zweite Definition an anderer Stelle
  wird zusammen mit der Stelle der ersten Definition gemeldet.
* `EQU`/`=` dürfen eine Definition nur mit **demselben** Wert wiederholen.
* `SET`-Variablen dürfen jederzeit neu zugewiesen werden.
* Ein undefiniertes Symbol in einem Ausdruck wird am Ende der
  Assemblierung gemeldet; im Code ist es ein Fehler.

### Sondersymbole

| Symbol | Wert |
|--------|------|
| `*` | aktueller Programmzähler (an Operandenposition) |
| `&` | aktueller BSS-Zähler |
| `?name` | Anzahl Bytes der Datenzeile mit dem Label `name` (siehe unten) |

`?name` liefert die Länge der `BYTE`-, `FCB`-, `FCC`-, `WORD`-, `FDB`-
oder `LONG`-Zeile an der Adresse von `name`:

```
Msg     BYTE "Hello"
        LDB  #?Msg          ; LDB #5
```

---

## 5. Konstanten und Ausdrücke

Alle Werte sind vorzeichenbehaftete 32-Bit-Ganzzahlen. Der Wert
`$5A5AA5A5` ist intern als Kennung für undefinierte Werte reserviert und
kann nicht verwendet werden.

### Konstanten

| Form | Beispiel | Wert |
|------|----------|------|
| dezimal | `100` | 100 |
| hexadezimal | `$FFD2` | $FFD2 |
| hexadezimal (Suffix) | `0FFh`, `1234H` | höchstens 5 Ziffern, muss mit einer Ziffer beginnen |
| binär | `%10100101` | $A5 |
| binär mit Leerzeichen | `%1010 0101` | $A5 |
| binär mit Symbolen | `%..*.*.**` | `*` = 1, `.` = 0 |
| Zeichen | `'A'` | $41 |
| Zeichen mit Escape | `'\n'` `'\r'` `'\a'` `'\e'` `'\0'` | 10, 13, 7, 27, 0 |
| Apostroph / Backslash | `'\''` / `'\'` | $27 / $5C |
| mehrere Zeichen | `"AB"` | $4142 (1 bis 4 Zeichen, Big Endian) |

Eine Dezimalkonstante mit nachfolgendem `A`-`F` ist ein Fehler („leading
$ for hex missing“).

### Operatoren

Unäre Operatoren:

| Operator | Bedeutung |
|----------|-----------|
| `+` `-` | Vorzeichen |
| `!` | logisches NICHT (Ergebnis 0 oder 1) |
| `~` | bitweises NICHT |
| `<` | Direct- bzw. 8-Bit-Modus erzwingen (wählt **nicht** das Low-Byte aus) |
| `>` | Extended- bzw. 16-Bit-Modus erzwingen |
| `( )` `[ ]` | Klammern (beide Arten) |

Binäre Operatoren, höchste Priorität zuerst (gleiche Priorität: von
links nach rechts):

| Priorität | Operatoren | Bedeutung |
|-----------|------------|-----------|
| 11 | `*` `/` | Multiplikation, ganzzahlige Division |
| 10 | `+` `-` | Addition, Subtraktion |
| 9  | `<<` `>>` | Linksschieben, arithmetisches Rechtsschieben |
| 8  | `<` `<=` `>` `>=` | Vergleich (Ergebnis 0 oder 1) |
| 7  | `==` `!=` | gleich, ungleich |
| 6  | `&` | bitweises UND |
| 5  | `^` | bitweises XOR |
| 4  | `\|` | bitweises ODER |
| 3  | `&&` | logisches UND |
| 2  | `\|\|` | logisches ODER |

Die Prioritäten entsprechen denen von C. Überlauf des 32-Bit-Bereichs,
Division durch null und Schieben um mehr als 31 Bit sind Fehler.

```
        LDX  #Table+2*[LEN-1]
        LDA  #(MASK & $F0) >> 4
        FDB  *-Start
```

---

## 6. Befehle und Adressierungsarten

### CPU-Auswahl

```
CPU = 6309          ; vollständiger 6309-Befehlssatz (Voreinstellung)
CPU = 6809          ; nur 6809: 6309-Befehle und -Register sind Fehler
```

Die CPU wird zu Beginn jedes Durchlaufs auf 6309 zurückgesetzt.

### Adressierungsarten

| Art | Syntax | Beispiel |
|-----|--------|----------|
| Inherent | – | `NOP`, `CLRA`, `PSHSW` |
| Immediate | `#wert` | `LDA #$20`, `LDD #1000`, `LDQ #$12345678` |
| Direct | `adresse` (High-Byte = DP) | `LDA $12` |
| Extended | `adresse` | `LDA $1234` |
| Direct erzwungen | `<adresse` | `LDA <$12` |
| Extended erzwungen | `>adresse` | `LDA >$0012` |
| Extended indirekt | `[adresse]` | `JMP [$FFFE]` |
| Relativ | `label` | `BNE Loop` |
| Register | `reg,reg` | `TFR A,B` |
| Registerliste | `reg,reg,...` | `PSHS A,B,X` |
| Indiziert | siehe unten | `LDA 5,X` |

**Direct oder Extended** wird automatisch gewählt: Eine Adresse, deren
High-Byte dem mit `SETDP` gesetzten Wert entspricht (Voreinstellung 0),
wird direkt adressiert, sonst extended. Eine Vorwärtsreferenz wird
extended adressiert, bis ihr Wert bekannt ist.

**Immediate-Werte** werden geprüft: 8-Bit-Register erlauben -128..255,
16-Bit-Register -32768..65535. `LDQ #` nimmt einen 32-Bit-Wert oder eine
Gleitkommakonstante (`LDQ #1.5`, siehe `REAL`).

### Indizierte Adressierung

Indexregister: `X`, `Y`, `U`, `S`; auf der 6309 auch `W` (mit
Einschränkungen, siehe Tabelle).

| Syntax | Bedeutung |
|--------|-----------|
| `,R` | ohne Offset |
| `n,R` | konstanter Offset (5, 8 oder 16 Bit, automatisch gewählt) |
| `<n,R` | 8-Bit-Offset erzwungen (-128..127, sonst Fehler) |
| `>n,R` | 16-Bit-Offset erzwungen |
| `A,R` `B,R` `D,R` | Akkumulator als Offset |
| `E,R` `F,R` `W,R` | Akkumulator als Offset (6309) |
| `,R+` `,R++` | nachträglich um 1 / 2 erhöhen |
| `,-R` `,--R` | vorher um 1 / 2 verringern |
| `label,PCR` | relativ zum Programmzähler: Offset = label - nächster Befehl |
| `n,PC` | Programmzähler mit konstantem Offset `n` |
| `[...]` | indirekte Form aller obigen Arten außer `,R+`, `,-R` und 5-Bit-Offsets |
| `,W` `,W++` `,--W` `n,W` | W-Register (6309), auch indirekt `[,W]` usw. |

Ein Offset von 0 (`0,X`) wird als `,X` assembliert. Das W-Register hat
nur einen 16-Bit-Offset; `<n,W` ist ein Fehler.

### Register zu Register

`TFR`, `EXG` und die 6309-Befehle `ADDR`, `ADCR`, `SUBR`, `SBCR`,
`ANDR`, `ORR`, `EORR`, `CMPR` erhalten zwei Register:

```
D X Y U S PC W V A B CC DP 0 E F      (6309)
D X Y U S PC     A B CC DP            (6809)
```

Die Registergrößen werden nicht geprüft.

### Push und Pull

```
PSHS A,B,X,PC
PULU CC,DP,Y
PSHS ALL              ; alle Register ($FF)
```

`PSHS`/`PULS` können `S` nicht sichern, `PSHU`/`PULU` nicht `U`.

### 6309-Blockübertragung

```
TFM X+,Y+             ; Quelle und Ziel erhöhen
TFM X-,Y-             ; beide verringern
TFM X+,Y              ; nur Quelle erhöhen
TFM X,Y+              ; nur Ziel erhöhen
```

Register: `D`, `X`, `Y`, `U`, `S`.

### 6309 Speicher mit Immediate-Wert (AIM, OIM, EIM, TIM)

```
AIM #$FE,$40          ; direct
AIM #$FE,$1234        ; extended
AIM #$FE,5,X          ; indiziert
AIM #$FE,[,X]         ; indirekt
```

### 6309-Bitbefehle

`BAND`, `BIAND`, `BOR`, `BIOR`, `BEOR`, `BIEOR`, `LDBT`, `STBT`:

```
BAND  A.1,$40.3       ; Register A Bit 1, Speicherbit 3 an Direct-Adresse $40
LDBT  CC.0,$12.7
```

Register: `CC`, `A`, `B`. Die Adresse muss im Bereich 0..255 liegen
(Offset in der Direct Page).

### Zusatzbefehle

Diese Pseudobefehle gibt es mit `CPU = 6309`:

| Befehl | erzeugter Code |
|--------|----------------|
| `CLRX` | `LDX #0` |
| `CLRY` | `SUBR Y,Y` |
| `DECX` / `INCX` | `LEAX -1,X` / `LEAX 1,X` |
| `DECY` / `INCY` | `LEAY -1,Y` / `LEAY 1,Y` |

---

## 7. Direktiven (Pseudo-Ops)

### Programmzähler und Speicheraufteilung

| Direktive | Bedeutung |
|-----------|-----------|
| `ORG adr` | Programmzähler setzen (0..$10000) |
| `* = adr` | wie `ORG` |
| `& = adr` | BSS-Zähler setzen |
| `label BSS n` | `label` = BSS-Zähler, BSS-Zähler += n (1..32767) |
| `BSS n` | BSS-Zähler += n |
| `RMB n` / `DS n` | n Bytes reservieren (Programmzähler += n, Speicher bleibt unverändert) |
| `ALIGN n` | Programmzähler auf ein Vielfaches von n ausrichten, mit `NOP` ($12) füllen |
| `ALIGN n (w)` / `ALIGN * n (w)` | ebenso, mit Byte w füllen |
| `ALIGN & n` / `ALIGN & n (w)` | BSS-Zähler ausrichten, mit dem Wert von `-l` bzw. w füllen |
| `SECT LOC=adr` | Programmzähler setzen |
| `SETDP n` | Direct-Page-Register = n annehmen (0..255 oder eine Adresse $nn00) |
| `CPU = 6809` / `CPU = 6309` | Befehlssatz wählen |

Der Programmzähler muss vor dem ersten Code- oder Datenbyte gesetzt sein.

### Dateien

| Direktive | Bedeutung |
|-----------|-----------|
| `INCLUDE "datei"` | eine weitere Quelltextdatei assemblieren (bis zu 99 Ebenen) |
| `LOAD adr,"datei"` | Binärdatei nach `adr` laden (Programmzähler bleibt) |
| `LOAD "datei"` | Binärdatei an den Programmzähler laden und ihn weiterzählen |
| `STORE start,länge,"datei"[,format[,start]]` | Binärdatei schreiben, siehe [Kapitel 12](#12-ausgabedateien) |

### Listing und Steuerung

| Direktive | Bedeutung |
|-----------|-----------|
| `LIST +` / `LIST -` | Listing ein- / ausschalten |
| `MACLIST ON` / `MACLIST OFF` | expandierte Makrozeilen im Listing zeigen / nicht zeigen (auch `1` / `0`) |
| `CASE +` / `CASE -` | Groß-/Kleinschreibung bei Symbolen unterscheiden / nicht unterscheiden |
| `SIZE` | Größe des aktuellen Moduls im Listing ausgeben |
| `END` | Assemblierung beenden (in einer Include-Datei: Ende dieser Datei) |
| `#error text` | mit Fehlermeldung abbrechen (normalerweise innerhalb von `IF`) |
| `TTL`, `EXTERN`, `INTERN` | werden ignoriert |
| `FORMLN n` | Zeilen pro Seite (ohne Funktion) |

---

## 8. Datendefinition

| Direktive | Bedeutung |
|-----------|-----------|
| `BYTE`  | Bytes, Strings; Werte außerhalb -128..255 werden als 16-Bit-Wort gespeichert |
| `FCB` / `DB` | Bytes und Strings; jeder Wert ist ein Byte (-128..255) |
| `FCC` | Strings (gleiche Regeln wie `BYTE`) |
| `BHEX` | wie `BYTE`, aber Zahlen sind hexadezimal ohne `$` |
| `WORD` / `FDB` / `DW` | 16-Bit-Worte, Big Endian (-32768..65535) |
| `LONG` | 32-Bit-Werte, Big Endian |
| `REAL` | 32-Bit-Gleitkommawerte |
| `FILL n (w)` | n Bytes mit dem Wert w |
| `BITS` | ein Byte aus 8 Bitsymbolen |
| `C5TO3` | gepackte 5-Bit-Zeichen |
| `CMAP` | ein Byte aus 8 Bitsymbolen (Zeichensätze) |

```
Text    BYTE  "Hello",13,0           ; 48 65 6C 6C 6F 0D 00
        BYTE  1,$1234                ; 01 12 34
        FCB   $20,"Example",0        ; 20 45 78 61 6D 70 6C 65 00
        FCB   300                    ; Fehler: außerhalb des Bereichs
Table   WORD  Start,Stop,$0200       ; 16-Bit-Adressen
        LONG  1000000                ; 00 0F 42 40
        FILL  16 ($FF)               ; 16 x $FF
        FILL  $A000-* (0)            ; auffüllen bis $9FFF
        BHEX  20,FF,0D               ; 20 FF 0D
```

### Strings

Strings stehen in `"`. Escape-Sequenzen:

| Escape | Wert | Escape | Wert |
|--------|------|--------|------|
| `\r` | 13 (CR) | `\e` | 27 (ESC) |
| `\n` | 10 (LF) | `\0` | 0 |
| `\t` | 9 (TAB) | `\b` | 29 (Fettschrift) |
| `\f` | 12 (FF) | `\s` | 28 (Standardfarbe) |
| `\a` | 7 (BEL) | `\x` | x für jedes andere Zeichen |

Ein `^` nach dem schließenden Anführungszeichen setzt Bit 7 des letzten
Zeichens: `FCC "AB"^` ergibt `41 C2`.

Besondere Einträge in `BYTE`, `FCB` und `FCC`:

| Eintrag | Bedeutung |
|---------|-----------|
| `$DATE` | Datum der Assemblierung als Text, z. B. `17-Nov-2026` |
| `#"ABC"` | Hashed ASCII: 3 Zeichen `@`..`Z` gepackt in ein 16-Bit-Wort (Basis 27) |

### REAL

`REAL` speichert einen 4 Byte langen Gleitkommawert: ein Exponentbyte
(Exzess $80) und eine 24-Bit-Mantisse mit dem Vorzeichen in Bit 7 des
ersten Mantissenbytes (Microsoft-BASIC-Format).

```
        REAL  1.0                    ; 81 00 00 00
        REAL  -0.5                   ; 80 80 00 00
        REAL  cosd(60)               ; Kosinus von 60 Grad
        REAL  $81490FDB              ; Hex-Bytes
```

### BITS, CMAP

Acht Symbole, `*` für 1 und `.` für 0, durch Leerzeichen getrennt:

```
        BITS  * . * . * . * .        ; $AA
```

### C5TO3

Packt 4 Zeichen in 20 Bit und einen Typbuchstaben in die Bits 20..22,
zusammen 3 Bytes. Erlaubte Zeichen: `?`, Leerzeichen, `A`..`Z`, `2`, `3`.
Typbuchstaben: `C`=1, `D`=2, `R`=3, `W`=4, andere 0.

```
        C5TO3 "ABCDC"                ; 12 90 62
```

---

## 9. Bedingte Assemblierung

```
IF ausdruck          ; wahr, wenn der Ausdruck definiert und nicht 0 ist
IFDEF symbol         ; wahr, wenn das Symbol definiert ist
IFNDEF symbol        ; wahr, wenn das Symbol nicht definiert ist
ELSE
ENDIF
```

* Bedingungen können bis zu 9 Ebenen tief verschachtelt werden.
* Ein undefiniertes Symbol in `IF` gilt als falsch.
* Die alte Schreibweise `#if`, `#ifdef`, `#else`, `#endif` ist erlaubt.
* `ELSE` ohne `IF` und ein fehlendes `ENDIF` sind Fehler.

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

Symbole für die bedingte Assemblierung lassen sich auf der Kommandozeile
definieren: `bs9 -DMO5 os` oder `bs9 -DVERSION=3 os`.

---

## 10. Makros

### Definition

```
MACRO Name(Arg1, Arg2, ...)
        ...
ENDM

MACRO Name                     ; Makro ohne Argumente
        ...
ENDM
```

### Aufruf

```
        Name(wert1, wert2)
Label   Name(wert1, wert2)     ; Label vor einem Makroaufruf
```

Beispiel:

```
MACRO PrintString(Message)
        LDX   #Message
        LDB   #?Message
        JSR   PrintN
ENDM

OK      BYTE "\nOK\n"
        PrintString(OK)
```

wird expandiert zu

```
        LDX   #OK
        LDB   #4
        JSR   PrintN
```

Regeln:

* Bis zu 10 Argumente. Die Anzahl der Argumente beim Aufruf muss der
  Definition entsprechen.
* Argumente dürfen ausgeglichene Klammern, Strings und Zeichen in
  Anführungszeichen sowie darin enthaltene Kommas enthalten.
* Ein Argumentname wird ersetzt, wo er als ganzes Wort steht (nicht als
  Teil eines längeren Namens aus Buchstaben, Ziffern, `_`, `.`, `$`).
  Ersetzt wird auch in Strings und Kommentaren.
* Makros dürfen andere Makros aufrufen (bis zu 9 Ebenen), aber nicht sich
  selbst. Eine Makrodefinition darf keine weitere Definition enthalten.
* Makronamen dürfen keine Mnemonics, Direktiven oder Schlüsselwörter sein.
* Das Zeichen `` ` `` darf in einer Makrodefinition nicht vorkommen.
* Eine expandierte Zeile darf höchstens 255 Zeichen lang sein.
* Ein Label im Makrorumpf wird bei jedem Aufruf definiert: Ein Makro mit
  Label kann nur einmal aufgerufen werden. Für Makros, die mehrfach
  aufgerufen werden, lokale Labels in verschiedenen Modulen verwenden.

`MACLIST OFF` unterdrückt die expandierten Zeilen im Listing.

---

## 11. Module und lokale Symbole

```
MODULE Delay              ; oder SUBROUTINE Delay
.loop   LEAX -1,X
        BNE  .loop
        RTS
ENDMOD                    ; oder ENDSUB
```

`MODULE name` definiert das Label `name` an der aktuellen Adresse und
öffnet einen Gültigkeitsbereich. Symbole, die mit `.` beginnen, erhalten
darin den Modulnamen als Präfix: `.loop` wird zu `Delay.loop`. Derselbe
lokale Name kann in anderen Modulen wieder verwendet werden. Von außen
wird ein lokales Symbol mit seinem vollen Namen angesprochen
(`Delay.loop`).

Lokale Symbole können nur innerhalb eines Moduls verwendet werden.

`ENDMOD` gibt die Größe des Moduls im Listing aus.

---

## 12. Ausgabedateien

```
STORE start,länge,"datei"[,format[,start]]
```

| Format | Ausgabe |
|--------|---------|
| `BIN` (Voreinstellung) | Reine Binärdatei. Ist der letzte Parameter angegeben, steht vor den Daten die Ladeadresse (`start`) als 2 Byte Big Endian |
| `S19` / `SREC` | Motorola-S-Records: S0-Kopf, S1-Datensätze (32 Bytes), S5-Satzanzahl, S9-Startadresse, falls angegeben. Zeilenende CR LF. Hat der Dateiname keine Endung, wird `.S19` angehängt |
| `SPARROW` | 8 Byte Kopf (`S`, 3 Byte Startadresse, 2 Byte Länge, 2 Byte Prüfsumme), danach die Daten |

```
STORE $E000,$2000,"basic.rom"            ; 8-KB-Binärdatei
STORE Start,End-Start,"prog.bin",bin,1   ; Binärdatei mit Ladeadresse
STORE Start,End-Start,"prog.s19",s19,Main ; S-Records, Startadresse Main
```

Bis zu 20 `STORE`-Befehle sind möglich. Die Dateien werden nach
erfolgreicher Assemblierung geschrieben; der Bereich muss innerhalb von
64 KB liegen.

### Symboldatei

`<name>.sym` enthält die Anzahl der Symbole, eine Prüfsummenzeile
(16-Bit-Summe der Bytes $8000..$FFFF) und eine Zeile pro Symbol:

```
    9
C 0000 Checksum
  0001 FOO
B 0300 X
M 2002 Mod
  2007 Later
```

Die erste Spalte ist der Typ: `B` = BSS-Label, `M` = Modul,
`D` = mit `-D` definiert, Leerzeichen = sonstige.

---

## 13. Aufbau des Listings

```
  27 9ff6   b6    fe30         LDA     IO_SDCARD
  28 9ff9 1034 8e              ANDR    A,E       ; test IO_INCD#
  29 9ffc   27      06         BEQ     +
  ^  ^    ^    ^  ^            ^       ^         ^
  |  |    |    |  |            |       |         \- Kommentar
  |  |    |    |  |            |       \----------- Operand
  |  |    |    |  |            \------------------- Mnemonic
  |  |    |    |  \----- Adresse/Wert des Operanden (Wort oder Byte)
  |  |    |    \-------- Postbyte
  |  |    \------------- Opcode (Byte oder Wort)
  |  \------------------ Programmadresse
  \--------------------- Zeilennummer (Option -n)
```

Andere Zeilen zeigen den Wert der Definition (Konstanten, `BSS`,
`SETDP`, `CPU`), `TRUE`/`FALSE` bei Bedingungen und `SKIP` bei Zeilen in
falschen Bedingungsblöcken.

Am Ende enthält das Listing die Symboltabelle dreimal:

1. alle Symbole nach Wert sortiert,
2. Symbole mit Werten $0000..$00FF nach Anzahl der Verweise sortiert,
3. Symbole mit Werten $0000..$4000 nach Anzahl der Verweise sortiert.

Jeder Eintrag zeigt Name, Wert, `L` bei Konstanten (EQU, SET, ENUM) und
die Zeilennummern der Definition (mit `D` markiert) und aller Verweise.

Mit `-x` kann eine Listing-Datei erneut assembliert werden: Die
Hex-Spalten vor dem Quelltext werden übersprungen.

---

## 14. Optimierung und Durchläufe

BS9 assembliert in bis zu 10 Durchläufen, bis kein Label mehr seinen
Wert ändert, und erzeugt dann in einem letzten Durchlauf den Code.

In der Voreinstellung werden Verzweigungen und Sprünge optimiert:

* Lange Verzweigungen (`LBRA`, `LBEQ`, ...) werden kurz, wenn das Ziel
  in Reichweite liegt.
* Kurze Verzweigungen außerhalb der Reichweite werden lang
  (`BRA` → `LBRA`, `BEQ` → `LBEQ`, ...).
* `JMP` und `JSR` mit einem Ziel in Reichweite einer kurzen Verzweigung
  werden zu `BRA` und `BSR`. `JMP >label` und `JSR >label` werden nie
  geändert.

Mit `-o` bleiben lange Verzweigungen lang und `JMP`/`JSR` unverändert.
Kurze Verzweigungen außerhalb der Reichweite werden trotzdem lang.

---

## 15. Fehler

Die meisten Fehler beenden die Assemblierung sofort. Die Meldung zeigt
Datei, Zeilennummer, Quelltextzeile und eine Markierung an der
Fehlerstelle:

```
*** Error in file test.as9 line 12:
Junk after operand
 lda $1234 junk
```

Undefinierte Symbole werden am Ende aufgelistet und als Fehler gezählt.
Der Rückgabewert von `bs9` ist die Anzahl der Fehler.

Typische Fehler:

| Meldung | Ursache |
|---------|---------|
| `Label [X] already defined in file F line N` | zweite Definition eines Labels oder einer Konstante |
| `Multiple assignments for label [X]` | `EQU` mit anderem Wert |
| `Phase error label [X]` | Wert eines Labels weicht im letzten Durchlauf ab |
| `Use of an undefined label` | undefiniertes Symbol im Code |
| `Immediate value out of range` | Wert passt nicht ins Register |
| `8 bit offset out of range` | `<n,R` mit n außerhalb -128..127 |
| `FCB value ... out of range` | `FCB`-Wert außerhalb -128..255 (stattdessen `BYTE` verwenden) |
| `Junk after operand` | Text nach einem vollständigen Operanden ohne `;` |
| `6309 instruction not allowed with CPU = 6809` | 6309-Code im 6809-Modus |
| `Undefined program counter (missing ORG)` | Code vor dem ersten `ORG` |

---

## 16. Grenzwerte

| Element | Grenze |
|---------|--------|
| Zeilenlänge | 255 Zeichen (auch für expandierte Makrozeilen) |
| Symbole | 8000 |
| Makros | 200 |
| Makroargumente | 10 |
| Makroverschachtelung | 9 Ebenen |
| Verschachtelung von Bedingungen | 9 Ebenen |
| Verschachtelung von Include-Dateien | 99 Ebenen |
| `STORE`-Befehle | 20 |
| Durchläufe | 10 |
| Adressraum | 64 KB |

---

## 17. Unterschiede zu anderen Assemblern

Im Vergleich mit lwasm und dem Motorola-Assembler:

* `BYTE` speichert Werte außerhalb des Bytebereichs als 16-Bit-Worte.
  `FCB` speichert einzelne Bytes, meldet aber Werte außerhalb -128..255
  als Fehler (lwasm schneidet sie ohne Meldung ab).
* Strings sind in `FCB` erlaubt; `FCB "AB"` speichert zwei Bytes.
* `FCC` unterstützt Escape-Sequenzen und zusätzliche Werte, aber keine
  beliebigen Begrenzer (`FCC /text/` wird nicht unterstützt).
* Kurze Verzweigungen außerhalb der Reichweite werden automatisch
  verlängert.
* Bitbefehle verwenden die Syntax `BAND A.1,$40.3` (lwasm:
  `BAND A,3,1,$40`) und erwarten eine Adresse 0..255.
* Zusätzliche Operatoren `<<`, `>>`, `!`, `==`, `!=`, `<`, `<=`, `>`,
  `>=`, `&&`, `||`.
* Zusätzliche Funktionen: Makros mit Argumenten, `MODULE`-Bereiche,
  `ENUM`, `BSS`, `?name`, `PSHS ALL`, `BHEX`, `C5TO3`, `REAL`, `$DATE`,
  Hashed ASCII.
