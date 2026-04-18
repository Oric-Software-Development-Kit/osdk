 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* structs and defs */

#include "xah.h"
#include "xar.h"
#include "xa.h"

// externals

#include "xam.h"
#include "xal.h"

// exported globals

char   *gError_LabelNamePointer;

// local prototypes 

static int b_fget(int*,int);
static int b_ltest(int,int);
static int b_get(int*);
int b_test(int);

static SEGMENT_e bt[MAXBLK];
static SEGMENT_e blk;
static int bi;

// Cheap local label (CLL) scope counter — incremented on each standard label definition
static int cll_current = 0;

static void cll_init(void)	{ cll_current = 0; }
static int  cll_get(void)	{ return cll_current; }

int  cll_getcur(void)		{ return cll_current; }
void cll_clear(void)		{ cll_current++; }

// Unnamed label chain — tracks definition order for :+/:- resolution
static int labind = -1;		// index of most recently defined label (any type)



// Link a new label into the definition-order chain
static void b_link(int newlab)
{
	SymbolEntry& entry = afile->m_cSymbolData.GetSymbolEntry(newlab);
	entry.SetBlockPrev(labind);
	entry.SetBlockNext(-1);
	if (labind >= 0)
	{
		afile->m_cSymbolData.GetSymbolEntry(labind).SetBlockNext(newlab);
	}
	labind = newlab;
}

int gm_lab(void)
{
	return ANZLAB;
}

long gm_labm(void)
{
	return ((long)LABMEM);
}

long ga_labm(void)
{
	return 0;
}


          
ErrorCode SymbolData::DefineLabel(char *ptr_src,int *size_read,int *x,bool *flag_redefine_label)
{
	*flag_redefine_label=false;

	int block_level=0;
	int n=0;
	int	i=0;
	LabelType_e label_type = eLABELTYPE_STANDARD;

	if (ptr_src[0]=='-')
	{
		// Allows label redefinition
		*flag_redefine_label=true;
		i++;
	}
	else
	if (ptr_src[0]=='+')
	{
		// Defines a global label
		i++;
		n++;
		block_level=0;
	}
	else
	if (ptr_src[0]=='@')
	{
		// Cheap local label — scoped to nearest standard label
		label_type = eLABELTYPE_CHEAP;
		i++;
		block_level = cll_get();
		n = 1;	// skip b_fget
	}
	while (ptr_src[i]=='&')
	{
		// If a label is proceeded by a '&', this label is defined one level 'up' in the block hierarchy, and you can use more than one '&'.
		n=0;
		i++;
		block_level++;
	}
	if (!n)
	{
		b_fget(&block_level,block_level);
	}

	// Standard labels reset the cheap local scope
	if (label_type == eLABELTYPE_STANDARD && !(*flag_redefine_label))
	{
		cll_clear();
	}

	if (!isalpha(ptr_src[i]) && ptr_src[i]!='_')
	{
		return E_SYNTAX;
	}

	ErrorCode er;
	if (label_type == eLABELTYPE_CHEAP)
	{
		er = SearchSymbol(ptr_src+i, &n, eLABELTYPE_CHEAP);
	}
	else
	{
		er = SearchSymbol(ptr_src+i, &n);
	}

	if (er==E_OK)
	{
		SymbolEntry& symbol_entry=GetSymbolEntry(n);

		if (*flag_redefine_label)
		{
            *size_read=symbol_entry.label_name_lenght+i;
		}
		else
		if (symbol_entry.symbol_status==eSYMBOLSTATUS_UNKNOWN)
		{
			*size_read=symbol_entry.label_name_lenght+i;
			if (label_type == eLABELTYPE_CHEAP)
			{
				// Cheap locals: just check block_level matches
				if (symbol_entry.GetBlockLevel() != block_level)
				{
					er=E_LABDEF;
				}
			}
			else
			if (b_ltest(symbol_entry.GetBlockLevel(),block_level))
			{
				er=E_LABDEF;
			}
			else
			{
				symbol_entry.SetBlockLevel(block_level);
			}

		}
		else
		{
			er=E_LABDEF;
		}
	}
	else
	if (er==ERR_UNDEFINED_LABEL)
	{
		//
		// Add a new label in the global table
		//
		if (!(er=DefineSymbol(ptr_src+i,&n,block_level))) /* ll_def(...,*flag_redefine_label) */
		{
			SymbolEntry& symbol_entry=GetSymbolEntry(n);
			*size_read=symbol_entry.label_name_lenght+i;
			symbol_entry.symbol_status=eSYMBOLSTATUS_UNKNOWN;
			symbol_entry.SetLabelType(label_type);
		}
	}
	*x=n;
	b_link(n);
	return er;
}

ErrorCode SymbolData::l_such(char *s, int *l, int *x, int *v, int *afl)
{
	ErrorCode er;
	int	n,b;

	*afl=0;

	// Check for cheap local label prefix (@)
	if (s[0]=='@')
	{
		er=SearchSymbol(s+1,&n,eLABELTYPE_CHEAP);
		if (er==E_OK)
		{
			SymbolEntry& symbol_entry=GetSymbolEntry(n);
			*l=symbol_entry.label_name_lenght+1;	// +1 for '@'
			if (symbol_entry.symbol_status==eSYMBOLSTATUS_VALID)
			{
				er=symbol_entry.Get(v,afl);
				*x=n;
			}
			else
			{
				er=ERR_UNDEFINED_LABEL;
				gError_LabelNamePointer=symbol_entry.ptr_label_name;
				*x=n;
			}
		}
		else
		{
			// Create new cheap local label entry (forward reference)
			int block_level = cll_get();
			er=DefineSymbol(s+1,x,block_level);
			SymbolEntry& symbol_entry=GetSymbolEntry(*x);
			*l=symbol_entry.label_name_lenght+1;	// +1 for '@'
			symbol_entry.SetLabelType(eLABELTYPE_CHEAP);
			b_link(*x);
			if (!er)
			{
				er=ERR_UNDEFINED_LABEL;
				gError_LabelNamePointer=symbol_entry.ptr_label_name;
			}
		}
		return er;
	}

	// Check for unnamed label reference (:+, :-, :++, etc.)
	if (s[0]==':' && gFlag_UnnamedLabels)
	{
		// Count direction characters
		int j=1;
		char dir = s[j];
		if (dir=='+' || dir=='-')
		{
			while (s[j]==dir) j++;
			// Create an UNNAMED_REF entry — name is the +/- chars
			// Allocate in symbol table but not in hash
			er = DefineLabel_Unnamed(s, l, x);
			if (!er)
			{
				SymbolEntry& symbol_entry=GetSymbolEntry(*x);
				er=ERR_UNDEFINED_LABEL;
				gError_LabelNamePointer=(char*)"<unnamed>";
			}
			return er;
		}
	}

	er=SearchSymbol(s,&n);
	if (er==E_OK)
	{
		SymbolEntry& symbol_entry=GetSymbolEntry(n);
		*l=symbol_entry.label_name_lenght;
		if (symbol_entry.symbol_status==eSYMBOLSTATUS_VALID)
		{
			SymbolEntry& symbol_entry=GetSymbolEntry(n);
			er=symbol_entry.Get(v,afl);
			*x=n;
		}
		else
		{
			er=ERR_UNDEFINED_LABEL;
			gError_LabelNamePointer=symbol_entry.ptr_label_name;
			*x=n;
		}
	}
	else
	{
		b_get(&b);
		er=DefineSymbol(s,x,b); /* ll_def(...,*v); */

		SymbolEntry& symbol_entry=GetSymbolEntry(*x);

		*l=symbol_entry.label_name_lenght;

		if (!er)
		{
			er=ERR_UNDEFINED_LABEL;
			gError_LabelNamePointer=symbol_entry.ptr_label_name;
		}
	}
	return er;
}




ErrorCode SymbolData::ll_pdef(char *t)
{
	int n;
	if (SearchSymbol(t,&n)==E_OK)
	{
		SymbolEntry& symbol_entry=GetSymbolEntry(n);
		if (symbol_entry.symbol_status)
		{
			return E_OK;
		}
	}
	return ERR_UNDEFINED_LABEL;
}


ErrorCode SymbolData::DefineLabel_Unnamed(char *ptr_src, int *size_read, int *x)
{
	// ptr_src points at ':' followed by '+'/'-' chars
	// For a definition (bare ':'), ptr_src[0]==':' and there are no +/- chars
	// For a reference (:+, :-, :++, etc.), count direction chars

	if (ptr_src[0]!=':')
		return E_SYNTAX;

	int j=1;
	char dir=ptr_src[j];
	bool is_ref = (dir=='+' || dir=='-');

	if (is_ref)
	{
		// Reference: count direction chars
		while (ptr_src[j]==dir) j++;
	}

	// Allocate a symbol entry directly (not hashed)
	if (!m_ptr_table_entries)
	{
		m_nb_labels = 0;
		m_max_labels = 1000;
		m_ptr_table_entries = (SymbolEntry*)malloc(m_max_labels * sizeof(SymbolEntry));
	}
	if (m_nb_labels >= m_max_labels)
	{
		m_max_labels = (int)(m_max_labels * 1.5);
		m_ptr_table_entries = (SymbolEntry*)realloc(m_ptr_table_entries, m_max_labels * sizeof(SymbolEntry));
	}
	if (!m_ptr_table_entries)
	{
		fprintf(stderr, "Oops: no memory!\n");
		exit(1);
	}

	int idx = m_nb_labels;
	SymbolEntry *entry = m_ptr_table_entries + idx;

	if (is_ref)
	{
		// Store the direction string as the name (e.g. "+", "++", "-")
		int namelen = j - 1;
		char *name = (char*)malloc(namelen + 1);
		memcpy(name, ptr_src + 1, namelen);
		name[namelen] = 0;
		entry->ptr_label_name		= name;
		entry->label_name_lenght	= namelen;
		entry->m_label_type			= eLABELTYPE_UNNAMED_REF;
	}
	else
	{
		// Definition: no searchable name
		entry->ptr_label_name		= NULL;
		entry->label_name_lenght	= 0;
		entry->m_label_type			= eLABELTYPE_UNNAMED;
	}

	entry->value			= 0;
	entry->symbol_status	= eSYMBOLSTATUS_UNKNOWN;
	entry->program_section	= eSEGMENT_ABS;
	entry->m_block_level	= 0;
	entry->nextindex		= 0;
	entry->m_blknext		= -1;
	entry->m_blkprev		= -1;

	m_nb_labels++;

	*x = idx;
	*size_read = j;

	// Link into definition-order chain
	b_link(idx);

	return E_OK;
}


int SymbolData::ResolveUnnamed(int label_index)
{
	SymbolEntry& ref_entry = GetSymbolEntry(label_index);
	if (ref_entry.GetLabelType() != eLABELTYPE_UNNAMED_REF)
		return -1;

	const char* name = ref_entry.GetSymbolName();
	if (!name || !name[0])
		return -1;

	char dir = name[0];
	int count = (int)strlen(name);		// number of steps (e.g. "++" = 2 forward)

	int cur = label_index;
	int found = 0;

	if (dir == '+')
	{
		// Walk forward in definition order
		while (cur >= 0 && found < count)
		{
			cur = GetSymbolEntry(cur).GetBlockNext();
			while (cur >= 0 && GetSymbolEntry(cur).GetLabelType() != eLABELTYPE_UNNAMED)
			{
				cur = GetSymbolEntry(cur).GetBlockNext();
			}
			if (cur >= 0) found++;
		}
	}
	else if (dir == '-')
	{
		// Walk backward in definition order
		while (cur >= 0 && found < count)
		{
			cur = GetSymbolEntry(cur).GetBlockPrev();
			while (cur >= 0 && GetSymbolEntry(cur).GetLabelType() != eLABELTYPE_UNNAMED)
			{
				cur = GetSymbolEntry(cur).GetBlockPrev();
			}
			if (cur >= 0) found++;
		}
	}

	return (found == count) ? cur : -1;
}


int b_init(void)
{
     blk =eSEGMENT_ABS;
     bi	 =0;
     bt[bi]=blk;
     cll_init();
     labind = -1;

     return E_OK;
}     

int b_depth(void)
{
     return bi;
}

int ga_blk(void)
{
	return blk;
}

int b_open(void)
{
	int er=E_BLKOVR;
	
	if (bi<MAXBLK-1)
	{
		blk=(SEGMENT_e)(blk+1);
		bt[++bi]=blk;
		
		er=E_OK;  
	}
	return er;
}

ErrorCode b_close(void)
{
	if (bi)
	{
		afile->m_cSymbolData.ExitBlock(bt[bi],bt[bi-1]);
		bi--;
		cll_clear();	// Reset cheap local scope on block close
	}
	else
	{
		return E_BLOCK;
	}

	return E_OK;
}

static int b_get(int *n)
{
	*n=bt[bi];
	
	return E_OK;
}

static int b_fget(int *n, int i)
{
	if((bi-i)>=0)
		*n=bt[bi-i];
	else
		*n=0;
	return E_OK;
}

int b_test(int n)
{
     int i=bi;

     while( i>=0 && n!=bt[i] )
          i--;

     return i+1 ? E_OK : E_NOBLK;
}

static int b_ltest(int a, int b)    /* testet ob bt^-1(b) in intervall [0,bt^-1(a)]   */
{
     int i=0;
	 int er=E_OK;

     if(a!=b)
     {
          er=E_OK;

          while(i<=bi && b!=bt[i])
          {
               if(bt[i]==a)
               {
                    er=E_NOBLK;
                    break;
               }
               i++;
          }
     }
     return er;
}



// -----------------------------------------------------------------------------
//
//								SymbolEntry
//
// -----------------------------------------------------------------------------


void SymbolEntry::Set(int v,SEGMENT_e afl)
{
	value			= v;
	symbol_status	=eSYMBOLSTATUS_VALID;
	program_section	=afl;
}

ErrorCode SymbolEntry::Get(int *v,int *afl)
{
	(*v)=value;
	gError_LabelNamePointer=ptr_label_name;
	*afl = program_section;
	return ( (symbol_status==eSYMBOLSTATUS_VALID) ? E_OK : ERR_UNDEFINED_LABEL);
}

int SymbolEntry::DefineSymbol(char *ptr_src,int block_level)
{
	int	j=0;
	while ((ptr_src[j]!=0) && (isalnum(ptr_src[j]) || (ptr_src[j]=='_'))) j++;
	char *label_name = (char*)malloc(j+1);
	if (!label_name) 
	{
		fprintf(stderr,"Oops: no memory!\nlabel_index");
		exit(1);
	}
	strncpy(label_name,ptr_src,j);
	label_name[j]=0;
	ErrorCode er=E_OK;
	value				=0;
	label_name_lenght	=j;
	ptr_label_name		=label_name;
	m_block_level		=block_level;
	symbol_status		=eSYMBOLSTATUS_UNKNOWN;
	program_section		=eSEGMENT_ABS;
	m_label_type		=eLABELTYPE_STANDARD;
	m_blknext			=-1;
	m_blkprev			=-1;
	int hash=hashcode(ptr_src,j);
	return hash;
}

