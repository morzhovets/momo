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

#ifdef MOMO_USE_INIT_LISTS
#include <initializer_list>
#endif

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

#ifdef MOMO_USE_DELETE_FUNCS
#define MOMO_DISABLE_COPY_CONSTRUCTOR(Class) Class(const Class&) = delete;
#define MOMO_DISABLE_COPY_OPERATOR(Class) Class& operator=(const Class&) = delete;
#else
#define MOMO_DISABLE_COPY_CONSTRUCTOR(Class) Class(const Class&);
#define MOMO_DISABLE_COPY_OPERATOR(Class) Class& operator=(const Class&);
#endif

#define MOMO_FRIEND_SWAP(Container) \
	friend void swap(Container& cont1, Container& cont2) MOMO_NOEXCEPT \
	{ \
		cont1.Swap(cont2); \
	}

#define MOMO_FRIENDS_BEGIN_END(ContainerRef, Iterator) \
	friend Iterator begin(ContainerRef contRef) MOMO_NOEXCEPT \
	{ \
		return contRef.GetBegin(); \
	} \
	friend Iterator end(ContainerRef contRef) MOMO_NOEXCEPT \
	{ \
		return contRef.GetEnd(); \
	}

#define MOMO_STATIC_ASSERT(expr) static_assert((expr), #expr);

#define MOMO_CHECK_TYPE(Type, var) \
	MOMO_STATIC_ASSERT((std::is_same<Type, typename std::decay<decltype(var)>::type>::value));

#define MOMO_CHECK(expr) assert(Settings::checkMode != CheckMode::assertion || (expr)); \
	if (Settings::checkMode != CheckMode::exception || (expr)) ; else throw std::invalid_argument(#expr);

#define MOMO_EXTRA_CHECK(expr) assert(Settings::extraCheckMode != ExtraCheckMode::assertion || (expr));

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
struct IsTriviallyRelocatable : public MOMO_IS_TRIVIALLY_COPYABLE<Object>
{
};

namespace internal
{
	template<typename Iterator>
	struct IsForwardIterator : public std::is_base_of<std::forward_iterator_tag,
		typename std::iterator_traits<Iterator>::iterator_category>
	{
	};

	template<bool value>
	struct BoolConstant : public std::integral_constant<bool, value>
	{
	};

#ifdef MOMO_USE_VARIADIC_TEMPLATES
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
#endif

	template<typename TValue>
	struct Log2;

	template<>
	struct Log2<uint32_t>
	{
		typedef uint32_t Value;

		static size_t Calc(uint32_t value) MOMO_NOEXCEPT
		{
			static const size_t tab32[32] =
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
	};

	template<>
	struct Log2<uint64_t>
	{
		typedef uint32_t Value;

		static size_t Calc(uint64_t value) MOMO_NOEXCEPT
		{
			static const size_t tab64[64] =
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
	};
}

} // namespace momo
