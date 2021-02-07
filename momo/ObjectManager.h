/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/ObjectManager.h

  namespace momo:
    struct IsTriviallyRelocatable
    struct IsNothrowMoveConstructible
    class ObjectDestroyer
    class ObjectRelocator

\**********************************************************/

#pragma once

#include "MemManager.h"

namespace momo
{

template<typename Object>
struct IsTriviallyRelocatable
	: public internal::BoolConstant<MOMO_IS_TRIVIALLY_RELOCATABLE(Object)>
{
};

template<typename Object, typename MemManager>
struct IsNothrowMoveConstructible
	: public std::is_nothrow_move_constructible<Object>
{
};

template<typename Object, typename Allocator>
struct IsNothrowMoveConstructible<Object, MemManagerStd<Allocator>>
	: public std::false_type
{
};

template<typename Object, typename AllocObject>
struct IsNothrowMoveConstructible<Object, MemManagerStd<std::allocator<AllocObject>>>
	: public std::is_nothrow_move_constructible<Object>
{
};

template<typename TObject, typename TMemManager>
class ObjectDestroyer
{
public:
	typedef TObject Object;
	typedef TMemManager MemManager;

public:
	static void Destroy(MemManager* /*memManager*/, Object& object) noexcept
	{
		(void)object;	// vs warning
		object.~Object();
	}
};

template<typename TObject, typename TMemManager>
class ObjectRelocator
{
public:
	typedef TObject Object;
	typedef TMemManager MemManager;

	static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;

	static const bool isNothrowRelocatable = isTriviallyRelocatable
		|| std::is_nothrow_move_constructible<Object>::value
		|| MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object);

public:
	static void Relocate(MemManager* memManager, Object& srcObject,
		Object* dstObject) noexcept(isNothrowRelocatable)
	{
		MOMO_ASSERT(std::addressof(srcObject) != dstObject);
		pvRelocate(memManager, srcObject, dstObject, IsTriviallyRelocatable<Object>());
	}

private:
	static void pvRelocate(MemManager* /*memManager*/, Object& srcObject, Object* dstObject,
		std::true_type /*isTriviallyRelocatable*/) noexcept
	{
		std::memcpy(dstObject, std::addressof(srcObject), sizeof(Object));
	}

	static void pvRelocate(MemManager* memManager, Object& srcObject, Object* dstObject,
		std::false_type /*isTriviallyRelocatable*/) noexcept(isNothrowRelocatable)
	{
		::new(static_cast<void*>(dstObject)) Object(std::move(srcObject));
		ObjectDestroyer<Object, MemManager>::Destroy(memManager, srcObject);
	}
};

namespace internal
{
	template<typename Object,
		typename = void>
	struct IsNothrowSwappable
	{
		static constexpr bool GetValue() noexcept
		{
			return false;
		}
	};

	template<typename Object>
	struct IsNothrowSwappable<Object,
		decltype(std::swap(std::declval<Object&>(), std::declval<Object&>()))>
	{
		static constexpr bool GetValue() noexcept
		{
			using std::swap;
			return noexcept(swap(std::declval<Object&>(), std::declval<Object&>()));
		}
	};

	template<typename TObject>
	class ObjectAlignmenter
	{
	public:
		typedef TObject Object;

		static const size_t alignment = (alignof(Object) < UIntConst::maxAlignment)
			? alignof(Object) : UIntConst::maxAlignment;

	public:
		static constexpr bool Check(size_t alignment, size_t size = sizeof(Object)) noexcept
		{
			return alignment > 0 && size % alignment == 0
				&& UIntConst::maxAlignment % alignment == 0;
		}
	};

	template<typename TObject, size_t tAlignment>
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t alignment = tAlignment;
		MOMO_STATIC_ASSERT(ObjectAlignmenter<Object>::Check(alignment));

	public:
		const Object* operator&() const noexcept
		{
			return reinterpret_cast<const Object*>(&mBuffer);
		}

		Object* operator&() noexcept
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

		typedef ObjectDestroyer<Object, MemManager> Destroyer;
		typedef ObjectRelocator<Object, MemManager> Relocator;

		static const bool isTriviallyRelocatable = Relocator::isTriviallyRelocatable;

		static const bool isNothrowRelocatable = Relocator::isNothrowRelocatable;

		static const bool isNothrowMoveConstructible =
			IsNothrowMoveConstructible<Object, MemManager>::value;

		static const bool isNothrowSwappable = IsNothrowSwappable<Object>::GetValue();

		static const bool isNothrowAnywayAssignable =
			std::is_nothrow_move_assignable<Object>::value || isNothrowSwappable
			|| isNothrowRelocatable || std::is_nothrow_copy_assignable<Object>::value;

		static const bool isNothrowShiftable = isNothrowRelocatable || isNothrowSwappable;

		static const size_t alignment = ObjectAlignmenter<Object>::alignment;

		template<typename... Args>
		class Creator
		{
		public:
			explicit Creator(MemManager& memManager, Args&&... args) noexcept
				: mMemManager(memManager),
				mArgs(std::forward<Args>(args)...)
			{
			}

			explicit Creator(MemManager& memManager, std::tuple<Args...>&& args) noexcept
				: mMemManager(memManager),
				mArgs(std::move(args))
			{
			}

			Creator(const Creator&) = delete;

			~Creator() = default;

			Creator& operator=(const Creator&) = delete;

			void operator()(Object* newObject) &&
			{
				pvCreate(mMemManager, newObject, std::index_sequence_for<Args...>());
			}

		private:
			template<typename MemManager, size_t... indexes>
			void pvCreate(MemManager& /*memManager*/, Object* newObject,
				std::index_sequence<indexes...>)
			{
				::new(static_cast<void*>(newObject))
					Object(std::forward<Args>(std::get<indexes>(mArgs))...);
			}

			template<typename Allocator, size_t... indexes>
			void pvCreate(MemManagerStd<Allocator>& memManager, Object* newObject,
				std::index_sequence<indexes...>)
			{
				std::allocator_traits<Allocator>::template rebind_traits<char>::construct(
					memManager.GetByteAllocator(), newObject,
					std::forward<Args>(std::get<indexes>(mArgs))...);
			}

		private:
			MemManager& mMemManager;
			std::tuple<Args&&...> mArgs;
		};

	public:
		static void Move(MemManager& memManager, Object&& srcObject, Object* dstObject)
			noexcept(isNothrowMoveConstructible)
		{
			Creator<Object&&>(memManager, std::move(srcObject))(dstObject);
		}

		static void Copy(MemManager& memManager, const Object& srcObject, Object* dstObject)
		{
			Creator<const Object&>(memManager, srcObject)(dstObject);
		}

		template<typename Func>
		static void MoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			Func&& func)
		{
			pvMoveExec(memManager, std::move(srcObject), dstObject, std::forward<Func>(func),
				BoolConstant<isNothrowMoveConstructible>());
		}

		template<typename Func>
		static void CopyExec(MemManager& memManager, const Object& srcObject, Object* dstObject,
			Func&& func)
		{
			Copy(memManager, srcObject, dstObject);
			try
			{
				std::forward<Func>(func)();
			}
			catch (...)
			{
				Destroy(memManager, *dstObject);
				throw;
			}
		}

		static void Destroy(MemManager& memManager, Object& object) noexcept
		{
			Destroyer::Destroy(&memManager, object);
		}

		template<typename Iterator>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
		{
			MOMO_CHECK_ITERATOR_REFERENCE(Iterator, Object);
			Iterator iter = begin;
			for (size_t i = 0; i < count; ++i, (void)++iter)
				Destroy(memManager, *iter);
		}

		static void Relocate(MemManager& memManager, Object& srcObject, Object* dstObject)
			noexcept(isNothrowRelocatable)
		{
			Relocator::Relocate(&memManager, srcObject, dstObject);
		}

		static void AssignAnyway(MemManager& memManager, Object& srcObject, Object& dstObject)
			noexcept(isNothrowAnywayAssignable)
		{
			pvAssignAnyway(memManager, srcObject, dstObject, std::is_nothrow_move_assignable<Object>(),
				BoolConstant<isNothrowSwappable>(), BoolConstant<isNothrowRelocatable>(),
				std::is_nothrow_copy_assignable<Object>());
		}

		static void Replace(MemManager& memManager, Object& srcObject, Object& dstObject)
			noexcept(isNothrowAnywayAssignable)
		{
			AssignAnyway(memManager, srcObject, dstObject);
			Destroy(memManager, srcObject);
		}

		static void ReplaceRelocate(MemManager& memManager, Object& srcObject, Object& midObject,
			Object* dstObject) noexcept(isNothrowRelocatable)
		{
			MOMO_ASSERT(std::addressof(srcObject) != std::addressof(midObject));
			pvReplaceRelocate(memManager, srcObject, midObject, dstObject,
				BoolConstant<isNothrowRelocatable>(), BoolConstant<isNothrowAnywayAssignable>());
		}

		template<typename Iterator>
		static void Relocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count) noexcept(isNothrowRelocatable)
		{
			MOMO_CHECK_ITERATOR_REFERENCE(Iterator, Object);
			pvRelocate(memManager, srcBegin, dstBegin, count, BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator, typename ObjectCreator>
		static void RelocateCreate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, ObjectCreator&& objectCreator, Object* newObject)
		{
			auto func = [&objectCreator, newObject] ()
				{ std::forward<ObjectCreator>(objectCreator)(newObject); };
			RelocateExec(memManager, srcBegin, dstBegin, count, func);
		}

		template<typename Iterator, typename Func>
		static void RelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, Func&& func)
		{
			MOMO_CHECK_ITERATOR_REFERENCE(Iterator, Object);
			pvRelocateExec(memManager, srcBegin, dstBegin, count, std::forward<Func>(func),
				BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			MOMO_STATIC_ASSERT(isNothrowShiftable);
			MOMO_CHECK_ITERATOR_REFERENCE(Iterator, Object);
			pvShiftNothrow(memManager, begin, shift, BoolConstant<isNothrowRelocatable>(),
				BoolConstant<isNothrowSwappable>());
		}

	private:
		template<typename Func>
		static void pvMoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			Func&& func, std::true_type /*isNothrowMoveConstructible*/)
		{
			std::forward<Func>(func)();
			Move(memManager, std::move(srcObject), dstObject);
		}

		template<typename Func>
		static void pvMoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			Func&& func, std::false_type /*isNothrowMoveConstructible*/)
		{
			Move(memManager, std::move(srcObject), dstObject);
			try
			{
				std::forward<Func>(func)();
			}
			catch (...)
			{
				// srcObject has been changed!
				Destroy(memManager, *dstObject);
				throw;
			}
		}

		template<bool isNothrowSwappable, bool isNothrowRelocatable, bool isNothrowCopyAssignable>
		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::true_type /*isNothrowMoveAssignable*/, BoolConstant<isNothrowSwappable>,
			BoolConstant<isNothrowRelocatable>, BoolConstant<isNothrowCopyAssignable>) noexcept
		{
			dstObject = std::move(srcObject);
		}

		template<bool isNothrowRelocatable, bool isNothrowCopyAssignable>
		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::true_type /*isNothrowSwappable*/,
			BoolConstant<isNothrowRelocatable>, BoolConstant<isNothrowCopyAssignable>) noexcept
		{
			using std::swap;
			swap(srcObject, dstObject);
		}

		template<bool isNothrowCopyAssignable>
		static void pvAssignAnyway(MemManager& memManager, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::true_type /*isNothrowRelocatable*/,
			BoolConstant<isNothrowCopyAssignable>) noexcept
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
			std::true_type /*isNothrowCopyAssignable*/) noexcept
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
			BoolConstant<isNothrowAnywayAssignable>) noexcept
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
			size_t count, std::true_type /*isNothrowRelocatable*/) noexcept
		{
			Iterator srcIter = srcBegin;
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, (void)++srcIter, (void)++dstIter)
				Relocate(memManager, *srcIter, std::addressof(*dstIter));
		}

		template<typename Iterator>
		static void pvRelocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, std::false_type /*isNothrowRelocatable*/)
		{
			if (count > 0)
			{
				RelocateCreate(memManager, std::next(srcBegin), std::next(dstBegin), count - 1,
					Creator<Object&&>(memManager, std::move(*srcBegin)), std::addressof(*dstBegin));
				Destroy(memManager, *srcBegin);
			}
		}

		template<typename Iterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, Func&& func, std::true_type /*isNothrowRelocatable*/)
		{
			std::forward<Func>(func)();
			Relocate(memManager, srcBegin, dstBegin, count);
		}

		template<typename Iterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, Func&& func, std::false_type /*isNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				Iterator srcIter = srcBegin;
				Iterator dstIter = dstBegin;
				for (; index < count; ++index, (void)++srcIter, (void)++dstIter)
					Copy(memManager, *srcIter, std::addressof(*dstIter));
				std::forward<Func>(func)();
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
			std::true_type /*isNothrowRelocatable*/, BoolConstant<isNothrowSwappable>) noexcept
		{
			ObjectBuffer<Object, alignment> objectBuffer;
			Relocate(memManager, *begin, &objectBuffer);
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, (void)++iter)
				Relocate(memManager, *std::next(iter), std::addressof(*iter));
			Relocate(memManager, *&objectBuffer, std::addressof(*iter));
		}

		template<typename Iterator>
		static void pvShiftNothrow(MemManager& /*memManager*/, Iterator begin, size_t shift,
			std::false_type /*isNothrowRelocatable*/,
			std::true_type /*isNothrowSwappable*/) noexcept
		{
			using std::swap;
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, (void)++iter)
				swap(*iter, *std::next(iter));
		}
	};
}

} // namespace momo
