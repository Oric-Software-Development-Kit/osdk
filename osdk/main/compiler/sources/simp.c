/* C compiler: tree simplification and constant folding */

#include "c.h"
#include <stdio.h>
#include <assert.h>

int needconst;		/* >=1 if parsing a constant expression */

dclproto(static int add,(double, double, double, double, int));
dclproto(static Tree addrnode,(Symbol, int, Type));
dclproto(static int divide,(double, double, double, double, int));
dclproto(static int mul,(double, double, double, double, int));
dclproto(static int sub,(double, double, double, double, int));

/* add - return 1 if min <= x+y <= max, 0 otherwise */
static int add(x, y, min, max, needconst) double x, y, min, max;  int needconst; {
	int cond =
        x == 0
    ||  y == 0
	|| (x < 0 && y < 0 && x >= min - y)
	|| (x < 0 && y > 0)
	|| (x > 0 && y < 0)
	|| (x > 0 && y > 0 && x <= max - y);
	if (!cond && needconst) {
		warning("overflow in constant expression\n");
		cond = 1;
	}
	return cond;
}

/* addrnode - create a tree for addressing expression p+n, type ty */
static Tree addrnode(p, n, ty) Symbol p; int n; Type ty; {
	Symbol q = (Symbol)talloc(sizeof *q);
	Tree e;
	static struct symbol z;

	*q = z;
	q->name = stringd(genlabel(1));
	q->sclass = p->sclass;
	q->scope = p->scope;
	q->type = ty;
	q->defined = 1;
	q->temporary = p->temporary;
	q->generated = p->generated;
	q->computed = 1;
	q->addressed = p->addressed;
	q->initialized = p->initialized;
	q->ref = 1;
	if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN) {
		q->sclass = p->sclass == AUTO ? STATIC : p->sclass;
		address(q, p, n);
		e = tree(ADDRG+P, ty, 0, 0);
	} else {
		Code cp;
		if (!p->defined)
			addlocal(p);
		cp = code(Address);
		cp->u.addr.sym = q;
		cp->u.addr.base = p;
		cp->u.addr.offset = n;
		e = tree(p->scope == PARAM ? ADDRF+P : ADDRL+P, ty, 0, 0);
	}
	e->u.sym = q;
	return e;
}

/* div - return 1 if min <= x/y <= max, 0 otherwise */
static int divide(x, y, min, max, needconst) double x, y, min, max; int needconst; {
	int cond;

	if (x < 0) x = -x;
	if (y < 0) y = -y;
	cond = y != 0 && (y > 1 || x <= max*y);
	if (!cond && y != 0 && needconst) {
		warning("overflow in constant expression\n");
		cond = 1;
	}
	return cond;
}

/* ispow2 - if u > 1 && u == 2^n, return n, otherwise return 0 */
int ispow2(u) unsigned u; {
	int n;

	if (u > 1 && (u&(u-1)) == 0)
		for (n = 0; u; u >>= 1, n++)
			if (u&1)
				return n;
	return 0;
}

/* mul - return 1 if min <= x*y <= max, 0 otherwise */
static int mul(x, y, min, max, needconst) double x, y, min, max; int needconst; {
	int cond =
	   (x > -1 && x <= 1)
    || (y > -1 && y <= 1)
	|| (x < 0 && y < 0 && -x <= max/-y)
	|| (x < 0 && y > 0 &&  x >= min/y)
	|| (x > 0 && y < 0 &&  x >= min/y)
	|| (x > 0 && y > 0 &&  x <= max/y);
	if (!cond && needconst) {
		warning("overflow in constant expression\n");
		cond = 1;
	}
	return cond;

/* sub - return 1 if min <= x-y <= max, 0 otherwise */
}

/* The target int/unsigned are 16 bits, but constant folding runs in HOST
 * arithmetic (32 or more bits): every folded result must be wrapped to the
 * target width, and every range guard must use the TARGET limits, or folded
 * constants disagree with the same expression computed at run time (cc65
 * regression bug2461: 0xFF50 + 0x100 folded to 0x10050 instead of 0x0050).
 * Signed overflow is undefined behavior: signed folds wrap exactly like the
 * run time macros do, keeping folded and computed results identical (only
 * refusing to fold is NOT an option - it would leave constant-constant
 * operand pairs behind, for which the macro library has no variants). */
#define TGT_INT_MIN  (-32767-1)
#define TGT_INT_MAX  32767
#define TGT_SHRT_MIN (-128)         /* the OSDK target short is 8 bits */
#define TGT_SHRT_MAX 127
#define tgtwrapu(x)  ((unsigned)((x) & 0xFFFFu))
#define tgtwrapi(x)  ((int)(short)(unsigned short)((x) & 0xFFFFu))
/* width-aware variants: long (4-byte) results fold at 32 bits */
#define tgtwrapu2(x,rty)  ((rty)->size==4 ? (unsigned)(x) : tgtwrapu(x))
#define tgtwrapi2(x,rty)  ((rty)->size==4 ? (int)(x) : tgtwrapi(x))

#define xcvtcnst(FTYPE,TTYPE,EXP,VAR,MIN,MAX) \
	if (l->op == CNST+FTYPE) { \
		if (needconst && (VAR < MIN || VAR > MAX)) \
			warning("overflow in constant expression\n"); \
		if (needconst || VAR >= MIN && VAR <= MAX) { \
			p = tree(CNST+ttob(TTYPE), TTYPE, 0, 0); EXP; return p; } }
#define cvtcnst(FTYPE,TTYPE,EXP) \
	if (l->op == CNST+FTYPE) { \
		p = tree(CNST+ttob(TTYPE), TTYPE, 0, 0); EXP; return p; }
#define commute(LT,RT) \
	if (generic(RT->op) == CNST && generic(LT->op) != CNST) { \
		Tree t = LT; LT = RT; RT = t; }
#define zerofield(OP,TYPE,VAR) \
	if (l->op == FIELD && r->op == CNST+TYPE && r->u.v.VAR == 0) \
		return eqnode(OP, bitnode(BAND, l->kids[0], \
			constnode(fieldmask(l->u.field)<<fieldright(l->u.field), \
				unsignedtype)), r);
#define ufoldcnst(TYPE,EXP) if (l->op == CNST+TYPE) return EXP
#define xfoldcnst(TYPE,VAR,OP,RTYPE,FUNC,MIN,MAX) \
	if (l->op == CNST+TYPE && r->op == CNST+TYPE \
	&& FUNC((double)l->u.v.VAR,(double)r->u.v.VAR,(double)MIN,(double)MAX, needconst)) { \
		p = tree(CNST+ttob(RTYPE), RTYPE, 0, 0); \
		p->u.v.VAR = l->u.v.VAR OP r->u.v.VAR; return p; }
#define foldcnst(TYPE,VAR,OP,RTYPE) \
	if (l->op == CNST+TYPE && r->op == CNST+TYPE) { \
		p = tree(CNST+ttob(RTYPE), RTYPE, 0, 0); \
		p->u.v.VAR = l->u.v.VAR OP r->u.v.VAR; return p; }
/* like foldcnst but wraps the result to the 16 bit target width (for the
   unsigned operations whose wrap-around is well defined behavior) */
#define wfoldcnst(TYPE,VAR,OP,RTYPE) \
	if (l->op == CNST+TYPE && r->op == CNST+TYPE) { \
		p = tree(CNST+ttob(RTYPE), RTYPE, 0, 0); \
		p->u.v.VAR = tgtwrapu2(l->u.v.VAR OP r->u.v.VAR, RTYPE); return p; }
/* signed fold: overflow is undefined behavior, so fold-and-wrap exactly like
   the run time macros would (refusing to fold would leave constant-constant
   operand pairs the macro library has no variants for). FUNC only provides
   the "overflow in constant expression" warning under needconst. */
#define wxfoldcnst(TYPE,VAR,OP,RTYPE,FUNC) \
	if (l->op == CNST+TYPE && r->op == CNST+TYPE) { \
		FUNC((double)l->u.v.VAR,(double)r->u.v.VAR,(double)TGT_INT_MIN,(double)TGT_INT_MAX, needconst); \
		p = tree(CNST+ttob(RTYPE), RTYPE, 0, 0); \
		p->u.v.VAR = tgtwrapi2(l->u.v.VAR OP r->u.v.VAR, RTYPE); return p; }
#define cfoldcnst(TYPE,VAR,OP,RTYPE) \
	if (l->op == CNST+TYPE && r->op == CNST+TYPE) { \
		p = tree(CNST+ttob(RTYPE), RTYPE, 0, 0); \
		p->u.v.i = l->u.v.VAR OP r->u.v.VAR; return p; }
#define sfoldcnst(TYPE,VAR,OP,RTYPE) \
	if (l->op == CNST+TYPE && r->op == CNST+I \
	&& r->u.v.i >= 0 && r->u.v.i < 8*l->type->size) { \
		p = tree(CNST+ttob(RTYPE), RTYPE, 0, 0); \
		p->u.v.VAR = tgtwrapu2(l->u.v.VAR OP r->u.v.i, RTYPE); return p; }
#define foldaddp(LT,RT,RTYPE,VAR) \
	if (LT->op == CNST+P && RT->op == CNST+RTYPE) { \
		p = tree(CNST+P, ty, 0, 0); \
		p->u.v.p = LT->u.v.p + RT->u.v.VAR; return p; }
#define geu(LT,RT,V) \
	if (RT->op == CNST+U && RT->u.v.u == 0) { \
		warning("result of unsigned comparison is constant\n"); \
		return tree(RIGHT, inttype, root(LT), constnode(V, inttype)); }
#define idempotent(OP) if (l->op == OP) return l->kids[0];
#define identity(X,Y,TYPE,VAR,VAL) if (X->op == CNST+TYPE && X->u.v.VAR == VAL) return Y

/* simplify - build node for op, simplifying and folding constants, if possible */
Tree simplify(op, ty, l, r) int op; Type ty; Tree l, r; {
	int n;
	Tree p;

	if (optype(op) == 0)
		op += ttob(ty);
	switch (op) {
	case ADD+D:
		xfoldcnst(D,d,+,doubletype,add,-DBL_MAX,DBL_MAX);
		commute(r,l);
		break;
	case ADD+F:
		xfoldcnst(F,f,+,floattype,add,-FLT_MAX,FLT_MAX);
		commute(r,l);
		break;
	case ADD+I:
		wxfoldcnst(I,i,+,ty,add);
		commute(r,l);
		break;
	case ADD+P:
		foldaddp(l,r,I,i);
		foldaddp(l,r,U,u);
		foldaddp(r,l,I,i);
		foldaddp(r,l,U,u);
		commute(r,l);
		identity(r,retype(l,ty),I,i,0);
		identity(r,retype(l,ty),U,u,0);
		if (isaddrop(l->op) && (r->op == CNST+I || r->op == CNST+U))
			/* l + c => l+c, where l is a symbolic address */
			return addrnode(l->u.sym, r->op == CNST+I ? r->u.v.i : r->u.v.u, ty);
		if ((l->op == ADD+I || l->op == SUB+I)
		&& l->kids[1]->op == CNST+I && isaddrop(r->op))
			/* (x +- c) + r => x + r+-c, where r is a symbolic address */
			return simplify(ADD+P, ty, l->kids[0],
				simplify(l->op == ADD+I ? ADD+P : SUB+P, ty, r, l->kids[1]));
		if (l->op == ADD+P && isaddrop(l->kids[1]->op)
		&& (r->op == CNST+I || r->op == CNST+U))
			/* (x + a) + r => x + a+r */
			return simplify(ADD+P, ty, l->kids[0],
				addrnode(l->kids[1]->u.sym, r->op == CNST+I ? r->u.v.i : r->u.v.u, ty));
		if (l->op == ADD+P && generic(l->kids[1]->op) == CNST && generic(r->op) == CNST)
			/* (x + c) + r => x + c+r */
			return simplify(ADD+P, ty, l->kids[0], (*opnode['+'])(ADD, l->kids[1], r));
		if (l->op == ADD+I && generic(l->kids[1]->op) == CNST
		&&  r->op == ADD+P && generic(r->kids[1]->op) == CNST)
			/* (x + c1) + (y + c2) => x + (y + c1+c2) */
			return simplify(ADD+P, ty, l->kids[0],
				simplify(ADD+P, ty, r->kids[0],
				(*opnode['+'])(ADD, l->kids[1], r->kids[1])));
		if (l->op == RIGHT && isstruct(l->type))	/* f().x */
			return tree(RIGHT, ty, l->kids[0],
				simplify(ADD+P, ty, l->kids[1], r));
		if (l->op == RIGHT) {
			if (l->kids[1])
				return tree(RIGHT, ty, l->kids[0],
					simplify(ADD+P, ty, l->kids[1], r));
			else
				return tree(RIGHT, ty,
					simplify(ADD+P, ty, l->kids[0], r), 0);
		}
		break;
	case ADD+U:
		wfoldcnst(U,u,+,ty);
		commute(r,l);
		break;
	case AND+I:
		op = AND;
		ufoldcnst(I,l->u.v.i ? cond(r) : l);	/* 0&&r => 0, 1&&r => r */
		break;
	case OR+I:
		op = OR;
		/* 0||r => r, 1||r => 1 */
		ufoldcnst(I,l->u.v.i ? constnode(1, inttype) : cond(r));
		break;
	case BAND+U:
		foldcnst(U,u,&,ty);
		commute(r,l);
		identity(r,l,U,u,0xFFFFu);
		if (r->op == CNST+U && r->u.v.u == 0)	/* l&0 => (l,0) */
			return tree(RIGHT, unsignedtype, root(l),
				constnode(0, unsignedtype));
		break;
	case BCOM+I:
		ufoldcnst(I,constnode(tgtwrapi2(~l->u.v.i, ty), ty));
		idempotent(BCOM+U);
		op = BCOM+U;
		break;
	case BCOM+U:
		ufoldcnst(U,constnode(tgtwrapu2(~l->u.v.u, ty), ty));
		idempotent(BCOM+U);
		break;
	case BOR+U:
		foldcnst(U,u,|,ty);
		commute(r,l);
		identity(r,l,U,u,0);
		break;
	case BXOR+U:
		foldcnst(U,u,^,ty);
		commute(r,l);
		identity(r,l,U,u,0);
		break;
	case CVC+I:  cvtcnst(C,      inttype,p->u.v.i  = (l->u.v.sc&0200 ? (~0<<8) : 0)|(l->u.v.sc&0377)); break;
	case CVC+U:  cvtcnst(C, unsignedtype,p->u.v.u  = l->u.v.uc); break;
	case CVD+F: xcvtcnst(D,    floattype,p->u.v.f  = l->u.v.d,l->u.v.d,-FLT_MAX,FLT_MAX); break;
	case CVD+I: xcvtcnst(D,      inttype,p->u.v.i  = l->u.v.d,l->u.v.d,TGT_INT_MIN,TGT_INT_MAX); break;
	case CVF+D:  cvtcnst(F,   doubletype,p->u.v.d  = l->u.v.f);  break;
	case CVI+C: xcvtcnst(I,     chartype,p->u.v.sc = l->u.v.i,l->u.v.i,SCHAR_MIN,SCHAR_MAX); break;
	case CVI+D:  cvtcnst(I,   doubletype,p->u.v.d  = l->u.v.i);  break;
	case CVI+S: xcvtcnst(I,    shorttype,p->u.v.ss = l->u.v.i,l->u.v.i,TGT_SHRT_MIN,TGT_SHRT_MAX); break;
	case CVI+U:  cvtcnst(I, unsignedtype,p->u.v.u  = tgtwrapu(l->u.v.i));  break;
	/* 32-bit long conversions: host int/unsigned are >= 32 bits, so the
	   values are simply carried (extend) or masked (truncate) */
	case CVI+L:  cvtcnst(I, ty, p->u.v.i = l->u.v.i);  break;
	case CVU+L:  cvtcnst(U, ty, p->u.v.i = (int)l->u.v.u);  break;
	case CVL+I:  cvtcnst(I, ty, p->u.v.i = tgtwrapi(l->u.v.i));  break;
	case CVL+U:
		cvtcnst(I, ty, p->u.v.u = tgtwrapu(l->u.v.i));
		cvtcnst(U, ty, p->u.v.u = tgtwrapu(l->u.v.u));
		break;
	case CVL+D:
		cvtcnst(I, doubletype, p->u.v.d = l->u.v.i);
		cvtcnst(U, doubletype, p->u.v.d = utod(l->u.v.u));
		break;
	case CVP+U:  cvtcnst(P, unsignedtype,p->u.v.u  = tgtwrapu((unsigned)l->u.v.p)); break;
	case CVS+I:  cvtcnst(S,      inttype,p->u.v.i  = l->u.v.ss); break;
	case CVS+U:  cvtcnst(S, unsignedtype,p->u.v.u  = l->u.v.us); break;
	case CVU+C:  cvtcnst(U, unsignedchar,p->u.v.uc = l->u.v.u);  break;
	case CVU+D:  cvtcnst(U,   doubletype,p->u.v.d  = utod(l->u.v.u));  break;
	case CVU+I:
		if (needconst && l->u.v.u > TGT_INT_MAX)
			warning("overflow in constant expression\n");
		cvtcnst(U,   inttype,p->u.v.i  = tgtwrapi(l->u.v.u));
		break;
	case CVU+P:  cvtcnst(U,    voidptype,p->u.v.p  = (char *)l->u.v.u);  break;
	case CVU+S:  cvtcnst(U,unsignedshort,p->u.v.us = l->u.v.u);  break;
	case DIV+D:
		xfoldcnst(D,d,/,doubletype, divide,-DBL_MAX,DBL_MAX);
		break;
	case DIV+F:
		xfoldcnst(F,f,/,floattype, divide,-FLT_MAX,FLT_MAX);
		break;
	case DIV+I:
		identity(r,l,I,i,1);
#ifdef mips
		if (l->op == CNST+I && r->op == CNST+I && r->u.v.i == -1
		&& !div((double)l->u.v.i, (double)r->u.v.i, (double)INT_MIN, (double)INT_MAX, 0))
			break;
#endif
		xfoldcnst(I,i,/,ty, divide,TGT_INT_MIN,TGT_INT_MAX);
		break;
	case DIV+U:
		identity(r,l,U,u,1);
		if (r->op == CNST+U && r->u.v.u == 0)
			break;
		if (r->op == CNST+U && (n = ispow2(r->u.v.u)))
			return simplify(RSH+U, ty, l, constnode(n, inttype));
		foldcnst(U,u,/,ty);
		break;
	case EQ+D:
		cfoldcnst(D,d,==,inttype);
		commute(r,l);
		break;
	case EQ+F:
		cfoldcnst(F,f,==,inttype);
		commute(r,l);
		break;
	case EQ+I:
		cfoldcnst(I,i,==,inttype);
		commute(r,l);
		zerofield(EQ,I,i);
		break;
	case EQ+U:
		cfoldcnst(U,u,==,inttype);
		commute(r,l);
		zerofield(EQ,U,u);
		op = EQ+I;
		break;
	case GE+D: cfoldcnst(D,d,>=,inttype); break;
	case GE+F: cfoldcnst(F,f,>=,inttype); break;
	case GE+I: cfoldcnst(I,i,>=,inttype); break;
	case GE+U:
		geu(l,r,1);	/* l >= 0 => (l,1) */
		cfoldcnst(U,u,>=,inttype);
		if (l->op == CNST+U && l->u.v.u == 0)	/* 0 >= r => 0 == r */
			return tree(EQ+I, ty, l, r);
		break;
	case GT+D: cfoldcnst(D,d, >,inttype); break;
	case GT+F: cfoldcnst(F,f, >,inttype); break;
	case GT+I: cfoldcnst(I,i, >,inttype); break;
	case GT+U:
		geu(r,l,0);	/* 0 > r => (r,0) */
		cfoldcnst(U,u, >,inttype);
		if (r->op == CNST+U && r->u.v.u == 0)	/* l > 0 => l != 0 */
			return tree(NE+I, ty, l, r);
		break;
	case LE+D: cfoldcnst(D,d,<=,inttype); break;
	case LE+F: cfoldcnst(F,f,<=,inttype); break;
	case LE+I: cfoldcnst(I,i,<=,inttype); break;
	case LE+U:
		geu(r,l,1);	/* 0 <= r => (r,1) */
		cfoldcnst(U,u,<=,inttype);
		if (r->op == CNST+U && r->u.v.u == 0)	/* l <= 0 => l == 0 */
			return tree(EQ+I, ty, l, r);
		break;
	case LSH+I:
		identity(r,l,I,i,0);
		if (l->op == CNST+I && r->op == CNST+I
		&& r->u.v.i >= 0 && r->u.v.i < 8*l->type->size) {
			/* warn under needconst, then fold-and-wrap like the runtime */
			mul((double)l->u.v.i, (double)(1<<r->u.v.i), (double)TGT_INT_MIN, (double)TGT_INT_MAX, needconst);
			return constnode(tgtwrapi2(l->u.v.i<<r->u.v.i, ty), ty);
		}
		break;
	case LSH+U:
		identity(r,l,I,i,0);
		sfoldcnst(U,u,<<,ty);
		break;
	case LT+D: cfoldcnst(D,d, <,inttype); break;
	case LT+F: cfoldcnst(F,f, <,inttype); break;
	case LT+I: cfoldcnst(I,i, <,inttype); break;
	case LT+U:
		geu(l,r,0);	/* l < 0 => (l,0) */
		cfoldcnst(U,u, <,inttype);
		if (l->op == CNST+U && l->u.v.u == 0)	/* 0 < r => 0 != r */
			return tree(NE+I, ty, l, r);
		break;
	case MOD+I:
		if (r->op == CNST+I && r->u.v.i == 1)	/* l%1 => (l,0) */
			return tree(RIGHT, inttype, root(l), constnode(0, inttype));
		if (r->op == CNST+I && r->u.v.i == 0)
			break;
#ifdef mips
		if (l->op == CNST+I && r->op == CNST+I && r->u.v.i == -1
		&& !divide((double)l->u.v.i, (double)r->u.v.i, (double)INT_MIN, (double)INT_MAX, 0))
			break;
#endif
		xfoldcnst(I,i,%,ty, divide,TGT_INT_MIN,TGT_INT_MAX);
		break;
	case MOD+U:
		if (r->op == CNST+U && ispow2(r->u.v.u))	/* l%2^n => l&(2^n-1) */
			return bitnode(BAND, l,
				constnode(r->u.v.u - 1, ty));
		if (r->op == CNST+U && r->u.v.u == 0)
			break;
		foldcnst(U,u,%,ty);
		break;
	case MUL+D:
		xfoldcnst(D,d,*,doubletype,mul,-DBL_MAX,DBL_MAX);
		commute(l,r);
		break;
	case MUL+F:
		xfoldcnst(F,f,*,floattype,mul,-FLT_MAX,FLT_MAX);
		commute(l,r);
		break;
	case MUL+I:
		commute(l,r);
		if (l->op == CNST+I && r->op == ADD+I && r->kids[1]->op == CNST+I)
			/* c1*(x + c2) => c1*x + c1*c2 */
			return simplify(ADD+I, inttype, simplify(MUL+I, inttype, l, r->kids[0]),
				simplify(MUL+I, inttype, l, r->kids[1]));
		if (l->op == CNST+I && r->op == SUB+I && r->kids[1]->op == CNST+I)
			/* c1*(x - c2) => c1*x - c1*c2 */
			return simplify(SUB+I, inttype, simplify(MUL+I, inttype, l, r->kids[0]),
				simplify(MUL+I, inttype, l, r->kids[1]));
		if (l->op == CNST+I && l->u.v.i > 0 && (n = ispow2(l->u.v.i)))
			/* 2^n * r => r<<n */
			return simplify(LSH+I, ty, r, constnode(n, inttype));
		wxfoldcnst(I,i,*,ty,mul);
		break;
	case MUL+U:
		commute(l,r);
		if (l->op == CNST+U && (n = ispow2(l->u.v.u)))
			/* 2^n * r => r<<n */
			return simplify(LSH+U, ty, r, constnode(n, inttype));
		wfoldcnst(U,u,*,ty);
		break;
	case NE+D:
		foldcnst(D,d,!=,inttype);
		commute(r,l);
		break;
	case NE+F:
		cfoldcnst(F,f,!=,inttype);
		commute(r,l);
		break;
	case NE+I:
		cfoldcnst(I,i,!=,inttype);
		commute(r,l);
		zerofield(NE,I,i);
		break;
	case NE+U:
		cfoldcnst(U,u,!=,inttype);
		commute(r,l);
		zerofield(NE,U,u);
		op = NE+I;
		break;
	case NEG+D:
		ufoldcnst(D,(p = tree(CNST+D, doubletype, 0, 0), p->u.v.d = -l->u.v.d, p));
		idempotent(NEG+D);
		break;
	case NEG+F:
		ufoldcnst(F,(p = tree(CNST+F, floattype, 0, 0), p->u.v.f = -l->u.v.f, p));
		idempotent(NEG+F);
		break;
	case NEG+I:
		if (l->op == CNST+I) {
			if (needconst && l->u.v.i == TGT_INT_MIN)
				warning("overflow in constant expression\n");
			if (needconst || l->u.v.i != TGT_INT_MIN)
				return constnode(-l->u.v.i, inttype);
		}
		idempotent(NEG+I);
		break;
 	case NOT+I:
		op = NOT;
		ufoldcnst(I,constnode(!l->u.v.i, inttype));
		break;
	case RSH+I:
		identity(r,l,I,i,0);
		if (l->op == CNST+I && r->op == CNST+I
		&& r->u.v.i >= 0 && r->u.v.i < 8*l->type->size) {
			int n = l->u.v.i>>r->u.v.i;
			if (l->u.v.i < 0)
				n |= ~0<<(8*l->type->size - r->u.v.i);
			return constnode(n, ty);
		}
		break;
	case RSH+U:
		identity(r,l,I,i,0);
		sfoldcnst(U,u,>>,ty);
		break;
	case SUB+D:
		xfoldcnst(D,d,-,doubletype,sub,-DBL_MAX,DBL_MAX);
		break;
	case SUB+F:
		xfoldcnst(F,f,-,floattype,sub,-FLT_MAX,FLT_MAX);
		break;
	case SUB+I:
		wxfoldcnst(I,i,-,ty,sub);
		break;
	case SUB+U:
		wfoldcnst(U,u,-,ty);
		break;
	case SUB+P:
		if (l->op == CNST+P && r->op == CNST+P)
			return constnode(l->u.v.p - r->u.v.p, inttype);
		if (r->op == CNST+I || r->op == CNST+U)
			return simplify(ADD+P, ty, l,
				constnode(r->op == CNST+I ? -r->u.v.i : -tgtwrapi(r->u.v.u), inttype));
		if (isaddrop(l->op) && r->op == ADD+I && r->kids[1]->op == CNST+I)
			/* l - (x + c) => l-c - x */
			return simplify(SUB+P, ty,
				simplify(SUB+P, ty, l, r->kids[1]), r->kids[0]);
		break;
	default:assert(0);
	}
	return tree(op, ty, l, r);
}

static int sub(x, y, min, max, needconst) double x, y, min, max; int needconst; {
	return add(x, -y, min, max, needconst);
}

/* vtoa - return string for the constant v of type ty */
char *vtoa(ty, v) Type ty; Value v; {
	char buf[50];

	ty = unqual(ty);
	switch (ty->op) {
	case CHAR:
		return stringf("%d", v.uc);
	case SHORT:
		return stringf("%d", v.ss);
	case INT:
		return stringf("%d", v.i);
	case UNSIGNED:
		if ((v.u&~0x7fff) == 0)
			return stringf("%d", v.u);
		else
			return stringf("0x%x", v.u);
	case FLOAT:
		if (v.f == 0.0)
			return "0";
		sprintf(buf, "%.*g", 8, v.f);
		return string(buf);
	case DOUBLE:
		if (v.d == 0.0)
			return "0";
		sprintf(buf, "%.*g", 18, v.d);
		return string(buf);
	case ARRAY:
		if (ty->type->op == CHAR)
			return v.p;
		/* else fall thru */
	case POINTER: case FUNCTION:
		if (((unsigned)v.p&~0x7fff) == 0)
			return stringf("%d", v.p);
		else
			return stringf("0x%x", v.p);
	default:assert(0);
	}
	return 0;
}
