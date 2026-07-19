# -*- coding: utf-8 -*-
# gen_lmacros.py - generate the flat 32-bit (L) macro families for MACROS.H.
# Shell design: load operand A -> op1:op2, point tmp at operand B, jsr the
# long32.s routine, store op1:op2 -> result. All helper text expanded FLAT
# (macrosplitter's expander is not assumed to handle nested #defines).
# Python 2.7. Writes lmacros.txt next to itself.
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'lmacros.txt')

# ---- helper text fragments (lists of asm lines) ---------------------------

def load_a(mode, arg):
    """operand A (4 bytes) -> op1:op2"""
    if mode == 'D':
        return [' lda %s :' % arg, ' sta op1 :',
                ' lda %s+1 :' % arg, ' sta op1+1 :',
                ' lda %s+2 :' % arg, ' sta op2 :',
                ' lda %s+3 :' % arg, ' sta op2+1 :']
    if mode == 'Y':   # arg = "ptr1,y1" -> two macro args ptr / y
        p, y = arg
        return [' ldy #%s :' % y,
                ' lda (%s),y :' % p, ' sta op1 :', ' iny :',
                ' lda (%s),y :' % p, ' sta op1+1 :', ' iny :',
                ' lda (%s),y :' % p, ' sta op2 :', ' iny :',
                ' lda (%s),y :' % p, ' sta op2+1 :']
    if mode == 'A':   # frame slot (fp),n
        return [' ldy #%s :' % arg,
                ' lda (fp),y :', ' sta op1 :', ' iny :',
                ' lda (fp),y :', ' sta op1+1 :', ' iny :',
                ' lda (fp),y :', ' sta op2 :', ' iny :',
                ' lda (fp),y :', ' sta op2+1 :']
    if mode == 'C':
        return [' lda #<(%s) :' % arg, ' sta op1 :',
                ' lda #>(%s) :' % arg, ' sta op1+1 :',
                ' lda #<((%s)>>16) :' % arg, ' sta op2 :',
                ' lda #>((%s)>>16) :' % arg, ' sta op2+1 :']
    raise ValueError(mode)

def point_b(mode, arg):
    """tmp -> address of operand B (4 bytes)"""
    if mode == 'D':
        return [' lda #<%s :' % arg, ' sta tmp :',
                ' lda #>%s :' % arg, ' sta tmp+1 :']
    if mode == 'Y':
        p, y = arg
        return [' clc :',
                ' lda %s :' % p, ' adc #%s :' % y, ' sta tmp :',
                ' lda %s+1 :' % p, ' adc #0 :', ' sta tmp+1 :']
    if mode == 'A':
        return [' clc :',
                ' lda fp :', ' adc #%s :' % arg, ' sta tmp :',
                ' lda fp+1 :', ' adc #0 :', ' sta tmp+1 :']
    if mode == 'C':   # constants stage through lscratch (long32.s .bss)
        return [' lda #<(%s) :' % arg, ' sta lscratch :',
                ' lda #>(%s) :' % arg, ' sta lscratch+1 :',
                ' lda #<((%s)>>16) :' % arg, ' sta lscratch+2 :',
                ' lda #>((%s)>>16) :' % arg, ' sta lscratch+3 :',
                ' lda #<lscratch :', ' sta tmp :',
                ' lda #>lscratch :', ' sta tmp+1 :']
    raise ValueError(mode)

def store_r(mode, arg):
    """op1:op2 -> result (4 bytes)"""
    if mode == 'D':
        return [' lda op1 :', ' sta %s :' % arg,
                ' lda op1+1 :', ' sta %s+1 :' % arg,
                ' lda op2 :', ' sta %s+2 :' % arg,
                ' lda op2+1 :', ' sta %s+3 :' % arg]
    if mode == 'Y':
        p, y = arg
        return [' ldy #%s :' % y,
                ' lda op1 :', ' sta (%s),y :' % p, ' iny :',
                ' lda op1+1 :', ' sta (%s),y :' % p, ' iny :',
                ' lda op2 :', ' sta (%s),y :' % p, ' iny :',
                ' lda op2+1 :', ' sta (%s),y :' % p]
    raise ValueError(mode)

# argument-name/signature helpers -------------------------------------------

def sig(mode, base):
    """(macro parameter list fragment, load_a/point_b arg) for one operand"""
    if mode == 'D':  return (['tmp%d' % base], 'tmp%d' % base)
    if mode == 'C':  return (['cte%d' % base], 'cte%d' % base)
    if mode == 'A':  return (['n%d' % base],   'n%d' % base)
    if mode == 'Y':  return (['ptr%d' % base, 'y%d' % base],
                             ('ptr%d' % base, 'y%d' % base))
    raise ValueError(mode)

out = []
def emit(name, params, body):
    out.append('#define %s(%s)\\' % (name, ','.join(params)))
    for ln in body:
        out.append('%s\\' % ln)
    out.append('')

# ---- binary families: NAME_<am><bm><rm> ------------------------------------

BIN = [('ADDL','ladd32'), ('SUBL','lsub32'), ('ANDL','land32'),
       ('ORL','lor32'),   ('XORL','lxor32'), ('MULL','lmul32'),
       ('DIVL','ldiv32i'),('DIVUL','ldiv32u'),
       ('MODL','lmod32i'),('MODUL','lmod32u'),
       ('LSHL','llsh32'), ('RSHL','lrshl32'), ('ASRL','lasr32')]
AM = ['C','D','Y']   # C needed first-position for the non-commutative ops
BM = ['A','C','D','Y']
RM = ['D','Y']

for name, routine in BIN:
    for am in AM:
        for bm in BM:
            for rm in RM:
                p1, a1 = sig(am, 1)
                p2, a2 = sig(bm, 2)
                p3, a3 = sig(rm, 3)
                body = (load_a(am, a1) + point_b(bm, a2)
                        + [' jsr %s :' % routine] + store_r(rm, a3))
                emit('%s_%c%c%c' % (name, am, bm, rm), p1+p2+p3, body)

# ---- compares: NAME_<am><bm>(a,b,label) ------------------------------------
# gen.c emits the NEGATED compare to branch over (same as the W families).
# lcmp32u/lcmp32i leave: C=1 <=> A>=B, Z=1 <=> A==B.

CMP = [
    ('EQL',  'lcmp32u', [' beq label :']),
    ('NEL',  'lcmp32u', [' bne label :']),
    ('LTL',  'lcmp32i', [' bcc label :']),
    ('GEL',  'lcmp32i', [' bcs label :']),
    ('LTUL', 'lcmp32u', [' bcc label :']),
    ('GEUL', 'lcmp32u', [' bcs label :']),
    # GT: C=1 and Z=0 ; LE: C=0 or Z=1  (beq *+4 skips the 2-byte bcs)
    ('GTL',  'lcmp32i', [' beq *+4 :', ' bcs label :']),
    ('LEL',  'lcmp32i', [' bcc label :', ' beq label :']),
    ('GTUL', 'lcmp32u', [' beq *+4 :', ' bcs label :']),
    ('LEUL', 'lcmp32u', [' bcc label :', ' beq label :']),
]
CAM = ['C','D','Y']   # LTI has C-first variants; provide them for all
CBM = ['A','C','D','Y']
for name, routine, branch in CMP:
    for am in CAM:
        for bm in CBM:
            p1, a1 = sig(am, 1)
            p2, a2 = sig(bm, 2)
            body = (load_a(am, a1) + point_b(bm, a2)
                    + [' jsr %s :' % routine] + branch)
            emit('%s_%c%c' % (name, am, bm), p1+p2+['label'], body)

# ---- INDIRL_<am><rm>: load 4 bytes through a pointer -----------------------
# am: D = pointer in zp var (load through it), Z = pointer IS the zp pair,
#     Y/A = pointer stored in frame/indirect slot, C = absolute address.

def load_ptr_to_tmp(mode, arg):
    if mode in ('D','Z'):
        return [' lda %s :' % arg, ' sta tmp :',
                ' lda %s+1 :' % arg, ' sta tmp+1 :']
    if mode == 'Y':
        p, y = arg
        return [' ldy #%s :' % y,
                ' lda (%s),y :' % p, ' sta tmp :', ' iny :',
                ' lda (%s),y :' % p, ' sta tmp+1 :']
    if mode == 'A':
        return [' ldy #%s :' % arg,
                ' lda (fp),y :', ' sta tmp :', ' iny :',
                ' lda (fp),y :', ' sta tmp+1 :']
    if mode == 'C':
        return [' lda #<(%s) :' % arg, ' sta tmp :',
                ' lda #>(%s) :' % arg, ' sta tmp+1 :']
    raise ValueError(mode)

def copy_tmp_deref_to(rm, arg):
    body = [' ldy #0 :']
    if rm == 'D':
        return body + [' lda (tmp),y :', ' sta %s :' % arg, ' iny :',
                       ' lda (tmp),y :', ' sta %s+1 :' % arg, ' iny :',
                       ' lda (tmp),y :', ' sta %s+2 :' % arg, ' iny :',
                       ' lda (tmp),y :', ' sta %s+3 :' % arg]
    if rm == 'Y':
        p, y = arg
        # go through op1:op2 so the destination index can differ
        return [' ldy #0 :',
                ' lda (tmp),y :', ' sta op1 :', ' iny :',
                ' lda (tmp),y :', ' sta op1+1 :', ' iny :',
                ' lda (tmp),y :', ' sta op2 :', ' iny :',
                ' lda (tmp),y :', ' sta op2+1 :'] + store_r('Y', (p, y))
    raise ValueError(rm)

for am in ['A','C','D','Y','Z']:
    for rm in ['D','Y']:
        p1, a1 = sig(am if am != 'Z' else 'D', 1)
        body = load_ptr_to_tmp(am, a1) + copy_tmp_deref_to(rm, sig(rm, 2)[1])
        emit('INDIRL_%c%c' % (am, rm), p1 + sig(rm, 2)[0], body)

# ---- ASGNL_<bm><am>(value,dest): store 4 bytes ------------------------------
# bm = value mode (A/C/D/Y), am = destination (D direct, Z through pointer)

for bm in ['A','C','D','Y']:
    for am in ['D','Z']:
        pv, av = sig(bm, 1)
        pd, ad = sig('D', 2)
        body = load_a(bm, av)
        if am == 'D':
            body = body + store_r('D', ad)
        else:  # through the zp pointer
            body = body + [' lda %s :' % ad, ' sta tmp :',
                           ' lda %s+1 :' % ad, ' sta tmp+1 :',
                           ' ldy #0 :',
                           ' lda op1 :', ' sta (tmp),y :', ' iny :',
                           ' lda op1+1 :', ' sta (tmp),y :', ' iny :',
                           ' lda op2 :', ' sta (tmp),y :', ' iny :',
                           ' lda op2+1 :', ' sta (tmp),y :']
        emit('ASGNL_%c%c' % (bm, am), pv + pd, body)

# ASGNL with Y destinations (store into (ptr),n)
for bm in ['A','C','D','Y']:
    pv, av = sig(bm, 1)
    p, y = sig('Y', 2)[1]
    body = load_a(bm, av) + store_r('Y', (p, y))
    emit('ASGNL_%cY' % bm, pv + sig('Y', 2)[0], body)

# ---- MOVL_<am><rm> ----------------------------------------------------------
for am in ['A','C','D','Y']:
    for rm in ['D','Y']:
        p1, a1 = sig(am, 1)
        p2, a2 = sig(rm, 2)
        body = load_a(am, a1) + store_r(rm, a2)
        emit('MOVL_%c%c' % (am, rm), p1 + p2, body)

# ---- conversions ------------------------------------------------------------
# CSWL/CZWL: word -> long (sign/zero extend). CLW: long -> word (low).
def load_w(mode, arg):
    if mode == 'D':
        return [' lda %s :' % arg, ' sta op1 :',
                ' lda %s+1 :' % arg, ' sta op1+1 :']
    if mode == 'Y':
        p, y = arg
        return [' ldy #%s :' % y,
                ' lda (%s),y :' % p, ' sta op1 :', ' iny :',
                ' lda (%s),y :' % p, ' sta op1+1 :']
    if mode == 'A':
        return [' ldy #%s :' % arg,
                ' lda (fp),y :', ' sta op1 :', ' iny :',
                ' lda (fp),y :', ' sta op1+1 :']
    if mode == 'C':
        return [' lda #<(%s) :' % arg, ' sta op1 :',
                ' lda #>(%s) :' % arg, ' sta op1+1 :']
    raise ValueError(mode)

for am in ['A','C','D','Y']:
    for rm in ['D','Y']:
        p1, a1 = sig(am, 1)
        p2, a2 = sig(rm, 2)
        # zero extend
        body = load_w(am, a1) + [' lda #0 :', ' sta op2 :', ' sta op2+1 :'] \
               + store_r(rm, a2)
        emit('CZWL_%c%c' % (am, rm), p1 + p2, body)
        # sign extend
        body = load_w(am, a1) + [' ldx #0 :', ' lda op1+1 :', ' bpl *+3 :',
                                 ' dex :', ' stx op2 :', ' stx op2+1 :'] \
               + store_r(rm, a2)
        emit('CSWL_%c%c' % (am, rm), p1 + p2, body)

# CLW: take the low word of a long
for am in ['D','Y','A']:
    for rm in ['D','Y']:
        p1, a1 = sig(am, 1)
        p2, a2 = sig(rm, 2)
        if am == 'D':
            src_lo, src_hi = [' lda %s :' % a1], [' lda %s+1 :' % a1]
        elif am == 'A':
            src = load_w('A', a1)
        else:
            src = load_w('Y', a1)
        if am == 'D' and rm == 'D':
            body = [' lda %s :' % a1, ' sta %s :' % a2,
                    ' lda %s+1 :' % a1, ' sta %s+1 :' % a2]
        else:
            body = load_w(am, a1)
            if rm == 'D':
                body += [' lda op1 :', ' sta %s :' % a2,
                         ' lda op1+1 :', ' sta %s+1 :' % a2]
            else:
                p, y = a2
                body += [' ldy #%s :' % y,
                         ' lda op1 :', ' sta (%s),y :' % p, ' iny :',
                         ' lda op1+1 :', ' sta (%s),y :' % p]
        emit('CLW_%c%c' % (am, rm), p1 + p2, body)

# ---- ARGL_<am>(a,(sp),off): 4 bytes into the outgoing arg area -------------
# (same convention as ARGW_D: the stack operand is passed as "(sp)" and used
#  with ,y indexing)
for am in ['A','C','D','Y']:
    p1, a1 = sig(am, 1)
    body = load_a(am, a1) + \
           [' ldy #off :',
            ' lda op1 :', ' sta ptr,y :', ' iny :',
            ' lda op1+1 :', ' sta ptr,y :', ' iny :',
            ' lda op2 :', ' sta ptr,y :', ' iny :',
            ' lda op2+1 :', ' sta ptr,y :']
    emit('ARGL_%c' % am, p1 + ['ptr','off'], body)

# ---- CALLL_<am><rm>(f,argbytes,result): 32-bit return = X:A low + op1 high --
# call mechanics reuse the proven CALLV_<am> macros (nested macro calls are
# supported - CALLW_DD already does exactly this)
for am in ['C','D','Y']:
    for rm in ['D','Y']:
        if am == 'C':
            call = [' ldy #nbparam :', ' jsr fn :']
            pf = ['fn']
        elif am == 'D':
            call = [' CALLV_D(fn,nbparam) ']
            pf = ['fn']
        else:
            p, y = sig('Y', 1)[1]
            call = [' CALLV_Y(%s,%s,nbparam) ' % (p, y)]
            pf = sig('Y', 1)[0]
        # after return: X = low byte, A = 2nd byte, op1:op1+1 = high word
        if rm == 'D':
            store = [' stx %s :' % 'res', ' sta %s+1 :' % 'res',
                     ' lda op1 :', ' sta %s+2 :' % 'res',
                     ' lda op1+1 :', ' sta %s+3 :' % 'res']
            pr = ['res']
        else:
            p, y = sig('Y', 3)[1]
            store = [' sta lscratch :', ' txa :', ' ldy #%s :' % y,
                     ' sta (%s),y :' % p, ' iny :', ' lda lscratch :',
                     ' sta (%s),y :' % p, ' iny :',
                     ' lda op1 :', ' sta (%s),y :' % p, ' iny :',
                     ' lda op1+1 :', ' sta (%s),y :' % p]
            pr = sig('Y', 3)[0]
        emit('CALLL_%c%c' % (am, rm), pf + ['nbparam'] + pr, call + store)

# ---- RETL_/LEAVEL_<am>(a): low word -> X:A, high word -> op1 ----------------
for kind, tail in [('RETL', [' rts :']), ('LEAVEL', [' jmp leave :'])]:
    for am in ['A','C','D','Y']:
        p1, a1 = sig(am, 1)
        if am == 'D':
            body = [' lda %s+2 :' % a1, ' sta op1 :',
                    ' lda %s+3 :' % a1, ' sta op1+1 :',
                    ' ldx %s :' % a1, ' lda %s+1 :' % a1]
        elif am == 'C':
            body = [' lda #<((%s)>>16) :' % a1, ' sta op1 :',
                    ' lda #>((%s)>>16) :' % a1, ' sta op1+1 :',
                    ' ldx #<(%s) :' % a1, ' lda #>(%s) :' % a1]
        elif am == 'A':
            body = [' ldy #%s+2 :' % a1,
                    ' lda (fp),y :', ' sta op1 :', ' iny :',
                    ' lda (fp),y :', ' sta op1+1 :',
                    ' ldy #%s :' % a1, ' lda (fp),y :', ' tax :', ' iny :',
                    ' lda (fp),y :']
        else:
            p, y = a1
            body = [' ldy #%s+2 :' % y,
                    ' lda (%s),y :' % p, ' sta op1 :', ' iny :',
                    ' lda (%s),y :' % p, ' sta op1+1 :',
                    ' ldy #%s :' % y, ' lda (%s),y :' % p, ' tax :', ' iny :',
                    ' lda (%s),y :' % p]
        emit('%s_%c' % (kind, am), p1, body + tail)

open(OUT, 'w').write('\n'.join(out) + '\n')
print('wrote %s: %d macros, %d lines' % (OUT, len([l for l in out if l.startswith('#define')]), len(out)))
