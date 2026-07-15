/* assert.h shim for the cc65 test/val import. */
#ifndef _ASSERT_H
#define _ASSERT_H

#define assert(e) ((e) ? (void)0 : (void)printf("Assertion failed, line %d\n", __LINE__))

/* C11 convenience macro; the wrapper supplies the _Static_assert emulation */
#define static_assert _Static_assert

#endif
