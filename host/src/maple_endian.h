#include <stdint.h>

#if (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(WORDS_BIG_ENDIAN)
#define MAPLE_HOST_BIG_ENDIAN 1
#elif (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || defined(WORDS_LITTLE_ENDIAN)
#define MAPLE_HOST_LITTLE_ENDIAN 1
#else
#error Unable to determine byte order, please define either WORDS_BIG_ENDIAN or WORDS_LITTLE_ENDIAN
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || (defined(__clang__) && __has_builtin(__builtin_bswap16))
#define maple_bswap16 __builtin_bswap16
#else
static inline uint16_t maple_bswap16(uint16_t v) { return (v<<8)|(v>>8); }
#endif

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || (defined(__clang__) && __has_builtin(__builtin_bswap32))
#define maple_bswap32 __builtin_bswap32
#else
static inline uint32_t maple_bswap32(uint32_t v) { return (maple_bswap16(v)<<16)|maple_bswap16(v>>16); }
#endif
