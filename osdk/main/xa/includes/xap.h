

extern char BufferLine[MAXLINE];



//#define PROCESSOR_USES_MAP

#ifdef PROCESSOR_USES_MAP

class List
{
public:
	std::string					m_search;
	std::string					m_replace;
	std::vector<std::string>	m_parameters;
};

#else

struct List
{
	char *search;
	int  search_length;
	char *replace;
	int  parameters;
	int  nextindex;
};

#endif




// A #print whose expression resolves a .data/.bss label that automatic segment chaining
// may still move cannot be answered during pass 1: the label only receives its final
// address once every segment has been measured and relocated. Such a line is queued and
// flushed right after that relocation. It is the TOKEN stream that is kept, not the source
// text: the tokens hold auto-chained labels symbolically and re-tokenising later would
// resolve names outside the block / cheap-local scope the directive stood in.
struct DeferredPrint
{
	std::string					m_Label;		//!< text left of the '=', printed verbatim
	std::string					m_Expression;	//!< macro-expanded expression text
	std::vector<signed char>	m_Tokens;		//!< tokenised expression, ready to re-evaluate
	int							m_Pc;			//!< PC at the directive, so a '*' keeps its meaning
	SEGMENT_e					m_Segment;		//!< segment it stood in, for error reporting
	std::string					m_File;			//!< source location, for error reporting
	int							m_Line;
};


class Preprocessor
{
public:

	enum 
	{
		e_command_echo,	// 0
		e_command_include,	// 1
		e_command_define,	// 2
		e_command_undef,	// 3
		e_command_prdef,	// 4
		e_command_print,	// 5
		e_command_error,	// 6
		e_command_ifdef,	// 7
		e_command_ifndef,	// 7
		e_command_else,	// 8
		e_command_endif,	// 9
		e_command_ifldef,	// 10
		e_command_iflused,	// 11
		e_command_if,		// 12
		e_command_file,	// 13
		e_command_elif,	// 14
		_e_command_max_
	};


public:
	int Init(void);
	void Close(void);
	void Terminate(void);

	void PushConditional();		//!< record an opened #if/#ifdef (its source location)
	void PopConditional();		//!< close the innermost #if, or note a stray #endif

	void FlushDeferredPrints();	//!< emit the #print lines held back until segment chaining had run

	ErrorCode GetLine(char *t);

	int suchdef(char *t);
	bool EvaluateDefined(char *t, bool &result);
	int CheckForPreprocessorCommand(char s[]);

	ErrorCode pp_replace(char *ptr_output,char *ptr_input,int a,int b);
	ErrorCode HandleCommand(char *ptr_preprocessor_directive);

	ErrorCode command_define(char *k);
	ErrorCode command_enum(char *content);
	ErrorCode command_include(char*);
	ErrorCode command_ifdef(char*);
	ErrorCode command_ifndef(char*);
	ErrorCode command_else(char*);
	ErrorCode command_endif(char*);
	ErrorCode command_echo(char*);
	ErrorCode command_if(char*);
	ErrorCode command_print(char*);
	ErrorCode command_error(char*);
	ErrorCode command_prdef(char*);
	ErrorCode command_ifldef(char*);
	ErrorCode command_iflused(char*);
	ErrorCode command_undef(char*);
	ErrorCode command_file(char*);
	ErrorCode command_elif(char*);

	int ga_pp(void)
	{
		return m_CurrentListIndex;
	}

	int gm_pp(void)
	{
		return ANZDEF;
	}

	long gm_ppm(void)
	{
		return MAX_PREPROCESSOR_BUFFER_SIZE;
	}

	long ga_ppm(void)
	{
		return MAX_PREPROCESSOR_BUFFER_SIZE-m_FreeMemory;
	}

public:
#ifdef PROCESSOR_USES_MAP
	std::map<std::string,List>	gPreprocessor_ReplaceMap;
#else
	List 	     		*gListArray;
	int 				hashindex[256];
	unsigned int		m_CurrentListIndex;
	unsigned long 		m_FreeMemory;
#endif
	char 				*m_CurrentBufferPtr;
	PreprocessorFile_c	*m_CurrentFile;
	bool				m_FlagNewLineFound;
	bool				m_FlagNewFileFound;
	int       			m_LogicalOpcodesStack;	//!< Contains one bit for each level of #if / #ifdef ...encountered during preprocessing
	int       			m_BranchTakenStack;		//!< Tracks if a true branch was already taken at each level (for #elif support)
	std::vector<std::pair<std::string,int> > m_OpenConditionals;	//!< file+line of each still-open #if/#ifdef (for unterminated-directive diagnostics)
	int       			m_UnmatchedEndifCount = 0;	//!< #endif seen with no matching open #if
	std::string 		m_FirstStrayEndifFile;		//!< source location of the first such stray #endif
	int       			m_FirstStrayEndifLine = 0;
	std::vector<DeferredPrint> m_DeferredPrints;	//!< #print lines awaiting the final auto-chained addresses
	char      			m_BufferLine[MAXLINE];
};


extern Preprocessor		gPreprocessor;

