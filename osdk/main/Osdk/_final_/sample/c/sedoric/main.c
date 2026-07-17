/*
  SEDORIC demo.

  Shows the three SEDORIC entry points of the OSDK library:
   - sedoric()      : run any SEDORIC command, here "!DIR"
   - sed_savefile() : save a memory area as a data file on the floppy
   - sed_loadfile() : load a file from the floppy, here a HIRES picture
                      stored as a separate file on the disk

  The C runtime zero page is sheltered around every call (lib/zeropage.s),
  so the program simply keeps running after each DOS call - including
  after an error, which is returned as the SEDORIC error number.

  The floppy is built by osdk_build.bat: the program and the picture are
  converted to TAP files, combined into a SEDORIC disk with tap2dsk, and
  the disk is made emulator-loadable with old2mfm.
*/
#include <lib.h>

unsigned char savebuf[64];
unsigned int len;

void main(void)
{
	int err, i;

	cls();
	printf("SEDORIC demo\n\nDirectory:\n");
	sedoric("!DIR");

	printf("\nSaving SAVED.DAT (64 bytes)...\n");
	for (i = 0; i < 64; i++)
		savebuf[i] = (unsigned char)i;
	err = sed_savefile("SAVED.DAT", savebuf, 64);
	printf("sed_savefile: err=%d\n", err);

	printf("\nPress a key to load the picture");
	getchar();

	hires();
	err = sed_loadfile("PICTURE.BIN", (void *)0xA000, &len);
	if (err) {
		text();
		printf("sed_loadfile: err=%d\n", err);
	} else {
		/* printf goes to the 3 text lines below the hires area */
		printf("PICTURE.BIN loaded, %u bytes, no error", len);
	}
	for (;;) ;
}
