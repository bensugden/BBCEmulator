; -----------------------------------------------------------------------------
; BASIC II (c) Acorn
; reverse engineered by Eelco Huininga using phpDisassembler v1.02 (65ce02)
; DOS Filename   : basic2.rom
; Acorn filename : BASIC2
; Load address   : 00008000
; Exec address   : 00008023
; Length         : 00004000
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; BASIC language work space in zero page
; -----------------------------------------------------------------------------
lomem                   = &00                               ; &00-&01: LOMEM (Pointer to start of BASIC variables)
vartop                  = &02                               ; &02-&03: VARTOP (Pointer to end of of BASIC variables)
                        = &04                               ; &04-&05: (BASIC stack pointer)
himem                   = &06                               ; &06-&07: HIMEM (Pointer to start of memory mapped screen area)
erl                     = &08                               ; &08-&09: ERL (Line number of last error)
                        = &0A                               ; &0A    : PtrA (Text pointer A offset)
ptra                    = &0B                               ; &0B    : PtrA (Text pointer A line number)
rnd                     = &0D                               ; &0D-&11: RND store
top                     = &12                               ; &12    : TOP (Pointer to end of BASIC program)
                        = &14                               ; &14    : (Print field width)
                        = &15                               ; &15    : Print flag; bit 7: Print variables as decimal/hex
                        = &16                               ; &16    : (Pointer to address of ON ERROR statement)
page                    = &18                               ; &18    : PAGE (MSB, LSB is always &00)
ptrb                    = &19                               ; &19-&1A: PtrB (Text pointer B line number)
ptrb_offset             = &1B                               ; &1B    : PtrB (Text pointer B offset)

count                   = &1E                               ; &1E    : COUNT
listo                   = &1F                               ; &1F    : LISTO option; bit 0: Space after line number
                                                            ;                        bit 1: Indent FORs
                                                            ;                        bit 2: Indent REPEATs
width                   = &23                               ; &23    : WIDTH variable

&001C/D  Pointer to next DATA item
&0020    Trace flag
&0021/2  TRACE maximum line number
&0024    Number of REPEATs left
&0025    Number of GOSUBs left
&0026    15 * number of FORs left
&0027    Variable type of current expression
&0028    OPT value for assembler
    Bit 0 - Produce listing
        1 - Give errors
        2 - Relocate
&0029-B  Assembled code buffer
&002A-D  Integer accumulator
&002E    Floating Point Accumulator #1 sign/point
&002F    Floating Point Accumulator #1 over/underflow
&0030    Floating Point Accumulator #1 exponent
&0031-4  Floating Point Accumulator #1 mantissa
&0035    Floating Point Accumulator #1 rounding byte
&0036    String Accumulator length (string accumulator is at &600)
&0037/8  Renumber pointer
&0039/A  Renumber/Delete line number
&0039    PROC/FN/variable name length
&0039-40 Integer store for multiply and divide
&003B    Floating Point Accumulator #2 sign
&003C    Floating Point Accumulator #2 over/underflow byte
&003D    Floating Point Accumulator #2 exponent
&003E-41 Floating Point Accumulator #2 mantissa
&0042    Floating Point Accumulator #2 rounding byte
&0043-47 Floating point multiple and divide workspace
&0043    Temporary store
&0044-7  Hex/numeric workspace
&0048    Series counter
&0049    String/number conversion flag
&004A    Exponent for string/number conversion
&004B/C  Current variable pointer
&004D/E  Pointer for series evaluator



; -----------------------------------------------------------------------------
; OS call addresses
; -----------------------------------------------------------------------------
brkv			= &0202                             ; BRKV BRK vector
wrchv			= &020E                             ; WRCHV Write character

osfind			= &FFCE                             ; OSFIND Open or close a file
osbput			= &FFD4                             ; OSBPUT Write byte to file
osbget			= &FFD7                             ; OSBGET Get byte from file
osargs			= &FFDA                             ; OSARGS Load or save file parameters
osfile			= &FFDD                             ; OSFILE Load or save a complete file
osrdch			= &FFE0                             ; OSRDCH Read a character
osasci			= &FFE3                             ; OSASCI Write a character, with CR converted to LF
osnewl			= &FFE7                             ; OSNEWL Write a new-line routine
oswrch			= &FFEE                             ; OSWRCH Write a character
osword			= &FFF1                             ; OSWORD Various OS routines
osbyte			= &FFF4                             ; OSBYTE Various OS routines
oscli			= &FFF7                             ; OSCLI Operating System Command Line Interface

; -----------------------------------------------------------------------------
; Start of code
; -----------------------------------------------------------------------------

8000  C9 01              ..      CMP #&01
8002  F0 1F              ..      BEQ language
8004  60                 `       RTS
8005  EA                 .       NOP
8006  60                 .       RTS                        ; ROM type; bit 7   = 0    : Doesn't have a service entry
                                                            ;           bit 6   = 1    : Has a language entry
                                                            ;           bit 5   = 1    : Is relocatable to the second processor
                                                            ;           bit 4   = 0    : No Electron firm key expansions availabe
                                                            ;           bit 3-0 = 0000 : 6502 code (BASIC)
8007  1A                 .       EQUB copyright MOD 256)-1  ; Copyright offset
8008  02                 .       EQUB &02                   ; Version 2

.title
8009  42 41 53 49 43     BASIC   EQUS "BASIC"
800E  00                 .       EQUB &00                   ; Title termination byte

.copyright
800F  28 43 29 31 39 38  (C)198  EQUS "(C)1982 Acorn"
      32 20 41 63 6F 72
      6E
801C  0A                 .       EQUB &0A
801D  0D                 .       EQUB &0D
801E  00                 .       EQUB &00                   ; Copyright termination byte

801F  00 80 00 00        ....    EQUD &00008000             ; Second processor relocation address

; -----------------------------------------------------------------------------
; Language entry
; -----------------------------------------------------------------------------
.language
8023  A9 84              ..      LDA #&84                   ; OSBYTE &84: Read bottom of display address (HIMEM)
8025  20 F4 FF            ..     JSR osbyte
8028  86 06              ..      STX &06                    ; Set MSB of HIMEM
802A  84 07              ..      STY &07                    ; Set LSB of HIMEM
802C  A9 83              ..      LDA #&83                   ; OSBYTE &83: Read top of operating system RAM address (OSHWM)
802E  20 F4 FF            ..     JSR osbyte
8031  84 18              ..      STY page                   ; Set PAGE
8033  A2 00              ..      LDX #&00
8035  86 1F              ..      STX listo                  ; Reset LISTO option
8037  8E 02 04           ...     STX &0402                  ; bit 16-32 of @% is 0
803A  8E 03 04           ...     STX &0403
803D  CA                 .       DEX
803E  86 23              .#      STX width                  ; Set WIDTH to 255
8040  A2 0A              ..      LDX #&0A
8042  8E 00 04           ...     STX &0400
8045  CA                 .       DEX
8046  8E 01 04           ...     STX &0401                  ; @% = &0000090A
8049  A9 01              ..      LDA #&01
804B  25 11              %.      AND rnd+4
804D  05 0D              ..      ORA rnd
804F  05 0E              ..      ORA rnd+1
8051  05 0F              ..      ORA rnd+2
8053  05 10              ..      ORA rnd+3
8055  D0 0C              ..      BNE &8063                  ; Jump if RND was already initialized
8057  A9 41              .A      LDA #&41
8059  85 0D              ..      STA rnd
805B  A9 52              .R      LDA #&52
805D  85 0E              ..      STA rnd+1
805F  A9 57              .W      LDA #&57
.
8061  85 0F              ..      STA rnd+2
.
8063  A9 02              ..      LDA #_brkv MOD 256
8065  8D 02 02           ...     STA brkv
8068  A9 B4              ..      LDA #_brkv DIV 256
806A  8D 03 02           ...     STA brkv+1
806D  58                 X       CLI
806E  4C DD 8A           L..     JMP &8ADD

; -----------------------------------------------------------------------------
; Keyword table
; -----------------------------------------------------------------------------

8071  41 4E 44           AND     EQUS "AND"
8074  80                 .       EQUB &80                   ; Token
8075  00                 .       EQUB &00

8076  41 42 53           ABS     EQUS "ABS"
8079  94                 .       EQUB &94                   ; Token
807A  00                 .       EQUB &00

807B  41 43 53           ACS     EQUS "ACS"
807E  95                 .       EQUB &95                   ; Token
807F  00                 .       EQUB &00

8080  41 44              AD      EOR (&44,X)
8082  56 41              VA      LSR &41,X
8084  4C 96 00           L..     JMP &0096
8087  41 53              AS      EOR (&53,X)
8089  43                 C       ...
808A  97 00              ..      SMB1 &00
808C  41 53              AS      EOR (&53,X)
808E  4E 98 00           N..     LSR &0098
8091  41 54              AT      EOR (&54,X)
8093  4E 99 00           N..     LSR &0099
8096  41 55              AU      EOR (&55,X)
8098  54                 T       ...
8099  4F C6              O.      BBR4 &8061
809B  10 42              .B      BPL &80DF
809D  47 45              GE      SMB4 &45
809F  54                 T       ...
80A0  9A                 .       TXS
80A1  01 42              .B      ORA (&42,X)
80A3  50 55              PU      BVC &80FA
80A5  54                 T       ...
80A6  D5 03              ..      CMP &03,X
80A8  43                 C       ...
80A9  4F 4C              OL      BBR4 &80F7
80AB  4F 55              OU      BBR4 &8102
80AD  52 FB              R.      EOR (&FB)
80AF  02                 .       ...
80B0  43                 C       ...
80B1  41 4C              AL      EOR (&4C,X)
80B3  4C D6 02           L..     JMP &02D6
80B6  43                 C       ...
80B7  48                 H       PHA
80B8  41 49              AI      EOR (&49,X)
80BA  4E D7 02           N..     LSR &02D7
80BD  43                 C       ...
80BE  48                 H       PHA
80BF  52 24              R$      EOR (&24)
80C1  BD 00 43           ..C     LDA &4300,X
80C4  4C 45 41           LEA     JMP &4145
80C7  52 D8              R.      EOR (&D8)
80C9  01 43              .C      ORA (&43,X)
80CB  4C 4F 53           LOS     JMP &534F
80CE  45 D9              E.      EOR &D9
80D0  03                 .       ...
80D1  43                 C       ...
80D2  4C 47 DA           LG.     JMP &DA47
80D5  01 43              .C      ORA (&43,X)
80D7  4C 53 DB           LS.     JMP &DB53
80DA  01 43              .C      ORA (&43,X)
80DC  4F 53              OS      BBR4 &8131
80DE  9B                 .       ...
80DF  00                 .       ...
80E0  43                 C       ...
80E1  4F 55              OU      BBR4 &8138
80E3  4E 54 9C           NT.     LSR &9C54
80E6  01 44              .D      ORA (&44,X)
80E8  41 54              AT      EOR (&54,X)
80EA  41 DC              A.      EOR (&DC,X)
80EC  20 44 45            DE     JSR &4544
80EF  47 9D              G.      SMB4 &9D
80F1  00                 .       ...
80F2  44                 D       ...
80F3  45 46              EF      EOR &46
80F5  DD 00 44           ..D     CMP &4400,X
80F8  45 4C              EL      EOR &4C
80FA  45 54              ET      EOR &54
80FC  45 C7              E.      EOR &C7
80FE  10 44              .D      BPL &8144
8100  49 56              IV      EOR #&56
8102  81 00              ..      STA (&00,X)
8104  44                 D       ...
8105  49 4D              IM      EOR #&4D
8107  DE 02 44           ..D     DEC &4402,X
810A  52 41              RA      EOR (&41)
810C  57 DF              W.      RMB5 &DF
810E  02                 .       ...
810F  45 4E              EN      EOR &4E
8111  44                 D       ...
8112  50 52              PR      BVC &8166
8114  4F 43              OC      BBR4 &8159
8116  E1 01              ..      SBC (&01,X)
8118  45 4E              EN      EOR &4E
811A  44                 D       ...
811B  E0 01              ..      CPX #&01
811D  45 4E              EN      EOR &4E
811F  56 45              VE      LSR &45,X
8121  4C 4F 50           LOP     JMP &504F
8124  45 E2              E.      EOR &E2
8126  02                 .       ...
8127  45 4C              EL      EOR &4C
8129  53                 S       ...
812A  45 8B              E.      EOR &8B
812C  14 45              .E      TRB &45
812E  56 41              VA      LSR &41,X
8130  4C A0 00           L..     JMP &00A0
8133  45 52              ER      EOR &52
8135  4C 9E 01           L..     JMP &019E
8138  45 52              ER      EOR &52
813A  52 4F              RO      EOR (&4F)
813C  52 85              R.      EOR (&85)
813E  04 45              .E      TSB &45
8140  4F 46              OF      BBR4 &8188
8142  C5 01              ..      CMP &01
8144  45 4F              EO      EOR &4F
8146  52 82              R.      EOR (&82)
8148  00                 .       ...
8149  45 52              ER      EOR &52
814B  52 9F              R.      EOR (&9F)
814D  01 45              .E      ORA (&45,X)
814F  58                 X       CLI
8150  50 A1              P.      BVC &80F3
8152  00                 .       ...
8153  45 58              EX      EOR &58
8155  54                 T       ...
8156  A2 01              ..      LDX #&01
8158  46 4F              FO      LSR &4F
815A  52 E3              R.      EOR (&E3)
815C  02                 .       ...
815D  46 41              FA      LSR &41
815F  4C 53 45           LSE     JMP &4553
8162  A3                 .       ...
8163  01 46              .F      ORA (&46,X)
8165  4E A4 08           N..     LSR &08A4
8168  47 4F              GO      SMB4 &4F
816A  54                 T       ...
816B  4F E5              O.      BBR4 &8152
816D  12 47              .G      ORA (&47)
816F  45 54              ET      EOR &54
8171  24 BE              $.      BIT &BE
8173  00                 .       ...
8174  47 45              GE      SMB4 &45
8176  54                 T       ...
8177  A5 00              ..      LDA &00
8179  47 4F              GO      SMB4 &4F
817B  53                 S       ...
817C  55 42              UB      EOR &42,X
817E  E4 12              ..      CPX &12
8180  47 43              GC      SMB4 &43
8182  4F 4C              OL      BBR4 &81D0
8184  E6 02              ..      INC &02
8186  48                 H       PHA
8187  49 4D              IM      EOR #&4D
8189  45 4D              EM      EOR &4D
818B  93                 .       ...
818C  43                 C       ...
818D  49 4E              IN      EOR #&4E
818F  50 55              PU      BVC &81E6
8191  54                 T       ...
8192  E8                 .       INX
8193  02                 .       ...
8194  49 46              IF      EOR #&46
8196  E7 02              ..      SMB6 &02
8198  49 4E              IN      EOR #&4E
819A  4B                 K       ...
819B  45 59              EY      EOR &59
819D  24 BF              $.      BIT &BF
819F  00                 .       ...
81A0  49 4E              IN      EOR #&4E
81A2  4B                 K       ...
81A3  45 59              EY      EOR &59
81A5  A6 00              ..      LDX &00
81A7  49 4E              IN      EOR #&4E
81A9  54                 T       ...
81AA  A8                 .       TAY
81AB  00                 .       ...
81AC  49 4E              IN      EOR #&4E
81AE  53                 S       ...
81AF  54                 T       ...
81B0  52 28              R(      EOR (&28)
81B2  A7 00              ..      SMB2 &00
81B4  4C 49 53           LIS     JMP &5349
81B7  54                 T       ...
81B8  C9 10              ..      CMP #&10
81BA  4C 49 4E           LIN     JMP &4E49
81BD  45 86              E.      EOR &86
81BF  00                 .       ...
81C0  4C 4F 41           LOA     JMP &414F
81C3  44                 D       ...
81C4  C8                 .       INY
81C5  02                 .       ...
81C6  4C 4F 4D           LOM     JMP &4D4F
81C9  45 4D              EM      EOR &4D
81CB  92 43              .C      STA (&43)
81CD  4C 4F 43           LOC     JMP &434F
81D0  41 4C              AL      EOR (&4C,X)
81D2  EA                 .       NOP
81D3  02                 .       ...
81D4  4C 45 46           LEF     JMP &4645
81D7  54                 T       ...
81D8  24 28              $(      BIT &28
81DA  C0 00              ..      CPY #&00
81DC  4C 45 4E           LEN     JMP &4E45
81DF  A9 00              ..      LDA #&00
81E1  4C 45 54           LET     JMP &5445
81E4  E9 04              ..      SBC #&04
81E6  4C 4F 47           LOG     JMP &474F
81E9  AB                 .       ...
81EA  00                 .       ...
81EB  4C 4E AA           LN.     JMP &AA4E
81EE  00                 .       ...
81EF  4D 49 44           MID     EOR &4449
81F2  24 28              $(      BIT &28
81F4  C1 00              ..      CMP (&00,X)
81F6  4D 4F 44           MOD     EOR &444F
81F9  45 EB              E.      EOR &EB
81FB  02                 .       ...
81FC  4D 4F 44           MOD     EOR &444F
81FF  83                 .       ...
8200  00                 .       ...
8201  4D 4F 56           MOV     EOR &564F
8204  45 EC              E.      EOR &EC
8206  02                 .       ...
8207  4E 45 58           NEX     LSR &5845
820A  54                 T       ...
820B  ED 02 4E           ..N     SBC &4E02
820E  45 57              EW      EOR &57
8210  CA                 .       DEX
8211  01 4E              .N      ORA (&4E,X)
8213  4F 54              OT      BBR4 &8269
8215  AC 00 4F           ..O     LDY &4F00
8218  4C 44 CB           LD.     JMP &CB44
821B  01 4F              .O      ORA (&4F,X)
821D  4E EE 02           N..     LSR &02EE
8220  4F 46              OF      BBR4 &8268
8222  46 87              F.      LSR &87
8224  00                 .       ...
8225  4F 52              OR      BBR4 &8279
8227  84 00              ..      STY &00
8229  4F 50              OP      BBR4 &827B
822B  45 4E              EN      EOR &4E
822D  49 4E              IN      EOR #&4E
822F  8E 00 4F           ..O     STX &4F00
8232  50 45              PE      BVC &8279
8234  4E 4F 55           NOU     LSR &554F
8237  54                 T       ...
8238  AE 00 4F           ..O     LDX &4F00
823B  50 45              PE      BVC &8282
823D  4E 55 50           NUP     LSR &5055
8240  AD 00 4F           ..O     LDA &4F00
8243  53                 S       ...
8244  43                 C       ...
8245  4C 49 FF           LI.     JMP &FF49
8248  02                 .       ...
8249  50 52              PR      BVC &829D
824B  49 4E              IN      EOR #&4E
824D  54                 T       ...
824E  F1 02              ..      SBC (&02),Y
8250  50 41              PA      BVC &8293
8252  47 45              GE      SMB4 &45
8254  90 43              .C      BCC &8299
8256  50 54              PT      BVC &82AC
8258  52 8F              R.      EOR (&8F)
825A  43                 C       ...
825B  50 49              PI      BVC &82A6
825D  AF 01              ..      BBS2 &8260
825F  50 4C              PL      BVC &82AD
8261  4F 54              OT      BBR4 &82B7
8263  F0 02              ..      BEQ &8267
8265  50 4F              PO      BVC &82B6
8267  49 4E              IN      EOR #&4E
8269  54                 T       ...
826A  28                 (       PLP
826B  B0 00              ..      BCS &826D
826D  50 52              PR      BVC &82C1
826F  4F 43              OC      BBR4 &82B4
8271  F2 0A              ..      SBC (&0A)
8273  50 4F              PO      BVC &82C4
8275  53                 S       ...
8276  B1 01              ..      LDA (&01),Y
8278  52 45              RE      EOR (&45)
827A  54                 T       ...
827B  55 52              UR      EOR &52,X
827D  4E F8 01           N..     LSR &01F8
8280  52 45              RE      EOR (&45)
8282  50 45              PE      BVC &82C9
8284  41 54              AT      EOR (&54,X)
8286  F5 00              ..      SBC &00,X
8288  52 45              RE      EOR (&45)
828A  50 4F              PO      BVC &82DB
828C  52 54              RT      EOR (&54)
828E  F6 01              ..      INC &01,X
8290  52 45              RE      EOR (&45)
8292  41 44              AD      EOR (&44,X)
8294  F3                 .       ...
8295  02                 .       ...
8296  52 45              RE      EOR (&45)
8298  4D F4 20           M.      EOR &20F4
829B  52 55              RU      EOR (&55)
829D  4E F9 01           N..     LSR &01F9
82A0  52 41              RA      EOR (&41)
82A2  44                 D       ...
82A3  B2 00              ..      LDA (&00)
82A5  52 45              RE      EOR (&45)
82A7  53                 S       ...
82A8  54                 T       ...
82A9  4F 52              OR      BBR4 &82FD
82AB  45 F7              E.      EOR &F7
82AD  12 52              .R      ORA (&52)
82AF  49 47              IG      EOR #&47
82B1  48                 H       PHA
82B2  54                 T       ...
82B3  24 28              $(      BIT &28
82B5  C2                 .       ...
82B6  00                 .       ...
82B7  52 4E              RN      EOR (&4E)
82B9  44                 D       ...
82BA  B3                 .       ...
82BB  01 52              .R      ORA (&52,X)
82BD  45 4E              EN      EOR &4E
82BF  55 4D              UM      EOR &4D,X
82C1  42                 B       ...
82C2  45 52              ER      EOR &52
82C4  CC 10 53           ..S     CPY &5310
82C7  54                 T       ...
82C8  45 50              EP      EOR &50
82CA  88                 .       DEY
82CB  00                 .       ...
82CC  53                 S       ...
82CD  41 56              AV      EOR (&56,X)
82CF  45 CD              E.      EOR &CD
82D1  02                 .       ...
82D2  53                 S       ...
82D3  47 4E              GN      SMB4 &4E
82D5  B4 00              ..      LDY &00
82D7  53                 S       ...
82D8  49 4E              IN      EOR #&4E
82DA  B5 00              ..      LDA &00,X
82DC  53                 S       ...
82DD  51 52              QR      EOR (&52),Y
82DF  B6 00              ..      LDX &00,Y
82E1  53                 S       ...
82E2  50 43              PC      BVC &8327
82E4  89 00 53           ..S     BIT #&5300
82E7  54                 T       ...
82E8  52 24              R$      EOR (&24)
82EA  C3                 .       ...
82EB  00                 .       ...
82EC  53                 S       ...
82ED  54                 T       ...
82EE  52 49              RI      EOR (&49)
82F0  4E 47 24           NG$     LSR &2447
82F3  28                 (       PLP
82F4  C4 00              ..      CPY &00
82F6  53                 S       ...
82F7  4F 55              OU      BBR4 &834E
82F9  4E 44 D4           ND.     LSR &D444
82FC  02                 .       ...
82FD  53                 S       ...
82FE  54                 T       ...
82FF  4F 50              OP      BBR4 &8351
8301  FA                 .       PLX
8302  01 54              .T      ORA (&54,X)
8304  41 4E              AN      EOR (&4E,X)
8306  B7 00              ..      SMB3 &00
8308  54                 T       ...
8309  48                 H       PHA
830A  45 4E              EN      EOR &4E
830C  8C 14 54           ..T     STY &5414
830F  4F B8              O.      BBR4 &82C9
8311  00                 .       ...
8312  54                 T       ...
8313  41 42              AB      EOR (&42,X)
8315  28                 (       PLP
8316  8A                 .       TXA
8317  00                 .       ...
8318  54                 T       ...
8319  52 41              RA      EOR (&41)
831B  43                 C       ...
831C  45 FC              E.      EOR &FC
831E  12 54              .T      ORA (&54)
8320  49 4D              IM      EOR #&4D
8322  45 91              E.      EOR &91
8324  43                 C       ...
8325  54                 T       ...
8326  52 55              RU      EOR (&55)
8328  45 B9              E.      EOR &B9
832A  01 55              .U      ORA (&55,X)
832C  4E 54 49           NTI     LSR &4954
832F  4C FD 02           L..     JMP &02FD
8332  55 53              US      EOR &53,X
8334  52 BA              R.      EOR (&BA)
8336  00                 .       ...
8337  56 44              VD      LSR &44,X
8339  55 EF              U.      EOR &EF,X
833B  02                 .       ...
833C  56 41              VA      LSR &41,X
833E  4C BB 00           L..     JMP &00BB
8341  56 50              VP      LSR &50,X
8343  4F 53              OS      BBR4 &8398
8345  BC 01 57           ..W     LDY &5701,X
8348  49 44              ID      EOR #&44
834A  54                 T       ...
834B  48                 H       PHA
834C  FE 02 50           ..P     INC &5002,X
834F  41 47              AG      EOR (&47,X)
8351  45 D0              E.      EOR &D0
8353  00                 .       ...
8354  50 54              PT      BVC &83AA
8356  52 CF              R.      EOR (&CF)
8358  00                 .       ...
8359  54                 T       ...
835A  49 4D              IM      EOR #&4D
835C  45 D1              E.      EOR &D1
835E  00                 .       ...
835F  4C 4F 4D           LOM     JMP &4D4F
8362  45 4D              EM      EOR &4D
8364  D2 00              ..      CMP (&00)
8366  48                 H       PHA
8367  49 4D              IM      EOR #&4D
8369  45 4D              EM      EOR &4D
836B  D3                 .       ...
836C  00                 .       ...

836D  78                 x       SEI
836E  47 C0              G.      SMB4 &C0
8370  B4 FC              ..      LDY &FC
8372  03                 .       ...
8373  6A                 j       ROR A
8374  D4                 .       ...
8375  33                 3       ...
8376  9E DA 07           ...     STZ &07DA,X
8379  6F 8D              o.      BBR6 &8308
837B  F7 C2              ..      SMB7 &C2
837D  9F A6              ..      BBS1 &8325
837F  E9 91              ..      SBC #&91
8381  46 CA              F.      LSR &CA
8383  95 B9              ..      STA &B9,X
8385  AD E2 78           ..x     LDA &78E2
8388  D1 FE              ..      CMP (&FE),Y
838A  A8                 .       TAY
838B  D1 80              ..      CMP (&80),Y
838D  7C CB 41           |.A     JMP (&41CB,X)
8390  6D B1 49           m.I     ADC &49B1
.
8393  88                 .       DEY
8394  98                 .       TYA
8395  B4 BE              ..      LDY &BE
8397  DC                 .       ...
.
8398  C4 D2              ..      CPY &D2
839A  2F 76              /v      BBR2 &8412
839C  BD BF 26           ..&     LDA &26BF,X
839F  CC 39 EE           .9.     CPY &EE39
83A2  94 C2              ..      STY &C2
83A4  B8                 .       CLV
83A5  AC 31 24           .1$     LDY &2431
.
83A8  9C DA B6           ...     STZ &B6DA
83AB  A3                 .       ...
83AC  F3                 .       ...
83AD  2A                 *       ROL A
83AE  30                 0

; LSB of jump table for tokens &D0-&FF

83AF  83                 .
83B0  C9 6F              .o      CMP #&6F
83B2  5D 4C 58           ]LX     EOR &584C,X
83B5  D2 2A              .*      CMP (&2A)
83B7  8D 99 BD           ...     STA &BD99
.
83BA  C4 7D              .}      CPY &7D
.
83BC  7D 2F E8           }/.     ADC &E82F,X
83BF  C8                 .       INY
83C0  56 72              Vr      LSR &72,X
83C2  C4 88              ..      CPY &88
83C4  CC 7A C2           .z.     CPY &C27A
83C7  44                 D       ...
83C8  E4 23              .#      CPX &23
83CA  9A                 .       TXS
83CB  E4 95              ..      CPX &95
83CD  15 2F              ./      ORA &2F,X
83CF  F1 9A              ..      SBC (&9A),Y
83D1  04 1F              ..      TSB &1F
83D3  7D E4 E4           }..     ADC &E4E4,X
83D6  E6 B6              ..      INC &B6
83D8  11 D0              ..      ORA (&D0),Y
83DA  8E 95 B1           ...     STX &B195
.
83DD  A0 C2              ..      LDY #&C2
.
83DF  BF BF              ..      BBS3 &83A0
83E1  AE AE AE           ...     LDX &AEAE
83E4  AF AD              ..      BBS2 &8393
83E6  A8                 .       TAY
83E7  AB                 .       ...
83E8  AC A8 A9           ...     LDY &A9A8
83EB  BF A9              ..      BBS3 &8396
83ED  AE AB AF           ...     LDX &AFAB
83F0  AF AB              ..      BBS2 &839D
83F2  AA                 .       TAX
83F3  BF AE              ..      BBS3 &83A3
83F5  B1 AF              ..      LDA (&AF),Y
83F7  AC AC AC           ...     LDY &ACAC
83FA  AE A7 AB           ...     LDX &ABA7
83FD  AC BF BF           ...     LDY &BFBF
8400  AB                 .       ...
8401  AB                 .       ...
8402  AB                 .       ...
.
8403  AB                 .       ...
8404  AF AB              ..      BBS2 &83B1
8406  A9 A7              ..      LDA #&A7
8408  A6 AE              ..      LDX &AE
840A  AC AB AC           ...     LDY &ACAB
840D  AB                 .       ...
840E  B3                 .       ...
840F  AF B0              ..      BBS2 &83C1
8411  AF B0              ..      BBS2 &83C3
8413  AF B0              ..      BBS2 &83C5
8415  B0 AC              ..      BCS &83C3
8417  90 8F              ..      BCC &83A8
8419  BF B5              ..      BBS3 &83D0
841B  8A                 .       TXA
841C  8A                 .       TXA
841D  8F BE              ..      BBS0 &83DD
841F  98                 .       TYA
8420  BF                 .

; MSB of jump table for tokens &D0-&FF
8421  92                 .
8422  92 92              ..      STA (&92)
8424  92 B4              ..      STA (&B4)
8426  BF 8E              ..      BBS3 &83B6
8428  BF 92              ..      BBS3 &83BC
842A  BF 8E              ..      BBS3 &83BA
842C  8E 8B 8B           ...     STX &8B8B
842F  91 93              ..      STA (&93),Y
8431  8A                 .       TXA
8432  93                 .       ...
8433  B4 B7              ..      LDY &B7
8435  B8                 .       CLV
8436  B8                 .       CLV
8437  93                 .       ...
8438  98                 .       TYA
8439  BA                 .       TXS
843A  8B                 .       ...
843B  93                 .       ...
843C  93                 .       ...
843D  93                 .       ...
843E  B6 B9              ..      LDX &B9,Y
8440  94 93              ..      STY &93
8442  8D 93 BB           ...     STA &BB93
8445  8B                 .       ...
8446  BB                 .       ...
8447  BF BA              ..      BBS3 &8403
8449  B8                 .       CLV
844A  BD 8A 93           ...     LDA &938A,X
844D  92 BB              ..      STA (&BB)
844F  B4                 .       DB &B4
8450  BE                 .       DB &BE

.
8451  4B                 K       ...
8452  83                 .       ...
8453  84 89              ..      STY &89
8455  96 B8              ..      STX &B8,Y
8457  B9 D8 D9           ...     LDA &D9D8,Y
845A  F0 01              ..      BEQ &845D
845C  10 81              ..      BPL &83DF
845E  90 89              ..      BCC &83E9
8460  93                 .       ...
8461  A3                 .       ...
8462  A4 A9              ..      LDY &A9
8464  38                 8       SEC
8465  39 78 01           9x.     AND &0178,Y
8468  13                 .       ...
8469  21 63              !c      AND (&63,X)
846B  73                 s       ...
846C  B1 A9              ..      LDA (&A9),Y
846E  C5 0C              ..      CMP &0C
8470  C3                 .       ...
8471  D3                 .       ...
8472  C4 F2              ..      CPY &F2
8474  41 83              A.      EOR (&83,X)
8476  B0 81              ..      BCS &83F9
8478  43                 C       ...
8479  6C 72 EC           lr.     JMP (&EC72)
847C  F2 A3              ..      SBC (&A3)
847E  C3                 .       ...
847F  18                 .       CLC
8480  19 34 B0           .4.     ORA &B034,Y
8483  72 98              r.      ADC (&98)
8485  99 81 98           ...     STA &9881,Y
8488  99 14              ..

.
848A  35                 5       DB &35
848B  0A                 .       ASL A
848C  0D 0D 0D           ...     ORA &0D0D
848F  0D 10 10           ...     ORA &1010
8492  25 25              %%      AND &25
8494  39 41 41           9AA     AND &4141,Y
8497  41 41              AA      EOR (&41,X)
8499  4A                 J       LSR A
849A  4A                 J       LSR A
849B  4C 4C 4C           LLL     JMP &4C4C
849E  50 50              PP      BVC &84F0
84A0  52 53              RS      EOR (&53)
84A2  53                 S       ...
84A3  53                 S       ...
84A4  08                 .       PHP
84A5  08                 .       PHP
84A6  08                 .       PHP
84A7  09 09              ..      ORA #&09
84A9  0A                 .       ASL A
84AA  0A                 .       ASL A
84AB  0A                 .       ASL A
84AC  05 15              ..      ORA &15
84AE  3E 04 0D           >..     ROL &0D04,X
84B1  30 4C              0L      BMI &84FF
84B3  06 32              .2      ASL &32
84B5  49 49              II      EOR #&49
84B7  10 25              .%      BPL &84DE
84B9  0E 0E 09           ...     ASL &090E
84BC  29 2A              )*      AND #&2A
84BE  30 30              00      BMI &84F0
84C0  4E 4E 4E           NNN     LSR &4E4E
84C3  3E 16              >.      DB &3E, &16

; -----------------------------------------------------------------------------
; Base opcode table
; -----------------------------------------------------------------------------
.
84C5  00                 .       BRK
84C6  18                 .       CLC
84C7  D8                 .       CLD
84C8  58                 X       CLI
84C9  B8                 .       CLV
84CA  CA                 .       DEX
84CB  88                 .       DEY
84CC  E8                 .       INX
84CD  C8                 .       INY
84CE  EA                 .       NOP
84CF  48                 H       PHA
84D0  08                 .       PHP
84D1  68                 h       PLA
84D2  28                 (       PLP
84D3  40                 @       RTI
84D4  60                 `       RTS
84D5  38                 8       SEC
84D6  F8                 .       SED
84D7  78                 x       SEI
84D8  AA                 .       TAX
84D9  A8                 .       TAY
84DA  BA                 .       TXS
84DB  8A                 .       TXA
84DC  9A                 .       TXS
84DD  98                 .       TYA
84DE  90                 .       BCC
84DF  B0                 .       BCS
84E0  F0                 .       BEQ
84E1  30                 0       BMI
84E2  D0                 .       BNE
84E3  10                 .       BPL
84E4  50                 P       BVC
84E5  70                 p       BVS
84E6  21                 !       AND
84E7  41                 A       EOR
84E8  01                 .       ORA
84E9  61                 a       ADC
84EA  C1                 .       CMP
84EB  A1                 .       LDA
84EC  E1                 .       SBC
84ED  06                 .       ASL
84EE  46                 F       LSR
84EF  26                 &       ROL
84F0  66                 f       ROR
84F1  C6                 .       DEC
84F2  E6                 .       INC
84F3  E0                 .       CPX
84F4  C0                 .       CPY
84F5  20                         BIT
84F6  4C                 .       JMP
84F7  20                         JSR
84F8  A2                 .       LDX
84F9  A0                 .       LDY
84FA  81                 .       STA
84FB  86                 .       STX
84FC  84                 .       STY

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
84FD  A9 FF              ..      LDA #&FF
84FF  85 28              .(      STA &28
8501  4C A3 8B           L..     JMP &8BA3
.
8504  A9 03              ..      LDA #&03                   ; OPT 3
8506  85 28              .(      STA &28
.
8508  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
850B  C9 5D              .]      CMP #']'
850D  F0 EE              ..      BEQ &84FD
850F  20 6D 98            m.     JSR &986D
.
8512  C6 0A              ..      DEC &0A
8514  20 BA 85            ..     JSR &85BA
8517  C6 0A              ..      DEC &0A
8519  A5 28              .(      LDA &28
851B  4A                 J       LSR A
851C  90 60              .`      BCC &857E
851E  A5 1E              ..      LDA &1E
8520  69 04              i.      ADC #&04
8522  85 3F              .?      STA &3F
8524  A5 38              .8      LDA &38
8526  20 45 B5            E.     JSR &B545
8529  A5 37              .7      LDA &37
852B  20 62 B5            b.     JSR &B562
852E  A2 FC              ..      LDX #&FC
8530  A4 39              .9      LDY &39
8532  10 02              ..      BPL &8536
8534  A4 36              .6      LDY &36
.
8536  84 38              .8      STY &38
8538  F0 1C              ..      BEQ &8556
853A  A0 00              ..      LDY #&00
.
853C  E8                 .       INX
853D  D0 0D              ..      BNE &854C
853F  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
8542  A6 3F              .?      LDX &3F
.
8544  20 65 B5            e.     JSR &B565
8547  CA                 .       DEX
8548  D0 FA              ..      BNE &8544
854A  A2 FD              ..      LDX #&FD
.
854C  B1 3A              .:      LDA (&3A),Y
854E  20 62 B5            b.     JSR &B562
8551  C8                 .       INY
8552  C6 38              .8      DEC &38
8554  D0 E6              ..      BNE &853C
.
8556  E8                 .       INX
8557  10 0C              ..      BPL &8565
8559  20 65 B5            e.     JSR &B565
855C  20 58 B5            X.     JSR &B558
855F  20 58 B5            X.     JSR &B558
8562  4C 56 85           LV.     JMP &8556
.
8565  A0 00              ..      LDY #&00
.
8567  B1 0B              ..      LDA (&0B),Y
8569  C9 3A              .:      CMP #&3A
856B  F0 0A              ..      BEQ &8577
856D  C9 0D              ..      CMP #&0D
856F  F0 0A              ..      BEQ &857B
.
8571  20 0E B5            ..     JSR &B50E                  ; Print character or token
8574  C8                 .       INY
8575  D0 F0              ..      BNE &8567
.
8577  C4 0A              ..      CPY &0A
8579  90 F6              ..      BCC &8571
.
857B  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
.
857E  A4 0A              ..      LDY &0A
8580  88                 .       DEY
.
8581  C8                 .       INY
8582  B1 0B              ..      LDA (&0B),Y
8584  C9 3A              .:      CMP #&3A
8586  F0 04              ..      BEQ &858C
8588  C9 0D              ..      CMP #&0D
858A  D0 F5              ..      BNE &8581
.
858C  20 59 98            Y.     JSR &9859
858F  88                 .       DEY
8590  B1 0B              ..      LDA (&0B),Y
8592  C9 3A              .:      CMP #&3A
8594  F0 0C              ..      BEQ &85A2
8596  A5 0C              ..      LDA &0C
8598  C9 07              ..      CMP #&07
859A  D0 03              ..      BNE &859F
859C  4C F6 8A           L..     JMP &8AF6
.
859F  20 90 98            ..     JSR &9890
.
85A2  4C 08 85           L..     JMP &8508
.
85A5  20 82 95            ..     JSR &9582
85A8  F0 5A              .Z      BEQ &8604
85AA  B0 58              .X      BCS &8604
85AC  20 94 BD            ..     JSR &BD94
85AF  20 3A AE            :.     JSR &AE3A
85B2  85 27              .'      STA &27
85B4  20 B4 B4            ..     JSR &B4B4
85B7  20 27 88            '.     JSR &8827
.
85BA  A2 03              ..      LDX #&03
85BC  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
85BF  A0 00              ..      LDY #&00
85C1  84 3D              .=      STY &3D
85C3  C9 3A              .:      CMP #':'
85C5  F0 64              .d      BEQ &862B
85C7  C9 0D              ..      CMP #&0D
85C9  F0 60              .`      BEQ &862B
85CB  C9 5C              .\      CMP #'\'
85CD  F0 5C              .\      BEQ &862B
85CF  C9 2E              ..      CMP #'.'
85D1  F0 D2              ..      BEQ &85A5
85D3  C6 0A              ..      DEC &0A
.
85D5  A4 0A              ..      LDY &0A
85D7  E6 0A              ..      INC &0A
85D9  B1 0B              ..      LDA (&0B),Y
85DB  30 2A              0*      BMI &8607
85DD  C9 20              .       CMP #&20
85DF  F0 10              ..      BEQ &85F1
85E1  A0 05              ..      LDY #&05
85E3  0A                 .       ASL A
85E4  0A                 .       ASL A
85E5  0A                 .       ASL A
.
85E6  0A                 .       ASL A
85E7  26 3D              &=      ROL &3D
85E9  26 3E              &>      ROL &3E
85EB  88                 .       DEY
85EC  D0 F8              ..      BNE &85E6
85EE  CA                 .       DEX
85EF  D0 E4              ..      BNE &85D5
.
85F1  A2 3A              .:      LDX #&3A
85F3  A5 3D              .=      LDA &3D
.
85F5  DD 50 84           .P.     CMP &8450,X
85F8  D0 07              ..      BNE &8601
85FA  BC 8A 84           ...     LDY &848A,X
85FD  C4 3E              .>      CPY &3E
85FF  F0 1F              ..      BEQ &8620
.
8601  CA                 .       DEX
8602  D0 F1              ..      BNE &85F5
.
8604  4C 2A 98           L*.     JMP &982A
.
8607  A2 22              ."      LDX #&22
8609  C9 80              ..      CMP #&80
860B  F0 13              ..      BEQ &8620
860D  E8                 .       INX
860E  C9 82              ..      CMP #&82
8610  F0 0E              ..      BEQ &8620
8612  E8                 .       INX
8613  C9 84              ..      CMP #&84
8615  D0 ED              ..      BNE &8604
8617  E6 0A              ..      INC &0A
8619  C8                 .       INY
861A  B1 0B              ..      LDA (&0B),Y
861C  C9 41              .A      CMP #&41
861E  D0 E4              ..      BNE &8604
.
8620  BD C4 84           ...     LDA &84C4,X
8623  85 29              .)      STA &29
8625  A0 01              ..      LDY #&01
8627  E0 1A              ..      CPX #&1A
8629  B0 48              .H      BCS &8673
.
862B  AD 40 04           .@.     LDA &0440
862E  85 37              .7      STA &37
8630  84 39              .9      STY &39
8632  A6 28              .(      LDX &28
8634  E0 04              ..      CPX #&04
8636  AE 41 04           .A.     LDX &0441
8639  86 38              .8      STX &38
863B  90 06              ..      BCC &8643
863D  AD 3C 04           .<.     LDA &043C
8640  AE 3D 04           .=.     LDX &043D
.
8643  85 3A              .:      STA &3A
8645  86 3B              .;      STX &3B
8647  98                 .       TYA
8648  F0 28              .(      BEQ &8672
864A  10 04              ..      BPL &8650
864C  A4 36              .6      LDY &36
864E  F0 22              ."      BEQ &8672
.
8650  88                 .       DEY
8651  B9 29 00           .).     LDA &0029,Y
8654  24 39              $9      BIT &39
8656  10 03              ..      BPL &865B
8658  B9 00 06           ...     LDA &0600,Y
.
865B  91 3A              .:      STA (&3A),Y
865D  EE 40 04           .@.     INC &0440
8660  D0 03              ..      BNE &8665
8662  EE 41 04           .A.     INC &0441
.
8665  90 08              ..      BCC &866F
8667  EE 3C 04           .<.     INC &043C
866A  D0 03              ..      BNE &866F
866C  EE 3D 04           .=.     INC &043D
.
866F  98                 .       TYA
8670  D0 DE              ..      BNE &8650
.
8672  60                 `       RTS
.
8673  E0 22              ."      CPX #&22
8675  B0 40              .@      BCS &86B7
8677  20 21 88            !.     JSR &8821
867A  18                 .       CLC
867B  A5 2A              .*      LDA &2A
867D  ED 40 04           .@.     SBC &0440
8680  A8                 .       TAY
8681  A5 2B              .+      LDA &2B
8683  ED 41 04           .A.     SBC &0441
8686  C0 01              ..      CPY #&01
8688  88                 .       DEY
8689  E9 00              ..      SBC #&00
868B  F0 25              .%      BEQ &86B2
868D  C9 FF              ..      CMP #&FF
868F  F0 1C              ..      BEQ &86AD
.
8691  A5 28              .(      LDA &28
8693  4A                 J       LSR A
8694  F0 0F              ..      BEQ &86A5

8696  00                 .       ...                        ; Error &01: Out of range
8697  01 4F              .O      ORA (&4F,X)
8699  75 74              ut      ADC &74,X
869B  20 6F 66            of     JSR &666F
869E  20 72 61            ra     JSR &6172
86A1  6E 67 65           nge     ROR &6567
86A4  00                 .       ...
.
86A5  A8                 .       TAY
.
86A6  84 2A              .*      STY &2A
.
86A8  A0 02              ..      LDY #&02
86AA  4C 2B 86           L+.     JMP &862B
.
86AD  98                 .       TYA
86AE  30 F6              0.      BMI &86A6
86B0  10 DF              ..      BPL &8691
.
86B2  98                 .       TYA
86B3  10 F1              ..      BPL &86A6
86B5  30 DA              0.      BMI &8691
.
86B7  E0 29              .)      CPX #&29
86B9  B0 18              ..      BCS &86D3
86BB  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
86BE  C9 23              .#      CMP #'#'
86C0  D0 18              ..      BNE &86DA
86C2  20 2F 88            /.     JSR &882F
.
86C5  20 21 88            !.     JSR &8821
.
86C8  A5 2B              .+      LDA &2B
86CA  F0 DC              ..      BEQ &86A8

.
86CC  00                 .       ...
86CD  02                 .       ...
86CE  42                 B       ...
86CF  79 74 65           yte     ADC &6574,Y
86D2  00                 .       ...
.
86D3  E0 36              .6      CPX #&36
86D5  D0 68              .h      BNE &873F
86D7  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
.
86DA  C9 28              .(      CMP #'('
86DC  D0 37              .7      BNE &8715
86DE  20 21 88            !.     JSR &8821
86E1  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
86E4  C9 29              .)      CMP #')'
86E6  D0 13              ..      BNE &86FB
86E8  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
86EB  C9 2C              .,      CMP #','
86ED  D0 1E              ..      BNE &870D                  ; Generate "Index" error message
86EF  20 2C 88            ,.     JSR &882C
86F2  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
86F5  C9 59              .Y      CMP #'Y'
86F7  D0 14              ..      BNE &870D                  ; Generate "Index" error message
86F9  F0 CD              ..      BEQ &86C8
.
86FB  C9 2C              .,      CMP #','
86FD  D0 0E              ..      BNE &870D                  ; Generate "Index" error message
86FF  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8702  C9 58              .X      CMP #'X'
8704  D0 07              ..      BNE &870D                  ; Generate "Index" error message
8706  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8709  C9 29              .)      CMP #')'
870B  F0 BB              ..      BEQ &86C8

.
870D  00                 .       ...
870E  03                 .       ...
870F  49 6E              In      EOR #&6E
8711  64 65              de      STZ &65
8713  78                 x       SEI
8714  00                 .       ...
.
8715  C6 0A              ..      DEC &0A
8717  20 21 88            !.     JSR &8821
871A  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
871D  C9 2C              .,      CMP #','
871F  D0 14              ..      BNE &8735
8721  20 2C 88            ,.     JSR &882C
8724  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8727  C9 58              .X      CMP #'X'
8729  F0 0A              ..      BEQ &8735
872B  C9 59              .Y      CMP #'Y'
872D  D0 DE              ..      BNE &870D                  ; Generate "Index" error message
.
872F  20 2F 88            /.     JSR &882F
8732  4C 9A 87           L..     JMP &879A
.
8735  20 32 88            2.     JSR &8832
.
8738  A5 2B              .+      LDA &2B
873A  D0 F3              ..      BNE &872F
873C  4C A8 86           L..     JMP &86A8
.
873F  E0 2F              ./      CPX #&2F
8741  B0 2B              .+      BCS &876E
8743  E0 2D              .-      CPX #&2D
8745  B0 09              ..      BCS &8750
8747  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
874A  C9 41              .A      CMP #'A'
874C  F0 19              ..      BEQ &8767
874E  C6 0A              ..      DEC &0A
.
8750  20 21 88            !.     JSR &8821
8753  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8756  C9 2C              .,      CMP #','
8758  D0 DE              ..      BNE &8738
875A  20 2C 88            ,.     JSR &882C
875D  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8760  C9 58              .X      CMP #'X'
8762  F0 D4              ..      BEQ &8738
8764  4C 0D 87           L..     JMP &870D                  ; Generate "Index" error message
.
8767  20 32 88            2.     JSR &8832
876A  A0 01              ..      LDY #&01
876C  D0 2E              ..      BNE &879C
.
876E  E0 32              .2      CPX #&32
8770  B0 16              ..      BCS &8788
8772  E0 31              .1      CPX #&31
8774  F0 0C              ..      BEQ &8782
8776  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8779  C9 23              .#      CMP #'#'
877B  D0 03              ..      BNE &8780
877D  4C C5 86           L..     JMP &86C5
.
8780  C6 0A              ..      DEC &0A
.
8782  20 21 88            !.     JSR &8821
8785  4C 35 87           L5.     JMP &8735
.
8788  E0 33              .3      CPX #&33
878A  F0 0B              ..      BEQ &8797
878C  B0 24              .$      BCS &87B2
878E  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8791  C9 28              .(      CMP #'('
8793  F0 0A              ..      BEQ &879F
8795  C6 0A              ..      DEC &0A
.
8797  20 21 88            !.     JSR &8821
.
879A  A0 03              ..      LDY #&03
.
879C  4C 2B 86           L+.     JMP &862B
.
879F  20 2C 88            ,.     JSR &882C
87A2  20 2C 88            ,.     JSR &882C
87A5  20 21 88            !.     JSR &8821
87A8  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
87AB  C9 29              .)      CMP #')'
87AD  F0 EB              ..      BEQ &879A
87AF  4C 0D 87           L..     JMP &870D                  ; Generate "Index" error message
.
87B2  E0 39              .9      CPX #&39
87B4  B0 5D              .]      BCS &8813
87B6  A5 3D              .=      LDA &3D
87B8  49 01              I.      EOR #&01
87BA  29 1F              ).      AND #&1F
87BC  48                 H       PHA
87BD  E0 37              .7      CPX #&37
87BF  B0 2F              ./      BCS &87F0
87C1  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
87C4  C9 23              .#      CMP #'#'
87C6  D0 04              ..      BNE &87CC
87C8  68                 h       PLA
87C9  4C C5 86           L..     JMP &86C5
.
87CC  C6 0A              ..      DEC &0A
87CE  20 21 88            !.     JSR &8821
87D1  68                 h       PLA
87D2  85 37              .7      STA &37
87D4  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
87D7  C9 2C              .,      CMP #','
87D9  F0 03              ..      BEQ &87DE
87DB  4C 35 87           L5.     JMP &8735
.
87DE  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
87E1  29 1F              ).      AND #&1F
87E3  C5 37              .7      CMP &37
87E5  D0 06              ..      BNE &87ED
87E7  20 2C 88            ,.     JSR &882C
87EA  4C 35 87           L5.     JMP &8735
.
87ED  4C 0D 87           L..     JMP &870D                  ; Generate "Index" error message
.
87F0  20 21 88            !.     JSR &8821
87F3  68                 h       PLA
87F4  85 37              .7      STA &37
87F6  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
87F9  C9 2C              .,      CMP #','
87FB  D0 13              ..      BNE &8810
87FD  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8800  29 1F              ).      AND #&1F
8802  C5 37              .7      CMP &37
8804  D0 E7              ..      BNE &87ED
8806  20 2C 88            ,.     JSR &882C
8809  A5 2B              .+      LDA &2B
880B  F0 03              ..      BEQ &8810
880D  4C CC 86           L..     JMP &86CC                  ; Generate "Byte" error message
.
8810  4C 38 87           L8.     JMP &8738
.
8813  D0 25              .%      BNE &883A
8815  20 21 88            !.     JSR &8821
8818  A5 2A              .*      LDA &2A
881A  85 28              .(      STA &28
881C  A0 00              ..      LDY #&00
881E  4C 2B 86           L+.     JMP &862B
.
8821  20 1D 9B            ..     JSR &9B1D
8824  20 F0 92            ..     JSR &92F0
.
8827  A4 1B              ..      LDY &1B
8829  84 0A              ..      STY &0A
882B  60                 `       RTS
.
882C  20 2F 88            /.     JSR &882F
.
882F  20 32 88            2.     JSR &8832
.
8832  A5 29              .)      LDA &29
8834  18                 .       CLC
8835  69 04              i.      ADC #&04
8837  85 29              .)      STA &29
8839  60                 `       RTS
.
883A  A2 01              ..      LDX #&01
883C  A4 0A              ..      LDY &0A
883E  E6 0A              ..      INC &0A
8840  B1 0B              ..      LDA (&0B),Y
8842  C9 42              .B      CMP #&42
8844  F0 12              ..      BEQ &8858
8846  E8                 .       INX
8847  C9 57              .W      CMP #&57
8849  F0 0D              ..      BEQ &8858
884B  A2 04              ..      LDX #&04
884D  C9 44              .D      CMP #&44
884F  F0 07              ..      BEQ &8858
8851  C9 53              .S      CMP #&53
8853  F0 15              ..      BEQ &886A
8855  4C 2A 98           L*.     JMP &982A
.
8858  8A                 .       TXA
8859  48                 H       PHA
885A  20 21 88            !.     JSR &8821
885D  A2 29              .)      LDX #&29
885F  20 44 BE            D.     JSR &BE44
8862  68                 h       PLA
8863  A8                 .       TAY
.
8864  4C 2B 86           L+.     JMP &862B
.
8867  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
886A  A5 28              .(      LDA &28
886C  48                 H       PHA
886D  20 1D 9B            ..     JSR &9B1D
8870  D0 F5              ..      BNE &8867
8872  68                 h       PLA
8873  85 28              .(      STA &28
8875  20 27 88            '.     JSR &8827
8878  A0 FF              ..      LDY #&FF
887A  D0 E8              ..      BNE &8864
.
887C  48                 H       PHA
887D  18                 .       CLC
887E  98                 .       TYA
887F  65 37              e7      ADC &37
8881  85 39              .9      STA &39
8883  A0 00              ..      LDY #&00
8885  98                 .       TYA
8886  65 38              e8      ADC &38
8888  85 3A              .:      STA &3A
888A  68                 h       PLA
888B  91 37              .7      STA (&37),Y
.
888D  C8                 .       INY
888E  B1 39              .9      LDA (&39),Y
8890  91 37              .7      STA (&37),Y
8892  C9 0D              ..      CMP #&0D
8894  D0 F7              ..      BNE &888D
8896  60                 `       RTS
.
8897  29 0F              ).      AND #&0F
8899  85 3D              .=      STA &3D
889B  84 3E              .>      STY &3E
.
889D  C8                 .       INY
889E  B1 37              .7      LDA (&37),Y
88A0  C9 3A              .:      CMP #&3A
88A2  B0 36              .6      BCS &88DA
88A4  C9 30              .0      CMP #&30
88A6  90 32              .2      BCC &88DA
88A8  29 0F              ).      AND #&0F
88AA  48                 H       PHA
88AB  A6 3E              .>      LDX &3E
88AD  A5 3D              .=      LDA &3D
88AF  0A                 .       ASL A
88B0  26 3E              &>      ROL &3E
88B2  30 21              0!      BMI &88D5
88B4  0A                 .       ASL A
88B5  26 3E              &>      ROL &3E
88B7  30 1C              0.      BMI &88D5
88B9  65 3D              e=      ADC &3D
88BB  85 3D              .=      STA &3D
88BD  8A                 .       TXA
88BE  65 3E              e>      ADC &3E
88C0  06 3D              .=      ASL &3D
88C2  2A                 *       ROL A
88C3  30 10              0.      BMI &88D5
88C5  B0 0E              ..      BCS &88D5
88C7  85 3E              .>      STA &3E
88C9  68                 h       PLA
88CA  65 3D              e=      ADC &3D
88CC  85 3D              .=      STA &3D
88CE  90 CD              ..      BCC &889D
88D0  E6 3E              .>      INC &3E
88D2  10 C9              ..      BPL &889D
88D4  48                 H       PHA
.
88D5  68                 h       PLA
88D6  A0 00              ..      LDY #&00
88D8  38                 8       SEC
88D9  60                 `       RTS
.
88DA  88                 .       DEY
88DB  A9 8D              ..      LDA #&8D
88DD  20 7C 88            |.     JSR &887C
88E0  A5 37              .7      LDA &37
88E2  69 02              i.      ADC #&02
88E4  85 39              .9      STA &39
88E6  A5 38              .8      LDA &38
88E8  69 00              i.      ADC #&00
88EA  85 3A              .:      STA &3A
.
88EC  B1 37              .7      LDA (&37),Y
88EE  91 39              .9      STA (&39),Y
88F0  88                 .       DEY
88F1  D0 F9              ..      BNE &88EC
88F3  A0 03              ..      LDY #&03
.
88F5  A5 3E              .>      LDA &3E
88F7  09 40              .@      ORA #&40
88F9  91 37              .7      STA (&37),Y
88FB  88                 .       DEY
88FC  A5 3D              .=      LDA &3D
88FE  29 3F              )?      AND #&3F
8900  09 40              .@      ORA #&40
8902  91 37              .7      STA (&37),Y
8904  88                 .       DEY
8905  A5 3D              .=      LDA &3D
8907  29 C0              ).      AND #&C0
8909  85 3D              .=      STA &3D
890B  A5 3E              .>      LDA &3E
890D  29 C0              ).      AND #&C0
890F  4A                 J       LSR A
8910  4A                 J       LSR A
8911  05 3D              .=      ORA &3D
8913  4A                 J       LSR A
8914  4A                 J       LSR A
8915  49 54              IT      EOR #&54
8917  91 37              .7      STA (&37),Y
8919  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
891C  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
891F  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
8922  A0 00              ..      LDY #&00
.
8924  18                 .       CLC
8925  60                 `       RTS
.
8926  C9 7B              .{      CMP #&7B
8928  B0 FA              ..      BCS &8924
892A  C9 5F              ._      CMP #&5F
892C  B0 0E              ..      BCS &893C
892E  C9 5B              .[      CMP #&5B
8930  B0 F2              ..      BCS &8924
8932  C9 41              .A      CMP #&41
8934  B0 06              ..      BCS &893C
.
8936  C9 3A              .:      CMP #'9'+1
8938  B0 EA              ..      BCS &8924                  ; Clear carry and exit
893A  C9 30              .0      CMP #'0'
.
893C  60                 `       RTS
.
893D  C9 2E              ..      CMP #'.'
893F  D0 F5              ..      BNE &8936
8941  60                 `       RTS
.
8942  B1 37              .7      LDA (&37),Y
.
8944  E6 37              .7      INC &37
8946  D0 02              ..      BNE &894A
8948  E6 38              .8      INC &38
.
894A  60                 `       RTS
.
894B  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
894E  B1 37              .7      LDA (&37),Y
8950  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
8951  A0 00              ..      LDY #&00
8953  84 3B              .;      STY &3B
.
8955  84 3C              .<      STY &3C
.
8957  B1 37              .7      LDA (&37),Y
8959  C9 0D              ..      CMP #&0D
895B  F0 ED              ..      BEQ &894A                  ; Jump if it's and end-of-line marker
895D  C9 20              .       CMP #' '
895F  D0 05              ..      BNE &8966
.
8961  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
8964  D0 F1              ..      BNE &8957                  ; And process the next character
.
8966  C9 26              .&      CMP #'&'                   ; Hex value marker
8968  D0 12              ..      BNE &897C
.
896A  20 4B 89            K.     JSR &894B                  ; Increase &0037-0038 (temporary text pointer) with 1 and load next character
896D  20 36 89            6.     JSR &8936                  ; Returns with C=1 if the character was numeric
8970  B0 F8              ..      BCS &896A                  ; Process next character if it was numeric
8972  C9 41              .A      CMP #'A'
8974  90 E1              ..      BCC &8957
8976  C9 47              .G      CMP #'G'
8978  90 F0              ..      BCC &896A
897A  B0 DB              ..      BCS &8957
.
897C  C9 22              ."      CMP #'"'
897E  D0 0C              ..      BNE &898C
.
8980  20 4B 89            K.     JSR &894B                  ; Increase &0037-0038 (temporary text pointer) with 1 and load next character
8983  C9 22              ."      CMP #'"'
8985  F0 DA              ..      BEQ &8961                  ; Process next character
8987  C9 0D              ..      CMP #&0D
8989  D0 F5              ..      BNE &8980
898B  60                 `       RTS
.
898C  C9 3A              .:      CMP #':'
898E  D0 06              ..      BNE &8996
8990  84 3B              .;      STY &3B
8992  84 3C              .<      STY &3C
8994  F0 CB              ..      BEQ &8961                  ; Process next character
.
8996  C9 2C              .,      CMP #','
8998  F0 C7              ..      BEQ &8961                  ; Process next character
899A  C9 2A              .*      CMP #'*'
899C  D0 05              ..      BNE &89A3
899E  A5 3B              .;      LDA &3B
89A0  D0 41              .A      BNE &89E3                  ;
89A2  60                 `       RTS
.
89A3  C9 2E              ..      CMP #'.'
89A5  F0 0E              ..      BEQ &89B5
89A7  20 36 89            6.     JSR &8936                  ; Returns with C=1 if the character was numeric
89AA  90 33              .3      BCC &89DF
89AC  A6 3C              .<      LDX &3C
89AE  F0 05              ..      BEQ &89B5
89B0  20 97 88            ..     JSR &8897
89B3  90 34              .4      BCC &89E9
.
89B5  B1 37              .7      LDA (&37),Y
89B7  20 3D 89            =.     JSR &893D
89BA  90 06              ..      BCC &89C2
89BC  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
89BF  4C B5 89           L..     JMP &89B5
.
89C2  A2 FF              ..      LDX #&FF
89C4  86 3B              .;      STX &3B
89C6  84 3C              .<      STY &3C
89C8  4C 57 89           LW.     JMP &8957
.
89CB  20 26 89            &.     JSR &8926
89CE  90 13              ..      BCC &89E3
.
89D0  A0 00              ..      LDY #&00
.
89D2  B1 37              .7      LDA (&37),Y
89D4  20 26 89            &.     JSR &8926
89D7  90 E9              ..      BCC &89C2
89D9  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
89DC  4C D2 89           L..     JMP &89D2
.
89DF  C9 41              .A      CMP #'A'
89E1  B0 09              ..      BCS &89EC                  ; Jump if character is >= 'A'
.
89E3  A2 FF              ..      LDX #&FF
89E5  86 3B              .;      STX &3B
89E7  84 3C              .<      STY &3C
.
89E9  4C 61 89           La.     JMP &8961
.
89EC  C9 58              .X      CMP #'X'                   ; Jump if character is >= 'X'
89EE  B0 DB              ..      BCS &89CB
89F0  A2 71              .q      LDX #&8071 MOD 256         ; Pointer to keyword table
89F2  86 39              .9      STX &39
89F4  A2 80              ..      LDX #&8071 DIV 256
89F6  86 3A              .:      STX &3A
.
89F8  D1 39              .9      CMP (&39),Y
89FA  90 D6              ..      BCC &89D2                  ; Jump if A < (&39),Y
89FC  D0 0F              ..      BNE &8A0D
.
89FE  C8                 .       INY
89FF  B1 39              .9      LDA (&39),Y
8A01  30 34              04      BMI &8A37
8A03  D1 37              .7      CMP (&37),Y                ; Compare with string in line
8A05  F0 F7              ..      BEQ &89FE
8A07  B1 37              .7      LDA (&37),Y
8A09  C9 2E              ..      CMP #'.'                   ; Is the keyword abbreviated?
8A0B  F0 0B              ..      BEQ &8A18
.
8A0D  C8                 .       INY
8A0E  B1 39              .9      LDA (&39),Y
8A10  10 FB              ..      BPL &8A0D                  ; Loop through this keyword
8A12  C9 FE              ..      CMP #&FE
8A14  D0 0F              ..      BNE &8A25
8A16  B0 B8              ..      BCS &89D0                  ; Jump if the token is &FE (WIDTH) or &FF (OSCLI)
.
8A18  C8                 .       INY
.
8A19  B1 39              .9      LDA (&39),Y                ; Find the token
8A1B  30 1A              0.      BMI &8A37
8A1D  E6 39              .9      INC &39                    ; Not found yet (keyword was abbreviated) so process next character
8A1F  D0 F8              ..      BNE &8A19
8A21  E6 3A              .:      INC &3A
8A23  D0 F4              ..      BNE &8A19
.
8A25  38                 8       SEC
8A26  C8                 .       INY
8A27  98                 .       TYA
8A28  65 39              e9      ADC &39
8A2A  85 39              .9      STA &39
8A2C  90 02              ..      BCC &8A30
8A2E  E6 3A              .:      INC &3A
.
8A30  A0 00              ..      LDY #&00
8A32  B1 37              .7      LDA (&37),Y
8A34  4C F8 89           L..     JMP &89F8
.                                                           ; Keyword found in keyword list
8A37  AA                 .       TAX                        ; Put token in X
8A38  C8                 .       INY
8A39  B1 39              .9      LDA (&39),Y                ; Keyword flag byte
8A3B  85 3D              .=      STA &3D
8A3D  88                 .       DEY
8A3E  4A                 J       LSR A
8A3F  90 07              ..      BCC &8A48                  ; Jump if bit 0 wasn't set
8A41  B1 37              .7      LDA (&37),Y
8A43  20 26 89            &.     JSR &8926
8A46  B0 88              ..      BCS &89D0
.
8A48  8A                 .       TXA
8A49  24 3D              $=      BIT &3D
8A4B  50 07              P.      BVC &8A54                  ; Jump if bit 6 wasn't set
8A4D  A6 3B              .;      LDX &3B
8A4F  D0 03              ..      BNE &8A54
8A51  18                 .       CLC
8A52  69 40              i@      ADC #&40
.
8A54  88                 .       DEY
8A55  20 7C 88            |.     JSR &887C
8A58  A0 00              ..      LDY #&00
8A5A  A2 FF              ..      LDX #&FF
8A5C  A5 3D              .=      LDA &3D
8A5E  4A                 J       LSR A
8A5F  4A                 J       LSR A
8A60  90 04              ..      BCC &8A66                  ; Jump if bit 1 wasn't set
8A62  86 3B              .;      STX &3B                    ; Middle flag is set, so &3B=&FF and &3C=&00
8A64  84 3C              .<      STY &3C
.
8A66  4A                 J       LSR A
8A67  90 04              ..      BCC &8A6D                  ; Jump if bit 2 wasn't set
8A69  84 3B              .;      STY &3B                    ; Start flag is set, so &3B=&00 and &3C=&00
8A6B  84 3C              .<      STY &3C
.
8A6D  4A                 J       LSR A
8A6E  90 11              ..      BCC &8A81                  ; Jump if bit 3 wasn't set
8A70  48                 H       PHA                        ; PROC/FN flag was set, so
8A71  C8                 .       INY
.
8A72  B1 37              .7      LDA (&37),Y
8A74  20 26 89            &.     JSR &8926
8A77  90 06              ..      BCC &8A7F
8A79  20 44 89            D.     JSR &8944                  ; Increase &0037-0038 (temporary text pointer) with 1
8A7C  4C 72 8A           Lr.     JMP &8A72
.
8A7F  88                 .       DEY
8A80  68                 h       PLA
.
8A81  4A                 J       LSR A
8A82  90 02              ..      BCC &8A86                  ; Jump if bit 4 was clear
8A84  86 3C              .<      STX &3C                    ; Line number flag was set, so set &3C=&FF
.
8A86  4A                 J       LSR A
8A87  B0 0D              ..      BCS &8A96                  ; Jump if bit 5 was set
8A89  4C 61 89           La.     JMP &8961                  ; Process next character
.
8A8C  A4 1B              ..      LDY &1B
8A8E  E6 1B              ..      INC &1B
8A90  B1 19              ..      LDA (&19),Y
8A92  C9 20              .       CMP #&20
8A94  F0 F6              ..      BEQ &8A8C
.
8A96  60                 `       RTS                        ; REM flag was set, so exit

; -----------------------------------------------------------------------------
; Find first non-SPACE character in PtrA
; -----------------------------------------------------------------------------
.
8A97  A4 0A              ..      LDY &0A
8A99  E6 0A              ..      INC &0A
8A9B  B1 0B              ..      LDA (&0B),Y
8A9D  C9 20              .       CMP #' '
8A9F  F0 F6              ..      BEQ &8A97
8AA1  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
8AA2  00                 .       ...
8AA3  05 4D              .M      ORA &4D
8AA5  69 73              is      ADC #&73
8AA7  73                 s       ...
8AA8  69 6E              in      ADC #&6E
8AAA  67 20              g       RMB6 &20
8AAC  2C 00 20           ,.      BIT &2000
8AAF  8C 8A C9           ...     STY &C98A
8AB2  2C D0 ED           ,..     BIT &EDD0
8AB5  60                 `       RTS
8AB6  20 57 98            W.     JSR &9857                  ; Check end of statement
8AB9  A5 18              ..      LDA page
8ABB  85 38              .8      STA &38                    ; Set MSB of RENUMBER pointer to PAGE
8ABD  A9 00              ..      LDA #&00
8ABF  85 37              .7      STA &37                    ; Set LSB of RENUMBER pointer to PAGE
8AC1  91 37              .7      STA (&37),Y
8AC3  20 6F BE            o.     JSR &BE6F
8AC6  D0 2B              .+      BNE &8AF3
8AC8  20 57 98            W.     JSR &9857                  ; Check end of statement
8ACB  20 6F BE            o.     JSR &BE6F
8ACE  D0 26              .&      BNE &8AF6
8AD0  20 57 98            W.     JSR &9857                  ; Check end of statement
8AD3  00                 .       ...
8AD4  00                 .       ...
8AD5  53                 S       ...
8AD6  54                 T       ...
8AD7  4F 50              OP      BBR4 &8B29
8AD9  00                 .       ...
8ADA  20 57 98            W.     JSR &9857                  ; Check end of statement
.
8ADD  A9 0D              ..      LDA #&0D                   ; End of line marker
8ADF  A4 18              ..      LDY page
8AE1  84 13              ..      STY &13                    ; Set TOP to PAGE
8AE3  A0 00              ..      LDY #&00
8AE5  84 12              ..      STY &12
8AE7  84 20              .       STY &20                    ; Reset TRACE flag
8AE9  91 12              ..      STA (&12),Y
8AEB  A9 FF              ..      LDA #&FF                   ; End of program marker
8AED  C8                 .       INY
8AEE  91 12              ..      STA (&12),Y
8AF0  C8                 .       INY
8AF1  84 12              ..      STY &12
.
8AF3  20 20 BD             .     JSR &BD20
.
8AF6  A0 07              ..      LDY #&0700 DIV 256         ; Set PtrA base to &0700 (BASIC text input buffer)
8AF8  84 0C              ..      STY &0C
8AFA  A0 00              ..      LDY #&0700 MOD 256
8AFC  84 0B              ..      STY &0B
8AFE  A9 33              .3      LDA #&B433 MOD 256         ; Set ON ERROR pointer to the default ON ERROR string
8B00  85 16              ..      STA &16
8B02  A9 B4              ..      LDA #&B433 DIV 256
8B04  85 17              ..      STA &17
8B06  A9 3E              .>      LDA #'>'
8B08  20 02 BC            ..     JSR &BC02
.
8B0B  A9 33              .3      LDA #&B433 MOD 256         ; Set ON ERROR pointer to the default ON ERROR string
8B0D  85 16              ..      STA &16
8B0F  A9 B4              ..      LDA #&B433 DIV 256
8B11  85 17              ..      STA &17
8B13  A2 FF              ..      LDX #&FF
8B15  86 28              .(      STX &28                    ; Reset assembler OPT mask
8B17  86 3C              .<      STX &3C
8B19  9A                 .       TXS                        ; Reset stack pointer
8B1A  20 3A BD            :.     JSR &BD3A
8B1D  A8                 .       TAY                        ; Y = 0
8B1E  A5 0B              ..      LDA &0B
8B20  85 37              .7      STA &37                    ; Set &0037-0038 (temporary text pointer) to PtrA (beginning of line)
8B22  A5 0C              ..      LDA &0C
8B24  85 38              .8      STA &38
8B26  84 3B              .;      STY &3B
8B28  84 0A              ..      STY &0A                    ; Set PtrA offset to 0
8B2A  20 57 89            W.     JSR &8957
8B2D  20 DF 97            ..     JSR &97DF
8B30  90 06              ..      BCC &8B38
8B32  20 8D BC            ..     JSR &BC8D
8B35  4C F3 8A           L..     JMP &8AF3
.
8B38  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8B3B  C9 C6              ..      CMP #&C6                   ; 'AUTO' token
8B3D  B0 72              .r      BCS &8BB1                  ; Jump if token is equal or greater than &C6
8B3F  90 7E              .~      BCC &8BBF                  ; Jump if token is less than &C6
.
8B41  4C F6 8A           L..     JMP &8AF6
.
8B44  4C 04 85           L..     JMP &8504
.
8B47  BA                 .       TXS
8B48  E0 FC              ..      CPX #&FC
8B4A  B0 0D              ..      BCS &8B59
8B4C  AD FF 01           ...     LDA &01FF
8B4F  C9 A4              ..      CMP #&A4
8B51  D0 06              ..      BNE &8B59
8B53  20 1D 9B            ..     JSR &9B1D
8B56  4C 4C 98           LL.     JMP &984C
.
8B59  00                 .       ...
8B5A  07 4E              .N      RMB0 &4E
8B5C  6F 20              o       BBR6 &8B7E
8B5E  A4 00              ..      LDY &00
.
8B60  A4 0A              ..      LDY &0A
8B62  88                 .       DEY
8B63  B1 0B              ..      LDA (&0B),Y
8B65  C9 3D              .=      CMP #'='
8B67  F0 DE              ..      BEQ &8B47                  ; Jump if it's a variable declaration
8B69  C9 2A              .*      CMP #'*'
8B6B  F0 06              ..      BEQ &8B73                  ; Jump if it's an OSCLI (*) command
8B6D  C9 5B              .[      CMP #'['
8B6F  F0 D3              ..      BEQ &8B44                  ; Jump if it's an "goto assembler" command
8B71  D0 23              .#      BNE &8B96
.                                                           ; OSCLI handler
8B73  20 6D 98            m.     JSR &986D
8B76  A6 0B              ..      LDX &0B
8B78  A4 0C              ..      LDY &0C
8B7A  20 F7 FF            ..     JSR oscli
.
8B7D  A9 0D              ..      LDA #&0D
8B7F  A4 0A              ..      LDY &0A
8B81  88                 .       DEY
.
8B82  C8                 .       INY
8B83  D1 0B              ..      CMP (&0B),Y
8B85  D0 FB              ..      BNE &8B82
.
8B87  C9 8B              ..      CMP #&8B                   ; ELSE
8B89  F0 F2              ..      BEQ &8B7D
8B8B  A5 0C              ..      LDA &0C
8B8D  C9 07              ..      CMP #&07
8B8F  F0 B0              ..      BEQ &8B41
8B91  20 90 98            ..     JSR &9890
8B94  D0 0D              ..      BNE &8BA3
.
8B96  C6 0A              ..      DEC &0A
.
8B98  20 57 98            W.     JSR &9857                  ; Check end of statement
.                                                           ; Main execution loop
8B9B  A0 00              ..      LDY #&00
8B9D  B1 0B              ..      LDA (&0B),Y
8B9F  C9 3A              .:      CMP #&3A
8BA1  D0 E4              ..      BNE &8B87
.
8BA3  A4 0A              ..      LDY &0A
8BA5  E6 0A              ..      INC &0A
8BA7  B1 0B              ..      LDA (&0B),Y
8BA9  C9 20              .       CMP #' '
8BAB  F0 F6              ..      BEQ &8BA3                  ; Loop through the line until the first non-space character is reached
8BAD  C9 CF              ..      CMP #&CF
8BAF  90 0E              ..      BCC &8BBF                  ; Jump if token is only executable from the command line (&C6-&CE)
.
8BB1  AA                 .       TAX
8BB2  BD DF 82           ...     LDA &82DF,X                ; LSB of jump table for tokens &D0-&FF
8BB5  85 37              .7      STA &37
8BB7  BD 51 83           .Q.     LDA &8351,X                ; MSB of jump table for tokens &D0-&FF
8BBA  85 38              .8      STA &38
8BBC  6C 37 00           l7.     JMP (&0037)
.
8BBF  A6 0B              ..      LDX &0B
8BC1  86 19              ..      STX &19
8BC3  A6 0C              ..      LDX &0C
8BC5  86 1A              ..      STX &1A
8BC7  84 1B              ..      STY &1B
8BC9  20 DD 95            ..     JSR &95DD
8BCC  D0 1B              ..      BNE &8BE9
8BCE  B0 90              ..      BCS &8B60
8BD0  86 1B              ..      STX &1B
8BD2  20 41 98            A.     JSR &9841
8BD5  20 FC 94            ..     JSR &94FC
8BD8  A2 05              ..      LDX #&05
8BDA  E4 2C              .,      CPX &2C
8BDC  D0 01              ..      BNE &8BDF
8BDE  E8                 .       INX
.
8BDF  20 31 95            1.     JSR &9531
8BE2  C6 0A              ..      DEC &0A
8BE4  20 82 95            ..     JSR &9582
8BE7  F0 22              ."      BEQ &8C0B
.
8BE9  90 10              ..      BCC &8BFB
8BEB  20 94 BD            ..     JSR &BD94
8BEE  20 13 98            ..     JSR &9813
8BF1  A5 27              .'      LDA &27
8BF3  D0 19              ..      BNE &8C0E                  ; Generate "Type mismatch" error message
8BF5  20 1E 8C            ..     JSR &8C1E
8BF8  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
8BFB  20 94 BD            ..     JSR &BD94
8BFE  20 13 98            ..     JSR &9813
8C01  A5 27              .'      LDA &27
8C03  F0 09              ..      BEQ &8C0E                  ; Generate "Type mismatch" error message
8C05  20 B4 B4            ..     JSR &B4B4
8C08  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
8C0B  4C 2A 98           L*.     JMP &982A
.
8C0E  00                 .       ...
8C0F  06 54              .T      ASL &54
8C11  79 70 65           ype     ADC &6570,Y
8C14  20 6D 69            mi     JSR &696D
8C17  73                 s       ...
8C18  6D 61 74           mat     ADC &7461
8C1B  63                 c       ...
8C1C  68                 h       PLA
8C1D  00                 .       ...
.
8C1E  20 EA BD            ..     JSR &BDEA
.
8C21  A5 2C              .,      LDA &2C
8C23  C9 80              ..      CMP #&80
8C25  F0 7B              .{      BEQ &8CA2
8C27  A0 02              ..      LDY #&02
8C29  B1 2A              .*      LDA (&2A),Y
8C2B  C5 36              .6      CMP &36
8C2D  B0 55              .U      BCS &8C84
8C2F  A5 02              ..      LDA &02
8C31  85 2C              .,      STA &2C
8C33  A5 03              ..      LDA &03
8C35  85 2D              .-      STA &2D
8C37  A5 36              .6      LDA &36
8C39  C9 08              ..      CMP #&08
8C3B  90 06              ..      BCC &8C43
8C3D  69 07              i.      ADC #&07
8C3F  90 02              ..      BCC &8C43
8C41  A9 FF              ..      LDA #&FF
.
8C43  18                 .       CLC
8C44  48                 H       PHA
8C45  AA                 .       TAX
8C46  B1 2A              .*      LDA (&2A),Y
8C48  A0 00              ..      LDY #&00
8C4A  71 2A              q*      ADC (&2A),Y
8C4C  45 02              E.      EOR &02
8C4E  D0 0F              ..      BNE &8C5F
8C50  C8                 .       INY
8C51  71 2A              q*      ADC (&2A),Y
8C53  45 03              E.      EOR &03
8C55  D0 08              ..      BNE &8C5F
8C57  85 2D              .-      STA &2D
8C59  8A                 .       TXA
8C5A  C8                 .       INY
8C5B  38                 8       SEC
8C5C  F1 2A              .*      SBC (&2A),Y
8C5E  AA                 .       TAX
.
8C5F  8A                 .       TXA
8C60  18                 .       CLC
8C61  65 02              e.      ADC &02
8C63  A8                 .       TAY
8C64  A5 03              ..      LDA &03
8C66  69 00              i.      ADC #&00
8C68  C4 04              ..      CPY &04
8C6A  AA                 .       TAX
8C6B  E5 05              ..      SBC &05
8C6D  B0 48              .H      BCS &8CB7                  ; Generate "No room" error message
8C6F  84 02              ..      STY &02
8C71  86 03              ..      STX &03
8C73  68                 h       PLA
8C74  A0 02              ..      LDY #&02
8C76  91 2A              .*      STA (&2A),Y
8C78  88                 .       DEY
8C79  A5 2D              .-      LDA &2D
8C7B  F0 07              ..      BEQ &8C84
8C7D  91 2A              .*      STA (&2A),Y
8C7F  88                 .       DEY
8C80  A5 2C              .,      LDA &2C
8C82  91 2A              .*      STA (&2A),Y
.
8C84  A0 03              ..      LDY #&03
8C86  A5 36              .6      LDA &36
8C88  91 2A              .*      STA (&2A),Y
8C8A  F0 15              ..      BEQ &8CA1
8C8C  88                 .       DEY
8C8D  88                 .       DEY
8C8E  B1 2A              .*      LDA (&2A),Y
8C90  85 2D              .-      STA &2D
8C92  88                 .       DEY
8C93  B1 2A              .*      LDA (&2A),Y
8C95  85 2C              .,      STA &2C
.
8C97  B9 00 06           ...     LDA &0600,Y
8C9A  91 2C              .,      STA (&2C),Y
8C9C  C8                 .       INY
8C9D  C4 36              .6      CPY &36
8C9F  D0 F6              ..      BNE &8C97
.
8CA1  60                 `       RTS
.
8CA2  20 BA BE            ..     JSR &BEBA
8CA5  C0 00              ..      CPY #&00
8CA7  F0 0B              ..      BEQ &8CB4
.
8CA9  B9 00 06           ...     LDA &0600,Y
8CAC  91 2A              .*      STA (&2A),Y
8CAE  88                 .       DEY
8CAF  D0 F8              ..      BNE &8CA9
8CB1  AD 00 06           ...     LDA &0600
.
8CB4  91 2A              .*      STA (&2A),Y
8CB6  60                 `       RTS
.
8CB7  00                 .       ...
8CB8  00                 .       ...
8CB9  4E 6F 20           No      LSR &206F
8CBC  72 6F              ro      ADC (&6F)
8CBE  6F 6D              om      BBR6 &8D2D
8CC0  00                 .       ...
.
8CC1  A5 39              .9      LDA &39
8CC3  C9 80              ..      CMP #&80
8CC5  F0 27              .'      BEQ &8CEE
8CC7  90 3A              .:      BCC &8D03
8CC9  A0 00              ..      LDY #&00
8CCB  B1 04              ..      LDA (&04),Y
8CCD  AA                 .       TAX
8CCE  F0 15              ..      BEQ &8CE5
8CD0  B1 37              .7      LDA (&37),Y
8CD2  E9 01              ..      SBC #&01
8CD4  85 39              .9      STA &39
8CD6  C8                 .       INY
8CD7  B1 37              .7      LDA (&37),Y
8CD9  E9 00              ..      SBC #&00
8CDB  85 3A              .:      STA &3A
.
8CDD  B1 04              ..      LDA (&04),Y
8CDF  91 39              .9      STA (&39),Y
8CE1  C8                 .       INY
8CE2  CA                 .       DEX
8CE3  D0 F8              ..      BNE &8CDD
.
8CE5  A1 04              ..      LDA (&04,X)
8CE7  A0 03              ..      LDY #&03
.
8CE9  91 37              .7      STA (&37),Y
8CEB  4C DC BD           L..     JMP &BDDC
.
8CEE  A0 00              ..      LDY #&00
8CF0  B1 04              ..      LDA (&04),Y
8CF2  AA                 .       TAX
8CF3  F0 0A              ..      BEQ &8CFF
.
8CF5  C8                 .       INY
8CF6  B1 04              ..      LDA (&04),Y
8CF8  88                 .       DEY
8CF9  91 37              .7      STA (&37),Y
8CFB  C8                 .       INY
8CFC  CA                 .       DEX
8CFD  D0 F6              ..      BNE &8CF5
.
8CFF  A9 0D              ..      LDA #&0D
8D01  D0 E6              ..      BNE &8CE9
.
8D03  A0 00              ..      LDY #&00
8D05  B1 04              ..      LDA (&04),Y
8D07  91 37              .7      STA (&37),Y
8D09  C8                 .       INY
8D0A  C4 39              .9      CPY &39
8D0C  B0 18              ..      BCS &8D26
8D0E  B1 04              ..      LDA (&04),Y
8D10  91 37              .7      STA (&37),Y
8D12  C8                 .       INY
8D13  B1 04              ..      LDA (&04),Y
8D15  91 37              .7      STA (&37),Y
8D17  C8                 .       INY
8D18  B1 04              ..      LDA (&04),Y
8D1A  91 37              .7      STA (&37),Y
8D1C  C8                 .       INY
8D1D  C4 39              .9      CPY &39
8D1F  B0 05              ..      BCS &8D26
8D21  B1 04              ..      LDA (&04),Y
8D23  91 37              .7      STA (&37),Y
8D25  C8                 .       INY
.
8D26  98                 .       TYA
8D27  18                 .       CLC
8D28  4C E1 BD           L..     JMP &BDE1
.
8D2B  C6 0A              ..      DEC &0A
.
8D2D  20 A9 BF            ..     JSR &BFA9
.
8D30  98                 .       TYA
8D31  48                 H       PHA
8D32  20 8C 8A            ..     JSR &8A8C
8D35  C9 2C              .,      CMP #&2C
8D37  D0 3E              .>      BNE &8D77
8D39  20 29 9B            ).     JSR &9B29
8D3C  20 85 A3            ..     JSR &A385
8D3F  68                 h       PLA
8D40  A8                 .       TAY
8D41  A5 27              .'      LDA &27
8D43  20 D4 FF            ..     JSR osbput
8D46  AA                 .       TAX
8D47  F0 1B              ..      BEQ &8D64
8D49  30 0C              0.      BMI &8D57
8D4B  A2 03              ..      LDX #&03
.
8D4D  B5 2A              .*      LDA &2A,X
8D4F  20 D4 FF            ..     JSR osbput
8D52  CA                 .       DEX
8D53  10 F8              ..      BPL &8D4D
8D55  30 D9              0.      BMI &8D30
.
8D57  A2 04              ..      LDX #&04
.
8D59  BD 6C 04           .l.     LDA &046C,X
8D5C  20 D4 FF            ..     JSR osbput
8D5F  CA                 .       DEX
8D60  10 F7              ..      BPL &8D59
8D62  30 CC              0.      BMI &8D30
.
8D64  A5 36              .6      LDA &36
8D66  20 D4 FF            ..     JSR osbput
8D69  AA                 .       TAX
8D6A  F0 C4              ..      BEQ &8D30
.
8D6C  BD FF 05           ...     LDA &05FF,X
8D6F  20 D4 FF            ..     JSR osbput
8D72  CA                 .       DEX
8D73  D0 F7              ..      BNE &8D6C
8D75  F0 B9              ..      BEQ &8D30
.
8D77  68                 h       PLA
8D78  84 0A              ..      STY &0A
8D7A  4C 98 8B           L..     JMP &8B98
.
8D7D  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
.
8D80  4C 96 8B           L..     JMP &8B96
.
8D83  A9 00              ..      LDA #&00
8D85  85 14              ..      STA &14
8D87  85 15              ..      STA &15
8D89  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8D8C  C9 3A              .:      CMP #':'
8D8E  F0 F0              ..      BEQ &8D80
8D90  C9 0D              ..      CMP #&0D
8D92  F0 EC              ..      BEQ &8D80
8D94  C9 8B              ..      CMP #&8B                   ; 'ELSE' token
8D96  F0 E8              ..      BEQ &8D80
8D98  D0 38              .8      BNE &8DD2
8D9A  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8D9D  C9 23              .#      CMP #&23
8D9F  F0 8A              ..      BEQ &8D2B
8DA1  C6 0A              ..      DEC &0A
8DA3  4C BB 8D           L..     JMP &8DBB
.
8DA6  AD 00 04           ...     LDA &0400
8DA9  F0 10              ..      BEQ &8DBB
8DAB  A5 1E              ..      LDA &1E
.
8DAD  F0 0C              ..      BEQ &8DBB
8DAF  ED 00 04           ...     SBC &0400
8DB2  B0 F9              ..      BCS &8DAD
8DB4  A8                 .       TAY
.
8DB5  20 65 B5            e.     JSR &B565
8DB8  C8                 .       INY
8DB9  D0 FA              ..      BNE &8DB5
.
8DBB  18                 .       CLC
8DBC  AD 00 04           ...     LDA &0400
8DBF  85 14              ..      STA &14
.
8DC1  66 15              f.      ROR &15
.
8DC3  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8DC6  C9 3A              .:      CMP #&3A
8DC8  F0 B3              ..      BEQ &8D7D
8DCA  C9 0D              ..      CMP #&0D
8DCC  F0 AF              ..      BEQ &8D7D
8DCE  C9 8B              ..      CMP #&8B
8DD0  F0 AB              ..      BEQ &8D7D
.
8DD2  C9 7E              .~      CMP #'~'
8DD4  F0 EB              ..      BEQ &8DC1
8DD6  C9 2C              .,      CMP #','
8DD8  F0 CC              ..      BEQ &8DA6
8DDA  C9 3B              .;      CMP #';'
8DDC  F0 A5              ..      BEQ &8D83
8DDE  20 70 8E            p.     JSR &8E70
8DE1  90 E0              ..      BCC &8DC3
8DE3  A5 14              ..      LDA &14
8DE5  48                 H       PHA
8DE6  A5 15              ..      LDA &15
8DE8  48                 H       PHA
8DE9  C6 1B              ..      DEC &1B
8DEB  20 29 9B            ).     JSR &9B29
8DEE  68                 h       PLA
8DEF  85 15              ..      STA &15
8DF1  68                 h       PLA
8DF2  85 14              ..      STA &14
8DF4  A5 1B              ..      LDA &1B
8DF6  85 0A              ..      STA &0A
8DF8  98                 .       TYA
8DF9  F0 13              ..      BEQ &8E0E
8DFB  20 DF 9E            ..     JSR &9EDF
8DFE  A5 14              ..      LDA &14
8E00  38                 8       SEC
8E01  E5 36              .6      SBC &36
8E03  90 09              ..      BCC &8E0E
8E05  F0 07              ..      BEQ &8E0E
8E07  A8                 .       TAY
.
8E08  20 65 B5            e.     JSR &B565
8E0B  88                 .       DEY
8E0C  D0 FA              ..      BNE &8E08
.
8E0E  A5 36              .6      LDA &36
8E10  F0 B1              ..      BEQ &8DC3
8E12  A0 00              ..      LDY #&00
.
8E14  B9 00 06           ...     LDA &0600,Y
8E17  20 58 B5            X.     JSR &B558
8E1A  C8                 .       INY
8E1B  C4 36              .6      CPY &36
8E1D  D0 F5              ..      BNE &8E14
8E1F  F0 A2              ..      BEQ &8DC3
.
8E21  4C A2 8A           L..     JMP &8AA2
.
8E24  C9 2C              .,      CMP #&2C
8E26  D0 F9              ..      BNE &8E21
8E28  A5 2A              .*      LDA &2A
8E2A  48                 H       PHA
8E2B  20 56 AE            V.     JSR &AE56
8E2E  20 F0 92            ..     JSR &92F0
8E31  A9 1F              ..      LDA #&1F                   ; Move text cursor to X, Y
8E33  20 EE FF            ..     JSR oswrch
8E36  68                 h       PLA
8E37  20 EE FF            ..     JSR oswrch
8E3A  20 56 94            V.     JSR &9456
8E3D  4C 6A 8E           Lj.     JMP &8E6A
.
8E40  20 DD 92            ..     JSR &92DD
8E43  20 8C 8A            ..     JSR &8A8C
8E46  C9 29              .)      CMP #&29
8E48  D0 DA              ..      BNE &8E24
8E4A  A5 2A              .*      LDA &2A
8E4C  E5 1E              ..      SBC &1E
8E4E  F0 1A              ..      BEQ &8E6A
8E50  A8                 .       TAY
8E51  B0 0C              ..      BCS &8E5F
8E53  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
8E56  F0 03              ..      BEQ &8E5B
.
8E58  20 E3 92            ..     JSR &92E3
.
8E5B  A4 2A              .*      LDY &2A
8E5D  F0 0B              ..      BEQ &8E6A
.
8E5F  20 65 B5            e.     JSR &B565
8E62  88                 .       DEY
8E63  D0 FA              ..      BNE &8E5F
8E65  F0 03              ..      BEQ &8E6A
.
8E67  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
.
8E6A  18                 .       CLC
8E6B  A4 1B              ..      LDY &1B
8E6D  84 0A              ..      STY &0A
8E6F  60                 `       RTS
.
8E70  A6 0B              ..      LDX &0B
8E72  86 19              ..      STX &19
8E74  A6 0C              ..      LDX &0C
8E76  86 1A              ..      STX &1A
8E78  A6 0A              ..      LDX &0A
8E7A  86 1B              ..      STX &1B
8E7C  C9 27              .'      CMP #&27
8E7E  F0 E7              ..      BEQ &8E67
8E80  C9 8A              ..      CMP #&8A
8E82  F0 BC              ..      BEQ &8E40
8E84  C9 89              ..      CMP #&89
8E86  F0 D0              ..      BEQ &8E58
8E88  38                 8       SEC
.
8E89  60                 `       RTS
.
8E8A  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8E8D  20 70 8E            p.     JSR &8E70
8E90  90 F7              ..      BCC &8E89
8E92  C9 22              ."      CMP #'"'
8E94  F0 11              ..      BEQ &8EA7
8E96  38                 8       SEC
8E97  60                 `       RTS

.                                                           ; Error &09: Missing "
8E98  00                 .       ...
8E99  09 4D              .M      ORA #&4D
8E9B  69 73              is      ADC #&73
8E9D  73                 s       ...
8E9E  69 6E              in      ADC #&6E
8EA0  67 20              g       RMB6 &20
8EA2  22                 "       ...
8EA3  00                 .       ...
.
8EA4  20 58 B5            X.     JSR &B558
.
8EA7  C8                 .       INY
8EA8  B1 19              ..      LDA (&19),Y
8EAA  C9 0D              ..      CMP #&0D
8EAC  F0 EA              ..      BEQ &8E98                  ; Error &09: Missing "
8EAE  C9 22              ."      CMP #'"'
8EB0  D0 F2              ..      BNE &8EA4
8EB2  C8                 .       INY
8EB3  84 1B              ..      STY &1B
8EB5  B1 19              ..      LDA (&19),Y
8EB7  C9 22              ."      CMP #&22
8EB9  D0 AF              ..      BNE &8E6A
8EBB  F0 E7              ..      BEQ &8EA4
8EBD  20 57 98            W.     JSR &9857                  ; Check end of statement
8EC0  A9 10              ..      LDA #&10                   ; Clear graphics area
8EC2  D0 08              ..      BNE &8ECC
8EC4  20 57 98            W.     JSR &9857                  ; Check end of statement
8EC7  20 28 BC            (.     JSR &BC28                  ; Reset COUNT variable to 0
8ECA  A9 0C              ..      LDA #&0C                   ; Clear text area
.
8ECC  20 EE FF            ..     JSR oswrch
8ECF  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
8ED2  20 1D 9B            ..     JSR &9B1D
8ED5  20 EE 92            ..     JSR &92EE
8ED8  20 94 BD            ..     JSR &BD94
8EDB  A0 00              ..      LDY #&00
8EDD  8C 00 06           ...     STY &0600
.
8EE0  8C FF 06           ...     STY &06FF
8EE3  20 8C 8A            ..     JSR &8A8C
8EE6  C9 2C              .,      CMP #&2C
8EE8  D0 22              ."      BNE &8F0C
8EEA  A4 1B              ..      LDY &1B
8EEC  20 D5 95            ..     JSR &95D5
8EEF  F0 2A              .*      BEQ &8F1B
8EF1  AC FF 06           ...     LDY &06FF
8EF4  C8                 .       INY
8EF5  A5 2A              .*      LDA &2A
8EF7  99 00 06           ...     STA &0600,Y
8EFA  C8                 .       INY
8EFB  A5 2B              .+      LDA &2B
8EFD  99 00 06           ...     STA &0600,Y
8F00  C8                 .       INY
8F01  A5 2C              .,      LDA &2C
8F03  99 00 06           ...     STA &0600,Y
8F06  EE 00 06           ...     INC &0600
8F09  4C E0 8E           L..     JMP &8EE0
.
8F0C  C6 1B              ..      DEC &1B
8F0E  20 52 98            R.     JSR &9852
8F11  20 EA BD            ..     JSR &BDEA
8F14  20 1E 8F            ..     JSR &8F1E
8F17  D8                 .       CLD
8F18  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
8F1B  4C 43 AE           LC.     JMP &AE43
.
8F1E  AD 0C 04           ...     LDA &040C
8F21  4A                 J       LSR A
8F22  AD 04 04           ...     LDA &0404
8F25  AE 60 04           .`.     LDX &0460
8F28  AC 64 04           .d.     LDY &0464
8F2B  6C 2A 00           l*.     JMP (&002A)
.
8F2E  4C 2A 98           L*.     JMP &982A
8F31  20 DF 97            ..     JSR &97DF
8F34  90 F8              ..      BCC &8F2E
8F36  20 94 BD            ..     JSR &BD94
8F39  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8F3C  C9 2C              .,      CMP #','
8F3E  D0 EE              ..      BNE &8F2E
8F40  20 DF 97            ..     JSR &97DF
8F43  90 E9              ..      BCC &8F2E
8F45  20 57 98            W.     JSR &9857                  ; Check end of statement
8F48  A5 2A              .*      LDA &2A
8F4A  85 39              .9      STA &39
8F4C  A5 2B              .+      LDA &2B
8F4E  85 3A              .:      STA &3A
8F50  20 EA BD            ..     JSR &BDEA
.
8F53  20 2D BC            -.     JSR &BC2D
8F56  20 7B 98            {.     JSR &987B
8F59  20 22 92            ".     JSR &9222
8F5C  A5 39              .9      LDA &39
8F5E  C5 2A              .*      CMP &2A
8F60  A5 3A              .:      LDA &3A
8F62  E5 2B              .+      SBC &2B
8F64  B0 ED              ..      BCS &8F53
8F66  4C F3 8A           L..     JMP &8AF3
.
8F69  A9 0A              ..      LDA #&0A
8F6B  20 D8 AE            ..     JSR &AED8
8F6E  20 DF 97            ..     JSR &97DF
8F71  20 94 BD            ..     JSR &BD94
8F74  A9 0A              ..      LDA #&0A
8F76  20 D8 AE            ..     JSR &AED8
8F79  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
8F7C  C9 2C              .,      CMP #','
8F7E  D0 0D              ..      BNE &8F8D
8F80  20 DF 97            ..     JSR &97DF
8F83  A5 2B              .+      LDA &2B
8F85  D0 58              .X      BNE &8FDF
8F87  A5 2A              .*      LDA &2A
8F89  F0 54              .T      BEQ &8FDF
8F8B  E6 0A              ..      INC &0A
.
8F8D  C6 0A              ..      DEC &0A
8F8F  4C 57 98           LW.     JMP &9857                  ; Check end of statement
.
8F92  A5 12              ..      LDA &12
8F94  85 3B              .;      STA &3B
8F96  A5 13              ..      LDA &13
8F98  85 3C              .<      STA &3C
.
8F9A  A5 18              ..      LDA page
8F9C  85 38              .8      STA &38                    ; Set MSB of RENUMBER pointer to PAGE
8F9E  A9 01              ..      LDA #&01
8FA0  85 37              .7      STA &37                    ; Set LSB of RENUMBER pointer
8FA2  60                 `       RTS
8FA3  20 69 8F            i.     JSR &8F69
8FA6  A2 39              .9      LDX #&39
8FA8  20 0D BE            ..     JSR &BE0D
8FAB  20 6F BE            o.     JSR &BE6F
8FAE  20 92 8F            ..     JSR &8F92
.
8FB1  A0 00              ..      LDY #&00
8FB3  B1 37              .7      LDA (&37),Y
8FB5  30 30              00      BMI &8FE7
8FB7  91 3B              .;      STA (&3B),Y
8FB9  C8                 .       INY
8FBA  B1 37              .7      LDA (&37),Y
8FBC  91 3B              .;      STA (&3B),Y
8FBE  38                 8       SEC
8FBF  98                 .       TYA
8FC0  65 3B              e;      ADC &3B
8FC2  85 3B              .;      STA &3B
8FC4  AA                 .       TAX
8FC5  A5 3C              .<      LDA &3C
8FC7  69 00              i.      ADC #&00
8FC9  85 3C              .<      STA &3C
8FCB  E4 06              ..      CPX &06
8FCD  E5 07              ..      SBC &07
8FCF  B0 05              ..      BCS &8FD6
8FD1  20 9F 90            ..     JSR &909F
8FD4  90 DB              ..      BCC &8FB1
.
8FD6  00                 .       ...
8FD7  00                 .       ...
8FD8  CC 20 73           . s     CPY &7320
8FDB  70 61              pa      BCS &903E
8FDD  63                 c       ...
8FDE  65 00              e.      ADC &00
8FE0  00                 .       ...
8FE1  53                 S       ...
8FE2  69 6C              il      ADC #&6C
8FE4  6C 79 00           ly.     JMP (&0079)
.
8FE7  20 9A 8F            ..     JSR &8F9A
.
8FEA  A0 00              ..      LDY #&00
8FEC  B1 37              .7      LDA (&37),Y
8FEE  30 1D              0.      BMI &900D
8FF0  A5 3A              .:      LDA &3A
8FF2  91 37              .7      STA (&37),Y
8FF4  A5 39              .9      LDA &39
8FF6  C8                 .       INY
8FF7  91 37              .7      STA (&37),Y
8FF9  18                 .       CLC
8FFA  A5 2A              .*      LDA &2A
8FFC  65 39              e9      ADC &39
8FFE  85 39              .9      STA &39
9000  A9 00              ..      LDA #&00
9002  65 3A              e:      ADC &3A
9004  29 7F              )      AND #&7F
9006  85 3A              .:      STA &3A
9008  20 9F 90            ..     JSR &909F
900B  90 DD              ..      BCC &8FEA
.
900D  A5 18              ..      LDA page
900F  85 0C              ..      STA &0C
9011  A0 00              ..      LDY #&00
9013  84 0B              ..      STY &0B
9015  C8                 .       INY
9016  B1 0B              ..      LDA (&0B),Y
9018  30 20              0       BMI &903A
.
901A  A0 04              ..      LDY #&04
.
901C  B1 0B              ..      LDA (&0B),Y
901E  C9 8D              ..      CMP #&8D
9020  F0 1B              ..      BEQ &903D
9022  C8                 .       INY
9023  C9 0D              ..      CMP #&0D
9025  D0 F5              ..      BNE &901C
9027  B1 0B              ..      LDA (&0B),Y
9029  30 0F              0.      BMI &903A
902B  A0 03              ..      LDY #&03
902D  B1 0B              ..      LDA (&0B),Y
902F  18                 .       CLC
9030  65 0B              e.      ADC &0B
9032  85 0B              ..      STA &0B
9034  90 E4              ..      BCC &901A
9036  E6 0C              ..      INC &0C
9038  B0 E0              ..      BCS &901A
.
903A  4C F3 8A           L..     JMP &8AF3
.
903D  20 EB 97            ..     JSR &97EB
9040  20 92 8F            ..     JSR &8F92
.
9043  A0 00              ..      LDY #&00
9045  B1 37              .7      LDA (&37),Y
9047  30 37              07      BMI &9080
9049  B1 3B              .;      LDA (&3B),Y
904B  C8                 .       INY
904C  C5 2B              .+      CMP &2B
904E  D0 21              .!      BNE &9071
9050  B1 3B              .;      LDA (&3B),Y
9052  C5 2A              .*      CMP &2A
9054  D0 1B              ..      BNE &9071
9056  B1 37              .7      LDA (&37),Y
9058  85 3D              .=      STA &3D
905A  88                 .       DEY
905B  B1 37              .7      LDA (&37),Y
905D  85 3E              .>      STA &3E
905F  A4 0A              ..      LDY &0A
9061  88                 .       DEY
9062  A5 0B              ..      LDA &0B
9064  85 37              .7      STA &37
9066  A5 0C              ..      LDA &0C
9068  85 38              .8      STA &38
906A  20 F5 88            ..     JSR &88F5
.
906D  A4 0A              ..      LDY &0A
906F  D0 AB              ..      BNE &901C
.
9071  20 9F 90            ..     JSR &909F
9074  A5 3B              .;      LDA &3B
9076  69 02              i.      ADC #&02
9078  85 3B              .;      STA &3B
907A  90 C7              ..      BCC &9043
907C  E6 3C              .<      INC &3C
907E  B0 C3              ..      BCS &9043
.
9080  20 CF BF            ..     JSR &BFCF
9083  46 61              Fa      LSR &61
9085  69 6C              il      ADC #&6C
9087  65 64              ed      ADC &64
9089  20 61 74            at     JSR &7461
908C  20 C8 B1            ..     JSR &B1C8
908F  0B                 .       ...
9090  85 2B              .+      STA &2B
9092  C8                 .       INY
9093  B1 0B              ..      LDA (&0B),Y
9095  85 2A              .*      STA &2A
9097  20 1F 99            ..     JSR &991F
909A  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
909D  F0 CE              ..      BEQ &906D
.
909F  C8                 .       INY
90A0  B1 37              .7      LDA (&37),Y
90A2  65 37              e7      ADC &37
90A4  85 37              .7      STA &37
90A6  90 03              ..      BCC &90AB
90A8  E6 38              .8      INC &38
90AA  18                 .       CLC
.
90AB  60                 `       RTS
90AC  20 69 8F            i.     JSR &8F69
90AF  A5 2A              .*      LDA &2A
90B1  48                 H       PHA
90B2  20 EA BD            ..     JSR &BDEA
.
90B5  20 94 BD            ..     JSR &BD94
90B8  20 23 99            #.     JSR &9923
90BB  A9 20              .       LDA #' '
90BD  20 02 BC            ..     JSR &BC02
90C0  20 EA BD            ..     JSR &BDEA
90C3  20 51 89            Q.     JSR &8951
90C6  20 8D BC            ..     JSR &BC8D
90C9  20 20 BD             .     JSR &BD20
90CC  68                 h       PLA
90CD  48                 H       PHA
90CE  18                 .       CLC
90CF  65 2A              e*      ADC &2A
90D1  85 2A              .*      STA &2A
90D3  90 E0              ..      BCC &90B5
90D5  E6 2B              .+      INC &2B
90D7  10 DC              ..      BPL &90B5
90D9  4C F3 8A           L..     JMP &8AF3
.
90DC  4C 18 92           L..     JMP &9218                  ; Generate "DIM space" error message
.
90DF  C6 0A              ..      DEC &0A
90E1  20 82 95            ..     JSR &9582
90E4  F0 41              .A      BEQ &9127
90E6  B0 3F              .?      BCS &9127
90E8  20 94 BD            ..     JSR &BD94
90EB  20 DD 92            ..     JSR &92DD
90EE  20 22 92            ".     JSR &9222
90F1  A5 2D              .-      LDA &2D
90F3  05 2C              .,      ORA &2C
90F5  D0 30              .0      BNE &9127
90F7  18                 .       CLC
90F8  A5 2A              .*      LDA &2A
90FA  65 02              e.      ADC &02
90FC  A8                 .       TAY
90FD  A5 2B              .+      LDA &2B
90FF  65 03              e.      ADC &03
9101  AA                 .       TAX
9102  C4 04              ..      CPY &04
9104  E5 05              ..      SBC &05
9106  B0 D4              ..      BCS &90DC
9108  A5 02              ..      LDA &02
910A  85 2A              .*      STA &2A
910C  A5 03              ..      LDA &03
910E  85 2B              .+      STA &2B
9110  84 02              ..      STY &02
9112  86 03              ..      STX &03
9114  A9 00              ..      LDA #&00
9116  85 2C              .,      STA &2C
9118  85 2D              .-      STA &2D
911A  A9 40              .@      LDA #&40
911C  85 27              .'      STA &27
911E  20 B4 B4            ..     JSR &B4B4
9121  20 27 88            '.     JSR &8827
9124  4C 0B 92           L..     JMP &920B
.
9127  00                 .       ...
9128  0A                 .       ASL A
9129  42                 B       ...
912A  61 64              ad      ADC (&64,X)
912C  20 DE 00            ..     JSR &00DE
.
912F  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
9132  98                 .       TYA
9133  18                 .       CLC
9134  65 0B              e.      ADC &0B
9136  A6 0C              ..      LDX &0C
9138  90 02              ..      BCC &913C
913A  E8                 .       INX
913B  18                 .       CLC
.
913C  E9 00              ..      SBC #&00
913E  85 37              .7      STA &37
9140  8A                 .       TXA
9141  E9 00              ..      SBC #&00
9143  85 38              .8      STA &38
9145  A2 05              ..      LDX #&05
9147  86 3F              .?      STX &3F
9149  A6 0A              ..      LDX &0A
914B  20 59 95            Y.     JSR &9559
914E  C0 01              ..      CPY #&01
9150  F0 D5              ..      BEQ &9127
9152  C9 28              .(      CMP #&28
9154  F0 15              ..      BEQ &916B
9156  C9 24              .$      CMP #&24
9158  F0 04              ..      BEQ &915E
915A  C9 25              .%      CMP #&25
915C  D0 0A              ..      BNE &9168
.
915E  C6 3F              .?      DEC &3F
9160  C8                 .       INY
9161  E8                 .       INX
9162  B1 37              .7      LDA (&37),Y
9164  C9 28              .(      CMP #&28
9166  F0 03              ..      BEQ &916B
.
9168  4C DF 90           L..     JMP &90DF
.
916B  84 39              .9      STY &39
916D  86 0A              ..      STX &0A
916F  20 69 94            i.     JSR &9469
9172  D0 B3              ..      BNE &9127
9174  20 FC 94            ..     JSR &94FC
9177  A2 01              ..      LDX #&01
9179  20 31 95            1.     JSR &9531
917C  A5 3F              .?      LDA &3F
917E  48                 H       PHA
917F  A9 01              ..      LDA #&01
9181  48                 H       PHA
9182  20 D8 AE            ..     JSR &AED8
.
9185  20 94 BD            ..     JSR &BD94
9188  20 21 88            !.     JSR &8821
918B  A5 2B              .+      LDA &2B
918D  29 C0              ).      AND #&C0
918F  05 2C              .,      ORA &2C
9191  05 2D              .-      ORA &2D
9193  D0 92              ..      BNE &9127
9195  20 22 92            ".     JSR &9222
9198  68                 h       PLA
9199  A8                 .       TAY
919A  A5 2A              .*      LDA &2A
919C  91 02              ..      STA (&02),Y
919E  C8                 .       INY
919F  A5 2B              .+      LDA &2B
91A1  91 02              ..      STA (&02),Y
91A3  C8                 .       INY
91A4  98                 .       TYA
91A5  48                 H       PHA
91A6  20 31 92            1.     JSR &9231
91A9  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
91AC  C9 2C              .,      CMP #','
91AE  F0 D5              ..      BEQ &9185
91B0  C9 29              .)      CMP #')'
91B2  F0 03              ..      BEQ &91B7
91B4  4C 27 91           L'.     JMP &9127
.
91B7  68                 h       PLA
91B8  85 15              ..      STA &15
91BA  68                 h       PLA
91BB  85 3F              .?      STA &3F
91BD  A9 00              ..      LDA #&00
91BF  85 40              .@      STA &40
91C1  20 36 92            6.     JSR &9236
91C4  A0 00              ..      LDY #&00
91C6  A5 15              ..      LDA &15
91C8  91 02              ..      STA (&02),Y
91CA  65 2A              e*      ADC &2A
91CC  85 2A              .*      STA &2A
91CE  90 02              ..      BCC &91D2
91D0  E6 2B              .+      INC &2B
.
91D2  A5 03              ..      LDA &03
91D4  85 38              .8      STA &38
91D6  A5 02              ..      LDA &02
91D8  85 37              .7      STA &37
91DA  18                 .       CLC
91DB  65 2A              e*      ADC &2A
91DD  A8                 .       TAY
91DE  A5 2B              .+      LDA &2B
91E0  65 03              e.      ADC &03
91E2  B0 34              .4      BCS &9218                  ; Generate "DIM space" error message
91E4  AA                 .       TAX
91E5  C4 04              ..      CPY &04
91E7  E5 05              ..      SBC &05
91E9  B0 2D              .-      BCS &9218                  ; Generate "DIM space" error message
91EB  84 02              ..      STY &02
91ED  86 03              ..      STX &03
91EF  A5 37              .7      LDA &37
91F1  65 15              e.      ADC &15
91F3  A8                 .       TAY
91F4  A9 00              ..      LDA #&00
91F6  85 37              .7      STA &37
91F8  90 02              ..      BCC &91FC
91FA  E6 38              .8      INC &38
.
91FC  91 37              .7      STA (&37),Y
91FE  C8                 .       INY
91FF  D0 02              ..      BNE &9203
9201  E6 38              .8      INC &38
.
9203  C4 02              ..      CPY &02
9205  D0 F5              ..      BNE &91FC
9207  E4 38              .8      CPX &38
9209  D0 F1              ..      BNE &91FC
.
920B  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
920E  C9 2C              .,      CMP #','
9210  F0 03              ..      BEQ &9215
9212  4C 96 8B           L..     JMP &8B96
.
9215  4C 2F 91           L/.     JMP &912F
.
9218  00                 .       ...
9219  0B                 .       ...
921A  DE 20 73           . s     DEC &7320,X
921D  70 61              pa      BCS &9280
921F  63                 c       ...
9220  65 00              e.      ADC &00
.
9222  E6 2A              .*      INC &2A
9224  D0 0A              ..      BNE &9230
9226  E6 2B              .+      INC &2B
9228  D0 06              ..      BNE &9230
922A  E6 2C              .,      INC &2C
922C  D0 02              ..      BNE &9230
922E  E6 2D              .-      INC &2D
.
9230  60                 `       RTS
.
9231  A2 3F              .?      LDX #&3F
9233  20 0D BE            ..     JSR &BE0D
.
9236  A2 00              ..      LDX #&00
9238  A0 00              ..      LDY #&00
.
923A  46 40              F@      LSR &40
923C  66 3F              f?      ROR &3F
923E  90 0B              ..      BCC &924B
9240  18                 .       CLC
9241  98                 .       TYA
9242  65 2A              e*      ADC &2A
9244  A8                 .       TAY
9245  8A                 .       TXA
9246  65 2B              e+      ADC &2B
9248  AA                 .       TAX
9249  B0 0F              ..      BCS &925A
.
924B  06 2A              .*      ASL &2A
924D  26 2B              &+      ROL &2B
924F  A5 3F              .?      LDA &3F
9251  05 40              .@      ORA &40
9253  D0 E5              ..      BNE &923A
9255  84 2A              .*      STY &2A
9257  86 2B              .+      STX &2B
9259  60                 `       RTS
.
925A  4C 27 91           L'.     JMP &9127
925D  20 EB 92            ..     JSR &92EB
9260  A5 2A              .*      LDA &2A
9262  85 06              ..      STA &06
9264  85 04              ..      STA &04
9266  A5 2B              .+      LDA &2B
9268  85 07              ..      STA &07
926A  85 05              ..      STA &05
926C  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
926F  20 EB 92            ..     JSR &92EB
9272  A5 2A              .*      LDA &2A
9274  85 00              ..      STA &00
9276  85 02              ..      STA &02
9278  A5 2B              .+      LDA &2B
927A  85 01              ..      STA &01
927C  85 03              ..      STA &03
927E  20 2F BD            /.     JSR &BD2F
9281  F0 07              ..      BEQ &928A
9283  20 EB 92            ..     JSR &92EB
9286  A5 2B              .+      LDA &2B
9288  85 18              ..      STA page
.
928A  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
928D  20 57 98            W.     JSR &9857                  ; Check end of statement
9290  20 20 BD             .     JSR &BD20
9293  F0 F5              ..      BEQ &928A
9295  20 DF 97            ..     JSR &97DF
9298  B0 0B              ..      BCS &92A5
929A  C9 EE              ..      CMP #&EE
929C  F0 19              ..      BEQ &92B7
929E  C9 87              ..      CMP #&87
92A0  F0 1E              ..      BEQ &92C0
92A2  20 21 88            !.     JSR &8821
.
92A5  20 57 98            W.     JSR &9857                  ; Check end of statement
92A8  A5 2A              .*      LDA &2A
92AA  85 21              .!      STA &21
92AC  A5 2B              .+      LDA &2B
.
92AE  85 22              ."      STA &22
92B0  A9 FF              ..      LDA #&FF
.
92B2  85 20              .       STA &20
92B4  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
92B7  E6 0A              ..      INC &0A
92B9  20 57 98            W.     JSR &9857                  ; Check end of statement
92BC  A9 FF              ..      LDA #&FF
92BE  D0 EE              ..      BNE &92AE
.
92C0  E6 0A              ..      INC &0A
92C2  20 57 98            W.     JSR &9857                  ; Check end of statement
92C5  A9 00              ..      LDA #&00
92C7  F0 E9              ..      BEQ &92B2
92C9  20 EB 92            ..     JSR &92EB
92CC  A2 2A              .*      LDX #&2A
92CE  A0 00              ..      LDY #&00
92D0  84 2E              ..      STY &2E
92D2  A9 02              ..      LDA #&02                   ; OSWORD &02: Write system clock
92D4  20 F1 FF            ..     JSR osword
92D7  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
92DA  20 AE 8A            ..     JSR &8AAE
.
92DD  20 29 9B            ).     JSR &9B29
92E0  4C F0 92           L..     JMP &92F0
.
92E3  20 EC AD            ..     JSR &ADEC
92E6  F0 0F              ..      BEQ &92F7
92E8  30 0A              0.      BMI &92F4
.
92EA  60                 `       RTS
.
92EB  20 07 98            ..     JSR &9807
.
92EE  A5 27              .'      LDA &27
.
92F0  F0 05              ..      BEQ &92F7
92F2  10 F6              ..      BPL &92EA
.
92F4  4C E4 A3           L..     JMP &A3E4
.
92F7  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
92FA  20 EC AD            ..     JSR &ADEC
.
92FD  F0 F8              ..      BEQ &92F7
92FF  30 E9              0.      BMI &92EA
9301  4C BE A2           L..     JMP &A2BE
9304  A5 0B              ..      LDA &0B
9306  85 19              ..      STA &19
9308  A5 0C              ..      LDA &0C
930A  85 1A              ..      STA &1A
930C  A5 0A              ..      LDA &0A
930E  85 1B              ..      STA &1B
9310  A9 F2              ..      LDA #&F2
9312  20 97 B1            ..     JSR &B197
9315  20 52 98            R.     JSR &9852
9318  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
931B  A0 03              ..      LDY #&03
931D  A9 00              ..      LDA #&00
931F  91 2A              .*      STA (&2A),Y
9321  F0 1E              ..      BEQ &9341
.
9323  BA                 .       TXS
9324  E0 FC              ..      CPX #&FC
9326  B0 43              .C      BCS &936B
9328  20 82 95            ..     JSR &9582
932B  F0 26              .&      BEQ &9353
932D  20 0D B3            ..     JSR &B30D
9330  A4 2C              .,      LDY &2C
9332  30 E7              0.      BMI &931B
9334  20 94 BD            ..     JSR &BD94
9337  A9 00              ..      LDA #&00
9339  20 D8 AE            ..     JSR &AED8
933C  85 27              .'      STA &27
933E  20 B4 B4            ..     JSR &B4B4
.
9341  BA                 .       TXS
9342  FE 06 01           ...     INC &0106,X
9345  A4 1B              ..      LDY &1B
9347  84 0A              ..      STY &0A
9349  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
934C  C9 2C              .,      CMP #','
934E  F0 D3              ..      BEQ &9323
9350  4C 96 8B           L..     JMP &8B96
.
9353  4C 98 8B           L..     JMP &8B98
9356  BA                 .       TXS
9357  E0 FC              ..      CPX #&FC
9359  B0 0A              ..      BCS &9365
935B  AD FF 01           ...     LDA &01FF
935E  C9 F2              ..      CMP #&F2
9360  D0 03              ..      BNE &9365
9362  4C 57 98           LW.     JMP &9857                  ; Check end of statement

.                                                           ; Error &0D: Not LOCAL
9365  00                 .       ...
9366  0D 4E 6F           .No     ORA &6F4E
9369  20 F2 00            ..     JSR &00F2
936C  0C 4E 6F           .No     TSB &6F4E
936F  74 20              t       STZ &20,X
9371  EA                 .       NOP

.                                                           ; Error &19: Bad MODE
9372  00                 .       ...
9373  19 42 61           .Ba     ORA &6142,Y
9376  64 20              d       STZ &20
9378  EB                 .       ...
9379  00                 .       ...

937A  20 21 88            !.     JSR &8821
937D  A5 2A              .*      LDA &2A
937F  48                 H       PHA
9380  20 DA 92            ..     JSR &92DA
9383  20 52 98            R.     JSR &9852
9386  A9 12              ..      LDA #&12                   ; Define graphics colour
9388  20 EE FF            ..     JSR oswrch
938B  4C DA 93           L..     JMP &93DA
938E  A9 11              ..      LDA #&11
9390  48                 H       PHA
9391  20 21 88            !.     JSR &8821
9394  20 57 98            W.     JSR &9857                  ; Check end of statement
9397  4C DA 93           L..     JMP &93DA
939A  A9 16              ..      LDA #&16
939C  48                 H       PHA
939D  20 21 88            !.     JSR &8821
93A0  20 57 98            W.     JSR &9857                  ; Check end of statement
93A3  20 E7 BE            ..     JSR &BEE7
93A6  E0 FF              ..      CPX #&FF
93A8  D0 2D              .-      BNE &93D7
93AA  C0 FF              ..      CPY #&FF
93AC  D0 29              .)      BNE &93D7
93AE  A5 04              ..      LDA &04
93B0  C5 06              ..      CMP &06
93B2  D0 BE              ..      BNE &9372
93B4  A5 05              ..      LDA &05
93B6  C5 07              ..      CMP &07
93B8  D0 B8              ..      BNE &9372
93BA  A6 2A              .*      LDX &2A
93BC  A9 85              ..      LDA #&85                   ; OSBYTE &85: Read bottom of display RAM address for a specified mode
93BE  20 F4 FF            ..     JSR osbyte
93C1  E4 02              ..      CPX &02
93C3  98                 .       TYA
93C4  E5 03              ..      SBC &03
93C6  90 AA              ..      BCC &9372
93C8  E4 12              ..      CPX &12
93CA  98                 .       TYA
93CB  E5 13              ..      SBC &13
93CD  90 A3              ..      BCC &9372
93CF  86 06              ..      STX &06
93D1  86 04              ..      STX &04
93D3  84 07              ..      STY &07
93D5  84 05              ..      STY &05
.
93D7  20 28 BC            (.     JSR &BC28                  ; Reset COUNT variable to 0
.
93DA  68                 h       PLA
93DB  20 EE FF            ..     JSR oswrch
93DE  20 56 94            V.     JSR &9456
93E1  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
93E4  A9 04              ..      LDA #&04
93E6  D0 02              ..      BNE &93EA
93E8  A9 05              ..      LDA #&05
.
93EA  48                 H       PHA
93EB  20 1D 9B            ..     JSR &9B1D
93EE  4C FD 93           L..     JMP &93FD
93F1  20 21 88            !.     JSR &8821
93F4  A5 2A              .*      LDA &2A
93F6  48                 H       PHA
93F7  20 AE 8A            ..     JSR &8AAE
93FA  20 29 9B            ).     JSR &9B29
.
93FD  20 EE 92            ..     JSR &92EE
9400  20 94 BD            ..     JSR &BD94
9403  20 DA 92            ..     JSR &92DA
9406  20 52 98            R.     JSR &9852
9409  A9 19              ..      LDA #&19                   ; PLOT K, X, Y
940B  20 EE FF            ..     JSR oswrch
940E  68                 h       PLA
940F  20 EE FF            ..     JSR oswrch
9412  20 0B BE            ..     JSR &BE0B
9415  A5 37              .7      LDA &37
9417  20 EE FF            ..     JSR oswrch
941A  A5 38              .8      LDA &38
941C  20 EE FF            ..     JSR oswrch
941F  20 56 94            V.     JSR &9456
9422  A5 2B              .+      LDA &2B
9424  20 EE FF            ..     JSR oswrch
9427  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
942A  A5 2B              .+      LDA &2B
942C  20 EE FF            ..     JSR oswrch
.
942F  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
.
9432  C9 3A              .:      CMP #':'
9434  F0 1D              ..      BEQ &9453
9436  C9 0D              ..      CMP #&0D
9438  F0 19              ..      BEQ &9453
943A  C9 8B              ..      CMP #&8B                   ; 'ELSE' token
943C  F0 15              ..      BEQ &9453
943E  C6 0A              ..      DEC &0A
9440  20 21 88            !.     JSR &8821
9443  20 56 94            V.     JSR &9456
9446  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
9449  C9 2C              .,      CMP #','
944B  F0 E2              ..      BEQ &942F
944D  C9 3B              .;      CMP #';'
944F  D0 E1              ..      BNE &9432
9451  F0 D7              ..      BEQ &942A
.
9453  4C 96 8B           L..     JMP &8B96
.
9456  A5 2A              .*      LDA &2A
9458  6C 0E 02           l..     JMP (wrchv)

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
945B  A0 01              ..      LDY #&01
945D  B1 37              .7      LDA (&37),Y
945F  A0 F6              ..      LDY #&F6
9461  C9 F2              ..      CMP #&F2
9463  F0 0A              ..      BEQ &946F
9465  A0 F8              ..      LDY #&F8
9467  D0 06              ..      BNE &946F
.
9469  A0 01              ..      LDY #&01
946B  B1 37              .7      LDA (&37),Y
946D  0A                 .       ASL A
946E  A8                 .       TAY
.
946F  B9 00 04           ...     LDA &0400,Y
9472  85 3A              .:      STA &3A
9474  B9 01 04           ...     LDA &0401,Y
9477  85 3B              .;      STA &3B
.
9479  A5 3B              .;      LDA &3B
947B  F0 35              .5      BEQ &94B2
947D  A0 00              ..      LDY #&00
947F  B1 3A              .:      LDA (&3A),Y
9481  85 3C              .<      STA &3C
9483  C8                 .       INY
9484  B1 3A              .:      LDA (&3A),Y
9486  85 3D              .=      STA &3D
9488  C8                 .       INY
9489  B1 3A              .:      LDA (&3A),Y
948B  D0 0D              ..      BNE &949A
948D  88                 .       DEY
948E  C4 39              .9      CPY &39
9490  D0 21              .!      BNE &94B3
9492  C8                 .       INY
9493  B0 12              ..      BCS &94A7
.
9495  C8                 .       INY
9496  B1 3A              .:      LDA (&3A),Y
9498  F0 19              ..      BEQ &94B3
.
949A  D1 37              .7      CMP (&37),Y
949C  D0 15              ..      BNE &94B3
949E  C4 39              .9      CPY &39
94A0  D0 F3              ..      BNE &9495
94A2  C8                 .       INY
94A3  B1 3A              .:      LDA (&3A),Y
94A5  D0 0C              ..      BNE &94B3
.
94A7  98                 .       TYA
94A8  65 3A              e:      ADC &3A
94AA  85 2A              .*      STA &2A
94AC  A5 3B              .;      LDA &3B
94AE  69 00              i.      ADC #&00
94B0  85 2B              .+      STA &2B
.
94B2  60                 `       RTS
.
94B3  A5 3D              .=      LDA &3D
94B5  F0 FB              ..      BEQ &94B2
94B7  A0 00              ..      LDY #&00
94B9  B1 3C              .<      LDA (&3C),Y
94BB  85 3A              .:      STA &3A
94BD  C8                 .       INY
94BE  B1 3C              .<      LDA (&3C),Y
94C0  85 3B              .;      STA &3B
94C2  C8                 .       INY
94C3  B1 3C              .<      LDA (&3C),Y
94C5  D0 0D              ..      BNE &94D4
94C7  88                 .       DEY
94C8  C4 39              .9      CPY &39
94CA  D0 AD              ..      BNE &9479
94CC  C8                 .       INY
94CD  B0 12              ..      BCS &94E1
.
94CF  C8                 .       INY
94D0  B1 3C              .<      LDA (&3C),Y
94D2  F0 A5              ..      BEQ &9479
.
94D4  D1 37              .7      CMP (&37),Y
94D6  D0 A1              ..      BNE &9479
94D8  C4 39              .9      CPY &39
94DA  D0 F3              ..      BNE &94CF
94DC  C8                 .       INY
94DD  B1 3C              .<      LDA (&3C),Y
94DF  D0 98              ..      BNE &9479
.
94E1  98                 .       TYA
94E2  65 3C              e<      ADC &3C
94E4  85 2A              .*      STA &2A
94E6  A5 3D              .=      LDA &3D
94E8  69 00              i.      ADC #&00
94EA  85 2B              .+      STA &2B
94EC  60                 `       RTS
.
94ED  A0 01              ..      LDY #&01
94EF  B1 37              .7      LDA (&37),Y
94F1  AA                 .       TAX
94F2  A9 F6              ..      LDA #&F6
94F4  E0 F2              ..      CPX #&F2
94F6  F0 09              ..      BEQ &9501
94F8  A9 F8              ..      LDA #&F8
94FA  D0 05              ..      BNE &9501
.
94FC  A0 01              ..      LDY #&01
94FE  B1 37              .7      LDA (&37),Y
9500  0A                 .       ASL A
.
9501  85 3A              .:      STA &3A
9503  A9 04              ..      LDA #&04
9505  85 3B              .;      STA &3B
.
9507  B1 3A              .:      LDA (&3A),Y
9509  F0 0B              ..      BEQ &9516
950B  AA                 .       TAX
950C  88                 .       DEY
950D  B1 3A              .:      LDA (&3A),Y
950F  85 3A              .:      STA &3A
9511  86 3B              .;      STX &3B
9513  C8                 .       INY
9514  10 F1              ..      BPL &9507
.
9516  A5 03              ..      LDA &03
9518  91 3A              .:      STA (&3A),Y
951A  A5 02              ..      LDA &02
951C  88                 .       DEY
951D  91 3A              .:      STA (&3A),Y
951F  98                 .       TYA
9520  C8                 .       INY
9521  91 02              ..      STA (&02),Y
9523  C4 39              .9      CPY &39
9525  F0 31              .1      BEQ &9558
.
9527  C8                 .       INY
9528  B1 37              .7      LDA (&37),Y
952A  91 02              ..      STA (&02),Y
952C  C4 39              .9      CPY &39
952E  D0 F7              ..      BNE &9527
9530  60                 `       RTS
.
9531  A9 00              ..      LDA #&00
.
9533  C8                 .       INY
9534  91 02              ..      STA (&02),Y
9536  CA                 .       DEX
9537  D0 FA              ..      BNE &9533
.
9539  38                 8       SEC
953A  98                 .       TYA
953B  65 02              e.      ADC &02
953D  90 02              ..      BCC &9541
953F  E6 03              ..      INC &03
.
9541  A4 03              ..      LDY &03
9543  C4 05              ..      CPY &05
9545  90 0F              ..      BCC &9556
9547  D0 04              ..      BNE &954D
9549  C5 04              ..      CMP &04
954B  90 09              ..      BCC &9556
.
954D  A9 00              ..      LDA #&00
954F  A0 01              ..      LDY #&01
9551  91 3A              .:      STA (&3A),Y
9553  4C B7 8C           L..     JMP &8CB7                  ; Generate "No room" error message
.
9556  85 02              ..      STA &02
.
9558  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
9559  A0 01              ..      LDY #&01
.
955B  B1 37              .7      LDA (&37),Y
955D  C9 30              .0      CMP #'0'
955F  90 18              ..      BCC &9579
9561  C9 40              .@      CMP #'@'
9563  B0 0C              ..      BCS &9571
9565  C9 3A              .:      CMP #':'
9567  B0 10              ..      BCS &9579
9569  C0 01              ..      CPY #&01
956B  F0 0C              ..      BEQ &9579
.
956D  E8                 .       INX
956E  C8                 .       INY
956F  D0 EA              ..      BNE &955B
.
9571  C9 5F              ._      CMP #'_'
9573  B0 05              ..      BCS &957A
9575  C9 5B              .[      CMP #'['
9577  90 F4              ..      BCC &956D
.
9579  60                 `       RTS
.
957A  C9 7B              .{      CMP #&7B
957C  90 EF              ..      BCC &956D
957E  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
957F  20 31 95            1.     JSR &9531
.
9582  20 C9 95            ..     JSR &95C9
9585  D0 1D              ..      BNE &95A4
9587  B0 1B              ..      BCS &95A4
9589  20 FC 94            ..     JSR &94FC
958C  A2 05              ..      LDX #&05
958E  E4 2C              .,      CPX &2C
9590  D0 ED              ..      BNE &957F
9592  E8                 .       INX
9593  D0 EA              ..      BNE &957F
.
9595  C9 21              .!      CMP #'!'
9597  F0 0C              ..      BEQ &95A5
9599  C9 24              .$      CMP #'$'
959B  F0 13              ..      BEQ &95B0
959D  49 3F              I?      EOR #&3F
959F  F0 06              ..      BEQ &95A7
95A1  A9 00              ..      LDA #&00
95A3  38                 8       SEC
.
95A4  60                 `       RTS
.
95A5  A9 04              ..      LDA #&04
.
95A7  48                 H       PHA
95A8  E6 1B              ..      INC &1B
95AA  20 E3 92            ..     JSR &92E3
95AD  4C 9F 96           L..     JMP &969F
.
95B0  E6 1B              ..      INC &1B
95B2  20 E3 92            ..     JSR &92E3
95B5  A5 2B              .+      LDA &2B
95B7  F0 06              ..      BEQ &95BF                  ; Generate "$ range" error message
95B9  A9 80              ..      LDA #&80
95BB  85 2C              .,      STA &2C
95BD  38                 8       SEC
95BE  60                 `       RTS

.
95BF  00                 .       ...
95C0  08                 .       PHP
95C1  24 20              $       BIT &20
95C3  72 61              ra      ADC (&61)
95C5  6E 67 65           nge     ROR &6567
95C8  00                 .       ...

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
.
95C9  A5 0B              ..      LDA &0B
95CB  85 19              ..      STA &19
95CD  A5 0C              ..      LDA &0C
95CF  85 1A              ..      STA &1A
95D1  A4 0A              ..      LDY &0A
95D3  88                 .       DEY
.
95D4  C8                 .       INY
.
95D5  84 1B              ..      STY &1B
95D7  B1 19              ..      LDA (&19),Y
95D9  C9 20              .       CMP #' '
95DB  F0 F7              ..      BEQ &95D4
.
95DD  C9 40              .@      CMP #'@'
95DF  90 B4              ..      BCC &9595
95E1  C9 5B              .[      CMP #'['
95E3  B0 1A              ..      BCS &95FF
95E5  0A                 .       ASL A
95E6  0A                 .       ASL A
95E7  85 2A              .*      STA &2A
95E9  A9 04              ..      LDA #&04
95EB  85 2B              .+      STA &2B
95ED  C8                 .       INY
95EE  B1 19              ..      LDA (&19),Y
95F0  C8                 .       INY
95F1  C9 25              .%      CMP #&25
95F3  D0 0A              ..      BNE &95FF
95F5  A2 04              ..      LDX #&04
95F7  86 2C              .,      STX &2C
95F9  B1 19              ..      LDA (&19),Y
95FB  C9 28              .(      CMP #&28
95FD  D0 66              .f      BNE &9665
.
95FF  A2 05              ..      LDX #&05
9601  86 2C              .,      STX &2C
9603  A5 1B              ..      LDA &1B
9605  18                 .       CLC
9606  65 19              e.      ADC &19
9608  A6 1A              ..      LDX &1A
960A  90 02              ..      BCC &960E
960C  E8                 .       INX
960D  18                 .       CLC
.
960E  E9 00              ..      SBC #&00
9610  85 37              .7      STA &37
9612  B0 01              ..      BCS &9615
9614  CA                 .       DEX
.
9615  86 38              .8      STX &38
9617  A6 1B              ..      LDX &1B
9619  A0 01              ..      LDY #&01
.
961B  B1 37              .7      LDA (&37),Y
961D  C9 41              .A      CMP #'A'
961F  B0 0C              ..      BCS &962D
9621  C9 30              .0      CMP #'0'
9623  90 1C              ..      BCC &9641
9625  C9 3A              .:      CMP #'9'+1
9627  B0 18              ..      BCS &9641
9629  E8                 .       INX
962A  C8                 .       INY
962B  D0 EE              ..      BNE &961B
.
962D  C9 5B              .[      CMP #'['
962F  B0 04              ..      BCS &9635
9631  E8                 .       INX
9632  C8                 .       INY
9633  D0 E6              ..      BNE &961B
.
9635  C9 5F              ._      CMP #&5F
9637  90 08              ..      BCC &9641
9639  C9 7B              .{      CMP #'{'
963B  B0 04              ..      BCS &9641
963D  E8                 .       INX
963E  C8                 .       INY
963F  D0 DA              ..      BNE &961B
.
9641  88                 .       DEY
9642  F0 2F              ./      BEQ &9673
9644  C9 24              .$      CMP #'$'
9646  F0 67              .g      BEQ &96AF
9648  C9 25              .%      CMP #'%'
964A  D0 08              ..      BNE &9654
964C  C6 2C              .,      DEC &2C
964E  C8                 .       INY
964F  E8                 .       INX
9650  C8                 .       INY
9651  B1 37              .7      LDA (&37),Y
9653  88                 .       DEY
.
9654  84 39              .9      STY &39
9656  C9 28              .(      CMP #'('
9658  F0 4C              .L      BEQ &96A6
965A  20 69 94            i.     JSR &9469
965D  F0 18              ..      BEQ &9677
965F  86 1B              ..      STX &1B
.
9661  A4 1B              ..      LDY &1B
9663  B1 19              ..      LDA (&19),Y
.
9665  C9 21              .!      CMP #'!'
9667  F0 16              ..      BEQ &967F
9669  C9 3F              .?      CMP #'?'
966B  F0 0E              ..      BEQ &967B
966D  18                 .       CLC
966E  84 1B              ..      STY &1B
9670  A9 FF              ..      LDA #&FF
9672  60                 `       RTS
.
9673  A9 00              ..      LDA #&00
9675  38                 8       SEC
9676  60                 `       RTS
.
9677  A9 00              ..      LDA #&00
9679  18                 .       CLC
967A  60                 `       RTS
.
967B  A9 00              ..      LDA #&00
967D  F0 02              ..      BEQ &9681
.
967F  A9 04              ..      LDA #&04
.
9681  48                 H       PHA
9682  C8                 .       INY
9683  84 1B              ..      STY &1B
9685  20 2C B3            ,.     JSR &B32C
9688  20 F0 92            ..     JSR &92F0
968B  A5 2B              .+      LDA &2B
968D  48                 H       PHA
968E  A5 2A              .*      LDA &2A
9690  48                 H       PHA
9691  20 E3 92            ..     JSR &92E3
9694  18                 .       CLC
9695  68                 h       PLA
9696  65 2A              e*      ADC &2A
9698  85 2A              .*      STA &2A
969A  68                 h       PLA
969B  65 2B              e+      ADC &2B
969D  85 2B              .+      STA &2B
.
969F  68                 h       PLA
96A0  85 2C              .,      STA &2C
96A2  18                 .       CLC
96A3  A9 FF              ..      LDA #&FF
96A5  60                 `       RTS
.
96A6  E8                 .       INX
96A7  E6 39              .9      INC &39
96A9  20 DF 96            ..     JSR &96DF
96AC  4C 61 96           La.     JMP &9661
.
96AF  E8                 .       INX
96B0  C8                 .       INY
96B1  84 39              .9      STY &39
96B3  C8                 .       INY
96B4  C6 2C              .,      DEC &2C
96B6  B1 37              .7      LDA (&37),Y
96B8  C9 28              .(      CMP #&28
96BA  F0 0D              ..      BEQ &96C9
96BC  20 69 94            i.     JSR &9469
96BF  F0 B6              ..      BEQ &9677
96C1  86 1B              ..      STX &1B
96C3  A9 81              ..      LDA #&81
96C5  85 2C              .,      STA &2C
96C7  38                 8       SEC
96C8  60                 `       RTS
.
96C9  E8                 .       INX
96CA  84 39              .9      STY &39
96CC  C6 2C              .,      DEC &2C
96CE  20 DF 96            ..     JSR &96DF
96D1  A9 81              ..      LDA #&81
96D3  85 2C              .,      STA &2C
96D5  38                 8       SEC
96D6  60                 `       RTS

.
96D7  00                 .       ...
96D8  0E 41 72           .Ar     ASL &7241
96DB  72 61              ra      ADC (&61)
96DD  79 00              y.

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
96DF  20 69 94            i.     JSR &9469
96E2  F0 F3              ..      BEQ &96D7                  ; Generate "Array" error message
96E4  86 1B              ..      STX &1B
96E6  A5 2C              .,      LDA &2C
96E8  48                 H       PHA
96E9  A5 2A              .*      LDA &2A
96EB  48                 H       PHA
96EC  A5 2B              .+      LDA &2B
96EE  48                 H       PHA
96EF  A0 00              ..      LDY #&00
96F1  B1 2A              .*      LDA (&2A),Y
96F3  C9 04              ..      CMP #&04
96F5  90 75              .u      BCC &976C
96F7  98                 .       TYA
96F8  20 D8 AE            ..     JSR &AED8
96FB  A9 01              ..      LDA #&01
96FD  85 2D              .-      STA &2D
.
96FF  20 94 BD            ..     JSR &BD94
9702  20 DD 92            ..     JSR &92DD
9705  E6 1B              ..      INC &1B
9707  E0 2C              .,      CPX #&2C
9709  D0 CC              ..      BNE &96D7                  ; Generate "Array" error message
970B  A2 39              .9      LDX #&39
970D  20 0D BE            ..     JSR &BE0D
9710  A4 3C              .<      LDY &3C
9712  68                 h       PLA
9713  85 38              .8      STA &38
9715  68                 h       PLA
9716  85 37              .7      STA &37
9718  48                 H       PHA
9719  A5 38              .8      LDA &38
971B  48                 H       PHA
971C  20 BA 97            ..     JSR &97BA
971F  84 2D              .-      STY &2D
9721  B1 37              .7      LDA (&37),Y
9723  85 3F              .?      STA &3F
9725  C8                 .       INY
9726  B1 37              .7      LDA (&37),Y
9728  85 40              .@      STA &40
972A  A5 2A              .*      LDA &2A
972C  65 39              e9      ADC &39
972E  85 2A              .*      STA &2A
9730  A5 2B              .+      LDA &2B
9732  65 3A              e:      ADC &3A
9734  85 2B              .+      STA &2B
9736  20 36 92            6.     JSR &9236
9739  A0 00              ..      LDY #&00
973B  38                 8       SEC
973C  B1 37              .7      LDA (&37),Y
973E  E5 2D              .-      SBC &2D
9740  C9 03              ..      CMP #&03
9742  B0 BB              ..      BCS &96FF
9744  20 94 BD            ..     JSR &BD94
9747  20 56 AE            V.     JSR &AE56
974A  20 F0 92            ..     JSR &92F0
974D  68                 h       PLA
974E  85 38              .8      STA &38
9750  68                 h       PLA
9751  85 37              .7      STA &37
9753  A2 39              .9      LDX #&39
9755  20 0D BE            ..     JSR &BE0D
9758  A4 3C              .<      LDY &3C
975A  20 BA 97            ..     JSR &97BA
975D  18                 .       CLC
975E  A5 39              .9      LDA &39
9760  65 2A              e*      ADC &2A
9762  85 2A              .*      STA &2A
9764  A5 3A              .:      LDA &3A
9766  65 2B              e+      ADC &2B
9768  85 2B              .+      STA &2B
976A  90 11              ..      BCC &977D
.
976C  20 56 AE            V.     JSR &AE56
976F  20 F0 92            ..     JSR &92F0
9772  68                 h       PLA
9773  85 38              .8      STA &38
9775  68                 h       PLA
9776  85 37              .7      STA &37
9778  A0 01              ..      LDY #&01
977A  20 BA 97            ..     JSR &97BA
.
977D  68                 h       PLA
977E  85 2C              .,      STA &2C
9780  C9 05              ..      CMP #&05
9782  D0 17              ..      BNE &979B
9784  A6 2B              .+      LDX &2B
9786  A5 2A              .*      LDA &2A
9788  06 2A              .*      ASL &2A
978A  26 2B              &+      ROL &2B
978C  06 2A              .*      ASL &2A
978E  26 2B              &+      ROL &2B
9790  65 2A              e*      ADC &2A
9792  85 2A              .*      STA &2A
9794  8A                 .       TXA
9795  65 2B              e+      ADC &2B
9797  85 2B              .+      STA &2B
9799  90 08              ..      BCC &97A3
.
979B  06 2A              .*      ASL &2A
979D  26 2B              &+      ROL &2B
979F  06 2A              .*      ASL &2A
97A1  26 2B              &+      ROL &2B
.
97A3  98                 .       TYA
97A4  65 2A              e*      ADC &2A
97A6  85 2A              .*      STA &2A
97A8  90 03              ..      BCC &97AD
97AA  E6 2B              .+      INC &2B
97AC  18                 .       CLC
.
97AD  A5 37              .7      LDA &37
97AF  65 2A              e*      ADC &2A
97B1  85 2A              .*      STA &2A
97B3  A5 38              .8      LDA &38
97B5  65 2B              e+      ADC &2B
97B7  85 2B              .+      STA &2B
97B9  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
97BA  A5 2B              .+      LDA &2B
97BC  29 C0              ).      AND #&C0
97BE  05 2C              .,      ORA &2C
97C0  05 2D              .-      ORA &2D
97C2  D0 0D              ..      BNE &97D1                  ; Generate "Subscript" error message
97C4  A5 2A              .*      LDA &2A
97C6  D1 37              .7      CMP (&37),Y
97C8  C8                 .       INY
97C9  A5 2B              .+      LDA &2B
97CB  F1 37              .7      SBC (&37),Y
97CD  B0 02              ..      BCS &97D1                  ; Generate "Subscript" error message
97CF  C8                 .       INY
97D0  60                 `       RTS
.
97D1  00                 .       ...
97D2  0F 53              .S      BBR0 &9827
97D4  75 62              ub      ADC &62,X
97D6  73                 s       ...
97D7  63                 c       ...
97D8  72 69              ri      ADC (&69)
97DA  70 74              pt      BCS &9850
97DC  00                 .       ...

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
97DD  E6 0A              ..      INC &0A
.
97DF  A4 0A              ..      LDY &0A
97E1  B1 0B              ..      LDA (&0B),Y
97E3  C9 20              .       CMP #' '
97E5  F0 F6              ..      BEQ &97DD
97E7  C9 8D              ..      CMP #&8D
97E9  D0 1A              ..      BNE &9805
.
97EB  C8                 .       INY
97EC  B1 0B              ..      LDA (&0B),Y
97EE  0A                 .       ASL A
97EF  0A                 .       ASL A
97F0  AA                 .       TAX
97F1  29 C0              ).      AND #&C0
97F3  C8                 .       INY
97F4  51 0B              Q.      EOR (&0B),Y
97F6  85 2A              .*      STA &2A
97F8  8A                 .       TXA
97F9  0A                 .       ASL A
97FA  0A                 .       ASL A
97FB  C8                 .       INY
97FC  51 0B              Q.      EOR (&0B),Y
97FE  85 2B              .+      STA &2B
9800  C8                 .       INY
9801  84 0A              ..      STY &0A
9803  38                 8       SEC
9804  60                 `       RTS
.
9805  18                 .       CLC
9806  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
9807  A5 0B              ..      LDA &0B
9809  85 19              ..      STA &19
980B  A5 0C              ..      LDA &0C
980D  85 1A              ..      STA &1A
980F  A5 0A              ..      LDA &0A
9811  85 1B              ..      STA &1B
.
9813  A4 1B              ..      LDY &1B
9815  E6 1B              ..      INC &1B
9817  B1 19              ..      LDA (&19),Y
9819  C9 20              .       CMP #' '
981B  F0 F6              ..      BEQ &9813
981D  C9 3D              .=      CMP #'='
981F  F0 28              .(      BEQ &9849
.
9821  00                 .       ...
9822  04 4D              .M      TSB &4D
9824  69 73              is      ADC #&73
9826  74 61              ta      STZ &61,X
9828  6B                 k       ...
9829  65                 e
.
9830  00                 .       BRK
982B  10 53              .S      BPL &9880
982D  79 6E 74           ynt     ADC &746E,Y
9830  61 78              ax      ADC (&78,X)
9832  20 65 72            er     JSR &7265
9835  72 6F              ro      ADC (&6F)
9837  72                 r
.
9838  00                 .       BRK
9839  11 45              .E      ORA (&45),Y
983B  73                 s       ...
983C  63                 c       ...
983D  61 70              ap      ADC (&70,X)
983F  65 00              e.      ADC &00

.
9841  20 8C 8A            ..     JSR &8A8C
9844  C9 3D              .=      CMP #&3D
9846  D0 D9              ..      BNE &9821                  ; Generate "Mistake" error message
9848  60                 `       RTS
.
9849  20 29 9B            ).     JSR &9B29
.
984C  8A                 .       TXA
984D  A4 1B              ..      LDY &1B
984F  4C 61 98           La.     JMP &9861
.
9852  A4 1B              ..      LDY &1B
9854  4C 59 98           LY.     JMP &9859

; -----------------------------------------------------------------------------
; Check end of statement
; -----------------------------------------------------------------------------
.
9857  A4 0A              ..      LDY &0A
.
9859  88                 .       DEY
.
985A  C8                 .       INY
985B  B1 0B              ..      LDA (&0B),Y
985D  C9 20              .       CMP #&20
985F  F0 F9              ..      BEQ &985A
.
9861  C9 3A              .:      CMP #&3A
9863  F0 08              ..      BEQ &986D
9865  C9 0D              ..      CMP #&0D
9867  F0 04              ..      BEQ &986D
9869  C9 8B              ..      CMP #&8B
986B  D0 BD              ..      BNE &982A
.
986D  18                 .       CLC
986E  98                 .       TYA
986F  65 0B              e.      ADC &0B
9871  85 0B              ..      STA &0B
9873  90 02              ..      BCC &9877
9875  E6 0C              ..      INC &0C
.
9877  A0 01              ..      LDY #&01
9879  84 0A              ..      STY &0A
.
987B  24 FF              $.      BIT &FF
987D  30 B9              0.      BMI &9838                  ; Generate "Escape" error message
.
987F  60                 `       RTS
.
9880  20 57 98            W.     JSR &9857                  ; Check end of statement
9883  88                 .       DEY
9884  B1 0B              ..      LDA (&0B),Y
9886  C9 3A              .:      CMP #&3A
9888  F0 F5              ..      BEQ &987F
988A  A5 0C              ..      LDA &0C
988C  C9 07              ..      CMP #&07
988E  F0 2C              .,      BEQ &98BC
.
9890  C8                 .       INY
9891  B1 0B              ..      LDA (&0B),Y
9893  30 27              0'      BMI &98BC
9895  A5 20              .       LDA &20
9897  F0 13              ..      BEQ &98AC
9899  98                 .       TYA
989A  48                 H       PHA
989B  C8                 .       INY
989C  B1 0B              ..      LDA (&0B),Y
989E  48                 H       PHA
989F  88                 .       DEY
98A0  B1 0B              ..      LDA (&0B),Y
98A2  A8                 .       TAY
98A3  68                 h       PLA
98A4  20 EA AE            ..     JSR &AEEA
98A7  20 05 99            ..     JSR &9905
98AA  68                 h       PLA
98AB  A8                 .       TAY
.
98AC  C8                 .       INY
98AD  38                 8       SEC
98AE  98                 .       TYA
98AF  65 0B              e.      ADC &0B
98B1  85 0B              ..      STA &0B
98B3  90 02              ..      BCC &98B7
98B5  E6 0C              ..      INC &0C
.
98B7  A0 01              ..      LDY #&01
98B9  84 0A              ..      STY &0A
.
98BB  60                 `       RTS
.
98BC  4C F6 8A           L..     JMP &8AF6
.
98BF  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
98C2  20 1D 9B            ..     JSR &9B1D
98C5  F0 F8              ..      BEQ &98BF
98C7  10 03              ..      BPL &98CC
98C9  20 E4 A3            ..     JSR &A3E4
.
98CC  A4 1B              ..      LDY &1B
98CE  84 0A              ..      STY &0A
98D0  A5 2A              .*      LDA &2A
98D2  05 2B              .+      ORA &2B
98D4  05 2C              .,      ORA &2C
98D6  05 2D              .-      ORA &2D
98D8  F0 17              ..      BEQ &98F1
98DA  E0 8C              ..      CPX #&8C
98DC  F0 03              ..      BEQ &98E1
.
98DE  4C A3 8B           L..     JMP &8BA3
.
98E1  E6 0A              ..      INC &0A
.
98E3  20 DF 97            ..     JSR &97DF
98E6  90 F6              ..      BCC &98DE
98E8  20 AF B9            ..     JSR &B9AF
98EB  20 77 98            w.     JSR &9877
98EE  4C D2 B8           L..     JMP &B8D2
.
98F1  A4 0A              ..      LDY &0A
.
98F3  B1 0B              ..      LDA (&0B),Y
98F5  C9 0D              ..      CMP #&0D
98F7  F0 09              ..      BEQ &9902
98F9  C8                 .       INY
98FA  C9 8B              ..      CMP #&8B
98FC  D0 F5              ..      BNE &98F3
98FE  84 0A              ..      STY &0A
9900  F0 E1              ..      BEQ &98E3
.
9902  4C 87 8B           L..     JMP &8B87
.
9905  A5 2A              .*      LDA &2A
9907  C5 21              .!      CMP &21
9909  A5 2B              .+      LDA &2B
990B  E5 22              ."      SBC &22
990D  B0 AC              ..      BCS &98BB
990F  A9 5B              .[      LDA #&5B
.
9911  20 58 B5            X.     JSR &B558
9914  20 1F 99            ..     JSR &991F
9917  A9 5D              .]      LDA #&5D
9919  20 58 B5            X.     JSR &B558
991C  4C 65 B5           Le.     JMP &B565
.
991F  A9 00              ..      LDA #&00
9921  F0 02              ..      BEQ &9925
.
9923  A9 05              ..      LDA #&05
.
9925  85 14              ..      STA &14
9927  A2 04              ..      LDX #&04
.
9929  A9 00              ..      LDA #&00
992B  95 3F              .?      STA &3F,X
992D  38                 8       SEC
.
992E  A5 2A              .*      LDA &2A
9930  FD 6B 99           .k.     SBC &996B,X
9933  A8                 .       TAY
9934  A5 2B              .+      LDA &2B
9936  FD B9 99           ...     SBC &99B9,X
9939  90 08              ..      BCC &9943
993B  85 2B              .+      STA &2B
993D  84 2A              .*      STY &2A
993F  F6 3F              .?      INC &3F,X
9941  D0 EB              ..      BNE &992E
.
9943  CA                 .       DEX
9944  10 E3              ..      BPL &9929
9946  A2 05              ..      LDX #&05
.
9948  CA                 .       DEX
9949  F0 04              ..      BEQ &994F
994B  B5 3F              .?      LDA &3F,X
994D  F0 F9              ..      BEQ &9948
.
994F  86 37              .7      STX &37
9951  A5 14              ..      LDA &14
9953  F0 0B              ..      BEQ &9960
9955  E5 37              .7      SBC &37
9957  F0 07              ..      BEQ &9960
9959  A8                 .       TAY
.
995A  20 65 B5            e.     JSR &B565
995D  88                 .       DEY
995E  D0 FA              ..      BNE &995A
.
9960  B5 3F              .?      LDA &3F,X
9962  09 30              .0      ORA #&30
9964  20 58 B5            X.     JSR &B558
9967  CA                 .       DEX
9968  10 F6              ..      BPL &9960
996A  60                 `       RTS
996B  01 0A              ..      ORA (&0A,X)
996D  64 E8              d.      STZ &E8
996F  10 A0              ..      BPL &9911
9971  00                 .       ...
9972  84 3D              .=      STY &3D
9974  A5 18              ..      LDA page
9976  85 3E              .>      STA &3E
.
9978  A0 01              ..      LDY #&01
997A  B1 3D              .=      LDA (&3D),Y
997C  C5 2B              .+      CMP &2B
997E  B0 0E              ..      BCS &998E
.
9980  A0 03              ..      LDY #&03
9982  B1 3D              .=      LDA (&3D),Y
9984  65 3D              e=      ADC &3D
9986  85 3D              .=      STA &3D
9988  90 EE              ..      BCC &9978
998A  E6 3E              .>      INC &3E
998C  B0 EA              ..      BCS &9978
.
998E  D0 14              ..      BNE &99A4
9990  A0 02              ..      LDY #&02
9992  B1 3D              .=      LDA (&3D),Y
9994  C5 2A              .*      CMP &2A
9996  90 E8              ..      BCC &9980
9998  D0 0A              ..      BNE &99A4
999A  98                 .       TYA
999B  65 3D              e=      ADC &3D
999D  85 3D              .=      STA &3D
999F  90 03              ..      BCC &99A4
99A1  E6 3E              .>      INC &3E
99A3  18                 .       CLC
.
99A4  A0 02              ..      LDY #&02
99A6  60                 `       RTS
.
99A7  00                 .       ...
99A8  12 44              .D      ORA (&44)
99AA  69 76              iv      ADC #&76
99AC  69 73              is      ADC #&73
99AE  69 6F              io      ADC #&6F
99B0  6E 20 62           n b     ROR &6220
99B3  79 20 7A           y z     ADC &7A20,Y
99B6  65 72              er      ADC &72
99B8  6F 00              o.      BBR6 &99BA
.
99BA  00                 .       ...
99BB  00                 .       ...
99BC  03                 .       ...
99BD  27 A8              '.      RMB2 &A8
99BF  20 F0 92            ..     JSR &92F0
99C2  A5 2D              .-      LDA &2D
99C4  48                 H       PHA
99C5  20 71 AD            q.     JSR &AD71
99C8  20 1D 9E            ..     JSR &9E1D
99CB  86 27              .'      STX &27
99CD  A8                 .       TAY
99CE  20 F0 92            ..     JSR &92F0
99D1  68                 h       PLA
99D2  85 38              .8      STA &38
99D4  45 2D              E-      EOR &2D
99D6  85 37              .7      STA &37
99D8  20 71 AD            q.     JSR &AD71
99DB  A2 39              .9      LDX #&39
99DD  20 0D BE            ..     JSR &BE0D
99E0  84 3D              .=      STY &3D
99E2  84 3E              .>      STY &3E
99E4  84 3F              .?      STY &3F
99E6  84 40              .@      STY &40
99E8  A5 2D              .-      LDA &2D
99EA  05 2A              .*      ORA &2A
99EC  05 2B              .+      ORA &2B
99EE  05 2C              .,      ORA &2C
99F0  F0 B5              ..      BEQ &99A7
99F2  A0 20              .       LDY #&20
.
99F4  88                 .       DEY
99F5  F0 41              .A      BEQ &9A38
99F7  06 39              .9      ASL &39
99F9  26 3A              &:      ROL &3A
99FB  26 3B              &;      ROL &3B
99FD  26 3C              &<      ROL &3C
99FF  10 F3              ..      BPL &99F4
.
9A01  26 39              &9      ROL &39
9A03  26 3A              &:      ROL &3A
9A05  26 3B              &;      ROL &3B
9A07  26 3C              &<      ROL &3C
9A09  26 3D              &=      ROL &3D
9A0B  26 3E              &>      ROL &3E
9A0D  26 3F              &?      ROL &3F
9A0F  26 40              &@      ROL &40
9A11  38                 8       SEC
9A12  A5 3D              .=      LDA &3D
9A14  E5 2A              .*      SBC &2A
9A16  48                 H       PHA
9A17  A5 3E              .>      LDA &3E
9A19  E5 2B              .+      SBC &2B
9A1B  48                 H       PHA
9A1C  A5 3F              .?      LDA &3F
9A1E  E5 2C              .,      SBC &2C
9A20  AA                 .       TAX
9A21  A5 40              .@      LDA &40
9A23  E5 2D              .-      SBC &2D
9A25  90 0C              ..      BCC &9A33
9A27  85 40              .@      STA &40
9A29  86 3F              .?      STX &3F
9A2B  68                 h       PLA
9A2C  85 3E              .>      STA &3E
9A2E  68                 h       PLA
9A2F  85 3D              .=      STA &3D
9A31  B0 02              ..      BCS &9A35
.
9A33  68                 h       PLA
9A34  68                 h       PLA
.
9A35  88                 .       DEY
9A36  D0 C9              ..      BNE &9A01
.
9A38  60                 `       RTS
.
9A39  86 27              .'      STX &27
9A3B  20 EA BD            ..     JSR &BDEA
9A3E  20 51 BD            Q.     JSR &BD51
9A41  20 BE A2            ..     JSR &A2BE
9A44  20 1E A2            ..     JSR &A21E
9A47  20 7E BD            ~.     JSR &BD7E
9A4A  20 B5 A3            ..     JSR &A3B5
9A4D  4C 62 9A           Lb.     JMP &9A62
.
9A50  20 51 BD            Q.     JSR &BD51
9A53  20 42 9C            B.     JSR &9C42
9A56  86 27              .'      STX &27
9A58  A8                 .       TAY
9A59  20 FD 92            ..     JSR &92FD
9A5C  20 7E BD            ~.     JSR &BD7E
.
9A5F  20 4E A3            N.     JSR &A34E
.
9A62  A6 27              .'      LDX &27
9A64  A0 00              ..      LDY #&00
9A66  A5 3B              .;      LDA &3B
9A68  29 80              ).      AND #&80
9A6A  85 3B              .;      STA &3B
9A6C  A5 2E              ..      LDA &2E
9A6E  29 80              ).      AND #&80
9A70  C5 3B              .;      CMP &3B
9A72  D0 1E              ..      BNE &9A92
9A74  A5 3D              .=      LDA &3D
9A76  C5 30              .0      CMP &30
9A78  D0 19              ..      BNE &9A93
9A7A  A5 3E              .>      LDA &3E
9A7C  C5 31              .1      CMP &31
9A7E  D0 13              ..      BNE &9A93
9A80  A5 3F              .?      LDA &3F
9A82  C5 32              .2      CMP &32
9A84  D0 0D              ..      BNE &9A93
9A86  A5 40              .@      LDA &40
9A88  C5 33              .3      CMP &33
9A8A  D0 07              ..      BNE &9A93
9A8C  A5 41              .A      LDA &41
9A8E  C5 34              .4      CMP &34
9A90  D0 01              ..      BNE &9A93
.
9A92  60                 `       RTS
.
9A93  6A                 j       ROR A
9A94  45 3B              E;      EOR &3B
9A96  2A                 *       ROL A
9A97  A9 01              ..      LDA #&01
9A99  60                 `       RTS
.
9A9A  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
9A9D  8A                 .       TXA
.
9A9E  F0 47              .G      BEQ &9AE7
9AA0  30 AE              0.      BMI &9A50
9AA2  20 94 BD            ..     JSR &BD94
9AA5  20 42 9C            B.     JSR &9C42
9AA8  A8                 .       TAY
9AA9  F0 EF              ..      BEQ &9A9A
9AAB  30 8C              0.      BMI &9A39
9AAD  A5 2D              .-      LDA &2D
9AAF  49 80              I.      EOR #&80
9AB1  85 2D              .-      STA &2D
9AB3  38                 8       SEC
9AB4  A0 00              ..      LDY #&00
9AB6  B1 04              ..      LDA (&04),Y
9AB8  E5 2A              .*      SBC &2A
9ABA  85 2A              .*      STA &2A
9ABC  C8                 .       INY
9ABD  B1 04              ..      LDA (&04),Y
9ABF  E5 2B              .+      SBC &2B
9AC1  85 2B              .+      STA &2B
9AC3  C8                 .       INY
9AC4  B1 04              ..      LDA (&04),Y
9AC6  E5 2C              .,      SBC &2C
9AC8  85 2C              .,      STA &2C
9ACA  C8                 .       INY
9ACB  B1 04              ..      LDA (&04),Y
9ACD  A0 00              ..      LDY #&00
9ACF  49 80              I.      EOR #&80
9AD1  E5 2D              .-      SBC &2D
9AD3  05 2A              .*      ORA &2A
9AD5  05 2B              .+      ORA &2B
9AD7  05 2C              .,      ORA &2C
9AD9  08                 .       PHP
9ADA  18                 .       CLC
9ADB  A9 04              ..      LDA #&04
9ADD  65 04              e.      ADC &04
9ADF  85 04              ..      STA &04
9AE1  90 02              ..      BCC &9AE5
9AE3  E6 05              ..      INC &05
.
9AE5  28                 (       PLP
9AE6  60                 `       RTS
.
9AE7  20 B2 BD            ..     JSR &BDB2
9AEA  20 42 9C            B.     JSR &9C42
9AED  A8                 .       TAY
9AEE  D0 AA              ..      BNE &9A9A
9AF0  86 37              .7      STX &37
9AF2  A6 36              .6      LDX &36
9AF4  A0 00              ..      LDY #&00
9AF6  B1 04              ..      LDA (&04),Y
9AF8  85 39              .9      STA &39
9AFA  C5 36              .6      CMP &36
9AFC  B0 01              ..      BCS &9AFF
9AFE  AA                 .       TAX
.
9AFF  86 3A              .:      STX &3A
9B01  A0 00              ..      LDY #&00
.
9B03  C4 3A              .:      CPY &3A
9B05  F0 0A              ..      BEQ &9B11
9B07  C8                 .       INY
9B08  B1 04              ..      LDA (&04),Y
9B0A  D9 FF 05           ...     CMP &05FF,Y
9B0D  F0 F4              ..      BEQ &9B03
9B0F  D0 04              ..      BNE &9B15
.
9B11  A5 39              .9      LDA &39
9B13  C5 36              .6      CMP &36
.
9B15  08                 .       PHP
9B16  20 DC BD            ..     JSR &BDDC
9B19  A6 37              .7      LDX &37
9B1B  28                 (       PLP
9B1C  60                 `       RTS
.
9B1D  A5 0B              ..      LDA &0B
9B1F  85 19              ..      STA &19
9B21  A5 0C              ..      LDA &0C
9B23  85 1A              ..      STA &1A
9B25  A5 0A              ..      LDA &0A
9B27  85 1B              ..      STA &1B
.
9B29  20 72 9B            r.     JSR &9B72
.
9B2C  E0 84              ..      CPX #&84
9B2E  F0 0A              ..      BEQ &9B3A
9B30  E0 82              ..      CPX #&82
9B32  F0 21              .!      BEQ &9B55
9B34  C6 1B              ..      DEC &1B
9B36  A8                 .       TAY
9B37  85 27              .'      STA &27
9B39  60                 `       RTS
.
9B3A  20 6B 9B            k.     JSR &9B6B
9B3D  A8                 .       TAY
9B3E  20 F0 92            ..     JSR &92F0
9B41  A0 03              ..      LDY #&03
.
9B43  B1 04              ..      LDA (&04),Y
9B45  19 2A 00           .*.     ORA &002A,Y
9B48  99 2A 00           .*.     STA &002A,Y
9B4B  88                 .       DEY
9B4C  10 F5              ..      BPL &9B43
.
9B4E  20 FF BD            ..     JSR &BDFF
9B51  A9 40              .@      LDA #&40
9B53  D0 D7              ..      BNE &9B2C
.
9B55  20 6B 9B            k.     JSR &9B6B
9B58  A8                 .       TAY
9B59  20 F0 92            ..     JSR &92F0
9B5C  A0 03              ..      LDY #&03
.
9B5E  B1 04              ..      LDA (&04),Y
9B60  59 2A 00           Y*.     EOR &002A,Y
9B63  99 2A 00           .*.     STA &002A,Y
9B66  88                 .       DEY
9B67  10 F5              ..      BPL &9B5E
9B69  30 E3              0.      BMI &9B4E
.
9B6B  A8                 .       TAY
9B6C  20 F0 92            ..     JSR &92F0
9B6F  20 94 BD            ..     JSR &BD94
.
9B72  20 9C 9B            ..     JSR &9B9C
.
9B75  E0 80              ..      CPX #&80
9B77  F0 01              ..      BEQ &9B7A
9B79  60                 `       RTS
.
9B7A  A8                 .       TAY
9B7B  20 F0 92            ..     JSR &92F0
9B7E  20 94 BD            ..     JSR &BD94
9B81  20 9C 9B            ..     JSR &9B9C
9B84  A8                 .       TAY
9B85  20 F0 92            ..     JSR &92F0
9B88  A0 03              ..      LDY #&03
.
9B8A  B1 04              ..      LDA (&04),Y
9B8C  39 2A 00           9*.     AND &002A,Y
9B8F  99 2A 00           .*.     STA &002A,Y
9B92  88                 .       DEY
9B93  10 F5              ..      BPL &9B8A
9B95  20 FF BD            ..     JSR &BDFF
9B98  A9 40              .@      LDA #&40
9B9A  D0 D9              ..      BNE &9B75
.
9B9C  20 42 9C            B.     JSR &9C42
9B9F  E0 3F              .?      CPX #&3F
9BA1  B0 04              ..      BCS &9BA7
9BA3  E0 3C              .<      CPX #&3C
9BA5  B0 01              ..      BCS &9BA8
.
9BA7  60                 `       RTS
.
9BA8  F0 16              ..      BEQ &9BC0
9BAA  E0 3E              .>      CPX #&3E
9BAC  F0 3A              .:      BEQ &9BE8
9BAE  AA                 .       TAX
9BAF  20 9E 9A            ..     JSR &9A9E
9BB2  D0 01              ..      BNE &9BB5
.
9BB4  88                 .       DEY
.
9BB5  84 2A              .*      STY &2A
9BB7  84 2B              .+      STY &2B
9BB9  84 2C              .,      STY &2C
9BBB  84 2D              .-      STY &2D
9BBD  A9 40              .@      LDA #&40
9BBF  60                 `       RTS
.
9BC0  AA                 .       TAX
9BC1  A4 1B              ..      LDY &1B
9BC3  B1 19              ..      LDA (&19),Y
9BC5  C9 3D              .=      CMP #&3D
9BC7  F0 0B              ..      BEQ &9BD4
9BC9  C9 3E              .>      CMP #&3E
9BCB  F0 12              ..      BEQ &9BDF
9BCD  20 9D 9A            ..     JSR &9A9D
9BD0  90 E2              ..      BCC &9BB4
9BD2  B0 E1              ..      BCS &9BB5
.
9BD4  E6 1B              ..      INC &1B
9BD6  20 9D 9A            ..     JSR &9A9D
9BD9  F0 D9              ..      BEQ &9BB4
9BDB  90 D7              ..      BCC &9BB4
9BDD  B0 D6              ..      BCS &9BB5
.
9BDF  E6 1B              ..      INC &1B
9BE1  20 9D 9A            ..     JSR &9A9D
9BE4  D0 CE              ..      BNE &9BB4
9BE6  F0 CD              ..      BEQ &9BB5
.
9BE8  AA                 .       TAX
9BE9  A4 1B              ..      LDY &1B
9BEB  B1 19              ..      LDA (&19),Y
9BED  C9 3D              .=      CMP #&3D
9BEF  F0 09              ..      BEQ &9BFA
9BF1  20 9D 9A            ..     JSR &9A9D
9BF4  F0 BF              ..      BEQ &9BB5
9BF6  B0 BC              ..      BCS &9BB4
9BF8  90 BB              ..      BCC &9BB5
.
9BFA  E6 1B              ..      INC &1B
9BFC  20 9D 9A            ..     JSR &9A9D
9BFF  B0 B3              ..      BCS &9BB4
9C01  90 B2              ..      BCC &9BB5
.
9C03  00                 .       ...
9C04  13                 .       ...
9C05  53                 S       ...
9C06  74 72              tr      STZ &72,X
9C08  69 6E              in      ADC #&6E
9C0A  67 20              g       RMB6 &20
9C0C  74 6F              to      STZ &6F,X
9C0E  6F 20              o       BBR6 &9C30
9C10  6C 6F 6E           lon     JMP (&6E6F)
9C13  67 00              g.      RMB6 &00
.
9C15  20 B2 BD            ..     JSR &BDB2
9C18  20 20 9E             .     JSR &9E20
9C1B  A8                 .       TAY
9C1C  D0 6A              .j      BNE &9C88
9C1E  18                 .       CLC
9C1F  86 37              .7      STX &37
9C21  A0 00              ..      LDY #&00
9C23  B1 04              ..      LDA (&04),Y
9C25  65 36              e6      ADC &36
9C27  B0 DA              ..      BCS &9C03
9C29  AA                 .       TAX
9C2A  48                 H       PHA
9C2B  A4 36              .6      LDY &36
.
9C2D  B9 FF 05           ...     LDA &05FF,Y
.
9C30  9D FF 05           ...     STA &05FF,X
9C33  CA                 .       DEX
9C34  88                 .       DEY
9C35  D0 F6              ..      BNE &9C2D
9C37  20 CB BD            ..     JSR &BDCB
9C3A  68                 h       PLA
9C3B  85 36              .6      STA &36
9C3D  A6 37              .7      LDX &37
9C3F  98                 .       TYA
9C40  F0 03              ..      BEQ &9C45
.
9C42  20 D1 9D            ..     JSR &9DD1
.
9C45  E0 2B              .+      CPX #&2B
9C47  F0 05              ..      BEQ &9C4E
9C49  E0 2D              .-      CPX #&2D
9C4B  F0 68              .h      BEQ &9CB5
9C4D  60                 `       RTS
.
9C4E  A8                 .       TAY
9C4F  F0 C4              ..      BEQ &9C15
9C51  30 38              08      BMI &9C8B
9C53  20 CE 9D            ..     JSR &9DCE
9C56  A8                 .       TAY
9C57  F0 2F              ./      BEQ &9C88
9C59  30 4C              0L      BMI &9CA7
9C5B  A0 00              ..      LDY #&00
9C5D  18                 .       CLC
9C5E  B1 04              ..      LDA (&04),Y
9C60  65 2A              e*      ADC &2A
9C62  85 2A              .*      STA &2A
9C64  C8                 .       INY
9C65  B1 04              ..      LDA (&04),Y
9C67  65 2B              e+      ADC &2B
9C69  85 2B              .+      STA &2B
9C6B  C8                 .       INY
9C6C  B1 04              ..      LDA (&04),Y
9C6E  65 2C              e,      ADC &2C
9C70  85 2C              .,      STA &2C
9C72  C8                 .       INY
9C73  B1 04              ..      LDA (&04),Y
9C75  65 2D              e-      ADC &2D
.
9C77  85 2D              .-      STA &2D
9C79  18                 .       CLC
9C7A  A5 04              ..      LDA &04
9C7C  69 04              i.      ADC #&04
9C7E  85 04              ..      STA &04
9C80  A9 40              .@      LDA #&40
9C82  90 C1              ..      BCC &9C45
9C84  E6 05              ..      INC &05
9C86  B0 BD              ..      BCS &9C45
.
9C88  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
9C8B  20 51 BD            Q.     JSR &BD51
9C8E  20 D1 9D            ..     JSR &9DD1
9C91  A8                 .       TAY
9C92  F0 F4              ..      BEQ &9C88
9C94  86 27              .'      STX &27
9C96  30 03              0.      BMI &9C9B
9C98  20 BE A2            ..     JSR &A2BE
.
9C9B  20 7E BD            ~.     JSR &BD7E
9C9E  20 00 A5            ..     JSR &A500
.
9CA1  A6 27              .'      LDX &27
9CA3  A9 FF              ..      LDA #&FF
9CA5  D0 9E              ..      BNE &9C45
.
9CA7  86 27              .'      STX &27
9CA9  20 EA BD            ..     JSR &BDEA
9CAC  20 51 BD            Q.     JSR &BD51
9CAF  20 BE A2            ..     JSR &A2BE
9CB2  4C 9B 9C           L..     JMP &9C9B
.
9CB5  A8                 .       TAY
9CB6  F0 D0              ..      BEQ &9C88
9CB8  30 27              0'      BMI &9CE1
9CBA  20 CE 9D            ..     JSR &9DCE
9CBD  A8                 .       TAY
9CBE  F0 C8              ..      BEQ &9C88
9CC0  30 38              08      BMI &9CFA
9CC2  38                 8       SEC
9CC3  A0 00              ..      LDY #&00
9CC5  B1 04              ..      LDA (&04),Y
9CC7  E5 2A              .*      SBC &2A
9CC9  85 2A              .*      STA &2A
9CCB  C8                 .       INY
9CCC  B1 04              ..      LDA (&04),Y
9CCE  E5 2B              .+      SBC &2B
9CD0  85 2B              .+      STA &2B
9CD2  C8                 .       INY
9CD3  B1 04              ..      LDA (&04),Y
9CD5  E5 2C              .,      SBC &2C
9CD7  85 2C              .,      STA &2C
9CD9  C8                 .       INY
9CDA  B1 04              ..      LDA (&04),Y
9CDC  E5 2D              .-      SBC &2D
9CDE  4C 77 9C           Lw.     JMP &9C77
.
9CE1  20 51 BD            Q.     JSR &BD51
9CE4  20 D1 9D            ..     JSR &9DD1
9CE7  A8                 .       TAY
9CE8  F0 9E              ..      BEQ &9C88
9CEA  86 27              .'      STX &27
9CEC  30 03              0.      BMI &9CF1
9CEE  20 BE A2            ..     JSR &A2BE
.
9CF1  20 7E BD            ~.     JSR &BD7E
9CF4  20 FD A4            ..     JSR &A4FD
9CF7  4C A1 9C           L..     JMP &9CA1
.
9CFA  86 27              .'      STX &27
9CFC  20 EA BD            ..     JSR &BDEA
9CFF  20 51 BD            Q.     JSR &BD51
9D02  20 BE A2            ..     JSR &A2BE
9D05  20 7E BD            ~.     JSR &BD7E
9D08  20 D0 A4            ..     JSR &A4D0
9D0B  4C A1 9C           L..     JMP &9CA1
.
9D0E  20 BE A2            ..     JSR &A2BE
.
9D11  20 EA BD            ..     JSR &BDEA
9D14  20 51 BD            Q.     JSR &BD51
9D17  20 BE A2            ..     JSR &A2BE
9D1A  4C 2C 9D           L,.     JMP &9D2C
.
9D1D  20 BE A2            ..     JSR &A2BE
.
9D20  20 51 BD            Q.     JSR &BD51
9D23  20 20 9E             .     JSR &9E20
9D26  86 27              .'      STX &27
9D28  A8                 .       TAY
9D29  20 FD 92            ..     JSR &92FD
.
9D2C  20 7E BD            ~.     JSR &BD7E
9D2F  20 56 A6            V.     JSR &A656
9D32  A9 FF              ..      LDA #&FF
9D34  A6 27              .'      LDX &27
9D36  4C D4 9D           L..     JMP &9DD4
.
9D39  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
9D3C  A8                 .       TAY
9D3D  F0 FA              ..      BEQ &9D39
9D3F  30 DF              0.      BMI &9D20
9D41  A5 2D              .-      LDA &2D
9D43  C5 2C              .,      CMP &2C
9D45  D0 D6              ..      BNE &9D1D
9D47  A8                 .       TAY
9D48  F0 04              ..      BEQ &9D4E
9D4A  C9 FF              ..      CMP #&FF
9D4C  D0 CF              ..      BNE &9D1D
.
9D4E  45 2B              E+      EOR &2B
9D50  30 CB              0.      BMI &9D1D
9D52  20 1D 9E            ..     JSR &9E1D
9D55  86 27              .'      STX &27
9D57  A8                 .       TAY
9D58  F0 DF              ..      BEQ &9D39
9D5A  30 B5              0.      BMI &9D11
9D5C  A5 2D              .-      LDA &2D
9D5E  C5 2C              .,      CMP &2C
9D60  D0 AC              ..      BNE &9D0E
9D62  A8                 .       TAY
9D63  F0 04              ..      BEQ &9D69
9D65  C9 FF              ..      CMP #&FF
9D67  D0 A5              ..      BNE &9D0E
.
9D69  45 2B              E+      EOR &2B
9D6B  30 A1              0.      BMI &9D0E
9D6D  A5 2D              .-      LDA &2D
9D6F  48                 H       PHA
9D70  20 71 AD            q.     JSR &AD71
9D73  A2 39              .9      LDX #&39
9D75  20 44 BE            D.     JSR &BE44
9D78  20 EA BD            ..     JSR &BDEA
9D7B  68                 h       PLA
9D7C  45 2D              E-      EOR &2D
9D7E  85 37              .7      STA &37
9D80  20 71 AD            q.     JSR &AD71
9D83  A0 00              ..      LDY #&00
9D85  A2 00              ..      LDX #&00
9D87  84 3F              .?      STY &3F
9D89  84 40              .@      STY &40
.
9D8B  46 3A              F:      LSR &3A
9D8D  66 39              f9      ROR &39
9D8F  90 15              ..      BCC &9DA6
9D91  18                 .       CLC
9D92  98                 .       TYA
9D93  65 2A              e*      ADC &2A
9D95  A8                 .       TAY
9D96  8A                 .       TXA
9D97  65 2B              e+      ADC &2B
9D99  AA                 .       TAX
9D9A  A5 3F              .?      LDA &3F
9D9C  65 2C              e,      ADC &2C
9D9E  85 3F              .?      STA &3F
9DA0  A5 40              .@      LDA &40
9DA2  65 2D              e-      ADC &2D
9DA4  85 40              .@      STA &40
.
9DA6  06 2A              .*      ASL &2A
9DA8  26 2B              &+      ROL &2B
9DAA  26 2C              &,      ROL &2C
9DAC  26 2D              &-      ROL &2D
9DAE  A5 39              .9      LDA &39
9DB0  05 3A              .:      ORA &3A
9DB2  D0 D7              ..      BNE &9D8B
9DB4  84 3D              .=      STY &3D
9DB6  86 3E              .>      STX &3E
9DB8  A5 37              .7      LDA &37
9DBA  08                 .       PHP
.
9DBB  A2 3D              .=      LDX #&3D
.
9DBD  20 56 AF            V.     JSR &AF56
9DC0  28                 (       PLP
9DC1  10 03              ..      BPL &9DC6
9DC3  20 93 AD            ..     JSR &AD93
.
9DC6  A6 27              .'      LDX &27
9DC8  4C D4 9D           L..     JMP &9DD4
.
9DCB  4C 3C 9D           L<.     JMP &9D3C
.
9DCE  20 94 BD            ..     JSR &BD94
.
9DD1  20 20 9E             .     JSR &9E20
.
9DD4  E0 2A              .*      CPX #&2A
9DD6  F0 F3              ..      BEQ &9DCB
9DD8  E0 2F              ./      CPX #&2F
9DDA  F0 09              ..      BEQ &9DE5
9DDC  E0 83              ..      CPX #&83
9DDE  F0 21              .!      BEQ &9E01
9DE0  E0 81              ..      CPX #&81
9DE2  F0 26              .&      BEQ &9E0A
9DE4  60                 `       RTS
.
9DE5  A8                 .       TAY
9DE6  20 FD 92            ..     JSR &92FD
9DE9  20 51 BD            Q.     JSR &BD51
9DEC  20 20 9E             .     JSR &9E20
9DEF  86 27              .'      STX &27
9DF1  A8                 .       TAY
9DF2  20 FD 92            ..     JSR &92FD
9DF5  20 7E BD            ~.     JSR &BD7E
9DF8  20 AD A6            ..     JSR &A6AD
9DFB  A6 27              .'      LDX &27
9DFD  A9 FF              ..      LDA #&FF
9DFF  D0 D3              ..      BNE &9DD4
.
9E01  20 BE 99            ..     JSR &99BE
9E04  A5 38              .8      LDA &38
9E06  08                 .       PHP
9E07  4C BB 9D           L..     JMP &9DBB
.
9E0A  20 BE 99            ..     JSR &99BE
9E0D  26 39              &9      ROL &39
9E0F  26 3A              &:      ROL &3A
9E11  26 3B              &;      ROL &3B
9E13  26 3C              &<      ROL &3C
9E15  24 37              $7      BIT &37
9E17  08                 .       PHP
9E18  A2 39              .9      LDX #&39
9E1A  4C BD 9D           L..     JMP &9DBD
.
9E1D  20 94 BD            ..     JSR &BD94
.
9E20  20 EC AD            ..     JSR &ADEC
.
9E23  48                 H       PHA
.
9E24  A4 1B              ..      LDY &1B
9E26  E6 1B              ..      INC &1B
9E28  B1 19              ..      LDA (&19),Y
9E2A  C9 20              .       CMP #&20
9E2C  F0 F6              ..      BEQ &9E24
9E2E  AA                 .       TAX
9E2F  68                 h       PLA
9E30  E0 5E              .^      CPX #&5E
9E32  F0 01              ..      BEQ &9E35
9E34  60                 `       RTS
.
9E35  A8                 .       TAY
9E36  20 FD 92            ..     JSR &92FD
9E39  20 51 BD            Q.     JSR &BD51
9E3C  20 FA 92            ..     JSR &92FA
9E3F  A5 30              .0      LDA &30
9E41  C9 87              ..      CMP #&87
9E43  B0 43              .C      BCS &9E88
9E45  20 86 A4            ..     JSR &A486
9E48  D0 0F              ..      BNE &9E59
9E4A  20 7E BD            ~.     JSR &BD7E
9E4D  20 B5 A3            ..     JSR &A3B5
9E50  A5 4A              .J      LDA &4A
9E52  20 12 AB            ..     JSR &AB12
9E55  A9 FF              ..      LDA #&FF
9E57  D0 CA              ..      BNE &9E23
.
9E59  20 81 A3            ..     JSR &A381
9E5C  A5 04              ..      LDA &04
9E5E  85 4B              .K      STA &4B
9E60  A5 05              ..      LDA &05
9E62  85 4C              .L      STA &4C
9E64  20 B5 A3            ..     JSR &A3B5
9E67  A5 4A              .J      LDA &4A
9E69  20 12 AB            ..     JSR &AB12
.
9E6C  20 7D A3            }.     JSR &A37D
9E6F  20 7E BD            ~.     JSR &BD7E
9E72  20 B5 A3            ..     JSR &A3B5
9E75  20 01 A8            ..     JSR &A801
9E78  20 D1 AA            ..     JSR &AAD1
9E7B  20 94 AA            ..     JSR &AA94
9E7E  20 ED A7            ..     JSR &A7ED
9E81  20 56 A6            V.     JSR &A656
9E84  A9 FF              ..      LDA #&FF
9E86  D0 9B              ..      BNE &9E23
.
9E88  20 81 A3            ..     JSR &A381
9E8B  20 99 A6            ..     JSR &A699
9E8E  D0 DC              ..      BNE &9E6C
.
9E90  98                 .       TYA
9E91  10 03              ..      BPL &9E96
9E93  20 E4 A3            ..     JSR &A3E4
.
9E96  A2 00              ..      LDX #&00
9E98  A0 00              ..      LDY #&00
.
9E9A  B9 2A 00           .*.     LDA &002A,Y
9E9D  48                 H       PHA
9E9E  29 0F              ).      AND #&0F
9EA0  95 3F              .?      STA &3F,X
9EA2  68                 h       PLA
9EA3  4A                 J       LSR A
9EA4  4A                 J       LSR A
9EA5  4A                 J       LSR A
9EA6  4A                 J       LSR A
9EA7  E8                 .       INX
9EA8  95 3F              .?      STA &3F,X
9EAA  E8                 .       INX
9EAB  C8                 .       INY
9EAC  C0 04              ..      CPY #&04
9EAE  D0 EA              ..      BNE &9E9A
.
9EB0  CA                 .       DEX
9EB1  F0 04              ..      BEQ &9EB7
9EB3  B5 3F              .?      LDA &3F,X
9EB5  F0 F9              ..      BEQ &9EB0
.
9EB7  B5 3F              .?      LDA &3F,X
9EB9  C9 0A              ..      CMP #&0A
9EBB  90 02              ..      BCC &9EBF
9EBD  69 06              i.      ADC #&06
.
9EBF  69 30              i0      ADC #&30
9EC1  20 66 A0            f.     JSR &A066
9EC4  CA                 .       DEX
9EC5  10 F0              ..      BPL &9EB7
9EC7  60                 `       RTS
.
9EC8  10 07              ..      BPL &9ED1
9ECA  A9 2D              .-      LDA #&2D
9ECC  85 2E              ..      STA &2E
9ECE  20 66 A0            f.     JSR &A066
.
9ED1  A5 30              .0      LDA &30
9ED3  C9 81              ..      CMP #&81
9ED5  B0 4E              .N      BCS &9F25
9ED7  20 F4 A1            ..     JSR &A1F4
9EDA  C6 49              .I      DEC &49
9EDC  4C D1 9E           L..     JMP &9ED1
.
9EDF  AE 02 04           ...     LDX &0402
9EE2  E0 03              ..      CPX #&03
9EE4  90 02              ..      BCC &9EE8
9EE6  A2 00              ..      LDX #&00
.
9EE8  86 37              .7      STX &37
9EEA  AD 01 04           ...     LDA &0401
9EED  F0 06              ..      BEQ &9EF5
9EEF  C9 0A              ..      CMP #&0A
9EF1  B0 06              ..      BCS &9EF9
9EF3  90 06              ..      BCC &9EFB
.
9EF5  E0 02              ..      CPX #&02
9EF7  F0 02              ..      BEQ &9EFB
.
9EF9  A9 0A              ..      LDA #&0A
.
9EFB  85 38              .8      STA &38
9EFD  85 4E              .N      STA &4E
9EFF  A9 00              ..      LDA #&00
9F01  85 36              .6      STA &36
9F03  85 49              .I      STA &49
9F05  24 15              $.      BIT &15
9F07  30 87              0.      BMI &9E90
9F09  98                 .       TYA
9F0A  30 03              0.      BMI &9F0F
9F0C  20 BE A2            ..     JSR &A2BE
.
9F0F  20 DA A1            ..     JSR &A1DA
9F12  D0 B4              ..      BNE &9EC8
9F14  A5 37              .7      LDA &37
9F16  D0 05              ..      BNE &9F1D
9F18  A9 30              .0      LDA #&30
9F1A  4C 66 A0           Lf.     JMP &A066
.
9F1D  4C 9C 9F           L..     JMP &9F9C
.
9F20  20 99 A6            ..     JSR &A699
9F23  D0 0F              ..      BNE &9F34
.
9F25  C9 84              ..      CMP #&84
9F27  90 10              ..      BCC &9F39
9F29  D0 06              ..      BNE &9F31
9F2B  A5 31              .1      LDA &31
9F2D  C9 A0              ..      CMP #&A0
9F2F  90 08              ..      BCC &9F39
.
9F31  20 4D A2            M.     JSR &A24D
.
9F34  E6 49              .I      INC &49
9F36  4C D1 9E           L..     JMP &9ED1
.
9F39  A5 35              .5      LDA &35
9F3B  85 27              .'      STA &27
9F3D  20 85 A3            ..     JSR &A385
9F40  A5 4E              .N      LDA &4E
9F42  85 38              .8      STA &38
9F44  A6 37              .7      LDX &37
9F46  E0 02              ..      CPX #&02
9F48  D0 12              ..      BNE &9F5C
9F4A  65 49              eI      ADC &49
9F4C  30 52              0R      BMI &9FA0
9F4E  85 38              .8      STA &38
9F50  C9 0B              ..      CMP #&0B
9F52  90 08              ..      BCC &9F5C
9F54  A9 0A              ..      LDA #&0A
9F56  85 38              .8      STA &38
9F58  A9 00              ..      LDA #&00
9F5A  85 37              .7      STA &37
.
9F5C  20 86 A6            ..     JSR &A686
9F5F  A9 A0              ..      LDA #&A0
9F61  85 31              .1      STA &31
9F63  A9 83              ..      LDA #&83
9F65  85 30              .0      STA &30
9F67  A6 38              .8      LDX &38
9F69  F0 06              ..      BEQ &9F71
.
9F6B  20 4D A2            M.     JSR &A24D
9F6E  CA                 .       DEX
9F6F  D0 FA              ..      BNE &9F6B
.
9F71  20 F5 A7            ..     JSR &A7F5
9F74  20 4E A3            N.     JSR &A34E
9F77  A5 27              .'      LDA &27
9F79  85 42              .B      STA &42
9F7B  20 0B A5            ..     JSR &A50B
.
9F7E  A5 30              .0      LDA &30
9F80  C9 84              ..      CMP #&84
9F82  B0 0E              ..      BCS &9F92
9F84  66 31              f1      ROR &31
9F86  66 32              f2      ROR &32
9F88  66 33              f3      ROR &33
9F8A  66 34              f4      ROR &34
9F8C  66 35              f5      ROR &35
9F8E  E6 30              .0      INC &30
9F90  D0 EC              ..      BNE &9F7E
.
9F92  A5 31              .1      LDA &31
9F94  C9 A0              ..      CMP #&A0
9F96  B0 88              ..      BCS &9F20
9F98  A5 38              .8      LDA &38
9F9A  D0 11              ..      BNE &9FAD
.
9F9C  C9 01              ..      CMP #&01
9F9E  F0 46              .F      BEQ &9FE6
.
9FA0  20 86 A6            ..     JSR &A686
9FA3  A9 00              ..      LDA #&00
9FA5  85 49              .I      STA &49
9FA7  A5 4E              .N      LDA &4E
9FA9  85 38              .8      STA &38
9FAB  E6 38              .8      INC &38
.
9FAD  A9 01              ..      LDA #&01
9FAF  C5 37              .7      CMP &37
9FB1  F0 33              .3      BEQ &9FE6
9FB3  A4 49              .I      LDY &49
9FB5  30 0C              0.      BMI &9FC3
9FB7  C4 38              .8      CPY &38
9FB9  B0 2B              .+      BCS &9FE6
9FBB  A9 00              ..      LDA #&00
9FBD  85 49              .I      STA &49
9FBF  C8                 .       INY
9FC0  98                 .       TYA
9FC1  D0 23              .#      BNE &9FE6
.
9FC3  A5 37              .7      LDA &37
9FC5  C9 02              ..      CMP #&02
9FC7  F0 06              ..      BEQ &9FCF
9FC9  A9 01              ..      LDA #&01
9FCB  C0 FF              ..      CPY #&FF
9FCD  D0 17              ..      BNE &9FE6
.
9FCF  A9 30              .0      LDA #&30
9FD1  20 66 A0            f.     JSR &A066
9FD4  A9 2E              ..      LDA #&2E
9FD6  20 66 A0            f.     JSR &A066
9FD9  A9 30              .0      LDA #&30
.
9FDB  E6 49              .I      INC &49
9FDD  F0 05              ..      BEQ &9FE4
9FDF  20 66 A0            f.     JSR &A066
9FE2  D0 F7              ..      BNE &9FDB
.
9FE4  A9 80              ..      LDA #&80
.
9FE6  85 4E              .N      STA &4E
.
9FE8  20 40 A0            @.     JSR &A040
9FEB  C6 4E              .N      DEC &4E
9FED  D0 05              ..      BNE &9FF4
9FEF  A9 2E              ..      LDA #&2E
9FF1  20 66 A0            f.     JSR &A066
.
9FF4  C6 38              .8      DEC &38
9FF6  D0 F0              ..      BNE &9FE8
9FF8  A4 37              .7      LDY &37
9FFA  88                 .       DEY
9FFB  F0 18              ..      BEQ &A015
9FFD  88                 .       DEY
9FFE  F0 11              ..      BEQ &A011
A000  A4 36              .6      LDY &36
.
A002  88                 .       DEY
A003  B9 00 06           ...     LDA &0600,Y
A006  C9 30              .0      CMP #&30
A008  F0 F8              ..      BEQ &A002
A00A  C9 2E              ..      CMP #&2E
A00C  F0 01              ..      BEQ &A00F
A00E  C8                 .       INY
.
A00F  84 36              .6      STY &36
.
A011  A5 49              .I      LDA &49
A013  F0 2A              .*      BEQ &A03F
.
A015  A9 45              .E      LDA #&45
A017  20 66 A0            f.     JSR &A066
A01A  A5 49              .I      LDA &49
A01C  10 0A              ..      BPL &A028
A01E  A9 2D              .-      LDA #&2D
A020  20 66 A0            f.     JSR &A066
A023  38                 8       SEC
A024  A9 00              ..      LDA #&00
A026  E5 49              .I      SBC &49
.
A028  20 52 A0            R.     JSR &A052
A02B  A5 37              .7      LDA &37
A02D  F0 10              ..      BEQ &A03F
A02F  A9 20              .       LDA #&20
A031  A4 49              .I      LDY &49
A033  30 03              0.      BMI &A038
A035  20 66 A0            f.     JSR &A066
.
A038  E0 00              ..      CPX #&00
A03A  D0 03              ..      BNE &A03F
A03C  4C 66 A0           Lf.     JMP &A066
.
A03F  60                 `       RTS
.
A040  A5 31              .1      LDA &31
A042  4A                 J       LSR A
A043  4A                 J       LSR A
A044  4A                 J       LSR A
A045  4A                 J       LSR A
A046  20 64 A0            d.     JSR &A064
A049  A5 31              .1      LDA &31
A04B  29 0F              ).      AND #&0F
A04D  85 31              .1      STA &31
A04F  4C 97 A1           L..     JMP &A197
.
A052  A2 FF              ..      LDX #&FF
A054  38                 8       SEC
.
A055  E8                 .       INX
A056  E9 0A              ..      SBC #&0A
A058  B0 FB              ..      BCS &A055
A05A  69 0A              i.      ADC #&0A
A05C  48                 H       PHA
A05D  8A                 .       TXA
A05E  F0 03              ..      BEQ &A063
A060  20 64 A0            d.     JSR &A064
.
A063  68                 h       PLA
.
A064  09 30              .0      ORA #&30
.
A066  86 3B              .;      STX &3B
A068  A6 36              .6      LDX &36
A06A  9D 00 06           ...     STA &0600,X
A06D  A6 3B              .;      LDX &3B
A06F  E6 36              .6      INC &36
A071  60                 `       RTS
.
A072  18                 .       CLC
A073  86 35              .5      STX &35
A075  20 DA A1            ..     JSR &A1DA
A078  A9 FF              ..      LDA #&FF
A07A  60                 `       RTS
.
A07B  A2 00              ..      LDX #&00
A07D  86 31              .1      STX &31
A07F  86 32              .2      STX &32
A081  86 33              .3      STX &33
A083  86 34              .4      STX &34
A085  86 35              .5      STX &35
A087  86 48              .H      STX &48
A089  86 49              .I      STX &49
A08B  C9 2E              ..      CMP #&2E
A08D  F0 11              ..      BEQ &A0A0
A08F  C9 3A              .:      CMP #&3A
A091  B0 DF              ..      BCS &A072
A093  E9 2F              ./      SBC #&2F
A095  30 DB              0.      BMI &A072
A097  85 35              .5      STA &35
.
A099  C8                 .       INY
A09A  B1 19              ..      LDA (&19),Y
A09C  C9 2E              ..      CMP #&2E
A09E  D0 08              ..      BNE &A0A8
.
A0A0  A5 48              .H      LDA &48
A0A2  D0 44              .D      BNE &A0E8
A0A4  E6 48              .H      INC &48
A0A6  D0 F1              ..      BNE &A099
.
A0A8  C9 45              .E      CMP #&45
A0AA  F0 35              .5      BEQ &A0E1
A0AC  C9 3A              .:      CMP #&3A
A0AE  B0 38              .8      BCS &A0E8
A0B0  E9 2F              ./      SBC #&2F
A0B2  90 34              .4      BCC &A0E8
A0B4  A6 31              .1      LDX &31
A0B6  E0 18              ..      CPX #&18
A0B8  90 08              ..      BCC &A0C2
A0BA  A6 48              .H      LDX &48
A0BC  D0 DB              ..      BNE &A099
A0BE  E6 49              .I      INC &49
A0C0  B0 D7              ..      BCS &A099
.
A0C2  A6 48              .H      LDX &48
A0C4  F0 02              ..      BEQ &A0C8
A0C6  C6 49              .I      DEC &49
.
A0C8  20 97 A1            ..     JSR &A197
A0CB  65 35              e5      ADC &35
A0CD  85 35              .5      STA &35
A0CF  90 C8              ..      BCC &A099
A0D1  E6 34              .4      INC &34
A0D3  D0 C4              ..      BNE &A099
A0D5  E6 33              .3      INC &33
A0D7  D0 C0              ..      BNE &A099
A0D9  E6 32              .2      INC &32
A0DB  D0 BC              ..      BNE &A099
A0DD  E6 31              .1      INC &31
A0DF  D0 B8              ..      BNE &A099
.
A0E1  20 40 A1            @.     JSR &A140
A0E4  65 49              eI      ADC &49
A0E6  85 49              .I      STA &49
.
A0E8  84 1B              ..      STY &1B
A0EA  A5 49              .I      LDA &49
A0EC  05 48              .H      ORA &48
A0EE  F0 2F              ./      BEQ &A11F
A0F0  20 DA A1            ..     JSR &A1DA
A0F3  F0 26              .&      BEQ &A11B
.
A0F5  A9 A8              ..      LDA #&A8
A0F7  85 30              .0      STA &30
A0F9  A9 00              ..      LDA #&00
A0FB  85 2F              ./      STA &2F
A0FD  85 2E              ..      STA &2E
A0FF  20 03 A3            ..     JSR &A303
A102  A5 49              .I      LDA &49
A104  30 0B              0.      BMI &A111
A106  F0 10              ..      BEQ &A118
.
A108  20 F4 A1            ..     JSR &A1F4
A10B  C6 49              .I      DEC &49
A10D  D0 F9              ..      BNE &A108
A10F  F0 07              ..      BEQ &A118
.
A111  20 4D A2            M.     JSR &A24D
A114  E6 49              .I      INC &49
A116  D0 F9              ..      BNE &A111
.
A118  20 5C A6            \.     JSR &A65C
.
A11B  38                 8       SEC
A11C  A9 FF              ..      LDA #&FF
A11E  60                 `       RTS
.
A11F  A5 32              .2      LDA &32
A121  85 2D              .-      STA &2D
A123  29 80              ).      AND #&80
A125  05 31              .1      ORA &31
A127  D0 CC              ..      BNE &A0F5
A129  A5 35              .5      LDA &35
A12B  85 2A              .*      STA &2A
A12D  A5 34              .4      LDA &34
A12F  85 2B              .+      STA &2B
A131  A5 33              .3      LDA &33
A133  85 2C              .,      STA &2C
A135  A9 40              .@      LDA #&40
A137  38                 8       SEC
A138  60                 `       RTS
.
A139  20 4B A1            K.     JSR &A14B
A13C  49 FF              I.      EOR #&FF
A13E  38                 8       SEC
A13F  60                 `       RTS
.
A140  C8                 .       INY
A141  B1 19              ..      LDA (&19),Y
A143  C9 2D              .-      CMP #&2D
A145  F0 F2              ..      BEQ &A139
A147  C9 2B              .+      CMP #&2B
A149  D0 03              ..      BNE &A14E
.
A14B  C8                 .       INY
A14C  B1 19              ..      LDA (&19),Y
.
A14E  C9 3A              .:      CMP #&3A
A150  B0 22              ."      BCS &A174
A152  E9 2F              ./      SBC #&2F
A154  90 1E              ..      BCC &A174
A156  85 4A              .J      STA &4A
A158  C8                 .       INY
A159  B1 19              ..      LDA (&19),Y
A15B  C9 3A              .:      CMP #&3A
A15D  B0 11              ..      BCS &A170
A15F  E9 2F              ./      SBC #&2F
A161  90 0D              ..      BCC &A170
A163  C8                 .       INY
A164  85 43              .C      STA &43
A166  A5 4A              .J      LDA &4A
A168  0A                 .       ASL A
A169  0A                 .       ASL A
A16A  65 4A              eJ      ADC &4A
A16C  0A                 .       ASL A
A16D  65 43              eC      ADC &43
A16F  60                 `       RTS
.
A170  A5 4A              .J      LDA &4A
A172  18                 .       CLC
A173  60                 `       RTS
.
A174  A9 00              ..      LDA #&00
A176  18                 .       CLC
A177  60                 `       RTS
.
A178  A5 35              .5      LDA &35
A17A  65 42              eB      ADC &42
A17C  85 35              .5      STA &35
A17E  A5 34              .4      LDA &34
A180  65 41              eA      ADC &41
A182  85 34              .4      STA &34
A184  A5 33              .3      LDA &33
A186  65 40              e@      ADC &40
A188  85 33              .3      STA &33
A18A  A5 32              .2      LDA &32
A18C  65 3F              e?      ADC &3F
A18E  85 32              .2      STA &32
A190  A5 31              .1      LDA &31
A192  65 3E              e>      ADC &3E
A194  85 31              .1      STA &31
A196  60                 `       RTS
.
A197  48                 H       PHA
A198  A6 34              .4      LDX &34
A19A  A5 31              .1      LDA &31
A19C  48                 H       PHA
A19D  A5 32              .2      LDA &32
A19F  48                 H       PHA
A1A0  A5 33              .3      LDA &33
A1A2  48                 H       PHA
A1A3  A5 35              .5      LDA &35
A1A5  0A                 .       ASL A
A1A6  26 34              &4      ROL &34
A1A8  26 33              &3      ROL &33
A1AA  26 32              &2      ROL &32
A1AC  26 31              &1      ROL &31
A1AE  0A                 .       ASL A
A1AF  26 34              &4      ROL &34
A1B1  26 33              &3      ROL &33
A1B3  26 32              &2      ROL &32
A1B5  26 31              &1      ROL &31
A1B7  65 35              e5      ADC &35
A1B9  85 35              .5      STA &35
A1BB  8A                 .       TXA
A1BC  65 34              e4      ADC &34
A1BE  85 34              .4      STA &34
A1C0  68                 h       PLA
A1C1  65 33              e3      ADC &33
A1C3  85 33              .3      STA &33
A1C5  68                 h       PLA
A1C6  65 32              e2      ADC &32
A1C8  85 32              .2      STA &32
A1CA  68                 h       PLA
A1CB  65 31              e1      ADC &31
A1CD  06 35              .5      ASL &35
A1CF  26 34              &4      ROL &34
A1D1  26 33              &3      ROL &33
A1D3  26 32              &2      ROL &32
A1D5  2A                 *       ROL A
A1D6  85 31              .1      STA &31
A1D8  68                 h       PLA
A1D9  60                 `       RTS
.
A1DA  A5 31              .1      LDA &31
A1DC  05 32              .2      ORA &32
A1DE  05 33              .3      ORA &33
A1E0  05 34              .4      ORA &34
A1E2  05 35              .5      ORA &35
A1E4  F0 07              ..      BEQ &A1ED
A1E6  A5 2E              ..      LDA &2E
A1E8  D0 09              ..      BNE &A1F3
A1EA  A9 01              ..      LDA #&01
A1EC  60                 `       RTS
.
A1ED  85 2E              ..      STA &2E
A1EF  85 30              .0      STA &30
A1F1  85 2F              ./      STA &2F
.
A1F3  60                 `       RTS
.
A1F4  18                 .       CLC
A1F5  A5 30              .0      LDA &30
A1F7  69 03              i.      ADC #&03
A1F9  85 30              .0      STA &30
A1FB  90 02              ..      BCC &A1FF
A1FD  E6 2F              ./      INC &2F
.
A1FF  20 1E A2            ..     JSR &A21E
A202  20 42 A2            B.     JSR &A242
A205  20 42 A2            B.     JSR &A242
.
A208  20 78 A1            x.     JSR &A178
.
A20B  90 10              ..      BCC &A21D
A20D  66 31              f1      ROR &31
A20F  66 32              f2      ROR &32
A211  66 33              f3      ROR &33
A213  66 34              f4      ROR &34
A215  66 35              f5      ROR &35
A217  E6 30              .0      INC &30
A219  D0 02              ..      BNE &A21D
A21B  E6 2F              ./      INC &2F
.
A21D  60                 `       RTS
.
A21E  A5 2E              ..      LDA &2E
.
A220  85 3B              .;      STA &3B
A222  A5 2F              ./      LDA &2F
A224  85 3C              .<      STA &3C
A226  A5 30              .0      LDA &30
A228  85 3D              .=      STA &3D
A22A  A5 31              .1      LDA &31
A22C  85 3E              .>      STA &3E
A22E  A5 32              .2      LDA &32
A230  85 3F              .?      STA &3F
A232  A5 33              .3      LDA &33
A234  85 40              .@      STA &40
A236  A5 34              .4      LDA &34
A238  85 41              .A      STA &41
A23A  A5 35              .5      LDA &35
A23C  85 42              .B      STA &42
A23E  60                 `       RTS
.
A23F  20 1E A2            ..     JSR &A21E
.
A242  46 3E              F>      LSR &3E
A244  66 3F              f?      ROR &3F
A246  66 40              f@      ROR &40
A248  66 41              fA      ROR &41
A24A  66 42              fB      ROR &42
A24C  60                 `       RTS
.
A24D  38                 8       SEC
A24E  A5 30              .0      LDA &30
A250  E9 04              ..      SBC #&04
A252  85 30              .0      STA &30
A254  B0 02              ..      BCS &A258
A256  C6 2F              ./      DEC &2F
.
A258  20 3F A2            ?.     JSR &A23F
A25B  20 08 A2            ..     JSR &A208
A25E  20 3F A2            ?.     JSR &A23F
A261  20 42 A2            B.     JSR &A242
A264  20 42 A2            B.     JSR &A242
A267  20 42 A2            B.     JSR &A242
A26A  20 08 A2            ..     JSR &A208
A26D  A9 00              ..      LDA #&00
A26F  85 3E              .>      STA &3E
A271  A5 31              .1      LDA &31
A273  85 3F              .?      STA &3F
A275  A5 32              .2      LDA &32
A277  85 40              .@      STA &40
A279  A5 33              .3      LDA &33
A27B  85 41              .A      STA &41
A27D  A5 34              .4      LDA &34
A27F  85 42              .B      STA &42
A281  A5 35              .5      LDA &35
A283  2A                 *       ROL A
A284  20 08 A2            ..     JSR &A208
A287  A9 00              ..      LDA #&00
A289  85 3E              .>      STA &3E
A28B  85 3F              .?      STA &3F
A28D  A5 31              .1      LDA &31
A28F  85 40              .@      STA &40
A291  A5 32              .2      LDA &32
A293  85 41              .A      STA &41
A295  A5 33              .3      LDA &33
A297  85 42              .B      STA &42
A299  A5 34              .4      LDA &34
A29B  2A                 *       ROL A
A29C  20 08 A2            ..     JSR &A208
A29F  A5 32              .2      LDA &32
A2A1  2A                 *       ROL A
A2A2  A5 31              .1      LDA &31
.
A2A4  65 35              e5      ADC &35
A2A6  85 35              .5      STA &35
A2A8  90 13              ..      BCC &A2BD
A2AA  E6 34              .4      INC &34
A2AC  D0 0F              ..      BNE &A2BD
A2AE  E6 33              .3      INC &33
A2B0  D0 0B              ..      BNE &A2BD
A2B2  E6 32              .2      INC &32
A2B4  D0 07              ..      BNE &A2BD
A2B6  E6 31              .1      INC &31
A2B8  D0 03              ..      BNE &A2BD
A2BA  4C 0B A2           L..     JMP &A20B
.
A2BD  60                 `       RTS
.
A2BE  A2 00              ..      LDX #&00
A2C0  86 35              .5      STX &35
A2C2  86 2F              ./      STX &2F
A2C4  A5 2D              .-      LDA &2D
A2C6  10 05              ..      BPL &A2CD
A2C8  20 93 AD            ..     JSR &AD93
A2CB  A2 FF              ..      LDX #&FF
.
A2CD  86 2E              ..      STX &2E
A2CF  A5 2A              .*      LDA &2A
A2D1  85 34              .4      STA &34
A2D3  A5 2B              .+      LDA &2B
A2D5  85 33              .3      STA &33
A2D7  A5 2C              .,      LDA &2C
A2D9  85 32              .2      STA &32
A2DB  A5 2D              .-      LDA &2D
A2DD  85 31              .1      STA &31
A2DF  A9 A0              ..      LDA #&A0
A2E1  85 30              .0      STA &30
A2E3  4C 03 A3           L..     JMP &A303
.
A2E6  85 2E              ..      STA &2E
A2E8  85 30              .0      STA &30
A2EA  85 2F              ./      STA &2F
.
A2EC  60                 `       RTS
.
A2ED  48                 H       PHA
A2EE  20 86 A6            ..     JSR &A686
A2F1  68                 h       PLA
A2F2  F0 F8              ..      BEQ &A2EC
A2F4  10 07              ..      BPL &A2FD
A2F6  85 2E              ..      STA &2E
A2F8  A9 00              ..      LDA #&00
A2FA  38                 8       SEC
A2FB  E5 2E              ..      SBC &2E
.
A2FD  85 31              .1      STA &31
A2FF  A9 88              ..      LDA #&88
A301  85 30              .0      STA &30
.
A303  A5 31              .1      LDA &31
A305  30 E5              0.      BMI &A2EC
A307  05 32              .2      ORA &32
A309  05 33              .3      ORA &33
A30B  05 34              .4      ORA &34
A30D  05 35              .5      ORA &35
A30F  F0 D5              ..      BEQ &A2E6
A311  A5 30              .0      LDA &30
.
A313  A4 31              .1      LDY &31
A315  30 D5              0.      BMI &A2EC
A317  D0 21              .!      BNE &A33A
A319  A6 32              .2      LDX &32
A31B  86 31              .1      STX &31
A31D  A6 33              .3      LDX &33
A31F  86 32              .2      STX &32
A321  A6 34              .4      LDX &34
A323  86 33              .3      STX &33
A325  A6 35              .5      LDX &35
A327  86 34              .4      STX &34
A329  84 35              .5      STY &35
A32B  38                 8       SEC
A32C  E9 08              ..      SBC #&08
A32E  85 30              .0      STA &30
A330  B0 E1              ..      BCS &A313
A332  C6 2F              ./      DEC &2F
A334  90 DD              ..      BCC &A313
.
A336  A4 31              .1      LDY &31
A338  30 B2              0.      BMI &A2EC
.
A33A  06 35              .5      ASL &35
A33C  26 34              &4      ROL &34
A33E  26 33              &3      ROL &33
A340  26 32              &2      ROL &32
A342  26 31              &1      ROL &31
A344  E9 00              ..      SBC #&00
A346  85 30              .0      STA &30
A348  B0 EC              ..      BCS &A336
A34A  C6 2F              ./      DEC &2F
A34C  90 E8              ..      BCC &A336
.
A34E  A0 04              ..      LDY #&04
A350  B1 4B              .K      LDA (&4B),Y
A352  85 41              .A      STA &41
A354  88                 .       DEY
A355  B1 4B              .K      LDA (&4B),Y
A357  85 40              .@      STA &40
A359  88                 .       DEY
A35A  B1 4B              .K      LDA (&4B),Y
A35C  85 3F              .?      STA &3F
A35E  88                 .       DEY
A35F  B1 4B              .K      LDA (&4B),Y
A361  85 3B              .;      STA &3B
A363  88                 .       DEY
A364  84 42              .B      STY &42
A366  84 3C              .<      STY &3C
A368  B1 4B              .K      LDA (&4B),Y
A36A  85 3D              .=      STA &3D
A36C  05 3B              .;      ORA &3B
A36E  05 3F              .?      ORA &3F
A370  05 40              .@      ORA &40
A372  05 41              .A      ORA &41
A374  F0 04              ..      BEQ &A37A
A376  A5 3B              .;      LDA &3B
A378  09 80              ..      ORA #&80
.
A37A  85 3E              .>      STA &3E
A37C  60                 `       RTS
.
A37D  A9 71              .q      LDA #&71
A37F  D0 06              ..      BNE &A387
.
A381  A9 76              .v      LDA #&76
A383  D0 02              ..      BNE &A387
.
A385  A9 6C              .l      LDA #&6C
.
A387  85 4B              .K      STA &4B
A389  A9 04              ..      LDA #&04
A38B  85 4C              .L      STA &4C
.
A38D  A0 00              ..      LDY #&00
A38F  A5 30              .0      LDA &30
A391  91 4B              .K      STA (&4B),Y
A393  C8                 .       INY
A394  A5 2E              ..      LDA &2E
A396  29 80              ).      AND #&80
A398  85 2E              ..      STA &2E
A39A  A5 31              .1      LDA &31
A39C  29 7F              )      AND #&7F
A39E  05 2E              ..      ORA &2E
A3A0  91 4B              .K      STA (&4B),Y
A3A2  A5 32              .2      LDA &32
A3A4  C8                 .       INY
A3A5  91 4B              .K      STA (&4B),Y
A3A7  A5 33              .3      LDA &33
A3A9  C8                 .       INY
A3AA  91 4B              .K      STA (&4B),Y
A3AC  A5 34              .4      LDA &34
A3AE  C8                 .       INY
A3AF  91 4B              .K      STA (&4B),Y
A3B1  60                 `       RTS
.
A3B2  20 F5 A7            ..     JSR &A7F5
.
A3B5  A0 04              ..      LDY #&04
A3B7  B1 4B              .K      LDA (&4B),Y
A3B9  85 34              .4      STA &34
A3BB  88                 .       DEY
A3BC  B1 4B              .K      LDA (&4B),Y
A3BE  85 33              .3      STA &33
A3C0  88                 .       DEY
A3C1  B1 4B              .K      LDA (&4B),Y
A3C3  85 32              .2      STA &32
A3C5  88                 .       DEY
A3C6  B1 4B              .K      LDA (&4B),Y
A3C8  85 2E              ..      STA &2E
A3CA  88                 .       DEY
A3CB  B1 4B              .K      LDA (&4B),Y
A3CD  85 30              .0      STA &30
A3CF  84 35              .5      STY &35
A3D1  84 2F              ./      STY &2F
A3D3  05 2E              ..      ORA &2E
A3D5  05 32              .2      ORA &32
A3D7  05 33              .3      ORA &33
A3D9  05 34              .4      ORA &34
A3DB  F0 04              ..      BEQ &A3E1
A3DD  A5 2E              ..      LDA &2E
A3DF  09 80              ..      ORA #&80
.
A3E1  85 31              .1      STA &31
A3E3  60                 `       RTS
.
A3E4  20 FE A3            ..     JSR &A3FE
.
A3E7  A5 31              .1      LDA &31
A3E9  85 2D              .-      STA &2D
A3EB  A5 32              .2      LDA &32
A3ED  85 2C              .,      STA &2C
A3EF  A5 33              .3      LDA &33
A3F1  85 2B              .+      STA &2B
A3F3  A5 34              .4      LDA &34
A3F5  85 2A              .*      STA &2A
A3F7  60                 `       RTS
.
A3F8  20 1E A2            ..     JSR &A21E
A3FB  4C 86 A6           L..     JMP &A686
.
A3FE  A5 30              .0      LDA &30
A400  10 F6              ..      BPL &A3F8
A402  20 53 A4            S.     JSR &A453
A405  20 DA A1            ..     JSR &A1DA
A408  D0 32              .2      BNE &A43C
A40A  F0 5C              .\      BEQ &A468
.
A40C  A5 30              .0      LDA &30
A40E  C9 A0              ..      CMP #&A0
A410  B0 54              .T      BCS &A466
A412  C9 99              ..      CMP #&99
A414  B0 26              .&      BCS &A43C
A416  69 08              i.      ADC #&08
A418  85 30              .0      STA &30
A41A  A5 40              .@      LDA &40
A41C  85 41              .A      STA &41
A41E  A5 3F              .?      LDA &3F
A420  85 40              .@      STA &40
A422  A5 3E              .>      LDA &3E
A424  85 3F              .?      STA &3F
A426  A5 34              .4      LDA &34
A428  85 3E              .>      STA &3E
A42A  A5 33              .3      LDA &33
A42C  85 34              .4      STA &34
A42E  A5 32              .2      LDA &32
A430  85 33              .3      STA &33
A432  A5 31              .1      LDA &31
A434  85 32              .2      STA &32
A436  A9 00              ..      LDA #&00
A438  85 31              .1      STA &31
A43A  F0 D0              ..      BEQ &A40C
.
A43C  46 31              F1      LSR &31
A43E  66 32              f2      ROR &32
A440  66 33              f3      ROR &33
A442  66 34              f4      ROR &34
A444  66 3E              f>      ROR &3E
A446  66 3F              f?      ROR &3F
A448  66 40              f@      ROR &40
A44A  66 41              fA      ROR &41
A44C  E6 30              .0      INC &30
A44E  D0 BC              ..      BNE &A40C
.
A450  4C 6C A6           Ll.     JMP &A66C
.
A453  A9 00              ..      LDA #&00
A455  85 3B              .;      STA &3B
A457  85 3C              .<      STA &3C
A459  85 3D              .=      STA &3D
A45B  85 3E              .>      STA &3E
A45D  85 3F              .?      STA &3F
A45F  85 40              .@      STA &40
A461  85 41              .A      STA &41
A463  85 42              .B      STA &42
A465  60                 `       RTS
.
A466  D0 E8              ..      BNE &A450
.
A468  A5 2E              ..      LDA &2E
A46A  10 19              ..      BPL &A485
.
A46C  38                 8       SEC
A46D  A9 00              ..      LDA #&00
A46F  E5 34              .4      SBC &34
A471  85 34              .4      STA &34
A473  A9 00              ..      LDA #&00
A475  E5 33              .3      SBC &33
A477  85 33              .3      STA &33
A479  A9 00              ..      LDA #&00
A47B  E5 32              .2      SBC &32
A47D  85 32              .2      STA &32
A47F  A9 00              ..      LDA #&00
A481  E5 31              .1      SBC &31
A483  85 31              .1      STA &31
.
A485  60                 `       RTS
.
A486  A5 30              .0      LDA &30
A488  30 07              0.      BMI &A491
A48A  A9 00              ..      LDA #&00
A48C  85 4A              .J      STA &4A
A48E  4C DA A1           L..     JMP &A1DA
.
A491  20 FE A3            ..     JSR &A3FE
A494  A5 34              .4      LDA &34
A496  85 4A              .J      STA &4A
A498  20 E8 A4            ..     JSR &A4E8
A49B  A9 80              ..      LDA #&80
A49D  85 30              .0      STA &30
A49F  A6 31              .1      LDX &31
A4A1  10 10              ..      BPL &A4B3
A4A3  45 2E              E.      EOR &2E
A4A5  85 2E              ..      STA &2E
A4A7  10 05              ..      BPL &A4AE
A4A9  E6 4A              .J      INC &4A
A4AB  4C B0 A4           L..     JMP &A4B0
.
A4AE  C6 4A              .J      DEC &4A
.
A4B0  20 6C A4            l.     JSR &A46C
.
A4B3  4C 03 A3           L..     JMP &A303
.
A4B6  E6 34              .4      INC &34
A4B8  D0 0C              ..      BNE &A4C6
A4BA  E6 33              .3      INC &33
A4BC  D0 08              ..      BNE &A4C6
A4BE  E6 32              .2      INC &32
A4C0  D0 04              ..      BNE &A4C6
A4C2  E6 31              .1      INC &31
A4C4  F0 8A              ..      BEQ &A450
.
A4C6  60                 `       RTS
.
A4C7  20 6C A4            l.     JSR &A46C
A4CA  20 B6 A4            ..     JSR &A4B6
A4CD  4C 6C A4           Ll.     JMP &A46C
.
A4D0  20 FD A4            ..     JSR &A4FD
A4D3  4C 7E AD           L~.     JMP &AD7E
.
A4D6  20 4E A3            N.     JSR &A34E
A4D9  20 8D A3            ..     JSR &A38D
.
A4DC  A5 3B              .;      LDA &3B
A4DE  85 2E              ..      STA &2E
A4E0  A5 3C              .<      LDA &3C
A4E2  85 2F              ./      STA &2F
A4E4  A5 3D              .=      LDA &3D
A4E6  85 30              .0      STA &30
.
A4E8  A5 3E              .>      LDA &3E
A4EA  85 31              .1      STA &31
A4EC  A5 3F              .?      LDA &3F
A4EE  85 32              .2      STA &32
A4F0  A5 40              .@      LDA &40
A4F2  85 33              .3      STA &33
A4F4  A5 41              .A      LDA &41
A4F6  85 34              .4      STA &34
A4F8  A5 42              .B      LDA &42
A4FA  85 35              .5      STA &35
.
A4FC  60                 `       RTS
.
A4FD  20 7E AD            ~.     JSR &AD7E
.
A500  20 4E A3            N.     JSR &A34E
A503  F0 F7              ..      BEQ &A4FC
.
A505  20 0B A5            ..     JSR &A50B
A508  4C 5C A6           L\.     JMP &A65C
.
A50B  20 DA A1            ..     JSR &A1DA
A50E  F0 CC              ..      BEQ &A4DC
A510  A0 00              ..      LDY #&00
A512  38                 8       SEC
A513  A5 30              .0      LDA &30
A515  E5 3D              .=      SBC &3D
A517  F0 77              .w      BEQ &A590
A519  90 37              .7      BCC &A552
A51B  C9 25              .%      CMP #&25
A51D  B0 DD              ..      BCS &A4FC
A51F  48                 H       PHA
A520  29 38              )8      AND #&38
A522  F0 19              ..      BEQ &A53D
A524  4A                 J       LSR A
A525  4A                 J       LSR A
A526  4A                 J       LSR A
A527  AA                 .       TAX
.
A528  A5 41              .A      LDA &41
A52A  85 42              .B      STA &42
A52C  A5 40              .@      LDA &40
A52E  85 41              .A      STA &41
A530  A5 3F              .?      LDA &3F
A532  85 40              .@      STA &40
A534  A5 3E              .>      LDA &3E
A536  85 3F              .?      STA &3F
A538  84 3E              .>      STY &3E
A53A  CA                 .       DEX
A53B  D0 EB              ..      BNE &A528
.
A53D  68                 h       PLA
A53E  29 07              ).      AND #&07
A540  F0 4E              .N      BEQ &A590
A542  AA                 .       TAX
.
A543  46 3E              F>      LSR &3E
A545  66 3F              f?      ROR &3F
A547  66 40              f@      ROR &40
A549  66 41              fA      ROR &41
A54B  66 42              fB      ROR &42
A54D  CA                 .       DEX
A54E  D0 F3              ..      BNE &A543
A550  F0 3E              .>      BEQ &A590
.
A552  38                 8       SEC
A553  A5 3D              .=      LDA &3D
A555  E5 30              .0      SBC &30
A557  C9 25              .%      CMP #&25
A559  B0 81              ..      BCS &A4DC
A55B  48                 H       PHA
A55C  29 38              )8      AND #&38
A55E  F0 19              ..      BEQ &A579
A560  4A                 J       LSR A
A561  4A                 J       LSR A
A562  4A                 J       LSR A
A563  AA                 .       TAX
.
A564  A5 34              .4      LDA &34
A566  85 35              .5      STA &35
A568  A5 33              .3      LDA &33
A56A  85 34              .4      STA &34
A56C  A5 32              .2      LDA &32
A56E  85 33              .3      STA &33
A570  A5 31              .1      LDA &31
A572  85 32              .2      STA &32
A574  84 31              .1      STY &31
A576  CA                 .       DEX
A577  D0 EB              ..      BNE &A564
.
A579  68                 h       PLA
A57A  29 07              ).      AND #&07
A57C  F0 0E              ..      BEQ &A58C
A57E  AA                 .       TAX
.
A57F  46 31              F1      LSR &31
A581  66 32              f2      ROR &32
A583  66 33              f3      ROR &33
A585  66 34              f4      ROR &34
A587  66 35              f5      ROR &35
A589  CA                 .       DEX
A58A  D0 F3              ..      BNE &A57F
.
A58C  A5 3D              .=      LDA &3D
A58E  85 30              .0      STA &30
.
A590  A5 2E              ..      LDA &2E
A592  45 3B              E;      EOR &3B
A594  10 49              .I      BPL &A5DF
A596  A5 31              .1      LDA &31
A598  C5 3E              .>      CMP &3E
A59A  D0 1B              ..      BNE &A5B7
A59C  A5 32              .2      LDA &32
A59E  C5 3F              .?      CMP &3F
A5A0  D0 15              ..      BNE &A5B7
A5A2  A5 33              .3      LDA &33
A5A4  C5 40              .@      CMP &40
A5A6  D0 0F              ..      BNE &A5B7
A5A8  A5 34              .4      LDA &34
A5AA  C5 41              .A      CMP &41
A5AC  D0 09              ..      BNE &A5B7
A5AE  A5 35              .5      LDA &35
A5B0  C5 42              .B      CMP &42
A5B2  D0 03              ..      BNE &A5B7
A5B4  4C 86 A6           L..     JMP &A686
.
A5B7  B0 2A              .*      BCS &A5E3
A5B9  38                 8       SEC
A5BA  A5 42              .B      LDA &42
A5BC  E5 35              .5      SBC &35
A5BE  85 35              .5      STA &35
A5C0  A5 41              .A      LDA &41
A5C2  E5 34              .4      SBC &34
A5C4  85 34              .4      STA &34
A5C6  A5 40              .@      LDA &40
A5C8  E5 33              .3      SBC &33
A5CA  85 33              .3      STA &33
A5CC  A5 3F              .?      LDA &3F
A5CE  E5 32              .2      SBC &32
A5D0  85 32              .2      STA &32
A5D2  A5 3E              .>      LDA &3E
A5D4  E5 31              .1      SBC &31
A5D6  85 31              .1      STA &31
A5D8  A5 3B              .;      LDA &3B
A5DA  85 2E              ..      STA &2E
A5DC  4C 03 A3           L..     JMP &A303
.
A5DF  18                 .       CLC
A5E0  4C 08 A2           L..     JMP &A208
.
A5E3  38                 8       SEC
A5E4  A5 35              .5      LDA &35
A5E6  E5 42              .B      SBC &42
A5E8  85 35              .5      STA &35
A5EA  A5 34              .4      LDA &34
A5EC  E5 41              .A      SBC &41
A5EE  85 34              .4      STA &34
A5F0  A5 33              .3      LDA &33
A5F2  E5 40              .@      SBC &40
A5F4  85 33              .3      STA &33
A5F6  A5 32              .2      LDA &32
A5F8  E5 3F              .?      SBC &3F
A5FA  85 32              .2      STA &32
A5FC  A5 31              .1      LDA &31
A5FE  E5 3E              .>      SBC &3E
A600  85 31              .1      STA &31
A602  4C 03 A3           L..     JMP &A303
.
A605  60                 `       RTS
.
A606  20 DA A1            ..     JSR &A1DA
A609  F0 FA              ..      BEQ &A605
A60B  20 4E A3            N.     JSR &A34E
A60E  D0 03              ..      BNE &A613
A610  4C 86 A6           L..     JMP &A686
.
A613  18                 .       CLC
A614  A5 30              .0      LDA &30
A616  65 3D              e=      ADC &3D
A618  90 03              ..      BCC &A61D
A61A  E6 2F              ./      INC &2F
A61C  18                 .       CLC
.
A61D  E9 7F              .      SBC #&7F
A61F  85 30              .0      STA &30
A621  B0 02              ..      BCS &A625
A623  C6 2F              ./      DEC &2F
.
A625  A2 05              ..      LDX #&05
A627  A0 00              ..      LDY #&00
.
A629  B5 30              .0      LDA &30,X
A62B  95 42              .B      STA &42,X
A62D  94 30              .0      STY &30
A62F  CA                 .       DEX
A630  D0 F7              ..      BNE &A629
A632  A5 2E              ..      LDA &2E
A634  45 3B              E;      EOR &3B
A636  85 2E              ..      STA &2E
A638  A0 20              .       LDY #&20
.
A63A  46 3E              F>      LSR &3E
A63C  66 3F              f?      ROR &3F
A63E  66 40              f@      ROR &40
A640  66 41              fA      ROR &41
A642  66 42              fB      ROR &42
A644  06 46              .F      ASL &46
A646  26 45              &E      ROL &45
A648  26 44              &D      ROL &44
A64A  26 43              &C      ROL &43
A64C  90 04              ..      BCC &A652
A64E  18                 .       CLC
A64F  20 78 A1            x.     JSR &A178
.
A652  88                 .       DEY
A653  D0 E5              ..      BNE &A63A
A655  60                 `       RTS
.
A656  20 06 A6            ..     JSR &A606
.
A659  20 03 A3            ..     JSR &A303
.
A65C  A5 35              .5      LDA &35
A65E  C9 80              ..      CMP #&80
A660  90 1A              ..      BCC &A67C
A662  F0 12              ..      BEQ &A676
A664  A9 FF              ..      LDA #&FF
A666  20 A4 A2            ..     JSR &A2A4
A669  4C 7C A6           L|.     JMP &A67C
.
A66C  00                 .       ...
A66D  14 54              .T      TRB &54
A66F  6F 6F              oo      BBR6 &A6E0
A671  20 62 69            bi     JSR &6962
A674  67 00              g.      RMB6 &00
.
A676  A5 34              .4      LDA &34
A678  09 01              ..      ORA #&01
A67A  85 34              .4      STA &34
.
A67C  A9 00              ..      LDA #&00
A67E  85 35              .5      STA &35
A680  A5 2F              ./      LDA &2F
A682  F0 14              ..      BEQ &A698
A684  10 E6              ..      BPL &A66C
.
A686  A9 00              ..      LDA #&00
A688  85 2E              ..      STA &2E
A68A  85 2F              ./      STA &2F
A68C  85 30              .0      STA &30
A68E  85 31              .1      STA &31
A690  85 32              .2      STA &32
A692  85 33              .3      STA &33
A694  85 34              .4      STA &34
A696  85 35              .5      STA &35
.
A698  60                 `       RTS
.
A699  20 86 A6            ..     JSR &A686
A69C  A0 80              ..      LDY #&80
A69E  84 31              .1      STY &31
A6A0  C8                 .       INY
A6A1  84 30              .0      STY &30
A6A3  98                 .       TYA
A6A4  60                 `       RTS
.
A6A5  20 85 A3            ..     JSR &A385
A6A8  20 99 A6            ..     JSR &A699
A6AB  D0 3A              .:      BNE &A6E7
.
A6AD  20 DA A1            ..     JSR &A1DA
A6B0  F0 09              ..      BEQ &A6BB
A6B2  20 1E A2            ..     JSR &A21E
A6B5  20 B5 A3            ..     JSR &A3B5
A6B8  D0 37              .7      BNE &A6F1
A6BA  60                 `       RTS
.
A6BB  4C A7 99           L..     JMP &99A7
A6BE  20 FA 92            ..     JSR &92FA
A6C1  20 D3 A9            ..     JSR &A9D3
A6C4  A5 4A              .J      LDA &4A
A6C6  48                 H       PHA
A6C7  20 E9 A7            ..     JSR &A7E9
A6CA  20 8D A3            ..     JSR &A38D
A6CD  E6 4A              .J      INC &4A
A6CF  20 9E A9            ..     JSR &A99E
A6D2  20 E9 A7            ..     JSR &A7E9
A6D5  20 D6 A4            ..     JSR &A4D6
A6D8  68                 h       PLA
A6D9  85 4A              .J      STA &4A
A6DB  20 9E A9            ..     JSR &A99E
A6DE  20 E9 A7            ..     JSR &A7E9
A6E1  20 E7 A6            ..     JSR &A6E7
A6E4  A9 FF              ..      LDA #&FF
A6E6  60                 `       RTS
.
A6E7  20 DA A1            ..     JSR &A1DA
A6EA  F0 AC              ..      BEQ &A698
A6EC  20 4E A3            N.     JSR &A34E
A6EF  F0 CA              ..      BEQ &A6BB
.
A6F1  A5 2E              ..      LDA &2E
A6F3  45 3B              E;      EOR &3B
A6F5  85 2E              ..      STA &2E
A6F7  38                 8       SEC
A6F8  A5 30              .0      LDA &30
A6FA  E5 3D              .=      SBC &3D
A6FC  B0 03              ..      BCS &A701
A6FE  C6 2F              ./      DEC &2F
A700  38                 8       SEC
.
A701  69 80              i.      ADC #&80
A703  85 30              .0      STA &30
A705  90 03              ..      BCC &A70A
A707  E6 2F              ./      INC &2F
A709  18                 .       CLC
.
A70A  A2 20              .       LDX #&20
.
A70C  B0 18              ..      BCS &A726
A70E  A5 31              .1      LDA &31
A710  C5 3E              .>      CMP &3E
A712  D0 10              ..      BNE &A724
A714  A5 32              .2      LDA &32
A716  C5 3F              .?      CMP &3F
A718  D0 0A              ..      BNE &A724
A71A  A5 33              .3      LDA &33
A71C  C5 40              .@      CMP &40
A71E  D0 04              ..      BNE &A724
A720  A5 34              .4      LDA &34
A722  C5 41              .A      CMP &41
.
A724  90 19              ..      BCC &A73F
.
A726  A5 34              .4      LDA &34
A728  E5 41              .A      SBC &41
A72A  85 34              .4      STA &34
A72C  A5 33              .3      LDA &33
A72E  E5 40              .@      SBC &40
A730  85 33              .3      STA &33
A732  A5 32              .2      LDA &32
A734  E5 3F              .?      SBC &3F
A736  85 32              .2      STA &32
A738  A5 31              .1      LDA &31
A73A  E5 3E              .>      SBC &3E
A73C  85 31              .1      STA &31
A73E  38                 8       SEC
.
A73F  26 46              &F      ROL &46
A741  26 45              &E      ROL &45
A743  26 44              &D      ROL &44
A745  26 43              &C      ROL &43
A747  06 34              .4      ASL &34
A749  26 33              &3      ROL &33
A74B  26 32              &2      ROL &32
A74D  26 31              &1      ROL &31
A74F  CA                 .       DEX
A750  D0 BA              ..      BNE &A70C
A752  A2 07              ..      LDX #&07
.
A754  B0 18              ..      BCS &A76E
A756  A5 31              .1      LDA &31
A758  C5 3E              .>      CMP &3E
A75A  D0 10              ..      BNE &A76C
A75C  A5 32              .2      LDA &32
A75E  C5 3F              .?      CMP &3F
A760  D0 0A              ..      BNE &A76C
A762  A5 33              .3      LDA &33
A764  C5 40              .@      CMP &40
A766  D0 04              ..      BNE &A76C
A768  A5 34              .4      LDA &34
A76A  C5 41              .A      CMP &41
.
A76C  90 19              ..      BCC &A787
.
A76E  A5 34              .4      LDA &34
A770  E5 41              .A      SBC &41
A772  85 34              .4      STA &34
A774  A5 33              .3      LDA &33
A776  E5 40              .@      SBC &40
A778  85 33              .3      STA &33
A77A  A5 32              .2      LDA &32
A77C  E5 3F              .?      SBC &3F
A77E  85 32              .2      STA &32
A780  A5 31              .1      LDA &31
A782  E5 3E              .>      SBC &3E
A784  85 31              .1      STA &31
A786  38                 8       SEC
.
A787  26 35              &5      ROL &35
A789  06 34              .4      ASL &34
A78B  26 33              &3      ROL &33
A78D  26 32              &2      ROL &32
A78F  26 31              &1      ROL &31
A791  CA                 .       DEX
A792  D0 C0              ..      BNE &A754
A794  06 35              .5      ASL &35
A796  A5 46              .F      LDA &46
A798  85 34              .4      STA &34
A79A  A5 45              .E      LDA &45
A79C  85 33              .3      STA &33
A79E  A5 44              .D      LDA &44
A7A0  85 32              .2      STA &32
A7A2  A5 43              .C      LDA &43
A7A4  85 31              .1      STA &31
A7A6  4C 59 A6           LY.     JMP &A659
.
A7A9  00                 .       ...
A7AA  15 2D              .-      ORA &2D,X
A7AC  76 65              ve      ROR &65,X
A7AE  20 72 6F            ro     JSR &6F72
A7B1  6F 74              ot      BBR6 &A827
A7B3  00                 .       ...
A7B4  20 FA 92            ..     JSR &92FA
.
A7B7  20 DA A1            ..     JSR &A1DA
A7BA  F0 2A              .*      BEQ &A7E6
A7BC  30 EB              0.      BMI &A7A9
A7BE  20 85 A3            ..     JSR &A385
A7C1  A5 30              .0      LDA &30
A7C3  4A                 J       LSR A
A7C4  69 40              i@      ADC #&40
A7C6  85 30              .0      STA &30
A7C8  A9 05              ..      LDA #&05
A7CA  85 4A              .J      STA &4A
A7CC  20 ED A7            ..     JSR &A7ED
.
A7CF  20 8D A3            ..     JSR &A38D
A7D2  A9 6C              .l      LDA #&6C
A7D4  85 4B              .K      STA &4B
A7D6  20 AD A6            ..     JSR &A6AD
A7D9  A9 71              .q      LDA #&71
A7DB  85 4B              .K      STA &4B
A7DD  20 00 A5            ..     JSR &A500
A7E0  C6 30              .0      DEC &30
A7E2  C6 4A              .J      DEC &4A
A7E4  D0 E9              ..      BNE &A7CF
.
A7E6  A9 FF              ..      LDA #&FF
A7E8  60                 `       RTS
.
A7E9  A9 7B              .{      LDA #&7B
A7EB  D0 0A              ..      BNE &A7F7
.
A7ED  A9 71              .q      LDA #&71
A7EF  D0 06              ..      BNE &A7F7
.
A7F1  A9 76              .v      LDA #&76
A7F3  D0 02              ..      BNE &A7F7
.
A7F5  A9 6C              .l      LDA #&6C
.
A7F7  85 4B              .K      STA &4B
A7F9  A9 04              ..      LDA #&04
A7FB  85 4C              .L      STA &4C
A7FD  60                 `       RTS
.
A7FE  20 FA 92            ..     JSR &92FA
.
A801  20 DA A1            ..     JSR &A1DA
A804  F0 02              ..      BEQ &A808
A806  10 0C              ..      BPL &A814
.
A808  00                 .       ...
A809  16 4C              .L      ASL &4C,X
A80B  6F 67              og      BBR6 &A874
A80D  20 72 61            ra     JSR &6172
A810  6E 67 65           nge     ROR &6567
A813  00                 .       ...
.
A814  20 53 A4            S.     JSR &A453
A817  A0 80              ..      LDY #&80
A819  84 3B              .;      STY &3B
A81B  84 3E              .>      STY &3E
.
A81D  C8                 .       INY
A81E  84 3D              .=      STY &3D
A820  A6 30              .0      LDX &30
A822  F0 06              ..      BEQ &A82A
A824  A5 31              .1      LDA &31
A826  C9 B5              ..      CMP #&B5
A828  90 02              ..      BCC &A82C
.
A82A  E8                 .       INX
A82B  88                 .       DEY
.
A82C  8A                 .       TXA
A82D  48                 H       PHA
A82E  84 30              .0      STY &30
A830  20 05 A5            ..     JSR &A505
A833  A9 7B              .{      LDA #&7B
A835  20 87 A3            ..     JSR &A387
.
A838  A9 73              .s      LDA #&73
A83A  A0 A8              ..      LDY #&A8
A83C  20 97 A8            ..     JSR &A897
A83F  20 E9 A7            ..     JSR &A7E9
A842  20 56 A6            V.     JSR &A656
A845  20 56 A6            V.     JSR &A656
A848  20 00 A5            ..     JSR &A500
A84B  20 85 A3            ..     JSR &A385
A84E  68                 h       PLA
A84F  38                 8       SEC
A850  E9 81              ..      SBC #&81
A852  20 ED A2            ..     JSR &A2ED
A855  A9 6E              .n      LDA #&6E
A857  85 4B              .K      STA &4B
A859  A9 A8              ..      LDA #&A8
A85B  85 4C              .L      STA &4C
A85D  20 56 A6            V.     JSR &A656
A860  20 F5 A7            ..     JSR &A7F5
A863  20 00 A5            ..     JSR &A500
A866  A9 FF              ..      LDA #&FF
A868  60                 `       RTS
A869  7F 5E              ^      BBR7 &A8C9
A86B  5B                 [       ...
A86C  D8                 .       CLD
A86D  AA                 .       TAX
A86E  80 31              .1      BRA &A8A1
A870  72 17              r.      ADC (&17)
A872  F8                 .       SED
A873  06 7A              .z      ASL &7A
A875  12 38              .8      ORA (&38)
A877  A5 0B              ..      LDA &0B
A879  88                 .       DEY
A87A  79 0E 9F           y..     ADC &9F0E,Y
A87D  F3                 .       ...
A87E  7C 2A AC           |*.     JMP (&AC2A,X)
A881  3F B5              ?.      BBR3 &A838
A883  86 34              .4      STX &34
A885  01 A2              ..      ORA (&A2,X)
A887  7A                 z       PLY
A888  7F 63              c      BBR7 &A8ED
A88A  8E 37 EC           .7.     STX &EC37
A88D  82                 .       ...
A88E  3F FF              ?.      BBR3 &A88F
A890  FF C1              ..      BBS7 &A853
A892  7F FF              .      BBR7 &A893
A894  FF FF              ..      BBS7 &A895
A896  FF 85              ..      BBS7 &A81D
A898  4D 84 4E           M.N     EOR &4E84
A89B  20 85 A3            ..     JSR &A385
A89E  A0 00              ..      LDY #&00
A8A0  B1 4D              .M      LDA (&4D),Y
A8A2  85 48              .H      STA &48
A8A4  E6 4D              .M      INC &4D
A8A6  D0 02              ..      BNE &A8AA
A8A8  E6 4E              .N      INC &4E
.
A8AA  A5 4D              .M      LDA &4D
A8AC  85 4B              .K      STA &4B
A8AE  A5 4E              .N      LDA &4E
A8B0  85 4C              .L      STA &4C
A8B2  20 B5 A3            ..     JSR &A3B5
.
A8B5  20 F5 A7            ..     JSR &A7F5
A8B8  20 AD A6            ..     JSR &A6AD
A8BB  18                 .       CLC
A8BC  A5 4D              .M      LDA &4D
A8BE  69 05              i.      ADC #&05
A8C0  85 4D              .M      STA &4D
A8C2  85 4B              .K      STA &4B
A8C4  A5 4E              .N      LDA &4E
A8C6  69 00              i.      ADC #&00
A8C8  85 4E              .N      STA &4E
A8CA  85 4C              .L      STA &4C
A8CC  20 00 A5            ..     JSR &A500
A8CF  C6 48              .H      DEC &48
A8D1  D0 E2              ..      BNE &A8B5
A8D3  60                 `       RTS
A8D4  20 DA A8            ..     JSR &A8DA
A8D7  4C 27 A9           L'.     JMP &A927
.
A8DA  20 FA 92            ..     JSR &92FA
A8DD  20 DA A1            ..     JSR &A1DA
A8E0  10 08              ..      BPL &A8EA
A8E2  46 2E              F.      LSR &2E
A8E4  20 EA A8            ..     JSR &A8EA
A8E7  4C 16 A9           L..     JMP &A916
.
A8EA  20 81 A3            ..     JSR &A381
.
A8ED  20 B1 A9            ..     JSR &A9B1
A8F0  20 DA A1            ..     JSR &A1DA
A8F3  F0 09              ..      BEQ &A8FE
A8F5  20 F1 A7            ..     JSR &A7F1
A8F8  20 AD A6            ..     JSR &A6AD
A8FB  4C 0A A9           L..     JMP &A90A
.
A8FE  20 55 AA            U.     JSR &AA55
A901  20 B5 A3            ..     JSR &A3B5
.
A904  A9 FF              ..      LDA #&FF
A906  60                 `       RTS
A907  20 FA 92            ..     JSR &92FA
.
A90A  20 DA A1            ..     JSR &A1DA
A90D  F0 F5              ..      BEQ &A904
A90F  10 0A              ..      BPL &A91B
A911  46 2E              F.      LSR &2E
A913  20 1B A9            ..     JSR &A91B
.
A916  A9 80              ..      LDA #&80
A918  85 2E              ..      STA &2E
A91A  60                 `       RTS
.
A91B  A5 30              .0      LDA &30
A91D  C9 81              ..      CMP #&81
A91F  90 15              ..      BCC &A936
A921  20 A5 A6            ..     JSR &A6A5
A924  20 36 A9            6.     JSR &A936
.
A927  20 48 AA            H.     JSR &AA48
A92A  20 00 A5            ..     JSR &A500
A92D  20 4C AA            L.     JSR &AA4C
A930  20 00 A5            ..     JSR &A500
A933  4C 7E AD           L~.     JMP &AD7E
.
A936  A5 30              .0      LDA &30
A938  C9 73              .s      CMP #&73
A93A  90 C8              ..      BCC &A904
A93C  20 81 A3            ..     JSR &A381
A93F  20 53 A4            S.     JSR &A453
A942  A9 80              ..      LDA #&80
A944  85 3D              .=      STA &3D
A946  85 3E              .>      STA &3E
A948  85 3B              .;      STA &3B
A94A  20 05 A5            ..     JSR &A505
A94D  A9 5A              .Z      LDA #&5A
A94F  A0 A9              ..      LDY #&A9
A951  20 97 A8            ..     JSR &A897
A954  20 D1 AA            ..     JSR &AAD1
A957  A9 FF              ..      LDA #&FF
A959  60                 `       RTS
A95A  09 85              ..      ORA #&85
A95C  A3                 .       ...
A95D  59 E8 67           Y.g     EOR &67E8,Y
A960  80 1C              ..      BRA &A97E
A962  9D 07 36           ..6     STA &3607,X
A965  80 57              .W      BRA &A9BE
A967  BB                 .       ...
A968  78                 x       SEI
A969  DF 80              ..      BBS5 &A8EB
A96B  CA                 .       DEX
A96C  9A                 .       TXS
A96D  0E 83 84           ...     ASL &8483
A970  8C BB CA           ...     STY &CABB
A973  6E 81 95           n..     ROR &9581
A976  96 06              ..      STX &06,Y
A978  DE 81 0A           ...     DEC &0A81,X
A97B  C7                 .       ...
A97C  6C 52 7F           lR     JMP (&7F52)
A97F  7D AD 90           }..     ADC &90AD,X
A982  A1 82              ..      LDA (&82,X)
A984  FB                 .       ...
A985  62                 b       ...
A986  57 2F              W/      RMB5 &2F
A988  80 6D              .m      BRA &A9F7
A98A  63                 c       ...
A98B  38                 8       SEC
A98C  2C 20 FA           , .     BIT &FA20
A98F  92 20              .       STA (&20)
A991  D3                 .       ...
A992  A9 E6              ..      LDA #&E6
A994  4A                 J       LSR A
A995  4C 9E A9           L..     JMP &A99E
A998  20 FA 92            ..     JSR &92FA
A99B  20 D3 A9            ..     JSR &A9D3
.
A99E  A5 4A              .J      LDA &4A
A9A0  29 02              ).      AND #&02
A9A2  F0 06              ..      BEQ &A9AA
A9A4  20 AA A9            ..     JSR &A9AA
A9A7  4C 7E AD           L~.     JMP &AD7E
.
A9AA  46 4A              FJ      LSR &4A
A9AC  90 15              ..      BCC &A9C3
A9AE  20 C3 A9            ..     JSR &A9C3
.
A9B1  20 85 A3            ..     JSR &A385
A9B4  20 56 A6            V.     JSR &A656
A9B7  20 8D A3            ..     JSR &A38D
A9BA  20 99 A6            ..     JSR &A699
A9BD  20 D0 A4            ..     JSR &A4D0
A9C0  4C B7 A7           L..     JMP &A7B7
.
A9C3  20 81 A3            ..     JSR &A381
A9C6  20 56 A6            V.     JSR &A656
A9C9  A9 72              .r      LDA #&72
A9CB  A0 AA              ..      LDY #&AA
A9CD  20 97 A8            ..     JSR &A897
A9D0  4C D1 AA           L..     JMP &AAD1
.
A9D3  A5 30              .0      LDA &30
A9D5  C9 98              ..      CMP #&98
A9D7  B0 5F              ._      BCS &AA38
A9D9  20 85 A3            ..     JSR &A385
A9DC  20 55 AA            U.     JSR &AA55
A9DF  20 4E A3            N.     JSR &A34E
A9E2  A5 2E              ..      LDA &2E
A9E4  85 3B              .;      STA &3B
A9E6  C6 3D              .=      DEC &3D
A9E8  20 05 A5            ..     JSR &A505
A9EB  20 E7 A6            ..     JSR &A6E7
A9EE  20 FE A3            ..     JSR &A3FE
A9F1  A5 34              .4      LDA &34
A9F3  85 4A              .J      STA &4A
A9F5  05 33              .3      ORA &33
.
A9F7  05 32              .2      ORA &32
A9F9  05 31              .1      ORA &31
A9FB  F0 38              .8      BEQ &AA35
A9FD  A9 A0              ..      LDA #&A0
A9FF  85 30              .0      STA &30
AA01  A0 00              ..      LDY #&00
AA03  84 35              .5      STY &35
AA05  A5 31              .1      LDA &31
AA07  85 2E              ..      STA &2E
AA09  10 03              ..      BPL &AA0E
AA0B  20 6C A4            l.     JSR &A46C
.
AA0E  20 03 A3            ..     JSR &A303
AA11  20 7D A3            }.     JSR &A37D
AA14  20 48 AA            H.     JSR &AA48
AA17  20 56 A6            V.     JSR &A656
AA1A  20 F5 A7            ..     JSR &A7F5
AA1D  20 00 A5            ..     JSR &A500
AA20  20 8D A3            ..     JSR &A38D
AA23  20 ED A7            ..     JSR &A7ED
AA26  20 B5 A3            ..     JSR &A3B5
AA29  20 4C AA            L.     JSR &AA4C
AA2C  20 56 A6            V.     JSR &A656
AA2F  20 F5 A7            ..     JSR &A7F5
AA32  4C 00 A5           L..     JMP &A500
.
AA35  4C B2 A3           L..     JMP &A3B2
.
AA38  00                 .       ...
AA39  17 41              .A      RMB1 &41
AA3B  63                 c       ...
AA3C  63                 c       ...
AA3D  75 72              ur      ADC &72,X
AA3F  61 63              ac      ADC (&63,X)
AA41  79 20 6C           y l     ADC &6C20,Y
AA44  6F 73              os      BBR6 &AAB9
AA46  74 00              t.      STZ &00,X
.
AA48  A9 59              .Y      LDA #&59
AA4A  D0 02              ..      BNE &AA4E
.
AA4C  A9 5E              .^      LDA #&5E
.
AA4E  85 4B              .K      STA &4B
AA50  A9 AA              ..      LDA #&AA
AA52  85 4C              .L      STA &4C
AA54  60                 `       RTS
.
AA55  A9 63              .c      LDA #&63
AA57  D0 F5              ..      BNE &AA4E
AA59  81 C9              ..      STA (&C9,X)
AA5B  10 00              ..      BPL &AA5D
.
AA5D  00                 .       ...
AA5E  6F 15              o.      BBR6 &AA75
AA60  77 7A              wz      RMB7 &7A
AA62  61 81              a.      ADC (&81,X)
AA64  49 0F              I.      EOR #&0F
AA66  DA                 .       PHX
AA67  A2 7B              .{      LDX #&7B
AA69  0E FA 35           ..5     ASL &35FA
AA6C  12 86              ..      ORA (&86)
AA6E  65 2E              e.      ADC &2E
AA70  E0 D3              ..      CPX #&D3
AA72  05 84              ..      ORA &84
AA74  8A                 .       TXA
.
AA75  EA                 .       NOP
AA76  0C 1B 84           ...     TSB &841B
AA79  1A                 .       INC A
AA7A  BE BB 2B           ..+     LDX &2BBB,Y
AA7D  84 37              .7      STY &37
AA7F  45 55              EU      EOR &55
AA81  AB                 .       ...
AA82  82                 .       ...
AA83  D5 55              .U      CMP &55,X
AA85  57 7C              W|      RMB5 &7C
AA87  83                 .       ...
AA88  C0 00              ..      CPY #&00
AA8A  00                 .       ...
AA8B  05 81              ..      ORA &81
AA8D  00                 .       ...
AA8E  00                 .       ...
AA8F  00                 .       ...
AA90  00                 .       ...
AA91  20 FA 92            ..     JSR &92FA
.
AA94  A5 30              .0      LDA &30
AA96  C9 87              ..      CMP #&87
AA98  90 1E              ..      BCC &AAB8
AA9A  D0 06              ..      BNE &AAA2
AA9C  A4 31              .1      LDY &31
AA9E  C0 B3              ..      CPY #&B3
AAA0  90 16              ..      BCC &AAB8
.
AAA2  A5 2E              ..      LDA &2E
AAA4  10 06              ..      BPL &AAAC
AAA6  20 86 A6            ..     JSR &A686
AAA9  A9 FF              ..      LDA #&FF
AAAB  60                 `       RTS
.
AAAC  00                 .       ...
AAAD  18                 .       CLC
AAAE  45 78              Ex      EOR &78
AAB0  70 20              p       BCS &AAD2
AAB2  72 61              ra      ADC (&61)
AAB4  6E 67 65           nge     ROR &6567
AAB7  00                 .       ...
.
AAB8  20 86 A4            ..     JSR &A486
AABB  20 DA AA            ..     JSR &AADA
AABE  20 81 A3            ..     JSR &A381
AAC1  A9 E4              ..      LDA #&E4
AAC3  85 4B              .K      STA &4B
AAC5  A9 AA              ..      LDA #&AA
AAC7  85 4C              .L      STA &4C
AAC9  20 B5 A3            ..     JSR &A3B5
AACC  A5 4A              .J      LDA &4A
AACE  20 12 AB            ..     JSR &AB12
.
AAD1  20 F1 A7            ..     JSR &A7F1
AAD4  20 56 A6            V.     JSR &A656
AAD7  A9 FF              ..      LDA #&FF
AAD9  60                 `       RTS
.
AADA  A9 E9              ..      LDA #&E9
AADC  A0 AA              ..      LDY #&AA
AADE  20 97 A8            ..     JSR &A897
AAE1  A9 FF              ..      LDA #&FF
AAE3  60                 `       RTS
AAE4  82                 .       ...
AAE5  2D F8 54           -.T     AND &54F8
AAE8  58                 X       CLI
AAE9  07 83              ..      RMB0 &83
AAEB  E0 20              .       CPX #&20
AAED  86 5B              .[      STX &5B
AAEF  82                 .       ...
AAF0  80 53              .S      BRA &AB45
AAF2  93                 .       ...
AAF3  B8                 .       CLV
AAF4  83                 .       ...
AAF5  20 00 06            ..     JSR &0600
AAF8  A1 82              ..      LDA (&82,X)
AAFA  00                 .       ...
AAFB  00                 .       ...
AAFC  21 63              !c      AND (&63,X)
AAFE  82                 .       ...
AAFF  C0 00              ..      CPY #&00
AB01  00                 .       ...
AB02  02                 .       ...
AB03  82                 .       ...
AB04  80 00              ..      BRA &AB06
.
AB06  00                 .       ...
AB07  0C 81 00           ...     TSB &0081
AB0A  00                 .       ...
AB0B  00                 .       ...
AB0C  00                 .       ...
AB0D  81 00              ..      STA (&00,X)
AB0F  00                 .       ...
AB10  00                 .       ...
AB11  00                 .       ...
.
AB12  AA                 .       TAX
AB13  10 09              ..      BPL &AB1E
AB15  CA                 .       DEX
AB16  8A                 .       TXA
AB17  49 FF              I.      EOR #&FF
AB19  48                 H       PHA
AB1A  20 A5 A6            ..     JSR &A6A5
AB1D  68                 h       PLA
.
AB1E  48                 H       PHA
AB1F  20 85 A3            ..     JSR &A385
AB22  20 99 A6            ..     JSR &A699
.
AB25  68                 h       PLA
AB26  F0 0A              ..      BEQ &AB32
AB28  38                 8       SEC
AB29  E9 01              ..      SBC #&01
AB2B  48                 H       PHA
AB2C  20 56 A6            V.     JSR &A656
AB2F  4C 25 AB           L%.     JMP &AB25
.
AB32  60                 `       RTS
AB33  20 E3 92            ..     JSR &92E3
AB36  A6 2A              .*      LDX &2A
AB38  A9 80              ..      LDA #&80                   ; OSBYTE &80: Read ADC channel (ADVAL) or get buffer status
AB3A  20 F4 FF            ..     JSR osbyte
AB3D  8A                 .       TXA
AB3E  4C EA AE           L..     JMP &AEEA
AB41  20 DD 92            ..     JSR &92DD
AB44  20 94 BD            ..     JSR &BD94
AB47  20 AE 8A            ..     JSR &8AAE
AB4A  20 56 AE            V.     JSR &AE56
AB4D  20 F0 92            ..     JSR &92F0
AB50  A5 2A              .*      LDA &2A
AB52  48                 H       PHA
AB53  A5 2B              .+      LDA &2B
AB55  48                 H       PHA
AB56  20 EA BD            ..     JSR &BDEA
AB59  68                 h       PLA
AB5A  85 2D              .-      STA &2D
AB5C  68                 h       PLA
AB5D  85 2C              .,      STA &2C
AB5F  A2 2A              .*      LDX #&2A
AB61  A9 09              ..      LDA #&09                   ; OSWORD &09: Read pixel value
AB63  20 F1 FF            ..     JSR osword
AB66  A5 2E              ..      LDA &2E
AB68  30 33              03      BMI &AB9D
AB6A  4C D8 AE           L..     JMP &AED8
AB6D  A9 86              ..      LDA #&86                   ; OSBYTE &86: Read text cursor position (POS and VPOS)
AB6F  20 F4 FF            ..     JSR osbyte
AB72  8A                 .       TXA                        ; POS
AB73  4C D8 AE           L..     JMP &AED8
AB76  A9 86              ..      LDA #&86                   ; OSBYTE &86: Read text cursor position (POS and VPOS)
AB78  20 F4 FF            ..     JSR osbyte
AB7B  98                 .       TYA                        ; VPOS
AB7C  4C D8 AE           L..     JMP &AED8
.
AB7F  20 DA A1            ..     JSR &A1DA
AB82  F0 1E              ..      BEQ &ABA2
AB84  10 1A              ..      BPL &ABA0
AB86  30 15              0.      BMI &AB9D
AB88  20 EC AD            ..     JSR &ADEC
AB8B  F0 59              .Y      BEQ &ABE6
AB8D  30 F0              0.      BMI &AB7F
AB8F  A5 2D              .-      LDA &2D
AB91  05 2C              .,      ORA &2C
AB93  05 2B              .+      ORA &2B
AB95  05 2A              .*      ORA &2A
AB97  F0 0C              ..      BEQ &ABA5
AB99  A5 2D              .-      LDA &2D
AB9B  10 03              ..      BPL &ABA0
.
AB9D  4C C4 AC           L..     JMP &ACC4
.
ABA0  A9 01              ..      LDA #&01
.
ABA2  4C D8 AE           L..     JMP &AED8
.
ABA5  A9 40              .@      LDA #&40
ABA7  60                 `       RTS
ABA8  20 FE A7            ..     JSR &A7FE
ABAB  A0 69              .i      LDY #&69
ABAD  A9 A8              ..      LDA #&A8
ABAF  D0 07              ..      BNE &ABB8
ABB1  20 FA 92            ..     JSR &92FA
ABB4  A0 68              .h      LDY #&68
ABB6  A9 AA              ..      LDA #&AA
.
ABB8  84 4B              .K      STY &4B
ABBA  85 4C              .L      STA &4C
ABBC  20 56 A6            V.     JSR &A656
ABBF  A9 FF              ..      LDA #&FF
ABC1  60                 `       RTS
ABC2  20 FA 92            ..     JSR &92FA
ABC5  A0 6D              .m      LDY #&6D
ABC7  A9 AA              ..      LDA #&AA
ABC9  D0 ED              ..      BNE &ABB8
ABCB  20 FE A8            ..     JSR &A8FE
ABCE  E6 30              .0      INC &30
ABD0  A8                 .       TAY
ABD1  60                 `       RTS
ABD2  20 E3 92            ..     JSR &92E3
ABD5  20 1E 8F            ..     JSR &8F1E
ABD8  85 2A              .*      STA &2A
ABDA  86 2B              .+      STX &2B
ABDC  84 2C              .,      STY &2C
ABDE  08                 .       PHP
ABDF  68                 h       PLA
ABE0  85 2D              .-      STA &2D
ABE2  D8                 .       CLD
ABE3  A9 40              .@      LDA #&40
ABE5  60                 `       RTS
.
ABE6  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
ABE9  20 EC AD            ..     JSR &ADEC
ABEC  D0 F8              ..      BNE &ABE6
ABEE  E6 36              .6      INC &36
ABF0  A4 36              .6      LDY &36
ABF2  A9 0D              ..      LDA #&0D
ABF4  99 FF 05           ...     STA &05FF,Y
ABF7  20 B2 BD            ..     JSR &BDB2
ABFA  A5 19              ..      LDA &19
ABFC  48                 H       PHA
ABFD  A5 1A              ..      LDA &1A
ABFF  48                 H       PHA
AC00  A5 1B              ..      LDA &1B
AC02  48                 H       PHA
AC03  A4 04              ..      LDY &04
AC05  A6 05              ..      LDX &05
AC07  C8                 .       INY
AC08  84 19              ..      STY &19
AC0A  84 37              .7      STY &37
AC0C  D0 01              ..      BNE &AC0F
AC0E  E8                 .       INX
.
AC0F  86 1A              ..      STX &1A
AC11  86 38              .8      STX &38
AC13  A0 FF              ..      LDY #&FF
AC15  84 3B              .;      STY &3B
AC17  C8                 .       INY
AC18  84 1B              ..      STY &1B
AC1A  20 55 89            U.     JSR &8955
AC1D  20 29 9B            ).     JSR &9B29
AC20  20 DC BD            ..     JSR &BDDC
.
AC23  68                 h       PLA
AC24  85 1B              ..      STA &1B
AC26  68                 h       PLA
AC27  85 1A              ..      STA &1A
AC29  68                 h       PLA
AC2A  85 19              ..      STA &19
AC2C  A5 27              .'      LDA &27
AC2E  60                 `       RTS
AC2F  20 EC AD            ..     JSR &ADEC
AC32  D0 67              .g      BNE &AC9B
.
AC34  A4 36              .6      LDY &36
AC36  A9 00              ..      LDA #&00
AC38  99 00 06           ...     STA &0600,Y
AC3B  A5 19              ..      LDA &19
AC3D  48                 H       PHA
AC3E  A5 1A              ..      LDA &1A
AC40  48                 H       PHA
AC41  A5 1B              ..      LDA &1B
AC43  48                 H       PHA
AC44  A9 00              ..      LDA #&00
AC46  85 1B              ..      STA &1B
AC48  A9 00              ..      LDA #&00
AC4A  85 19              ..      STA &19
AC4C  A9 06              ..      LDA #&06
AC4E  85 1A              ..      STA &1A
AC50  20 8C 8A            ..     JSR &8A8C
AC53  C9 2D              .-      CMP #&2D
AC55  F0 0F              ..      BEQ &AC66
AC57  C9 2B              .+      CMP #&2B
AC59  D0 03              ..      BNE &AC5E
AC5B  20 8C 8A            ..     JSR &8A8C
.
AC5E  C6 1B              ..      DEC &1B
AC60  20 7B A0            {.     JSR &A07B
AC63  4C 73 AC           Ls.     JMP &AC73
.
AC66  20 8C 8A            ..     JSR &8A8C
AC69  C6 1B              ..      DEC &1B
AC6B  20 7B A0            {.     JSR &A07B
AC6E  90 03              ..      BCC &AC73
AC70  20 8F AD            ..     JSR &AD8F
.
AC73  85 27              .'      STA &27
AC75  4C 23 AC           L#.     JMP &AC23
AC78  20 EC AD            ..     JSR &ADEC
AC7B  F0 1E              ..      BEQ &AC9B
AC7D  10 1B              ..      BPL &AC9A
AC7F  A5 2E              ..      LDA &2E
AC81  08                 .       PHP
AC82  20 FE A3            ..     JSR &A3FE
AC85  28                 (       PLP
AC86  10 0D              ..      BPL &AC95
AC88  A5 3E              .>      LDA &3E
AC8A  05 3F              .?      ORA &3F
AC8C  05 40              .@      ORA &40
AC8E  05 41              .A      ORA &41
AC90  F0 03              ..      BEQ &AC95
AC92  20 C7 A4            ..     JSR &A4C7
.
AC95  20 E7 A3            ..     JSR &A3E7
AC98  A9 40              .@      LDA #&40
.
AC9A  60                 `       RTS
.
AC9B  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
AC9E  20 EC AD            ..     JSR &ADEC
ACA1  D0 F8              ..      BNE &AC9B
ACA3  A5 36              .6      LDA &36
ACA5  F0 1D              ..      BEQ &ACC4
ACA7  AD 00 06           ...     LDA &0600
.
ACAA  4C D8 AE           L..     JMP &AED8
ACAD  20 AD AF            ..     JSR &AFAD
ACB0  C0 00              ..      CPY #&00
ACB2  D0 10              ..      BNE &ACC4
ACB4  8A                 .       TXA
ACB5  4C EA AE           L..     JMP &AEEA
ACB8  20 B5 BF            ..     JSR &BFB5
ACBB  AA                 .       TAX
ACBC  A9 7F              .      LDA #&7F                   ; OSBYTE &7F: Check for EOF on an opened file
ACBE  20 F4 FF            ..     JSR osbyte
ACC1  8A                 .       TXA
ACC2  F0 E6              ..      BEQ &ACAA                  ; Jump if EOF hasn't been reached
.
ACC4  A9 FF              ..      LDA #&FF
ACC6  85 2A              .*      STA &2A
ACC8  85 2B              .+      STA &2B
ACCA  85 2C              .,      STA &2C
ACCC  85 2D              .-      STA &2D
ACCE  A9 40              .@      LDA #&40
ACD0  60                 `       RTS
ACD1  20 E3 92            ..     JSR &92E3
ACD4  A2 03              ..      LDX #&03
.
ACD6  B5 2A              .*      LDA &2A,X
ACD8  49 FF              I.      EOR #&FF
ACDA  95 2A              .*      STA &2A,X
ACDC  CA                 .       DEX
ACDD  10 F7              ..      BPL &ACD6
ACDF  A9 40              .@      LDA #&40
ACE1  60                 `       RTS
ACE2  20 29 9B            ).     JSR &9B29
ACE5  D0 B4              ..      BNE &AC9B
ACE7  E0 2C              .,      CPX #&2C
ACE9  D0 18              ..      BNE &AD03
ACEB  E6 1B              ..      INC &1B
ACED  20 B2 BD            ..     JSR &BDB2
ACF0  20 29 9B            ).     JSR &9B29
ACF3  D0 A6              ..      BNE &AC9B
ACF5  A9 01              ..      LDA #&01
ACF7  85 2A              .*      STA &2A
ACF9  E6 1B              ..      INC &1B
ACFB  E0 29              .)      CPX #&29
ACFD  F0 13              ..      BEQ &AD12
ACFF  E0 2C              .,      CPX #&2C
AD01  F0 03              ..      BEQ &AD06
.
AD03  4C A2 8A           L..     JMP &8AA2
.
AD06  20 B2 BD            ..     JSR &BDB2
AD09  20 56 AE            V.     JSR &AE56
AD0C  20 F0 92            ..     JSR &92F0
AD0F  20 CB BD            ..     JSR &BDCB
.
AD12  A0 00              ..      LDY #&00
AD14  A6 2A              .*      LDX &2A
AD16  D0 02              ..      BNE &AD1A
AD18  A2 01              ..      LDX #&01
.
AD1A  86 2A              .*      STX &2A
AD1C  8A                 .       TXA
AD1D  CA                 .       DEX
AD1E  86 2D              .-      STX &2D
AD20  18                 .       CLC
AD21  65 04              e.      ADC &04
AD23  85 37              .7      STA &37
AD25  98                 .       TYA
AD26  65 05              e.      ADC &05
AD28  85 38              .8      STA &38
AD2A  B1 04              ..      LDA (&04),Y
AD2C  38                 8       SEC
AD2D  E5 2D              .-      SBC &2D
AD2F  90 21              .!      BCC &AD52
AD31  E5 36              .6      SBC &36
AD33  90 1D              ..      BCC &AD52
AD35  69 00              i.      ADC #&00
AD37  85 2B              .+      STA &2B
AD39  20 DC BD            ..     JSR &BDDC
.
AD3C  A0 00              ..      LDY #&00
AD3E  A6 36              .6      LDX &36
AD40  F0 0B              ..      BEQ &AD4D
.
AD42  B1 37              .7      LDA (&37),Y
AD44  D9 00 06           ...     CMP &0600,Y
AD47  D0 10              ..      BNE &AD59
AD49  C8                 .       INY
AD4A  CA                 .       DEX
AD4B  D0 F5              ..      BNE &AD42
.
AD4D  A5 2A              .*      LDA &2A
.
AD4F  4C D8 AE           L..     JMP &AED8
.
AD52  20 DC BD            ..     JSR &BDDC
.
AD55  A9 00              ..      LDA #&00
AD57  F0 F6              ..      BEQ &AD4F
.
AD59  E6 2A              .*      INC &2A
AD5B  C6 2B              .+      DEC &2B
AD5D  F0 F6              ..      BEQ &AD55
AD5F  E6 37              .7      INC &37
AD61  D0 D9              ..      BNE &AD3C
AD63  E6 38              .8      INC &38
AD65  D0 D5              ..      BNE &AD3C
.
AD67  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
AD6A  20 EC AD            ..     JSR &ADEC
AD6D  F0 F8              ..      BEQ &AD67
AD6F  30 06              0.      BMI &AD77
.
AD71  24 2D              $-      BIT &2D
AD73  30 1E              0.      BMI &AD93
AD75  10 33              .3      BPL &ADAA
.
AD77  20 DA A1            ..     JSR &A1DA
AD7A  10 0D              ..      BPL &AD89
AD7C  30 05              0.      BMI &AD83
.
AD7E  20 DA A1            ..     JSR &A1DA
AD81  F0 06              ..      BEQ &AD89
.
AD83  A5 2E              ..      LDA &2E
AD85  49 80              I.      EOR #&80
AD87  85 2E              ..      STA &2E
.
AD89  A9 FF              ..      LDA #&FF
AD8B  60                 `       RTS
.
AD8C  20 02 AE            ..     JSR &AE02
.
AD8F  F0 D6              ..      BEQ &AD67
AD91  30 EB              0.      BMI &AD7E
.
AD93  38                 8       SEC
AD94  A9 00              ..      LDA #&00
AD96  A8                 .       TAY
AD97  E5 2A              .*      SBC &2A
AD99  85 2A              .*      STA &2A
AD9B  98                 .       TYA
AD9C  E5 2B              .+      SBC &2B
AD9E  85 2B              .+      STA &2B
ADA0  98                 .       TYA
ADA1  E5 2C              .,      SBC &2C
ADA3  85 2C              .,      STA &2C
ADA5  98                 .       TYA
ADA6  E5 2D              .-      SBC &2D
ADA8  85 2D              .-      STA &2D
.
ADAA  A9 40              .@      LDA #&40
ADAC  60                 `       RTS
.
ADAD  20 8C 8A            ..     JSR &8A8C
ADB0  C9 22              ."      CMP #&22
ADB2  F0 15              ..      BEQ &ADC9
ADB4  A2 00              ..      LDX #&00
.
ADB6  B1 19              ..      LDA (&19),Y
ADB8  9D 00 06           ...     STA &0600,X
ADBB  C8                 .       INY
ADBC  E8                 .       INX
ADBD  C9 0D              ..      CMP #&0D
ADBF  F0 04              ..      BEQ &ADC5
ADC1  C9 2C              .,      CMP #&2C
ADC3  D0 F1              ..      BNE &ADB6
.
ADC5  88                 .       DEY
ADC6  4C E1 AD           L..     JMP &ADE1
.
ADC9  A2 00              ..      LDX #&00
.
ADCB  C8                 .       INY
.
ADCC  B1 19              ..      LDA (&19),Y
ADCE  C9 0D              ..      CMP #&0D
ADD0  F0 17              ..      BEQ &ADE9
ADD2  C8                 .       INY
ADD3  9D 00 06           ...     STA &0600,X
ADD6  E8                 .       INX
ADD7  C9 22              ."      CMP #'"'
ADD9  D0 F1              ..      BNE &ADCC
ADDB  B1 19              ..      LDA (&19),Y
ADDD  C9 22              ."      CMP #&22
ADDF  F0 EA              ..      BEQ &ADCB
.
ADE1  CA                 .       DEX
ADE2  86 36              .6      STX &36
ADE4  84 1B              ..      STY &1B
ADE6  A9 00              ..      LDA #&00
ADE8  60                 `       RTS
.
ADE9  4C 98 8E           L..     JMP &8E98                  ; Error &09: Missing "
.
ADEC  A4 1B              ..      LDY &1B
ADEE  E6 1B              ..      INC &1B
ADF0  B1 19              ..      LDA (&19),Y
ADF2  C9 20              .       CMP #&20
ADF4  F0 F6              ..      BEQ &ADEC
ADF6  C9 2D              .-      CMP #&2D
ADF8  F0 92              ..      BEQ &AD8C
ADFA  C9 22              ."      CMP #&22
ADFC  F0 CB              ..      BEQ &ADC9
ADFE  C9 2B              .+      CMP #&2B
AE00  D0 03              ..      BNE &AE05
.
AE02  20 8C 8A            ..     JSR &8A8C
.
AE05  C9 8E              ..      CMP #&8E
AE07  90 07              ..      BCC &AE10
AE09  C9 C6              ..      CMP #&C6
AE0B  B0 36              .6      BCS &AE43
AE0D  4C B1 8B           L..     JMP &8BB1
.
AE10  C9 3F              .?      CMP #&3F
AE12  B0 0C              ..      BCS &AE20
AE14  C9 2E              ..      CMP #&2E
AE16  B0 12              ..      BCS &AE2A
AE18  C9 26              .&      CMP #&26
AE1A  F0 51              .Q      BEQ &AE6D
AE1C  C9 28              .(      CMP #&28
AE1E  F0 36              .6      BEQ &AE56
.
AE20  C6 1B              ..      DEC &1B
AE22  20 DD 95            ..     JSR &95DD
AE25  F0 09              ..      BEQ &AE30
AE27  4C 2C B3           L,.     JMP &B32C
.
AE2A  20 7B A0            {.     JSR &A07B
AE2D  90 14              ..      BCC &AE43
AE2F  60                 `       RTS
.
AE30  A5 28              .(      LDA &28
AE32  29 02              ).      AND #&02
AE34  D0 0D              ..      BNE &AE43
AE36  B0 0B              ..      BCS &AE43
AE38  86 1B              ..      STX &1B
.
AE3A  AD 40 04           .@.     LDA &0440
AE3D  AC 41 04           .A.     LDY &0441
AE40  4C EA AE           L..     JMP &AEEA
.
AE43  00                 .       ...
AE44  1A                 .       INC A
AE45  4E 6F 20           No      LSR &206F
AE48  73                 s       ...
AE49  75 63              uc      ADC &63,X
AE4B  68                 h       PLA
AE4C  20 76 61            va     JSR &6176
AE4F  72 69              ri      ADC (&69)
AE51  61 62              ab      ADC (&62,X)
AE53  6C 65 00           le.     JMP (&0065)
.
AE56  20 29 9B            ).     JSR &9B29
AE59  E6 1B              ..      INC &1B
AE5B  E0 29              .)      CPX #&29
AE5D  D0 02              ..      BNE &AE61
AE5F  A8                 .       TAY
AE60  60                 `       RTS
.
AE61  00                 .       ...
AE62  1B                 .       ...
AE63  4D 69 73           Mis     EOR &7369
AE66  73                 s       ...
AE67  69 6E              in      ADC #&6E
AE69  67 20              g       RMB6 &20
AE6B  29 00              ).      AND #&00
.
AE6D  A2 00              ..      LDX #&00
AE6F  86 2A              .*      STX &2A
AE71  86 2B              .+      STX &2B
AE73  86 2C              .,      STX &2C
AE75  86 2D              .-      STX &2D
AE77  A4 1B              ..      LDY &1B
.
AE79  B1 19              ..      LDA (&19),Y
AE7B  C9 30              .0      CMP #&30
AE7D  90 23              .#      BCC &AEA2
AE7F  C9 3A              .:      CMP #&3A
AE81  90 0A              ..      BCC &AE8D
AE83  E9 37              .7      SBC #&37
AE85  C9 0A              ..      CMP #&0A
AE87  90 19              ..      BCC &AEA2
AE89  C9 10              ..      CMP #&10
AE8B  B0 15              ..      BCS &AEA2
.
AE8D  0A                 .       ASL A
AE8E  0A                 .       ASL A
AE8F  0A                 .       ASL A
AE90  0A                 .       ASL A
AE91  A2 03              ..      LDX #&03
.
AE93  0A                 .       ASL A
AE94  26 2A              &*      ROL &2A
AE96  26 2B              &+      ROL &2B
AE98  26 2C              &,      ROL &2C
AE9A  26 2D              &-      ROL &2D
AE9C  CA                 .       DEX
AE9D  10 F4              ..      BPL &AE93
AE9F  C8                 .       INY
AEA0  D0 D7              ..      BNE &AE79
.
AEA2  8A                 .       TXA
AEA3  10 05              ..      BPL &AEAA
AEA5  84 1B              ..      STY &1B
AEA7  A9 40              .@      LDA #&40
AEA9  60                 `       RTS
.
AEAA  00                 .       ...
AEAB  1C 42 61           .Ba     TRB &6142
AEAE  64 20              d       STZ &20
AEB0  48                 H       PHA
AEB1  45 58              EX      EOR &58
AEB3  00                 .       ...
AEB4  A2 2A              .*      LDX #&2A
AEB6  A0 00              ..      LDY #&00
AEB8  A9 01              ..      LDA #&01                   ; OSWORD &02: Read system clock
AEBA  20 F1 FF            ..     JSR osword
AEBD  A9 40              .@      LDA #&40
AEBF  60                 `       RTS
AEC0  A9 00              ..      LDA #&00
AEC2  A4 18              ..      LDY page
AEC4  4C EA AE           L..     JMP &AEEA
.
AEC7  4C 43 AE           LC.     JMP &AE43
AECA  A9 00              ..      LDA #&00
AECC  F0 0A              ..      BEQ &AED8
.
AECE  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
AED1  20 EC AD            ..     JSR &ADEC
AED4  D0 F8              ..      BNE &AECE
AED6  A5 36              .6      LDA &36
.
AED8  A0 00              ..      LDY #&00
AEDA  F0 0E              ..      BEQ &AEEA
AEDC  A4 1B              ..      LDY &1B
AEDE  B1 19              ..      LDA (&19),Y
AEE0  C9 50              .P      CMP #&50
AEE2  D0 E3              ..      BNE &AEC7
AEE4  E6 1B              ..      INC &1B
AEE6  A5 12              ..      LDA &12
AEE8  A4 13              ..      LDY &13
.
AEEA  85 2A              .*      STA &2A
AEEC  84 2B              .+      STY &2B
AEEE  A9 00              ..      LDA #&00
AEF0  85 2C              .,      STA &2C
AEF2  85 2D              .-      STA &2D
AEF4  A9 40              .@      LDA #&40
AEF6  60                 `       RTS
AEF7  A5 1E              ..      LDA &1E
AEF9  4C D8 AE           L..     JMP &AED8
AEFC  A5 00              ..      LDA &00
AEFE  A4 01              ..      LDY &01
AF00  4C EA AE           L..     JMP &AEEA
AF03  A5 06              ..      LDA &06
AF05  A4 07              ..      LDY &07
AF07  4C EA AE           L..     JMP &AEEA
.
AF0A  E6 1B              ..      INC &1B
AF0C  20 56 AE            V.     JSR &AE56
AF0F  20 F0 92            ..     JSR &92F0
AF12  A5 2D              .-      LDA &2D
AF14  30 29              0)      BMI &AF3F
AF16  05 2C              .,      ORA &2C
AF18  05 2B              .+      ORA &2B
AF1A  D0 08              ..      BNE &AF24
AF1C  A5 2A              .*      LDA &2A
AF1E  F0 4C              .L      BEQ &AF6C
AF20  C9 01              ..      CMP #&01
AF22  F0 45              .E      BEQ &AF69
.
AF24  20 BE A2            ..     JSR &A2BE
AF27  20 51 BD            Q.     JSR &BD51
AF2A  20 69 AF            i.     JSR &AF69
AF2D  20 7E BD            ~.     JSR &BD7E
AF30  20 06 A6            ..     JSR &A606
AF33  20 03 A3            ..     JSR &A303
AF36  20 E4 A3            ..     JSR &A3E4
AF39  20 22 92            ".     JSR &9222
AF3C  A9 40              .@      LDA #&40
AF3E  60                 `       RTS
.
AF3F  A2 0D              ..      LDX #&0D
AF41  20 44 BE            D.     JSR &BE44
AF44  A9 40              .@      LDA #&40
AF46  85 11              ..      STA &11
AF48  60                 `       RTS
AF49  A4 1B              ..      LDY &1B
AF4B  B1 19              ..      LDA (&19),Y
AF4D  C9 28              .(      CMP #&28
AF4F  F0 B9              ..      BEQ &AF0A
AF51  20 87 AF            ..     JSR &AF87
AF54  A2 0D              ..      LDX #&0D
.
AF56  B5 00              ..      LDA &00,X
AF58  85 2A              .*      STA &2A
AF5A  B5 01              ..      LDA &01,X
AF5C  85 2B              .+      STA &2B
AF5E  B5 02              ..      LDA &02,X
AF60  85 2C              .,      STA &2C
AF62  B5 03              ..      LDA &03,X
AF64  85 2D              .-      STA &2D
AF66  A9 40              .@      LDA #&40
AF68  60                 `       RTS
.
AF69  20 87 AF            ..     JSR &AF87
.
AF6C  A2 00              ..      LDX #&00
AF6E  86 2E              ..      STX &2E
AF70  86 2F              ./      STX &2F
AF72  86 35              .5      STX &35
AF74  A9 80              ..      LDA #&80
AF76  85 30              .0      STA &30
.
AF78  B5 0D              ..      LDA rnd,X
AF7A  95 31              .1      STA &31,X
AF7C  E8                 .       INX
AF7D  E0 04              ..      CPX #&04
AF7F  D0 F7              ..      BNE &AF78
AF81  20 59 A6            Y.     JSR &A659
AF84  A9 FF              ..      LDA #&FF
AF86  60                 `       RTS
.
AF87  A0 20              .       LDY #&20
.
AF89  A5 0F              ..      LDA rnd+2
AF8B  4A                 J       LSR A
AF8C  4A                 J       LSR A
AF8D  4A                 J       LSR A
AF8E  45 11              E.      EOR rnd+4
AF90  6A                 j       ROR A
AF91  26 0D              &.      ROL rnd
AF93  26 0E              &.      ROL rnd+1
AF95  26 0F              &.      ROL rnd+2
AF97  26 10              &.      ROL rnd+3
AF99  26 11              &.      ROL rnd+4
AF9B  88                 .       DEY
AF9C  D0 EB              ..      BNE &AF89
AF9E  60                 `       RTS
AF9F  A4 09              ..      LDY &09
AFA1  A5 08              ..      LDA &08
AFA3  4C EA AE           L..     JMP &AEEA
AFA6  A0 00              ..      LDY #&00
AFA8  B1 FD              ..      LDA (&FD),Y
AFAA  4C EA AE           L..     JMP &AEEA
.
AFAD  20 E3 92            ..     JSR &92E3
AFB0  A9 81              ..      LDA #&81                   ; OSBYTE &81: Read key with time limit (INKEY)
AFB2  A6 2A              .*      LDX &2A
AFB4  A4 2B              .+      LDY &2B
AFB6  4C F4 FF           L..     JMP osbyte
AFB9  20 E0 FF            ..     JSR osrdch
AFBC  4C D8 AE           L..     JMP &AED8
AFBF  20 E0 FF            ..     JSR osrdch
.
AFC2  8D 00 06           ...     STA &0600
AFC5  A9 01              ..      LDA #&01
AFC7  85 36              .6      STA &36
AFC9  A9 00              ..      LDA #&00
AFCB  60                 `       RTS
AFCC  20 29 9B            ).     JSR &9B29
AFCF  D0 62              .b      BNE &B033
AFD1  E0 2C              .,      CPX #&2C
AFD3  D0 61              .a      BNE &B036
AFD5  E6 1B              ..      INC &1B
AFD7  20 B2 BD            ..     JSR &BDB2
AFDA  20 56 AE            V.     JSR &AE56
AFDD  20 F0 92            ..     JSR &92F0
AFE0  20 CB BD            ..     JSR &BDCB
AFE3  A5 2A              .*      LDA &2A
AFE5  C5 36              .6      CMP &36
AFE7  B0 02              ..      BCS &AFEB
AFE9  85 36              .6      STA &36
.
AFEB  A9 00              ..      LDA #&00
AFED  60                 `       RTS
AFEE  20 29 9B            ).     JSR &9B29
AFF1  D0 40              .@      BNE &B033
AFF3  E0 2C              .,      CPX #&2C
AFF5  D0 3F              .?      BNE &B036
AFF7  E6 1B              ..      INC &1B
AFF9  20 B2 BD            ..     JSR &BDB2
AFFC  20 56 AE            V.     JSR &AE56
AFFF  20 F0 92            ..     JSR &92F0
B002  20 CB BD            ..     JSR &BDCB
B005  A5 36              .6      LDA &36
B007  38                 8       SEC
B008  E5 2A              .*      SBC &2A
B00A  90 17              ..      BCC &B023
B00C  F0 17              ..      BEQ &B025
B00E  AA                 .       TAX
B00F  A5 2A              .*      LDA &2A
B011  85 36              .6      STA &36
B013  F0 10              ..      BEQ &B025
B015  A0 00              ..      LDY #&00
.
B017  BD 00 06           ...     LDA &0600,X
B01A  99 00 06           ...     STA &0600,Y
B01D  E8                 .       INX
B01E  C8                 .       INY
B01F  C6 2A              .*      DEC &2A
B021  D0 F4              ..      BNE &B017
.
B023  A9 00              ..      LDA #&00
.
B025  60                 `       RTS
B026  20 AD AF            ..     JSR &AFAD
B029  8A                 .       TXA
B02A  C0 00              ..      CPY #&00
B02C  F0 94              ..      BEQ &AFC2
.
B02E  A9 00              ..      LDA #&00
B030  85 36              .6      STA &36
B032  60                 `       RTS
.
B033  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
B036  4C A2 8A           L..     JMP &8AA2
B039  20 29 9B            ).     JSR &9B29
B03C  D0 F5              ..      BNE &B033
B03E  E0 2C              .,      CPX #&2C
B040  D0 F4              ..      BNE &B036
B042  20 B2 BD            ..     JSR &BDB2
B045  E6 1B              ..      INC &1B
B047  20 DD 92            ..     JSR &92DD
B04A  A5 2A              .*      LDA &2A
B04C  48                 H       PHA
B04D  A9 FF              ..      LDA #&FF
B04F  85 2A              .*      STA &2A
B051  E6 1B              ..      INC &1B
B053  E0 29              .)      CPX #&29
B055  F0 0A              ..      BEQ &B061
B057  E0 2C              .,      CPX #&2C
B059  D0 DB              ..      BNE &B036
B05B  20 56 AE            V.     JSR &AE56
B05E  20 F0 92            ..     JSR &92F0
.
B061  20 CB BD            ..     JSR &BDCB
B064  68                 h       PLA
B065  A8                 .       TAY
B066  18                 .       CLC
B067  F0 06              ..      BEQ &B06F
B069  E5 36              .6      SBC &36
B06B  B0 C1              ..      BCS &B02E
B06D  88                 .       DEY
B06E  98                 .       TYA
.
B06F  85 2C              .,      STA &2C
B071  AA                 .       TAX
B072  A0 00              ..      LDY #&00
B074  A5 36              .6      LDA &36
B076  38                 8       SEC
B077  E5 2C              .,      SBC &2C
B079  C5 2A              .*      CMP &2A
B07B  B0 02              ..      BCS &B07F
B07D  85 2A              .*      STA &2A
.
B07F  A5 2A              .*      LDA &2A
B081  F0 AB              ..      BEQ &B02E
.
B083  BD 00 06           ...     LDA &0600,X
B086  99 00 06           ...     STA &0600,Y
B089  C8                 .       INY
B08A  E8                 .       INX
B08B  C4 2A              .*      CPY &2A
B08D  D0 F4              ..      BNE &B083
B08F  84 36              .6      STY &36
B091  A9 00              ..      LDA #&00
B093  60                 `       RTS
B094  20 8C 8A            ..     JSR &8A8C
B097  A0 FF              ..      LDY #&FF
B099  C9 7E              .~      CMP #&7E
B09B  F0 04              ..      BEQ &B0A1
B09D  A0 00              ..      LDY #&00
B09F  C6 1B              ..      DEC &1B
.
B0A1  98                 .       TYA
B0A2  48                 H       PHA
B0A3  20 EC AD            ..     JSR &ADEC
B0A6  F0 17              ..      BEQ &B0BF
B0A8  A8                 .       TAY
B0A9  68                 h       PLA
B0AA  85 15              ..      STA &15
B0AC  AD 03 04           ...     LDA &0403
B0AF  D0 08              ..      BNE &B0B9
B0B1  85 37              .7      STA &37
B0B3  20 F9 9E            ..     JSR &9EF9
B0B6  A9 00              ..      LDA #&00
B0B8  60                 `       RTS
.
B0B9  20 DF 9E            ..     JSR &9EDF
B0BC  A9 00              ..      LDA #&00
B0BE  60                 `       RTS
.
B0BF  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
B0C2  20 DD 92            ..     JSR &92DD
B0C5  20 94 BD            ..     JSR &BD94
B0C8  20 AE 8A            ..     JSR &8AAE
B0CB  20 56 AE            V.     JSR &AE56
B0CE  D0 EF              ..      BNE &B0BF
B0D0  20 EA BD            ..     JSR &BDEA
B0D3  A4 36              .6      LDY &36
B0D5  F0 1E              ..      BEQ &B0F5
B0D7  A5 2A              .*      LDA &2A
B0D9  F0 1D              ..      BEQ &B0F8
B0DB  C6 2A              .*      DEC &2A
B0DD  F0 16              ..      BEQ &B0F5
.
B0DF  A2 00              ..      LDX #&00
.
B0E1  BD 00 06           ...     LDA &0600,X
B0E4  99 00 06           ...     STA &0600,Y
B0E7  E8                 .       INX
B0E8  C8                 .       INY
B0E9  F0 10              ..      BEQ &B0FB
B0EB  E4 36              .6      CPX &36
B0ED  90 F2              ..      BCC &B0E1
B0EF  C6 2A              .*      DEC &2A
B0F1  D0 EC              ..      BNE &B0DF
B0F3  84 36              .6      STY &36
.
B0F5  A9 00              ..      LDA #&00
B0F7  60                 `       RTS
.
B0F8  85 36              .6      STA &36
B0FA  60                 `       RTS
.
B0FB  4C 03 9C           L..     JMP &9C03
.
B0FE  68                 h       PLA
B0FF  85 0C              ..      STA &0C
B101  68                 h       PLA
B102  85 0B              ..      STA &0B
B104  00                 .       ...
B105  1D 4E 6F           .No     ORA &6F4E,X
B108  20 73 75            su     JSR &7573
B10B  63                 c       ...
B10C  68                 h       PLA
B10D  20 A4 2F            ./     JSR &2FA4
B110  F2 00              ..      SBC (&00)
.
B112  A5 18              ..      LDA page
B114  85 0C              ..      STA &0C
B116  A9 00              ..      LDA #&00
B118  85 0B              ..      STA &0B
.
B11A  A0 01              ..      LDY #&01
B11C  B1 0B              ..      LDA (&0B),Y
B11E  30 DE              0.      BMI &B0FE
B120  A0 03              ..      LDY #&03
.
B122  C8                 .       INY
B123  B1 0B              ..      LDA (&0B),Y
B125  C9 20              .       CMP #&20
B127  F0 F9              ..      BEQ &B122
B129  C9 DD              ..      CMP #&DD
B12B  F0 0F              ..      BEQ &B13C
.
B12D  A0 03              ..      LDY #&03
B12F  B1 0B              ..      LDA (&0B),Y
B131  18                 .       CLC
B132  65 0B              e.      ADC &0B
B134  85 0B              ..      STA &0B
B136  90 E2              ..      BCC &B11A
B138  E6 0C              ..      INC &0C
B13A  B0 DE              ..      BCS &B11A
.
B13C  C8                 .       INY
B13D  84 0A              ..      STY &0A
B13F  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B142  98                 .       TYA
B143  AA                 .       TAX
B144  18                 .       CLC
B145  65 0B              e.      ADC &0B
B147  A4 0C              ..      LDY &0C
B149  90 02              ..      BCC &B14D
B14B  C8                 .       INY
B14C  18                 .       CLC
.
B14D  E9 00              ..      SBC #&00
B14F  85 3C              .<      STA &3C
B151  98                 .       TYA
B152  E9 00              ..      SBC #&00
B154  85 3D              .=      STA &3D
B156  A0 00              ..      LDY #&00
.
B158  C8                 .       INY
B159  E8                 .       INX
B15A  B1 3C              .<      LDA (&3C),Y
B15C  D1 37              .7      CMP (&37),Y
B15E  D0 CD              ..      BNE &B12D
B160  C4 39              .9      CPY &39
B162  D0 F4              ..      BNE &B158
B164  C8                 .       INY
B165  B1 3C              .<      LDA (&3C),Y
B167  20 26 89            &.     JSR &8926
B16A  B0 C1              ..      BCS &B12D
B16C  8A                 .       TXA
B16D  A8                 .       TAY
B16E  20 6D 98            m.     JSR &986D
B171  20 ED 94            ..     JSR &94ED
B174  A2 01              ..      LDX #&01
B176  20 31 95            1.     JSR &9531
B179  A0 00              ..      LDY #&00
B17B  A5 0B              ..      LDA &0B
B17D  91 02              ..      STA (&02),Y
B17F  C8                 .       INY
B180  A5 0C              ..      LDA &0C
B182  91 02              ..      STA (&02),Y
B184  20 39 95            9.     JSR &9539
B187  4C F4 B1           L..     JMP &B1F4
.
B18A  00                 .       ...
B18B  1E 42 61           .Ba     ASL &6142,Y
B18E  64 20              d       STZ &20
B190  63                 c       ...
B191  61 6C              al      ADC (&6C,X)
B193  6C 00 A9           l..     JMP (&A900)
B196  A4 85              ..      LDY &85
B198  27 BA              '.      RMB2 &BA
B19A  8A                 .       TXA
B19B  18                 .       CLC
B19C  65 04              e.      ADC &04
B19E  20 2E BE            ..     JSR &BE2E
B1A1  A0 00              ..      LDY #&00
B1A3  8A                 .       TXA
B1A4  91 04              ..      STA (&04),Y
.
B1A6  E8                 .       INX
B1A7  C8                 .       INY
B1A8  BD 00 01           ...     LDA &0100,X
B1AB  91 04              ..      STA (&04),Y
B1AD  E0 FF              ..      CPX #&FF
B1AF  D0 F5              ..      BNE &B1A6
B1B1  9A                 .       TXS                        ; Reset stack pointer
B1B2  A5 27              .'      LDA &27
B1B4  48                 H       PHA
B1B5  A5 0A              ..      LDA &0A
B1B7  48                 H       PHA
B1B8  A5 0B              ..      LDA &0B
B1BA  48                 H       PHA
B1BB  A5 0C              ..      LDA &0C
B1BD  48                 H       PHA
B1BE  A5 1B              ..      LDA &1B
B1C0  AA                 .       TAX
B1C1  18                 .       CLC
B1C2  65 19              e.      ADC &19
B1C4  A4 1A              ..      LDY &1A
B1C6  90 02              ..      BCC &B1CA
.
B1C8  C8                 .       INY
B1C9  18                 .       CLC
.
B1CA  E9 01              ..      SBC #&01
B1CC  85 37              .7      STA &37
B1CE  98                 .       TYA
B1CF  E9 00              ..      SBC #&00
B1D1  85 38              .8      STA &38
B1D3  A0 02              ..      LDY #&02
B1D5  20 5B 95            [.     JSR &955B
B1D8  C0 02              ..      CPY #&02
B1DA  F0 AE              ..      BEQ &B18A
B1DC  86 1B              ..      STX &1B
B1DE  88                 .       DEY
B1DF  84 39              .9      STY &39
B1E1  20 5B 94            [.     JSR &945B
B1E4  D0 03              ..      BNE &B1E9
B1E6  4C 12 B1           L..     JMP &B112
.
B1E9  A0 00              ..      LDY #&00
B1EB  B1 2A              .*      LDA (&2A),Y
B1ED  85 0B              ..      STA &0B
B1EF  C8                 .       INY
B1F0  B1 2A              .*      LDA (&2A),Y
B1F2  85 0C              ..      STA &0C
.
B1F4  A9 00              ..      LDA #&00
B1F6  48                 H       PHA
B1F7  85 0A              ..      STA &0A
B1F9  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B1FC  C9 28              .(      CMP #'('
B1FE  F0 4D              .M      BEQ &B24D
B200  C6 0A              ..      DEC &0A
.
B202  A5 1B              ..      LDA &1B
B204  48                 H       PHA
B205  A5 19              ..      LDA &19
B207  48                 H       PHA
B208  A5 1A              ..      LDA &1A
B20A  48                 H       PHA
B20B  20 A3 8B            ..     JSR &8BA3
B20E  68                 h       PLA
B20F  85 1A              ..      STA &1A
B211  68                 h       PLA
B212  85 19              ..      STA &19
B214  68                 h       PLA
B215  85 1B              ..      STA &1B
B217  68                 h       PLA
B218  F0 0C              ..      BEQ &B226
B21A  85 3F              .?      STA &3F
.
B21C  20 0B BE            ..     JSR &BE0B
B21F  20 C1 8C            ..     JSR &8CC1
B222  C6 3F              .?      DEC &3F
B224  D0 F6              ..      BNE &B21C
.
B226  68                 h       PLA
B227  85 0C              ..      STA &0C
B229  68                 h       PLA
B22A  85 0B              ..      STA &0B
B22C  68                 h       PLA
B22D  85 0A              ..      STA &0A
B22F  68                 h       PLA
B230  A0 00              ..      LDY #&00
B232  B1 04              ..      LDA (&04),Y
B234  AA                 .       TAX
B235  9A                 .       TXS
.
B236  C8                 .       INY
B237  E8                 .       INX
B238  B1 04              ..      LDA (&04),Y
B23A  9D 00 01           ...     STA &0100,X
B23D  E0 FF              ..      CPX #&FF
B23F  D0 F5              ..      BNE &B236
B241  98                 .       TYA
B242  65 04              e.      ADC &04
B244  85 04              ..      STA &04
B246  90 02              ..      BCC &B24A
B248  E6 05              ..      INC &05
.
B24A  A5 27              .'      LDA &27
B24C  60                 `       RTS
.
B24D  A5 1B              ..      LDA &1B
B24F  48                 H       PHA
B250  A5 19              ..      LDA &19
B252  48                 H       PHA
B253  A5 1A              ..      LDA &1A
B255  48                 H       PHA
B256  20 82 95            ..     JSR &9582
B259  F0 5A              .Z      BEQ &B2B5
B25B  A5 1B              ..      LDA &1B
B25D  85 0A              ..      STA &0A
B25F  68                 h       PLA
B260  85 1A              ..      STA &1A
B262  68                 h       PLA
B263  85 19              ..      STA &19
B265  68                 h       PLA
B266  85 1B              ..      STA &1B
B268  68                 h       PLA
B269  AA                 .       TAX
B26A  A5 2C              .,      LDA &2C
B26C  48                 H       PHA
B26D  A5 2B              .+      LDA &2B
B26F  48                 H       PHA
B270  A5 2A              .*      LDA &2A
B272  48                 H       PHA
B273  E8                 .       INX
B274  8A                 .       TXA
B275  48                 H       PHA
B276  20 0D B3            ..     JSR &B30D
B279  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B27C  C9 2C              .,      CMP #','
B27E  F0 CD              ..      BEQ &B24D
B280  C9 29              .)      CMP #')'
B282  D0 31              .1      BNE &B2B5
B284  A9 00              ..      LDA #&00
B286  48                 H       PHA
B287  20 8C 8A            ..     JSR &8A8C
B28A  C9 28              .(      CMP #'('
B28C  D0 27              .'      BNE &B2B5
.
B28E  20 29 9B            ).     JSR &9B29
B291  20 90 BD            ..     JSR &BD90
B294  A5 27              .'      LDA &27
B296  85 2D              .-      STA &2D
B298  20 94 BD            ..     JSR &BD94
B29B  68                 h       PLA
B29C  AA                 .       TAX
B29D  E8                 .       INX
B29E  8A                 .       TXA
B29F  48                 H       PHA
B2A0  20 8C 8A            ..     JSR &8A8C
B2A3  C9 2C              .,      CMP #&2C
B2A5  F0 E7              ..      BEQ &B28E
B2A7  C9 29              .)      CMP #&29
B2A9  D0 0A              ..      BNE &B2B5
B2AB  68                 h       PLA
B2AC  68                 h       PLA
B2AD  85 4D              .M      STA &4D
B2AF  85 4E              .N      STA &4E
B2B1  E4 4D              .M      CPX &4D
B2B3  F0 15              ..      BEQ &B2CA
.
B2B5  A2 FB              ..      LDX #&FB
B2B7  9A                 .       TXS
B2B8  68                 h       PLA
B2B9  85 0C              ..      STA &0C
B2BB  68                 h       PLA
B2BC  85 0B              ..      STA &0B
B2BE  00                 .       ...
B2BF  1F 41              .A      BBR1 &B302
B2C1  72 67              rg      ADC (&67)
B2C3  75 6D              um      ADC &6D,X
B2C5  65 6E              en      ADC &6E
B2C7  74 73              ts      STZ &73,X
B2C9  00                 .       ...
.
B2CA  20 EA BD            ..     JSR &BDEA
B2CD  68                 h       PLA
B2CE  85 2A              .*      STA &2A
B2D0  68                 h       PLA
B2D1  85 2B              .+      STA &2B
B2D3  68                 h       PLA
B2D4  85 2C              .,      STA &2C
B2D6  30 21              0!      BMI &B2F9
B2D8  A5 2D              .-      LDA &2D
B2DA  F0 D9              ..      BEQ &B2B5
B2DC  85 27              .'      STA &27
B2DE  A2 37              .7      LDX #&37
B2E0  20 44 BE            D.     JSR &BE44
B2E3  A5 27              .'      LDA &27
B2E5  10 09              ..      BPL &B2F0
B2E7  20 7E BD            ~.     JSR &BD7E
B2EA  20 B5 A3            ..     JSR &A3B5
B2ED  4C F3 B2           L..     JMP &B2F3
.
B2F0  20 EA BD            ..     JSR &BDEA
.
B2F3  20 B7 B4            ..     JSR &B4B7
B2F6  4C 03 B3           L..     JMP &B303
.
B2F9  A5 2D              .-      LDA &2D
B2FB  D0 B8              ..      BNE &B2B5
B2FD  20 CB BD            ..     JSR &BDCB
B300  20 21 8C            !.     JSR &8C21
.
B303  C6 4D              .M      DEC &4D
B305  D0 C3              ..      BNE &B2CA
B307  A5 4E              .N      LDA &4E
B309  48                 H       PHA
B30A  4C 02 B2           L..     JMP &B202
.
B30D  A4 2C              .,      LDY &2C
B30F  C0 04              ..      CPY #&04
B311  D0 05              ..      BNE &B318
B313  A2 37              .7      LDX #&37
B315  20 44 BE            D.     JSR &BE44
.
B318  20 2C B3            ,.     JSR &B32C
B31B  08                 .       PHP
B31C  20 90 BD            ..     JSR &BD90
B31F  28                 (       PLP
B320  F0 07              ..      BEQ &B329
B322  30 05              0.      BMI &B329
B324  A2 37              .7      LDX #&37
B326  20 56 AF            V.     JSR &AF56
.
B329  4C 94 BD           L..     JMP &BD94
.
B32C  A4 2C              .,      LDY &2C
B32E  30 54              0T      BMI &B384
B330  F0 1D              ..      BEQ &B34F
B332  C0 05              ..      CPY #&05
B334  F0 1E              ..      BEQ &B354
B336  A0 03              ..      LDY #&03
B338  B1 2A              .*      LDA (&2A),Y
B33A  85 2D              .-      STA &2D
B33C  88                 .       DEY
B33D  B1 2A              .*      LDA (&2A),Y
B33F  85 2C              .,      STA &2C
B341  88                 .       DEY
B342  B1 2A              .*      LDA (&2A),Y
B344  AA                 .       TAX
B345  88                 .       DEY
B346  B1 2A              .*      LDA (&2A),Y
B348  85 2A              .*      STA &2A
B34A  86 2B              .+      STX &2B
B34C  A9 40              .@      LDA #&40
B34E  60                 `       RTS
.
B34F  B1 2A              .*      LDA (&2A),Y
B351  4C EA AE           L..     JMP &AEEA
.
B354  88                 .       DEY
B355  B1 2A              .*      LDA (&2A),Y
B357  85 34              .4      STA &34
B359  88                 .       DEY
B35A  B1 2A              .*      LDA (&2A),Y
B35C  85 33              .3      STA &33
B35E  88                 .       DEY
B35F  B1 2A              .*      LDA (&2A),Y
B361  85 32              .2      STA &32
B363  88                 .       DEY
B364  B1 2A              .*      LDA (&2A),Y
B366  85 2E              ..      STA &2E
B368  88                 .       DEY
B369  B1 2A              .*      LDA (&2A),Y
B36B  85 30              .0      STA &30
B36D  84 35              .5      STY &35
B36F  84 2F              ./      STY &2F
B371  05 2E              ..      ORA &2E
B373  05 32              .2      ORA &32
B375  05 33              .3      ORA &33
B377  05 34              .4      ORA &34
B379  F0 04              ..      BEQ &B37F
B37B  A5 2E              ..      LDA &2E
B37D  09 80              ..      ORA #&80
.
B37F  85 31              .1      STA &31
B381  A9 FF              ..      LDA #&FF
B383  60                 `       RTS
.
B384  C0 80              ..      CPY #&80
B386  F0 1F              ..      BEQ &B3A7
B388  A0 03              ..      LDY #&03
B38A  B1 2A              .*      LDA (&2A),Y
B38C  85 36              .6      STA &36
B38E  F0 16              ..      BEQ &B3A6
B390  A0 01              ..      LDY #&01
B392  B1 2A              .*      LDA (&2A),Y
B394  85 38              .8      STA &38
B396  88                 .       DEY
B397  B1 2A              .*      LDA (&2A),Y
B399  85 37              .7      STA &37
B39B  A4 36              .6      LDY &36
.
B39D  88                 .       DEY
B39E  B1 37              .7      LDA (&37),Y
B3A0  99 00 06           ...     STA &0600,Y
B3A3  98                 .       TYA
B3A4  D0 F7              ..      BNE &B39D
.
B3A6  60                 `       RTS
.
B3A7  A5 2B              .+      LDA &2B
B3A9  F0 15              ..      BEQ &B3C0
B3AB  A0 00              ..      LDY #&00
.
B3AD  B1 2A              .*      LDA (&2A),Y
B3AF  99 00 06           ...     STA &0600,Y
B3B2  49 0D              I.      EOR #&0D
B3B4  F0 04              ..      BEQ &B3BA
B3B6  C8                 .       INY
B3B7  D0 F4              ..      BNE &B3AD
B3B9  98                 .       TYA
.
B3BA  84 36              .6      STY &36
B3BC  60                 `       RTS
B3BD  20 E3 92            ..     JSR &92E3
.
B3C0  A5 2A              .*      LDA &2A
B3C2  4C C2 AF           L..     JMP &AFC2
.
B3C5  A0 00              ..      LDY #&00
B3C7  84 08              ..      STY &08
B3C9  84 09              ..      STY &09
B3CB  A6 18              ..      LDX page
B3CD  86 38              .8      STX &38
B3CF  84 37              .7      STY &37
B3D1  A6 0C              ..      LDX &0C
B3D3  E0 07              ..      CPX #&07
B3D5  F0 2A              .*      BEQ &B401
B3D7  A6 0B              ..      LDX &0B
.
B3D9  20 42 89            B.     JSR &8942
B3DC  C9 0D              ..      CMP #&0D
B3DE  D0 19              ..      BNE &B3F9
B3E0  E4 37              .7      CPX &37
B3E2  A5 0C              ..      LDA &0C
B3E4  E5 38              .8      SBC &38
B3E6  90 19              ..      BCC &B401
B3E8  20 42 89            B.     JSR &8942
B3EB  09 00              ..      ORA #&00
B3ED  30 12              0.      BMI &B401
B3EF  85 09              ..      STA &09
B3F1  20 42 89            B.     JSR &8942
B3F4  85 08              ..      STA &08
B3F6  20 42 89            B.     JSR &8942
.
B3F9  E4 37              .7      CPX &37
B3FB  A5 0C              ..      LDA &0C
B3FD  E5 38              .8      SBC &38
B3FF  B0 D8              ..      BCS &B3D9
.
B401  60                 `       RTS

; -----------------------------------------------------------------------------
; The BRKV handler (default BASIC error handler)
; -----------------------------------------------------------------------------
._brkv
B402  20 C5 B3            ..     JSR &B3C5
B405  84 20              .       STY &20
B407  B1 FD              ..      LDA (&FD),Y
B409  D0 08              ..      BNE &B413
B40B  A9 33              .3      LDA #&B433 MOD 256                   ; Default ON ERROR message
B40D  85 16              ..      STA &16
B40F  A9 B4              ..      LDA #&B433 DIV 256
B411  85 17              ..      STA &17
.
B413  A5 16              ..      LDA &16                              ; Pointer to BASIC line number error handler
B415  85 0B              ..      STA &0B
B417  A5 17              ..      LDA &17
B419  85 0C              ..      STA &0C                              ; Set PtrA base to ON ERROR message
B41B  20 3A BD            :.     JSR &BD3A
B41E  AA                 .       TAX                                  ; X = 0
B41F  86 0A              ..      STX &0A                              ; Reset PtrA offset
B421  A9 DA              ..      LDA #&DA                             ; OSBYTE &DA: Read/write number of items in the VDU queue
B423  20 F4 FF            ..     JSR osbyte
B426  A9 7E              .~      LDA #&7E                             ; OSBYTE &7E: Acknowledge detection of an ESCAPE condition
B428  20 F4 FF            ..     JSR osbyte
B42B  A2 FF              ..      LDX #&FF
B42D  86 28              .(      STX &28                              ; Reset assembler OPT mask
B42F  9A                 .       TXS                                  ; Reset stack pointer
B430  4C A3 8B           L..     JMP &8BA3

; -----------------------------------------------------------------------------
; Default ON ERROR statement
; -----------------------------------------------------------------------------
.
B433  F6 3A              .:      INC &3A,X                            ; REPORT:IFERLPRINT" at line ";ERL:ENDELSEPRINT:END
B435  E7 9E              ..      SMB6 &9E
B437  F1 22              ."      SBC (&22),Y
B439  20 61 74            at     JSR &7461
B43C  20 6C 69            li     JSR &696C
B43F  6E 65 20           ne      ROR &2065
B442  22                 "       ...
B443  3B                 ;       ...
B444  9E 3A E0           .:.     STZ &E03A,X
B447  8B                 .       ...
B448  F1 3A              .:      SBC (&3A),Y
B44A  E0 0D              ..      CPX #&0D


B44C  20 21 88            !.     JSR &8821
B44F  A2 03              ..      LDX #&03
.
B451  A5 2A              .*      LDA &2A
B453  48                 H       PHA
B454  A5 2B              .+      LDA &2B
B456  48                 H       PHA
B457  8A                 .       TXA
B458  48                 H       PHA
B459  20 DA 92            ..     JSR &92DA
B45C  68                 h       PLA
B45D  AA                 .       TAX
B45E  CA                 .       DEX
B45F  D0 F0              ..      BNE &B451
B461  20 52 98            R.     JSR &9852
B464  A5 2A              .*      LDA &2A
B466  85 3D              .=      STA &3D
B468  A5 2B              .+      LDA &2B
B46A  85 3E              .>      STA &3E
B46C  A0 07              ..      LDY #&07
B46E  A2 05              ..      LDX #&05
B470  D0 1D              ..      BNE &B48F
B472  20 21 88            !.     JSR &8821
B475  A2 0D              ..      LDX #&0D
.
B477  A5 2A              .*      LDA &2A
B479  48                 H       PHA
B47A  8A                 .       TXA
B47B  48                 H       PHA
B47C  20 DA 92            ..     JSR &92DA
B47F  68                 h       PLA
B480  AA                 .       TAX
B481  CA                 .       DEX
B482  D0 F3              ..      BNE &B477
B484  20 52 98            R.     JSR &9852
B487  A5 2A              .*      LDA &2A
B489  85 44              .D      STA &44
B48B  A2 0C              ..      LDX #&0C
B48D  A0 08              ..      LDY #&08                   ; OSWORD &08: Define an ENVELOPE
.
B48F  68                 h       PLA
B490  95 37              .7      STA &37,X
B492  CA                 .       DEX
B493  10 FA              ..      BPL &B48F
B495  98                 .       TYA
B496  A2 37              .7      LDX #&37
B498  A0 00              ..      LDY #&00
B49A  20 F1 FF            ..     JSR osword
B49D  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
B4A0  20 21 88            !.     JSR &8821
B4A3  20 52 98            R.     JSR &9852
B4A6  A4 2A              .*      LDY &2A
B4A8  88                 .       DEY
B4A9  84 23              .#      STY width
B4AB  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
B4AE  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
B4B1  20 29 9B            ).     JSR &9B29
.
B4B4  20 0B BE            ..     JSR &BE0B
.
B4B7  A5 39              .9      LDA &39
B4B9  C9 05              ..      CMP #&05
B4BB  F0 23              .#      BEQ &B4E0
B4BD  A5 27              .'      LDA &27
B4BF  F0 ED              ..      BEQ &B4AE
B4C1  10 03              ..      BPL &B4C6
B4C3  20 E4 A3            ..     JSR &A3E4
.
B4C6  A0 00              ..      LDY #&00
B4C8  A5 2A              .*      LDA &2A
B4CA  91 37              .7      STA (&37),Y
B4CC  A5 39              .9      LDA &39
B4CE  F0 0F              ..      BEQ &B4DF
B4D0  A5 2B              .+      LDA &2B
B4D2  C8                 .       INY
B4D3  91 37              .7      STA (&37),Y
B4D5  A5 2C              .,      LDA &2C
B4D7  C8                 .       INY
B4D8  91 37              .7      STA (&37),Y
B4DA  A5 2D              .-      LDA &2D
B4DC  C8                 .       INY
B4DD  91 37              .7      STA (&37),Y
.
B4DF  60                 `       RTS
.
B4E0  A5 27              .'      LDA &27
B4E2  F0 CA              ..      BEQ &B4AE
B4E4  30 03              0.      BMI &B4E9
B4E6  20 BE A2            ..     JSR &A2BE
.
B4E9  A0 00              ..      LDY #&00
B4EB  A5 30              .0      LDA &30
B4ED  91 37              .7      STA (&37),Y
B4EF  C8                 .       INY
B4F0  A5 2E              ..      LDA &2E
B4F2  29 80              ).      AND #&80
B4F4  85 2E              ..      STA &2E
B4F6  A5 31              .1      LDA &31
B4F8  29 7F              )      AND #&7F
B4FA  05 2E              ..      ORA &2E
B4FC  91 37              .7      STA (&37),Y
B4FE  C8                 .       INY
B4FF  A5 32              .2      LDA &32
B501  91 37              .7      STA (&37),Y
B503  C8                 .       INY
B504  A5 33              .3      LDA &33
B506  91 37              .7      STA (&37),Y
B508  C8                 .       INY
B509  A5 34              .4      LDA &34
B50B  91 37              .7      STA (&37),Y
B50D  60                 `       RTS

; -----------------------------------------------------------------------------
; Print character or token
; -----------------------------------------------------------------------------
.
B50E  85 37              .7      STA &37
B510  C9 80              ..      CMP #&80
B512  90 44              .D      BCC &B558
B514  A9 71              .q      LDA #&71
B516  85 38              .8      STA &38
B518  A9 80              ..      LDA #&80
B51A  85 39              .9      STA &39
B51C  84 3A              .:      STY &3A
.
B51E  A0 00              ..      LDY #&00
.
B520  C8                 .       INY
B521  B1 38              .8      LDA (&38),Y
B523  10 FB              ..      BPL &B520
B525  C5 37              .7      CMP &37
B527  F0 0D              ..      BEQ &B536
B529  C8                 .       INY
B52A  98                 .       TYA
B52B  38                 8       SEC
B52C  65 38              e8      ADC &38
B52E  85 38              .8      STA &38
B530  90 EC              ..      BCC &B51E
B532  E6 39              .9      INC &39
B534  B0 E8              ..      BCS &B51E
.
B536  A0 00              ..      LDY #&00
.
B538  B1 38              .8      LDA (&38),Y
B53A  30 06              0.      BMI &B542
B53C  20 58 B5            X.     JSR &B558
B53F  C8                 .       INY
B540  D0 F6              ..      BNE &B538
.
B542  A4 3A              .:      LDY &3A
B544  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
B545  48                 H       PHA
B546  4A                 J       LSR A
B547  4A                 J       LSR A
B548  4A                 J       LSR A
B549  4A                 J       LSR A
B54A  20 50 B5            P.     JSR &B550
B54D  68                 h       PLA
B54E  29 0F              ).      AND #&0F
.
B550  C9 0A              ..      CMP #&0A
B552  90 02              ..      BCC &B556
B554  69 06              i.      ADC #&06
.
B556  69 30              i0      ADC #&30

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
B558  C9 0D              ..      CMP #&0D
B55A  D0 0B              ..      BNE &B567
B55C  20 EE FF            ..     JSR oswrch
B55F  4C 28 BC           L(.     JMP &BC28                  ; Reset COUNT variable to 0
.
B562  20 45 B5            E.     JSR &B545
.
B565  A9 20              .       LDA #&20
.
B567  48                 H       PHA
B568  A5 23              .#      LDA width
B56A  C5 1E              ..      CMP &1E
B56C  B0 03              ..      BCS &B571                  ; Jump if WIDTH >= COUNT
B56E  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
.
B571  68                 h       PLA
B572  E6 1E              ..      INC &1E                    ; Increase COUNT by one
B574  6C 0E 02           l..     JMP (wrchv)                ; Why an indirect call to wrch?

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
B577  25 1F              %.      AND listo
B579  F0 0E              ..      BEQ &B589
B57B  8A                 .       TXA
B57C  F0 0B              ..      BEQ &B589
B57E  30 E5              0.      BMI &B565
.
B580  20 65 B5            e.     JSR &B565
B583  20 58 B5            X.     JSR &B558
B586  CA                 .       DEX
B587  D0 F7              ..      BNE &B580
.
B589  60                 `       RTS
.
B58A  E6 0A              ..      INC &0A
B58C  20 1D 9B            ..     JSR &9B1D
B58F  20 4C 98            L.     JSR &984C
B592  20 EE 92            ..     JSR &92EE
B595  A5 2A              .*      LDA &2A
B597  85 1F              ..      STA listo
B599  4C F6 8A           L..     JMP &8AF6
B59C  C8                 .       INY
B59D  B1 0B              ..      LDA (&0B),Y
B59F  C9 4F              .O      CMP #&4F
B5A1  F0 E7              ..      BEQ &B58A
B5A3  A9 00              ..      LDA #&00
B5A5  85 3B              .;      STA &3B
B5A7  85 3C              .<      STA &3C
B5A9  20 D8 AE            ..     JSR &AED8
B5AC  20 DF 97            ..     JSR &97DF
B5AF  08                 .       PHP
B5B0  20 94 BD            ..     JSR &BD94
B5B3  A9 FF              ..      LDA #&FF
B5B5  85 2A              .*      STA &2A
B5B7  A9 7F              .      LDA #&7F
B5B9  85 2B              .+      STA &2B
B5BB  28                 (       PLP
B5BC  90 11              ..      BCC &B5CF
B5BE  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B5C1  C9 2C              .,      CMP #','
B5C3  F0 13              ..      BEQ &B5D8
B5C5  20 EA BD            ..     JSR &BDEA
B5C8  20 94 BD            ..     JSR &BD94
B5CB  C6 0A              ..      DEC &0A
B5CD  10 0C              ..      BPL &B5DB
.
B5CF  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B5D2  C9 2C              .,      CMP #','
B5D4  F0 02              ..      BEQ &B5D8
B5D6  C6 0A              ..      DEC &0A
.
B5D8  20 DF 97            ..     JSR &97DF
.
B5DB  A5 2A              .*      LDA &2A
B5DD  85 31              .1      STA &31
B5DF  A5 2B              .+      LDA &2B
B5E1  85 32              .2      STA &32
B5E3  20 57 98            W.     JSR &9857                  ; Check end of statement
B5E6  20 6F BE            o.     JSR &BE6F
B5E9  20 EA BD            ..     JSR &BDEA
B5EC  20 70 99            p.     JSR &9970
B5EF  A5 3D              .=      LDA &3D
B5F1  85 0B              ..      STA &0B
B5F3  A5 3E              .>      LDA &3E
B5F5  85 0C              ..      STA &0C
B5F7  90 16              ..      BCC &B60F
B5F9  88                 .       DEY
B5FA  B0 06              ..      BCS &B602
.
B5FC  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
B5FF  20 6D 98            m.     JSR &986D
.
B602  B1 0B              ..      LDA (&0B),Y
B604  85 2B              .+      STA &2B
B606  C8                 .       INY
B607  B1 0B              ..      LDA (&0B),Y
B609  85 2A              .*      STA &2A
B60B  C8                 .       INY
B60C  C8                 .       INY
B60D  84 0A              ..      STY &0A
.
B60F  A5 2A              .*      LDA &2A
B611  18                 .       CLC
B612  E5 31              .1      SBC &31
B614  A5 2B              .+      LDA &2B
B616  E5 32              .2      SBC &32
B618  90 03              ..      BCC &B61D
B61A  4C F6 8A           L..     JMP &8AF6
.
B61D  20 23 99            #.     JSR &9923
B620  A2 FF              ..      LDX #&FF
B622  86 4D              .M      STX &4D
B624  A9 01              ..      LDA #&01
B626  20 77 B5            w.     JSR &B577
B629  A6 3B              .;      LDX &3B
B62B  A9 02              ..      LDA #&02
B62D  20 77 B5            w.     JSR &B577
B630  A6 3C              .<      LDX &3C
B632  A9 04              ..      LDA #&04
B634  20 77 B5            w.     JSR &B577
.
B637  A4 0A              ..      LDY &0A
.
B639  B1 0B              ..      LDA (&0B),Y
B63B  C9 0D              ..      CMP #&0D
B63D  F0 BD              ..      BEQ &B5FC
B63F  C9 22              ."      CMP #&22
B641  D0 0E              ..      BNE &B651
B643  A9 FF              ..      LDA #&FF
B645  45 4D              EM      EOR &4D
B647  85 4D              .M      STA &4D
B649  A9 22              ."      LDA #&22
.
B64B  20 58 B5            X.     JSR &B558
B64E  C8                 .       INY
B64F  D0 E8              ..      BNE &B639
.
B651  24 4D              $M      BIT &4D
B653  10 F6              ..      BPL &B64B
B655  C9 8D              ..      CMP #&8D
B657  D0 0F              ..      BNE &B668
B659  20 EB 97            ..     JSR &97EB
B65C  84 0A              ..      STY &0A
B65E  A9 00              ..      LDA #&00
B660  85 14              ..      STA &14
B662  20 1F 99            ..     JSR &991F
B665  4C 37 B6           L7.     JMP &B637
.
B668  C9 E3              ..      CMP #&E3
B66A  D0 02              ..      BNE &B66E
B66C  E6 3B              .;      INC &3B
.
B66E  C9 ED              ..      CMP #&ED
B670  D0 06              ..      BNE &B678
B672  A6 3B              .;      LDX &3B
B674  F0 02              ..      BEQ &B678
B676  C6 3B              .;      DEC &3B
.
B678  C9 F5              ..      CMP #&F5
B67A  D0 02              ..      BNE &B67E
B67C  E6 3C              .<      INC &3C
.
B67E  C9 FD              ..      CMP #&FD
B680  D0 06              ..      BNE &B688
B682  A6 3C              .<      LDX &3C
B684  F0 02              ..      BEQ &B688
B686  C6 3C              .<      DEC &3C
.
B688  20 0E B5            ..     JSR &B50E                  ; Print character or token
B68B  C8                 .       INY
B68C  D0 AB              ..      BNE &B639
.
B68E  00                 .       ...
B68F  20 4E 6F            No     JSR &6F4E
B692  20 E3 00            ..     JSR &00E3
.
B695  20 C9 95            ..     JSR &95C9
B698  D0 09              ..      BNE &B6A3
B69A  A6 26              .&      LDX &26
B69C  F0 F0              ..      BEQ &B68E
B69E  B0 37              .7      BCS &B6D7
.
B6A0  4C 2A 98           L*.     JMP &982A
.
B6A3  B0 FB              ..      BCS &B6A0
B6A5  A6 26              .&      LDX &26
B6A7  F0 E5              ..      BEQ &B68E
.
B6A9  A5 2A              .*      LDA &2A
B6AB  DD F1 04           ...     CMP &04F1,X
B6AE  D0 0E              ..      BNE &B6BE
B6B0  A5 2B              .+      LDA &2B
B6B2  DD F2 04           ...     CMP &04F2,X
B6B5  D0 07              ..      BNE &B6BE
B6B7  A5 2C              .,      LDA &2C
B6B9  DD F3 04           ...     CMP &04F3,X
B6BC  F0 19              ..      BEQ &B6D7
.
B6BE  8A                 .       TXA
B6BF  38                 8       SEC
B6C0  E9 0F              ..      SBC #&0F
B6C2  AA                 .       TAX
B6C3  86 26              .&      STX &26
B6C5  D0 E2              ..      BNE &B6A9
B6C7  00                 .       ...
B6C8  21 43              !C      AND (&43,X)
B6CA  61 6E              an      ADC (&6E,X)
B6CC  27 74              't      RMB2 &74
B6CE  20 4D 61            Ma     JSR &614D
B6D1  74 63              tc      STZ &63,X
B6D3  68                 h       PLA
B6D4  20 E3 00            ..     JSR &00E3
.
B6D7  BD F1 04           ...     LDA &04F1,X
B6DA  85 2A              .*      STA &2A
B6DC  BD F2 04           ...     LDA &04F2,X
B6DF  85 2B              .+      STA &2B
B6E1  BC F3 04           ...     LDY &04F3,X
B6E4  C0 05              ..      CPY #&05
B6E6  F0 7E              .~      BEQ &B766
B6E8  A0 00              ..      LDY #&00
B6EA  B1 2A              .*      LDA (&2A),Y
B6EC  7D F4 04           }..     ADC &04F4,X
B6EF  91 2A              .*      STA (&2A),Y
B6F1  85 37              .7      STA &37
B6F3  C8                 .       INY
B6F4  B1 2A              .*      LDA (&2A),Y
B6F6  7D F5 04           }..     ADC &04F5,X
B6F9  91 2A              .*      STA (&2A),Y
B6FB  85 38              .8      STA &38
B6FD  C8                 .       INY
B6FE  B1 2A              .*      LDA (&2A),Y
B700  7D F6 04           }..     ADC &04F6,X
B703  91 2A              .*      STA (&2A),Y
B705  85 39              .9      STA &39
B707  C8                 .       INY
B708  B1 2A              .*      LDA (&2A),Y
B70A  7D F7 04           }..     ADC &04F7,X
B70D  91 2A              .*      STA (&2A),Y
B70F  A8                 .       TAY
B710  A5 37              .7      LDA &37
B712  38                 8       SEC
B713  FD F9 04           ...     SBC &04F9,X
B716  85 37              .7      STA &37
B718  A5 38              .8      LDA &38
B71A  FD FA 04           ...     SBC &04FA,X
B71D  85 38              .8      STA &38
B71F  A5 39              .9      LDA &39
B721  FD FB 04           ...     SBC &04FB,X
B724  85 39              .9      STA &39
B726  98                 .       TYA
B727  FD FC 04           ...     SBC &04FC,X
B72A  05 37              .7      ORA &37
B72C  05 38              .8      ORA &38
B72E  05 39              .9      ORA &39
B730  F0 0F              ..      BEQ &B741
B732  98                 .       TYA
B733  5D F7 04           ]..     EOR &04F7,X
B736  5D FC 04           ]..     EOR &04FC,X
B739  10 04              ..      BPL &B73F
B73B  B0 04              ..      BCS &B741
B73D  90 12              ..      BCC &B751
.
B73F  B0 10              ..      BCS &B751
.
B741  BC FE 04           ...     LDY &04FE,X
B744  BD FF 04           ...     LDA &04FF,X
B747  84 0B              ..      STY &0B
B749  85 0C              ..      STA &0C
B74B  20 77 98            w.     JSR &9877
B74E  4C A3 8B           L..     JMP &8BA3
.
B751  A5 26              .&      LDA &26
B753  38                 8       SEC
B754  E9 0F              ..      SBC #&0F
B756  85 26              .&      STA &26
B758  A4 1B              ..      LDY &1B
B75A  84 0A              ..      STY &0A
B75C  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B75F  C9 2C              .,      CMP #','
B761  D0 3E              .>      BNE &B7A1
B763  4C 95 B6           L..     JMP &B695
.
B766  20 54 B3            T.     JSR &B354
B769  A5 26              .&      LDA &26
B76B  18                 .       CLC
B76C  69 F4              i.      ADC #&F4
B76E  85 4B              .K      STA &4B
B770  A9 05              ..      LDA #&05
B772  85 4C              .L      STA &4C
B774  20 00 A5            ..     JSR &A500
B777  A5 2A              .*      LDA &2A
B779  85 37              .7      STA &37
B77B  A5 2B              .+      LDA &2B
B77D  85 38              .8      STA &38
B77F  20 E9 B4            ..     JSR &B4E9
B782  A5 26              .&      LDA &26
B784  85 27              .'      STA &27
B786  18                 .       CLC
B787  69 F9              i.      ADC #&F9
B789  85 4B              .K      STA &4B
B78B  A9 05              ..      LDA #&05
B78D  85 4C              .L      STA &4C
B78F  20 5F 9A            _.     JSR &9A5F
B792  F0 AD              ..      BEQ &B741
B794  BD F5 04           ...     LDA &04F5,X
B797  30 04              0.      BMI &B79D
B799  B0 A6              ..      BCS &B741
B79B  90 B4              ..      BCC &B751
.
B79D  90 A2              ..      BCC &B741
B79F  B0 B0              ..      BCS &B751
.
B7A1  4C 96 8B           L..     JMP &8B96
.
B7A4  00                 .       ...
B7A5  22                 "       ...
B7A6  E3                 .       ...
B7A7  20 76 61            va     JSR &6176
B7AA  72 69              ri      ADC (&69)
B7AC  61 62              ab      ADC (&62,X)
B7AE  6C 65 00           le.     JMP (&0065)
B7B1  23                 #       ...
B7B2  54                 T       ...
B7B3  6F 6F              oo      BBR6 &B824
B7B5  20 6D 61            ma     JSR &616D
B7B8  6E 79 20           ny      ROR &2079
B7BB  E3                 .       ...
B7BC  73                 s       ...
.
B7BD  00                 .       ...
B7BE  24 4E              $N      BIT &4E
B7C0  6F 20              o       BBR6 &B7E2
B7C2  B8                 .       CLV
B7C3  00                 .       ...
B7C4  20 82 95            ..     JSR &9582
B7C7  F0 DB              ..      BEQ &B7A4
B7C9  B0 D9              ..      BCS &B7A4
B7CB  20 94 BD            ..     JSR &BD94
B7CE  20 41 98            A.     JSR &9841
B7D1  20 B1 B4            ..     JSR &B4B1
B7D4  A4 26              .&      LDY &26
B7D6  C0 96              ..      CPY #&96
B7D8  B0 D6              ..      BCS &B7B0
B7DA  A5 37              .7      LDA &37
B7DC  99 00 05           ...     STA &0500,Y
B7DF  A5 38              .8      LDA &38
B7E1  99 01 05           ...     STA &0501,Y
B7E4  A5 39              .9      LDA &39
B7E6  99 02 05           ...     STA &0502,Y
B7E9  AA                 .       TAX
B7EA  20 8C 8A            ..     JSR &8A8C
B7ED  C9 B8              ..      CMP #&B8
B7EF  D0 CC              ..      BNE &B7BD
B7F1  E0 05              ..      CPX #&05
B7F3  F0 5A              .Z      BEQ &B84F
B7F5  20 DD 92            ..     JSR &92DD
B7F8  A4 26              .&      LDY &26
B7FA  A5 2A              .*      LDA &2A
B7FC  99 08 05           ...     STA &0508,Y
B7FF  A5 2B              .+      LDA &2B
B801  99 09 05           ...     STA &0509,Y
B804  A5 2C              .,      LDA &2C
B806  99 0A 05           ...     STA &050A,Y
B809  A5 2D              .-      LDA &2D
B80B  99 0B 05           ...     STA &050B,Y
B80E  A9 01              ..      LDA #&01
B810  20 D8 AE            ..     JSR &AED8
B813  20 8C 8A            ..     JSR &8A8C
B816  C9 88              ..      CMP #&88
B818  D0 05              ..      BNE &B81F
B81A  20 DD 92            ..     JSR &92DD
B81D  A4 1B              ..      LDY &1B
.
B81F  84 0A              ..      STY &0A
B821  A4 26              .&      LDY &26
B823  A5 2A              .*      LDA &2A
B825  99 03 05           ...     STA &0503,Y
B828  A5 2B              .+      LDA &2B
B82A  99 04 05           ...     STA &0504,Y
B82D  A5 2C              .,      LDA &2C
B82F  99 05 05           ...     STA &0505,Y
B832  A5 2D              .-      LDA &2D
B834  99 06 05           ...     STA &0506,Y
.
B837  20 80 98            ..     JSR &9880
B83A  A4 26              .&      LDY &26
B83C  A5 0B              ..      LDA &0B
B83E  99 0D 05           ...     STA &050D,Y
B841  A5 0C              ..      LDA &0C
B843  99 0E 05           ...     STA &050E,Y
B846  18                 .       CLC
B847  98                 .       TYA
B848  69 0F              i.      ADC #&0F
B84A  85 26              .&      STA &26
B84C  4C A3 8B           L..     JMP &8BA3
.
B84F  20 29 9B            ).     JSR &9B29
B852  20 FD 92            ..     JSR &92FD
B855  A5 26              .&      LDA &26
B857  18                 .       CLC
B858  69 08              i.      ADC #&08
B85A  85 4B              .K      STA &4B
B85C  A9 05              ..      LDA #&05
B85E  85 4C              .L      STA &4C
B860  20 8D A3            ..     JSR &A38D
B863  20 99 A6            ..     JSR &A699
B866  20 8C 8A            ..     JSR &8A8C
B869  C9 88              ..      CMP #&88
B86B  D0 08              ..      BNE &B875
B86D  20 29 9B            ).     JSR &9B29
B870  20 FD 92            ..     JSR &92FD
B873  A4 1B              ..      LDY &1B
.
B875  84 0A              ..      STY &0A
B877  A5 26              .&      LDA &26
B879  18                 .       CLC
B87A  69 03              i.      ADC #&03
B87C  85 4B              .K      STA &4B
B87E  A9 05              ..      LDA #&05
B880  85 4C              .L      STA &4C
B882  20 8D A3            ..     JSR &A38D
B885  4C 37 B8           L7.     JMP &B837
B888  20 9A B9            ..     JSR &B99A
.
B88B  20 57 98            W.     JSR &9857                  ; Check end of statement
B88E  A4 25              .%      LDY &25
B890  C0 1A              ..      CPY #&1A
B892  B0 0E              ..      BCS &B8A2
B894  A5 0B              ..      LDA &0B
B896  99 CC 05           ...     STA &05CC,Y
B899  A5 0C              ..      LDA &0C
B89B  99 E6 05           ...     STA &05E6,Y
B89E  E6 25              .%      INC &25
B8A0  90 30              .0      BCC &B8D2
.
B8A2  00                 .       ...
B8A3  25 54              %T      AND &54
B8A5  6F 6F              oo      BBR6 &B916
B8A7  20 6D 61            ma     JSR &616D
B8AA  6E 79 20           ny      ROR &2079
B8AD  E4 73              .s      CPX &73
.
B8AF  00                 .       ...
B8B0  26 4E              &N      ROL &4E
B8B2  6F 20              o       BBR6 &B8D4
B8B4  E4 00              ..      CPX &00
B8B6  20 57 98            W.     JSR &9857                  ; Check end of statement
B8B9  A6 25              .%      LDX &25
B8BB  F0 F2              ..      BEQ &B8AF
B8BD  C6 25              .%      DEC &25
B8BF  BC CB 05           ...     LDY &05CB,X
B8C2  BD E5 05           ...     LDA &05E5,X
B8C5  84 0B              ..      STY &0B
B8C7  85 0C              ..      STA &0C
B8C9  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
B8CC  20 9A B9            ..     JSR &B99A
B8CF  20 57 98            W.     JSR &9857                  ; Check end of statement
.
B8D2  A5 20              .       LDA &20
.
B8D4  F0 03              ..      BEQ &B8D9
B8D6  20 05 99            ..     JSR &9905
.
B8D9  A4 3D              .=      LDY &3D
B8DB  A5 3E              .>      LDA &3E
.
B8DD  84 0B              ..      STY &0B
B8DF  85 0C              ..      STA &0C
B8E1  4C A3 8B           L..     JMP &8BA3
.
B8E4  20 57 98            W.     JSR &9857                  ; Check end of statement
B8E7  A9 33              .3      LDA #&B433 MOD 256         ; Set ON ERROR pointer to the default ON ERROR string
B8E9  85 16              ..      STA &16
B8EB  A9 B4              ..      LDA #&B433 DIV 256
B8ED  85 17              ..      STA &17
B8EF  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
B8F2  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B8F5  C9 87              ..      CMP #&87                   ; 'OFF' token
B8F7  F0 EB              ..      BEQ &B8E4
B8F9  A4 0A              ..      LDY &0A
B8FB  88                 .       DEY
B8FC  20 6D 98            m.     JSR &986D
B8FF  A5 0B              ..      LDA &0B
B901  85 16              ..      STA &16
B903  A5 0C              ..      LDA &0C
B905  85 17              ..      STA &17
B907  4C 7D 8B           L}.     JMP &8B7D
.
B90A  00                 .       ...
B90B  27 EE              '.      RMB2 &EE
B90D  20 73 79            sy     JSR &7973
B910  6E 74 61           nta     ROR &6174
B913  78                 x       SEI
B914  00                 .       ...
B915  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B918  C9 85              ..      CMP #&85                   ; 'ERROR' token
B91A  F0 D6              ..      BEQ &B8F2
B91C  C6 0A              ..      DEC &0A
B91E  20 1D 9B            ..     JSR &9B1D
B921  20 F0 92            ..     JSR &92F0
B924  A4 1B              ..      LDY &1B
B926  C8                 .       INY
B927  84 0A              ..      STY &0A
B929  E0 E5              ..      CPX #&E5
B92B  F0 04              ..      BEQ &B931
B92D  E0 E4              ..      CPX #&E4
B92F  D0 D9              ..      BNE &B90A
.
B931  8A                 .       TXA
B932  48                 H       PHA
B933  A5 2B              .+      LDA &2B
B935  05 2C              .,      ORA &2C
B937  05 2D              .-      ORA &2D
B939  D0 42              .B      BNE &B97D
B93B  A6 2A              .*      LDX &2A
B93D  F0 3E              .>      BEQ &B97D
B93F  CA                 .       DEX
B940  F0 1A              ..      BEQ &B95C
B942  A4 0A              ..      LDY &0A
.
B944  B1 0B              ..      LDA (&0B),Y
B946  C8                 .       INY
B947  C9 0D              ..      CMP #&0D
B949  F0 32              .2      BEQ &B97D
B94B  C9 3A              .:      CMP #&3A
B94D  F0 2E              ..      BEQ &B97D
B94F  C9 8B              ..      CMP #&8B
B951  F0 2A              .*      BEQ &B97D
B953  C9 2C              .,      CMP #&2C
B955  D0 ED              ..      BNE &B944
B957  CA                 .       DEX
B958  D0 EA              ..      BNE &B944
B95A  84 0A              ..      STY &0A
.
B95C  20 9A B9            ..     JSR &B99A
B95F  68                 h       PLA
B960  C9 E4              ..      CMP #&E4
B962  F0 06              ..      BEQ &B96A
B964  20 77 98            w.     JSR &9877
B967  4C D2 B8           L..     JMP &B8D2
.
B96A  A4 0A              ..      LDY &0A
.
B96C  B1 0B              ..      LDA (&0B),Y
B96E  C8                 .       INY
B96F  C9 0D              ..      CMP #&0D
B971  F0 04              ..      BEQ &B977
B973  C9 3A              .:      CMP #&3A
B975  D0 F5              ..      BNE &B96C
.
B977  88                 .       DEY
B978  84 0A              ..      STY &0A
B97A  4C 8B B8           L..     JMP &B88B
.
B97D  A4 0A              ..      LDY &0A
B97F  68                 h       PLA
.
B980  B1 0B              ..      LDA (&0B),Y
B982  C8                 .       INY
B983  C9 8B              ..      CMP #&8B
B985  F0 0E              ..      BEQ &B995
B987  C9 0D              ..      CMP #&0D
B989  D0 F5              ..      BNE &B980
B98B  00                 .       ...
B98C  28                 (       PLP
B98D  EE 20 72           . r     INC &7220
B990  61 6E              an      ADC (&6E,X)
B992  67 65              ge      RMB6 &65
B994  00                 .       ...
.
B995  84 0A              ..      STY &0A
B997  4C E3 98           L..     JMP &98E3
.
B99A  20 DF 97            ..     JSR &97DF
B99D  B0 10              ..      BCS &B9AF
B99F  20 1D 9B            ..     JSR &9B1D
B9A2  20 F0 92            ..     JSR &92F0
B9A5  A5 1B              ..      LDA &1B
B9A7  85 0A              ..      STA &0A
B9A9  A5 2B              .+      LDA &2B
B9AB  29 7F              )      AND #&7F
B9AD  85 2B              .+      STA &2B
.
B9AF  20 70 99            p.     JSR &9970
B9B2  B0 01              ..      BCS &B9B5
B9B4  60                 `       RTS
.
B9B5  00                 .       ...
B9B6  29 4E              )N      AND #&4E
B9B8  6F 20              o       BBR6 &B9DA
B9BA  73                 s       ...
B9BB  75 63              uc      ADC &63,X
B9BD  68                 h       PLA
B9BE  20 6C 69            li     JSR &696C
B9C1  6E 65 00           ne.     ROR &0065
.
B9C4  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
B9C7  4C 2A 98           L*.     JMP &982A
.
B9CA  84 0A              ..      STY &0A
B9CC  4C 98 8B           L..     JMP &8B98
.
B9CF  C6 0A              ..      DEC &0A
B9D1  20 A9 BF            ..     JSR &BFA9
B9D4  A5 1B              ..      LDA &1B
B9D6  85 0A              ..      STA &0A
B9D8  84 4D              .M      STY &4D
.
B9DA  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
B9DD  C9 2C              .,      CMP #','
B9DF  D0 E9              ..      BNE &B9CA
B9E1  A5 4D              .M      LDA &4D
B9E3  48                 H       PHA
B9E4  20 82 95            ..     JSR &9582
B9E7  F0 DE              ..      BEQ &B9C7
B9E9  A5 1B              ..      LDA &1B
B9EB  85 0A              ..      STA &0A
B9ED  68                 h       PLA
B9EE  85 4D              .M      STA &4D
B9F0  08                 .       PHP
B9F1  20 94 BD            ..     JSR &BD94
B9F4  A4 4D              .M      LDY &4D
B9F6  20 D7 FF            ..     JSR osbget
B9F9  85 27              .'      STA &27
B9FB  28                 (       PLP
B9FC  90 1B              ..      BCC &BA19
B9FE  A5 27              .'      LDA &27
BA00  D0 C2              ..      BNE &B9C4
BA02  20 D7 FF            ..     JSR osbget
BA05  85 36              .6      STA &36
BA07  AA                 .       TAX
BA08  F0 09              ..      BEQ &BA13
.
BA0A  20 D7 FF            ..     JSR osbget
BA0D  9D FF 05           ...     STA &05FF,X
BA10  CA                 .       DEX
BA11  D0 F7              ..      BNE &BA0A
.
BA13  20 1E 8C            ..     JSR &8C1E
BA16  4C DA B9           L..     JMP &B9DA
.
BA19  A5 27              .'      LDA &27
BA1B  F0 A7              ..      BEQ &B9C4
BA1D  30 0C              0.      BMI &BA2B
BA1F  A2 03              ..      LDX #&03
.
BA21  20 D7 FF            ..     JSR osbget
BA24  95 2A              .*      STA &2A,X
BA26  CA                 .       DEX
BA27  10 F8              ..      BPL &BA21
BA29  30 0E              0.      BMI &BA39
.
BA2B  A2 04              ..      LDX #&04
.
BA2D  20 D7 FF            ..     JSR osbget
BA30  9D 6C 04           .l.     STA &046C,X
BA33  CA                 .       DEX
BA34  10 F7              ..      BPL &BA2D
BA36  20 B2 A3            ..     JSR &A3B2
.
BA39  20 B4 B4            ..     JSR &B4B4
BA3C  4C DA B9           L..     JMP &B9DA
.
BA3F  68                 h       PLA
BA40  68                 h       PLA
BA41  4C 98 8B           L..     JMP &8B98
BA44  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
BA47  C9 23              .#      CMP #'#'
BA49  F0 84              ..      BEQ &B9CF
BA4B  C9 86              ..      CMP #&86                   ; 'LINE' token
BA4D  F0 03              ..      BEQ &BA52
BA4F  C6 0A              ..      DEC &0A
BA51  18                 .       CLC
.
BA52  66 4D              fM      ROR &4D
BA54  46 4D              FM      LSR &4D
BA56  A9 FF              ..      LDA #&FF
BA58  85 4E              .N      STA &4E
.
BA5A  20 8A 8E            ..     JSR &8E8A
BA5D  B0 0A              ..      BCS &BA69
.
BA5F  20 8A 8E            ..     JSR &8E8A
BA62  90 FB              ..      BCC &BA5F
BA64  A2 FF              ..      LDX #&FF
BA66  86 4E              .N      STX &4E
BA68  18                 .       CLC
.
BA69  08                 .       PHP
BA6A  06 4D              .M      ASL &4D
BA6C  28                 (       PLP
BA6D  66 4D              fM      ROR &4D
BA6F  C9 2C              .,      CMP #&2C
BA71  F0 E7              ..      BEQ &BA5A
BA73  C9 3B              .;      CMP #&3B
BA75  F0 E3              ..      BEQ &BA5A
BA77  C6 0A              ..      DEC &0A
BA79  A5 4D              .M      LDA &4D
BA7B  48                 H       PHA
BA7C  A5 4E              .N      LDA &4E
BA7E  48                 H       PHA
BA7F  20 82 95            ..     JSR &9582
BA82  F0 BB              ..      BEQ &BA3F
BA84  68                 h       PLA
BA85  85 4E              .N      STA &4E
BA87  68                 h       PLA
BA88  85 4D              .M      STA &4D
BA8A  A5 1B              ..      LDA &1B
BA8C  85 0A              ..      STA &0A
BA8E  08                 .       PHP
BA8F  24 4D              $M      BIT &4D
BA91  70 06              p.      BCS &BA99
BA93  A5 4E              .N      LDA &4E
BA95  C9 FF              ..      CMP #&FF
BA97  D0 17              ..      BNE &BAB0
.
BA99  24 4D              $M      BIT &4D
BA9B  10 05              ..      BPL &BAA2
BA9D  A9 3F              .?      LDA #&3F
BA9F  20 58 B5            X.     JSR &B558
.
BAA2  20 FC BB            ..     JSR &BBFC
BAA5  84 36              .6      STY &36
BAA7  06 4D              .M      ASL &4D
BAA9  18                 .       CLC
BAAA  66 4D              fM      ROR &4D
BAAC  24 4D              $M      BIT &4D
BAAE  70 1D              p.      BCS &BACD
.
BAB0  85 1B              ..      STA &1B
BAB2  A9 00              ..      LDA #&00
BAB4  85 19              ..      STA &19
BAB6  A9 06              ..      LDA #&06
BAB8  85 1A              ..      STA &1A
BABA  20 AD AD            ..     JSR &ADAD
.
BABD  20 8C 8A            ..     JSR &8A8C
BAC0  C9 2C              .,      CMP #&2C
BAC2  F0 06              ..      BEQ &BACA
BAC4  C9 0D              ..      CMP #&0D
BAC6  D0 F5              ..      BNE &BABD
BAC8  A0 FE              ..      LDY #&FE
.
BACA  C8                 .       INY
BACB  84 4E              .N      STY &4E
.
BACD  28                 (       PLP
BACE  B0 0C              ..      BCS &BADC
BAD0  20 94 BD            ..     JSR &BD94
BAD3  20 34 AC            4.     JSR &AC34
BAD6  20 B4 B4            ..     JSR &B4B4
BAD9  4C 5A BA           LZ.     JMP &BA5A
.
BADC  A9 00              ..      LDA #&00
BADE  85 27              .'      STA &27
BAE0  20 21 8C            !.     JSR &8C21
BAE3  4C 5A BA           LZ.     JMP &BA5A
BAE6  A0 00              ..      LDY #&00
BAE8  84 3D              .=      STY &3D
BAEA  A4 18              ..      LDY page
BAEC  84 3E              .>      STY &3E
BAEE  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
BAF1  C6 0A              ..      DEC &0A
BAF3  C9 3A              .:      CMP #':'
BAF5  F0 10              ..      BEQ &BB07
BAF7  C9 0D              ..      CMP #&0D
BAF9  F0 0C              ..      BEQ &BB07
BAFB  C9 8B              ..      CMP #&8B                   ; 'ELSE' token
BAFD  F0 08              ..      BEQ &BB07
BAFF  20 9A B9            ..     JSR &B99A
BB02  A0 01              ..      LDY #&01
BB04  20 55 BE            U.     JSR &BE55
.
BB07  20 57 98            W.     JSR &9857                  ; Check end of statement
BB0A  A5 3D              .=      LDA &3D
BB0C  85 1C              ..      STA &1C
BB0E  A5 3E              .>      LDA &3E
BB10  85 1D              ..      STA &1D
BB12  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
BB15  20 97 8A            ..     JSR &8A97                  ; Find first non-SPACE character in PtrA
BB18  C9 2C              .,      CMP #','
BB1A  F0 03              ..      BEQ &BB1F
BB1C  4C 96 8B           L..     JMP &8B96
.
BB1F  20 82 95            ..     JSR &9582
BB22  F0 F1              ..      BEQ &BB15
BB24  B0 0C              ..      BCS &BB32
BB26  20 50 BB            P.     JSR &BB50
BB29  20 94 BD            ..     JSR &BD94
BB2C  20 B1 B4            ..     JSR &B4B1
BB2F  4C 40 BB           L@.     JMP &BB40
.
BB32  20 50 BB            P.     JSR &BB50
BB35  20 94 BD            ..     JSR &BD94
BB38  20 AD AD            ..     JSR &ADAD
BB3B  85 27              .'      STA &27
BB3D  20 1E 8C            ..     JSR &8C1E
.
BB40  18                 .       CLC
BB41  A5 1B              ..      LDA &1B
BB43  65 19              e.      ADC &19
BB45  85 1C              ..      STA &1C
BB47  A5 1A              ..      LDA &1A
BB49  69 00              i.      ADC #&00
BB4B  85 1D              ..      STA &1D
BB4D  4C 15 BB           L..     JMP &BB15
.
BB50  A5 1B              ..      LDA &1B
BB52  85 0A              ..      STA &0A
BB54  A5 1C              ..      LDA &1C
BB56  85 19              ..      STA &19
BB58  A5 1D              ..      LDA &1D
BB5A  85 1A              ..      STA &1A
BB5C  A0 00              ..      LDY #&00
BB5E  84 1B              ..      STY &1B
BB60  20 8C 8A            ..     JSR &8A8C
BB63  C9 2C              .,      CMP #&2C
BB65  F0 49              .I      BEQ &BBB0
BB67  C9 DC              ..      CMP #&DC
BB69  F0 45              .E      BEQ &BBB0
BB6B  C9 0D              ..      CMP #&0D
BB6D  F0 0B              ..      BEQ &BB7A
.
BB6F  20 8C 8A            ..     JSR &8A8C
BB72  C9 2C              .,      CMP #&2C
BB74  F0 3A              .:      BEQ &BBB0
BB76  C9 0D              ..      CMP #&0D
BB78  D0 F5              ..      BNE &BB6F
.
BB7A  A4 1B              ..      LDY &1B
BB7C  B1 19              ..      LDA (&19),Y
BB7E  30 1C              0.      BMI &BB9C
BB80  C8                 .       INY
BB81  C8                 .       INY
BB82  B1 19              ..      LDA (&19),Y
BB84  AA                 .       TAX
.
BB85  C8                 .       INY
BB86  B1 19              ..      LDA (&19),Y
BB88  C9 20              .       CMP #&20
BB8A  F0 F9              ..      BEQ &BB85
BB8C  C9 DC              ..      CMP #&DC
BB8E  F0 1D              ..      BEQ &BBAD
BB90  8A                 .       TXA
BB91  18                 .       CLC
BB92  65 19              e.      ADC &19
BB94  85 19              ..      STA &19
BB96  90 E2              ..      BCC &BB7A
BB98  E6 1A              ..      INC &1A
BB9A  B0 DE              ..      BCS &BB7A
.
BB9C  00                 .       ...
BB9D  2A                 *       ROL A
BB9E  4F 75              Ou      BBR4 &BC15
BBA0  74 20              t       STZ &20,X
BBA2  6F 66              of      BBR6 &BC0A
BBA4  20 DC 00            ..     JSR &00DC
BBA7  2B                 +       ...
BBA8  4E 6F 20           No      LSR &206F
BBAB  F5 00              ..      SBC &00,X
.
BBAD  C8                 .       INY
BBAE  84 1B              ..      STY &1B
.
BBB0  60                 `       RTS
BBB1  20 1D 9B            ..     JSR &9B1D
BBB4  20 4C 98            L.     JSR &984C
BBB7  20 EE 92            ..     JSR &92EE
BBBA  A6 24              .$      LDX &24
BBBC  F0 E8              ..      BEQ &BBA6
BBBE  A5 2A              .*      LDA &2A
BBC0  05 2B              .+      ORA &2B
BBC2  05 2C              .,      ORA &2C
BBC4  05 2D              .-      ORA &2D
BBC6  F0 05              ..      BEQ &BBCD
BBC8  C6 24              .$      DEC &24
BBCA  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
BBCD  BC A3 05           ...     LDY &05A3,X
BBD0  BD B7 05           ...     LDA &05B7,X
BBD3  4C DD B8           L..     JMP &B8DD
.
BBD6  00                 .       ...
BBD7  2C 54 6F           ,To     BIT &6F54
BBDA  6F 20              o       BBR6 &BBFC
BBDC  6D 61 6E           man     ADC &6E61
BBDF  79 20 F5           y .     ADC &F520,Y
BBE2  73                 s       ...
BBE3  00                 .       ...
BBE4  A6 24              .$      LDX &24
BBE6  E0 14              ..      CPX #&14
BBE8  B0 EC              ..      BCS &BBD6
BBEA  20 6D 98            m.     JSR &986D
BBED  A5 0B              ..      LDA &0B
BBEF  9D A4 05           ...     STA &05A4,X
BBF2  A5 0C              ..      LDA &0C
BBF4  9D B8 05           ...     STA &05B8,X
BBF7  E6 24              .$      INC &24
BBF9  4C A3 8B           L..     JMP &8BA3

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
BBFC  A0 00              ..      LDY #&0600 MOD 256         ; Store OSWORD &00 line at &0600
BBFE  A9 06              ..      LDA #&0600 DIV 256
BC00  D0 07              ..      BNE &BC09
.
BC02  20 58 B5            X.     JSR &B558
BC05  A0 00              ..      LDY #&0700 MOD 256         ; Store OSWORD &00 line at &0700
BC07  A9 07              ..      LDA #&0700 DIV 256
.
BC09  84 37              .7      STY &37
BC0B  85 38              .8      STA &38
BC0D  A9 EE              ..      LDA #&EE                   ; Maximum line length is &EE
BC0F  85 39              .9      STA &39
BC11  A9 20              .       LDA #&20                   ; Minimum acceptable ASCII value is 32
BC13  85 3A              .:      STA &3A
BC15  A0 FF              ..      LDY #&FF                   ; Maximum acceptable ASCII value is 255
BC17  84 3B              .;      STY &3B
BC19  C8                 .       INY
BC1A  A2 37              .7      LDX #&0037 MOD 256         ; OSWORD &00 parameterblock is at &0037
BC1C  98                 .       TYA
BC1D  20 F1 FF            ..     JSR osword                 ; Do an OSWORD &00: Read line from current input stream
BC20  90 06              ..      BCC &BC28                  ; Jump if ESCAPE wasn't pressed
BC22  4C 38 98           L8.     JMP &9838                  ; Generate "Escape" error message

; -----------------------------------------------------------------------------
; Print CR/LF and reset COUNT variable to 0
; -----------------------------------------------------------------------------
.newline
BC25  20 E7 FF            ..     JSR osnewl
.newline1
BC28  A9 00              ..      LDA #&00
BC2A  85 1E              ..      STA &1E
BC2C  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
BC2D  20 70 99            p.     JSR &9970
BC30  B0 4E              .N      BCS &BC80
BC32  A5 3D              .=      LDA &3D
BC34  E9 02              ..      SBC #&02
BC36  85 37              .7      STA &37
BC38  85 3D              .=      STA &3D
BC3A  85 12              ..      STA &12
BC3C  A5 3E              .>      LDA &3E
BC3E  E9 00              ..      SBC #&00
BC40  85 38              .8      STA &38
BC42  85 13              ..      STA &13
BC44  85 3E              .>      STA &3E
BC46  A0 03              ..      LDY #&03
BC48  B1 37              .7      LDA (&37),Y
BC4A  18                 .       CLC
BC4B  65 37              e7      ADC &37
BC4D  85 37              .7      STA &37
BC4F  90 02              ..      BCC &BC53
BC51  E6 38              .8      INC &38
.
BC53  A0 00              ..      LDY #&00
.
BC55  B1 37              .7      LDA (&37),Y
BC57  91 12              ..      STA (&12),Y
BC59  C9 0D              ..      CMP #&0D
BC5B  F0 09              ..      BEQ &BC66
.
BC5D  C8                 .       INY
BC5E  D0 F5              ..      BNE &BC55
BC60  E6 38              .8      INC &38
BC62  E6 13              ..      INC &13
BC64  D0 EF              ..      BNE &BC55
.
BC66  C8                 .       INY
BC67  D0 04              ..      BNE &BC6D
BC69  E6 38              .8      INC &38
BC6B  E6 13              ..      INC &13
.
BC6D  B1 37              .7      LDA (&37),Y
BC6F  91 12              ..      STA (&12),Y
BC71  30 09              0.      BMI &BC7C
BC73  20 81 BC            ..     JSR &BC81
BC76  20 81 BC            ..     JSR &BC81
BC79  4C 5D BC           L].     JMP &BC5D
.
BC7C  20 92 BE            ..     JSR &BE92
BC7F  18                 .       CLC
.
BC80  60                 `       RTS
.
BC81  C8                 .       INY
BC82  D0 04              ..      BNE &BC88
BC84  E6 13              ..      INC &13
BC86  E6 38              .8      INC &38
.
BC88  B1 37              .7      LDA (&37),Y
BC8A  91 12              ..      STA (&12),Y
BC8C  60                 `       RTS
.
BC8D  84 3B              .;      STY &3B
BC8F  20 2D BC            -.     JSR &BC2D
BC92  A0 07              ..      LDY #&07
BC94  84 3C              .<      STY &3C
BC96  A0 00              ..      LDY #&00
BC98  A9 0D              ..      LDA #&0D
BC9A  D1 3B              .;      CMP (&3B),Y
BC9C  F0 72              .r      BEQ &BD10
.
BC9E  C8                 .       INY
BC9F  D1 3B              .;      CMP (&3B),Y
BCA1  D0 FB              ..      BNE &BC9E
BCA3  C8                 .       INY
BCA4  C8                 .       INY
BCA5  C8                 .       INY
BCA6  84 3F              .?      STY &3F
BCA8  E6 3F              .?      INC &3F
BCAA  A5 12              ..      LDA &12
BCAC  85 39              .9      STA &39
BCAE  A5 13              ..      LDA &13
BCB0  85 3A              .:      STA &3A
BCB2  20 92 BE            ..     JSR &BE92
BCB5  85 37              .7      STA &37
BCB7  A5 13              ..      LDA &13
BCB9  85 38              .8      STA &38
BCBB  88                 .       DEY
BCBC  A5 06              ..      LDA &06
BCBE  C5 12              ..      CMP &12
BCC0  A5 07              ..      LDA &07
BCC2  E5 13              ..      SBC &13
BCC4  B0 10              ..      BCS &BCD6
BCC6  20 6F BE            o.     JSR &BE6F
BCC9  20 20 BD             .     JSR &BD20
BCCC  00                 .       ...
BCCD  00                 .       ...
BCCE  86 20              .       STX &20
BCD0  73                 s       ...
BCD1  70 61              pa      BCS &BD34
BCD3  63                 c       ...
BCD4  65 00              e.      ADC &00
.
BCD6  B1 39              .9      LDA (&39),Y
BCD8  91 37              .7      STA (&37),Y
BCDA  98                 .       TYA
BCDB  D0 04              ..      BNE &BCE1
BCDD  C6 3A              .:      DEC &3A
BCDF  C6 38              .8      DEC &38
.
BCE1  88                 .       DEY
BCE2  98                 .       TYA
BCE3  65 39              e9      ADC &39
BCE5  A6 3A              .:      LDX &3A
BCE7  90 01              ..      BCC &BCEA
BCE9  E8                 .       INX
.
BCEA  C5 3D              .=      CMP &3D
BCEC  8A                 .       TXA
BCED  E5 3E              .>      SBC &3E
BCEF  B0 E5              ..      BCS &BCD6
BCF1  38                 8       SEC
BCF2  A0 01              ..      LDY #&01
BCF4  A5 2B              .+      LDA &2B
BCF6  91 3D              .=      STA (&3D),Y
BCF8  C8                 .       INY
BCF9  A5 2A              .*      LDA &2A
BCFB  91 3D              .=      STA (&3D),Y
BCFD  C8                 .       INY
BCFE  A5 3F              .?      LDA &3F
BD00  91 3D              .=      STA (&3D),Y
BD02  20 56 BE            V.     JSR &BE56
BD05  A0 FF              ..      LDY #&FF
.
BD07  C8                 .       INY
BD08  B1 3B              .;      LDA (&3B),Y
BD0A  91 3D              .=      STA (&3D),Y
BD0C  C9 0D              ..      CMP #&0D
BD0E  D0 F7              ..      BNE &BD07
.
BD10  60                 `       RTS
BD11  20 57 98            W.     JSR &9857                  ; Check end of statement
.
BD14  20 20 BD             .     JSR &BD20
BD17  A5 18              ..      LDA page
BD19  85 0C              ..      STA &0C
BD1B  86 0B              ..      STX &0B
BD1D  4C 0B 8B           L..     JMP &8B0B

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
BD20  A5 12              ..      LDA &12                    ; Set LOMEM and HEAP pointer to TOP
BD22  85 00              ..      STA &00
BD24  85 02              ..      STA &02
BD26  A5 13              ..      LDA &13
BD28  85 01              ..      STA &01
BD2A  85 03              ..      STA &03
BD2C  20 3A BD            :.     JSR &BD3A
.
BD2F  A2 80              ..      LDX #&80
BD31  A9 00              ..      LDA #&00
.
BD33  9D 7F 04           ..     STA &047F,X                ; Clear VLL (Variables linked list)
BD36  CA                 .       DEX
BD37  D0 FA              ..      BNE &BD33
BD39  60                 `       RTS

; -----------------------------------------------------------------------------
; 
; -----------------------------------------------------------------------------
.
BD3A  A5 18              ..      LDA page
BD3C  85 1D              ..      STA &1D                    ; Set pointer to next DATA item to PAGE
BD3E  A5 06              ..      LDA &06
BD40  85 04              ..      STA &04                    ; Set BASIC stack pointer to HIMEM
BD42  A5 07              ..      LDA &07
BD44  85 05              ..      STA &05
BD46  A9 00              ..      LDA #&00                   ; Set number of REPEATs, GOSUBs and FORs to 0
BD48  85 24              .$      STA &24
BD4A  85 26              .&      STA &26
BD4C  85 25              .%      STA &25
BD4E  85 1C              ..      STA &1C
BD50  60                 `       RTS

.
BD51  A5 04              ..      LDA &04
BD53  38                 8       SEC
BD54  E9 05              ..      SBC #&05
BD56  20 2E BE            ..     JSR &BE2E
BD59  A0 00              ..      LDY #&00
BD5B  A5 30              .0      LDA &30
BD5D  91 04              ..      STA (&04),Y
BD5F  C8                 .       INY
BD60  A5 2E              ..      LDA &2E
BD62  29 80              ).      AND #&80
BD64  85 2E              ..      STA &2E
BD66  A5 31              .1      LDA &31
BD68  29 7F              )      AND #&7F
BD6A  05 2E              ..      ORA &2E
BD6C  91 04              ..      STA (&04),Y
BD6E  C8                 .       INY
BD6F  A5 32              .2      LDA &32
BD71  91 04              ..      STA (&04),Y
BD73  C8                 .       INY
BD74  A5 33              .3      LDA &33
BD76  91 04              ..      STA (&04),Y
BD78  C8                 .       INY
BD79  A5 34              .4      LDA &34
BD7B  91 04              ..      STA (&04),Y
BD7D  60                 `       RTS
.
BD7E  A5 04              ..      LDA &04
BD80  18                 .       CLC
BD81  85 4B              .K      STA &4B
BD83  69 05              i.      ADC #&05
BD85  85 04              ..      STA &04
BD87  A5 05              ..      LDA &05
BD89  85 4C              .L      STA &4C
BD8B  69 00              i.      ADC #&00
BD8D  85 05              ..      STA &05
BD8F  60                 `       RTS
.
BD90  F0 20              .       BEQ &BDB2
BD92  30 BD              0.      BMI &BD51
.
BD94  A5 04              ..      LDA &04
BD96  38                 8       SEC
BD97  E9 04              ..      SBC #&04
BD99  20 2E BE            ..     JSR &BE2E
BD9C  A0 03              ..      LDY #&03
BD9E  A5 2D              .-      LDA &2D
BDA0  91 04              ..      STA (&04),Y
BDA2  88                 .       DEY
BDA3  A5 2C              .,      LDA &2C
BDA5  91 04              ..      STA (&04),Y
BDA7  88                 .       DEY
BDA8  A5 2B              .+      LDA &2B
BDAA  91 04              ..      STA (&04),Y
BDAC  88                 .       DEY
BDAD  A5 2A              .*      LDA &2A
BDAF  91 04              ..      STA (&04),Y
BDB1  60                 `       RTS
.
BDB2  18                 .       CLC
BDB3  A5 04              ..      LDA &04
BDB5  E5 36              .6      SBC &36
BDB7  20 2E BE            ..     JSR &BE2E
BDBA  A4 36              .6      LDY &36
BDBC  F0 08              ..      BEQ &BDC6
.
BDBE  B9 FF 05           ...     LDA &05FF,Y
BDC1  91 04              ..      STA (&04),Y
BDC3  88                 .       DEY
BDC4  D0 F8              ..      BNE &BDBE
.
BDC6  A5 36              .6      LDA &36
BDC8  91 04              ..      STA (&04),Y
BDCA  60                 `       RTS
.
BDCB  A0 00              ..      LDY #&00
BDCD  B1 04              ..      LDA (&04),Y
BDCF  85 36              .6      STA &36
BDD1  F0 09              ..      BEQ &BDDC
BDD3  A8                 .       TAY
.
BDD4  B1 04              ..      LDA (&04),Y
BDD6  99 FF 05           ...     STA &05FF,Y
BDD9  88                 .       DEY
BDDA  D0 F8              ..      BNE &BDD4
.
BDDC  A0 00              ..      LDY #&00
BDDE  B1 04              ..      LDA (&04),Y
BDE0  38                 8       SEC
.
BDE1  65 04              e.      ADC &04
BDE3  85 04              ..      STA &04
BDE5  90 23              .#      BCC &BE0A
BDE7  E6 05              ..      INC &05
BDE9  60                 `       RTS
.
BDEA  A0 03              ..      LDY #&03
BDEC  B1 04              ..      LDA (&04),Y
BDEE  85 2D              .-      STA &2D
BDF0  88                 .       DEY
BDF1  B1 04              ..      LDA (&04),Y
BDF3  85 2C              .,      STA &2C
BDF5  88                 .       DEY
BDF6  B1 04              ..      LDA (&04),Y
BDF8  85 2B              .+      STA &2B
BDFA  88                 .       DEY
BDFB  B1 04              ..      LDA (&04),Y
BDFD  85 2A              .*      STA &2A
.
BDFF  18                 .       CLC
BE00  A5 04              ..      LDA &04
BE02  69 04              i.      ADC #&04
BE04  85 04              ..      STA &04
BE06  90 02              ..      BCC &BE0A
BE08  E6 05              ..      INC &05
.
BE0A  60                 `       RTS
.
BE0B  A2 37              .7      LDX #&37
.
BE0D  A0 03              ..      LDY #&03
BE0F  B1 04              ..      LDA (&04),Y
BE11  95 03              ..      STA &03,X
BE13  88                 .       DEY
BE14  B1 04              ..      LDA (&04),Y
BE16  95 02              ..      STA &02,X
BE18  88                 .       DEY
BE19  B1 04              ..      LDA (&04),Y
BE1B  95 01              ..      STA &01,X
BE1D  88                 .       DEY
BE1E  B1 04              ..      LDA (&04),Y
BE20  95 00              ..      STA &00,X
BE22  18                 .       CLC
BE23  A5 04              ..      LDA &04
BE25  69 04              i.      ADC #&04
BE27  85 04              ..      STA &04
BE29  90 DF              ..      BCC &BE0A
BE2B  E6 05              ..      INC &05
BE2D  60                 `       RTS
.
BE2E  85 04              ..      STA &04
BE30  B0 02              ..      BCS &BE34
BE32  C6 05              ..      DEC &05
.
BE34  A4 05              ..      LDY &05
BE36  C4 03              ..      CPY &03
BE38  90 07              ..      BCC &BE41
BE3A  D0 04              ..      BNE &BE40
BE3C  C5 02              ..      CMP &02
BE3E  90 01              ..      BCC &BE41
.
BE40  60                 `       RTS
.
BE41  4C B7 8C           L..     JMP &8CB7                  ; Generate "No room" error message
.
BE44  A5 2A              .*      LDA &2A
BE46  95 00              ..      STA &00,X
BE48  A5 2B              .+      LDA &2B
BE4A  95 01              ..      STA &01,X
BE4C  A5 2C              .,      LDA &2C
BE4E  95 02              ..      STA &02,X
BE50  A5 2D              .-      LDA &2D
BE52  95 03              ..      STA &03,X
BE54  60                 `       RTS
.
BE55  18                 .       CLC
.
BE56  98                 .       TYA
BE57  65 3D              e=      ADC &3D
BE59  85 3D              .=      STA &3D
BE5B  90 02              ..      BCC &BE5F
BE5D  E6 3E              .>      INC &3E
.
BE5F  A0 01              ..      LDY #&01
BE61  60                 `       RTS
.
BE62  20 DD BE            ..     JSR &BEDD
BE65  A8                 .       TAY
BE66  A9 FF              ..      LDA #&FF                   ; OSFILE &FF: Load file
BE68  84 3D              .=      STY &3D
BE6A  A2 37              .7      LDX #&37
BE6C  20 DD FF            ..     JSR osfile
.
BE6F  A5 18              ..      LDA page
BE71  85 13              ..      STA &13
BE73  A0 00              ..      LDY #&00
BE75  84 12              ..      STY &12
BE77  C8                 .       INY
.
BE78  88                 .       DEY
BE79  B1 12              ..      LDA (&12),Y
BE7B  C9 0D              ..      CMP #&0D
BE7D  D0 1F              ..      BNE &BE9E
BE7F  C8                 .       INY
BE80  B1 12              ..      LDA (&12),Y
BE82  30 0C              0.      BMI &BE90
BE84  A0 03              ..      LDY #&03
BE86  B1 12              ..      LDA (&12),Y
BE88  F0 14              ..      BEQ &BE9E
BE8A  18                 .       CLC
BE8B  20 93 BE            ..     JSR &BE93
BE8E  D0 E8              ..      BNE &BE78
.
BE90  C8                 .       INY
BE91  18                 .       CLC
.
BE92  98                 .       TYA
.
BE93  65 12              e.      ADC &12
BE95  85 12              ..      STA &12
BE97  90 02              ..      BCC &BE9B
BE99  E6 13              ..      INC &13
.
BE9B  A0 01              ..      LDY #&01
BE9D  60                 `       RTS
.
BE9E  20 CF BF            ..     JSR &BFCF
BEA1  0D 42 61           .Ba     ORA &6142
BEA4  64 20              d       STZ &20
BEA6  70 72              pr      BCS &BF1A
BEA8  6F 67              og      BBR6 &BF11
BEAA  72 61              ra      ADC (&61)
BEAC  6D 0D EA           m..     ADC &EA0D
BEAF  4C F6 8A           L..     JMP &8AF6
.
BEB2  A9 00              ..      LDA #&00
BEB4  85 37              .7      STA &37
BEB6  A9 06              ..      LDA #&06
BEB8  85 38              .8      STA &38
.
BEBA  A4 36              .6      LDY &36
BEBC  A9 0D              ..      LDA #&0D
BEBE  99 00 06           ...     STA &0600,Y
BEC1  60                 `       RTS
BEC2  20 D2 BE            ..     JSR &BED2
BEC5  A2 00              ..      LDX #&00
BEC7  A0 06              ..      LDY #&06
BEC9  20 F7 FF            ..     JSR oscli
BECC  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
BECF  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
.
BED2  20 1D 9B            ..     JSR &9B1D
BED5  D0 F8              ..      BNE &BECF
BED7  20 B2 BE            ..     JSR &BEB2
BEDA  4C 4C 98           LL.     JMP &984C
.
BEDD  20 D2 BE            ..     JSR &BED2
BEE0  88                 .       DEY
BEE1  84 39              .9      STY &39
BEE3  A5 18              ..      LDA page
BEE5  85 3A              .:      STA &3A
.
BEE7  A9 82              ..      LDA #&82                   ; OSBYTE &82: Read machine high order address
BEE9  20 F4 FF            ..     JSR osbyte
BEEC  86 3B              .;      STX &3B
BEEE  84 3C              .<      STY &3C
BEF0  A9 00              ..      LDA #&00
BEF2  60                 `       RTS
BEF3  20 6F BE            o.     JSR &BE6F
BEF6  A5 12              ..      LDA &12
BEF8  85 45              .E      STA &45
BEFA  A5 13              ..      LDA &13
BEFC  85 46              .F      STA &46
BEFE  A9 23              .#      LDA #&23
BF00  85 3D              .=      STA &3D
BF02  A9 80              ..      LDA #&80
BF04  85 3E              .>      STA &3E
BF06  A5 18              ..      LDA page
BF08  85 42              .B      STA &42
BF0A  20 DD BE            ..     JSR &BEDD
BF0D  86 3F              .?      STX &3F
BF0F  84 40              .@      STY &40
.
BF11  86 43              .C      STX &43
BF13  84 44              .D      STY &44
BF15  86 47              .G      STX &47
BF17  84 48              .H      STY &48
BF19  85 41              .A      STA &41
BF1B  A8                 .       TAY
BF1C  A2 37              .7      LDX #&37                   ; OSFILE &--: 
BF1E  20 DD FF            ..     JSR osfile
BF21  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
BF24  20 62 BE            b.     JSR &BE62
BF27  4C F3 8A           L..     JMP &8AF3
BF2A  20 62 BE            b.     JSR &BE62
BF2D  4C 14 BD           L..     JMP &BD14
BF30  20 A9 BF            ..     JSR &BFA9
BF33  48                 H       PHA
BF34  20 13 98            ..     JSR &9813
BF37  20 EE 92            ..     JSR &92EE
BF3A  68                 h       PLA
BF3B  A8                 .       TAY
BF3C  A2 2A              .*      LDX #&2A
BF3E  A9 01              ..      LDA #&01                   ; OSARGS &01: 
BF40  20 DA FF            ..     JSR osargs
BF43  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
BF46  38                 8       SEC
BF47  A9 00              ..      LDA #&00
BF49  2A                 *       ROL A
BF4A  2A                 *       ROL A
BF4B  48                 H       PHA
BF4C  20 B5 BF            ..     JSR &BFB5
BF4F  A2 2A              .*      LDX #&2A
BF51  68                 h       PLA                        ; OSARGS &--: 
BF52  20 DA FF            ..     JSR osargs
BF55  A9 40              .@      LDA #&40
BF57  60                 `       RTS
BF58  20 A9 BF            ..     JSR &BFA9
BF5B  48                 H       PHA
BF5C  20 AE 8A            ..     JSR &8AAE
BF5F  20 49 98            I.     JSR &9849
BF62  20 EE 92            ..     JSR &92EE
BF65  68                 h       PLA
BF66  A8                 .       TAY
BF67  A5 2A              .*      LDA &2A
BF69  20 D4 FF            ..     JSR osbput
BF6C  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
BF6F  20 B5 BF            ..     JSR &BFB5
BF72  20 D7 FF            ..     JSR osbget
BF75  4C D8 AE           L..     JMP &AED8
BF78  A9 40              .@      LDA #&40                   ; OSFIND &40: Open file for input only
BF7A  D0 06              ..      BNE &BF82
BF7C  A9 80              ..      LDA #&80                   ; OSFIND &80: Open file for output only
BF7E  D0 02              ..      BNE &BF82
BF80  A9 C0              ..      LDA #&C0                   ; OSFIND &C0: Open file for random access
.
BF82  48                 H       PHA
BF83  20 EC AD            ..     JSR &ADEC
BF86  D0 0E              ..      BNE &BF96
BF88  20 BA BE            ..     JSR &BEBA
BF8B  A2 00              ..      LDX #&00
BF8D  A0 06              ..      LDY #&06
BF8F  68                 h       PLA
BF90  20 CE FF            ..     JSR osfind
BF93  4C D8 AE           L..     JMP &AED8
.
BF96  4C 0E 8C           L..     JMP &8C0E                  ; Generate "Type mismatch" error message
BF99  20 A9 BF            ..     JSR &BFA9
BF9C  20 52 98            R.     JSR &9852
BF9F  A4 2A              .*      LDY &2A
BFA1  A9 00              ..      LDA #&00                   ; OSFIND &00: Close file
BFA3  20 CE FF            ..     JSR osfind
BFA6  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop
.
BFA9  A5 0A              ..      LDA &0A
BFAB  85 1B              ..      STA &1B
BFAD  A5 0B              ..      LDA &0B
BFAF  85 19              ..      STA &19
BFB1  A5 0C              ..      LDA &0C
BFB3  85 1A              ..      STA &1A
.
BFB5  20 8C 8A            ..     JSR &8A8C
BFB8  C9 23              .#      CMP #&23
BFBA  D0 07              ..      BNE &BFC3
BFBC  20 E3 92            ..     JSR &92E3
BFBF  A4 2A              .*      LDY &2A
BFC1  98                 .       TYA
BFC2  60                 `       RTS
.
BFC3  00                 .       ...
BFC4  2D 4D 69           -Mi     AND &694D
BFC7  73                 s       ...
BFC8  73                 s       ...
BFC9  69 6E              in      ADC #&6E
BFCB  67 20              g       RMB6 &20
BFCD  23                 #       ...
BFCE  00                 .       ...
.
BFCF  68                 h       PLA
BFD0  85 37              .7      STA &37
BFD2  68                 h       PLA
BFD3  85 38              .8      STA &38
BFD5  A0 00              ..      LDY #&00
BFD7  F0 03              ..      BEQ &BFDC
.
BFD9  20 E3 FF            ..     JSR osasci
.
BFDC  20 4B 89            K.     JSR &894B                  ; Increase &0037-0038 (temporary text pointer) with 1 and load next character
BFDF  10 F8              ..      BPL &BFD9
BFE1  6C 37 00           l7.     JMP (&0037)

; -----------------------------------------------------------------------------
; REPORT
; -----------------------------------------------------------------------------
.report
BFE4  20 57 98            W.     JSR &9857                  ; Check end of statement
BFE7  20 25 BC            %.     JSR &BC25                  ; Print CR/LF and reset COUNT variable to 0
BFEA  A0 01              ..      LDY #&01
.
BFEC  B1 FD              ..      LDA (&FD),Y                ; Get byte
BFEE  F0 06              ..      BEQ &BFF6                  ; Exit if &00 (terminator byte)
BFF0  20 0E B5            ..     JSR &B50E                  ; Print character or token
BFF3  C8                 .       INY
BFF4  D0 F6              ..      BNE &BFEC
.
BFF6  4C 9B 8B           L..     JMP &8B9B                  ; Jump to main execution loop

BFF9  00                 .       EQUB &00
BFFA  52 6F 67 65 72     Roger   EQUS "Roger"
BFFF  00                 .       EQUB &00
