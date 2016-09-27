/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/ObjectManager.h

  namespace momo:
    struct IsTriviallyRelocatable
    struct IsNothrowSwappable

\**********************************************************/

#pragma once

#include "Utility.h"

#define MOMO_ALIGNMENT_OF(Object) ((MOMO_MAX_ALIGNMENT < std::alignment_of<Object>::value) \
	? MOMO_MAX_ALIGNMENT : std::alignment_of<Object>::value)

namespace momo
{

template<typename Object>
struct IsTriviallyRelocatable
#ifdef MOMO_USE_TRIVIALLY_COPYABLE
	: public std::is_trivially_copyable<Object>
#else
	: public std::is_trivial<Object>
#endif
{
};

template<typename Object>
struct IsNothrowSwappable
#ifdef MOMO_USE_NOTHROW_SWAPPABLE
	: public std::is_nothrow_swappable<Object>
#else
	: public std::false_type
#endif
{
};

namespace internal
{
	template<typename TObject, size_t tAlignment>
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t alignment = tAlignment;
		//MOMO_STATIC_ASSERT(alignment > 0 && ((alignment - 1) & alignment) == 0);

	public:
		const Object* operator&() const MOMO_NOEXCEPT
		{
			return reinterpret_cast<const Object*>(&mBuffer);
		}

		Object* operator&() MOMO_NOEXCEPT
		{
			return reinterpret_cast<Object*>(&mBuffer);
		}

	private:
		typename std::aligned_storage<sizeof(Object), alignment>::type mBuffer;
	};

	template<typename TObject>
	struct ObjectManager
	{
		typedef TObject Object;

		static const bool isNothrowMoveConstructible = MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object);

		static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;

		static const bool isNothrowRelocatable = isTriviallyRelocatable
			|| isNothrowMoveConstructible;

		static const bool isNothrowAnywayAssignable =
			std::is_nothrow_move_assignable<Object>::value || IsNothrowSwappable<Object>::value
			|| isTriviallyRelocatable || isNothrowMoveConstructible
			|| std::is_nothrow_copy_assignable<Object>::value;

		static const bool isNothrowShiftable = isNothrowRelocatable
			|| IsNothrowSwappable<Object>::value;

		static const size_t alignment = MOMO_ALIGNMENT_OF(Object);

		template<typename... Args>
		class Creator
		{
		public:
			explicit Creator(Args&&... args)
				: mArgs(std::forward<Args>(args)...)
			{
			}

			explicit Creator(std::tuple<Args...>&& args)
				: mArgs(std::move(args))
			{
			}

			Creator(const Creator&) = delete;

			~Creator() MOMO_NOEXCEPT
			{
			}

			Creator& operator=(const Creator&) = delete;

			void operator()(Object* newObject) const
			{
				_Create(newObject, typename MakeSequence<sizeof...(Args)>::Sequence());
			}

		private:
			template<size_t... sequence>
			void _Create(Object* newObject, Sequence<sequence...>) const
			{
				new(newObject) Object(std::forward<Args>(std::get<sequence>(mArgs))...);
			}

		private:
			std::tuple<Args&&...> mArgs;
		};

		static void Move(Object&& srcObject, Object* dstObject)
			MOMO_NOEXCEPT_IF(isNothrowMoveConstructible)
		{
			new(dstObject) Object(std::move(srcObject));
		}

		static void Copy(const Object& srcObject, Object* dstObject)
		{
			new(dstObject) Object(srcObject);
		}

		static void Destroy(Object& object) MOMO_NOEXCEPT
		{
			(void)object;	// vs warning
			object.~Object();
		}

		template<typename Iterator>
		static void Destroy(Iterator begin, size_t count) MOMO_NOEXCEPT
		{
			MOMO_CHECK_TYPE(Object, *begin);
			if (!std::is_trivially_destructible<Object>::value)
			{
				Iterator iter = begin;
				for (size_t i = 0; i < count; ++i, ++iter)
					Destroy(*iter);
			}
		}

		static void AssignNothrowAnyway(Object&& srcObject, Object& dstObject) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(isNothrowAnywayAssignable);
			_AssignNothrowAnyway(std::move(srcObject), dstObject,
				std::is_nothrow_move_assignable<Object>(), IsNothrowSwappable<Object>(),
				BoolConstant<isTriviallyRelocatable>(), BoolConstant<isNothrowMoveConstructible>());
		}

		static void Relocate(Object& srcObject, Object* dstObject)
			MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			MOMO_ASSERT(std::addressof(srcObject) != dstObject);
			_Relocate(srcObject, dstObject, BoolConstant<isTriviallyRelocatable>());
		}

		template<typename Iterator>
		static void Relocate(Iterator srcBegin, Iterator dstBegin, size_t count)
			MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			MOMO_CHECK_TYPE(Object, *srcBegin);
			_Relocate(srcBegin, dstBegin, count, BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator, typename ObjectCreator>
		static void RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const ObjectCreator& objectCreator, Object* newObject)
		{
			MOMO_CHECK_TYPE(Object, *srcBegin);
			_RelocateCreate(srcBegin, dstBegin, count, objectCreator, newObject,
				BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator>
		static void ShiftNothrow(Iterator begin, size_t shift) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(isNothrowShiftable);
			MOMO_CHECK_TYPE(Object, *begin);
			_ShiftNothrow(begin, shift, BoolConstant<isNothrowRelocatable>());
		}

	private:
		static void _SwapNothrowAdl(Object& object1, Object& object2) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(IsNothrowSwappable<Object>::value);
			using std::swap;
			swap(object1, object2);
		}

		static void _SwapNothrowMemory(Object& object1, Object& object2) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(isTriviallyRelocatable);
			static const size_t size = sizeof(Object);
			ObjectBuffer<Object, alignment> objectBuffer;
			memcpy(&objectBuffer, std::addressof(object1), size);
			memcpy(std::addressof(object1), std::addressof(object2), size);
			memcpy(std::addressof(object2), &objectBuffer, size);
		}

		template<bool isNothrowSwappable, bool isTriviallyRelocatable, bool isNothrowMoveConstructible>
		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::true_type /*isNothrowMoveAssignable*/, BoolConstant<isNothrowSwappable>,
			BoolConstant<isTriviallyRelocatable>, BoolConstant<isNothrowMoveConstructible>) MOMO_NOEXCEPT
		{
			dstObject = std::move(srcObject);
		}

		template<bool isTriviallyRelocatable, bool isNothrowMoveConstructible>
		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::true_type /*isNothrowSwappable*/,
			BoolConstant<isTriviallyRelocatable>, BoolConstant<isNothrowMoveConstructible>) MOMO_NOEXCEPT
		{
			_SwapNothrowAdl(srcObject, dstObject);
		}

		template<bool isNothrowMoveConstructible>
		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::true_type /*isTriviallyRelocatable*/,
			BoolConstant<isNothrowMoveConstructible>) MOMO_NOEXCEPT
		{
			_SwapNothrowMemory(srcObject, dstObject);
		}

		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::false_type /*isTriviallyRelocatable*/,
			std::true_type /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			if (std::addressof(srcObject) != std::addressof(dstObject))
			{
				Destroy(dstObject);
				Move(std::move(srcObject), std::addressof(dstObject));
			}
		}

		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::false_type /*isTriviallyRelocatable*/,
			std::false_type /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(std::is_nothrow_copy_assignable<Object>::value);
			dstObject = static_cast<const Object&>(srcObject);
		}

		static void _Relocate(Object& srcObject, Object* dstObject,
			std::true_type /*isTriviallyRelocatable*/) MOMO_NOEXCEPT
		{
			memcpy(dstObject, std::addressof(srcObject), sizeof(Object));
		}

		static void _Relocate(Object& srcObject, Object* dstObject,
			std::false_type /*isTriviallyRelocatable*/) MOMO_NOEXCEPT_IF(isNothrowMoveConstructible)
		{
			Move(std::move(srcObject), dstObject);
			Destroy(srcObject);
		}

		template<typename Iterator>
		static void _Relocate(Iterator srcBegin, Iterator dstBegin, size_t count,
			std::true_type /*isNothrowRelocatable*/) MOMO_NOEXCEPT
		{
			Iterator srcIter = srcBegin;
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, ++srcIter, ++dstIter)
				Relocate(*srcIter, std::addressof(*dstIter));
		}

		template<typename Iterator>
		static void _Relocate(Iterator srcBegin, Iterator dstBegin, size_t count,
			std::false_type /*isNothrowRelocatable*/)
		{
			if (count > 0)
			{
				_RelocateCreate(std::next(srcBegin), std::next(dstBegin), count - 1,
					Creator<Object>(std::move(*srcBegin)), std::addressof(*dstBegin),
					std::false_type());
				Destroy(*srcBegin);
			}
		}

		template<typename Iterator, typename ObjectCreator>
		static void _RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const ObjectCreator& objectCreator, Object* newObject,
			std::true_type /*isNothrowRelocatable*/)
		{
			objectCreator(newObject);
			Relocate(srcBegin, dstBegin, count);
		}

		template<typename Iterator, typename ObjectCreator>
		static void _RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const ObjectCreator& objectCreator, Object* newObject,
			std::false_type /*isNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				Iterator srcIter = srcBegin;
				Iterator dstIter = dstBegin;
				for (; index < count; ++index, ++srcIter, ++dstIter)
					Copy(*srcIter, std::addressof(*dstIter));
				objectCreator(newObject);
			}
			catch (...)
			{
				Destroy(dstBegin, index);
				throw;
			}
			Destroy(srcBegin, count);
		}

		template<typename Iterator>
		static void _ShiftNothrow(Iterator begin, size_t shift,
			std::true_type /*isNothrowRelocatable*/) MOMO_NOEXCEPT
		{
			ObjectBuffer<Object, alignment> objectBuffer;
			Relocate(*begin, &objectBuffer);
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, ++iter)
				Relocate(*std::next(iter), std::addressof(*iter));
			Relocate(*&objectBuffer, std::addressof(*iter));
		}

		template<typename Iterator>
		static void _ShiftNothrow(Iterator begin, size_t shift,
			std::false_type /*isNothrowRelocatable*/) MOMO_NOEXCEPT
		{
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, ++iter)
				_SwapNothrowAdl(*iter, *std::next(iter));
		}
	};
}

} // namespace momo
