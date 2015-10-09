/**********************************************************\

  momo/Utility.h

  namespace momo:
    enum class CheckMode
    enum class ExtraCheckMode
    struct IsTriviallyRelocatable

\**********************************************************/

#pragma once

#include "Settings.h"

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <cstring>
#include <memory>
#include <cassert>
#include <exception>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <initializer_list>

#ifdef MOMO_USE_MEM_MANAGER_WIN
#include <Windows.h>
#endif

#if MOMO_USE_UNSAFE_MOVE_CONSTRUCTORS == 1
#define MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object) \
	std::is_nothrow_move_constructible<Object>::value \
	|| !std::is_copy_constructible<Object>::value
#elif MOMO_USE_UNSAFE_MOVE_CONSTRUCTORS == 2
#define MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object) true
#else
#define MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object) \
	std::is_nothrow_move_constructible<Object>::value
#endif

#define MOMO_ALIGNMENT_OF(Object) ((MOMO_MAX_ALIGNMENT < std::alignment_of<Object>::value) \
	? MOMO_MAX_ALIGNMENT : std::alignment_of<Object>::value)

#ifdef MOMO_USE_NOEXCEPT
#define MOMO_NOEXCEPT noexcept
#define MOMO_NOEXCEPT_IF(expr) noexcept(expr)
#else
#define MOMO_NOEXCEPT throw()
#define MOMO_NOEXCEPT_IF(expr)
#endif

#define MOMO_FRIEND_SWAP(Object) \
	friend void swap(Object& object1, Object& object2) MOMO_NOEXCEPT \
	{ \
		object1.Swap(object2); \
	}

#define MOMO_FRIENDS_BEGIN_END(Reference, Iterator) \
	friend Iterator begin(Reference ref) MOMO_NOEXCEPT \
	{ \
		return ref.GetBegin(); \
	} \
	friend Iterator end(Reference ref) MOMO_NOEXCEPT \
	{ \
		return ref.GetEnd(); \
	}

#define MOMO_STATIC_ASSERT(expr) static_assert((expr), #expr);

#define MOMO_CHECK_TYPE(Type, var) \
	MOMO_STATIC_ASSERT((std::is_same<Type, typename std::decay<decltype(var)>::type>::value));

#define MOMO_CHECK(expr) assert(Settings::checkMode != CheckMode::assertion || (expr)); \
	if (Settings::checkMode != CheckMode::exception || (expr)) ; else throw std::invalid_argument(#expr);

#define MOMO_EXTRA_CHECK(expr) assert(Settings::extraCheckMode != ExtraCheckMode::assertion || (expr));

#define MOMO_SWITCH16(var, Func, ...) \
	switch (var) \
	{ \
		case  1: return Func< 1>(__VA_ARGS__); \
		case  2: return Func< 2>(__VA_ARGS__); \
		case  3: return Func< 3>(__VA_ARGS__); \
		case  4: return Func< 4>(__VA_ARGS__); \
		case  5: return Func< 5>(__VA_ARGS__); \
		case  6: return Func< 6>(__VA_ARGS__); \
		case  7: return Func< 7>(__VA_ARGS__); \
		case  8: return Func< 8>(__VA_ARGS__); \
		case  9: return Func< 9>(__VA_ARGS__); \
		case 10: return Func<10>(__VA_ARGS__); \
		case 11: return Func<11>(__VA_ARGS__); \
		case 12: return Func<12>(__VA_ARGS__); \
		case 13: return Func<13>(__VA_ARGS__); \
		case 14: return Func<14>(__VA_ARGS__); \
		case 15: return Func<15>(__VA_ARGS__); \
		case 16: return Func<16>(__VA_ARGS__); \
	}

MOMO_STATIC_ASSERT(MOMO_MAX_ALIGNMENT > 0 && (MOMO_MAX_ALIGNMENT & (MOMO_MAX_ALIGNMENT - 1)) == 0);

namespace momo
{

enum class CheckMode
{
	assertion = 1,
	exception = 2,
	bydefault = MOMO_DEFAULT_CHECK_MODE,
};

enum class ExtraCheckMode
{
	nothing = 0,
	assertion = 1,
	bydefault = MOMO_DEFAULT_EXTRA_CHECK_MODE,
};

template<typename Object>
struct IsTriviallyRelocatable
#ifdef MOMO_USE_TRIVIALLY_COPIABLE
	: public std::is_trivially_copyable<Object>
#else
	: public std::is_trivial<Object>
#endif
{
};

namespace internal
{
	template<typename Iterator>
	using IsForwardIterator = std::is_base_of<std::forward_iterator_tag,
		typename std::iterator_traits<Iterator>::iterator_category>;

	template<bool value>
	using BoolConstant = std::integral_constant<bool, value>;

	template<size_t... sequence>
	struct Sequence
	{
	};

	template<size_t count, size_t... sequence>
	struct MakeSequence : public MakeSequence<count - 1, count - 1, sequence...>
	{
	};

	template<size_t... sequence>
	struct MakeSequence<0, sequence...>
	{
		typedef internal::Sequence<sequence...> Sequence;
	};

	template<typename TUInt>
	struct UIntMath
	{
		typedef TUInt UInt;

		struct DivResult
		{
			UInt quotient;
			UInt remainder;
		};

		static UInt Ceil(UInt value, UInt mod) MOMO_NOEXCEPT
		{
			assert(value != 0 && mod != 0);
			return (((value - 1) / mod) + 1) * mod;
		}

		static UInt GCD(UInt value1, UInt value2) MOMO_NOEXCEPT
		{
			while (value2 != 0)
			{
				size_t value3 = value1 % value2;
				value1 = value2;
				value2 = value3;
			}
			return value1;
		}

		template<UInt mod>
		static DivResult DivByConst(UInt value) MOMO_NOEXCEPT
		{
			DivResult result;
			result.quotient = value / mod;
			result.remainder = value - result.quotient * mod;
			return result;
		}

		static DivResult DivBySmall(UInt value, UInt mod) MOMO_NOEXCEPT
		{
			MOMO_SWITCH16(mod, DivByConst, value);
			DivResult result;
			result.quotient = value / mod;
			result.remainder = value % mod;
			return result;
		}

		static UInt Log2(UInt value) MOMO_NOEXCEPT;
	};

	template<>
	inline uint32_t UIntMath<uint32_t>::Log2(uint32_t value) MOMO_NOEXCEPT
	{
		static const uint32_t tab32[32] =
		{
			 0,  9,  1, 10, 13, 21,  2, 29,
			11, 14, 16, 18, 22, 25,  3, 30,
			 8, 12, 20, 28, 15, 17, 24,  7,
			19, 27, 23,  6, 26,  5,  4, 31,
		};
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		return tab32[(value * (uint32_t)0x07C4ACDD) >> 27];
	}

	template<>
	inline uint64_t UIntMath<uint64_t>::Log2(uint64_t value) MOMO_NOEXCEPT
	{
		static const uint64_t tab64[64] =
		{
			63,  0, 58,  1, 59, 47, 53,  2,
			60, 39, 48, 27, 54, 33, 42,  3,
			61, 51, 37, 40, 49, 18, 28, 20,
			55, 30, 34, 11, 43, 14, 22,  4,
			62, 57, 46, 52, 38, 26, 32, 41,
			50, 36, 17, 19, 29, 10, 13, 21,
			56, 45, 25, 31, 35, 16,  9, 12,
			44, 24, 15,  8, 23,  7,  6,  5,
		};
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		value |= value >> 32;
		value -= value >> 1;
		return tab64[(value * (uint64_t)0x07EDD5E59A4E28C2) >> 58];
	}
}

} // namespace momo
