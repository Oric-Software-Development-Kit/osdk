/* ctype.h */

#ifndef _CTYPE_

#define _CTYPE_


/* Check if c is alphabetic. */

   /* Returns 1 if yes, 0 if no. */

extern __fastcall int isalpha(char c);


/* Check if c is an upper case character. */

extern __fastcall int isupper(char c);


/* Check if c is a lower case character. */

extern __fastcall int islower(char c);


/* Check if c is a decimal digit. */

extern __fastcall int isdigit(char c);


/* Check if c is a white space character. */

extern __fastcall int isspace(char c);


/* Check if c is a character used in punctuation. */

extern __fastcall int ispunct(char c);


/* Check if c is a printable ASCII character.*/

extern __fastcall int isprint(char c);


/* Check if c is an ASCII control character. */

extern __fastcall int iscntrl(char c);


/* Check if c is an ASCII character. */

extern __fastcall int isascii(char c);


/* Return c converted to upper case. */

extern __fastcall char toupper(char c);


/* Return c converted to lower case. */

extern __fastcall char tolower(char c);


/* Return c with the 8th bit stripped off. */

extern __fastcall char toascii(char c);


#endif /* _CTYPE_

/* end of file ctype.h */

