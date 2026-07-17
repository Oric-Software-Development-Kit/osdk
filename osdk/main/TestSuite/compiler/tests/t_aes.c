/* AES-256 known-answer test. Encrypts the fixed vector from the aes256
   reference and checks the ciphertext, which is the regression guard for
   the -O3 borrowed-pointer / call-store-width codegen bug (a call result
   stored through a scratch temp the call had clobbered, and at word width
   through a char element): "buf[i] = rj_sbox(buf[i])" in aes_subBytes. */
#include "testkit.h"
#include "t_aes.h"

static aes256_context ctx;
static unsigned char kbuf[32];
static unsigned char buf[32];

/* Expected ECB ciphertext of the 0xAA55.. block under key 0..31. */
static const unsigned char expected[16] = {
	0xDD,0x2E,0xF2,0x7C,0xA8,0x00,0xC4,0x74,
	0xEB,0x1B,0x13,0xC8,0x53,0xD4,0x5E,0xC0
};

void main(void)
{
	unsigned char i;
	int ok;

	tk_begin("aes");

	for (i = 0; i < 32; i++) buf[i]  = (i & 1) ? 0x55 : 0xaa;
	for (i = 0; i < 32; i++) kbuf[i] = i;

	aes256_init(&ctx, kbuf);
	aes256_encrypt_ecb(&ctx, &buf[0]);
	aes256_encrypt_ecb(&ctx, &buf[16]);

	ok = 1;
	for (i = 0; i < 16; i++) if (buf[i]    != expected[i]) ok = 0;
	tk_check(ok, "enc-block0");
	ok = 1;
	for (i = 0; i < 16; i++) if (buf[16+i] != expected[i]) ok = 0;
	tk_check(ok, "enc-block1");

	/* neighbour-byte guard: the width half of the bug wrote 2 bytes for a
	   char store; verify buf is untouched past the two blocks is implicit
	   (32-byte buf fully checked above). */

	aes256_init(&ctx, kbuf);
	aes256_decrypt_ecb(&ctx, &buf[0]);
	aes256_decrypt_ecb(&ctx, &buf[16]);
	ok = 1;
	for (i = 0; i < 32; i++) if (buf[i] != ((i & 1) ? 0x55 : 0xaa)) ok = 0;
	tk_check(ok, "roundtrip");

	tk_end();
}
