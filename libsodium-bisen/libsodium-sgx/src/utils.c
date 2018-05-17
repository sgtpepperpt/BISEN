#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* legacy : to remove */
void byte_zero(void *yv,long long ylen)
{
  char *y = yv;
  while (ylen > 0) { *y++ = 0; --ylen; }
}

void byte_copy(void *yv,long long ylen,const void *xv)
{
  char *y = yv;
  const char *x = xv;
  while (ylen > 0) { *y++ = *x++; --ylen; }
}

/* core.c */
int
sodium_crit_enter(void)
{
    return 0;
}

int
sodium_crit_leave(void)
{
    return 0;
}

static void (*_misuse_handler)(void);

void
sodium_misuse(void)
{
    void (*handler)(void);

    if (sodium_crit_enter() == 0) {
        handler = _misuse_handler;
        if (sodium_crit_leave() == 0 && handler != NULL) {
            handler();
        }
    }
/* LCOV_EXCL_START */
    abort();
}
/* LCOV_EXCL_STOP */


/* utils.c */
/* $ cd ../../libsodium && grep -o "#define HAVE_WEAK_SYMBOLS [0-9]" configure */
#define HAVE_WEAK_SYMBOLS 1

/* LCOV_EXCL_START */
#ifdef HAVE_WEAK_SYMBOLS
__attribute__((weak)) void
_sodium_dummy_symbol_to_prevent_memzero_lto(void *const  pnt,
                                            const size_t len)
{
    (void) pnt; /* LCOV_EXCL_LINE */
    (void) len; /* LCOV_EXCL_LINE */
}
#endif
/* LCOV_EXCL_STOP */

void
sodium_memzero(void *const pnt, const size_t len)
{
#ifdef _WIN32
    SecureZeroMemory(pnt, len);
#elif defined(HAVE_MEMSET_S)
    if (len > 0U && memset_s(pnt, (rsize_t) len, 0, (rsize_t) len) != 0) {
        sodium_misuse(); /* LCOV_EXCL_LINE */
    }
#elif defined(HAVE_EXPLICIT_BZERO)
    explicit_bzero(pnt, len);
#elif HAVE_WEAK_SYMBOLS
    memset(pnt, 0, len);
    _sodium_dummy_symbol_to_prevent_memzero_lto(pnt, len);
#else
    volatile unsigned char *volatile pnt_ =
        (volatile unsigned char *volatile) pnt;
    size_t i = (size_t) 0U;

    while (i < len) {
        pnt_[i++] = 0U;
    }
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
__attribute__((weak)) void
_sodium_dummy_symbol_to_prevent_memcmp_lto(const unsigned char *b1,
                                           const unsigned char *b2,
                                           const size_t         len)
{
    (void) b1;
    (void) b2;
    (void) len;
}
#endif

int
sodium_memcmp(const void *const b1_, const void *const b2_, size_t len)
{
#ifdef HAVE_WEAK_SYMBOLS
    const unsigned char *b1 = (const unsigned char *) b1_;
    const unsigned char *b2 = (const unsigned char *) b2_;
#else
    const volatile unsigned char *volatile b1 =
        (const volatile unsigned char *volatile) b1_;
    const volatile unsigned char *volatile b2 =
        (const volatile unsigned char *volatile) b2_;
#endif
    size_t                 i;
    volatile unsigned char d = 0U;

#if HAVE_WEAK_SYMBOLS
    _sodium_dummy_symbol_to_prevent_memcmp_lto(b1, b2, len);
#endif
    for (i = 0U; i < len; i++) {
        d |= b1[i] ^ b2[i];
    }
    return (1 & ((d - 1) >> 8)) - 1;
}

void * memmove(void * destination, const void * source, size_t num)
{
  char *d = destination;
  const char *s = source;
  while (num > 0) { *d++ = *s++; --num; }
  return destination;
}

/* *****************************************************************************
 *  begin: randombytes_buf  
 *  code from: Intel® Digital Random Number Generator (DRNG) Software Implementation Guide
 *******************************************************************************/
#define RDRAND_RETRIES 10

int rdrand64_step (uint64_t *rand)
{
	unsigned char ok;

	asm volatile ("rdrand %0; setc %1"
		: "=r" (*rand), "=qm" (ok));

	return (int) ok;
}

int rdrand64_retry (unsigned int retries, uint64_t *rand)
{
	unsigned int count= 0;
	while ( count <= retries ) {
		if ( rdrand64_step(rand) ) {
			return 1;
		}
		++count;
	}
	return 0;
}

unsigned int rdrand_get_bytes (unsigned int n, unsigned char *dest)
{
	unsigned char *headstart, *tailstart;
	uint64_t *blockstart;
	unsigned int count, ltail, lhead, lblock;
	uint64_t i, temprand;

	/* Get the address of the first 64-bit aligned block in the
	 * destination buffer. */
	headstart= dest;
	if ( ( (uint64_t)headstart % (uint64_t)8 ) == 0 ) {

		blockstart= (uint64_t *)headstart;
		lblock= n;
		lhead= 0;
	} else {
		blockstart= (uint64_t *) 
			( ((uint64_t)headstart & ~(uint64_t)7) + (uint64_t)8 );

		lblock= n - (8 - (unsigned int) ( (uint64_t)headstart & (uint64_t)8 ));

		lhead= (unsigned int) ( (uint64_t)blockstart - (uint64_t)headstart );
	}

	/* Compute the number of 64-bit blocks and the remaining number
	 * of bytes (the tail) */
	ltail= n-lblock-lhead;
	count= lblock/8;	/* The number 64-bit rands needed */

	if ( ltail ) {
		tailstart= (unsigned char *)( (uint64_t) blockstart + (uint64_t) lblock );
	}

	/* Populate the starting, mis-aligned section (the head) */

	if ( lhead ) {
		if ( ! rdrand64_retry(RDRAND_RETRIES, &temprand) ) {
			return 0;
		}

		memcpy(headstart, &temprand, lhead);
	}

	/* Populate the central, aligned block */

	for (i= 0; i< count; ++i, ++blockstart) {
		if ( ! rdrand64_retry(RDRAND_RETRIES, blockstart) ) {
			return i*8+lhead;
		}
	}

	/* Populate the tail */

	if ( ltail ) {
		if ( ! rdrand64_retry(RDRAND_RETRIES, &temprand) ) {
			return count*8+lhead;
		}

		memcpy(tailstart, &temprand, ltail);
	}

	return n;
}

void
randombytes(unsigned char *output, unsigned long long len)
{
	rdrand_get_bytes(len, output);
}

void
randombytes_buf(void * const buf, const size_t size)
{
  rdrand_get_bytes(size, buf);
}

uint32_t randombytes_random(void)
{
  uint64_t rnd;
  rdrand64_retry(10, &rnd);
  return (uint32_t)rnd;
}
