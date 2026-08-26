

extern int gDsbLen;

extern ErrorCode t_p1(signed char *ptr_text,signed char *t,int *ll,int *ptr_size_written);
extern ErrorCode t_p2(signed char *t, int *ll, int fl, int *al);
extern int b_term(char *s, int *v, int *l, int pc);

// As b_term, but the token stream is written to caller storage (MAXLINE bytes) and its
// length returned in *l. Auto-chained labels stay symbolic in that stream, so it can be
// re-evaluated after relocation to obtain final addresses. Re-tokenising the source text
// instead would look the names up outside the block and cheap-local scope they stood in.
extern int b_term_tokens(char *s, int *v, int *l, int pc, signed char *ptr_tokens);

