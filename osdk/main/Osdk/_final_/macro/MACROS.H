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

; in-place byte shift by 1 (operand==result): a single memory read-modify-write
; instead of load/shift/store. Emitted by gen.c byte_shift1() for char x<<=1 / x>>=1.
#define LSH1B_D(tmp1)\
 asl tmp1 :\

#define RSH1B_D(tmp1)\
 lsr tmp1 :\

; in-place byte increment/decrement (char c++ / c--): a single memory RMW
; instead of clc/lda/adc/sta. Emitted by gen.c when a narrowed ADDB/SUBB by 1
; has operand==result (like the word INCW/DECW special case).
#define INCB_D(tmp1)\
 inc tmp1 :\

#define DECB_D(tmp1)\
 dec tmp1 :\

; shift a 16-bit value by exactly 8 = a byte move (no loop). Emitted for
; x<<8 / x>>8 (e.g. splitting an address, or fetching the integer part of an
; 8.8 fixed-point value) instead of the 8-iteration LSHW/RSHW/ASRW routine.
; LSHW8: r = x<<8  -> r.hi = x.lo, r.lo = 0
#define LSHW8_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta tmp2+1 :\
 lda #0 :\
 sta tmp2 :\

#define LSHW8_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 ldy #y2+1 :\
 sta ptr2,y :\
 lda #0 :\
 ldy #y2 :\
 sta ptr2,y :\

#define LSHW8_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2+1 :\
 lda #0 :\
 sta tmp2 :\

#define LSHW8_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 ldy #y2+1 :\
 sta ptr2,y :\
 lda #0 :\
 ldy #y2 :\
 sta ptr2,y :\

; RSHW8: r = x>>8 unsigned/logical -> r.lo = x.hi, r.hi = 0
#define RSHW8_DD(tmp1,tmp2)\
 lda tmp1+1 :\
 sta tmp2 :\
 lda #0 :\
 sta tmp2+1 :\

#define RSHW8_DY(tmp1,ptr2,y2)\
 lda tmp1+1 :\
 ldy #y2 :\
 sta ptr2,y :\
 lda #0 :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define RSHW8_YD(ptr1,y1,tmp2)\
 ldy #y1+1 :\
 lda ptr1,y :\
 sta tmp2 :\
 lda #0 :\
 sta tmp2+1 :\

#define RSHW8_YY(ptr1,y1,ptr2,y2)\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 lda #0 :\
 ldy #y2+1 :\
 sta ptr2,y :\

; ASRW8: r = x>>8 signed/arithmetic -> r.lo = x.hi, r.hi = sign of x.hi
; (branchless: after storing x.hi, asl pushes its bit 7 into carry, then
;  0-borrow gives $00/$FF inverted, eor #$FF corrects it to $FF when negative)
#define ASRW8_DD(tmp1,tmp2)\
 lda tmp1+1 :\
 sta tmp2 :\
 asl :\
 lda #0 :\
 sbc #0 :\
 eor #$FF :\
 sta tmp2+1 :\

#define ASRW8_DY(tmp1,ptr2,y2)\
 lda tmp1+1 :\
 ldy #y2 :\
 sta ptr2,y :\
 asl :\
 lda #0 :\
 sbc #0 :\
 eor #$FF :\
 ldy #y2+1 :\
 sta ptr2,y :\

#define ASRW8_YD(ptr1,y1,tmp2)\
 ldy #y1+1 :\
 lda ptr1,y :\
 sta tmp2 :\
 asl :\
 lda #0 :\
 sbc #0 :\
 eor #$FF :\
 sta tmp2+1 :\

#define ASRW8_YY(ptr1,y1,ptr2,y2)\
 ldy #y1+1 :\
 lda ptr1,y :\
 ldy #y2 :\
 sta ptr2,y :\
 asl :\
 lda #0 :\
 sbc #0 :\
 eor #$FF :\
 ldy #y2+1 :\
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

; in-place 16-bit shift-left-by-1 (operand==result, direct/zero-page). Used by
; gen.c to unroll a small constant left shift (x<<2, x<<3): one LSH1W to shift
; a->r, then this once/twice more in place. Smaller AND faster than the
; variable-count LSHW loop for counts 2-3 (no ldx/beq/dex/bne scaffold).
#define LSH1W_D(tmp1)\
 asl tmp1 :\
 rol tmp1+1 :\

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

; byte compare against zero: lda already sets Z (and N), so no cmp #0 is
; needed. Emitted by gen.c for a narrowed == 0 / != 0 (the byte mirror of
; EQ0W_D / NE0W_D). Equality only - the ordered byte compares (LTUB/GTUB/...)
; need the carry that cmp sets, so they keep their cmp.
#define EQ0B_D(tmp1,label)\
 .( :\
 lda tmp1 :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define NE0B_D(tmp1,label)\
 .( :\
 lda tmp1 :\
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
 .( :\
 ldy #y0 :\
 lda ptr,y :\
 sta call+1 :\
 iny :\
 lda ptr,y :\
 sta call+2 :\
 ldy #size :\
call :\
 jsr 0000 :\
 .) :\
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


;
; ============================================================
; In-place 32-bit RMW with a constant - ADDLK/SUBLK (hand-written)
; ============================================================
; emitted by gen.c's mark_inplace_rmw for `mem_long += const` / `-= const`
; when the store destination is the SAME lvalue as the add/sub's load source.
; Operate directly on the 4 destination bytes with a carry/borrow chain - no
; op1:op2 staging, no lscratch, no jsr ladd32/lsub32. iny / lda / sta preserve
; the carry between bytes. NOT part of the gen_lmacros.py family set.
;
; _A: frame/param slot  (fp),off       _C: direct/static variable
;
#define ADDLK_A(ptr1,y1,cte)\
 ldy #y1 :\
 clc :\
 lda ptr1,y :\
 adc #<(cte) :\
 sta ptr1,y :\
 iny :\
 lda ptr1,y :\
 adc #>(cte) :\
 sta ptr1,y :\
 iny :\
 lda ptr1,y :\
 adc #<((cte)>>16) :\
 sta ptr1,y :\
 iny :\
 lda ptr1,y :\
 adc #>((cte)>>16) :\
 sta ptr1,y :\

#define SUBLK_A(ptr1,y1,cte)\
 ldy #y1 :\
 sec :\
 lda ptr1,y :\
 sbc #<(cte) :\
 sta ptr1,y :\
 iny :\
 lda ptr1,y :\
 sbc #>(cte) :\
 sta ptr1,y :\
 iny :\
 lda ptr1,y :\
 sbc #<((cte)>>16) :\
 sta ptr1,y :\
 iny :\
 lda ptr1,y :\
 sbc #>((cte)>>16) :\
 sta ptr1,y :\

#define ADDLK_C(var,cte)\
 clc :\
 lda var :\
 adc #<(cte) :\
 sta var :\
 lda var+1 :\
 adc #>(cte) :\
 sta var+1 :\
 lda var+2 :\
 adc #<((cte)>>16) :\
 sta var+2 :\
 lda var+3 :\
 adc #>((cte)>>16) :\
 sta var+3 :\

#define SUBLK_C(var,cte)\
 sec :\
 lda var :\
 sbc #<(cte) :\
 sta var :\
 lda var+1 :\
 sbc #>(cte) :\
 sta var+1 :\
 lda var+2 :\
 sbc #<((cte)>>16) :\
 sta var+2 :\
 lda var+3 :\
 sbc #>((cte)>>16) :\
 sta var+3 :\


;
; ============================================================
; Fused 32-bit `dst = src +/- const` - ADDLKM/SUBLKM (hand-written)
; ============================================================
; emitted by gen.c's emit_fused_addk for `temp = mem_long +/- const`: read the
; source long (frame (fp),y = _A ; static/absolute var = _C), add/sub the
; constant's bytes with an inline carry/borrow chain, write the result temp.
; No op1:op2, no lscratch, no jsr ladd32/lsub32. iny/lda/sta preserve the carry.
; The source INDIR's own load is suppressed by gen.c (its node is marked inplace).
;
#define ADDLKM_A(ptr1,y1,cte,dst)\
 ldy #y1 :\
 clc :\
 lda ptr1,y :\
 adc #<(cte) :\
 sta dst :\
 iny :\
 lda ptr1,y :\
 adc #>(cte) :\
 sta dst+1 :\
 iny :\
 lda ptr1,y :\
 adc #<((cte)>>16) :\
 sta dst+2 :\
 iny :\
 lda ptr1,y :\
 adc #>((cte)>>16) :\
 sta dst+3 :\

#define SUBLKM_A(ptr1,y1,cte,dst)\
 ldy #y1 :\
 sec :\
 lda ptr1,y :\
 sbc #<(cte) :\
 sta dst :\
 iny :\
 lda ptr1,y :\
 sbc #>(cte) :\
 sta dst+1 :\
 iny :\
 lda ptr1,y :\
 sbc #<((cte)>>16) :\
 sta dst+2 :\
 iny :\
 lda ptr1,y :\
 sbc #>((cte)>>16) :\
 sta dst+3 :\

#define ADDLKM_C(src,cte,dst)\
 clc :\
 lda src :\
 adc #<(cte) :\
 sta dst :\
 lda src+1 :\
 adc #>(cte) :\
 sta dst+1 :\
 lda src+2 :\
 adc #<((cte)>>16) :\
 sta dst+2 :\
 lda src+3 :\
 adc #>((cte)>>16) :\
 sta dst+3 :\

#define SUBLKM_C(src,cte,dst)\
 sec :\
 lda src :\
 sbc #<(cte) :\
 sta dst :\
 lda src+1 :\
 sbc #>(cte) :\
 sta dst+1 :\
 lda src+2 :\
 sbc #<((cte)>>16) :\
 sta dst+2 :\
 lda src+3 :\
 sbc #>((cte)>>16) :\
 sta dst+3 :\


;
; ============================================================
; Inline 32-bit compare vs a constant - <REL>LK_D (hand-written)
; ============================================================
; emitted by gen.c's compare() for a width-4 relation whose variable operand is
; a D-mode temp and whose other operand is an integer constant. Compares the 4
; temp bytes MSB->LSB directly against the constant's bytes (b0..b3, passed as
; literals by gen.c) - no op1:op2, no lscratch, no jsr lcmp32. Only LT/GE/EQ/NE
; (+ unsigned LTU/GEU) exist: gen.c canonicalises the constant to the right and
; folds a<=k -> a<k+1, a>k -> a>=k+1. Signed (LTLK/GELK) flip A's and (via the
; passed byte) the constant's sign bit, turning the signed compare into an
; unsigned one; the 4-byte compare leaves C and Z exactly like lcmp32i/u, so the
; branch tail is the same as the routine macros'.
;
#define LTLK_D(var,b0,b1,b2,b3,label)\
 .( :\
 lda var+3 :\
 eor #$80 :\
 cmp #b3 :\
 bne cdone :\
 lda var+2 :\
 cmp #b2 :\
 bne cdone :\
 lda var+1 :\
 cmp #b1 :\
 bne cdone :\
 lda var :\
 cmp #b0 :\
cdone :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define GELK_D(var,b0,b1,b2,b3,label)\
 .( :\
 lda var+3 :\
 eor #$80 :\
 cmp #b3 :\
 bne cdone :\
 lda var+2 :\
 cmp #b2 :\
 bne cdone :\
 lda var+1 :\
 cmp #b1 :\
 bne cdone :\
 lda var :\
 cmp #b0 :\
cdone :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define LTULK_D(var,b0,b1,b2,b3,label)\
 .( :\
 lda var+3 :\
 cmp #b3 :\
 bne cdone :\
 lda var+2 :\
 cmp #b2 :\
 bne cdone :\
 lda var+1 :\
 cmp #b1 :\
 bne cdone :\
 lda var :\
 cmp #b0 :\
cdone :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define GEULK_D(var,b0,b1,b2,b3,label)\
 .( :\
 lda var+3 :\
 cmp #b3 :\
 bne cdone :\
 lda var+2 :\
 cmp #b2 :\
 bne cdone :\
 lda var+1 :\
 cmp #b1 :\
 bne cdone :\
 lda var :\
 cmp #b0 :\
cdone :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define EQLK_D(var,b0,b1,b2,b3,label)\
 .( :\
 lda var+3 :\
 cmp #b3 :\
 bne cdone :\
 lda var+2 :\
 cmp #b2 :\
 bne cdone :\
 lda var+1 :\
 cmp #b1 :\
 bne cdone :\
 lda var :\
 cmp #b0 :\
cdone :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define NELK_D(var,b0,b1,b2,b3,label)\
 .( :\
 lda var+3 :\
 cmp #b3 :\
 bne cdone :\
 lda var+2 :\
 cmp #b2 :\
 bne cdone :\
 lda var+1 :\
 cmp #b1 :\
 bne cdone :\
 lda var :\
 cmp #b0 :\
cdone :\
 beq skip :\
 jmp label :\
skip :\
 .) :\


;
; ============================================================
; 32-bit (long) macro families - L suffix
; ============================================================
;
; Shell convention (see lib/long32.s): operand A is loaded into the
; 4 contiguous zero-page bytes op1..op2+1, operand B is pointed to by
; tmp (constants staged through lscratch), the long32.s routine is
; called, and the result is stored from op1:op2. 32-bit returns travel
; as X:A = low word + op1 = high word (leave never touches op1).
; GENERATED by macro/gen_lmacros.py - regenerate rather than
; hand-editing families wholesale.
;
#define ADDL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ADDL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ADDL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ladd32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define SUBL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define SUBL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lsub32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ANDL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ANDL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr land32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ORL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ORL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define XORL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define XORL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lxor32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MULL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MULL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmul32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define DIVUL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define DIVUL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr ldiv32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32i :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define MODUL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define MODUL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lmod32u :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define LSHL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define LSHL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr llsh32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define RSHL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define RSHL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lrshl32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_CAD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_CAY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_CCD(cte1,cte2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_CCY(cte1,cte2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_CDD(cte1,tmp2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_CDY(cte1,tmp2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_CYD(cte1,ptr2,y2,tmp3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_CYY(cte1,ptr2,y2,ptr3,y3)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_DAD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_DAY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_DCD(tmp1,cte2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_DCY(tmp1,cte2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_DDD(tmp1,tmp2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_DDY(tmp1,tmp2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_DYD(tmp1,ptr2,y2,tmp3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_DYY(tmp1,ptr2,y2,ptr3,y3)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_YAD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_YAY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_YCD(ptr1,y1,cte2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_YCY(ptr1,y1,cte2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_YDD(ptr1,y1,tmp2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_YDY(ptr1,y1,tmp2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define ASRL_YYD(ptr1,y1,ptr2,y2,tmp3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 lda op1 :\
 sta tmp3 :\
 lda op1+1 :\
 sta tmp3+1 :\
 lda op2 :\
 sta tmp3+2 :\
 lda op2+1 :\
 sta tmp3+3 :\

#define ASRL_YYY(ptr1,y1,ptr2,y2,ptr3,y3)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lasr32 :\
 ldy #y3 :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\
 iny :\
 lda op2 :\
 sta ptr3,y :\
 iny :\
 lda op2+1 :\
 sta ptr3,y :\

#define COML_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 jsr lcom32 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define COML_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 jsr lcom32 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define COML_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 jsr lcom32 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define COML_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 jsr lcom32 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define NEGL_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 jsr lneg32 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define NEGL_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 jsr lneg32 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define NEGL_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 jsr lneg32 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define NEGL_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 jsr lneg32 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define EQL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define EQL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bne skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define NEL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define LTUL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcs skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GEUL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define LEL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32i :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define GTUL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define GTUL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq skip :\
 bcc skip :\
 jmp label :\
skip :\
 .) :\

#define LEUL_CA(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_CC(cte1,cte2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_CD(cte1,tmp2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_CY(cte1,ptr2,y2,label)\
 .( :\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_DA(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_DC(tmp1,cte2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_DD(tmp1,tmp2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_DY(tmp1,ptr2,y2,label)\
 .( :\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_YA(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_YC(ptr1,y1,cte2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<(cte2) :\
 sta lscratch :\
 lda #>(cte2) :\
 sta lscratch+1 :\
 lda #<((cte2)>>16) :\
 sta lscratch+2 :\
 lda #>((cte2)>>16) :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_YD(ptr1,y1,tmp2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda #<tmp2 :\
 sta tmp :\
 lda #>tmp2 :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define LEUL_YY(ptr1,y1,ptr2,y2,label)\
 .( :\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta lscratch :\
 iny :\
 lda ptr2,y :\
 sta lscratch+1 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+2 :\
 iny :\
 lda ptr2,y :\
 sta lscratch+3 :\
 lda #<lscratch :\
 sta tmp :\
 lda #>lscratch :\
 sta tmp+1 :\
 jsr lcmp32u :\
 beq dolbl :\
 bcs skip :\
dolbl :\
 jmp label :\
skip :\
 .) :\

#define INDIRL_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define INDIRL_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define INDIRL_CD(cte1,tmp2)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta op1 :\
 iny :\
 lda (tmp),y :\
 sta op1+1 :\
 iny :\
 lda (tmp),y :\
 sta op2 :\
 iny :\
 lda (tmp),y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define INDIRL_CY(cte1,ptr2,y2)\
 lda #<(cte1) :\
 sta tmp :\
 lda #>(cte1) :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta op1 :\
 iny :\
 lda (tmp),y :\
 sta op1+1 :\
 iny :\
 lda (tmp),y :\
 sta op2 :\
 iny :\
 lda (tmp),y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define INDIRL_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta op1 :\
 iny :\
 lda (tmp),y :\
 sta op1+1 :\
 iny :\
 lda (tmp),y :\
 sta op2 :\
 iny :\
 lda (tmp),y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define INDIRL_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta tmp :\
 lda tmp1+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta op1 :\
 iny :\
 lda (tmp),y :\
 sta op1+1 :\
 iny :\
 lda (tmp),y :\
 sta op2 :\
 iny :\
 lda (tmp),y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define INDIRL_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta op1 :\
 iny :\
 lda (tmp),y :\
 sta op1+1 :\
 iny :\
 lda (tmp),y :\
 sta op2 :\
 iny :\
 lda (tmp),y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define INDIRL_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp :\
 iny :\
 lda ptr1,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda (tmp),y :\
 sta op1 :\
 iny :\
 lda (tmp),y :\
 sta op1+1 :\
 iny :\
 lda (tmp),y :\
 sta op2 :\
 iny :\
 lda (tmp),y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define INDIRL_ZD(zp1,tmp2)\
 ldy #0 :\
 lda (zp1),y :\
 sta op1 :\
 iny :\
 lda (zp1),y :\
 sta op1+1 :\
 iny :\
 lda (zp1),y :\
 sta op2 :\
 iny :\
 lda (zp1),y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define INDIRL_ZY(zp1,ptr2,y2)\
 ldy #0 :\
 lda (zp1),y :\
 sta op1 :\
 iny :\
 lda (zp1),y :\
 sta op1+1 :\
 iny :\
 lda (zp1),y :\
 sta op2 :\
 iny :\
 lda (zp1),y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define ASGNL_AA(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define ASGNL_AC(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define ASGNL_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda tmp2 :\
 sta tmp :\
 lda tmp2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_AZ(ptr1,y1,zp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #0 :\
 lda op1 :\
 sta (zp2),y :\
 iny :\
 lda op1+1 :\
 sta (zp2),y :\
 iny :\
 lda op2 :\
 sta (zp2),y :\
 iny :\
 lda op2+1 :\
 sta (zp2),y :\

#define ASGNL_CA(cte1,ptr2,y2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define ASGNL_CC(cte1,tmp2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define ASGNL_CD(cte1,tmp2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda tmp2 :\
 sta tmp :\
 lda tmp2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_CY(cte1,ptr2,y2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_CZ(cte1,zp2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #0 :\
 lda op1 :\
 sta (zp2),y :\
 iny :\
 lda op1+1 :\
 sta (zp2),y :\
 iny :\
 lda op2 :\
 sta (zp2),y :\
 iny :\
 lda op2+1 :\
 sta (zp2),y :\

#define ASGNL_DA(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define ASGNL_DC(tmp1,tmp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define ASGNL_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda tmp2 :\
 sta tmp :\
 lda tmp2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_DZ(tmp1,zp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #0 :\
 lda op1 :\
 sta (zp2),y :\
 iny :\
 lda op1+1 :\
 sta (zp2),y :\
 iny :\
 lda op2 :\
 sta (zp2),y :\
 iny :\
 lda op2+1 :\
 sta (zp2),y :\

#define ASGNL_YA(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define ASGNL_YC(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define ASGNL_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda tmp2 :\
 sta tmp :\
 lda tmp2+1 :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda ptr2,y :\
 sta tmp :\
 iny :\
 lda ptr2,y :\
 sta tmp+1 :\
 ldy #0 :\
 lda op1 :\
 sta (tmp),y :\
 iny :\
 lda op1+1 :\
 sta (tmp),y :\
 iny :\
 lda op2 :\
 sta (tmp),y :\
 iny :\
 lda op2+1 :\
 sta (tmp),y :\

#define ASGNL_YZ(ptr1,y1,zp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #0 :\
 lda op1 :\
 sta (zp2),y :\
 iny :\
 lda op1+1 :\
 sta (zp2),y :\
 iny :\
 lda op2 :\
 sta (zp2),y :\
 iny :\
 lda op2+1 :\
 sta (zp2),y :\

#define MOVL_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define MOVL_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define MOVL_CD(cte1,tmp2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define MOVL_CY(cte1,ptr2,y2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define MOVL_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define MOVL_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define MOVL_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define MOVL_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CZWL_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CSWL_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CZWL_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CSWL_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CZWL_CD(cte1,tmp2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CSWL_CD(cte1,tmp2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CZWL_CY(cte1,ptr2,y2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CSWL_CY(cte1,ptr2,y2)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CZWL_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CSWL_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CZWL_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CSWL_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CZWL_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CSWL_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 lda op1 :\
 sta tmp2 :\
 lda op1+1 :\
 sta tmp2+1 :\
 lda op2 :\
 sta tmp2+2 :\
 lda op2+1 :\
 sta tmp2+3 :\

#define CZWL_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 lda #0 :\
 sta op2 :\
 sta op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CSWL_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldx #0 :\
 lda op1+1 :\
 bpl *+3 :\
 dex :\
 stx op2 :\
 stx op2+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\
 iny :\
 lda op2 :\
 sta ptr2,y :\
 iny :\
 lda op2+1 :\
 sta ptr2,y :\

#define CLW_AD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2 :\
 iny :\
 lda ptr1,y :\
 sta tmp2+1 :\

#define CLW_AY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\

#define CLW_DD(tmp1,tmp2)\
 lda tmp1 :\
 sta tmp2 :\
 lda tmp1+1 :\
 sta tmp2+1 :\

#define CLW_DY(tmp1,ptr2,y2)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\

#define CLW_YD(ptr1,y1,tmp2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta tmp2 :\
 iny :\
 lda ptr1,y :\
 sta tmp2+1 :\

#define CLW_YY(ptr1,y1,ptr2,y2)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y2 :\
 lda op1 :\
 sta ptr2,y :\
 iny :\
 lda op1+1 :\
 sta ptr2,y :\

#define ARGL_A(ptr1,y1,ptr,off)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #off :\
 lda op1 :\
 sta ptr,y :\
 iny :\
 lda op1+1 :\
 sta ptr,y :\
 iny :\
 lda op2 :\
 sta ptr,y :\
 iny :\
 lda op2+1 :\
 sta ptr,y :\

#define ARGL_C(cte1,ptr,off)\
 lda #<(cte1) :\
 sta op1 :\
 lda #>(cte1) :\
 sta op1+1 :\
 lda #<((cte1)>>16) :\
 sta op2 :\
 lda #>((cte1)>>16) :\
 sta op2+1 :\
 ldy #off :\
 lda op1 :\
 sta ptr,y :\
 iny :\
 lda op1+1 :\
 sta ptr,y :\
 iny :\
 lda op2 :\
 sta ptr,y :\
 iny :\
 lda op2+1 :\
 sta ptr,y :\

#define ARGL_D(tmp1,ptr,off)\
 lda tmp1 :\
 sta op1 :\
 lda tmp1+1 :\
 sta op1+1 :\
 lda tmp1+2 :\
 sta op2 :\
 lda tmp1+3 :\
 sta op2+1 :\
 ldy #off :\
 lda op1 :\
 sta ptr,y :\
 iny :\
 lda op1+1 :\
 sta ptr,y :\
 iny :\
 lda op2 :\
 sta ptr,y :\
 iny :\
 lda op2+1 :\
 sta ptr,y :\

#define ARGL_Y(ptr1,y1,ptr,off)\
 ldy #y1 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 iny :\
 lda ptr1,y :\
 sta op2 :\
 iny :\
 lda ptr1,y :\
 sta op2+1 :\
 ldy #off :\
 lda op1 :\
 sta ptr,y :\
 iny :\
 lda op1+1 :\
 sta ptr,y :\
 iny :\
 lda op2 :\
 sta ptr,y :\
 iny :\
 lda op2+1 :\
 sta ptr,y :\

#define CALLL_CD(fn,nbparam,res)\
 ldy #nbparam :\
 jsr fn :\
 stx res :\
 sta res+1 :\
 lda op1 :\
 sta res+2 :\
 lda op1+1 :\
 sta res+3 :\

#define CALLL_CY(fn,nbparam,ptr3,y3)\
 ldy #nbparam :\
 jsr fn :\
 sta lscratch :\
 txa :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda lscratch :\
 sta ptr3,y :\
 iny :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\

#define CALLL_DD(fn,nbparam,res)\
 CALLV_D(fn,nbparam) \
 stx res :\
 sta res+1 :\
 lda op1 :\
 sta res+2 :\
 lda op1+1 :\
 sta res+3 :\

#define CALLL_DY(fn,nbparam,ptr3,y3)\
 CALLV_D(fn,nbparam) \
 sta lscratch :\
 txa :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda lscratch :\
 sta ptr3,y :\
 iny :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\

#define CALLL_YD(ptr1,y1,nbparam,res)\
 CALLV_Y(ptr1,y1,nbparam) \
 stx res :\
 sta res+1 :\
 lda op1 :\
 sta res+2 :\
 lda op1+1 :\
 sta res+3 :\

#define CALLL_YY(ptr1,y1,nbparam,ptr3,y3)\
 CALLV_Y(ptr1,y1,nbparam) \
 sta lscratch :\
 txa :\
 ldy #y3 :\
 sta ptr3,y :\
 iny :\
 lda lscratch :\
 sta ptr3,y :\
 iny :\
 lda op1 :\
 sta ptr3,y :\
 iny :\
 lda op1+1 :\
 sta ptr3,y :\

#define RETL_A(ptr1,y1)\
 ldy #y1+2 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 sta lscratch :\
 iny :\
 lda ptr1,y :\
 ldx lscratch :\
 rts :\

#define RETL_C(cte1)\
 lda #<((cte1)>>16) :\
 sta op1 :\
 lda #>((cte1)>>16) :\
 sta op1+1 :\
 ldx #<(cte1) :\
 lda #>(cte1) :\
 rts :\

#define RETL_D(tmp1)\
 lda tmp1+2 :\
 sta op1 :\
 lda tmp1+3 :\
 sta op1+1 :\
 ldx tmp1 :\
 lda tmp1+1 :\
 rts :\

#define RETL_Y(ptr1,y1)\
 ldy #y1+2 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 sta lscratch :\
 iny :\
 lda ptr1,y :\
 ldx lscratch :\
 rts :\

#define LEAVEL_A(ptr1,y1)\
 ldy #y1+2 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 sta lscratch :\
 iny :\
 lda ptr1,y :\
 ldx lscratch :\
 jmp leave :\

#define LEAVEL_C(cte1)\
 lda #<((cte1)>>16) :\
 sta op1 :\
 lda #>((cte1)>>16) :\
 sta op1+1 :\
 ldx #<(cte1) :\
 lda #>(cte1) :\
 jmp leave :\

#define LEAVEL_D(tmp1)\
 lda tmp1+2 :\
 sta op1 :\
 lda tmp1+3 :\
 sta op1+1 :\
 ldx tmp1 :\
 lda tmp1+1 :\
 jmp leave :\

#define LEAVEL_Y(ptr1,y1)\
 ldy #y1+2 :\
 lda ptr1,y :\
 sta op1 :\
 iny :\
 lda ptr1,y :\
 sta op1+1 :\
 ldy #y1 :\
 lda ptr1,y :\
 sta lscratch :\
 iny :\
 lda ptr1,y :\
 ldx lscratch :\
 jmp leave :\

