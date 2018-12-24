/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/Utility.h

  namespace momo:
    enum class CheckMode
    enum class ExtraCheckMode

\**********************************************************/

#pragma once

#include "UserSettings.h"

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
#include <utility>
#include <array>
#include <initializer_list>

#ifdef MOMO_USE_MEM_MANAGER_WIN
#include <Windows.h>
#endif

#ifdef MOMO_USE_SSE2
#include <emmintrin.h>
#include <xmmintrin.h>
#endif

#define MOMO_FRIEND_SWAP(Object) \
	friend void swap(Object& object1, Object& object2) \
		noexcept(noexcept(object1.Swap(object2))) \
	{ \
		object1.Swap(object2); \
	}

#define MOMO_FRIENDS_BEGIN_END(Reference, Iterator) \
	friend Iterator begin(Reference ref) noexcept(noexcept(ref.GetBegin())) \
	{ \
		return ref.GetBegin(); \
	} \
	friend Iterator end(Reference ref) noexcept(noexcept(ref.GetEnd())) \
	{ \
		return ref.GetEnd(); \
	}

#define MOMO_STATIC_ASSERT(expr) static_assert((expr), #expr)

#define MOMO_CHECK_ITERATOR_REFERENCE(Iterator, Type) MOMO_STATIC_ASSERT((std::is_same<Type, \
	typename std::decay<typename std::iterator_traits<Iterator>::reference>::type>::value) \
	&& std::is_reference<typename std::iterator_traits<Iterator>::reference>::value)

#define MOMO_CHECK(expr) \
	do { \
		MOMO_ASSERT(Settings::checkMode != CheckMode::assertion || (expr)); \
		if (Settings::checkMode == CheckMode::exception && !(expr)) throw std::invalid_argument(#expr); \
	} while (false)

#define MOMO_EXTRA_CHECK(expr) MOMO_ASSERT(Settings::extraCheckMode != ExtraCheckMode::assertion || (expr))

#define MOMO_DECLARE_PROXY_CONSTRUCTOR(BaseClass) \
	template<typename... Args> \
	explicit BaseClass##Proxy(Args&&... args) \
		noexcept((std::is_nothrow_constructible<BaseClass, Args&&...>::value)) \
		: BaseClass(std::forward<Args>(args)...) \
	{ \
	}

// Result = decltype((std::declval<Object&&>().*&BaseClass##Proxy::pt##Func)(std::declval<Args&&>()...))	// gcc
#define MOMO_DECLARE_PROXY_FUNCTION(BaseClass, Func, Result) \
	template<typename Object, typename... Args> \
	static Result Func(Object&& object, Args&&... args) \
		noexcept(noexcept((std::forward<Object>(object).*&BaseClass##Proxy::pt##Func) \
			(std::forward<Args>(args)...))) \
	{ \
		return (std::forward<Object>(object).*&BaseClass##Proxy::pt##Func) \
			(std::forward<Args>(args)...); \
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

namespace internal
{
	template<size_t size,
		typename Default = void>
	struct UIntSelector
	{
		typedef Default UInt;
	};

	template<typename Default>
	struct UIntSelector<1, Default>
	{
		typedef uint8_t UInt;
	};

	template<typename Default>
	struct UIntSelector<2, Default>
	{
		typedef uint16_t UInt;
	};

	template<typename Default>
	struct UIntSelector<4, Default>
	{
		typedef uint32_t UInt;
	};

	template<typename Default>
	struct UIntSelector<8, Default>
	{
		typedef uint64_t UInt;
	};

	struct UIntPtrConst
	{
		static const uintptr_t null = MOMO_NULL_UINTPTR;
		static const uintptr_t invalid = MOMO_INVALID_UINTPTR;
		MOMO_STATIC_ASSERT(null != invalid);
	};

	template<typename Iterator>
	using IsForwardIterator = std::is_base_of<std::forward_iterator_tag,
		typename std::iterator_traits<Iterator>::iterator_category>;

	template<bool value>
	using BoolConstant = std::integral_constant<bool, value>;

	template<typename Allocator,
		typename = bool>
	struct IsAllocatorAlwaysEqual : public std::is_empty<Allocator>
	{
	};

	template<typename Allocator>
	struct IsAllocatorAlwaysEqual<Allocator, decltype(Allocator::is_always_equal::value)>
		: public Allocator::is_always_equal
	{
	};

	template<size_t... sequence>
	struct Sequence
	{
	};

	template<size_t count, size_t... sequence>
	struct SequenceMaker : public SequenceMaker<count - 1, count - 1, sequence...>
	{
	};

	template<size_t... sequence>
	struct SequenceMaker<0, sequence...>
	{
		typedef internal::Sequence<sequence...> Sequence;
	};

	//template<typename...>
	//using Void = void;

	template<typename... Types>
	struct VoidMaker
	{
		typedef void Void;
	};

	template<typename... Types>
	using Void = typename VoidMaker<Types...>::Void;

	template<typename TUInt>
	class UIntMath
	{
	public:
		typedef TUInt UInt;

		struct DivResult
		{
			UInt quotient;
			UInt remainder;
		};

	public:
		static UInt Ceil(UInt value, UInt mod) noexcept
		{
			MOMO_ASSERT(value != 0 && mod != 0);
			return (((value - 1) / mod) + 1) * mod;
		}

		static UInt GCD(UInt value1, UInt value2) noexcept
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
		static DivResult DivByConst(UInt value) noexcept
		{
			DivResult result;
			result.quotient = value / mod;
			result.remainder = value - result.quotient * mod;
			return result;
		}

		static DivResult DivBySmall(UInt value, UInt mod) noexcept
		{
			switch (mod)
			{
			case 1:
				return DivByConst<1>(value);
			case 2:
				return DivByConst<2>(value);
			case 3:
				return DivByConst<3>(value);
			case 4:
				return DivByConst<4>(value);
			}
			DivResult result;
			result.quotient = value / mod;
			result.remainder = value % mod;
			return result;
		}

		static UInt Log2(UInt value) noexcept;
	};

	template<>
	inline uint32_t UIntMath<uint32_t>::Log2(uint32_t value) noexcept
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
	inline uint64_t UIntMath<uint64_t>::Log2(uint64_t value) noexcept
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
