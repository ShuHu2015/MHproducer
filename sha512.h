
#ifndef _SHA512_H
#define _SHA512_H

#ifdef  __cplusplus
extern "C" {
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

typedef unsigned __int64 uint64_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;



# define SHA_LBLOCK      16
# define SHA_CBLOCK      (SHA_LBLOCK*4)/* SHA treats input data as a
                                        * contiguous array of 32 bit wide
                                        * big-endian values. */
# define SHA384_DIGEST_LENGTH    48
# define SHA512_DIGEST_LENGTH    64

/*
 * Unlike 32-bit digest algorithms, SHA-512 *relies* on SHA_LONG64
 * being exactly 64-bit wide. See Implementation Notes in sha512.c
 * for further details.
 */
/*
 * SHA-512 treats input data as a
 * contiguous array of 64 bit
 * wide big-endian values.
 */
#  define SHA512_CBLOCK   (SHA_LBLOCK*8)
#  if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
#   define SHA_LONG64 unsigned __int64
#   define U64(C)     C##UI64
#  elif defined(__arch64__)
#   define SHA_LONG64 unsigned long
#   define U64(C)     C##UL
#  else
#   define SHA_LONG64 unsigned long long
#   define U64(C)     C##ULL
#  endif

typedef struct SHA512state_st {
    SHA_LONG64 h[8];
    SHA_LONG64 Nl, Nh;
    union {
        SHA_LONG64 d[SHA_LBLOCK];
        unsigned char p[SHA512_CBLOCK];
    } u;
    unsigned int num, md_len;
} SHA512Context;

int SHA384Init(SHA512Context *c);
int SHA384Update(SHA512Context *c, const void *data, size_t len);
int SHA384Final(unsigned char *md, SHA512Context *c);
unsigned char *SHA384(const unsigned char *d, size_t n, unsigned char *md);
int SHA512Init(SHA512Context *c);
int SHA512Update(SHA512Context *c, const void *data, size_t len);
int SHA512Final(unsigned char *md, SHA512Context *c);
unsigned char *SHA512(const unsigned char *d, size_t n, unsigned char *md);
void SHA512Transform(SHA512Context *c, const unsigned char *data);

#ifdef __cplusplus
}
#endif

#endif /* !_SHA256_H */
