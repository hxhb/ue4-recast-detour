#pragma once

#include <stdio.h>
#include <stdarg.h>

#ifdef __clang__
template <typename T>
auto ArrayCountHelper(T& t) -> typename TEnableIf<__is_array(T), char(&)[sizeof(t) / sizeof(t[0]) + 1]>::Type;
#else
template <typename T, unsigned int N>
char(&ArrayCountHelper(const T(&)[N]))[N + 1];
#endif

// Number of elements in an array.
#define ARRAY_COUNT( array ) (sizeof(ArrayCountHelper(array)) - 1)

static int GetVarArgs(char* Dest, size_t DestSize, const char*& Fmt, va_list ArgPtr)
{
	int Result = vsnprintf(Dest, DestSize, Fmt, ArgPtr);
	va_end(ArgPtr);
	return (Result != -1 && Result < (int)DestSize) ? Result : -1;
}


namespace FMath
{
	template< class T > 
	static constexpr inline T Min( const T A, const T B )
	{
		return (A<=B) ? A : B;
	}

	/** Multiples value by itself */
	template< class T >
	static inline T Square(const T A)
	{
		return A * A;
	}
}