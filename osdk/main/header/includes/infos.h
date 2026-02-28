/*

Change history for Header

0.2 - 2014/05/11
- The address can now be specified with either $ or 0x as an hexadecimal prefix

1.0 - 2020/12/12
- Added the flag -b to specify if the converted file should expose 00 (BASIC) or 80 (Binary) in the header

1.1 - 2023/07/07
- If the source file starts by SEDORIC then the 12 first bytes are automatically ignored when the tape is converted

1.2 - 2025/05/11
- [iss] Added the -3 and -4 flag to specify if the tape header should have 3 or 4 times the value $16
- [iss] Added a -nName option to specify the filename to write in the internal tape header

*/

#define TOOL_VERSION_MAJOR	1
#define TOOL_VERSION_MINOR	2

