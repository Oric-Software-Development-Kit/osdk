/* macros v1.33 for the 6502 code generator, F.Frances */

/*
   Naming convention
   =================

   The compiler emits one macro call per intermediate-code operation, named

        <OPERATION><size>_<operand modes>

   Operation + size (the "16-bit pseudo instructions"):
        ADDW / SUBW     16 bit add / subtract
        MULI / MULU     16 bit multiply, signed / unsigned (jsr mul16i/mul16u)
        DIVI / DIVU     16 bit divide,   signed / unsigned
        MODI / MODU     16 bit modulo,   signed / unsigned
        LSHW            16 bit left shift (LSH1W: optimized shift-by-one)
        RSHW            16 bit right shift, LOGICAL    (unsigned >>)
        ASRW            16 bit right shift, ARITHMETIC (signed >>, keeps sign)
        ANDW/ORW/XORW   16 bit bitwise and / or / exclusive or
        COMW            16 bit complement (~), NEGI: negate (-)
        INDIRB / INDIRW load (dereference) a byte / word
        ASGNB / ASGNW   store a byte / word
        ASGNS           struct copy (extra 'length' argument)
        MOVW            word copy, CWB: word to byte truncation
        CSBW / CZBW     byte to word, sign extended / zero extended
        ADDF/SUBF/MULF/DIVF/NEGF/MOVF   float operations (5-byte, ROM based)
        CIF / CFI       int to float / float to int conversion
        EQW/NEW/EQ0W/NE0W and GTI/GEI/LTI/LEI (signed) GTU/GEU/LTU/LEU
                        (unsigned): compare and branch to a label
        ARGW/ARGF/ARGS  push a word / float / struct argument for a call
        CALLV/CALLW/CALLF  call returning void / word / float
        ENTER/LEAVE/RETW/LEAVEW  function prologue / epilogue / return

   Operand mode suffix, one letter per operand in source order
   (for ASGN*: value first, then destination; result always last):
        C    constant: an immediate value or a label whose address is the
             operand (for stores: the named location itself, e.g. a register
             variable reg0..reg7)
        D    direct: the 16 bit value stored at the named location
             (a zero page temporary tmp0..tmp7, or an absolute label)
        Z    a pointer stored in zero page, dereferenced (lda (tmpN),y);
             only used where it must differ from D (INDIR*, ASGN* targets)
        A    a frame location: the pair (pointer,offset), accessed with
             lda (ptr),y - used for function parameters (ap) and locals (fp)
        Y    like A but the operand is itself dereferenced through a
             pointer read from the frame, or a folded "(tmpN),0" indirection
        R    a register variable reg0..reg7 (zero page)

   Example: ADDW_DCY(tmp0,_giarr+4,(fp),6) reads the word in tmp0 (D),
   adds the constant _giarr+4 (C), and stores the result to the local
   variable at (fp)+6 (Y).
*/

#ifdef XA
#define STRING .asc
#define DB(n) .byt n 
#define DW(n) .word n 
#define SPACE(n) .dsb n 
#define ZERO(n) .dsb n
#endif

/* -O0 aliases: at optimization level 0 the compiler emits the raw
   intermediate opcode names (lcc's dag operators: ADDI = add integer,
   DIVU = divide unsigned, MODI = modulo signed, RSHI = right shift
   signed...) with every operand in a zero page temporary, so they all
   map to the all-direct (_DDD) variant of the corresponding macro. */
#define BANDU ANDW_DDD
#define BORU  ORW_DDD
#define BXORU XORW_DDD
#define ADDD  ADDF_YYY
#define ADDF  ADDF_YYY
#define ADDI  ADDW_DDD
#define ADDP  ADDW_DDD
#define ADDU  ADDW_DDD
#define SUBD  SUBF_YYY
#define SUBF  SUBF_YYY
#define SUBI  SUBW_DDD
#define SUBP  SUBW_DDD
#define SUBU  SUBW_DDD
#define MULD  MULF_YYY
#define MULF  MULF_YYY
#define MULI  MULI_DDD
#define MULU  MULU_DDD
#define DIVD  DIVF_YYY
#define DIVF  DIVF_YYY
#define DIVI  DIVI_DDD
#define DIVU  DIVU_DDD
#define MODI  MODI_DDD
#define MODU  MODU_DDD
#define RSHU  RSHW_DDD
#define RSHI  ASRW_DDD
#define LSHI  LSHW_DDD
#define LSHU  LSHW_DDD
#define INDIRC INDIRB_ZD
#define INDIRS INDIRB_ZD
#define INDIRI INDIRW_ZD
#define INDIRP INDIRW_ZD
#define INDIRD INDIRF_DY
#define INDIRF INDIRF_DY
#define INDIRB INDIRS_DD
#define BCOMU COMW_DD
#define NEGD  NEGF_YY
#define NEGF  NEGF_YY
#define NEGI  NEGI_DD
#define CVCI  CSBW_DD
#define CVSI  CSBW_DD
#define CVCU  CZBW_DD
#define CVSU  CZBW_DD
#define CVUC  CWB_DD
#define CVUS  CWB_DD
#define CVIC  CWB_DD
#define CVIS  CWB_DD
#define CVPU  MOVW_DD
#define CVUP  MOVW_DD
#define CVIU  MOVW_DD
#define CVUI  MOVW_DD
#define CVID  CIF_DY
#define CVDF  MOVF_YY
#define CVFD  MOVF_YY
#define CVDI  CFI_YD
#define RETD  LEAVEF_Y
#define RETF  LEAVEF_Y
#define RETI  LEAVEW_D
#define RETV  LEAVE
#define ADDRGP ADDR_CD
#define ADDRFP ADDR_AD
#define ADDRLP ADDR_AD
#define CNSTC CNST_CD
#define CNSTS CNST_CD
#define CNSTI CNST_CD
#define CNSTU CNST_CD
#define CNSTP CNST_CD
#define JUMPV JUMP_D
#define ASGNB ASGNS_DZ
#define ASGNC ASGNB_DZ
#define ASGNS ASGNB_DZ
#define ASGND ASGNF_YZ
#define ASGNF ASGNF_YZ
#define ASGNI ASGNW_DZ
#define ASGNP ASGNW_DZ
#define ARGB(tmp,ptr1,offset,size) ARGS_D(tmp,ptr1,offset,size)
#define ARGD(ptr1,y1,offset) ARGF_Y(ptr1,y1,(sp),offset)
#define ARGF(ptr1,y1,offset) ARGF_Y(ptr1,y1,(sp),offset)
#define ARGI(tmp,offset) ARGW_D(tmp,(sp),offset)
#define ARGP(tmp,offset) ARGW_D(tmp,(sp),offset)
#define CALLV CALLV_D
#define CALLD CALLF_DY
#define CALLF CALLF_DY
#define CALLI CALLW_DD
#define EQD EQF_YY
#define EQF EQF_YY
#define EQI EQW_DD
#define GED GEF_YY
#define GEF GEF_YY
#define GEI GEI_DD
#define GEU GEU_DD
#define GTD GTF_YY
#define GTF GTF_YY
#define GTI GTI_DD
#define GTU GTU_DD
#define LED LEF_YY
#define LEF LEF_YY
#define LEI LEI_DD
#define LEU LEU_DD
#define LTD LTF_YY
#define LTF LTF_YY
#define LTI LTI_DD
#define LTU LTU_DD
#define NED NEF_YY
#define NEF NEF_YY
#define NEI NEW_DD

#define ENTER(r,n)\
 ldx #n :\
 lda #r :\
 jsr enter :\

#define LEAVE\
 jmp leave :\


#define LEAVEW_C(cte1)\
 ldx #<(cte1) :\
 lda #>(cte1) :\
 jmp leave :\

#define LEAVEW_A(ptr1,y1)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 jmp leave :\

#define LEAVEW_D(tmp1)\
 ldx tmp1 :\
 lda tmp1+1 :\
 jmp leave :\

#define LEAVEW_Y(ptr1,y1)\
 ldy #y1 :\
 lda ptr1,y :\
 tax :\
 iny :\
 lda ptr1,y :\
 jmp leave :\

#define RET\
 rts :\

#define RETW_C(cte1)\
 ldx #<(cte1) :\
 lda #>(cte1) :\
 rts :\

#define RETW_A(ptr1,y1)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 rts :\

#define RETW_D(tmp1)\
 ldx tmp1 :\
 lda tmp1+1 :\
 rts :\

#define RETW_Y(ptr1,y1)\
 ldy #y1 :\
 lda ptr1,y :\
 tax :\
 iny :\
 lda ptr1,y :\
 rts :\

#define CNST_CD(cte1,tmp)\
 ldx #<(cte1) :\
 stx tmp :\
 lda #>(cte1) :\
 sta tmp+1 :\


#define ADDR_CD(adr1,tmp)\
 ldx #<(adr1) :\
 stx tmp :\
 lda #>(adr1) :\
 sta tmp+1 :\

#define ADDR_AD(ptr1,y1,tmp2)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 sta tmp2 :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp2+1 :\


#define CZBW_CD(cte1,tmp2)\
 lda #cte1 :\
 sta tmp2 :\
 lda #0 :\
 sta tmp2+1 :\

#define CZBW_CY(cte1,ptr2,y2)\
 ldy #y2 :\
 lda #cte1 :\
 sta ptr2,y :\
 iny :\
 lda #0 :\
 sta ptr2,y :\

#define CZBW_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta tmp2 :\
 lda #0 :\
 sta tmp2+1 :\

#define CZBW_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 sta ptr2,y :\
 iny :\
 lda #0 :\
 sta ptr2,y :\

#define CZBW_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2 :\
 lda #0 :\
 sta tmp2+1 :\

#define CZBW_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 iny :\
 lda #0 :\
 sta ptr2,y :\


#define CWB_CD(cte1,tmp2)\
 lda #< cte1 :\
 sta tmp2 :\

#define CWB_CY(cte1,ptr2,y2)\
 ldy #y2 :\
 lda #< cte1 :\
 sta ptr2,y :\

#define CWB_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta tmp2 :\

#define CWB_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 sta ptr2,y :\

#define CWB_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2 :\

#define CWB_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\


#define MOVW_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta tmp2 :\
 lda tmp1+1 :\
 sta tmp2+1 :\

#define MOVW_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 sta ptr2,y :\
 iny :\
 lda tmp1+1 :\
 sta ptr2,y :\

#define MOVW_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2 :\
 iny :\
 lda ptr1,y :\
 sta tmp2+1 :\

#define MOVW_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define MOVW_CD(cte1,tmp2)\
 lda #<(cte1) :\
 sta tmp2 :\
 lda #>(cte1) :\
 sta tmp2+1 :\

#define MOVW_CY(cte1,ptr2,y2)\
 ldy #y2 :\
 lda #<(cte1) :\
 sta ptr2,y :\
 iny :\
 lda #>(cte1) :\
 sta ptr2,y :\

#define MOVW_AD(ptr1,y1,tmp2)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 sta tmp2 :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp2+1 :\

#define MOVW_AY(ptr1,y1,ptr2,y2)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 ldy #y2 :\
 sta ptr2,y :\
 iny :\
 lda 1+ptr1 :\
 adc #0 :\
 sta ptr2,y :\

#define CSBW_CD(cte1,tmp2)\
 lda #0 :\
 ldx #cte1 :\
 stx tmp2 :\
 .( :\
 bpl skip :\
 lda #$FF :\
skip :\
 .)  :\
 sta tmp2+1 :\


#define CSBW_CY(cte1,ptr2,y2)\
 ldy #y2 :\
 ldx #0 :\
 lda #cte1 :\
 sta ptr2,y :\
 .( :\
 bpl skip :\
 ldx #$FF :\
skip :\
 .)  :\
 txa :\
 iny :\
 sta ptr2,y :\

#define CSBW_DD(tmp1,tmp2)\
 lda #0 :\
 ldx tmp1 :\
 stx tmp2 :\
 .( :\
 bpl skip :\
 lda #$FF :\
skip :\
 .)  :\
 sta tmp2+1 :\

#define CSBW_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 ldx #0 :\
 lda tmp1 :\
 sta ptr2,y :\
 .( :\
 bpl skip :\
 ldx #$FF :\
skip :\
 .)  :\
 txa :\
 iny :\
 sta ptr2,y :\

#define CSBW_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 ldx #0 :\
 lda ptr1,y :\
 sta tmp2 :\
 .( :\
 bpl skip :\
 ldx #$FF :\
skip :\
 .)  :\
 txa :\
 sta tmp2+1 :\

#define CSBW_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 ldx #0 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 .( :\
 bpl skip :\
 ldx #$FF :\
skip :\
 .)  :\
 txa :\
 iny :\
 sta ptr2,y :\

#define INDIRB_CD(var1,tmp2)\
 lda var1 :\
 sta tmp2 :\

#define INDIRB_ZD(ptr1,tmp2)\
 ldy #0 :\
 lda (ptr1),y :\
 sta tmp2 :\

#define INDIRB_DD(var1, tmp2)\
 lda var1 :\
 sta tmp :\
 lda var1+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta tmp2 :\

#define INDIRB_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2 :\

#define INDIRB_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y ;\
 sta tmp2 :\

#define INDIRB_CY(var1,ptr2,y2)\
 ldy #y2 :\
 lda var1 :\
 sta ptr2,y :\

#define INDIRB_ZY(ptr1,ptr2,y2)\
 ldy #0 :\
 lda (ptr1),y :\
 ldy #y2 :\
 sta ptr2,y :\

#define INDIRB_DY(var1,ptr2,y2)\
 lda var1 :\
 sta tmp :\
 lda var1+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 ldy #y2 :\
 sta ptr2,y :\

#define INDIRB_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\

#define INDIRB_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y ;\
 ldy #y2 :\
 sta ptr2,y :\

#define INDIRW_CD(var1,tmp2)\
 lda var1 :\
 sta tmp2 :\
 lda var1+1 :\
 sta tmp2+1 :\

#define INDIRW_ZD(ptr1,tmp2)\
 ldy #0 :\
 lda (ptr1),y :\
 tax :\
 iny :\
 lda (ptr1),y :\
 stx tmp2 :\
 sta tmp2+1 :\

#define INDIRW_DD(var1,var2)\
 lda var1 :\
 sta tmp :\
 lda var1+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta var2 :\
 iny :\
 lda (tmp),y :\
 sta var2+1 :\

#define INDIRW_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2 :\
 iny :\
 lda ptr1,y :\
 sta tmp2+1 :\

#define INDIRW_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y ;\
 sta tmp2 :\
 iny :\
 lda (tmp),y :\
 sta tmp2+1 :\

#define INDIRW_CY(var1,ptr2,y2)\
 ldy #y2 :\
 lda var1 :\
 sta ptr2,y :\
 iny :\
 lda var1+1 :\
 sta ptr2,y :\

#define INDIRW_ZY(ptr1,ptr2,y2)\
 ldy #0 :\
 lda (ptr1),y :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #1 :\
 lda (ptr1),y :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define INDIRW_DY(var1,ptr2,y2)\
 lda var1 :\
 sta tmp :\
 lda var1+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #1 :\
 lda (tmp),y :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define INDIRW_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define INDIRW_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y ;\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #1 :\
 lda (tmp),y :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define INDIRS_CD(var1,tmp2)\
 lda #<(var1) :\
 sta tmp2 :\
 lda #>(var1) :\
 sta tmp2+1 :\

#define INDIRS_DD(var1,tmp2)\
 lda var1 :\
 sta tmp2 :\
 lda var1+1 :\
 sta tmp2+1 :\

#define NEGI_DD(tmp1,tmp2)\
 lda #0 :\
 sec :\
 sbc tmp1 :\
 sta tmp2 :\
 lda #0 :\
 sbc tmp1+1 :\
 sta tmp2+1 :\

#define NEGI_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda #0 :\
 sec :\
 sbc tmp1 :\
 sta ptr2,y :\
 iny :\
 lda #0 :\
 sbc tmp1+1 :\
 sta ptr2,y :\

#define NEGI_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda #0 :\
 sec :\
 sbc ptr1,y :\
 sta tmp2 :\
 iny :\
 lda #0 :\
 sbc ptr1,y :\
 sta tmp2+1 :\

#define NEGI_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda #0 :\
 sec :\
 sbc ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #y1+1 :\
 lda #0 :\
 sbc ptr1,y :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define INCW_D(var1)\
 inc var1 :\
 .( :\
 bne skip :\
 inc var1+1 :\
skip :\
 .)  :\

#define INCW_Z INCW_D

#define DECW_D(var1)\
 lda var1 :\
 .( :\
 bne skip :\
 dec var1+1 :\
skip :\
 .)  :\
 dec var1 :\

#define DECW_Z DECW_D

;
; 8-bit (byte) arithmetic/logic - Phase 1 of native char operations.
; These are the low-byte halves of the matching *W families: when the
; compiler proves a char op's result is only used as a char, it emits the
; B family and skips the 16-bit widen/narrow. Operands are always D (zp
; temp) or C (const char); the result is a D temp. See gen.c narrowing.
;
#define ADDB_DDD(tmp1,tmp2,tmp3)\
 clc :\
 lda tmp1 :\
 adc tmp2 :\
 sta tmp3 :\

#define ADDB_DCD(tmp2,cte1,tmp3)\
 clc :\
 lda #<(cte1) :\
 adc tmp2 :\
 sta tmp3 :\

#define SUBB_DDD(tmp1,tmp2,tmp3)\
 sec :\
 lda tmp1 :\
 sbc tmp2 :\
 sta tmp3 :\

#define SUBB_DCD(tmp1,cte2,tmp3)\
 sec :\
 lda tmp1 :\
 sbc #<(cte2) :\
 sta tmp3 :\

#define ANDB_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 and tmp2 :\
 sta tmp3 :\

#define ANDB_DCD(tmp2,cte1,tmp3)\
 lda #<(cte1) :\
 and tmp2 :\
 sta tmp3 :\

#define XORB_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 eor tmp2 :\
 sta tmp3 :\

#define XORB_DCD(tmp2,cte1,tmp3)\
 lda #<(cte1) :\
 eor tmp2 :\
 sta tmp3 :\

#define ORB_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 ora tmp2 :\
 sta tmp3 :\

#define ORB_DCD(tmp2,cte1,tmp3)\
 lda #<(cte1) :\
 ora tmp2 :\
 sta tmp3 :\

; 8-bit (byte) unary + shift - Phase 2 of native char operations.
; Low-byte halves of COMW / NEGI / LSH1W. Emitted when a char ~x, -x or
; x<<1 has a char-only result (see gen.c). Same operand/result modes as
; the word forms so -O3 folded operands are covered.
#define COMB_DD(tmp1,tmp2)\
 lda tmp1 :\
 eor #$FF :\
 sta tmp2 :\

#define COMB_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 eor #$FF :\
 sta ptr2,y :\

#define COMB_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 eor #$FF :\
 sta tmp2 :\

#define COMB_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 eor #$FF :\
 ldy #y2 :\
 sta ptr2,y :\

#define NEGB_DD(tmp1,tmp2)\
 lda #0 :\
 sec :\
 sbc tmp1 :\
 sta tmp2 :\

#define NEGB_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda #0 :\
 sec :\
 sbc tmp1 :\
 sta ptr2,y :\

#define NEGB_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda #0 :\
 sec :\
 sbc ptr1,y :\
 sta tmp2 :\

#define NEGB_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda #0 :\
 sec :\
 sbc ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\

#define LSH1B_DD(tmp1,tmp2)\
 lda tmp1 :\
 asl :\
 sta tmp2 :\

#define LSH1B_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 asl :\
 sta ptr2,y :\

#define LSH1B_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 asl :\
 sta tmp2 :\

#define LSH1B_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 asl :\
 ldy #y2 :\
 sta ptr2,y :\

; x >> 1 on an unsigned char: a LOGICAL byte shift (lsr, bit 7 <- 0). Only
; CVCU (unsigned) operands are narrowed to this, so the value is 0..255 and
; the low byte's high bit is data, not sign - lsr (not an arithmetic shift)
; is the correct byte form even though the promoted C op is signed.
#define RSH1B_DD(tmp1,tmp2)\
 lda tmp1 :\
 lsr :\
 sta tmp2 :\

#define RSH1B_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 lsr :\
 sta ptr2,y :\

#define RSH1B_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 lsr :\
 sta tmp2 :\

#define RSH1B_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 lsr :\
 ldy #y2 :\
 sta ptr2,y :\

#define ADDW_DDD(tmp1,tmp2,tmp3)\
 clc :\
 lda tmp1 :\
 adc tmp2 :\
 sta tmp3 :\
 lda tmp1+1 :\
 adc tmp2+1 :\
 sta tmp3+1 :\

#define ADDW_YDD(ptr1,y1,tmp2,tmp3)\
 clc :\
 ldy #y1 :\
 lda ptr1,y :\
 adc tmp2 :\
 sta tmp3 :\
 iny :\
 lda ptr1,y :\
 adc tmp2+1 :\
 sta tmp3+1 :\

#define ADDW_DYD(tmp1,ptr2,y2,tmp3)\
 clc :\
 lda tmp1 :\
 ldy #y2 :\
 adc ptr2,y :\
 sta tmp3 :\
 iny :\
 lda tmp1+1 :\
 adc ptr2,y :\
 sta tmp3+1 :\

#define ADDW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 clc :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 adc ptr2,y :\
 sta tmp3 :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 adc ptr2,y :\
 sta tmp3+1 :\

#define ADDW_DDY(tmp1,tmp2,ptr3,y3)\
 clc :\
 lda tmp1 :\
 adc tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda tmp1+1 :\
 adc tmp2+1 :\
 sta ptr3,y :\

#define ADDW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 clc :\
 ldy #y1 :\
 lda ptr1,y :\
 adc tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 adc tmp2+1 :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ADDW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 clc :\
 lda tmp1 :\
 ldy #y2 :\
 adc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda tmp1+1 :\
 ldy #y2+1 :\
 adc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ADDW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 clc :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 adc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 adc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ADDW_DCD(tmp2,cte1,tmp3)\
 clc :\
 lda #<(cte1) :\
 adc tmp2 :\
 sta tmp3 :\
 lda #>(cte1) :\
 adc tmp2+1 :\
 sta tmp3+1 :\

#define ADDW_DAD(tmp2,ptr1,y1,tmp3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 txa :\
 adc tmp2 :\
 sta tmp3 :\
 lda tmp+1 :\
 adc tmp2+1 :\
 sta tmp3+1 :\

#define ADDW_YCD(ptr2,y2,cte1,tmp3)\
 clc :\
 lda #<(cte1) :\
 ldy #y2 :\
 adc ptr2,y :\
 sta tmp3 :\
 iny :\
 lda #>(cte1) :\
 adc ptr2,y :\
 sta tmp3+1 :\

#define ADDW_YAD(ptr2,y2,ptr1,y1,tmp3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 txa :\
 ldy #y2 :\
 adc ptr2,y :\
 sta tmp3 :\
 iny :\
 lda tmp+1 :\
 adc ptr2,y :\
 sta tmp3+1 :\

#define ADDW_DCY(tmp2,cte1,ptr3,y3)\
 clc :\
 lda #<(cte1) :\
 adc tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda #>(cte1) :\
 adc tmp2+1 :\
 sta ptr3,y :\

#define ADDW_DAY(tmp2,ptr1,y1,ptr3,y3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 txa :\
 adc tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda tmp+1 :\
 adc tmp2+1 :\
 sta ptr3,y :\

#define ADDW_YCY(ptr2,y2,cte1,ptr3,y3)\
 clc :\
 lda #<(cte1) :\
 ldy #y2 :\
 adc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda #>(cte1) :\
 ldy #y2+1 :\
 adc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ADDW_YAY(ptr2,y2,ptr1,y1,ptr3,y3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 txa :\
 ldy #y2 :\
 adc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda tmp+1 :\
 ldy #y2+1 :\
 adc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define SUBW_DCD(tmp1,cte2,tmp3)\
 sec :\
 lda tmp1 :\
 sbc #<(cte2) :\
 sta tmp3 :\
 lda tmp1+1 :\
 sbc #>(cte2) :\
 sta tmp3+1 :\

#define SUBW_CDD(cte1,tmp2,tmp3)\
 sec :\
 lda #<(cte1) :\
 sbc tmp2 :\
 sta tmp3 :\
 lda #>(cte1) :\
 sbc tmp2+1 :\
 sta tmp3+1 :\

#define SUBW_ADD(ptr1,y1,tmp2,tmp3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 sec :\
 txa :\
 sbc tmp2 :\
 sta tmp3 :\
 lda tmp+1 :\
 sbc tmp2+1 :\
 sta tmp3+1 :\

#define SUBW_YCD(ptr1,y1,cte2,tmp3)\
 sec :\
 ldy #y1 :\
 lda ptr1,y :\
 sbc #<(cte2) :\
 sta tmp3 :\
 iny :\
 lda ptr1,y :\
 sbc #>(cte2) :\
 sta tmp3+1 :\

#define SUBW_CYD(cte1,ptr2,y2,tmp3)\
 sec :\
 ldy #y2 :\
 lda #<(cte1) :\
 sbc ptr2,y :\
 sta tmp3 :\
 iny :\
 lda #>(cte1) :\
 sbc ptr2,y :\
 sta tmp3+1 :\

#define SUBW_AYD(ptr1,y1,ptr2,y2,tmp3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 sec :\
 ldy #y2 :\
 txa :\
 sbc ptr2,y :\
 sta tmp3 :\
 iny :\
 lda tmp+1 :\
 sbc ptr2,y :\
 sta tmp3+1 :\

#define SUBW_DCY(tmp1,cte2,ptr3,y3)\
 sec :\
 ldy #y3 :\
 lda tmp1 :\
 sbc #<(cte2) :\
 sta ptr3,y :\
 iny :\
 lda tmp1+1 :\
 sbc #>(cte2) :\
 sta ptr3,y :\

#define SUBW_CDY(cte1,tmp2,ptr3,y3)\
 sec :\
 ldy #y3 :\
 lda #<(cte1) :\
 sbc tmp2 :\
 sta ptr3,y :\
 iny :\
 lda #>(cte1) :\
 sbc tmp2+1 :\
 sta ptr3,y :\

#define SUBW_ADY(ptr1,y1,tmp2,ptr3,y3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 sec :\
 ldy #y3 :\
 txa :\
 sbc tmp2 :\
 sta ptr3,y :\
 iny :\
 lda tmp+1 :\
 sbc tmp2+1 :\
 sta ptr3,y :\

#define SUBW_YCY(ptr1,y1,cte2,ptr3,y3)\
 sec :\
 ldy #y1 :\
 lda ptr1,y :\
 sbc #<(cte2) :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 sbc #>(cte2) :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define SUBW_CYY(cte1,ptr2,y2,ptr3,y3)\
 sec :\
 lda #<(cte1) :\
 ldy #y2 :\
 sbc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda #>(cte1) :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define SUBW_AYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 sta tmp+1 :\
 sec :\
 txa :\
 ldy #y2 :\
 sbc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda tmp+1 :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define SUBW_DDD(tmp1,tmp2,tmp3)\
 sec :\
 lda tmp1 :\
 sbc tmp2 :\
 sta tmp3 :\
 lda tmp1+1 :\
 sbc tmp2+1 :\
 sta tmp3+1 :\

#define SUBW_YDD(ptr1,y1,tmp2,tmp3)\
 sec :\
 ldy #y1 :\
 lda ptr1,y :\
 sbc tmp2 :\
 sta tmp3 :\
 iny :\
 lda ptr1,y :\
 sbc tmp2+1 :\
 sta tmp3+1 :\

#define SUBW_DYD(tmp1,ptr2,y2,tmp3)\
 sec :\
 lda tmp1 :\
 ldy #y2 :\
 sbc ptr2,y :\
 sta tmp3 :\
 iny :\
 lda tmp1+1 :\
 sbc ptr2,y :\
 sta tmp3+1 :\

#define SUBW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 sec :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sbc ptr2,y :\
 sta tmp3 :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 sta tmp3+1 :\

#define SUBW_DDY(tmp1,tmp2,ptr3,y3)\
 sec :\
 lda tmp1 :\
 sbc tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda tmp1+1 :\
 sbc tmp2+1 :\
 sta ptr3,y :\

#define SUBW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 sec :\
 ldy #y1 :\
 lda ptr1,y :\
 sbc tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 sbc tmp2+1 :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define SUBW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 sec :\
 lda tmp1 :\
 ldy #y2 :\
 sbc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda tmp1+1 :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define SUBW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 sec :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sbc ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define COMW_DD(tmp1,tmp2)\
 lda tmp1 :\
 eor #$FF :\
 sta tmp2 :\
 lda tmp1+1 :\
 eor #$FF :\
 sta tmp2+1 :\

#define COMW_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 eor #$FF :\
 sta ptr2,y :\
 iny :\
 lda tmp1+1 :\
 eor #$FF :\
 sta ptr2,y :\

#define COMW_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 eor #$FF :\
 sta tmp2 :\
 iny :\
 lda ptr1,y :\
 eor #$FF :\
 sta tmp2+1 :\

#define COMW_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 eor #$FF :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 eor #$FF :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define ANDW_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 and tmp2 :\
 sta tmp3 :\
 lda tmp1+1 :\
 and tmp2+1 :\
 sta tmp3+1 :\

#define ANDW_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 and tmp2 :\
 sta tmp3 :\
 iny :\
 lda ptr1,y :\
 and tmp2+1 :\
 sta tmp3+1 :\

#define ANDW_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 ldy #y2 :\
 and ptr2,y :\
 sta tmp3 :\
 iny :\
 lda tmp1+1 :\
 and ptr2,y :\
 sta tmp3+1 :\

#define ANDW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 and ptr2,y :\
 sta tmp3 :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 and ptr2,y :\
 sta tmp3+1 :\

#define ANDW_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 and tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda tmp1+1 :\
 and tmp2+1 :\
 sta ptr3,y :\

#define ANDW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 and tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 and tmp2+1 :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ANDW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 ldy #y2 :\
 and ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda tmp1+1 :\
 ldy #y2+1 :\
 and ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ANDW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 and ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 and ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ANDW_DCD(tmp2,cte1,tmp3)\
 lda #<(cte1) :\
 and tmp2 :\
 sta tmp3 :\
 lda #>(cte1) :\
 and tmp2+1 :\
 sta tmp3+1 :\

#define ANDW_YCD(ptr2,y2,cte1,tmp3)\
 lda #<(cte1) :\
 ldy #y2 :\
 and ptr2,y :\
 sta tmp3 :\
 iny :\
 lda #>(cte1) :\
 and ptr2,y :\
 sta tmp3+1 :\

#define ANDW_DCY(tmp2,cte1,ptr3,y3)\
 lda #<(cte1) :\
 and tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda #>(cte1) :\
 and tmp2+1 :\
 sta ptr3,y :\

#define ANDW_YCY(ptr2,y2,cte1,ptr3,y3)\
 lda #<(cte1) :\
 ldy #y2 :\
 and ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda #>(cte1) :\
 ldy #y2+1 :\
 and ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define XORW_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 eor tmp2 :\
 sta tmp3 :\
 lda tmp1+1 :\
 eor tmp2+1 :\
 sta tmp3+1 :\

#define XORW_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 eor tmp2 :\
 sta tmp3 :\
 iny :\
 lda ptr1,y :\
 eor tmp2+1 :\
 sta tmp3+1 :\

#define XORW_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 ldy #y2 :\
 eor ptr2,y :\
 sta tmp3 :\
 iny :\
 lda tmp1+1 :\
 eor ptr2,y :\
 sta tmp3+1 :\

#define XORW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 eor ptr2,y :\
 sta tmp3 :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 eor ptr2,y :\
 sta tmp3+1 :\

#define XORW_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 eor tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda tmp1+1 :\
 eor tmp2+1 :\
 sta ptr3,y :\

#define XORW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 eor tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 eor tmp2+1 :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define XORW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 ldy #y2 :\
 eor ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda tmp1+1 :\
 ldy #y2+1 :\
 eor ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define XORW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 eor ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 eor ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define XORW_DCD(tmp2,cte1,tmp3)\
 lda #<(cte1) :\
 eor tmp2 :\
 sta tmp3 :\
 lda #>(cte1) :\
 eor tmp2+1 :\
 sta tmp3+1 :\

#define XORW_YCD(ptr2,y2,cte1,tmp3)\
 lda #<(cte1) :\
 ldy #y2 :\
 eor ptr2,y :\
 sta tmp3 :\
 iny :\
 lda #>(cte1) :\
 eor ptr2,y :\
 sta tmp3+1 :\

#define XORW_DCY(tmp2,cte1,ptr3,y3)\
 lda #<(cte1) :\
 eor tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda #>(cte1) :\
 eor tmp2+1 :\
 sta ptr3,y :\

#define XORW_YCY(ptr2,y2,cte1,ptr3,y3)\
 lda #<(cte1) :\
 ldy #y2 :\
 eor ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda #>(cte1) :\
 ldy #y2+1 :\
 eor ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ORW_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 ora tmp2 :\
 sta tmp3 :\
 lda tmp1+1 :\
 ora tmp2+1 :\
 sta tmp3+1 :\

#define ORW_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 ora tmp2 :\
 sta tmp3 :\
 iny :\
 lda ptr1,y :\
 ora tmp2+1 :\
 sta tmp3+1 :\

#define ORW_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 ldy #y2 :\
 ora ptr2,y :\
 sta tmp3 :\
 iny :\
 lda tmp1+1 :\
 ora ptr2,y :\
 sta tmp3+1 :\

#define ORW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 ora ptr2,y :\
 sta tmp3 :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 ora ptr2,y :\
 sta tmp3+1 :\

#define ORW_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 ora tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda tmp1+1 :\
 ora tmp2+1 :\
 sta ptr3,y :\

#define ORW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 ora tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ora tmp2+1 :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ORW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 ldy #y2 :\
 ora ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda tmp1+1 :\
 ldy #y2+1 :\
 ora ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ORW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 ora ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 ora ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define ORW_DCD(tmp2,cte1,tmp3)\
 lda #<(cte1) :\
 ora tmp2 :\
 sta tmp3 :\
 lda #>(cte1) :\
 ora tmp2+1 :\
 sta tmp3+1 :\

#define ORW_YCD(ptr2,y2,cte1,tmp3)\
 lda #<(cte1) :\
 ldy #y2 :\
 ora ptr2,y :\
 sta tmp3 :\
 iny :\
 lda #>(cte1) :\
 ora ptr2,y :\
 sta tmp3+1 :\

#define ORW_DCY(tmp2,cte1,ptr3,y3)\
 lda #<(cte1) :\
 ora tmp2 :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda #>(cte1) :\
 ora tmp2+1 :\
 sta ptr3,y :\

#define ORW_YCY(ptr2,y2,cte1,ptr3,y3)\
 lda #<(cte1) :\
 ldy #y2 :\
 ora ptr2,y :\
 ldy #y3 :\
 sta ptr3,y :\
 lda #>(cte1) :\
 ldy #y2+1 :\
 ora ptr2,y :\
 ldy #y3+1 :\
 sta ptr3,y :\

#define DIVI_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVI_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVI_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVI_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVI_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVI_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVI_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVI_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVI_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define DIVU_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define DIVU_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr div16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODU_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODU_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MODI_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MODI_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mod16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16u :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULU_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULU_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16u :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda tmp2 :\
 sta op2 :\
 lda tmp2+1 :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta op2 :\
 iny :\
 lda ptr2,y :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16i :\
 stx tmp3 :\
 sta tmp3+1 :\

#define MULI_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define MULI_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #<(cte2) :\
 sta op2 :\
 lda #>(cte2) :\
 sta op2+1 :\
 jsr mul16i :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSH1W_DD(tmp1,tmp2)\
 lda tmp1 :\
 asl :\
 sta tmp2 :\
 lda tmp1+1 :\
 rol :\
 sta tmp2+1 :\

#define LSH1W_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 asl :\
 sta tmp2 :\
 iny :\
 lda ptr1,y :\
 rol :\
 sta tmp2+1 :\

#define LSH1W_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 asl :\
 sta ptr2,y :\
 iny :\
 lda tmp1+1 :\
 rol :\
 sta ptr2,y :\

#define LSH1W_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 asl :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 rol :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define LSHW\
 .( :\
 beq done :\
loop :\
 asl tmp :\
 rol :\
 dex :\
 bne loop :\
done :\
 .) :\
 ldx tmp :\

#define LSHW_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx tmp2 :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx #cte2 :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_DYD(tmp1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ldx tmp2 :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_CYD(cte1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx tmp2 :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx #cte2 :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 LSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define LSHW_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx tmp2 :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSHW_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx #cte2 :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSHW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSHW_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ldx tmp2 :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSHW_CYY(cte1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSHW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx tmp2 :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSHW_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx #cte2 :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define LSHW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 LSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW\
 .( :\
 beq done :\
loop :\
 lsr :\
 ror tmp :\
 dex :\
 bne loop :\
done :\
 .) :\
 ldx tmp :\

#define RSHW_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx tmp2 :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx #cte2 :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_DYD(tmp1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ldx tmp2 :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_CYD(cte1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx tmp2 :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx #cte2 :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 RSHW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define RSHW_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx tmp2 :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx #cte2 :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ldx tmp2 :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW_CYY(cte1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx tmp2 :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx #cte2 :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define RSHW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 RSHW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\


; Arithmetic (sign preserving) 16 bit right shift, for signed >> :
; identical to the RSHW family except the core injects the sign bit.
#define ASRW\
 .( :\
 beq done :\
loop :\
 cmp #$80 :\
 ror :\
 ror tmp :\
 dex :\
 bne loop :\
done :\
 .) :\
 ldx tmp :\

#define ASRW_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx tmp2 :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx #cte2 :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_DYD(tmp1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ldx tmp2 :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_CYD(cte1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx tmp2 :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx #cte2 :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ASRW :\
 stx tmp3 :\
 sta tmp3+1 :\

#define ASRW_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx tmp2 :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASRW_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ldx #cte2 :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASRW_DYY(tmp1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASRW_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ldx tmp2 :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASRW_CYY(cte1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASRW_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx tmp2 :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASRW_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ldx #cte2 :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASRW_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y2 :\
 lda ptr2,y :\
 tax :\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 ASRW :\
 ldy #y3+1 :\
 sta ptr3,y :\
 dey :\
 txa :\
 sta ptr3,y :\

#define ASGNB_CC(cte1,var2)\
 lda #<(cte1) :\
 sta var2 :\

#define ASGNB_CZ(cte1,ptr2)\
 ldy #0 :\
 lda #<(cte1) :\
 sta (ptr2),y :\

#define ASGNB_CD(cte1,var2)\
 lda var2 :\
 sta tmp :\
 lda var2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda #<(cte1) :\
 sta (tmp),y :\

#define ASGNB_CA(cte1,ptr2,y2)\
 ldy #y2 :\
 lda #<(cte1) :\
 sta ptr2,y :\

#define ASGNB_CY(cte1,ptr2,y2)\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda #<(cte1) :\
 sta (tmp),y :\

#define ASGNB_DC(tmp1,var2)\
 lda tmp1 :\
 sta var2 :\

#define ASGNB_DZ(tmp1,ptr2)\
 ldy #0 :\
 lda tmp1 :\
 sta (ptr2),y :\

#define ASGNB_DD(tmp1,var2)\
 lda var2 :\
 sta tmp :\
 lda var2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda tmp1 :\
 sta (tmp),y :\

#define ASGNB_DA(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 sta ptr2,y :\

#define ASGNB_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda tmp1 :\
 sta (tmp),y :\

#define ASGNB_YC(ptr1,y1,var2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta var2 :\

#define ASGNB_YZ(ptr1,y1,ptr2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #0 :\
 sta (ptr2),y :\

#define ASGNB_YD(ptr1,y1,var2)\
 lda var2 :\
 sta tmp :\
 lda var2+1 :\
 sta tmp+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #0 :\
 sta (tmp),y :\

#define ASGNB_YA(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\

#define ASGNB_YY(ptr1,y1,ptr2,y2)\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #0 :\
 sta (tmp),y :\

#define ASGNW_CC(cte1,var2)\
 lda #<(cte1) :\
 sta var2 :\
 lda #>(cte1) :\
 sta var2+1 :\

#define ASGNW_CZ(cte1,ptr2)\
 ldy #0 :\
 lda #<(cte1) :\
 sta (ptr2),y :\
 iny :\
 lda #>(cte1) :\
 sta (ptr2),y :\

#define ASGNW_CD(cte1,var2)\
 lda var2 :\
 sta tmp :\
 lda var2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda #<(cte1) :\
 sta (tmp),y :\
 iny :\
 lda #>(cte1) :\
 sta (tmp),y :\

#define ASGNW_CA(cte1,ptr2,y2)\
 ldy #y2 :\
 lda #<(cte1) :\
 sta ptr2,y :\
 iny :\
 lda #>(cte1) :\
 sta ptr2,y :\

#define ASGNW_CY(cte1,ptr2,y2)\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda #<(cte1) :\
 sta (tmp),y :\
 iny :\
 lda #>(cte1) :\
 sta (tmp),y :\

#define ASGNW_DC(tmp1,var2)\
 lda tmp1 :\
 sta var2 :\
 lda tmp1+1 :\
 sta var2+1 :\

#define ASGNW_DD(tmp1,var2)\
 lda var2 :\
 sta tmp :\
 lda var2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda tmp1 :\
 sta (tmp),y :\
 iny :\
 lda tmp1+1 :\
 sta (tmp),y :\

#define ASGNW_DZ(tmp1,ptr2)\
 ldy #0 :\
 lda tmp1 :\
 sta (ptr2),y :\
 iny :\
 lda tmp1+1 :\
 sta (ptr2),y :\

#define ASGNW_DA(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda tmp1 :\
 sta ptr2,y :\
 iny :\
 lda tmp1+1 :\
 sta ptr2,y :\

#define ASGNW_DY(tmp1,ptr2,y2)\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda tmp1 :\
 sta (tmp),y :\
 iny :\
 lda tmp1+1 :\
 sta (tmp),y :\

#define ASGNW_AC(ptr1,y1,var2)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 sta var2 :\
 lda 1+ptr1 :\
 adc #0 :\
 sta var2+1 :\

#define ASGNW_AZ(ptr1,y1,ptr2)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 ldy #0 :\
 sta (ptr2),y :\
 iny :\
 lda 1+ptr1 :\
 adc #0 :\
 sta (ptr2),y :\

#define ASGNW_AD(ptr1,y1,var2)\
 lda var2 :\
 sta tmp :\
 lda var2+1 :\
 sta tmp+1 :\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 ldy #0 :\
 sta (tmp),y :\
 iny :\
 lda 1+ptr1 :\
 adc #0 :\
 sta (tmp),y :\

#define ASGNW_AA(ptr1,y1,ptr2,y2)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 ldy #y2 :\
 sta ptr2,y :\
 iny :\
 lda 1+ptr1 :\
 adc #0 :\
 sta ptr2,y :\

#define ASGNW_AY(ptr1,y1,ptr2,y2)\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #0 :\
 sta (tmp),y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #1 :\
 sta (tmp),y :\

#define ASGNW_YC(ptr1,y1,var2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta var2 :\
 iny :\
 lda ptr1,y :\
 sta var2+1 :\

#define ASGNW_YZ(ptr1,y1,ptr2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #0 :\
 sta (ptr2),y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #1 :\
 sta (ptr2),y :\

#define ASGNW_YD(ptr1,y1,var2)\
 lda var2 :\
 sta tmp :\
 lda var2+1 :\
 sta tmp+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #0 :\
 sta (tmp),y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #1 :\
 sta (tmp),y :\

#define ASGNW_YA(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define ASGNW_YY(ptr1,y1,ptr2,y2)\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #0 :\
 sta (tmp),y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #1 :\
 sta (tmp),y :\

#define ASGNS_CR(adr1,reg2,length) \
 .( :\
 ldx #0 :\
loop :\
 lda adr1,x :\
 sta reg2,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_CC(adr1,adr2,length) \
 .( :\
 ldx #0 :\
loop :\
 lda adr1,x :\
 sta adr2,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_DR(adr1,reg2,length) \
 .( :\
 ldx adr1 :\
 lda adr1+1 :\
 stx src+1 :\
 sta src+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
 sta reg2,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_DC(adr1,adr2,length) \
 .( :\
 ldx adr1 :\
 lda adr1+1 :\
 stx src+1 :\
 sta src+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
 sta adr2,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_AR(ptr1,y1,reg2,length) \
 .( :\
 ldx #0 :\
 ldy #y1 :\
loop :\
 lda ptr1,y :\
 sta reg2,x :\
 iny :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_AC(ptr1,y1,adr2,length) \
 .( :\
 ldx #0 :\
 ldy #y1 :\
loop :\
 lda ptr1,y :\
 sta adr2,x :\
 iny :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_YR(ptr1,y1,reg2,length) \
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta src+1 :\
 iny :\
 lda ptr1,y :\
 sta src+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
 sta reg2,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_YC(ptr1,y1,adr2,length) \
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta src+1 :\
 iny :\
 lda ptr1,y :\
 sta src+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
 sta adr2,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_CA(adr1,ptr2,y2,length)\
 .( :\
 ldx #0 :\
 ldy #y2 :\
loop :\
 lda adr1,x :\
 sta ptr2,y :\
 inx :\
 iny :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_DA(adr1,ptr2,y2,length) \
 .( :\
 ldx adr1 :\
 lda adr1+1 :\
 stx src+1 :\
 sta src+2 :\
 ldx #0 :\
 ldy #y2 :\
loop :\
src :\
 lda $1234,x :\
 sta ptr2,y :\
 inx :\
 iny :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_AA(ptr1,y1,ptr2,y2,length)\
 .( :\
 ldy #y1 :\
 sty tmp :\
 ldy #y2 :\
 sty tmp+1 :\
 ldx #length :\
loop :\
 ldy tmp :\
 lda ptr1,y :\
 iny :\
 sty tmp :\
 ldy tmp+1 :\
 sta ptr2,y :\
 iny :\
 sty tmp+1 :\
 dex :\
 bne loop :\
 .) :\

#define ASGNS_AY(ptr1,y1,ptr2,y2,length)\
 .( :\
 ldy #y2 :\
 lda ptr2,y :\
 sta dst+1 :\
 iny :\
 lda ptr2,y :\
 sta dst+2 :\
 ldy #y1 :\
 ldx #0 :\
loop :\
 lda ptr1,y :\
dst :\
 sta $5678,x :\
 inx :\
 iny :\
 cpx #length :\
 bne loop :\
 .) :\


#define ASGNS_YA(ptr1,y1,ptr2,y2,length)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta src+1 :\
 iny :\
 lda ptr1,y :\
 sta src+2 :\
 ldx #0 :\
 ldy #y2 :\
loop :\
src :\
 lda $1234,x :\
 sta ptr2,y :\
 inx :\
 iny :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_CZ(adr1,tmp1,length) \
 .( :\
 ldx tmp1 :\
 lda tmp1+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
loop :\
 lda adr1,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_DZ(adr1,tmp1,length) \
 .( :\
 ldx adr1 :\
 lda adr1+1 :\
 stx src+1 :\
 sta src+2 :\
 ldx tmp1 :\
 lda tmp1+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_AZ(ptr1,y1,tmp1,length) \
 .( :\
 ldy #y1 :\
 ldx tmp1 :\
 lda tmp1+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
loop :\
 lda ptr1,y :\
dst :\
 sta $5678,x :\
 iny :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_CD(adr1,adr2,length) \
 .( :\
 ldx adr2 :\
 lda adr2+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
loop :\
 lda adr1,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_DD(adr1,adr2,length) \
 .( :\
 ldx adr1 :\
 lda adr1+1 :\
 stx src+1 :\
 sta src+2 :\
 ldx adr2 :\
 lda adr2+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_AD(ptr1,y1,adr2,length) \
 .( :\
 ldx adr2 :\
 lda adr2+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
 ldy #y1 :\
loop :\
 lda ptr1,y :\
dst :\
 sta $5678,x :\
 iny :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_YD(ptr1,y1,adr2,length) \
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta src+1 :\
 iny :\
 lda ptr1,y :\
 sta src+2 :\
 ldx adr2 :\
 lda adr2+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_CY(adr1,ptr2,y2,length) \
 .( :\
 ldy #y2 :\
 lda ptr2,y :\
 sta dst+1 :\
 iny :\
 lda ptr2,y :\
 sta dst+2 :\
 ldx #0 :\
loop :\
 lda adr1,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_DY(adr1,ptr2,y2,length) \
 .( :\
 ldx adr1 :\
 lda adr1+1 :\
 stx src+1 :\
 sta src+2 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta dst+1 :\
 iny :\
 lda ptr2,y :\
 sta dst+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_YY(ptr1,y1,ptr2,y2,length) \
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta src+1 :\
 iny :\
 lda ptr1,y :\
 sta src+2 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta dst+1 :\
 iny :\
 lda ptr2,y :\
 sta dst+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\

#define ASGNS_YZ(ptr1,y1,tmp2,length) \
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta src+1 :\
 iny :\
 lda ptr1,y :\
 sta src+2 :\
 ldx tmp2 :\
 lda tmp2+1 :\
 stx dst+1 :\
 sta dst+2 :\
 ldx #0 :\
loop :\
src :\
 lda $1234,x :\
dst :\
 sta $5678,x :\
 inx :\
 cpx #length :\
 bne loop :\
 .) :\


#define EQ0W_D(tmp,label)\
 .( :\
 lda tmp :\
 ora tmp+1 :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQ0W_Y(ptr,y2,label)\
 .( :\
 ldy #y2 :\
 lda ptr,y :\
 iny :\
 ora ptr,y :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQ0W_C(tmp,label)\
 .( :\
 lda #<(tmp) :\
 ora #>(tmp) :\
 bne skip :\
 jmp label :\
skip :\
 .) :\
 
#define EQW_DC(tmp2,cte1,label)\
 .( :\
 lda #<(cte1) :\
 eor tmp2 :\
 sta tmp :\
 lda #>(cte1) :\
 eor tmp2+1 :\
 ora tmp :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQW_DA(tmp2,ptr1,y1,label)\
 .( :\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 eor tmp2 :\
 sta tmp :\
 lda 1+ptr1 :\
 adc #0 :\
 eor tmp2+1 :\
 ora tmp :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQW_YC(ptr2,y2,cte1,label)\
 .( :\
 lda #<(cte1) :\
 ldy #y2 :\
 cmp ptr2,y :\
 bne skip :\
 iny :\
 lda #>(cte1) :\
 cmp ptr2,y :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQW_YA(ptr2,y2,ptr1,y1,label)\
 .( :\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 ldy #y2 :\
 eor ptr2,y :\
 sta tmp :\
 iny :\
 lda 1+ptr1 :\
 adc #0 :\
 eor ptr2,y :\
 ora tmp :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQW_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 eor tmp2 :\
 sta tmp :\
 lda tmp1+1 :\
 eor tmp2+1 :\
 ora tmp :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQW_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 ldy #y2 :\
 eor ptr2,y :\
 sta tmp :\
 iny :\
 lda tmp1+1 :\
 eor ptr2,y :\
 ora tmp :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQW_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 eor tmp2 :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 eor tmp2+1 :\
 ora tmp :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQW_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 cmp ptr2,y :\
 bne skip :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 cmp ptr2,y :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define NE0W_D(tmp,label)\
 .( :\
 lda tmp :\
 ora tmp+1 :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NE0W_Y(ptr,y2,label)\
 .( :\
 ldy #y2 :\
 lda ptr,y :\
 iny :\
 ora ptr,y :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEW_DC(tmp2,cte1,label)\
 .( :\
 lda #<(cte1) :\
 eor tmp2 :\
 sta tmp :\
 lda #>(cte1) :\
 eor tmp2+1 :\
 ora tmp :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEW_DA(tmp2,ptr1,y1,label)\
 .( :\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 eor tmp2 :\
 sta tmp :\
 lda 1+ptr1 :\
 adc #0 :\
 eor tmp2+1 :\
 ora tmp :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEW_YC(ptr2,y2,cte1,label)\
 .( :\
 lda #<(cte1) :\
 ldy #y2 :\
 cmp ptr2,y :\
 beq skip1 :\
 jmp label :\
skip1 :\
 iny :\
 lda #>(cte1) :\
 cmp ptr2,y :\
 beq skip2 :\
 jmp label :\
skip2 :\
 .) :\

#define NEW_YA(ptr2,y2,ptr1,y1,label)\
 .( :\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 ldy #y2 :\
 eor ptr2,y :\
 sta tmp :\
 iny :\
 lda 1+ptr1 :\
 adc #0 :\
 eor ptr2,y :\
 ora tmp :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEW_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 eor tmp2 :\
 sta tmp :\
 lda tmp1+1 :\
 eor tmp2+1 :\
 ora tmp :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEW_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 ldy #y2 :\
 eor ptr2,y :\
 sta tmp :\
 iny :\
 lda tmp1+1 :\
 eor ptr2,y :\
 ora tmp :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEW_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 eor tmp2 :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 eor tmp2+1 :\
 ora tmp :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEW_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 cmp ptr2,y :\
 beq skip1 :\
 jmp label :\
skip1 :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 cmp ptr2,y :\
 beq skip2 :\
 jmp label :\
skip2 :\
 .) :\

#define BRANCH_GEI(label)\
 .( :\
 bvc noflip :\
 eor #$80 :\
noflip :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define BRANCH_GEU(label)\
 .( :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEI_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 lda tmp1+1 :\
 sbc #>(cte2) :\
 BRANCH_GEI(label) :\

#define GEI_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 lda tmp1+1 :\
 sbc tmp2+1 :\
 BRANCH_GEI(label) :\

#define GEI_DY(tmp1,ptr2,y2,label)\
 lda tmp1 :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda tmp1+1 :\
 sbc ptr2,y :\
 BRANCH_GEI(label) :\

#define GEI_YC(ptr1,y1,cte2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp #<(cte2) :\
 iny :\
 lda ptr1,y :\
 sbc #>(cte2) :\
 BRANCH_GEI(label) :\

#define GEI_YD(ptr1,y1,tmp2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp tmp2 :\
 iny :\
 lda ptr1,y :\
 sbc tmp2+1 :\
 BRANCH_GEI(label) :\

#define GEI_YY(ptr1,y1,ptr2,y2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 cmp ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 BRANCH_GEI(label) :\

#define GEI_CD(cte1,tmp2,label)\
 lda #<(cte1) :\
 cmp tmp2 :\
 lda #>(cte1) :\
 sbc tmp2+1 :\
 BRANCH_GEI(label) :\

#define GEI_CY(cte1,ptr2,y2,label)\
 lda #<(cte1) :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda #>(cte1) :\
 sbc ptr2,y :\
 BRANCH_GEI(label) :\

#define GEU_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 lda tmp1+1 :\
 sbc #>(cte2) :\
 BRANCH_GEU(label) :\

#define GEU_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 lda tmp1+1 :\
 sbc tmp2+1 :\
 BRANCH_GEU(label) :\

#define GEU_DY(tmp1,ptr2,y2,label)\
 lda tmp1 :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda tmp1+1 :\
 sbc ptr2,y :\
 BRANCH_GEU(label) :\

#define GEU_YC(ptr1,y1,cte2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp #<(cte2) :\
 iny :\
 lda ptr1,y :\
 sbc #>(cte2) :\
 BRANCH_GEU(label) :\

#define GEU_YD(ptr1,y1,tmp2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp tmp2 :\
 iny :\
 lda ptr1,y :\
 sbc tmp2+1 :\
 BRANCH_GEU(label) :\

#define GEU_YY(ptr1,y1,ptr2,y2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 cmp ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 BRANCH_GEU(label) :\

#define GEU_CD(cte1,tmp2,label)\
 lda #<(cte1) :\
 cmp tmp2 :\
 lda #>(cte1) :\
 sbc tmp2+1 :\
 BRANCH_GEU(label) :\

#define GEU_CY(cte1,ptr2,y2,label)\
 lda #<(cte1) :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda #>(cte1) :\
 sbc ptr2,y :\
 BRANCH_GEU(label) :\

#define BRANCH_LTI(label)\
 .( :\
 bvc noflip :\
 eor #$80 :\
noflip :\
 bpl skip :\
 jmp label :\
skip :\
 .) :\

#define BRANCH_LTU(label)\
 .( :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTI_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 lda tmp1+1 :\
 sbc #>(cte2) :\
 BRANCH_LTI(label) :\

#define LTI_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 lda tmp1+1 :\
 sbc tmp2+1 :\
 BRANCH_LTI(label) :\

#define LTI_DY(tmp1,ptr2,y2,label)\
 lda tmp1 :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda tmp1+1 :\
 sbc ptr2,y :\
 BRANCH_LTI(label) :\

#define LTI_YC(ptr1,y1,cte2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp #<(cte2) :\
 iny :\
 lda ptr1,y :\
 sbc #>(cte2) :\
 BRANCH_LTI(label) :\

#define LTI_YD(ptr1,y1,tmp2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp tmp2 :\
 iny :\
 lda ptr1,y :\
 sbc tmp2+1 :\
 BRANCH_LTI(label) :\

#define LTI_YY(ptr1,y1,ptr2,y2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 cmp ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 BRANCH_LTI(label) :\

#define LTI_CD(cte1,tmp2,label)\
 lda #<(cte1) :\
 cmp tmp2 :\
 lda #>(cte1) :\
 sbc tmp2+1 :\
 BRANCH_LTI(label) :\

#define LTI_CY(cte1,ptr2,y2,label)\
 lda #<(cte1) :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda #>(cte1) :\
 sbc ptr2,y :\
 BRANCH_LTI(label) :\

#define LTU_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 lda tmp1+1 :\
 sbc #>(cte2) :\
 BRANCH_LTU(label) :\

#define LTU_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 lda tmp1+1 :\
 sbc tmp2+1 :\
 BRANCH_LTU(label) :\

#define LTU_DY(tmp1,ptr2,y2,label)\
 lda tmp1 :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda tmp1+1 :\
 sbc ptr2,y :\
 BRANCH_LTU(label) :\

#define LTU_YC(ptr1,y1,cte2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp #<(cte2) :\
 iny :\
 lda ptr1,y :\
 sbc #>(cte2) :\
 BRANCH_LTU(label) :\

#define LTU_YD(ptr1,y1,tmp2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 cmp tmp2 :\
 iny :\
 lda ptr1,y :\
 sbc tmp2+1 :\
 BRANCH_LTU(label) :\

#define LTU_YY(ptr1,y1,ptr2,y2,label)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 cmp ptr2,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sbc ptr2,y :\
 BRANCH_LTU(label) :\

#define LTU_CD(cte1,tmp2,label)\
 lda #<(cte1) :\
 cmp tmp2 :\
 lda #>(cte1) :\
 sbc tmp2+1 :\
 BRANCH_LTU(label) :\

#define LTU_CY(cte1,ptr2,y2,label)\
 lda #<(cte1) :\
 ldy #y2 :\
 cmp ptr2,y :\
 iny :\
 lda #>(cte1) :\
 sbc ptr2,y :\
 BRANCH_LTU(label) :\

#define GTI_DC(tmp1,cte2,label)\
 LTI_CD(cte2,tmp1,label) :\

#define GTI_DD(tmp1,tmp2,label)\
 LTI_DD(tmp2,tmp1,label) :\

#define GTI_DY(tmp1,ptr2,y2,label)\
 LTI_YD(ptr2,y2,tmp1,label) :\

#define GTI_YC(ptr1,y1,cte2,label)\
 LTI_CY(cte2,ptr1,y1,label) :\

#define GTI_YD(ptr1,y1,tmp2,label)\
 LTI_DY(tmp2,ptr1,y1,label) :\

#define GTI_YY(ptr1,y1,ptr2,y2,label)\
 LTI_YY(ptr2,y2,ptr1,y1,label) :\

#define GTI_CD(cte1,tmp2,label)\
 LTI_DC(tmp2,cte1,label) :\

#define GTI_CY(cte1,ptr2,y2,label)\
 LTI_YC(ptr2,y2,cte1,label) :\

#define LEI_DC(tmp1,cte2,label)\
 GEI_CD(cte2,tmp1,label) :\

#define LEI_DD(tmp1,tmp2,label)\
 GEI_DD(tmp2,tmp1,label) :\

#define LEI_DY(tmp1,ptr2,y2,label)\
 GEI_YD(ptr2,y2,tmp1,label) :\

#define LEI_YC(ptr1,y1,cte2,label)\
 GEI_CY(cte2,ptr1,y1,label) :\

#define LEI_YD(ptr1,y1,tmp2,label)\
 GEI_DY(tmp2,ptr1,y1,label) :\

#define LEI_YY(ptr1,y1,ptr2,y2,label)\
 GEI_YY(ptr2,y2,ptr1,y1,label) :\

#define LEI_CD(cte1,tmp2,label)\
 GEI_DC(tmp2,cte1,label) :\

#define LEI_CY(cte1,ptr2,y2,label)\
 GEI_YC(ptr2,y2,cte1,label) :\

#define GTU_DC(tmp1,cte2,label)\
 LTU_CD(cte2,tmp1,label) :\

#define GTU_DD(tmp1,tmp2,label)\
 LTU_DD(tmp2,tmp1,label) :\

#define GTU_DY(tmp1,ptr2,y2,label)\
 LTU_YD(ptr2,y2,tmp1,label) :\

#define GTU_YC(ptr1,y1,cte2,label)\
 LTU_CY(cte2,ptr1,y1,label) :\

#define GTU_YD(ptr1,y1,tmp2,label)\
 LTU_DY(tmp2,ptr1,y1,label) :\

#define GTU_YY(ptr1,y1,ptr2,y2,label)\
 LTU_YY(ptr2,y2,ptr1,y1,label) :\

#define GTU_CD(cte1,tmp2,label)\
 LTU_DC(tmp2,cte1,label) :\

#define GTU_CY(cte1,ptr2,y2,label)\
 LTU_YC(ptr2,y2,cte1,label) :\

#define LEU_DC(tmp1,cte2,label)\
 GEU_CD(cte2,tmp1,label) :\

#define LEU_DD(tmp1,tmp2,label)\
 GEU_DD(tmp2,tmp1,label) :\

#define LEU_DY(tmp1,ptr2,y2,label)\
 GEU_YD(ptr2,y2,tmp1,label) :\

#define LEU_YC(ptr1,y1,cte2,label)\
 GEU_CY(cte2,ptr1,y1,label) :\

#define LEU_YD(ptr1,y1,tmp2,label)\
 GEU_DY(tmp2,ptr1,y1,label) :\

#define LEU_YY(ptr1,y1,ptr2,y2,label)\
 GEU_YY(ptr2,y2,ptr1,y1,label) :\

#define LEU_CD(cte1,tmp2,label)\
 GEU_DC(tmp2,cte1,label) :\

#define LEU_CY(cte1,ptr2,y2,label)\
 GEU_YC(ptr2,y2,cte1,label) :\

; --- byte (unsigned) comparisons - Phase 3 of native char operations ------
; When both compared values are unsigned char (0..255), the promoted 16-bit
; compare reduces to a single-byte compare. Equality is sign-agnostic; the
; ordered forms are unsigned (a uchar is always non-negative, so the promoted
; signed compare of two 0..255 values agrees with the unsigned byte compare).
; gen.c only narrows char-first operand order and byte-range (0..255)
; constants, so only _DD and _DC are needed.
#define BRANCH_GTU(label)\
 .( :\
 bcc skip :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define BRANCH_LEU(label)\
 .( :\
 bcc take :\
 bne skip :\
take :\
 jmp label :\
skip :\
 .) :\

#define EQB_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 cmp tmp2 :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQB_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 cmp #<(cte2) :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define NEB_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 cmp tmp2 :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEB_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 cmp #<(cte2) :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define LTUB_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 BRANCH_LTU(label) :\

#define LTUB_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 BRANCH_LTU(label) :\

#define GEUB_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 BRANCH_GEU(label) :\

#define GEUB_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 BRANCH_GEU(label) :\

#define GTUB_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 BRANCH_GTU(label) :\

#define GTUB_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 BRANCH_GTU(label) :\

#define LEUB_DD(tmp1,tmp2,label)\
 lda tmp1 :\
 cmp tmp2 :\
 BRANCH_LEU(label) :\

#define LEUB_DC(tmp1,cte2,label)\
 lda tmp1 :\
 cmp #<(cte2) :\
 BRANCH_LEU(label) :\

#define SAVE(tmp,ptr,y2) ARGW_D(tmp,ptr,y2)
#define RESTORE(ptr,y2,tmp) MOVW_YD(ptr,y2,tmp)

#define ARGW_C(cte,ptr,y2)\
 lda #<(cte) :\
 ldy #y2 :\
 sta ptr,y :\
 iny :\
 lda #>(cte) :\
 sta ptr,y :\

#define ARGW_D(tmp,ptr,y2)\
 lda tmp :\
 ldy #y2 :\
 sta ptr,y :\
 iny :\
 lda tmp+1 :\
 sta ptr,y :\

#define ARGW_A(ptr1,y1,ptr,y2)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 ldy #y2 :\
 sta ptr,y :\
 iny :\
 lda 1+ptr1 :\
 adc #0 :\
 sta ptr,y :\

#define ARGW_Y(ptr1,y1,ptr,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr,y :\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sta ptr,y :\

#define CALLV_C(adr,size)\
 ldy #size :\
 jsr adr :\

#define CALLV_D(tmp,size)\
 .( :\
 lda tmp :\
 sta call+1:\
 lda tmp+1 :\
 sta call+2 :\
 ldy #size :\
call :\
 jsr 0000 :\
 .) :\

#define CALLV_Y(ptr,y2,size)\
 .( :\
 ldy #y2 :\
 lda ptr,y :\
 sta call+1 :\
 iny :\
 lda ptr,y :\
 sta call+2 :\
 ldy #size :\
call :\
 jsr 0000 :\
 .) :\

#define CALLW_CD(adr,size,tmp)\
 ldy #size :\
 jsr adr :\
 stx tmp :\
 sta tmp+1 :\

#define CALLW_DD(tmp1,size,tmp)\
 CALLV_D(tmp1,size) \
 stx tmp :\
 sta tmp+1 :\

#define CALLW_YD(ptr,y2,size,tmp)\
 CALLV_Y(ptr,y2,size) \
 stx tmp :\
 sta tmp+1 :\

#define CALLW_CY(adr,size,ptr,y2)\
 ldy #size :\
 jsr adr :\
 ldy #y2+1 :\
 sta ptr,y :\
 dey :\
 txa :\
 sta ptr,y :\

#define CALLW_YY(ptr,y0,size,ptr2,y2)\
 CALLV_Y(ptr,y0,size) \
 ldy #y2+1 :\
 sta ptr2,y :\
 dey :\
 txa :\
 sta ptr2,y :\


#define JUMP_C(label)\
 jmp label :\

#define JUMP_D(tmp)\
 .( :\
 lda tmp :\
 sta jump+1 :\
 lda tmp+1 :\
 sta jump+2 :\
jump :\
 jmp 0000 :\
 .) :\

#define JUMP_Y(ptr1,y1)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta jump+1 :\
 iny :\
 lda ptr1,y :\
 sta jump+2 :\
jump :\
 jmp 0000 :\
 .) :\

#define FLOAD1_C(tmp)\
 lda #<(tmp) :\
 ldy #>(tmp) :\
 jsr load_acc1 :\

#define FLOAD1_D(tmp)\
 lda tmp :\
 ldy tmp+1 :\
 jsr load_acc1 :\

#define FLOAD1_A(ptr1,y1)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 tay :\
 txa :\
 jsr load_acc1 :\

#define FLOAD1_Y(ptr1,y1)\
 ldy #y1 :\
 lda ptr1,y :\
 tax :\
 iny :\
 lda ptr1,y :\
 tay :\
 txa :\
 jsr load_acc1 :\

#define FLOAD2_C(tmp)\
 lda #<(tmp) :\
 ldy #>(tmp) :\
 jsr load_acc2 :\

#define FLOAD2_D(tmp)\
 lda tmp :\
 ldy tmp+1 :\
 jsr load_acc2 :\

#define FLOAD2_A(ptr1,y1)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 tay :\
 txa :\
 jsr load_acc2 :\

#define FLOAD2_Y(ptr1,y1)\
 ldy #y1 :\
 lda ptr1,y :\
 tax :\
 iny :\
 lda ptr1,y :\
 tay :\
 txa :\
 jsr load_acc2 :\

#define FSTORE_C(tmp)\
 ldx #<(tmp) :\
 ldy #>(tmp) :\
 jsr store_acc :\

#define FSTORE_D(tmp)\
 ldx tmp :\
 ldy tmp+1 :\
 jsr store_acc :\

#define FSTORE_A(ptr1,y1)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 tay :\
 jsr store_acc :\

#define FSTORE_Y(ptr1,y1)\
 ldy #y1 :\
 lda ptr1,y :\
 tax :\
 iny :\
 lda ptr1,y :\
 tay :\
 jsr store_acc :\

#define FADD\
 jsr fadd :\

#define ADDF_DDD(tmp1,tmp2,tmp3)\
 FLOAD1_C(tmp1) :\
 FLOAD2_C(tmp2) :\
 FADD :\
 FSTORE_C(tmp3) :\

#define ADDF_YDD(ptr1,y1,tmp2,tmp3)\
 FLOAD1_A(ptr1,y1) :\
 FLOAD2_C(tmp2) :\
 FADD :\
 FSTORE_C(tmp3) :\

#define ADDF_DYD(tmp1,ptr2,y2,tmp3)\
 FLOAD1_C(tmp1) :\
 FLOAD2_A(ptr2,y2) :\
 FADD :\
 FSTORE_C(tmp3) :\

#define ADDF_YYD(ptr1,y1,ptr2,y2,tmp3)\
 FLOAD1_A(ptr1,y1) :\
 FLOAD2_A(ptr2,y2) :\
 FADD :\
 FSTORE_C(tmp3) :\

#define ADDF_DDY(tmp1,tmp2,ptr3,y3)\
 FLOAD1_C(tmp1) :\
 FLOAD2_C(tmp2) :\
 FADD :\
 FSTORE_A(ptr3,y3) :\

#define ADDF_YDY(ptr1,y1,tmp2,ptr3,y3)\
 FLOAD1_A(ptr1,y1) :\
 FLOAD2_C(tmp2) :\
 FADD :\
 FSTORE_A(ptr3,y3) :\

#define ADDF_DYY(tmp1,ptr2,y2,ptr3,y3)\
 FLOAD1_C(tmp1) :\
 FLOAD2_A(ptr2,y2) :\
 FADD :\
 FSTORE_A(ptr3,y3) :\

#define ADDF_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 FLOAD1_A(ptr1,y1) :\
 FLOAD2_A(ptr2,y2) :\
 FADD :\
 FSTORE_A(ptr3,y3) :\

#define FSUB\
 jsr fsub :\

#define SUBF_DDD(tmp1,tmp2,tmp3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_C(tmp1) :\
 FSUB :\
 FSTORE_C(tmp3) :\

#define SUBF_YDD(ptr1,y1,tmp2,tmp3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_A(ptr1,y1) :\
 FSUB :\
 FSTORE_C(tmp3) :\

#define SUBF_DYD(tmp1,ptr2,y2,tmp3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_C(tmp1) :\
 FSUB :\
 FSTORE_C(tmp3) :\

#define SUBF_YYD(ptr1,y1,ptr2,y2,tmp3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_A(ptr1,y1) :\
 FSUB :\
 FSTORE_C(tmp3) :\

#define SUBF_DDY(tmp1,tmp2,ptr3,y3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_C(tmp1) :\
 FSUB :\
 FSTORE_A(ptr3,y3) :\

#define SUBF_YDY(ptr1,y1,tmp2,ptr3,y3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_A(ptr1,y1) :\
 FSUB :\
 FSTORE_A(ptr3,y3) :\

#define SUBF_DYY(tmp1,ptr2,y2,ptr3,y3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_C(tmp1) :\
 FSUB :\
 FSTORE_A(ptr3,y3) :\

#define SUBF_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_A(ptr1,y1) :\
 FSUB :\
 FSTORE_A(ptr3,y3) :\

#define FMUL\
 jsr fmul :\

#define MULF_DDD(tmp1,tmp2,tmp3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_C(tmp1) :\
 FMUL :\
 FSTORE_C(tmp3) :\

#define MULF_YDD(ptr1,y1,tmp2,tmp3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_A(ptr1,y1) :\
 FMUL :\
 FSTORE_C(tmp3) :\

#define MULF_DYD(tmp1,ptr2,y2,tmp3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_C(tmp1) :\
 FMUL :\
 FSTORE_C(tmp3) :\

#define MULF_YYD(ptr1,y1,ptr2,y2,tmp3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_A(ptr1,y1) :\
 FMUL :\
 FSTORE_C(tmp3) :\

#define MULF_DDY(tmp1,tmp2,ptr3,y3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_C(tmp1) :\
 FMUL :\
 FSTORE_A(ptr3,y3) :\

#define MULF_YDY(ptr1,y1,tmp2,ptr3,y3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_A(ptr1,y1) :\
 FMUL :\
 FSTORE_A(ptr3,y3) :\

#define MULF_DYY(tmp1,ptr2,y2,ptr3,y3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_C(tmp1) :\
 FMUL :\
 FSTORE_A(ptr3,y3) :\

#define MULF_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_A(ptr1,y1) :\
 FMUL :\
 FSTORE_A(ptr3,y3) :\

#define FDIV\
 jsr fdiv :\

#define DIVF_DDD(tmp1,tmp2,tmp3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_C(tmp1) :\
 FDIV :\
 FSTORE_C(tmp3) :\

#define DIVF_YDD(ptr1,y1,tmp2,tmp3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_A(ptr1,y1) :\
 FDIV :\
 FSTORE_C(tmp3) :\

#define DIVF_DYD(tmp1,ptr2,y2,tmp3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_C(tmp1) :\
 FDIV :\
 FSTORE_C(tmp3) :\

#define DIVF_YYD(ptr1,y1,ptr2,y2,tmp3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_A(ptr1,y1) :\
 FDIV :\
 FSTORE_C(tmp3) :\

#define DIVF_DDY(tmp1,tmp2,ptr3,y3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_C(tmp1) :\
 FDIV :\
 FSTORE_A(ptr3,y3) :\

#define DIVF_YDY(ptr1,y1,tmp2,ptr3,y3)\
 FLOAD1_C(tmp2) :\
 FLOAD2_A(ptr1,y1) :\
 FDIV :\
 FSTORE_A(ptr3,y3) :\

#define DIVF_DYY(tmp1,ptr2,y2,ptr3,y3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_C(tmp1) :\
 FDIV :\
 FSTORE_A(ptr3,y3) :\

#define DIVF_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 FLOAD1_A(ptr2,y2) :\
 FLOAD2_A(ptr1,y1) :\
 FDIV :\
 FSTORE_A(ptr3,y3) :\

#define FNEG\
 jsr fneg :\

#define NEGF_DD(tmp1,tmp2)\
 FLOAD1_C(tmp1) :\
 FNEG :\
 FSTORE_C(tmp2) :\

#define NEGF_YD(ptr1,y1,tmp2)\
 FLOAD1_A(ptr1,y1) :\
 FNEG :\
 FSTORE_C(tmp2) :\

#define NEGF_DY(tmp1,ptr2,y2)\
 FLOAD1_C(tmp1) :\
 FNEG :\
 FSTORE_A(ptr2,y2) :\

#define NEGF_YY(ptr1,y1,ptr2,y2)\
 FLOAD1_A(ptr1,y1) :\
 FNEG :\
 FSTORE_A(ptr2,y2) :\

#define INDIRF_CD(tmp1,tmp2)\
 FLOAD1_C(tmp1) :\
 FSTORE_C(tmp2) :\

#define INDIRF_CY(tmp1,ptr2,y2)\
 FLOAD1_C(tmp1) :\
 FSTORE_A(ptr2,y2) :\

#define INDIRF_DD(tmp1,tmp2)\
 FLOAD1_D(tmp1) :\
 FSTORE_C(tmp2) :\

#define INDIRF_DY(tmp1,ptr2,y2)\
 FLOAD1_D(tmp1) :\
 FSTORE_A(ptr2,y2) :\

#define INDIRF_AD(ptr1,y1,tmp2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_C(tmp2) :\

#define INDIRF_AY(ptr1,y1,ptr2,y2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_A(ptr2,y2) :\

#define INDIRF_YD(ptr1,y1,tmp2)\
 FLOAD1_Y(ptr1,y1) :\
 FSTORE_C(tmp2) :\

#define INDIRF_YY(ptr1,y1,ptr2,y2)\
 FLOAD1_Y(ptr1,y1) :\
 FSTORE_A(ptr2,y2) :\

#define MOVF_DD(tmp1,tmp2)\
 INDIRF_CD(tmp1,tmp2) :\

#define MOVF_DY(tmp1,ptr2,y2)\
 INDIRF_CY(tmp1,ptr2,y2) :\

#define MOVF_YD(ptr1,y1,tmp2)\
 INDIRF_AD(ptr1,y1,tmp2) :\

#define MOVF_YY(ptr1,y1,ptr2,y2)\
 INDIRF_AY(ptr1,y1,ptr2,y2) :\

#define CFI_D(tmp1)\
 jsr cfi :\
 sta tmp1 :\
 stx tmp1+1 :\

#define CFI_Y(ptr1,y1)\
 jsr cfi :\
 ldy #y1 :\
 sta ptr1,y :\
 iny :\
 txa :\
 sta ptr1,y :\

#define CFI_DD(tmp1,tmp2)\
 FLOAD1_C(tmp1) :\
 CFI_D(tmp2) :\

#define CFI_DY(tmp1,ptr2,y2)\
 FLOAD1_C(tmp1) :\
 CFI_Y(ptr2,y2) :\

#define CFI_YD(ptr1,y1,tmp2)\
 FLOAD1_A(ptr1,y1) :\
 CFI_D(tmp2) :\

#define CFI_YY(ptr1,y1,ptr2,y2)\
 FLOAD1_A(ptr1,y1) :\
 CFI_Y(ptr2,y2) :\

#define CIF_C(cte)\
 ldy #<(cte) :\
 lda #>(cte) :\
 jsr cif :\

#define CIF_D(tmp1)\
 ldy tmp1 :\
 lda tmp1+1 :\
 jsr cif :\

#define CIF_Y(ptr1,y1)\
 ldy #y1 :\
 iny :\
 lda ptr1,y :\
 tax :\
 dey :\
 lda ptr1,y :\
 tay :\
 txa :\
 jsr cif :\

#define CIF_CD(cte,tmp) \
 CIF_C(cte) :\
 FSTORE_C(tmp) :\

#define CIF_CY(cte,ptr1,y1) \
 CIF_C(cte) :\
 FSTORE_A(ptr1,y1) :\

#define CIF_DD(tmp1,tmp2)\
 CIF_D(tmp1) :\
 FSTORE_C(tmp2) :\

#define CIF_DY(tmp1,ptr2,y2)\
 CIF_D(tmp1) :\
 FSTORE_A(ptr2,y2) :\

#define CIF_YD(ptr1,y1,tmp2)\
 CIF_Y(ptr1,y1) :\
 FSTORE_C(tmp2) :\

#define CIF_YY(ptr1,y1,ptr2,y2)\
 CIF_Y(ptr1,y1) :\
 FSTORE_A(ptr2,y2) :\

#define RETF_D(tmp1)\
 FLOAD1_C(tmp1) :\
 rts :\

#define RETF_Y(ptr1,y1)\
 FLOAD1_A(ptr1,y1) :\
 rts :\

#define LEAVEF_D(tmp1)\
 FLOAD1_C(tmp1) :\
 jmp leave :\

#define LEAVEF_Y(ptr1,y1)\
 FLOAD1_A(ptr1,y1) :\
 jmp leave :\

#define ASGNF_DC(tmp1,tmp2)\
 FLOAD1_C(tmp1) :\
 FSTORE_C(tmp2) :\

#define ASGNF_DA(tmp1,ptr2,y2)\
 FLOAD1_C(tmp1) :\
 FSTORE_A(ptr2,y2) :\

#define ASGNF_DZ(tmp1,tmp2)\
 FLOAD1_C(tmp1) :\
 FSTORE_D(tmp2) :\

#define ASGNF_DD(tmp1,tmp2)\
 FLOAD1_C(tmp1) :\
 FSTORE_D(tmp2) :\

#define ASGNF_DY(tmp1,ptr2,y2)\
 FLOAD1_C(tmp1) :\
 FSTORE_Y(ptr2,y2) :\

#define ASGNF_YC(ptr1,y1,tmp2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_C(tmp2) :\

#define ASGNF_YZ(ptr1,y1,tmp2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_D(tmp2) :\

#define ASGNF_YD(ptr1,y1,tmp2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_D(tmp2) :\

#define ASGNF_YA(ptr1,y1,ptr2,y2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_A(ptr2,y2) :\

#define ASGNF_YY(ptr1,y1,ptr2,y2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_Y(ptr2,y2) :\

#define ARGF_D(tmp1,ptr2,y2)\
 FLOAD1_C(tmp1) :\
 FSTORE_A(ptr2,y2) :\

#define ARGF_Y(ptr1,y1,ptr2,y2)\
 FLOAD1_A(ptr1,y1) :\
 FSTORE_A(ptr2,y2) :\

#define ARGS_C(adr,ptr2,y2,size) \
 ASGNS_CA(adr,ptr2,y2,size) :\

#define ARGS_D(tmp1,ptr2,y2,size)\
 ASGNS_DA(tmp1,ptr2,y2,size) :\

#define ARGS_A(ptr1,y1,ptr2,y2,size) \
 ASGNS_AA(ptr1,y1,ptr2,y2,size) :\

#define ARGS_Y(ptr1,y1,ptr2,y2,size)\
 ASGNS_YA(ptr1,y1,ptr2,y2,size) :\

#define CALLF_CD(adr,size,tmp)\
 ldy #size :\
 jsr adr :\
 FSTORE_C(tmp) :\

#define CALLF_DD(tmp1,size,tmp)\
 .( :\
 lda tmp1 :\
 sta jsrto+1 :\
 lda tmp1+1 :\
 sta jsrto+2 :\
 ldy #size :\
jsrto :\
 jsr 0000 :\
 .) :\
 FSTORE_C(tmp) :\

#define CALLF_YD(ptr,y2,size,tmp)\
 ldy #y2 :\
 lda ptr,y :\
 sta jsrvect+1 :\
 iny :\
 lda ptr,y :\
 sta jsrvect+2 :\
 ldy #size :\
 jsr jsrvect :\
 FSTORE_C(tmp) :\

#define CALLF_CY(adr,size,ptr2,y2)\
 ldy #size :\
 jsr adr :\
 FSTORE_A(ptr2,y2) :\

#define CALLF_DY(tmp1,size,ptr2,y2)\
 .( :\
 lda tmp1 :\
 sta jsrto+1 :\
 lda tmp1+1 :\
 sta jsrto+2 :\
 ldy #size :\
jsrto :\
 jsr 0000 :\
 .) :\
 FSTORE_A(ptr2,y2) :\

#define CALLF_YY(ptr,y0,size,ptr2,y2)\
 ldy #y0 :\
 lda ptr,y :\
 sta jsrvect+1 :\
 iny :\
 lda ptr,y :\
 sta jsrvect+2 :\
 ldy #size :\
 jsr jsrvect :\
 FSTORE_A(ptr2,y2) :\

#define FCOMP_D(tmp1)\
 lda #<(tmp1) :\
 ldy #>(tmp1) :\
 jsr fcomp :\

#define FCOMP_Y(ptr1,y1)\
 clc :\
 lda 0+ptr1 :\
 adc #y1 :\
 tax :\
 lda 1+ptr1 :\
 adc #0 :\
 tay :\
 txa :\
 jsr fcomp :\

#define EQF_DD(tmp1,tmp2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_D(tmp2) :\
 .( :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQF_DY(tmp1,ptr2,y2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQF_YD(ptr1,y1,tmp2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_D(tmp2) :\
 .( :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQF_YY(ptr1,y1,ptr2,y2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define NEF_DD(tmp1,tmp2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_D(tmp2) :\
 .( :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEF_DY(tmp1,ptr2,y2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEF_YD(ptr1,y1,tmp2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_D(tmp2) :\
 .( :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEF_YY(ptr1,y1,ptr2,y2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define GEF_DD(tmp1,tmp2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_D(tmp2) :\
 .( :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define GEF_DY(tmp1,ptr2,y2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define GEF_YD(ptr1,y1,tmp2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_D(tmp2) :\
 .( :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define GEF_YY(ptr1,y1,ptr2,y2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define LTF_DD(tmp1,tmp2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_D(tmp2) :\
 .( :\
 bpl skip :\
 jmp label :\
skip :\
 .) :\

#define LTF_DY(tmp1,ptr2,y2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 bpl skip :\
 jmp label :\
skip :\
 .) :\

#define LTF_YD(ptr1,y1,tmp2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_D(tmp2) :\
 .( :\
 bpl skip :\
 jmp label :\
skip :\
 .) :\

#define LTF_YY(ptr1,y1,ptr2,y2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 bpl skip :\
 jmp label :\
skip :\
 .) :\

#define GTF_DD(tmp1,tmp2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_D(tmp2) :\
 .( :\
 beq skip :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define GTF_DY(tmp1,ptr2,y2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 beq skip :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define GTF_YD(ptr1,y1,tmp2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_D(tmp2) :\
 .( :\
 beq skip :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define GTF_YY(ptr1,y1,ptr2,y2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 beq skip :\
 bmi skip :\
 jmp label :\
skip :\
 .) :\

#define LEF_DD(tmp1,tmp2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_D(tmp2) :\
 .( :\
 beq dojmp :\
 bpl skip :\
dojmp :\
 jmp label :\
skip :\
 .) :\

#define LEF_DY(tmp1,ptr2,y2,label)\
 FLOAD1_C(tmp1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 beq dojmp :\
 bpl skip :\
dojmp :\
 jmp label :\
skip :\
 .) :\

#define LEF_YD(ptr1,y1,tmp2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_D(tmp2) :\
 .( :\
 beq dojmp :\
 bpl skip :\
dojmp :\
 jmp label :\
skip :\
 .) :\

#define LEF_YY(ptr1,y1,ptr2,y2,label)\
 FLOAD1_A(ptr1,y1) :\
 FCOMP_Y(ptr2,y2) :\
 .( :\
 beq dojmp :\
 bpl skip :\
dojmp :\
 jmp label :\
skip :\
 .) :\


#define CALLW_DY(tmp1,size,ptr,y2)\
    CALLV_D(tmp1,size) \
    ldy #y2+1 :\
    sta ptr,y :\
    dey :\
    txa :\
    sta ptr,y :\


