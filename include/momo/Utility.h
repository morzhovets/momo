/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/Utility.h

  namespace momo:
    enum class CheckMode
    enum class ExtraCheckMode

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_UTILITY
#define MOMO_INCLUDE_GUARD_UTILITY

#include "UserSettings.h"

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <utility>
#include <new>
#include <iterator>
#include <type_traits>
#include <array>
#include <tuple>
#include <initializer_list>

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
# include <compare>
#endif

#define MOMO_FRIEND_SWAP(Object) \
	friend void swap(Object& object1, Object& object2) \
		noexcept(noexcept(object1.Swap(object2))) \
	{ \
		object1.Swap(object2); \
	}

#define MOMO_FRIENDS_SIZE_BEGIN_END_CONST(Object, ConstIterator) \
	friend size_t size(const Object& object) noexcept(noexcept(object.GetCount())) \
	{ \
		return object.GetCount(); \
	} \
	friend ConstIterator begin(const Object& object) noexcept(noexcept(object.GetBegin())) \
	{ \
		return object.GetBegin(); \
	} \
	friend ConstIterator end(const Object& object) noexcept(noexcept(object.GetEnd())) \
	{ \
		return object.GetEnd(); \
	}

#define MOMO_FRIENDS_BEGIN_END(Object, Iterator) \
	friend Iterator begin(Object& object) noexcept(noexcept(object.GetBegin())) \
	{ \
		return object.GetBegin(); \
	} \
	friend Iterator end(Object& object) noexcept(noexcept(object.GetEnd())) \
	{ \
		return object.GetEnd(); \
	}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
# define MOMO_MORE_COMPARISON_OPERATORS(ObjectArg)
#else
# define MOMO_MORE_COMPARISON_OPERATORS(ObjectArg) \
	friend bool operator!=(ObjectArg obj1, ObjectArg obj2) noexcept(noexcept(obj1 == obj2)) \
	{ \
		return !(obj1 == obj2); \
	} \
	friend bool operator>(ObjectArg obj1, ObjectArg obj2) \
	{ \
		return obj2 < obj1; \
	} \
	friend bool operator<=(ObjectArg obj1, ObjectArg obj2) \
	{ \
		return !(obj2 < obj1); \
	} \
	friend bool operator>=(ObjectArg obj1, ObjectArg obj2) \
	{ \
		return obj2 <= obj1; \
	}
#endif

#define MOMO_STATIC_ASSERT(expr) static_assert((expr), #expr)

#define MOMO_CHECK(expr) \
	do { \
		MOMO_ASSERT(Settings::checkMode != CheckMode::assertion || (expr)); \
		if (Settings::checkMode == CheckMode::exception) MOMO_CHECK_EXCEPTION(expr); \
	} while (false)

#define MOMO_EXTRA_CHECK(expr) \
	MOMO_ASSERT(Settings::extraCheckMode != ExtraCheckMode::assertion || (expr))

#define MOMO_DECLARE_PROXY_CONSTRUCTOR(Object) \
	template<typename... Args> \
	explicit Object##Proxy(Args&&... args) \
		noexcept((std::is_nothrow_constructible<Object, Args&&...>::value)) \
		: Object(std::forward<Args>(args)...) \
	{ \
	}

// Result = decltype((std::declval<ObjectArg&&>().*&Object##Proxy::pt##Func)(std::declval<Args&&>()...))	// gcc
#define MOMO_DECLARE_PROXY_FUNCTION(Object, Func, Result) \
	template<typename ObjectArg, typename... Args> \
	static Result Func(ObjectArg&& object, Args&&... args) \
		noexcept(noexcept((std::forward<ObjectArg>(object).*&Object##Proxy::pt##Func) \
			(std::forward<Args>(args)...))) \
	{ \
		MOMO_STATIC_ASSERT((std::is_same<Object, typename std::decay<ObjectArg>::type>::value)); \
		return (std::forward<ObjectArg>(object).*&Object##Proxy::pt##Func) \
			(std::forward<Args>(args)...); \
	}

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
	typedef unsigned char Byte;

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
	struct SequenceMaker : public SequenceMaker<count - 1, count - 1, sequence...>
	{
	};

	template<size_t... sequence>
	struct SequenceMaker<0, sequence...>
	{
		typedef internal::Sequence<sequence...> Sequence;
	};

	//template<typename...>
	//using Void = void;	// gcc 4.9

	template<typename...>
	struct VoidMaker
	{
		typedef void Void;
	};

	template<typename... Types>
	using Void = typename VoidMaker<Types...>::Void;

	template<typename Func, typename Result, typename... Args>
	struct IsInvocable : public std::false_type
	{
	};

	template<typename Func, typename... Args>
	struct IsInvocable<Func, decltype(std::declval<Func>()(std::declval<Args>()...)), Args...>
		: public std::true_type
	{
	};

	template<bool value,
		typename Type = void>
	using EnableIf = typename std::enable_if<value, Type>::type;

	template<typename Type>
	using Identity = EnableIf<true, Type>;

	template<size_t size,
		typename = void>
	struct UIntSelector;

	template<>
	struct UIntSelector<1, void>
	{
		typedef uint8_t UInt;
	};

	template<>
	struct UIntSelector<2, void>
	{
		typedef uint16_t UInt;
	};

	template<size_t size>
	struct UIntSelector<size, EnableIf<(2 < size && size <= 4)>>
	{
		typedef uint32_t UInt;
	};

	template<size_t size>
	struct UIntSelector<size, EnableIf<(4 < size && size <= 8)>>
	{
		typedef uint64_t UInt;
	};

	class MemCopyer
	{
	public:
		template<typename Object>
		static void ToBuffer(Object object, void* buffer) noexcept
		{
			MOMO_STATIC_ASSERT(std::is_trivial<Object>::value);
			std::memcpy(buffer, &object, sizeof(Object));
		}

		template<typename ResObject>
		static ResObject FromBuffer(const void* buffer) noexcept
		{
			MOMO_STATIC_ASSERT(std::is_trivial<ResObject>::value);
			ResObject object{};
			std::memcpy(&object, buffer, sizeof(ResObject));
			return object;
		}
	};

	class PtrCaster
	{
	public:
		template<typename Object>
		static uintptr_t ToUInt(Object* ptr) noexcept
		{
			return reinterpret_cast<uintptr_t>(ptr);
		}

		template<typename ResObject = void>
		static ResObject* FromUInt(uintptr_t intPtr) noexcept
		{
			return reinterpret_cast<ResObject*>(intPtr);
		}

		template<typename ResObject, typename Object, typename Offset>
		static ResObject* Shift(Object* ptr, Offset byteOffset) noexcept
		{
			typedef typename std::conditional<std::is_const<Object>::value,
				const Byte, Byte>::type CByte;
			return reinterpret_cast<ResObject*>(reinterpret_cast<CByte*>(ptr)
				+ static_cast<ptrdiff_t>(byteOffset));
		}
	};

	template<typename TUInt = size_t>
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
		static constexpr UInt Ceil(UInt value, UInt mod) noexcept
		{
			return ((value + mod - 1) / mod) * mod;
		}

		template<typename Iterator>
		static UInt Dist(Iterator begin, Iterator end)
		{
			return static_cast<UInt>(std::distance(begin, end));
		}

		template<typename Iterator>
		static Iterator Next(Iterator iter, UInt dist)
		{
			return std::next(iter, static_cast<ptrdiff_t>(dist));
		}

		static bool GetBit(const UInt* data, size_t bitIndex)
		{
			static const size_t bitSize = sizeof(UInt) * 8;
			return (data[bitIndex / bitSize] & (UInt{1} << static_cast<UInt>(bitIndex % bitSize))) != 0;
		}

		static void SetBit(UInt* data, size_t bitIndex)
		{
			static const size_t bitSize = sizeof(UInt) * 8;
			data[bitIndex / bitSize] |= UInt{1} << static_cast<UInt>(bitIndex % bitSize);
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

		static UInt Log2(UInt value) noexcept
		{
			return pvLog2(value);
		}

	private:
		template<size_t size = sizeof(UInt)>
		static EnableIf<size == 4,
		UInt> pvLog2(UInt value) noexcept
		{
			static const UInt tab32[32] =
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
			return tab32[(value * UInt{0x07C4ACDD}) >> 27];
		}

		template<size_t size = sizeof(UInt)>
		static EnableIf<size == 8,
		UInt> pvLog2(UInt value) noexcept
		{
			static const UInt tab64[64] =
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
			return tab64[(value * UInt{0x07EDD5E59A4E28C2}) >> 58];
		}
	};

	struct UIntConst
	{
		static const uintptr_t nullPtr = MOMO_NULL_UINTPTR;
		static const uintptr_t invalidPtr = MOMO_INVALID_UINTPTR;
		MOMO_STATIC_ASSERT(nullPtr != invalidPtr);

		static const size_t maxAlignment = MOMO_MAX_ALIGNMENT;
		static const size_t maxAllocAlignment = alignof(std::max_align_t);
		MOMO_STATIC_ASSERT(maxAllocAlignment != 0 && (maxAllocAlignment & (maxAllocAlignment - 1)) == 0);
		MOMO_STATIC_ASSERT(maxAllocAlignment % maxAlignment == 0);

		static const size_t maxSize = SIZE_MAX;

		static const uint32_t max32 = UINT32_MAX;
	};
}

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_UTILITY
