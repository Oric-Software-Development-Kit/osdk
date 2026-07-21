/* C compiler: configuration parameters for 6502 generator */

#define R6502

/* type metrics: size,alignment,constants */
#define CHAR_METRICS     1,1,0
#define SHORT_METRICS    1,1,0
#define INT_METRICS      2,1,0
#define LONG_METRICS     4,1,0
#define FLOAT_METRICS    5,1,1
#define DOUBLE_METRICS   5,1,1
#define POINTER_METRICS  2,1,0
#define STRUCT_ALIGN     1

#define LEFT_TO_RIGHT	 /* evaluate args left-to-right */
#define LITTLE_ENDIAN	 /* right-to-left bit fields */
#define JUMP_ON_RETURN	0

typedef struct {
	int offset;	/* max offset of locals in current sub-block */
} Env;

enum symboltype { UNKNOWN, GLOBALVAR, LOCALVAR, TEMPORARY, PARAMETER, ARGBUILD};

typedef struct {
	char	*name;		/* node's result external representation */
	char	adrmode;	/* addressing mode of the result */
	Symbol	result;		/* operator's result */
	int		argoffset;	/* offset pour ARG et CALL */
    unsigned int busy;  /* busy state for CALL */
	Node	next;		/* next node on linearized list */
	char	optimized;
	char	visited;
	char	borrowed;	/* folded INDIR owns (reserved) the temp holding its address */
	char	narrow;		/* 8-bit narrowing: on an arith node -> emit the byte (B)
				   family; on a CVCU/CVSU widen -> skip it (the result is
				   only used as a char, so the high byte is irrelevant) */
	char	width;		/* value width in bytes (0/2 = word, 4 = 32-bit long).
				   dag nodes carry no Type, and long ops share the I/U
				   opcodes, so this is the only channel telling the
				   backend a node computes a 32-bit value. Stamped at
				   dag construction from the tree's type; compare nodes
				   read their KIDS' width (their own type is int). */
	char	inplace;	/* in-place long RMW: set by mark_inplace_rmw on the
				   ASGN/ADD-or-SUB/INDIR triple of a `mem_long op= const`
				   so the store emits one ADDLK/SUBLK macro and the load
				   and arithmetic nodes emit nothing. Survives tmpalloc
				   (which only clears optimized/borrowed). */
	char	fusek;		/* fused const add/sub: set by mark_fuse_addk on an
				   ADD/SUB node whose result is `mem_long +/- const`
				   into a temp - it emits one ADDLKM/SUBLKM macro that
				   reads the source memory and writes the result temp
				   directly (its INDIR operand is marked inplace to
				   suppress its load). Survives tmpalloc. */
	char	fastreg;	/* __fastcall register arg. On an ARG node: 1 = word
				   arg (A:X low:high), 2 = byte arg (A only) - width
				   taken from the callee prototype, not the promoted
				   arg. Its own emit is suppressed; the value is loaded
				   at the CALL after save_busy. On a CALL node: nonzero
				   = calls a __fastcall function. Set by
				   mark_fastcall_args; survives tmpalloc. */
	Node	fastargs;	/* __fastcall CALL: the (v1 single) fastcall ARG node
				   whose value is loaded into A/X/Y just before the
				   jsr, or 0 for a 0-argument fastcall. */
} Xnode;

typedef struct {
	char	*name;			/* name for back end */
	char	adrmode;		/* addressing mode */
} Xsymbol;

#define stabblock(a,b,c)
#define stabend(a,b,c,d,e)
#define stabfend(a,b)
#define stabinit(a,b,c)
#define stabline(a) do { if ((a)->file) print(".csource \"%s\" %d\n", (a)->file, (a)->y); } while(0)
/* .ctype annotation functions (implemented in gen.c) */
extern void emit_stabsym(Symbol);
extern void emit_stabtype(Symbol);
#define stabsym(a)  emit_stabsym(a)
#define stabtype(a) emit_stabtype(a)
