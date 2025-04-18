/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/ObjectManager.h

  namespace momo:
    struct IsTriviallyRelocatable
    struct IsNothrowMoveConstructible
    class ObjectDestroyer
    class ObjectRelocator

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_OBJECT_MANAGER
#define MOMO_INCLUDE_GUARD_OBJECT_MANAGER

#include "MemManager.h"

namespace momo
{

namespace internal
{
	template<typename MemManager, typename Object,
		typename = void>
	struct HasCustomMoveConstructor : public std::false_type
	{
	};

	template<typename Allocator, typename Object>
	struct HasCustomMoveConstructor<MemManagerStd<Allocator>, Object, Void<
		decltype(std::declval<typename MemManagerStd<Allocator>::ByteAllocator&>().construct(
			std::declval<Object*>(), std::declval<Object&&>())),
		EnableIf<!std::is_same<std::allocator<Byte>, typename MemManagerStd<Allocator>::ByteAllocator>::value>>>
		: public std::true_type
	{
	};

	template<typename Object>
	struct ConvertibleToReferences
	{
		operator const Object&();
		operator Object&&();
	};
}

template<typename Object>
struct IsTriviallyRelocatable
	: public internal::BoolConstant<MOMO_IS_TRIVIALLY_RELOCATABLE(Object)>
{
};

template<typename Object, typename MemManager>
struct IsNothrowMoveConstructible
	: public internal::BoolConstant<std::is_nothrow_move_constructible<Object>::value
		&& !internal::HasCustomMoveConstructor<MemManager, Object>::value>
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
		|| (std::is_move_constructible<Object>::value
			&& (MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object)));

public:
	static void Relocate(MemManager* /*memManager*/, Object& srcObject,
		Object* dstObject) noexcept(isNothrowRelocatable)
	{
		MOMO_ASSERT(std::addressof(srcObject) != dstObject);
		pvRelocate(srcObject, dstObject,
			internal::BoolConstant<isTriviallyRelocatable
				&& !std::is_nothrow_move_constructible<Object>::value>());	//?
	}

	// optional function:
	//template<typename Iterator>
	//static void RelocateNothrow(MemManager& memManager,
	//	Iterator srcBegin, Iterator dstBegin, size_t count) noexcept

	template<bool enable = isTriviallyRelocatable>
	static internal::EnableIf<enable> RelocateNothrow(MemManager& /*memManager*/,
		Object* srcObjects, Object* dstObjects, size_t count) noexcept
	{
		if (count > 0)
			std::memcpy(dstObjects, srcObjects, sizeof(Object) * count);
	}

private:
	static void pvRelocate(Object& srcObject, Object* dstObject,
		std::true_type /*isTriviallyRelocatable*/) noexcept
	{
		std::memcpy(dstObject, std::addressof(srcObject), sizeof(Object));
	}

	static void pvRelocate(Object& srcObject, Object* dstObject,
		std::false_type /*isTriviallyRelocatable*/) noexcept(isNothrowRelocatable)
	{
		::new(static_cast<void*>(dstObject)) Object(std::move(srcObject));
		srcObject.~Object();
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

	template<typename Relocator, typename Iterator,
		typename = void>
	struct HasRelocateNothrow : public std::false_type
	{
	};

	template<typename Relocator, typename Iterator>
	struct HasRelocateNothrow<Relocator, Iterator,
		decltype(Relocator::RelocateNothrow(std::declval<typename Relocator::MemManager&>(),
			std::declval<Iterator>(), std::declval<Iterator>(), size_t{}))>
		: public std::true_type
	{
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

	template<typename TObject, size_t tAlignment,
		size_t tCount = 1>
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t alignment = tAlignment;
		MOMO_STATIC_ASSERT(ObjectAlignmenter<Object>::Check(alignment));

		static const size_t count = tCount;
		MOMO_STATIC_ASSERT(count > 0);

	public:
		ObjectBuffer() = default;

		ObjectBuffer(const ObjectBuffer&) = delete;

		~ObjectBuffer() = default;

		ObjectBuffer& operator=(const ObjectBuffer&) = delete;

		template<bool isWithinLifetime = false>
		const Object* GetPtr() const noexcept
		{
			return PtrCaster::FromBytePtr<Object, isWithinLifetime, count == 1>(
				static_cast<const void*>(&mBuffer));
		}

		template<bool isWithinLifetime = false>
		Object* GetPtr() noexcept
		{
			return PtrCaster::FromBytePtr<Object, isWithinLifetime, count == 1>(
				static_cast<void*>(&mBuffer));
		}

		const Object& Get() const noexcept
		{
			MOMO_STATIC_ASSERT(count == 1);
			return *GetPtr<true>();
		}

		Object& Get() noexcept
		{
			MOMO_STATIC_ASSERT(count == 1);
			return *GetPtr<true>();
		}

	private:
		MOMO_ALIGNED_STORAGE(sizeof(Object) * count, alignment) mBuffer;
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

		static const bool isNothrowAnywayAssignable = std::is_nothrow_move_assignable<Object>::value
			|| isNothrowSwappable || isNothrowRelocatable;

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
				pvCreate(mMemManager, newObject,
					typename SequenceMaker<sizeof...(Args)>::Sequence());
			}

		private:
			template<typename MemManager, size_t... sequence>
			void pvCreate(MemManager& /*memManager*/, Object* newObject,
				Sequence<sequence...>)
			{
				::new(static_cast<void*>(newObject))
					Object(std::forward<Args>(std::get<sequence>(mArgs))...);
			}

			template<typename Allocator, size_t... sequence>
			void pvCreate(MemManagerStd<Allocator>& memManager, Object* newObject,
				Sequence<sequence...>)
			{
				std::allocator_traits<Allocator>::template rebind_traits<Byte>::construct(
					memManager.GetByteAllocator(), newObject,
					std::forward<Args>(std::get<sequence>(mArgs))...);
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

		template<typename Executor>
		static void MoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			Executor&& exec)
		{
			pvMoveExec(memManager, std::move(srcObject), dstObject, std::forward<Executor>(exec),
				BoolConstant<isNothrowMoveConstructible>());
		}

		template<typename Executor>
		static void CopyExec(MemManager& memManager, const Object& srcObject, Object* dstObject,
			Executor&& exec)
		{
			Copy(memManager, srcObject, dstObject);
			try
			{
				std::forward<Executor>(exec)();
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
			MOMO_STATIC_ASSERT(std::is_same<Object&,
				typename std::iterator_traits<Iterator>::reference>::value);
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
				BoolConstant<isNothrowSwappable>(), BoolConstant<isNothrowRelocatable>());
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
			MOMO_STATIC_ASSERT(std::is_same<Object&,
				typename std::iterator_traits<Iterator>::reference>::value);
			pvRelocate(memManager, srcBegin, dstBegin, count,
				HasRelocateNothrow<Relocator, Iterator>(), BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator, typename ObjectCreator>
		static void RelocateCreate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, ObjectCreator&& objectCreator, Object* newObject)
		{
			auto exec = [&objectCreator, newObject] ()
				{ std::forward<ObjectCreator>(objectCreator)(newObject); };
			RelocateExec(memManager, srcBegin, dstBegin, count, exec);
		}

		template<typename Iterator, typename Executor>
		static void RelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, Executor&& exec)
		{
			MOMO_STATIC_ASSERT(std::is_same<Object&,
				typename std::iterator_traits<Iterator>::reference>::value);
			pvRelocateExec(memManager, srcBegin, dstBegin, count, std::forward<Executor>(exec),
				BoolConstant<isNothrowRelocatable>());
		}

		template<typename Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			MOMO_STATIC_ASSERT(isNothrowShiftable);
			MOMO_STATIC_ASSERT(std::is_same<Object&,
				typename std::iterator_traits<Iterator>::reference>::value);
			if (shift > 0)
			{
				pvShiftNothrow(memManager, begin, shift, BoolConstant<isNothrowRelocatable>(),
					BoolConstant<isNothrowSwappable>());
			}
		}

	private:
		template<typename Executor>
		static void pvMoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			Executor&& exec, std::true_type /*isNothrowMoveConstructible*/)
		{
			std::forward<Executor>(exec)();
			Move(memManager, std::move(srcObject), dstObject);
		}

		template<typename Executor>
		static void pvMoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			Executor&& exec, std::false_type /*isNothrowMoveConstructible*/)
		{
			Move(memManager, std::move(srcObject), dstObject);
			try
			{
				std::forward<Executor>(exec)();
			}
			catch (...)
			{
				// srcObject has been changed!
				Destroy(memManager, *dstObject);
				throw;
			}
		}

		template<bool isNothrowSwappable, bool isNothrowRelocatable>
		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::true_type /*isNothrowMoveAssignable*/, BoolConstant<isNothrowSwappable>,
			BoolConstant<isNothrowRelocatable>) noexcept
		{
			dstObject = std::move(srcObject);
		}

		template<bool isNothrowRelocatable>
		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::true_type /*isNothrowSwappable*/,
			BoolConstant<isNothrowRelocatable>) noexcept
		{
			std::iter_swap(std::addressof(srcObject), std::addressof(dstObject));
		}

		static void pvAssignAnyway(MemManager& memManager, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::true_type /*isNothrowRelocatable*/) noexcept
		{
			if (std::addressof(srcObject) != std::addressof(dstObject))
			{
				ObjectBuffer<Object, alignment> objectBuffer;
				Relocate(memManager, dstObject, objectBuffer.GetPtr());
				Relocate(memManager, srcObject, std::addressof(dstObject));
				Relocate(memManager, objectBuffer.Get(), std::addressof(srcObject));
			}
		}

		static void pvAssignAnyway(MemManager& /*memManager*/, Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowSwappable*/,
			std::false_type /*isNothrowRelocatable*/)
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

		template<typename Iterator, bool isNothrowRelocatable>
		static void pvRelocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, std::true_type /*hasRelocateNothrow*/,
			BoolConstant<isNothrowRelocatable>) noexcept
		{
			Relocator::RelocateNothrow(memManager, srcBegin, dstBegin, count);
		}

		template<typename Iterator>
		static void pvRelocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, std::false_type /*hasRelocateNothrow*/,
			std::true_type /*isNothrowRelocatable*/) noexcept
		{
			Iterator srcIter = srcBegin;
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, (void)++srcIter, (void)++dstIter)
				Relocate(memManager, *srcIter, std::addressof(*dstIter));
		}

		template<typename Iterator>
		static void pvRelocate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, std::false_type /*hasRelocateNothrow*/,
			std::false_type /*isNothrowRelocatable*/)
		{
			if (count > 0)
			{
				RelocateCreate(memManager, std::next(srcBegin), std::next(dstBegin), count - 1,
					Creator<Object&&>(memManager, std::move(*srcBegin)), std::addressof(*dstBegin));
				Destroy(memManager, *srcBegin);
			}
		}

		template<typename Iterator, typename Executor>
		static void pvRelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, Executor&& exec, std::true_type /*isNothrowRelocatable*/)
		{
			std::forward<Executor>(exec)();
			Relocate(memManager, srcBegin, dstBegin, count);
		}

		template<typename Iterator, typename Executor>
		static void pvRelocateExec(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, Executor&& exec, std::false_type /*isNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				Iterator srcIter = srcBegin;
				Iterator dstIter = dstBegin;
				for (; index < count; ++index, (void)++srcIter, (void)++dstIter)
					Copy(memManager, *srcIter, std::addressof(*dstIter));
				std::forward<Executor>(exec)();
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
			Relocate(memManager, *begin, objectBuffer.GetPtr());
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, (void)++iter)
				Relocate(memManager, *std::next(iter), std::addressof(*iter));
			Relocate(memManager, objectBuffer.Get(), std::addressof(*iter));
		}

		template<typename Iterator>
		static void pvShiftNothrow(MemManager& /*memManager*/, Iterator begin, size_t shift,
			std::false_type /*isNothrowRelocatable*/,
			std::true_type /*isNothrowSwappable*/) noexcept
		{
			Iterator iter = begin;
			for (size_t i = 0; i < shift; ++i, (void)++iter)
				std::iter_swap(iter, std::next(iter));
		}
	};
}

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_OBJECT_MANAGER
