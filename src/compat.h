/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2011  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef COMPAT_H
#define COMPAT_H

// Set BOOST_FILEYSTEM_VERSION to 2 since boost-1.46 defaults to 3
#define BOOST_FILESYSTEM_VERSION 2

#include <stddef.h>
#include <assert.h>
//We do not use cstdint, because thats not available on msvc
//but there are replacement stdint.h available for msvc
#include <stdint.h>
#include <iostream>
// TODO: This should be reworked to use CMake feature detection where possible

/* gettext support */
#include <locale.h>
#include <libintl.h>
#define _(STRING) gettext(STRING)


#include <glib.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
//#include <winsock2.h>
#include <windows.h>
//#include <intrin.h>

#undef min
#undef max
#undef RGB
#undef exception_info // Let's hope MS functions always use _exception_info
//#define snprintf _snprintf
#define isnan _isnan
#undef DOUBLE_CLICK

// Current Windows is always little-endian
#define be64toh(x) _byteswap_uint64(x)
#if 0
#include <malloc.h>
inline int aligned_malloc(void **memptr, std::size_t alignment, std::size_t size)
{
	*memptr = _aligned_malloc(size, alignment);
	return (*memptr != NULL) ? 0: -1;
}
inline void aligned_free(void *mem)
{
	_aligned_free(mem);
}
#endif
// Emulate these functions
int round(double f);
long lrint(double f);


// WINTODO: Should be set by CMake?
#define PATH_MAX 260
#define DATADIR "."
#define GNASH_PATH "NONEXISTENT_PATH_GNASH_SUPPORT_DISABLED"

#else //GCC
#	ifndef __STDC_LIMIT_MACROS
#		define __STDC_LIMIT_MACROS
#	endif

#	ifndef __STDC_CONSTANT_MACROS
#		define __STDC_CONSTANT_MACROS
#	endif


int aligned_malloc(void **memptr, std::size_t alignment, std::size_t size);
void aligned_free(void *mem);
#endif //if _WIN32

#ifdef _WIN32
// WINTODO: Hopefully, the MSVC instrinsics are similar enough
//          to what the standard mandates
#	define ATOMIC_INT32(x) __declspec(align(4)) volatile long x
#	define ATOMIC_INCREMENT(x) InterlockedIncrement(&x)
#	define ATOMIC_DECREMENT(x) InterlockedDecrement(&x)
#	define ACQUIRE_RELEASE_FLAG(x) ATOMIC_INT32(x)
#	define ACQUIRE_READ(x) InterlockedCompareExchange(const_cast<long*>(&x),1,1)
#	define RELEASE_WRITE(x, v) InterlockedExchange(&x,v)
#	define TLSDATA __declspec( thread )
#else
#	define TLSDATA __thread
#	define CALLBACK

//Support both atomic header ( gcc >= 4.6 ), and earlier ( stdatomic.h )
#	ifdef HAVE_ATOMIC
#		include <atomic>
#	else
#		include <stdatomic.h>
#	endif

#	define ATOMIC_INT32(x) std::atomic<int32_t> x
#	define ATOMIC_INCREMENT(x) x.fetch_add(1)
#	define ATOMIC_DECREMENT(x) (x.fetch_sub(1)-1)

//Boolean type con acquire release barrier semantics
#	define ACQUIRE_RELEASE_FLAG(x) std::atomic_bool x
#	define ACQUIRE_READ(x) x.load(std::memory_order_acquire)
#	define RELEASE_WRITE(x, v) x.store(v, std::memory_order_release)
#endif


/* DLL_LOCAL / DLL_PUBLIC */
#if defined _WIN32 || defined __CYGWIN__
// No DLLs, for now
#   define DLL_PUBLIC
#	define DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define DLL_PUBLIC __attribute__ ((visibility("default")))
		#define DLL_LOCAL  __attribute__ ((visibility("hidden")))
	#else
		#error GCC version less than 4
	#endif
#endif


/* min/max */
template<class T>
inline T minTmpl(T a, T b)
{
	return (a<b)?a:b;
}
template<class T>
inline T maxTmpl(T a, T b)
{
	return (a>b)?a:b;
}
#define imin minTmpl<int>
#define imax maxTmpl<int>
#define dmin minTmpl<double>
#define dmax maxTmpl<double>

/* timing */

uint64_t compat_msectiming();
void compat_msleep(unsigned int time);
uint64_t compat_get_current_time_ms();
uint64_t compat_get_current_time_us();
uint64_t compat_get_thread_cputime_us();

int kill_child(GPid p);

/* byte order */
#if defined(__FreeBSD__)
#	include <sys/endian.h>
#elif defined(__APPLE__)
#	define _BSD_SOURCE
#	include <architecture/byte_order.h>
#elif defined(_WIN32)
	//windows is always le
#	define	__LITTLE_ENDIAN	1234
#	define	__BIG_ENDIAN	4321
#	define __BYTE_ORDER	__LITTLE_ENDIAN
#	define be32toh(x) _byteswap_ulong(x)
#	define be16toh(x) _byteswap_ushort(x)
#else
#	include <endian.h>
#endif

#if __BYTE_ORDER == __BIG_ENDIAN

inline uint16_t LittleEndianToHost16(uint16_t x)
{
	return le16toh(x);
}

inline uint32_t LittleEndianToSignedHost24(uint32_t x)
{
	uint32_t ret=le32toh(x);
	assert(ret<0x1000000);
	//Sign extend
	if(ret&0x800000)
		ret|=0xff000000;
	return ret;
}

inline uint32_t LittleEndianToUnsignedHost24(uint32_t x)
{
	assert(x<0x1000000);
	uint32_t ret=le32toh(x);
	return ret;
}

inline uint32_t LittleEndianToHost32(uint32_t x)
{
	return le32toh(x);
}

inline uint64_t LittleEndianToHost64(uint64_t x)
{
	return le64toh(x);
}

inline uint16_t BigEndianToHost16(uint16_t x)
{
	return x;
}

inline uint32_t BigEndianToSignedHost24(uint32_t x)
{
	//Sign extend
	x>>=8;
	assert(x<0x1000000);
	if(x&0x800000)
		x|=0xff000000;
	return x;
}

inline uint32_t BigEndianToUnsignedHost24(uint32_t x)
{
	x>>=8;
	assert(x<0x1000000);
	return x;
}

inline uint32_t BigEndianToHost32(uint32_t x)
{
	return x;
}

inline uint64_t BigEndianToHost64(uint64_t x)
{
	return x;
}

#else //__BYTE_ORDER == __LITTLE_ENDIAN
inline uint16_t LittleEndianToHost16(uint16_t x)
{
	return x;
}

inline uint32_t LittleEndianToSignedHost24(uint32_t x)
{
	assert(x<0x1000000);
	if(x&0x800000)
		x|=0xff000000;
	return x;
}

inline uint32_t LittleEndianToUnsignedHost24(uint32_t x)
{
	assert(x<0x1000000);
	return x;
}

inline uint32_t LittleEndianToHost32(uint32_t x)
{
	return x;
}

inline uint64_t LittleEndianToHost64(uint64_t x)
{
	return x;
}

inline uint16_t BigEndianToHost16(uint16_t x)
{
	return be16toh(x);
}

inline uint32_t BigEndianToSignedHost24(uint32_t x)
{
	assert(x<0x1000000);
	//Discard the lowest byte, as it was the highest
	uint32_t ret=be32toh(x)>>8;
	//Sign extend
	if(ret&0x800000)
		ret|=0xff000000;
	return ret;
}

inline uint32_t BigEndianToUnsignedHost24(uint32_t x)
{
	assert(x<0x1000000);
	//Discard the lowest byte, as it was the highest
	uint32_t ret=be32toh(x)>>8;
	return ret;
}

inline uint32_t BigEndianToHost32(uint32_t x)
{
	return be32toh(x);
}

inline uint64_t BigEndianToHost64(uint64_t x)
{
	return be64toh(x);
}

#endif // __BYTE_ORDER == __BIG_ENDIAN

#endif
