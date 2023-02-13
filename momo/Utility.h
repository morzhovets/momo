/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/Utility.h

  namespace momo:
    enum class CheckMode
    enum class ExtraCheckMode

\**********************************************************/

#pragma once

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
#include <optional>
#include <concepts>
#include <compare>
#include <bit>

#define MOMO_FRIEND_SWAP(Object) \
	friend void swap(Object& object1, Object& object2) \
		noexcept(noexcept(object1.Swap(object2))) \
	{ \
		object1.Swap(object2); \
	}

#define MOMO_FRIENDS_SIZE_BEGIN_END(Object) \
	friend auto size(const Object& object) noexcept(noexcept(object.GetCount())) \
	{ \
		return object.GetCount(); \
	} \
	friend auto begin(const Object& object) noexcept(noexcept(object.GetBegin())) \
	{ \
		return object.GetBegin(); \
	} \
	friend auto end(const Object& object) noexcept(noexcept(object.GetEnd())) \
	{ \
		return object.GetEnd(); \
	} \
	friend auto begin(Object& object) noexcept(noexcept(object.GetBegin())) \
	{ \
		return object.GetBegin(); \
	} \
	friend auto end(Object& object) noexcept(noexcept(object.GetEnd())) \
	{ \
		return object.GetEnd(); \
	}

#define MOMO_CHECK(expr) \
	do { \
		MOMO_ASSERT(Settings::checkMode != CheckMode::assertion || (expr)); \
		if constexpr (Settings::checkMode == CheckMode::exception) \
			MOMO_CHECK_EXCEPTION(expr); \
	} while (false)

#define MOMO_EXTRA_CHECK(expr) \
	MOMO_ASSERT(Settings::extraCheckMode != ExtraCheckMode::assertion || (expr))

#define MOMO_DECLARE_PROXY_CONSTRUCTOR(Object) \
	template<typename... Args> \
	explicit Object##Proxy(Args&&... args) \
		noexcept(std::is_nothrow_constructible_v<Object, Args&&...>) \
		: Object(std::forward<Args>(args)...) \
	{ \
	}

#define MOMO_DECLARE_PROXY_FUNCTION(Object, Func) \
	template<typename ObjectArg, typename... Args> \
	static decltype(auto) Func(ObjectArg&& object, Args&&... args) \
		noexcept(noexcept((std::forward<ObjectArg>(object).*&Object##Proxy::pt##Func) \
			(std::forward<Args>(args)...))) \
	{ \
		static_assert(std::is_same_v<Object, std::decay_t<ObjectArg>>); \
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
	template<typename Iterator, typename IteratorCategory>
	concept conceptIterator =
		std::is_base_of_v<IteratorCategory,
			typename std::iterator_traits<Iterator>::iterator_category>;

	template<typename Iterator>
	concept conceptInputIterator = conceptIterator<Iterator, std::input_iterator_tag>;

	template<size_t size>
	struct UIntSelector;

	template<size_t size>
	requires (size == 1)
	struct UIntSelector<size>
	{
		typedef uint8_t UInt;
	};

	template<size_t size>
	requires (size == 2)
	struct UIntSelector<size>
	{
		typedef uint16_t UInt;
	};

	template<size_t size>
	requires (2 < size && size <= 4)
	struct UIntSelector<size>
	{
		typedef uint32_t UInt;
	};

	template<size_t size>
	requires (4 < size && size <= 8)
	struct UIntSelector<size>
	{
		typedef uint64_t UInt;
	};

	class MemCopyer
	{
	public:
		template<typename Object>
		requires std::is_trivial_v<Object>
		static void ToBuffer(Object object, void* buffer) noexcept
		{
			std::memcpy(buffer, &object, sizeof(Object));
		}

		template<typename ResObject>
		requires std::is_trivial_v<ResObject>
		static ResObject FromBuffer(const void* buffer) noexcept
		{
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

		template<typename Object>
		static void ToBuffer(Object* ptr, void* ptrBuffer) noexcept
		{
			MemCopyer::ToBuffer(ptr, ptrBuffer);
		}

		template<typename ResObject = void>
		static ResObject* FromBuffer(const void* ptrBuffer) noexcept
		{
			return MemCopyer::FromBuffer<ResObject*>(ptrBuffer);
		}

		template<typename ResObject, typename Object, typename Offset>
		static ResObject* Shift(Object* ptr, Offset byteOffset) noexcept
		{
			typedef std::conditional_t<std::is_const_v<Object>, const std::byte, std::byte> Byte;
			return reinterpret_cast<ResObject*>(reinterpret_cast<Byte*>(ptr)
				+ static_cast<ptrdiff_t>(byteOffset));
		}
	};

	template<std::unsigned_integral TUInt = size_t>
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

		template<conceptInputIterator Iterator>
		static UInt Dist(Iterator begin, Iterator end)
		{
			return static_cast<UInt>(std::distance(begin, end));
		}

		template<conceptInputIterator Iterator>
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

		static constexpr UInt GCD(UInt value1, UInt value2) noexcept
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
	};

	struct UIntConst
	{
		static const uintptr_t nullPtr = MOMO_NULL_UINTPTR;
		static const uintptr_t invalidPtr = MOMO_INVALID_UINTPTR;
		static_assert(nullPtr != invalidPtr);

		static const size_t maxAlignment = MOMO_MAX_ALIGNMENT;
		static const size_t maxAllocAlignment = alignof(std::max_align_t);
		static_assert(std::has_single_bit(maxAllocAlignment));
		static_assert(maxAllocAlignment % maxAlignment == 0);

		static const size_t maxSize = SIZE_MAX;

		static const uint32_t max32 = UINT32_MAX;
	};
}

} // namespace momo
