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
static Symbol temp[32];      /* 32 symbols pointing to temporary variables... */
static Symbol flt_temp[32];  /* 32 symbols pointing to temporary floating-point variables... */
static char *regname[8];     /* 8 register variables names */

static char *opcode_names[] = {
    NULL,"CNST","ARG","ASGN","INDIR","CVC","CVD","CVF","CVI","CVP",
    "CVS","CVU","NEG","CALL","LOAD","RET","ADDRG","ADDRF","ADDRL","ADD",
    "SUB","LSH","MOD","RSH","BAND","BCOM","BOR","BXOR","DIV","MUL",
    "EQ","GE","GT","LE","LT","NE","JUMP","LABEL","MAXOP" };
static char type_name[] = " FDCSIUPVB??????";
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

void progend(void) {
    if (graph_output) printf("}\n");
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
    } else if (p->sclass == STATIC)
        p->x.name = stringf("L%s%d", NamePrefix, genlabel(1));
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
    if (nbregs==8 || p->type->size==5) return 0;
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
            if (p->x.result == temp[i])   busy &= ~(1<<i);
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
                if (p->count <= 1 && is_dereferenceable(left->x.adrmode)) {
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
/* an integer constant whose value fits an unsigned char (0..255) - the only
 * constants a byte compare can use unchanged */
static int is_byte_const(Node n) {
    return is_int_const(n) && n->syms[0]
        && n->syms[0]->u.c.v.i >= 0 && n->syms[0]->u.c.v.i <= 255;
}
static int is_eq_cmp(int op)  { return op==EQI || op==NEI; }
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
            ka = under_conv(m->kids[0]);
            kb = under_conv(m->kids[1]);
            /* == / != are symmetric, so accept char on either side */
            if (is_byte_widen(ka) && is_byte_widen(kb)) {
                m->x.narrow = ka->x.narrow = kb->x.narrow = 1;
            } else if (is_byte_widen(ka) && is_byte_const(kb)) {
                m->x.narrow = ka->x.narrow = 1;
            } else if (is_byte_widen(kb) && is_byte_const(ka)) {
                m->x.narrow = kb->x.narrow = 1;
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

Node gen(Node p) {
    Node head, *last;
    for (last = &head; p; p = p->link)
        last = linearize(p, last, 0);
    if (!graph_output) mark_byte_narrowing(head);
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
        || strcmp(inst,"MULI")==0 || strcmp(inst,"MULU")==0;
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
        print("\tMOVW_C%c(%s,%s)\n", rm, output_arg(a), output_arg(r));
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
                 || strcmp(inst,"NEGI")==0)) {
        /* these families have no constant-operand variant */
        print("\tMOVW_C%c(%s,%s)\n", rm, output_arg(a), output_arg(r));
        print("\t%s_%c%c(%s,%s)\n", inst, rm, rm, output_arg(r), output_arg(r));
        return;
    }
    print("\t%s_%c%c(%s,%s)\n"
        ,inst ,am ,rm
        ,output_arg(a)
        ,output_arg(r));
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

static void compare(char *inst) {
     if (optimizelevel==0)
        print("\t%s(%s,%s,%s)\n"
            ,inst
            ,output_arg(a)
            ,output_arg(b)
            ,r->syms[0]->x.name);
    else if (simple_adrmode(a->x.adrmode)=='C' && simple_adrmode(b->x.adrmode)=='C') {
        print("\tMOVW_CD(%s,op1)\n", output_arg(a));
        print("\t%s_DC(op1,%s,%s)\n"
            ,inst
            ,output_arg(b)
            ,r->syms[0]->x.name);
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
        case BANDU:        if (p->x.narrow) binary("ANDB"); else binary("ANDW");   break;
        case BORU:         if (p->x.narrow) binary("ORB" ); else binary("ORW" );   break;
        case BXORU:        if (p->x.narrow) binary("XORB"); else binary("XORW");   break;
        case ADDD:  case ADDF:            binary("ADDF");   break;
        case ADDI:  case ADDP:  case ADDU:
            if (p->x.narrow)
                binary("ADDB");
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
            if (p->x.narrow)
                binary("SUBB");
            else if (optimizelevel>=2
                    && strcmp(a->x.name,p->x.name)==0
                    && strcmp(b->x.name,"1")==0
                    && (p->x.adrmode=='Z' || p->x.adrmode=='D'))
                print("\tDECW_%c(%s)\n" ,simple_adrmode(p->x.adrmode) ,output_arg(p));
            else
                binary("SUBW");
            break;
        case MULD:  case MULF:            binary("MULF");   break;
        case MULI:                        binary("MULI");   break;
        case MULU:                        binary("MULU");   break;
        case DIVD:  case DIVF:            binary("DIVF");   break;
        case DIVI:                        binary("DIVI");   break;
        case DIVU:                        binary("DIVU");   break;
        case MODI:                        binary("MODI");   break;
        case MODU:                        binary("MODU");   break;
        case RSHU:
            if (p->x.narrow) unary("RSH1B");                        /* uchar x >> 1 */
            else if (optimizelevel>=2 && strcmp(b->x.name,"8")==0
                     && simple_adrmode(a->x.adrmode)!='C') unary("RSHW8");  /* >>8 = byte move */
            else binary("RSHW");
            break;
        case RSHI:
            if (p->x.narrow) unary("RSH1B");                        /* uchar x >> 1 is logical */
            else if (optimizelevel>=2 && strcmp(b->x.name,"8")==0
                     && simple_adrmode(a->x.adrmode)!='C') unary("ASRW8");  /* signed >>8 = byte move + sign fill */
            else binary("ASRW");                                   /* signed >> keeps the sign */
            break;
        case LSHI:  case LSHU:
            if (p->x.narrow)
                unary("LSH1B");         /* char x << 1 (byte-narrowed) */
            else if (optimizelevel>=2 && strcmp(b->x.name,"1")==0)
                unary("LSH1W");
            else if (optimizelevel>=2 && strcmp(b->x.name,"8")==0
                     && simple_adrmode(a->x.adrmode)!='C')
                unary("LSHW8");         /* x << 8 = byte move */
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
            if (!p->x.optimized)
                print("\tINDIRW_%c%c(%s,%s)\n"
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
        case BCOMU:        if (p->x.narrow) unary("COMB"); else unary("COMW");   break;
        case NEGD:  case NEGF:            unary("NEGF" );   break;
        case NEGI:         if (p->x.narrow) unary("NEGB"); else unary("NEGI");   break;
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
            if (!p->x.optimized)
                print("\tASGNW_%c%c(%s,%s)\n"
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
                print("\tARGW_%c(%s,(sp),%d)\n"
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
            print("\tCALLW_%c%c(%s,%d,%s)\n"
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
            if (p->x.narrow)
                compare("EQB");
            else if (optimizelevel>=2 && strcmp(b->x.name,"0")==0)
                compare0("EQ0W");
            else compare("EQW" );
            break;
        case GED:   case GEF:             compare("GEF" ); break;
        case GEI:          if (p->x.narrow) compare("GEUB"); else compare("GEI"); break;
        case GEU:          if (p->x.narrow) compare("GEUB"); else compare("GEU"); break;
        case GTD:   case GTF:             compare("GTF" ); break;
        case GTI:          if (p->x.narrow) compare("GTUB"); else compare("GTI"); break;
        case GTU:          if (p->x.narrow) compare("GTUB"); else compare("GTU"); break;
        case LED:   case LEF:             compare("LEF" ); break;
        case LEI:          if (p->x.narrow) compare("LEUB"); else compare("LEI"); break;
        case LEU:          if (p->x.narrow) compare("LEUB"); else compare("LEU"); break;
        case LTD:   case LTF:             compare("LTF" ); break;
        case LTI:          if (p->x.narrow) compare("LTUB"); else compare("LTI"); break;
        case LTU:          if (p->x.narrow) compare("LTUB"); else compare("LTU"); break;
        case NED:   case NEF:             compare("NEF" ); break;
        case NEI:
            /* symmetric: constant first operand goes right (see EQI) */
            if (simple_adrmode(a->x.adrmode)=='C' && simple_adrmode(b->x.adrmode)!='C') {
                Node t=a; a=b; b=t;
            }
            if (p->x.narrow)
                compare("NEB");
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
