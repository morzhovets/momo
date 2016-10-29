/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/ObjectManager.h

  namespace momo:
    struct IsTriviallyRelocatable
    struct IsNothrowMoveConstructible

\**********************************************************/

#pragma once

#include "MemManager.h"

#define MOMO_ALIGNMENT_OF(Object) ((MOMO_MAX_ALIGNMENT < std::alignment_of<Object>::value) \
	? MOMO_MAX_ALIGNMENT : std::alignment_of<Object>::value)

namespace momo
{

template<typename Object>
struct IsTriviallyRelocatable
	: public internal::BoolConstant<MOMO_IS_TRIVIALLY_RELOCATABLE(Object)>
{
};

template<typename Object, typename MemManager, bool onRelocate>
struct IsNothrowMoveConstructible
	: public internal::BoolConstant<MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object)>
{
};

template<typename Object, typename Allocator>
struct IsNothrowMoveConstructible<Object, MemManagerStd<Allocator>, false>
	: public std::false_type
{
};

template<typename Object, typename AllocObject>
struct IsNothrowMoveConstructible<Object, MemManagerStd<std::allocator<AllocObject>>, false>
	: public internal::BoolConstant<MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object)>
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

	template<typename TObject, typename TMemManager>
	class ObjectManager
	{
	public:
		typedef TObject Object;
		typedef TMemManager MemManager;

		static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;

		static const bool isNothrowMoveConstructible =
			IsNothrowMoveConstructible<Object, MemManager, false>::value;

		static const bool isNothrowSwappable = MOMO_IS_NOTHROW_SWAPPABLE(Object);

		static const bool isNothrowRelocatable = isTriviallyRelocatable
			|| IsNothrowMoveConstructible<Object, MemManager, true>::value;

		static const bool isNothrowAnywayAssignable =
			std::is_nothrow_move_assignable<Object>::value || isNothrowSwappable
			|| isNothrowRelocatable || std::is_nothrow_copy_assignable<Object>::value;

		static const bool isNothrowShiftable = isNothrowRelocatable || isNothrowSwappable;

		static const size_t alignment = MOMO_ALIGNMENT_OF(Object);

		template<typename... Args>
		class Creator
		{
		public:
			explicit Creator(MemManager& memManager, Args&&... args) MOMO_NOEXCEPT
				: mMemManager(memManager),
				mArgs(std::forward<Args>(args)...)
			{
			}

			Creator(MemManager& memManager, std::tuple<Args...>&& args) MOMO_NOEXCEPT
				: mMemManager(memManager),
				mArgs(std::move(args))
			{
			}

			Creator(const Creator&) = delete;

			~Creator() MOMO_NOEXCEPT
			{
			}

			Creator& operator=(const Creator&) = delete;

			void operator()(Object* newObject) const
			{
				pvCreate(mMemManager, newObject,
					typename MakeSequence<sizeof...(Args)>::Sequence());
			}

		private:
			template<typename MemManager, size_t... sequence>
			void pvCreate(MemManager& /*memManager*/, Object* newObject,
				Sequence<sequence...>) const
			{
				new(newObject) Object(std::forward<Args>(std::get<sequence>(mArgs))...);
			}

			template<typename Allocator, size_t... sequence>
			void pvCreate(MemManagerStd<Allocator>& memManager, Object* newObject,
				Sequence<sequence...>) const
			{
				std::allocator_traits<Allocator>::template rebind_traits<char>::construct(
					memManager.GetCharAllocator(), newObject,
					std::forward<Args>(std::get<sequence>(mArgs))...);
			}

		private:
			MemManager& mMemManager;
			std::tuple<Args&&...> mArgs;
		};

	public:
		static void Move(MemManager& memManager, Object&& srcObject, Object* dstObject)
			MOMO_NOEXCEPT_IF(isNothrowMoveConstructible)
		{
			Creator<Object>(memManager, std::move(srcObject))(dstObject);
		}

		static void Copy(MemManager& memManager, const Object& srcObject, Object* dstObject)
		{
			Creator<const Object&>(memManager, srcObject)(dstObject);
		}

		template<typename Func>
		static void MoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			const Func& func)
		{
			if (isNothrowMoveConstructible)
			{
				func();
				Move(memManager, std::move(srcObject), dstObject);
			}
			else
			{
				Move(memManager, std::move(srcObject), dstObject);
				try
				{
					func();
				}
				catch (...)
				{
					// srcObject has been changed!
					pvDestroy(memManager, *dstObject);
					throw;
				}
			}
		}

		template<typename Func>
		static void CopyExec(MemManager& memManager, const Object& srcObject, Object* dstObject,
			const Func& func)
		{
			Copy(memManager, srcObject, dstObject);
			try
			{
				func();
			}
			catch (...)
			{
				pvDestroy(memManager, *dstObject);
				throw;
			}
		}

		static void Destroy(MemManager& memManager, Object& object) MOMO_NOEXCEPT
		{
			pvDestroy(memManager, object);
		}

		template<typename Iterator>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) MOMO_NOEXCEPT
		{
			MOMO_CHECK_TYPE(Object, *begin);
			Iterator iter = begin;
			for (size_t i = 0; i < count; ++i, ++iter)
				pvDestroy(memManager, *iter);
		}

		static void Relocate(MemManager& memManager, Object& srcObject, Object* dstObject)
			MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			MOMO_ASSERT(std::addressof(srcObject) != dstObject);
			pvRelocate(memManager, srcObject, dstObject, BoolConstant<isTriviallyRelocatable>());
		}

		static void AssignAnyway(MemManager& memManager, Object& srcObject, Object& dstObject)
			MOMO_NOEXCEPT_IF(isNothrowAnywayAssignable)
		{
			pvAssignAnyway(memManager, srcObject, dstObject, std::is_nothrow_move_assignable<Object>(),
				BoolConstant<isNothrowSwappable>(), BoolConstant<isNothrowRelocatable>(),
				std::is_nothrow_copy_assignable<Object>());
		}

		static void Replace(MemManager& memManager, Object& srcObject, Object& dstObject)
			MOMO_NOEXCEPT_IF(isNothrowAnywayAssignable)
		{
			AssignAnyway(memManager, srcObject, dstObject);
			pvDestroy(memManager, srcObject);
		}

		static void ReplaceRelocate(MemManager& memManager, Object& srcObject, Object& midObject,
			Object* dstObject) MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			MOMO_ASSERT(std::addressof(srcObject) != std::addressof(midObject));
			pvReplaceRelocate(memManager, srcObject, midObject, dstObject,
				BoolConstant<isNothrowRelocatable>(), BoolConstant<isNothrowAnywayAssignable>());
		}

		template<typename Iterator>
		static void Relocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count) MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			MOMO_CHECK_TYPE(Object, *srcBegin);
			pvRelocate(memManager, srcBegin, dstBegin, count, BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator, typename ObjectCreator>
		static void RelocateCreate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, const ObjectCreator& objectCreator, Object* newObject)
		{
			auto func = [&objectCreator, newObject] () { objectCreator(newObject); };
			RelocateExec(memManager, srcBegin, dstBegin, count, func);
		}

		template<typename Iterator, typename Func>
		static void RelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, const Func& func)
		{
			MOMO_CHECK_TYPE(Object, *srcBegin);
			pvRelocateExec(memManager, srcBegin, dstBegin, count, func,
				BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(isNothrowShiftable);
			MOMO_CHECK_TYPE(Object, *begin);
			pvShiftNothrow(memManager, begin, shift, BoolConstant<isNothrowRelocatable>(),
				BoolConstant<isNothrowSwappable>());
		}

	private:
		template<typename MemManager>
		static void pvDestroy(MemManager& /*memManager*/, Object& object) MOMO_NOEXCEPT
		{
			(void)object;	// vs warning
			object.~Object();
		}

		template<typename Allocator>
		static void pvDestroy(MemManagerStd<Allocator>& memManager, Object& object) MOMO_NOEXCEPT
		{
			std::allocator_traits<Allocator>::template rebind_traits<char>::destroy(
				memManager.GetCharAllocator(), std::addressof(object));
		}

		static void pvRelocate(MemManager& /*memManager*/, Object& srcObject, Object* dstObject,
			std::true_type /*isTriviallyRelocatable*/) MOMO_NOEXCEPT
		{
			memcpy(dstObject, std::addressof(srcObject), sizeof(Object));
		}

		static void pvRelocate(MemManager& memManager, Object& srcObject, Object* dstObject,
			std::false_type /*isTriviallyRelocatable*/)
			MOMO_NOEXCEPT_IF((IsNothrowMoveConstructible<Object, MemManager, true>::value))
		{
			Move(memManager, std::move(srcObject), dstObject);
			pvDestroy(memManager, srcObject);
		}

		template<bool isNothrowSwappable, bool isNothrowRelocatable, bool isNothrowCopyAssignable>
		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::true_type /*isNothrowMoveAssignable*/, BoolConstant<isNothrowSwappable>,
			BoolConstant<isNothrowRelocatable>, BoolConstant<isNothrowCopyAssignable>) MOMO_NOEXCEPT
		{
			dstObject = std::move(srcObject);
		}

		template<bool isNothrowRelocatable, bool isNothrowCopyAssignable>
		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::true_type /*isNothrowSwappable*/,
			BoolConstant<isNothrowRelocatable>, BoolConstant<isNothrowCopyAssignable>) MOMO_NOEXCEPT
		{
			using std::swap;
			swap(srcObject, dstObject);
		}

		template<bool isNothrowCopyAssignable>
		static void pvAssignAnyway(MemManager& memManager, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::true_type /*isNothrowRelocatable*/,
			BoolConstant<isNothrowCopyAssignable>) MOMO_NOEXCEPT
		{
			if (std::addressof(srcObject) != std::addressof(dstObject))
			{
				ObjectBuffer<Object, alignment> objectBuffer;
				Relocate(memManager, dstObject, &objectBuffer);
				Relocate(memManager, srcObject, std::addressof(dstObject));
				Relocate(memManager, *&objectBuffer, std::addressof(srcObject));
			}
		}

		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::false_type /*isNothrowRelocatable*/,
			std::true_type /*isNothrowCopyAssignable*/) MOMO_NOEXCEPT
		{
			dstObject = static_cast<const Object&>(srcObject);
		}

		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::false_type /*isNothrowRelocatable*/, std::false_type /*isNothrowCopyAssignable*/)
		{
			dstObject = std::move(srcObject);
		}

		template<bool isNothrowAnywayAssignable>
		static void pvReplaceRelocate(MemManager& memManager, Object& srcObject, Object& midObject,
			Object* dstObject, std::true_type /*isNothrowRelocatable*/,
			BoolConstant<isNothrowAnywayAssignable>) MOMO_NOEXCEPT
		{
			Relocate(memManager, midObject, dstObject);
			Relocate(memManager, srcObject, std::addressof(midObject));
		}

		static void pvReplaceRelocate(MemManager& memManager, Object& srcObject, Object& midObject,
			Object* dstObject, std::false_type /*isNothrowRelocatable*/,
			std::true_type /*isNothrowAnywayAssignable*/)
		{
			Move(memManager, std::move(midObject), dstObject);
			Replace(memManager, srcObject, midObject);
		}

		static void pvReplaceRelocate(MemManager& memManager, Object& srcObject, Object& midObject,
			Object* dstObject, std::false_type /*isNothrowRelocatable*/,
			std::false_type /*isNothrowAnywayAssignable*/)
		{
			Copy(memManager, midObject, dstObject);
			try
			{
				Replace(memManager, srcObject, midObject);
			}
			catch (...)
			{
				Destroy(memManager, *dstObject);
				throw;
			}
		}

		template<typename Iterator>
		static void pvRelocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, std::true_type /*isNothrowRelocatable*/) MOMO_NOEXCEPT
		{
			Iterator srcIter = srcBegin;
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, ++srcIter, ++dstIter)
				Relocate(memManager, *srcIter, std::addressof(*dstIter));
		}

		template<typename Iterator>
		static void pvRelocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, std::false_type /*isNothrowRelocatable*/)
		{
			if (count > 0)
			{
				RelocateCreate(memManager, std::next(srcBegin), std::next(dstBegin), count - 1,
					Creator<Object>(memManager, std::move(*srcBegin)), std::addressof(*dstBegin));
				pvDestroy(memManager, *srcBegin);
			}
		}

		template<typename Iterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, const Func& func, std::true_type /*isNothrowRelocatable*/)
		{
			func();
			Relocate(memManager, srcBegin, dstBegin, count);
		}

		template<typename Iterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, const Func& func, std::false_type /*isNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				Iterator srcIter = srcBegin;
				Iterator dstIter = dstBegin;
				for (; index < count; ++index, ++srcIter, ++dstIter)
					Copy(memManager, *srcIter, std::addressof(*dstIter));
				func();
			}
			catch (...)
			{
				Destroy(memManager, dstBegin, index);
				throw;
			}
			Destroy(memManager, srcBegin, count);
		}

		template<typename Iterator, bool isNothrowSwappable>
		static void pvShiftNothrow(MemManager& memManager, Iterator begin, size_t shift,
			std::true_type /*isNothrowRelocatable*/, BoolConstant<isNothrowSwappable>) MOMO_NOEXCEPT
		{
			ObjectBuffer<Object, alignment> objectBuffer;
			Relocate(memManager, *begin, &objectBuffer);
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, ++iter)
				Relocate(memManager, *std::next(iter), std::addressof(*iter));
			Relocate(memManager, *&objectBuffer, std::addressof(*iter));
		}

		template<typename Iterator>
		static void pvShiftNothrow(MemManager& /*memManager*/, Iterator begin, size_t shift,
			std::false_type /*isNothrowRelocatable*/,
			std::true_type /*isNothrowSwappable*/) MOMO_NOEXCEPT
		{
			using std::swap;
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, ++iter)
				swap(*iter, *std::next(iter));
		}
	};
}

} // namespace momo
