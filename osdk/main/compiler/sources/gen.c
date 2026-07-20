/* C compiler: 16 bits 6502 code generator */
/* (C) Fabrice Frances 1997-2019 */
/*
 * Implementation notes:
 *
 * The general structure of this backend follows the interfacing guidelines of
 * Hanson & Fraser's Retargetable C Compiler : a number of specified functions
 * have to be implemented by every backend, and the frontend and backend
 * communicate information through a few structures (most important ones being
 * the symbol structure and the node structure).
 *
 * Each backend defines specific extensions of these symbol and node structures
 * in config.h (Xnode and Xsymbol), along with type metrics and a few other
 * parameters.
 *
 * This 6502 backend aims to be kept as simple as possible by introducing an
 * intermediate language of macros, so that most of the time, there's a one to
 * one correspondence between the Operator Nodes passed by the frontend and
 * the macros emitted by the backend. However, lcc's frontend was designed
 * for working straight forward with backends for RISC processors with
 * a load/store architecture and many registers so that operations only take
 * register operands. The 6502 instead has a single register that can be used
 * in operations whilst the second operand is in memory. So, I designed this
 * 6502 backend to use pseudo-registers in zero-page (this seems natural),
 * but also to propagate addressing-mode information in order to exploit
 * some 6502 addressing modes. This way it becomes possible to optimize the
 * generated code by combining the load/store operations provided by the
 * frontend with the operations on pseudo-registers and/or memory.
 */


#include "infos.h"

#define _COMP_XSTR(s) _COMP_STR(s)
#define _COMP_STR(s)  #s

char *version="/* 16-bit code V" _COMP_XSTR(TOOL_VERSION_MAJOR) "." _COMP_XSTR(TOOL_VERSION_MINOR) " */\n";
#include "c.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>    // atoi()
#include <stdbool.h>   // Not available on VS2010, but available on VS2019
extern void exit(int);

static bool graph_output;    /* output forest of dags from frontend */
static int localsize;        /* max size of locals */
static int argoffset;        /* current stack position */
static int offset;           /* current local size */
static int tmpsize;          /* max size of temporary variables */
static int nbregs;           /* number of used registers */
static unsigned busy;        /* busy&(1<<t) == 1 if tmp t is used */
static unsigned busy_flt;    /* busy_flt&(1<<t) == 1 if tmp t is used */
static char *NamePrefix;     /* Prefix for all local names */
static int omit_frame;       /* if no params and no locals */
static int optimizelevel=3;  /* set by command line option -On */
static int optimize_for_size=0; /* -Os: size axis. Disables (future) transforms that
                                 * trade size for speed. No such transform exists yet, so
                                 * -Os currently produces identical codegen to -O3 (which
                                 * is the smallest level - its folds are all size wins). */
static int optstack[16];     /* saved levels for #pragma optimize(push,n) */
static int optsp;            /* optstack stack pointer */
static Symbol temp[32];      /* 32 symbols pointing to temporary variables... */
static Symbol flt_temp[32];  /* 32 symbols pointing to temporary floating-point variables... */
static char *regname[8];     /* 8 register variables names */

static char *opcode_names[] = {
    NULL,"CNST","ARG","ASGN","INDIR","CVC","CVD","CVF","CVI","CVP",
    "CVS","CVU","NEG","CALL","LOAD","RET","ADDRG","ADDRF","ADDRL","ADD",
    "SUB","LSH","MOD","RSH","BAND","BCOM","BOR","BXOR","DIV","MUL",
    "EQ","GE","GT","LE","LT","NE","JUMP","LABEL","CVL","MAXOP" };
static char type_name[] = " FDCSIUPVBL?????";
static char *additional_operators[] = {
    "AND","NOT","OR","COND","RIGHT","FIELD" };

void print_node(Node p) {
    fprintf(stderr,"Node %s%c\n",
            opcode_names[generic(p->op)>>4], type_name[optype(p->op)]);
    fprintf(stderr,"Node addr: %p\n", p);
    fprintf(stderr,"  optimized: %d\n", p->x.optimized);
    fprintf(stderr,"  referenced: %d\n", p->count);
    fprintf(stderr,"  Operator = %d\n", p->op);
    fprintf(stderr,"  link to next dag:  %p\n", p->link);
    fprintf(stderr,"  next on linearized list: %p\n", p->x.next);
    fprintf(stderr,"  syms[0]: %p\n", p->syms[0]);
    fprintf(stderr,"  syms[1]: %p\n", p->syms[1]);
    fprintf(stderr,"  kids[0]: %p\n", p->kids[0]);
    fprintf(stderr,"  kids[1]: %p\n", p->kids[1]);
    fprintf(stderr,"  result: %p\n",  p->x.result);
    fprintf(stderr,"  adrmode: %c\n", p->x.adrmode);
    fprintf(stderr,"  name: %s\n",    p->x.name);
}

static void print_graph_node(Node p) {
    Symbol s;
    Node left, right;
    if (p == NULL) return;
    s     = p->syms[0];
    left  = p->kids[0];
    right = p->kids[1];

    if (p->op < MAXOP)
        printf("\t%s%p [label=\"%s%c\"];\n", fname, p,
            opcode_names[generic(p->op)>>4], type_name[optype(p->op)]);
    else printf("\t%s%p [label=\"%s\"];\n", fname, p,
            additional_operators[p->op - MAXOP]);

    switch (generic(p->op)) {
    case ADDRF: case ADDRG: case ADDRL: case CNST: case LABEL:
        /* 1 symbol */
        printf("\t%s%p [shape=box,label=\"%s\"];\n",fname,s,s->x.name);
        printf("\t%s%p -> %s%p [style=dotted];\n",fname,p,fname,s);
        break;

    case EQ: case GE: case GT: case LE: case LT: case NE:
        /* 1 symbol, 2 kids */
        printf("\tN%p [shape=box,label=\"%s\"];\n",s,s->x.name);
        printf("\t%s%p -> N%p [style=dotted,label=\"label\"];\n",fname,p,s);

    case ASGN:
    case ADD: case SUB: case BAND: case BOR: case BXOR:
    case DIV: case LSH: case MOD: case MUL: case RSH:
        /* 2 kids */
        printf("\t%s%p -> %s%p [label=\"left\"];\n",fname,p,fname,left);
        printf("\t%s%p -> %s%p;\n",fname,p,fname,right);
        break;

    case BCOM:
    case CVC: case CVD: case CVF: case CVI: case CVP: case CVS: case CVU:
    case INDIR: case NEG:
    case JUMP:
    case ARG:
        /* 1 kid */
        printf("\t%s%p -> %s%p;\n",fname,p,fname,left);
        break;

    case RET:
        /* 0 or 1 kid */
        if (optype(p->op) != V)
            printf("\t%s%p -> %s%p;\n",fname,p,fname,left);
        break;

    case CALL:
        /* 1 or 2 kids */
        printf("\t%s%p -> %s%p;\n", fname,p,fname,left);
        if (optype(p->op) == B)
            printf("\t%s%p -> %s%p [label=\"res\"];\n",fname,p,fname,right);
        break;
    }
}

static Node *linearize(Node p, Node *last, Node next) {
    if (p && !p->x.visited) {
        if ( optimizelevel>0 ) {
            switch (generic(p->op)) {
            case CNST:
            case ADDRG: case ADDRL: case ADDRF:
                p->x.optimized=1;
                p->x.result=p->syms[0];
                p->x.adrmode=p->syms[0]->x.adrmode;
                p->x.name=p->syms[0]->x.name;
                return last;
            }
        }
        last = linearize(p->kids[0], last, NULL);
        last = linearize(p->kids[1], last, NULL);
        p->x.visited = 1;
        *last = p;
        last = &p->x.next;
    }
    *last = next;
    return last;
}

void progbeg(int argc,char *argv[]) {
    int i;
    for(i=1;i<argc;i++) {
        if (strncmp(argv[i],"-N",2)==0) {
            NamePrefix=argv[i]+2;
        } else if (strcmp(argv[i],"-G")==0) {
            graph_output=true;
        } else if (strcmp(argv[i],"-O")==0) {
            optimizelevel=3;
        } else if (strcmp(argv[i],"-O0")==0) {
            optimizelevel=0;    /* no optimization */
        } else if (strcmp(argv[i],"-O1")==0) {
            optimizelevel=1;    /* remove ADDR and CNST leaves */
        } else if (strcmp(argv[i],"-O2")==0) {
            optimizelevel=2;    /* allocate register variables */
                                /* and do some easy opt. (INC...) */
        } else if (strcmp(argv[i],"-O3")==0) {
            optimizelevel=3;    /* optimizes INDIR, ASGN ... */
        } else if (strcmp(argv[i],"-Os")==0 || strcmp(argv[i],"-OS")==0) {
            optimizelevel=3;        /* smallest level today (its folds are size wins) */
            optimize_for_size=1;    /* size axis - see decl; == -O3 codegen for now */
        } else {
            fprintf(stderr,"Unknown option %s\n",argv[i]);
            exit(1);
        }
    }
    if (graph_output) optimizelevel=0;
    if (graph_output) printf("digraph Frontend_output {\n");
    else print(version);

    /* 8 virtual registers */
    for (i=0;i<8;i++) regname[i]=stringf("reg%d",i);

    /* No spilling for simplicity purpose:
     * 32 temporaries for integer/pointer expressions, 32 for float expresssions.
     * => it should be nearly impossible to have such a complex expression
     * that we run out of temporaries (a compilation error will be raised in such
     * a pathological case).
     * All 32 integer/pointer temporaries in page zero,
     * and all 32 floating-point temporaries on stack frame.
     * => the runtime has to declare enough temporaries in zero page,
     * previous versions only used tmp0-tmp7...
     */
    for (i=0;i<32;i++) {
        temp[i]     = newtemp(STATIC,I);
        flt_temp[i] = newtemp(STATIC,F);
    }
    for (i=0;i<32;i++) {
        temp[i]->x.name=stringf("tmp%d",i);
        temp[i]->x.adrmode='Z';
    }
}

/* optimizeset/optimizepush/optimizepop - support for #pragma optimize.
 * Code generation for a function runs before the lexer consumes its
 * closing brace, so a pragma placed between two functions affects only
 * the functions that follow it; a pragma inside a body affects that
 * whole function. */
void optimizeset(int level) {
    if (!graph_output) optimizelevel = level;
}

int optimizepush(int level) {
    if (optsp >= (int)(sizeof optstack / sizeof optstack[0])) return -1;
    optstack[optsp++] = optimizelevel;
    optimizeset(level);
    return 0;
}

int optimizepop(void) {
    if (optsp <= 0) return -1;
    optimizelevel = optstack[--optsp];
    return 0;
}

static void emit_ctype_flush(void);  /* defined below, after ctype_name */

void progend(void) {
    if (graph_output) printf("}\n");
    else emit_ctype_flush();
}

static bool is_temporary(Symbol s) {
    int i;
    for (i=0;i<32;i++)
        if (s==temp[i]) return true;
    return false;
}

void defsymbol(Symbol p) {
    if (p->x.name) return;
    if (p->scope == CONSTANTS) {
        p->x.name = p->name;
        if (p->x.name[0]=='0' && p->x.name[1]=='x') {
            p->x.name[0]=' '; p->x.name[1]='$';
        }
    } else if (p->sclass == STATIC) {
        int lab = genlabel(1);
        /* Append the original C name to the generated label for readable asm
           (L<prefix><n><name>, e.g. Lpi129arr). The unique <n> keeps it
           collision-free; the name is added only when it is a real identifier,
           so compiler-generated statics (string literals, switch tables) whose
           name is already a number stay as L<prefix><n>. Cap the appended part
           so an unusually long identifier can't overflow assembler label
           limits. Purely cosmetic - labels emit no bytes. */
        char c0 = p->name ? p->name[0] : 0;
        if ((c0 >= 'a' && c0 <= 'z') || (c0 >= 'A' && c0 <= 'Z') || c0 == '_') {
            char buf[24]; int i;
            for (i = 0; i < 23 && p->name[i]; i++) buf[i] = p->name[i];
            buf[i] = 0;
            p->x.name = stringf("L%s%d%s", NamePrefix, lab, buf);
        } else
            p->x.name = stringf("L%s%d", NamePrefix, lab);
    }
    else if (p->generated)
        p->x.name = stringf("L%s%s", NamePrefix, p->name);
    else
        p->x.name = stringf("_%s", p->name);
    p->x.adrmode = 'C';

}

void export(Symbol p) {}
void import(Symbol p) {}
void segment(int s) {}
void global(Symbol p) { if (!graph_output) print("%s\n", p->x.name); }

void printfloat(double val)
{
    int i,exp=32,negative=0;
    double two_pow31,two_pow32;
    unsigned long mantissa;

    if (val==0.0) {
        print("\tDB(0)\n");
        print("\tDB(0)\n");
        print("\tDB(0)\n");
        print("\tDB(0)\n");
        print("\tDB(0)\n");
        return;
    }
    if (val<0.0) { negative=1; val= -val; }
    for (two_pow31=1.0,i=0;i<31;i++) two_pow31*=2;
    two_pow32=two_pow31*2;
    while (val>=two_pow32) {
        val/=2;
        exp++;
    }
    while (val<two_pow31) {
        val*=2;
        exp--;
    }
    if (!negative) val-=two_pow31;
    mantissa=val;
    print("\tDB($%x)\n",exp+128);
    print("\tDB($%x)\n",(mantissa>>24)&0xFF);
    print("\tDB($%x)\n",(mantissa>>16)&0xFF);
    print("\tDB($%x)\n",(mantissa>>8)&0xFF);
    print("\tDB($%x)\n",mantissa&0xFF);
}


void defconst(int ty, Value v) {
    if (graph_output) return;
    switch (ty) {
    case C: print("\tDB(%d)\n",   v.uc); break;
    case S: print("\tDB(%d)\n",   v.us); break;
    case I: print("\tDW(%d)\n",   v.i ); break;
    case U: print("\tDW($%x)\n",  v.u ); break;
    case P: print("\tDW($%x)\n",  v.p ); break;
    case F: printfloat(v.f); break;
    case D: printfloat(v.d); break;
    default: assert(0);
    }
}

void defstring(int len, char *s) {
    if (graph_output) return;
    while (len > 0) {
        print("\tDB($%x)\n",(unsigned char)*s++);
        len--;
    }
}

void defaddress(Symbol p) {
    if (graph_output) return;
    print("\tDW(%s)\n",p->x.name);
}

void space(int n) {
    if (graph_output) return;
    print("\tZERO(%d)\n",n);
}

int allocreg(Symbol p) {
    /* size 5 = float (frame allocated); size 4 = 32-bit long - the reg
       slots are 2 bytes, so longs stay in memory (v1: no reg pairs) */
    if (nbregs==8 || p->type->size==5 || p->type->size==4) return 0;
    p->x.name=regname[nbregs];
    p->x.adrmode='R';
    nbregs++;
    return 1;
}

void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls) {
    int i;

    if (graph_output) {
        printf("subgraph cluster%s {\n", f->x.name);
        printf("\tlabel=\"%s\";\n",f->x.name);
    }
    localsize=offset=tmpsize=nbregs=0; fname=f->x.name;
    busy=0; busy_flt=0;

    /* Only tmp0-tmp7 are available (declared in zp_crt.inc).
     * Mark tmp8-tmp31 as permanently busy so gettmp() will hit the
     * "Too complex expression" error instead of emitting ******. */
    for (i=8;i<32;i++) busy |= (1u<<i);
    /* Float temporaries are different: they are allocated on demand in the
     * stack frame by local() (see gettmp), and the leading '*' in the name
     * is the "not allocated yet" marker local() tests for. Reset the names
     * so each function starts with fresh slots; do NOT mark them busy
     * (that would make any float expression fail as "too complex"). */
    for (i=0;i<32;i++) flt_temp[i]->x.name="******";

    for (i = 0; caller[i] && callee[i]; i++) {
        caller[i]->x.name=stringf(graph_output?"param(%d)":"(ap),%d",offset);
        caller[i]->x.adrmode='A';
        offset+=caller[i]->type->size;
        if (optimizelevel>1 && callee[i]->sclass==REGISTER && allocreg(callee[i]))
            ; /* allocreg ok */
        else {
            callee[i]->x.adrmode=caller[i]->x.adrmode;
            callee[i]->x.name=caller[i]->x.name;
            callee[i]->sclass=AUTO;
        }
    }
    busy=localsize=0; offset=6;
    gencode(caller,callee);

    omit_frame=(i==0 && localsize==6);
    if (!graph_output) {
        print("%s\n",fname);
        /* Tag the entry code (ENTER prologue) with the function's definition
           line: the last .csource emitted before it belongs to the PREVIOUS
           function, so without this the debugger maps the entry address to
           that function's closing brace. */
        if (glevel && f->src.file)
            print(".csource \"%s\" %d\n", f->src.file, f->src.y);
        if (optimizelevel>1 && omit_frame && nbregs==0)
            ;
        else print("\tENTER(%d,%d)\n",nbregs,localsize);
        if (isstruct(freturn(f->type)))
            print("\tMOVW_DY(op1,(fp),6)\n");
    }
    emitcode();

    if (graph_output) printf("}\n");
}

void local(Symbol p) {
    if (optimizelevel>1 && p->sclass==REGISTER && allocreg(p))
        return; /* allocreg ok */
    if (p->x.name && p->x.name[0]!='*') return; /* keep previous local (it isn't busy) */
    p->x.name = stringf(graph_output?"local(%d)":"(fp),%d",offset);
    p->x.adrmode = 'A';
    p->sclass = AUTO;
    offset+=p->type->size;
}

void address(Symbol q, Symbol p, int n) {
    q->x.name = stringf("%s%s%d", p->x.name, n >= 0 ? "+" : "", n);
    q->x.adrmode=p->x.adrmode;
}

void blockbeg(Env *e) { e->offset = offset; }
void blockend(Env *e) {
    if (offset > localsize) localsize = offset;
    offset = e->offset;
}

static void gettmp(Node p) {
    int t;
    if ( optype(p->op)!=F && optype(p->op)!=D ) {
        if (p->x.width==4) {
            /* 32-bit long: two ADJACENT 2-byte slots (tmp<t> + tmp<t+1> are
               4 contiguous zero-page bytes in zp_crt.inc). Only tmp0-7 have
               zero-page backing, so the pair must fit below slot 7. */
            for (t=0;t<7;t++)
                if ((busy&(3<<t))==0) {
                    busy |= 3<<t;
                    p->x.result=temp[t];
                    p->x.adrmode='Z';
                    p->x.name = temp[t]->x.name;
                    return;
                }
            fprintf(stderr, "Error in function '%s': expression too complex "
                            "(no adjacent temporary pair free for a 32-bit value).\n", fname);
            exit(1);
        }
        for (t=0;t<32;t++)
            if ((busy&(1<<t))==0) {
                busy |= 1<<t;
                p->x.result=temp[t];
                p->x.adrmode='Z';

                p->x.name = temp[t]->x.name;
                return;
            }
    } else
        for (t=0;t<32;t++)
            if ((busy_flt&(1<<t))==0) {
                busy_flt |= 1<<t;
                p->x.result=flt_temp[t];
                p->x.adrmode='Y';
                local(flt_temp[t]);
                p->x.name = flt_temp[t]->x.name;
                return;
            }
    fprintf(stderr, "Error in function '%s': expression too complex (out of temporary registers, max 8).\n"
                    "Simplify the expression by splitting it into smaller statements with intermediate variables.\n", fname);
    exit(1);
}

static void releasetmp(Node p) {
    if (!p) return;
    assert(p->count!=0);
    p->count--;
    switch(generic(p->op)) {
        case ADDRG: case ADDRL: case ADDRF: case CNST:
            if (optimizelevel>0) return;
            break;
        case INDIR:
            if (p->x.optimized) return;
    }
    if (p->count==0) {
        int i;
        for (i=0;i<32;i++) {
            if (p->x.result == temp[i]) {
                busy &= ~(1<<i);
                if (p->x.width==4 && i<31)
                    busy &= ~(1<<(i+1));   /* free the pair's second slot */
            }
            if (p->x.result==flt_temp[i]) busy_flt &= ~(1<<i);
        }
    }
}

void print_busy() {
    if (busy) fprintf(stderr,"Busy tmps when calling function: %x\n", busy);
    if (busy_flt) fprintf(stderr,"Busy flt_tmps when calling function: %x\n", busy_flt);
}

static bool is_dereferenceable(char adrmode)
{
    return adrmode=='C' || adrmode=='R' || adrmode=='A' || adrmode=='Z';
}

static char dereference(char adrmode)
{
    switch (adrmode) {
        case 'C': return 'D';
        case 'R': return 'Z';
        case 'A': return 'Y';
        case 'Z': return 'I';
        default:
            assert(is_dereferenceable(adrmode));
            return 0;
    }

}

/* An optimized (folded) INDIR borrows the temporary that holds the address
 * it dereferences. If the fold consumed the LAST reference to its child,
 * that temporary was already released, so without protection the parent
 * node - or any node evaluated between the INDIR and its parent - could
 * allocate the same slot and clobber the address before it is
 * dereferenced. Two observed damage patterns: MOVW_YD((tmp0),0,tmp0) reads
 * the low byte through tmp0, overwrites the pointer with it, then reads
 * the high byte through a corrupted pointer; and ADDW_DCD(tmp1,1,tmp0)
 * overwriting a pointer that a following NEW_YD((tmp0),0,...) still needs.
 * The fold re-marks the slot busy (borrow_temp) and tmpalloc() frees it
 * (release_borrowed) only after the parent allocated its own result.
 * When the child is still shared (count > 0) its temporary is still owned
 * by the child itself and must NOT be managed here - freeing it early was
 * just as harmful as not reserving it. */
static int borrow_temp(Node p, Node left) {
    int i;
    if (left->count > 0) return 0;      /* child still owns its temporary */
    for (i=0;i<32;i++)
        if (left->x.result==temp[i]) { busy |= (1u<<i); return 1; }
    return 0;                           /* not a temporary: nothing to own */
}

static void release_borrowed(Node p) {
    int i;
    if (!p) return;
    if (!p->x.borrowed || p->count!=0) return;
    for (i=0;i<32;i++)
        if (p->x.result==temp[i]) { busy &= ~(1u<<i); return; }
}

static int needtmp(Node p) {
    Node left = p->kids[0];
    if (graph_output) return 0;
    switch (generic(p->op)) {
        case ADDRF: case ADDRG: case ADDRL:
        case CNST:
            assert(optimizelevel==0);
            return 1;
        case INDIR:
            if (optimizelevel!=0)
                if (optype(p->op)==B) { /* remove all INDIRB nodes */
                    p->x.optimized = 1;
                    p->x.result    = left->x.result;
                    p->x.name      = p->x.result->x.name;
                    p->x.adrmode   = left->x.adrmode;
                    p->x.borrowed  = borrow_temp(p, left);
                    return 0;
                }

            if (optimizelevel>=3) {
        /* these conditions must be true to optimize (=get rid of) an INDIR node:
         * - this INDIR node is referenced only once
         *   (otherwise by delaying the indirection in parent nodes we would duplicate it,
         *   and hence could have different behavior (TODO: example needed))
         * - the address mode of the INDIR operand is "de-referenceable"
         */
                /* never fold a 32-bit INDIR into its address child: the
                   child's temporary holds a 2-byte pointer, the result
                   needs a 4-byte pair - allocate one normally instead */
                if (p->count <= 1 && is_dereferenceable(left->x.adrmode)
                    && p->x.width!=4) {
                    p->x.optimized = 1;
                    p->x.result    = left->x.result;
                    p->x.name      = left->x.result->x.name;
                    p->x.adrmode   = dereference(left->x.adrmode);
                    p->x.borrowed  = borrow_temp(p, left);
                    return 0;
                }
            }
            return 1;
        case ASGN:
        case ARG:
        case EQ: case GE: case GT: case LE: case LT: case NE:
        case RET:
        case JUMP: case LABEL:
            return 0;
        case CALL:
            if (optype(p->op)==B) return 0;
            if (p->count==0) p->op=CALLV;
            if (optype(p->op)==V) return 0;
            else return 1;
        default:
            return 1;
    }
}

static void tmpalloc(Node p) {
    Node left = p->kids[0], right = p->kids[1];
    p->x.optimized=0;
    p->x.borrowed=0;
    p->x.name="*******";
    p->x.adrmode='*';
    releasetmp(left); releasetmp(right);

    switch (generic(p->op)) {
    case ARG:
        p->x.argoffset = argoffset;
        argoffset += p->syms[0]->u.c.v.i;
        break;
    case CALL:
        p->x.argoffset = argoffset;
        p->x.busy      = busy;
        argoffset = 0;
        break;
    case ASGN:
        if (optimizelevel>=3) {
        /* these conditions must be true to optimize (=get rid of) an ASGN node:
         * - it gets its value (right child node) from a temporary variable,
         * - the right child child has not been eliminated (optimized)
         * - the ASGN node comes just after its right child node
         *   (TODO: could it be more general? => re-ordering ?)
         * - the left value is de-referenceable,
         * - and the temporary variable is not used afterwards,
         *
         * In this case, we try to directly assign the result inside the right child node
         * (as the result of this child node)
         */
            if (optype(p->op)!=B        // no optimization on ASGNB (struct) nodes
                && p->x.width!=4        // v1: no store-fold for 32-bit longs
                && right->x.width!=4    //     (conservative - revisit later)
                && !right->x.optimized
                && p==right->x.next
                && is_temporary(right->x.result)
                && is_dereferenceable(left->x.adrmode)
                && right->count==0
                // Folding the store into the right child makes the child emit
                // the assignment as part of its own result. For a CALL that is
                // unsafe on two counts, both hit by e.g. "buf[i] = f(buf[i])":
                //  - the store inherits the child's WIDTH, so folding an int
                //    CALL into a char ASGN would store a word (clobbering the
                //    next byte). Require the widths to match.
                //  - the store executes as the call returns, but the call
                //    clobbers the scratch temporaries; if the destination
                //    address lives in a temporary it is stale by then (it is
                //    only RESTOREd after the call). Require a stable address.
                && !(generic(right->op)==CALL
                     && (optype(right->op)!=optype(p->op)
                         || is_temporary(left->x.result)))
                )
            {
                p->x.optimized     = 1;
                right->x.result    = left->x.result;
                right->x.name      = left->x.name;
                right->x.adrmode   = dereference(left->x.adrmode);
            }
        }
        break;
    }
    if (needtmp(p)) gettmp(p);
    /* our folded-INDIR children may hold a reserved (borrowed) temporary:
       now that our own result is allocated, their address slot can go */
    release_borrowed(left); release_borrowed(right);
}

/* --- 8-bit (byte) narrowing, Phase 1: + - & | ^ on char ------------------
 * The front end promotes char to int, so "c = a + b" (all char) is
 *   ASGNC(c, CVUC(CVIU( ADDI( CVUI(CVCU(a)), CVUI(CVCU(b)) ) )))
 * i.e. widen both operands to 16 bits (CVCU -> CZBW), add as 16 bits, narrow
 * the result back to a char. For + - & | ^ the char result depends only on
 * the operands' low bytes, so when the result is only ever used as a char we
 * can drop the two widenings and do the op at byte width (ADDB etc.).
 * CVUI/CVIU are 16-bit reinterprets (no-ops), skipped when walking the tree. */
static Node under_conv(Node n) {
    while (n && (n->op==CVUI || n->op==CVIU || n->op==CVPU || n->op==CVUP))
        n = n->kids[0];
    return n;
}
static int is_byte_widen(Node n) { return n && n->op==CVCU; }   /* CZBW: uchar->uint */
/* Phase 1: binary ops whose char result depends only on the operands' low
 * bytes. Phase 2 adds the unary ~ / - and << 1 (below). */
static int is_narrowable_binary(int op) {
    return op==ADDI || op==ADDU || op==SUBI || op==SUBU
        || op==BANDU || op==BORU || op==BXORU;
}
static int is_narrowable_unary(int op) {
    return op==BCOMU || op==NEGI;         /* ~x , -x */
}
static int is_int_const(Node n) {
    return n && generic(n->op)==CNST && (optype(n->op)==I || optype(n->op)==U);
}
/* long (width-4) add/sub and word INDIR opcode groups, for in-place RMW below */
static int is_long_add(int op)  { return op==ADDI || op==ADDU || op==ADDP; }
static int is_long_sub(int op)  { return op==SUBI || op==SUBU || op==SUBP; }
static int is_indir_word(int op){ return op==INDIRI || op==INDIRP; }
/* an integer constant whose value fits an unsigned char (0..255) - the only
 * constants a byte compare can use unchanged */
static int is_byte_const(Node n) {
    return is_int_const(n) && n->syms[0]
        && n->syms[0]->u.c.v.i >= 0 && n->syms[0]->u.c.v.i <= 255;
}
static int is_eq_cmp(int op)  { return op==EQI || op==NEI; }
/* a single-use BANDU whose result provably fits a byte: AND can only clear
 * bits, so one byte-widen operand bounds the result to 0..255 regardless of
 * the other side. The partner must still be a byte widen or a byte constant
 * so the emitted ANDB reads the right low byte. Test only - marking is done
 * by mark_byte_and() once the surrounding compare shape is accepted. */
static int is_byte_and(Node n) {
    Node a, b;
    if (!n || n->op != BANDU || n->count != 1) return 0;
    a = under_conv(n->kids[0]);
    b = under_conv(n->kids[1]);
    if (is_byte_widen(a) && (is_byte_widen(b) || is_byte_const(b))) return 1;
    if (is_byte_widen(b) && (is_byte_widen(a) || is_byte_const(a))) return 1;
    return 0;
}
static void mark_byte_and(Node n) {
    Node a = under_conv(n->kids[0]);
    Node b = under_conv(n->kids[1]);
    n->x.narrow = 1;                      /* emit ANDB */
    if (is_byte_widen(a)) a->x.narrow = 1;   /* widen elision candidates */
    if (is_byte_widen(b)) b->x.narrow = 1;
}
static int is_ord_cmp(int op) {
    return op==LTI||op==LTU||op==GTI||op==GTU
        || op==LEI||op==LEU||op==GEI||op==GEU;
}
static void mark_byte_narrowing(Node head) {
    Node m, n, ka, kb;
    for (m = head; m; m = m->x.next) m->x.narrow = 0;
    if (optimizelevel < 2) return;
    for (m = head; m; m = m->x.next) {
        /* Phase 3: char comparisons. uchar operands are 0..255, so the
         * promoted 16-bit compare equals an unsigned single-byte compare of
         * the low bytes. Only CVCU (unsigned) widens qualify, so signed char
         * stays on the word path. The compare node is flagged narrow so the
         * operand widens can be elided (it counts as a byte reader). */
        if (is_eq_cmp(m->op)) {
            int aval, bval;
            ka = under_conv(m->kids[0]);
            kb = under_conv(m->kids[1]);
            /* == / != are symmetric, so accept char on either side. A side
             * qualifies as byte-VALUED when it is a CVCU widen of a char or
             * a single-use AND bounded by a byte operand (the "(x & mask)"
             * test idiom: the AND result cannot exceed 255, so the 16-bit
             * compare equals the single-byte one and the operand's dead
             * zero-extend can be elided). */
            aval = is_byte_widen(ka) || is_byte_and(ka);
            bval = is_byte_widen(kb) || is_byte_and(kb);
            if ((aval && (bval || is_byte_const(kb)))
             || (bval && is_byte_const(ka))) {
                m->x.narrow = 1;
                if (is_byte_widen(ka))    ka->x.narrow = 1;
                else if (aval)            mark_byte_and(ka);
                if (is_byte_widen(kb))    kb->x.narrow = 1;
                else if (bval)            mark_byte_and(kb);
            }
            continue;
        }
        if (is_ord_cmp(m->op)) {
            ka = under_conv(m->kids[0]);
            kb = under_conv(m->kids[1]);
            /* ordered forms are emitted char-first only (no CD macros) */
            if (!is_byte_widen(ka)) continue;
            if (is_byte_widen(kb))       m->x.narrow = ka->x.narrow = kb->x.narrow = 1;
            else if (is_byte_const(kb))  m->x.narrow = ka->x.narrow = 1;
            continue;
        }
        if (m->op != CVUC && m->op != CVIC) continue;   /* narrow (u)int -> char */
        n = under_conv(m->kids[0]);
        /* single-use = count==1 here: this pre-pass runs before tmpalloc
         * decrements the reference counts */
        if (!n || n->count != 1) continue;

        if (is_narrowable_unary(n->op)) {
            /* ~x / -x : one byte-widened operand, low byte is all that matters. */
            ka = under_conv(n->kids[0]);
            if (!is_byte_widen(ka)) continue;
            n->x.narrow  = 1;             /* emit COMB / NEGB */
            ka->x.narrow = 1;             /* elision candidate */
            continue;
        }
        if (n->op==LSHI || n->op==LSHU || n->op==RSHI || n->op==RSHU) {
            /* x << 1 / x >> 1 : the char result needs only x's low byte. Only
             * the shift-by-1 form maps to a single asl/lsr (LSH1B / RSH1B);
             * other counts stay on the word path (the byte would need a
             * count-driven loop), so leave them un-narrowed with their widens
             * intact. >> is a LOGICAL byte shift and is only reached with a
             * CVCU (unsigned, 0..255) operand, so lsr is correct even though
             * the promoted op is the signed RSHI. */
            ka = under_conv(n->kids[0]);
            kb = n->kids[1];
            if (!is_byte_widen(ka)) continue;
            if (!(is_int_const(kb) && kb->syms[0] && kb->syms[0]->u.c.v.i == 1))
                continue;
            n->x.narrow  = 1;             /* emit LSH1B / RSH1B */
            ka->x.narrow = 1;             /* elision candidate */
            continue;
        }
        if (is_narrowable_binary(n->op)) {
            ka = under_conv(n->kids[0]);
            kb = under_conv(n->kids[1]);
            if (!is_byte_widen(ka)) continue;
            /* operand B: a byte widen, or an integer constant (its low byte is
             * what the byte op reads). */
            if (!is_byte_widen(kb) && !is_int_const(kb))
                continue;
            /* The byte op reads only the low bytes, so ADDB/... is ALWAYS
             * correct regardless of sharing. Flag the operand widens as
             * ELISION CANDIDATES; whether the dead zero-extend is actually
             * dropped is decided at emit time by widen_dead_after() (Phase 1b),
             * which checks the physical temp is not read by any wider consumer. */
            n->x.narrow  = 1;   /* emit ADDB/SUBB/ANDB/ORB/XORB */
            ka->x.narrow = 1;   /* candidate: elide its CZBW if the temp stays byte-only */
            if (is_byte_widen(kb))
                kb->x.narrow = 1;
            continue;
        }
    }
}

/* Phase 1b: is the value produced by widen node `p` (into temp p->x.name) used
 * only by byte-narrowed ops before that temp is next overwritten? If so the
 * zero-extend is dead and can be dropped. A reader that is NOT byte-narrowed
 * (e.g. a 16-bit != 0 test that shares the load in `while(i--)`) needs the high
 * byte, so the widen must stay. Works on physical temp names after tmpalloc, so
 * it is correct even when several DAG values alias the same temporary. */
static int widen_dead_after(Node p) {
    char *t = p->x.name;
    Node q;
    if (!t) return 0;
    for (q = p->x.next; q; q = q->x.next) {
        Node k0 = q->kids[0], k1 = q->kids[1];
        int reads = (k0 && k0->x.name && strcmp(k0->x.name, t) == 0)
                 || (k1 && k1->x.name && strcmp(k1->x.name, t) == 0);
        /* CVUI/CVIU/CVPU/CVUP are 16-bit reinterprets that emit no code; an
         * in-place one just carries the value forward in the same temp, so it
         * neither consumes the zero-extend nor redefines the temp - see through
         * it (the same nodes under_conv() skips when marking). */
        if ((q->op==CVUI || q->op==CVIU || q->op==CVPU || q->op==CVUP)
            && reads && q->x.name && strcmp(q->x.name, t) == 0)
            continue;
        if (reads && !q->x.narrow)
            return 0;                       /* a wider consumer needs the extend */
        if (q->x.name && strcmp(q->x.name, t) == 0)
            return 1;                       /* temp redefined: all prior reads byte-safe */
    }
    return 1;                               /* temp never read again */
}

static char simple_adrmode(char adrmode);   /* defined below; used by the pre-passes */

/* In-place long read-modify-write with a constant. `long i += k` (and -=) is
 * lowered by the front end to asgn(i, add(indir(i), k)), and dag CSE gives the
 * store target and the add's INDIR source the SAME address node, so the store
 * provably aliases the load. Rather than INDIRL + ADDL (which stages the
 * constant into lscratch and calls jsr ladd32) + ASGNL, emit one ADDLK/SUBLK
 * macro that does an in-place adc/sbc chain straight on the memory bytes. The
 * ASGN emits the macro; the ADD/SUB and the INDIR emit nothing. Mirrors the
 * mark_byte_narrowing pre-pass: runs before tmpalloc so node counts are still
 * the true source counts, and uses x.inplace (which tmpalloc does not clear). */
static void mark_inplace_rmw(Node head) {
    Node m;
    for (m = head; m; m = m->x.next) m->x.inplace = 0;
    if (optimizelevel < 2) return;
    for (m = head; m; m = m->x.next) {
        Node v, ind, cnst;
        char dm;
        int isadd;
        if (m->op != ASGNI && m->op != ASGNP) continue;
        if (!(m->syms[0] && m->syms[0]->u.c.v.i == 4)) continue;   /* store 4 bytes */
        v = m->kids[1];                                            /* value = ADD/SUB */
        if (!v || v->x.width != 4 || v->count != 1) continue;
        isadd = is_long_add(v->op);
        if (!isadd && !is_long_sub(v->op)) continue;
        /* one operand is an INDIR of the SAME address as the store target
         * (pointer identity = proven alias), the other is an int constant.
         * ADD is commutative (const either side); SUB needs the const on the
         * right (i - k). Require single-use so nothing else needs the load or
         * the sum. */
        if (v->kids[0] && is_indir_word(v->kids[0]->op)
            && v->kids[0]->x.width == 4 && v->kids[0]->count == 1
            && v->kids[0]->kids[0] == m->kids[0]
            && is_int_const(v->kids[1])) {
            ind = v->kids[0]; cnst = v->kids[1];
        } else if (isadd
            && v->kids[1] && is_indir_word(v->kids[1]->op)
            && v->kids[1]->x.width == 4 && v->kids[1]->count == 1
            && v->kids[1]->kids[0] == m->kids[0]
            && is_int_const(v->kids[0])) {
            ind = v->kids[1]; cnst = v->kids[0];
        } else continue;
        (void)cnst;
        /* destination addressing we can render in place: frame slot / static */
        dm = m->kids[0]->x.adrmode;
        if (dm != 'A' && dm != 'C') continue;
        m->x.inplace = v->x.inplace = ind->x.inplace = 1;
    }
}

/* Fused long `mem +/- const -> temp`. A width-4 ADD/SUB (not already an in-place
 * RMW) with one operand a single-use INDIR of a frame/static long and the other
 * an int constant, whose result is a temp: instead of INDIRL (load the long into
 * a temp) + ADDL (stage the constant, jsr ladd32, store), emit one ADDLKM/SUBLKM
 * that reads the source memory and writes the result temp with an inline adc/sbc
 * chain - no op1:op2, no lscratch, no jsr. Mark the ADD (emits the macro) and its
 * INDIR (inplace = suppress its load). ADD takes the constant on either side;
 * SUB needs the INDIR on the left (mem - const). Runs before tmpalloc, so counts
 * are still the source counts; the source addressing mode lives on the leaf
 * address node and is already known here. */
static void mark_fuse_addk(Node head) {
    Node m;
    for (m = head; m; m = m->x.next) m->x.fusek = 0;
    if (optimizelevel < 2) return;
    for (m = head; m; m = m->x.next) {
        Node ind = 0, cnst = 0, addr;
        int isadd;
        char sm;
        if (m->x.width != 4 || m->x.inplace) continue;      /* leave RMW alone */
        isadd = is_long_add(m->op);
        if (!isadd && !is_long_sub(m->op)) continue;
        if (m->kids[0] && is_indir_word(m->kids[0]->op)
            && m->kids[0]->x.width == 4 && m->kids[0]->count == 1
            && is_int_const(m->kids[1])) {
            ind = m->kids[0]; cnst = m->kids[1];
        } else if (isadd && m->kids[1] && is_indir_word(m->kids[1]->op)
            && m->kids[1]->x.width == 4 && m->kids[1]->count == 1
            && is_int_const(m->kids[0])) {
            ind = m->kids[1]; cnst = m->kids[0];            /* const + mem */
        } else continue;
        (void)cnst;
        addr = ind->kids[0];                                /* the source address */
        if (!addr) continue;
        sm = simple_adrmode(addr->x.adrmode);
        if (sm != 'A' && sm != 'C') continue;               /* frame / static only */
        m->x.fusek = 1;
        ind->x.inplace = 1;                                 /* suppress the load */
    }
}

Node gen(Node p) {
    Node head, *last;
    for (last = &head; p; p = p->link)
        last = linearize(p, last, 0);
    if (!graph_output) { mark_byte_narrowing(head); mark_inplace_rmw(head); mark_fuse_addk(head); }
    for (p = head; p; p = p->x.next) {
        if (graph_output) print_graph_node(p);
        else tmpalloc(p);
    }
    return head;
}

void asmcode(char *str, Symbol argv[]) {
    for ( ; *str; str++)
        if (*str == '%' && str[1] >= 0 && str[1] <= 9)
            print("%s", argv[(int)*++str]->x.name);
        else
            print("%c", *str);
    print("\n");
}

static Node a,b,r;

/* avoid some proliferation of macros by rewriting
 * Zero-Page address mode as Direct address mode,
 * Register addresses as Constants,
 * and Indirect address mode as Indirect Y-indexed
 */
static char simple_adrmode(char adrmode) {
    if (adrmode=='Z') return 'D';
    else if (adrmode=='R') return 'C';
    else if (adrmode=='I') return 'Y';
    else return adrmode;
}
static char reduced_adrmode(char adrmode) {
    if (adrmode=='R') return 'C';
    else if (adrmode=='I') return 'Y';
    else return adrmode;
}
static char *output_name(Symbol s) {
    return s->x.adrmode=='I' ? stringf("(%s),0",s->x.name) : s->x.name;
}
static char *output_arg(Node n) {
    if (optimizelevel==0) return n->x.name;
    return n->x.adrmode=='I' ? stringf("(%s),0",n->x.name) : n->x.name;
}

/* Constant-constant operand pairs reach the emitters when the front end
 * cannot fold them: a comma expression can hide a constant behind a RIGHT
 * node ("(f(), 42) + 1"), and bitfield lowering can synthesize masked
 * constant pairs. The macro library deliberately has no constant-constant
 * variants (nor constant-first variants of the commutative families), so
 * these helpers normalize the operands: commutative operands are swapped,
 * anything else materializes the first constant into the result location
 * (binary/unary) or the op1 scratch (compares; op1 is dead between the
 * multiply/divide helper calls that use it). */
static int is_commutative(char *inst) {
    return strcmp(inst,"ADDW")==0 || strcmp(inst,"ANDW")==0
        || strcmp(inst,"ORW" )==0 || strcmp(inst,"XORW")==0
        || strcmp(inst,"MULI")==0 || strcmp(inst,"MULU")==0
        || strcmp(inst,"ADDL")==0 || strcmp(inst,"ANDL")==0
        || strcmp(inst,"ORL" )==0 || strcmp(inst,"XORL")==0
        || strcmp(inst,"MULL")==0;
}

static void binary(char *inst) {
    char am, bm, rm;
    if (optimizelevel==0) {
        print("\t%s(%s,%s,%s)\n"
            ,inst
            ,output_arg(a)
            ,output_arg(b)
            ,output_arg(r));
        return;
    }
    am = simple_adrmode(a->x.adrmode);
    bm = simple_adrmode(b->x.adrmode);
    rm = simple_adrmode(r->x.adrmode);
    if (am=='C' && bm!='C' && is_commutative(inst)) {
        Node t=a; a=b; b=t;
        am = bm; bm = 'C';
    }
    if (am=='C' && bm=='C') {
        print("\tMOV%s_C%c(%s,%s)\n", r->x.width==4 ? "L" : "W",
              rm, output_arg(a), output_arg(r));
        print("\t%s_%c%c%c(%s,%s,%s)\n"
            ,inst ,rm ,bm ,rm
            ,output_arg(r) ,output_arg(b) ,output_arg(r));
        return;
    }
    print("\t%s_%c%c%c(%s,%s,%s)\n"
        ,inst ,am ,bm ,rm
        ,output_arg(a)
        ,output_arg(b)
        ,output_arg(r));
}

/* Emit a fused `result_temp = mem_long +/- const` (see mark_fuse_addk). Reads
 * the source memory (frame 'A' -> ADDLKM_A / static 'C' -> ADDLKM_C) and writes
 * the ADD/SUB result temp with an inline adc/sbc chain; the INDIR operand's own
 * load was suppressed. a/b are the node's kids, r its result. */
static void emit_fused_addk(int isadd) {
    Node ind  = is_int_const(a) ? b : a;
    Node cnst = is_int_const(a) ? a : b;
    Node addr = ind->kids[0];
    print("\t%sLKM_%c(%s,%s,%s)\n"
        ,isadd ? "ADD" : "SUB"
        ,simple_adrmode(addr->x.adrmode)
        ,output_arg(addr)
        ,output_arg(cnst)
        ,output_arg(r));
}

static void unary(char *inst) {
    char am, rm;
    if (optimizelevel==0) {
        print("\t%s(%s,%s)\n"
            ,inst
            ,output_arg(a)
            ,output_arg(r));
        return;
    }
    am = simple_adrmode(a->x.adrmode);
    rm = simple_adrmode(r->x.adrmode);
    if (am=='C' && (strcmp(inst,"LSH1W")==0 || strcmp(inst,"COMW")==0
                 || strcmp(inst,"NEGI")==0 || strcmp(inst,"COML")==0
                 || strcmp(inst,"NEGL")==0)) {
        /* these families have no constant-operand variant */
        print("\tMOV%s_C%c(%s,%s)\n", r->x.width==4 ? "L" : "W",
              rm, output_arg(a), output_arg(r));
        print("\t%s_%c%c(%s,%s)\n", inst, rm, rm, output_arg(r), output_arg(r));
        return;
    }
    print("\t%s_%c%c(%s,%s)\n"
        ,inst ,am ,rm
        ,output_arg(a)
        ,output_arg(r));
}

/* Byte shift-by-1 (LSH1B/RSH1B). When the shift is in place (operand==result,
 * D mode) it collapses to a single memory read-modify-write - LSH1B_D -> asl
 * mem, RSH1B_D -> lsr mem - instead of the load/shift/store LSH1B_DD form. */
static void byte_shift1(char *inst) {
    if (simple_adrmode(a->x.adrmode)=='D' && strcmp(a->x.name, r->x.name)==0)
        print("\t%s_D(%s)\n", inst, output_arg(r));
    else
        unary(inst);
}

static void compare0(char *inst) {
    if (simple_adrmode(a->x.adrmode)=='C') {
        print("\tMOVW_CD(%s,op1)\n", output_arg(a));
        print("\t%s_D(op1,%s)\n", inst, r->syms[0]->x.name);
        return;
    }
    print("\t%s_%c(%s,%s)\n"
            ,inst
            ,simple_adrmode(a->x.adrmode)
            ,output_arg(a)
            ,r->syms[0]->x.name);
}

/* Inline long compare against a constant. Given the long compare mnemonic
 * `inst` and whether the constant is the LEFT operand, produce the "var REL
 * const" form: pick the inline macro, whether its MSB compare is sign-flipped,
 * and whether the constant must be incremented (folding a<=k -> a<k+1 and
 * a>k -> a>=k+1, so only LT/GE/EQ/NE + unsigned LTU/GEU need macros). Const-
 * left is canonicalised by flipping the relation (a<b == b>a). Returns 0 for a
 * mnemonic that is not one of the ten long relations. */
static int long_cmp_k(const char *inst, int constLeft,
                      const char **macro, int *flipMSB, int *plus1)
{
    const char *rel = inst;
    if (constLeft) {
        if      (!strcmp(inst,"LTL"))  rel="GTL";
        else if (!strcmp(inst,"GTL"))  rel="LTL";
        else if (!strcmp(inst,"GEL"))  rel="LEL";
        else if (!strcmp(inst,"LEL"))  rel="GEL";
        else if (!strcmp(inst,"LTUL")) rel="GTUL";
        else if (!strcmp(inst,"GTUL")) rel="LTUL";
        else if (!strcmp(inst,"GEUL")) rel="LEUL";
        else if (!strcmp(inst,"LEUL")) rel="GEUL";
        else if (!strcmp(inst,"EQL") || !strcmp(inst,"NEL")) rel=inst;
        else return 0;
    }
    *plus1 = 0; *flipMSB = 0;
    if      (!strcmp(rel,"LTL"))  { *macro="LTLK_D";  *flipMSB=1; }
    else if (!strcmp(rel,"GEL"))  { *macro="GELK_D";  *flipMSB=1; }
    else if (!strcmp(rel,"LEL"))  { *macro="LTLK_D";  *flipMSB=1; *plus1=1; }
    else if (!strcmp(rel,"GTL"))  { *macro="GELK_D";  *flipMSB=1; *plus1=1; }
    else if (!strcmp(rel,"LTUL")) { *macro="LTULK_D"; }
    else if (!strcmp(rel,"GEUL")) { *macro="GEULK_D"; }
    else if (!strcmp(rel,"LEUL")) { *macro="LTULK_D"; *plus1=1; }
    else if (!strcmp(rel,"GTUL")) { *macro="GEULK_D"; *plus1=1; }
    else if (!strcmp(rel,"EQL"))  { *macro="EQLK_D"; }
    else if (!strcmp(rel,"NEL"))  { *macro="NELK_D"; }
    else return 0;
    return 1;
}

static void compare(char *inst) {
    if (optimizelevel==0) {
        print("\t%s(%s,%s,%s)\n"
            ,inst ,output_arg(a) ,output_arg(b) ,r->syms[0]->x.name);
        return;
    }
    /* Inline a width-4 compare against an integer constant when the variable
       operand is a D-mode temp: a direct 4-byte MSB-down compare against the
       constant's bytes, no lscratch staging and no jsr lcmp32. The bytes are
       computed here (gen.c has the value); signed relations pass an already-
       flipped MSB byte so the macro's `eor #$80` yields the signed result.
       Falls back to the routine path for non-D operands and for the a<=MAX /
       a>MAX cases whose +1 fold would wrap the type. */
    if ((a->x.width==4 || b->x.width==4) && (is_int_const(a) ^ is_int_const(b))) {
        Node var  = is_int_const(a) ? b : a;
        Node cnst = is_int_const(a) ? a : b;
        int constLeft = is_int_const(a);
        const char *macro; int flipMSB, plus1;
        if (simple_adrmode(var->x.adrmode)=='D'
            && long_cmp_k(inst, constLeft, &macro, &flipMSB, &plus1)) {
            unsigned long uv  = (unsigned long)cnst->syms[0]->u.c.v.i & 0xFFFFFFFFUL;
            unsigned long lim = flipMSB ? 0x7FFFFFFFUL : 0xFFFFFFFFUL;
            if (!(plus1 && uv == lim)) {          /* skip: +1 would wrap the type max */
                unsigned long tv = plus1 ? ((uv + 1) & 0xFFFFFFFFUL) : uv;
                int b0=(int)(tv&0xff), b1=(int)((tv>>8)&0xff),
                    b2=(int)((tv>>16)&0xff), b3=(int)((tv>>24)&0xff);
                if (flipMSB) b3 ^= 0x80;
                print("\t%s(%s,%d,%d,%d,%d,%s)\n"
                      ,macro ,output_arg(var) ,b0,b1,b2,b3 ,r->syms[0]->x.name);
                return;
            }
        }
    }
    if (simple_adrmode(a->x.adrmode)=='C' && simple_adrmode(b->x.adrmode)=='C') {
        print("\tMOVW_CD(%s,op1)\n", output_arg(a));
        print("\t%s_DC(op1,%s,%s)\n"
            ,inst ,output_arg(b) ,r->syms[0]->x.name);
    }
    else
        print("\t%s_%c%c(%s,%s,%s)\n"
            ,inst
            ,simple_adrmode(a->x.adrmode)
            ,simple_adrmode(b->x.adrmode)
            ,output_arg(a)
            ,output_arg(b)
            ,r->syms[0]->x.name);
}

static void save_busy(Node p) {
    int i, offset=p->x.argoffset;
    for (/*int*/ i=0; i<8; i++) {
        if (p->x.busy & (1<<i)) {
            // save on stack and increase the argsize
            print("\tSAVE(tmp%d,(sp),%d)\n", i, offset);
            offset += 2;
        }
    }
    // update the CALL's argoffset so that it is passed to the callee
    p->x.argoffset = offset;
}

static void restore_busy(Node p) {
    int i, offset=p->x.argoffset;
    for (/*int*/ i=7; i>=0; i--) { // reverse order
        if (p->x.busy & (1<<i)) {
            offset -= 2;
            print("\tRESTORE((sp),%d,tmp%d)\n", offset, i);
        }
    }
}

static void emitdag0(Node p) {
    a = p->kids[0]; b = p->kids[1]; r=p;
    if (p->x.width==4 || (a && a->x.width==4) || (b && b->x.width==4)) {
        fprintf(stderr, "Error in function '%s': 32-bit long requires "
                        "optimization level -O1 or higher.\n", fname);
        exit(1);
    }
    switch (p->op) {
        case BANDU:  binary("BANDU");  break;
        case BORU:   binary("BORU" );  break;
        case BXORU:  binary("BXORU");  break;
        case ADDD:   binary("ADDD");   break;
        case ADDF:   binary("ADDF");   break;
        case ADDI:   binary("ADDI");   break;
        case ADDP:   binary("ADDP");   break;
        case ADDU:   binary("ADDU");   break;
        case SUBD:   binary("SUBD");   break;
        case SUBF:   binary("SUBF");   break;
        case SUBI:   binary("SUBI");   break;
        case SUBP:   binary("SUBP");   break;
        case SUBU:   binary("SUBU");   break;
        case MULD:   binary("MULD");   break;
        case MULF:   binary("MULF");   break;
        case MULI:   binary("MULI");   break;
        case MULU:   binary("MULU");   break;
        case DIVD:   binary("DIVD");   break;
        case DIVF:   binary("DIVF");   break;
        case DIVI:   binary("DIVI");   break;
        case DIVU:   binary("DIVU");   break;
        case MODI:   binary("MODI");   break;
        case MODU:   binary("MODU");   break;
        case RSHU:   binary("RSHU");   break;
        case RSHI:   binary("RSHI");   break;
        case LSHI:   binary("LSHI");   break;
        case LSHU:   binary("LSHU");   break;
        case INDIRC: unary("INDIRC");  break;
        case INDIRS: unary("INDIRS");  break;
        case INDIRI: unary("INDIRI");  break;
        case INDIRP: unary("INDIRP");  break;
        case INDIRD: unary("INDIRD");  break;
        case INDIRF: unary("INDIRF");  break;
        case INDIRB: unary("INDIRB");  break;
        case BCOMU:  unary("BCOMU" );  break;
        case NEGD:   unary("NEGD" );   break;
        case NEGF:   unary("NEGF" );   break;
        case NEGI:   unary("NEGI" );   break;
        case CVCI:   unary("CVCI");    break;
        case CVSI:   unary("CVSI");    break;
        case CVCU:   unary("CVCU");    break;
        case CVSU:   unary("CVSU");    break;
        case CVUC:   unary("CVUC");    break;
        case CVUS:   unary("CVUS");    break;
        case CVIC:   unary("CVIC");    break;
        case CVIS:   unary("CVIS");    break;
        case CVPU:   unary("CVPU");    break;
        case CVUP:   unary("CVUP");    break;
        case CVIU:   unary("CVIU");    break;
        case CVUI:   unary("CVUI");    break;
        case CVID:   unary("CVID" );   break;
        case CVDF:   unary("CVDF");    break;
        case CVFD:   unary("CVFD");    break;
        case CVDI:   unary("CVDI" );   break;
        case RETD:   print("\tRETD(%s)\n",output_arg(a)); break;
        case RETF:   print("\tRETF(%s)\n",output_arg(a)); break;
        case RETI:   print("\tRETI(%s)\n",output_arg(a)); break;
        case RETV:   print("\tRETV\n"); break;
        case ADDRGP: print("\tADDRGP(%s,%s)\n"
                        ,p->syms[0]->x.name
                        ,output_arg(p)); break;
        case ADDRFP: print("\tADDRFP(%s,%s)\n"
                        ,p->syms[0]->x.name
                        ,output_arg(p)); break;
        case ADDRLP: print("\tADDRLP(%s,%s)\n"
                        ,p->syms[0]->x.name
                        ,output_arg(p)); break;
        case CNSTC: print("\tCNSTC(%s,%s)\n" ,output_name(p->syms[0]) ,output_arg(p)); break;
        case CNSTS: print("\tCNSTS(%s,%s)\n" ,output_name(p->syms[0]) ,output_arg(p)); break;
        case CNSTI: print("\tCNSTI(%s,%s)\n" ,output_name(p->syms[0]) ,output_arg(p)); break;
        case CNSTU: print("\tCNSTU(%s,%s)\n" ,output_name(p->syms[0]) ,output_arg(p)); break;
        case CNSTP: print("\tCNSTP(%s,%s)\n" ,output_name(p->syms[0]) ,output_arg(p)); break;
        case JUMPV: print("\tJUMPV(%s)\n" , output_arg(a)); break;
        case ASGNB: print("\tASGNB(%s,%s,%s)\n"
                    ,output_arg(b)
                    ,output_arg(a)
                    ,output_name(p->syms[0])); break;
        case ASGNC: print("\tASGNC(%s,%s)\n" ,output_arg(b) ,output_arg(a)); break;
        case ASGNS: print("\tASGNS(%s,%s)\n" ,output_arg(b) ,output_arg(a)); break;
        case ASGND: print("\tASGND(%s,%s)\n" ,output_arg(b) ,output_arg(a)); break;
        case ASGNF: print("\tASGNF(%s,%s)\n" ,output_arg(b) ,output_arg(a)); break;
        case ASGNI: print("\tASGNI(%s,%s)\n" ,output_arg(b) ,output_arg(a)); break;
        case ASGNP: print("\tASGNP(%s,%s)\n" ,output_arg(b) ,output_arg(a)); break;
        case ARGB:  print("\tARGB(%s,(sp),%d,%s)\n"
                    ,output_arg(a)
                    ,p->x.argoffset
                    ,output_name(p->syms[0])); break;
        case ARGD:  print("\tARGD(%s,%d)\n" ,output_arg(a) ,p->x.argoffset); break;
        case ARGF:  print("\tARGF(%s,%d)\n" ,output_arg(a) ,p->x.argoffset); break;
        case ARGI:  print("\tARGI(%s,%d)\n" ,output_arg(a) ,p->x.argoffset); break;
        case ARGP:  print("\tARGP(%s,%d)\n" ,output_arg(a) ,p->x.argoffset); break;
        case CALLB:
            save_busy(p);
            print("\tMOVW_%cD(%s,op1)\n"
                    ,simple_adrmode(b->x.adrmode)
                    ,output_arg(b));
            print("\tCALLV(%s,%d)\n"
                    ,output_arg(a)
                    ,p->x.argoffset);
            restore_busy(p);
            break;
        case CALLV:
            save_busy(p);
            print("\tCALLV(%s,%d)\n" ,output_arg(a) ,p->x.argoffset);
            restore_busy(p);
            break;
        case CALLD:
            save_busy(p);
            print("\tCALLD(%s,%d,%s)\n"
                    ,output_arg(a)
                    ,p->x.argoffset
                    ,output_arg(p));
            restore_busy(p);
            break;
        case CALLF:
            save_busy(p);
            print("\tCALLF(%s,%d,%s)\n"
                    ,output_arg(a)
                    ,p->x.argoffset
                    ,output_arg(p));
            restore_busy(p);
            break;
        case CALLI:
            save_busy(p);
            print("\tCALLI(%s,%d,%s)\n"
                    ,output_arg(a)
                    ,p->x.argoffset
                    ,output_arg(p));
            restore_busy(p);
            break;
        case EQD:     compare("EQD" ); break;
        case EQF:     compare("EQF" ); break;
        case EQI:     compare("EQI" ); break;
        case GED:     compare("GED" ); break;
        case GEF:     compare("GEF" ); break;
        case GEI:     compare("GEI" ); break;
        case GEU:     compare("GEU" ); break;
        case GTD:     compare("GTD" ); break;
        case GTF:     compare("GTF" ); break;
        case GTI:     compare("GTI" ); break;
        case GTU:     compare("GTU" ); break;
        case LED:     compare("LED" ); break;
        case LEF:     compare("LEF" ); break;
        case LEI:     compare("LEI" ); break;
        case LEU:     compare("LEU" ); break;
        case LTD:     compare("LTD" ); break;
        case LTF:     compare("LTF" ); break;
        case LTI:     compare("LTI" ); break;
        case LTU:     compare("LTU" ); break;
        case NED:     compare("NED" ); break;
        case NEF:     compare("NEF" ); break;
        case NEI:     compare("NEI" ); break;
        case LABELV: print("%s\n", p->syms[0]->x.name); break;
        default: assert(0);
    }
}

/* emitdag - emit one 16-bit macro call per dag operator (see MACROS.H for
 * the full naming glossary). The lcc operator names combine an operation
 * with a type letter: I = signed int, U = unsigned int, P = pointer,
 * C/S = char/short (bytes), F/D = float/double, B = struct, V = void.
 * So DIVU = unsigned divide, MODI = signed modulo, RSHI = signed right
 * shift (arithmetic, emits ASRW), RSHU = unsigned right shift (logical,
 * emits RSHW), CVIU = convert int to unsigned, INDIRC = load a char, etc.
 * The emitted macro name appends one addressing-mode letter per operand
 * (C constant, D direct, Z zero page pointer, A frame slot, Y indirect). */
static void emitdag(Node p) {

    a = p->kids[0]; b = p->kids[1]; r=p;

    switch (p->op) {
        case BANDU:        if (p->x.width==4) binary("ANDL");
                           else if (p->x.narrow) binary("ANDB"); else binary("ANDW");   break;
        case BORU:         if (p->x.width==4) binary("ORL");
                           else if (p->x.narrow) binary("ORB" ); else binary("ORW" );   break;
        case BXORU:        if (p->x.width==4) binary("XORL");
                           else if (p->x.narrow) binary("XORB"); else binary("XORW");   break;
        case ADDD:  case ADDF:            binary("ADDF");   break;
        case ADDI:  case ADDP:  case ADDU:
            if (p->x.width==4) { if (p->x.fusek) emit_fused_addk(1); else if (!p->x.inplace) binary("ADDL"); break; }
            if (p->x.narrow) {
                if (strcmp(a->x.name,p->x.name)==0 && strcmp(b->x.name,"1")==0
                    && (p->x.adrmode=='Z' || p->x.adrmode=='D'))
                    print("\tINCB_%c(%s)\n" ,simple_adrmode(p->x.adrmode) ,output_arg(p));  /* c++ in place */
                else binary("ADDB");
            }
            else if (optimizelevel>=2
                    && strcmp(a->x.name,p->x.name)==0
                    && strcmp(b->x.name,"1")==0
                    && (p->x.adrmode=='Z' || p->x.adrmode=='D'))
                print("\tINCW_%c(%s)\n" ,simple_adrmode(p->x.adrmode) ,output_arg(p));
            else
                binary("ADDW");
            break;
        case SUBD:  case SUBF:            binary("SUBF");  break;
        case SUBI:  case SUBP:  case SUBU:
            if (p->x.width==4) { if (p->x.fusek) emit_fused_addk(0); else if (!p->x.inplace) binary("SUBL"); break; }
            if (p->x.narrow) {
                if (strcmp(a->x.name,p->x.name)==0 && strcmp(b->x.name,"1")==0
                    && (p->x.adrmode=='Z' || p->x.adrmode=='D'))
                    print("\tDECB_%c(%s)\n" ,simple_adrmode(p->x.adrmode) ,output_arg(p));  /* c-- in place */
                else binary("SUBB");
            }
            else if (optimizelevel>=2
                    && strcmp(a->x.name,p->x.name)==0
                    && strcmp(b->x.name,"1")==0
                    && (p->x.adrmode=='Z' || p->x.adrmode=='D'))
                print("\tDECW_%c(%s)\n" ,simple_adrmode(p->x.adrmode) ,output_arg(p));
            else
                binary("SUBW");
            break;
        case MULD:  case MULF:            binary("MULF");   break;
        case MULI:  if (p->x.width==4) binary("MULL"); else binary("MULI");   break;
        case MULU:  if (p->x.width==4) binary("MULL"); else binary("MULU");   break;
        case DIVD:  case DIVF:            binary("DIVF");   break;
        case DIVI:  if (p->x.width==4) binary("DIVL");  else binary("DIVI");   break;
        case DIVU:  if (p->x.width==4) binary("DIVUL"); else binary("DIVU");   break;
        case MODI:  if (p->x.width==4) binary("MODL");  else binary("MODI");   break;
        case MODU:  if (p->x.width==4) binary("MODUL"); else binary("MODU");   break;
        case RSHU:
            if (p->x.width==4) { binary("RSHL"); break; }           /* 32-bit logical >> */
            if (p->x.narrow) byte_shift1("RSH1B");                  /* uchar x >> 1 (lsr) */
            else if (optimizelevel>=2 && strcmp(b->x.name,"8")==0
                     && simple_adrmode(a->x.adrmode)!='C') unary("RSHW8");  /* >>8 = byte move */
            else binary("RSHW");
            break;
        case RSHI:
            if (p->x.width==4) { binary("ASRL"); break; }           /* 32-bit arithmetic >> */
            if (p->x.narrow) byte_shift1("RSH1B");                  /* uchar x >> 1 is logical (lsr) */
            else if (optimizelevel>=2 && strcmp(b->x.name,"8")==0
                     && simple_adrmode(a->x.adrmode)!='C') unary("ASRW8");  /* signed >>8 = byte move + sign fill */
            else binary("ASRW");                                   /* signed >> keeps the sign */
            break;
        case LSHI:  case LSHU:
            if (p->x.width==4) { binary("LSHL"); break; }           /* 32-bit << */
            if (p->x.narrow)
                byte_shift1("LSH1B");   /* char x << 1 (asl) */
            else if (optimizelevel>=2 && strcmp(b->x.name,"1")==0)
                unary("LSH1W");
            else if (optimizelevel>=2 && strcmp(b->x.name,"8")==0
                     && simple_adrmode(a->x.adrmode)!='C')
                unary("LSHW8");         /* x << 8 = byte move */
            else if (optimizelevel>=2
                     && (strcmp(b->x.name,"2")==0 || strcmp(b->x.name,"3")==0)
                     && simple_adrmode(a->x.adrmode)!='C'
                     && simple_adrmode(r->x.adrmode)=='D') {
                /* unroll a small constant left shift: one LSH1W (a->r) then the
                 * rest in place. Smaller AND faster than the LSHW loop for n<=3
                 * (no ldx/beq/dex/bne scaffold), so it needs no size/speed knob. */
                int n = b->x.name[0]-'0', i;
                unary("LSH1W");                              /* r = a << 1     */
                for (i=1; i<n; i++)
                    print("\tLSH1W_D(%s)\n", output_arg(r));  /* r <<= 1 (asl/rol) */
            }
            else binary("LSHW");
            break;
        case INDIRC: case INDIRS:
            if (!p->x.optimized)
                print("\tINDIRB_%c%c(%s,%s)\n"
                        ,reduced_adrmode(a->x.adrmode)    // keep 'Z' adrmode different from 'D'
                        ,simple_adrmode(r->x.adrmode)
                        ,output_arg(a)
                        ,output_arg(r));
            break;
        case INDIRI: case INDIRP:
            if (!p->x.optimized && !p->x.inplace)   // in-place RMW consumes the load
                print("\tINDIR%s_%c%c(%s,%s)\n"
                        ,p->x.width==4 ? "L" : "W"
                        ,reduced_adrmode(a->x.adrmode)    // keep 'Z' adrmode different from 'D'
                        ,simple_adrmode(r->x.adrmode)
                        ,output_arg(a)
                        ,output_arg(r));
            break;
        case INDIRD: case INDIRF:
            if (!p->x.optimized)
                unary("INDIRF");
            break;
        case INDIRB:
            if (!p->x.optimized)
                unary("INDIRS");
            break;
        case BCOMU:        if (p->x.width==4) unary("COML");
                           else if (p->x.narrow) unary("COMB"); else unary("COMW");   break;
        case NEGD:  case NEGF:            unary("NEGF" );   break;
        case NEGI:         if (p->x.width==4) unary("NEGL");
                           else if (p->x.narrow) unary("NEGB"); else unary("NEGI");   break;
        case CVCI: case CVSI:             unary("CSBW");    break;
        case CVCU: case CVSU:
            /* Phase 1b: drop a dead zero-extend. widen_dead_after() proves the
             * widened value's high byte is never read (only byte-narrowed ops
             * consume it before the temp is overwritten). Two shapes:
             *  -O2: a separate INDIRB already loaded the byte and this is an
             *       in-place widen (operand==result, D) -> emit nothing.
             *  -O3: the byte load is fused into this node (operand!=result, and
             *       the INDIR was folded in) -> emit just the byte load with
             *       CWB (the word->byte family: same addressing as CZBW but
             *       without the high-byte zero). Eliding here keeps -O3 from
             *       paying for dead widens that -O2 already avoids, so -O3 is
             *       no slower than -O2 on char-heavy code.
             * Not-narrowed or live-high-byte widens still emit the full CZBW. */
            if (p->x.narrow && widen_dead_after(p)) {
                if (!(simple_adrmode(a->x.adrmode)=='D'
                      && strcmp(a->x.name,p->x.name)==0))
                    unary("CWB");     /* fused load: byte only, no zero-extend */
            } else {
                unary("CZBW");
            }
            break;
        case CVUC: case CVUS: case CVIC: case CVIS:
            /* A conversion is a no-op only if operand and result are the SAME
             * location: same name AND same addressing mode. Comparing names
             * alone is not enough: after the -O3 INDIR folding, the operand
             * can be "(tmp0),0" (mode 'I', the value POINTED TO by tmp0)
             * while the result is "tmp0" (mode 'Z') - same name, and eliding
             * the conversion would silently drop the dereference. */
            if (optimizelevel<=1 || strcmp(a->x.name,p->x.name)!=0
                || a->x.adrmode!=p->x.adrmode)
                unary("CWB");
            break;
        case CVPU: case CVUP: case CVIU: case CVUI:
            if (optimizelevel<=1 || strcmp(a->x.name,p->x.name)!=0
                || a->x.adrmode!=p->x.adrmode)
                unary("MOVW");
            break;
        case CVIL:                        unary("CSWL");  break;  /* int -> long: sign extend  */
        case CVUL:                        unary("CZWL");  break;  /* uint -> long: zero extend */
        case CVLI: case CVLU:
            /* long -> int/unsigned: take the low word. A no-op only when
               operand and result are the same location (same rule as CVUC) */
            if (optimizelevel<=1 || strcmp(a->x.name,p->x.name)!=0
                || a->x.adrmode!=p->x.adrmode)
                unary("CLW");
            break;
        case CVLD:
            fprintf(stderr, "Error in function '%s': long to float conversion "
                            "is not supported yet.\n", fname);
            exit(1);
            break;
        case CVID:                        unary("CIF" );  break;
        case CVDF: case CVFD:
            if (optimizelevel<=1 || strcmp(a->x.name,p->x.name)!=0
                || a->x.adrmode!=p->x.adrmode)
                unary("MOVF");
            break;
        case CVDI:                        unary("CFI" );    break;
        case RETD: case RETF:
            if (optimizelevel>=2 && omit_frame && nbregs==0)
                print("\tRETF_%c(%s)\n"
                        ,simple_adrmode(a->x.adrmode)
                        ,output_arg(a));
            else
                print("\tLEAVEF_%c(%s)\n"
                        ,simple_adrmode(a->x.adrmode)
                        ,output_arg(a));
            break;
        case RETI:
            if (a && a->x.width==4) {
                /* 32-bit return value: whole value through op1:op2 (X:A only
                   carries 16 bits); CALLL on the caller side reads it back */
                if (optimizelevel>=2 && omit_frame && nbregs==0)
                    print("\tRETL_%c(%s)\n"
                            ,simple_adrmode(a->x.adrmode)
                            ,output_arg(a));
                else
                    print("\tLEAVEL_%c(%s)\n"
                            ,simple_adrmode(a->x.adrmode)
                            ,output_arg(a));
                break;
            }
            if (optimizelevel>=2 && omit_frame && nbregs==0)
                print("\tRETW_%c(%s)\n"
                        ,simple_adrmode(a->x.adrmode)
                        ,output_arg(a));
            else
                print("\tLEAVEW_%c(%s)\n"
                        ,simple_adrmode(a->x.adrmode)
                        ,output_arg(a));
            break;
        case RETV:
            if (optimizelevel>=2 && omit_frame && nbregs==0)
                print("\tRET\n");
            else print("\tLEAVE\n");
            break;
        case ADDRGP: case ADDRFP: case ADDRLP:
            if (optimizelevel==0)
                print("\tADDR_%c%c(%s,%s)\n"
                        ,simple_adrmode(p->syms[0]->x.adrmode)
                        ,simple_adrmode(p->x.adrmode)
                        ,output_name(p->syms[0])
                        ,output_arg(p));
            break;
        case CNSTC: case CNSTS:
        case CNSTI: case CNSTU:
        case CNSTP:
            if (optimizelevel==0)
                print("\tCNST_%c%c(%s,%s)\n"
                        ,simple_adrmode(p->syms[0]->x.adrmode)
                        ,simple_adrmode(p->x.adrmode)
                        ,output_name(p->syms[0])
                        ,output_arg(p));
            break;
        case JUMPV:
            print("\tJUMP_%c(%s)\n" ,simple_adrmode(a->x.adrmode), output_arg(a));
            break;
        case ASGNB:
            print("\tASGNS_%c%c(%s,%s,%s)\n"
                    ,simple_adrmode(b->x.adrmode)
                    ,reduced_adrmode(a->x.adrmode)     // keep 'Z' adrmode different from 'D'
                    ,output_arg(b)
                    ,output_arg(a)
                    ,output_name(p->syms[0]));
            break;
        case ASGNC: case ASGNS:
            if (!p->x.optimized)
                print("\tASGNB_%c%c(%s,%s)\n"
                        ,simple_adrmode(b->x.adrmode)
                        ,reduced_adrmode(a->x.adrmode)     // keep 'Z' adrmode different from 'D'
                        ,output_arg(b)
                        ,output_arg(a));
            break;
        case ASGND: case ASGNF:
            if (!p->x.optimized)
                print("\tASGNF_%c%c(%s,%s)\n"
                        ,simple_adrmode(b->x.adrmode)
                        ,reduced_adrmode(a->x.adrmode)     // keep 'Z' adrmode different from 'D'
                        ,output_arg(b)
                        ,output_arg(a));
            break;
        case ASGNI: case ASGNP:
            if (p->x.inplace) {
                /* in-place long RMW: b is the ADD/SUB, one kid the constant */
                Node cnst = is_int_const(b->kids[0]) ? b->kids[0] : b->kids[1];
                print("\t%sLK_%c(%s,%s)\n"
                        ,is_long_add(b->op) ? "ADD" : "SUB"
                        ,reduced_adrmode(a->x.adrmode)
                        ,output_arg(a)
                        ,output_arg(cnst));
            }
            else if (!p->x.optimized)
                print("\tASGN%s_%c%c(%s,%s)\n"
                        /* the store size travels in syms[0] (the ASGN dag
                           node is never the listnodes return value, so it
                           bypasses the width choke point) */
                        ,p->syms[0] && p->syms[0]->u.c.v.i==4 ? "L" : "W"
                        ,simple_adrmode(b->x.adrmode)
                        ,reduced_adrmode(a->x.adrmode)     // keep 'Z' adrmode different from 'D'
                        ,output_arg(b)
                        ,output_arg(a));
            break;
        case ARGB:
            print("\tARGS_%c(%s,(sp),%d,%s)\n"
                    ,simple_adrmode(a->x.adrmode)
                    ,output_arg(a)
                    ,p->x.argoffset
                    ,output_name(p->syms[0]));
            break;
        case ARGD: case ARGF:
            if (!p->x.optimized)
                print("\tARGF_%c(%s,(sp),%d)\n"
                        ,simple_adrmode(a->x.adrmode)
                        ,output_arg(a)
                        ,p->x.argoffset);
            break;
        case ARGI: case ARGP:
            if (!p->x.optimized)
                print("\tARG%s_%c(%s,(sp),%d)\n"
                        /* the arg's byte size travels in syms[0] */
                        ,p->syms[0]->u.c.v.i==4 ? "L" : "W"
                        ,simple_adrmode(a->x.adrmode)
                        ,output_arg(a)
                        ,p->x.argoffset);
            break;
        case CALLB:
            save_busy(p);
            print("\tMOVW_%cD(%s,op1)\n"
                    ,simple_adrmode(b->x.adrmode)
                    ,output_arg(b));
            print("\tCALLV_%c(%s,%d)\n"
                    ,simple_adrmode(a->x.adrmode)
                    ,output_arg(a)
                    ,p->x.argoffset);
            restore_busy(p);
            break;
        case CALLV:
            save_busy(p);
            print("\tCALLV_%c(%s,%d)\n"
                    ,simple_adrmode(a->x.adrmode)
                    ,output_arg(a)
                    ,p->x.argoffset);
            restore_busy(p);
            break;
        case CALLD: case CALLF:
            save_busy(p);
            print("\tCALLF_%c%c(%s,%d,%s)\n"
                    ,simple_adrmode(a->x.adrmode)
                    ,simple_adrmode(p->x.adrmode)
                    ,output_arg(a)
                    ,p->x.argoffset
                    ,output_arg(p));
            restore_busy(p);
            break;
        case CALLI:
            save_busy(p);
            print("\tCALL%s_%c%c(%s,%d,%s)\n"
                    ,p->x.width==4 ? "L" : "W"
                    ,simple_adrmode(a->x.adrmode)
                    ,simple_adrmode(p->x.adrmode)
                    ,output_arg(a)
                    ,p->x.argoffset
                    ,output_arg(p));
            restore_busy(p);
            break;
        case EQD:   case EQF:             compare("EQF" ); break;
        case EQI:
            /* == and != are symmetric: put a constant first operand on the
             * right, the macro library only has constant-second variants
             * (a constant first operand survives the front end when the
             * expression cannot be normalized, e.g. 0 == -x with x unsigned) */
            if (simple_adrmode(a->x.adrmode)=='C' && simple_adrmode(b->x.adrmode)!='C') {
                Node t=a; a=b; b=t;
            }
            if (a->x.width==4 || b->x.width==4) { compare("EQL"); break; }
            if (p->x.narrow) {
                if (strcmp(b->x.name,"0")==0)
                    compare0("EQ0B");   /* lda sets Z: no cmp #0 */
                else
                    compare("EQB");
            }
            else if (optimizelevel>=2 && strcmp(b->x.name,"0")==0)
                compare0("EQ0W");
            else compare("EQW" );
            break;
        case GED:   case GEF:             compare("GEF" ); break;
        case GEI:          if (a->x.width==4) compare("GEL");
                           else if (p->x.narrow) compare("GEUB"); else compare("GEI"); break;
        case GEU:          if (a->x.width==4) compare("GEUL");
                           else if (p->x.narrow) compare("GEUB"); else compare("GEU"); break;
        case GTD:   case GTF:             compare("GTF" ); break;
        case GTI:          if (a->x.width==4) compare("GTL");
                           else if (p->x.narrow) compare("GTUB"); else compare("GTI"); break;
        case GTU:          if (a->x.width==4) compare("GTUL");
                           else if (p->x.narrow) compare("GTUB"); else compare("GTU"); break;
        case LED:   case LEF:             compare("LEF" ); break;
        case LEI:          if (a->x.width==4) compare("LEL");
                           else if (p->x.narrow) compare("LEUB"); else compare("LEI"); break;
        case LEU:          if (a->x.width==4) compare("LEUL");
                           else if (p->x.narrow) compare("LEUB"); else compare("LEU"); break;
        case LTD:   case LTF:             compare("LTF" ); break;
        case LTI:          if (a->x.width==4) compare("LTL");
                           else if (p->x.narrow) compare("LTUB"); else compare("LTI"); break;
        case LTU:          if (a->x.width==4) compare("LTUL");
                           else if (p->x.narrow) compare("LTUB"); else compare("LTU"); break;
        case NED:   case NEF:             compare("NEF" ); break;
        case NEI:
            /* symmetric: constant first operand goes right (see EQI) */
            if (simple_adrmode(a->x.adrmode)=='C' && simple_adrmode(b->x.adrmode)!='C') {
                Node t=a; a=b; b=t;
            }
            if (a->x.width==4 || b->x.width==4) { compare("NEL"); break; }
            if (p->x.narrow) {
                if (strcmp(b->x.name,"0")==0)
                    compare0("NE0B");   /* lda sets Z: no cmp #0 */
                else
                    compare("NEB");
            }
            else if (optimizelevel>=2 && strcmp(b->x.name,"0")==0)
                compare0("NE0W");
            else compare("NEW" );
            break;
        case LABELV:
            print("%s\n", p->syms[0]->x.name); break;
        default: assert(0);
    }
}

void emit(Node p) {
    if (graph_output) return;
    for (; p; p=p->x.next)
        if (optimizelevel==0) emitdag0(p);
        else emitdag(p);
}

/* ----------------------------------------------------------------
 * .ctype annotation emission — write type info to assembly output
 * so the debug toolchain can display structured variable data.
 * ---------------------------------------------------------------- */

/* Map struct/union types to their typedef names.
   Populated by emit_stabtype() so that ctype_name() can use "score_entry"
   instead of the compiler-generated numeric tag "129". */
#define MAX_TYPEDEF_MAP 256
static struct { Type type; const char *name; } typedef_map[MAX_TYPEDEF_MAP];
static int typedef_map_count = 0;

static const char *typedef_lookup(Type t) {
    int i;
    for (i = 0; i < typedef_map_count; i++)
        if (typedef_map[i].type == t) return typedef_map[i].name;
    return NULL;
}

static void typedef_register(Type t, const char *name) {
    int i;
    /* Update existing entry or add new one */
    for (i = 0; i < typedef_map_count; i++)
        if (typedef_map[i].type == t) { typedef_map[i].name = name; return; }
    if (typedef_map_count < MAX_TYPEDEF_MAP) {
        typedef_map[typedef_map_count].type = t;
        typedef_map[typedef_map_count].name = name;
        typedef_map_count++;
    }
}

/* Build a type name string into buf (e.g. "int", "uchar", "*char",
   "score_entry[24]").  Returns buf for convenience. */
static char *ctype_name(Type t, char *buf, int bufsize)
{
    Type u;
    if (!t) { buf[0] = '?'; buf[1] = 0; return buf; }

    /* Strip const/volatile qualifiers */
    u = unqual(t);

    /* Exact pointer comparison with known type globals first —
       this is the most reliable way to distinguish signed/unsigned. */
    if (u == chartype)       { strncpy(buf, "char",   bufsize); }
    else if (u == unsignedchar)   { strncpy(buf, "uchar",  bufsize); }
    else if (u == shorttype)      { strncpy(buf, "short",  bufsize); }
    else if (u == unsignedshort)  { strncpy(buf, "ushort", bufsize); }
    else if (u == inttype)        { strncpy(buf, "int",    bufsize); }
    else if (u == unsignedtype)   { strncpy(buf, "uint",   bufsize); }
    else if (u == longtype)       { strncpy(buf, "long",   bufsize); }
    else if (u == unsignedlong)   { strncpy(buf, "ulong",  bufsize); }
    else if (u == floattype)      { strncpy(buf, "float",  bufsize); }
    else if (u == doubletype)     { strncpy(buf, "double", bufsize); }
    else if (u == voidtype)       { strncpy(buf, "void",   bufsize); }
    else switch (u->op) {
    case POINTER: {
        char inner[128];
        ctype_name(u->type, inner, sizeof(inner));
        sprintf(buf, "*%s", inner);
        break;
    }
    case ARRAY: {
        char inner[128];
        int count = (u->type && u->type->size > 0) ? u->size / u->type->size : 0;
        ctype_name(u->type, inner, sizeof(inner));
        sprintf(buf, "%s[%d]", inner, count);
        break;
    }
    case STRUCT: {
        const char *tn = typedef_lookup(u);
        if (tn)
            strncpy(buf, tn, bufsize);
        else if (u->u.sym && u->u.sym->name && u->u.sym->name[0]
                 && !(u->u.sym->name[0] >= '0' && u->u.sym->name[0] <= '9'))
            strncpy(buf, u->u.sym->name, bufsize);
        else
            sprintf(buf, "struct_%d", u->size);
        break;
    }
    case UNION: {
        const char *tn = typedef_lookup(u);
        if (tn)
            strncpy(buf, tn, bufsize);
        else if (u->u.sym && u->u.sym->name && u->u.sym->name[0]
                 && !(u->u.sym->name[0] >= '0' && u->u.sym->name[0] <= '9'))
            strncpy(buf, u->u.sym->name, bufsize);
        else
            sprintf(buf, "union_%d", u->size);
        break;
    }
    case ENUM: {
        /* Name the enum by its typedef/tag (e.g. "EntityKind") like a struct,
           so the debugger can show the symbolic type and map values to names.
           The underlying storage is still an int (see emit_ctype_flush, which
           records the byte size); this only affects the displayed type. */
        const char *tn = typedef_lookup(u);
        if (tn)
            strncpy(buf, tn, bufsize);
        else if (u->u.sym && u->u.sym->name && u->u.sym->name[0]
                 && !(u->u.sym->name[0] >= '0' && u->u.sym->name[0] <= '9'))
            strncpy(buf, u->u.sym->name, bufsize);
        else
            strncpy(buf, "int", bufsize);   /* anonymous enum: fall back to int */
        break;
    }
    case FUNCTION:
        strncpy(buf, "func", bufsize);
        break;
    default: {
        /* Fallback by op */
        static const char *fallback[] = {
            "?","float","double","char","short","int","uint",
            "ptr","void","struct","union","func","array","int","long"
        };
        if (u->op >= 1 && u->op <= 14)
            strncpy(buf, fallback[u->op], bufsize);
        else
            strncpy(buf, "?", bufsize);
        break;
    }
    }
    buf[bufsize-1] = 0;
    return buf;
}

/* Deferred struct/union type definitions — we collect the Type pointers during
   compilation (registering typedef names immediately), then emit the .ctype struct
   lines at progend() so that ALL typedef names are resolved before any field type
   strings are generated.  This prevents inner struct references from showing
   numeric tags (e.g. "struct_19" instead of "score_entry"). */
#define MAX_DEFERRED_TYPES 256
static struct { Type type; const char *name; } deferred_types[MAX_DEFERRED_TYPES];
static int deferred_type_count = 0;

/* Deferred enum type definitions — collected like structs, emitted at progend()
   as ".ctype enum <name> <size> <NAME>=<val> ..." so the debugger can render an
   enum-typed value both symbolically (KIND_HERO) and numerically. */
#define MAX_DEFERRED_ENUMS 128
static struct { Type type; const char *name; } deferred_enums[MAX_DEFERRED_ENUMS];
static int deferred_enum_count = 0;

/* Register a struct/union type for deferred emission.
   Called from stabtype() for TYPEDEF and anonymous struct/union symbols. */
void emit_stabtype(Symbol p)
{
    Type t;
    const char *name;
    int i;

    if (graph_output) return;
    if (!p || !p->type) return;

    t = unqual(p->type);
    if (!t) return;
    if (t->op != STRUCT && t->op != UNION && t->op != ENUM) return;

    /* Prefer the typedef name (e.g. "score_entry") over the compiler-generated
       numeric tag (e.g. "129") used for anonymous structs.  Fall back to the
       tag name when the typedef is unnamed or absent. */
    name = NULL;
    if (p->name && p->name[0] && !(p->name[0] >= '0' && p->name[0] <= '9'))
        name = p->name;
    else if (t->u.sym && t->u.sym->name && t->u.sym->name[0]
             && !(t->u.sym->name[0] >= '0' && t->u.sym->name[0] <= '9'))
        name = t->u.sym->name;
    if (!name) return;  /* skip anonymous types with no typedef */

    /* Register this typedef immediately so ctype_name() can resolve it */
    typedef_register(t, name);

    if (t->op == ENUM) {
        /* Collect the enum for deferred ".ctype enum" emission (dedup by Type) */
        for (i = 0; i < deferred_enum_count; i++)
            if (deferred_enums[i].type == t) return;
        if (deferred_enum_count < MAX_DEFERRED_ENUMS) {
            deferred_enums[deferred_enum_count].type = t;
            deferred_enums[deferred_enum_count].name = name;
            deferred_enum_count++;
        }
        return;
    }

    /* Deduplicate: skip if this Type is already collected */
    for (i = 0; i < deferred_type_count; i++)
        if (deferred_types[i].type == t) return;

    if (deferred_type_count < MAX_DEFERRED_TYPES) {
        deferred_types[deferred_type_count].type = t;
        deferred_types[deferred_type_count].name = name;
        deferred_type_count++;
    }
}

/* Deferred variable annotations — collected during compilation, emitted at
   progend() after all typedef mappings are established. */
#define MAX_DEFERRED_VARS 512
static struct { const char *asmname; const char *cname; Type type; const char *func; } deferred_vars[MAX_DEFERRED_VARS];
static int deferred_var_count = 0;

/* Collect a variable for deferred .ctype emission.
   Called from stabsym() for globals, externs, statics, and locals. */
void emit_stabsym(Symbol p)
{
    if (graph_output) return;
    if (!p || !p->type) return;
    /* Only emit for data variables, not functions or compiler internals */
    if (isfunc(p->type)) return;
    if (!p->name) return;
    if (p->scope == CONSTANTS || p->scope == LABELS) return;
    /* Skip register-allocated variables */
    if (p->x.adrmode == 'R') return;
    /* A block-scoped local with no stack slot (optimized away, or a register with no
       address) has no inspectable location — skip it so we don't emit it as a bogus
       global var. Parameters (scope PARAM) and file-scope vars are unaffected. */
    if (p->scope >= LOCAL && (!p->x.name || p->x.name[0] != '(')) return;
    /* Skip compiler-generated names (numeric temps, string literals) */
    if (p->name[0] >= '0' && p->name[0] <= '9') return;
    if (p->x.name && p->x.name[0] == 'L' && p->generated) return;

    if (deferred_var_count < MAX_DEFERRED_VARS) {
        /* Use x.name if available, else build the asm name from C name */
        deferred_vars[deferred_var_count].asmname = p->x.name ? p->x.name : stringf("_%s", p->name);
        deferred_vars[deferred_var_count].cname = p->name;
        deferred_vars[deferred_var_count].type = p->type;
        deferred_vars[deferred_var_count].func = fname;
        deferred_var_count++;
    }
}

/* Flush all deferred .ctype annotations.  Called from progend().
   Struct definitions are emitted first (all typedef names are now registered),
   then variable annotations (which reference those type names). */
static void emit_ctype_flush(void)
{
    int i;
    char tname[128];

    /* Phase 1: emit deferred struct/union definitions */
    for (i = 0; i < deferred_type_count; i++) {
        Type t = deferred_types[i].type;
        const char *name = deferred_types[i].name;
        const char *kind = (t->op == STRUCT) ? "struct" : "union";
        Field f;

        print(".ctype %s %s %d", kind, name, t->size);
        f = fieldlist(t);
        while (f) {
            ctype_name(f->type, tname, sizeof(tname));
            print(" %s:%s:%d:%d", f->name, tname, f->offset, f->type ? f->type->size : 0);
            f = f->link;
        }
        print("\n");
    }
    deferred_type_count = 0;

    /* Phase 1b: emit deferred enum definitions.
       Format: .ctype enum <name> <size> <ENUMERATOR>=<value> ...
       The enumerator list lives on the tag symbol (u.idlist), NULL-terminated. */
    for (i = 0; i < deferred_enum_count; i++) {
        Type t = deferred_enums[i].type;
        const char *name = deferred_enums[i].name;
        print(".ctype enum %s %d", name, t->size);
        if (t->u.sym && t->u.sym->u.idlist) {
            Symbol *ids = t->u.sym->u.idlist;
            int j;
            for (j = 0; ids[j]; j++)
                print(" %s=%d", ids[j]->name, ids[j]->u.value);
        }
        print("\n");
    }
    deferred_enum_count = 0;

    /* Phase 2: emit deferred variable annotations */
    for (i = 0; i < deferred_var_count; i++) {
        const char *asmname = deferred_vars[i].asmname;
        ctype_name(deferred_vars[i].type, tname, sizeof(tname));
        if (asmname[0] == '(') {
            /* Local or parameter: (fp),N or (ap),N
               Emit: .ctype local <func> <cname> <base> <offset> <type> <size>
               where base is "fp" or "ap" and offset is the numeric part */
            int off = 0;
            const char *base = "fp";
            if (asmname[1] == 'a') base = "ap";
            /* Parse offset after ")," */
            { const char *cp = asmname;
              while (*cp && *cp != ',') cp++;
              if (*cp == ',') off = atoi(cp + 1);
            }
            print(".ctype local %s %s %s %d %s %d\n",
                  deferred_vars[i].func ? deferred_vars[i].func : "?",
                  deferred_vars[i].cname ? deferred_vars[i].cname : "?",
                  base, off, tname, deferred_vars[i].type->size);
        } else {
            /* Global/static variable */
            print(".ctype var %s %s %d\n", asmname, tname,
                  deferred_vars[i].type->size);
        }
    }
    deferred_var_count = 0;
}
