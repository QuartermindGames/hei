/**
 * @file portable_endian.h
 *
 * Endianness conversion functions for a wide variety of systems,
 * including Linux, FreeBSD, MacOS X and Windows.
 */

// "License": Public Domain
// I, Mathias Panzenböck, place this file hereby into the public domain. Use it at your own risk for whatever you like.
// In case there are jurisdictions that don't support putting things in the public domain you can also consider it to
// be "dual licensed" under the BSD, MIT and Apache licenses, if you want to. This code is trivial anyway. Consider it
// an example on how to get the endian conversion functions on different platforms.

#ifndef PORTABLE_ENDIAN_H__
#define PORTABLE_ENDIAN_H__

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)
#define __WINDOWS__
#endif

#if defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#define __BYTE_ORDER BYTE_ORDER
#define __BIG_ENDIAN BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __PDP_ENDIAN PDP_ENDIAN
#elif defined(__OpenBSD__)
#include <sys/endian.h>
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>

#define be16toh(x) betoh16(x)
#define le16toh(x) letoh16(x)

#define be32toh(x) betoh32(x)
#define le32toh(x) letoh32(x)

#define be64toh(x) betoh64(x)
#define le64toh(x) letoh64(x)
#elif defined(__WINDOWS__)
#include <winsock2.h>
#include <sys/param.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define htobe16(x) htons(x)
#define htole16(x) (x)
#define be16toh(x) ntohs(x)
#define le16toh(x) (x)

#define htobe32(x) htonl(x)
#define htole32(x) (x)
#define be32toh(x) ntohl(x)
#define le32toh(x) (x)

#define htobe64(x) htonll(x)
#define htole64(x) (x)
#define be64toh(x) ntohll(x)
#define le64toh(x) (x)
#elif BYTE_ORDER == BIG_ENDIAN
/* That would be Xbox 360. */
#define htobe16(x) (x)
#define htole16(x) __builtin_bswap16(x)
#define be16toh(x) (x)
#define le16toh(x) __builtin_bswap16(x)

#define htobe32(x) (x)
#define htole32(x) __builtin_bswap32(x)
#define be32toh(x) (x)
#define le32toh(x) __builtin_bswap32(x)

#define htobe64(x) (x)
#define htole64(x) __builtin_bswap64(x)
#define be64toh(x) (x)
#define le64toh(x) __builtin_bswap64(x)
#else
#error byte order not supported
#endif

#define __BYTE_ORDER BYTE_ORDER
#define __BIG_ENDIAN BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __PDP_ENDIAN PDP_ENDIAN
#else
#error platform not supported
#endif

#ifdef __cplusplus

#include <cstdint>
#include <cstring>

#else

#include <stdint.h>
#include <string.h>

#endif

// libcaer: define macros for floating-point conversions too.
// Floats must be 4 bytes in size, so like a 32-bit integer, but their endianness
// is undefined, so we use the 32bit conversion functions and memcpy() to do the
// conversions safely (assumed integer and floating-point do have the same
// endianness on the system, which is almost always true). For further details, see:
// https://stackoverflow.com/questions/10620601/portable-serialisation-of-ieee754-floating-point-values
static inline float htobeflt(float val) {
	uint32_t rep;
	memcpy(&rep, &val, sizeof(rep));
	rep = htobe32(rep);
	memcpy(&val, &rep, sizeof(rep));
	return (val);
}

static inline float htoleflt(float val) {
	uint32_t rep;
	memcpy(&rep, &val, sizeof(rep));
	rep = htole32(rep);
	memcpy(&val, &rep, sizeof(rep));
	return (val);
}

static inline float beflttoh(float val) {
	uint32_t rep;
	memcpy(&rep, &val, sizeof(rep));
	rep = be32toh(rep);
	memcpy(&val, &rep, sizeof(rep));
	return (val);
}

static inline float leflttoh(float val) {
	uint32_t rep;
	memcpy(&rep, &val, sizeof(rep));
	rep = le32toh(rep);
	memcpy(&val, &rep, sizeof(rep));
	return (val);
}

#endif /* PORTABLE_ENDIAN_H__ */
