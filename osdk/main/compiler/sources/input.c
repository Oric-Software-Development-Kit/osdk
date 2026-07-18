/* C compiler: input processing */

#include "c.h"
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <io.h>
#endif

unsigned char *cp;	/* current input character */
char *file;		/* current input file name */
char *firstfile;	/* first input file */
unsigned char *limit;	/* points to last character + 1 */
char *line;		/* current line */
int lineno;		/* line number of current line */

static int infd;	/* input file descriptor */
static int bsize;	/* number of chars in last read */
static unsigned char buffer[MAXTOKEN+1 + BUFSIZE+1];	/* input buffer */

dclproto(static void pragma,(void));
dclproto(static void resynch,(void));

/* inputInit - initialize input processing */
void inputInit(int fd) {
	limit = cp = &buffer[MAXTOKEN + 1];
	lineno = 0;
	file = 0;			/* omit */
	bsize = -1;
	infd = fd;
	nextline();
}

/* inputstring - arrange to read str as next input */
void inputstring(str) char *str; {
	limit = cp = &buffer[MAXTOKEN+1];
	while ((*limit++ = *str++))
		;
	*limit = '\n';
	bsize = 0;
}

/* fillbuf - fill the input buffer, moving tail portion if necessary */
void fillbuf() {
	if (bsize == 0)
		return;
	if (limit <= cp)
		cp = &buffer[MAXTOKEN + 1];
	else {		/* move tail portion */
		int n = limit - cp;
		unsigned char *s = &buffer[MAXTOKEN + 1] - n;
		assert(s >= buffer);
		line = (char *)s - ((char *)cp - line);
		while (cp < limit)
			*s++ = *cp++;
		cp = &buffer[MAXTOKEN + 1] - n;
	}
	bsize = read(infd, (char *)&buffer[MAXTOKEN + 1], BUFSIZE);
	assert(bsize >= 0);
	limit = &buffer[MAXTOKEN + 1 + bsize];
	*limit = '\n';
}

/* nextline - prepare to read next line */
void nextline() {
	if (cp >= limit) {	/* refill buffer */
		fillbuf();
		if (cp >= limit) {	/* signal end of file */
			cp = limit = &buffer[MAXTOKEN+1];
			*limit = '\0';
		}
		if (lineno > 0)
			return;
	}
	lineno++;
	for (line = (char *)cp; *cp == ' ' || *cp == '\t'; cp++)
		;
	if (*cp == '#') {			/* omit */
		resynch();			/* omit */
		nextline();			/* omit */
	}					/* omit */
}

/* pragma - handle #pragma ref id... and #pragma optimize(...) */
static void pragma() {
	if ((t = gettok()) == ID && strcmp(token, "ref") == 0)
		for (;;) {
			while (*cp == ' ' || *cp == '\t')
				cp++;
			if (*cp == '\n' || *cp == 0)
				break;
			if ((t = gettok()) == ID && tsym) {
				tsym->ref++;
				use(tsym, src);
			}
		}
	else if (t == ID && strcmp(token, "optimize") == 0) {
		/* #pragma optimize(push, n) - save current level, switch to n (0-3)
		   #pragma optimize(pop)     - restore previously pushed level
		   #pragma optimize(n)       - set level n without saving
		   Applies to the function definitions that follow the pragma. */
		int push = 0, pop = 0, level = -1;
		while (*cp == ' ' || *cp == '\t' || *cp == '(')
			cp++;
		if (strncmp((char *)cp, "push", 4) == 0) {
			push = 1;
			cp += 4;
			while (*cp == ' ' || *cp == '\t' || *cp == ',')
				cp++;
		} else if (strncmp((char *)cp, "pop", 3) == 0) {
			pop = 1;
			cp += 3;
		}
		if (*cp >= '0' && *cp <= '9')
			level = *cp++ - '0';
		if (pop) {
			if (optimizepop() < 0)
				warning("#pragma optimize(pop) without matching push\n");
		} else if (level < 0 || level > 3)
			warning("malformed #pragma optimize: expected (push, n), (pop) or (n) with n in 0..3\n");
		else if (push) {
			if (optimizepush(level) < 0)
				warning("#pragma optimize(push) nested too deeply\n");
		} else
			optimizeset(level);
		while (*cp != '\n' && *cp != 0)	/* discard rest of line */
			cp++;
	}
}

/* resynch - set line number/file name in # n [ "file" ] and #pragma ... */
static void resynch() {
	for (cp++; *cp == ' ' || *cp == '\t'; )
		cp++;
	if (limit - cp < MAXTOKEN)
		fillbuf();
	if (strncmp((char *)cp, "pragma", 6) == 0) {
		cp += 6;
		pragma();
	} else if (*cp >= '0' && *cp <= '9') {
	line:	for (lineno = 0; *cp >= '0' && *cp <= '9'; )
			lineno = 10*lineno + *cp++ - '0';
		lineno--;
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp == '"') {
			file = (char *)++cp;
			while (*cp && *cp != '"' && *cp != '\n')
				cp++;
			if (cp == limit) {
				char buf[MAXLINE], *s = buf;
				while (file < (char *)cp)
					*s++ = *file++;
				while (cp == limit && *cp) {
					cp++;
					nextline();
					for (; *cp && *cp != '"' && *cp != '\n'; cp++)
						if (s < &buf[sizeof buf])
							*s++ = *cp;
				}
				file = stringn(buf, s - buf);
				if (s == &buf[sizeof buf])
					warning("file name is too long\n");
			} else
				file = stringn(file, (char *)cp - file);
			if (*cp == '\n')
				warning("missing \" in preprocessor line\n");
			if (firstfile == 0)
				firstfile = file;
		}
	} else if (strncmp((char *)cp, "line", 4) == 0) {
		for (cp += 4; *cp == ' ' || *cp == '\t'; )
			cp++;
		if (*cp >= '0' && *cp <= '9')
			goto line;
		if (Aflag >= 2)
			warning("unrecognized control line\n");
	} else if (Aflag >= 2 && *cp != '\n')
		warning("unrecognized control line\n");
	while (*cp)
		if (*cp++ == '\n') {
			if (cp == limit + 1)
				nextline();
			else
				break;
		}
}
