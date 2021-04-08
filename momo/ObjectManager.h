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
concept conceptObject = std::is_object_v<Object>
	&& !std::is_const_v<Object> && !std::is_volatile_v<Object>;

template<conceptObject Object>
struct IsTriviallyRelocatable : public std::bool_constant<MOMO_IS_TRIVIALLY_RELOCATABLE(Object)>
{
};

template<conceptObject Object, conceptMemManager MemManager>
struct IsNothrowMoveConstructible
	: public std::is_nothrow_move_constructible<Object>
{
};

template<conceptObject Object, typename Allocator>
struct IsNothrowMoveConstructible<Object, MemManagerStd<Allocator>>
	: public std::false_type
{
};

template<conceptObject Object, typename AllocObject>
struct IsNothrowMoveConstructible<Object, MemManagerStd<std::allocator<AllocObject>>>
	: public std::is_nothrow_move_constructible<Object>
{
};

template<conceptObject TObject, conceptMemManager TMemManager>
class ObjectDestroyer
{
public:
	typedef TObject Object;
	typedef TMemManager MemManager;

public:
	static void Destroy(MemManager* /*memManager*/, Object& object) noexcept
	{
		if constexpr (!std::is_trivially_destructible_v<Object>)
			std::destroy_at(std::addressof(object));
	}
};

template<conceptObject TObject, conceptMemManager TMemManager>
class ObjectRelocator
{
public:
	typedef TObject Object;
	typedef TMemManager MemManager;

	static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;

	static const bool isNothrowRelocatable = isTriviallyRelocatable
		|| std::is_nothrow_move_constructible_v<Object>
		|| MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object);

public:
	static void Relocate(MemManager* memManager, Object& srcObject,
		Object* dstObject) noexcept(isNothrowRelocatable)
	{
		MOMO_ASSERT(std::addressof(srcObject) != dstObject);
		if constexpr (isTriviallyRelocatable)
		{
			std::memcpy(dstObject, std::addressof(srcObject), sizeof(Object));
		}
		else
		{
			std::construct_at(dstObject, std::move(srcObject));
			ObjectDestroyer<Object, MemManager>::Destroy(memManager, srcObject);
		}
	}
};

namespace internal
{
	template<conceptObject TObject>
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

	template<conceptObject TObject, size_t tAlignment>
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t alignment = tAlignment;
		static_assert(ObjectAlignmenter<Object>::Check(alignment));

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
		std::aligned_storage_t<sizeof(Object), alignment> mBuffer;
	};

	template<conceptObject TObject, conceptMemManager TMemManager>
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

		static const bool isNothrowSwappable = std::is_nothrow_swappable_v<Object>;

		static const bool isNothrowAnywayAssignable =
			std::is_nothrow_move_assignable_v<Object> || isNothrowSwappable
			|| isNothrowRelocatable || std::is_nothrow_copy_assignable_v<Object>;

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
				std::construct_at(newObject, std::forward<Args>(std::get<indexes>(mArgs))...);
			}

			template<typename Allocator, size_t... indexes>
			void pvCreate(MemManagerStd<Allocator>& memManager, Object* newObject,
				std::index_sequence<indexes...>)
			{
				std::allocator_traits<Allocator>::template rebind_traits<std::byte>::construct(
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
		requires std::invocable<Func&&>
		static void MoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			Func&& func)
		{
			if constexpr (isNothrowMoveConstructible)
			{
				std::forward<Func>(func)();
				Move(memManager, std::move(srcObject), dstObject);
			}
			else
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
		}

		template<typename Func>
		requires std::invocable<Func&&>
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

		template<conceptInputIterator Iterator>
		requires std::is_same_v<Object&, std::iter_reference_t<Iterator>>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
		{
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
			if constexpr (std::is_nothrow_move_assignable_v<Object>)
			{
				dstObject = std::move(srcObject);
			}
			else if constexpr (isNothrowSwappable)
			{
				std::iter_swap(std::addressof(srcObject), std::addressof(dstObject));
			}
			else if constexpr (isNothrowRelocatable)
			{
				if (std::addressof(srcObject) != std::addressof(dstObject))
				{
					ObjectBuffer<Object, alignment> objectBuffer;
					Relocate(memManager, dstObject, &objectBuffer);
					Relocate(memManager, srcObject, std::addressof(dstObject));
					Relocate(memManager, *&objectBuffer, std::addressof(srcObject));
				}
			}
			else if constexpr (std::is_nothrow_copy_assignable_v<Object>)
			{
				dstObject = static_cast<const Object&>(srcObject);
			}
			else
			{
				dstObject = std::move(srcObject);
			}
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
			if constexpr (isNothrowRelocatable)
			{
				Relocate(memManager, midObject, dstObject);
				Relocate(memManager, srcObject, std::addressof(midObject));
			}
			else if constexpr (isNothrowAnywayAssignable)
			{
				Move(memManager, std::move(midObject), dstObject);
				Replace(memManager, srcObject, midObject);
			}
			else
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
		}

		template<conceptInputIterator SrcIterator, conceptInputIterator DstIterator>
		requires std::is_same_v<Object&, std::iter_reference_t<SrcIterator>> &&
			std::is_same_v<Object&, std::iter_reference_t<DstIterator>>
		static void Relocate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count) noexcept(isNothrowRelocatable)
		{
			if constexpr (isNothrowRelocatable)
			{
				SrcIterator srcIter = srcBegin;
				DstIterator dstIter = dstBegin;
				for (size_t i = 0; i < count; ++i, (void)++srcIter, (void)++dstIter)
					Relocate(memManager, *srcIter, std::addressof(*dstIter));
			}
			else
			{
				if (count > 0)
				{
					Object& srcObject0 = *srcBegin;
					Object& dstObject0 = *dstBegin;
					RelocateCreate(memManager, std::next(srcBegin), std::next(dstBegin), count - 1,
						Creator<Object&&>(memManager, std::move(srcObject0)), std::addressof(dstObject0));
					Destroy(memManager, srcObject0);
				}
			}
		}

		template<conceptInputIterator SrcIterator, conceptInputIterator DstIterator,
			typename ObjectCreator>
		requires std::is_same_v<Object&, std::iter_reference_t<SrcIterator>> &&
			std::is_same_v<Object&, std::iter_reference_t<DstIterator>> &&
			std::invocable<ObjectCreator&&, Object*>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, ObjectCreator&& objectCreator, Object* newObject)
		{
			auto func = [&objectCreator, newObject] ()
				{ std::forward<ObjectCreator>(objectCreator)(newObject); };
			RelocateExec(memManager, srcBegin, dstBegin, count, func);
		}

		template<conceptInputIterator SrcIterator, conceptInputIterator DstIterator, typename Func>
		requires std::is_same_v<Object&, std::iter_reference_t<SrcIterator>> &&
			std::is_same_v<Object&, std::iter_reference_t<DstIterator>> &&
			std::invocable<Func&&>
		static void RelocateExec(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, Func&& func)
		{
			if constexpr (isNothrowRelocatable)
			{
				std::forward<Func>(func)();
				Relocate(memManager, srcBegin, dstBegin, count);
			}
			else
			{
				size_t index = 0;
				try
				{
					SrcIterator srcIter = srcBegin;
					DstIterator dstIter = dstBegin;
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
		}

		template<conceptInputIterator Iterator>
		requires std::is_same_v<Object&, std::iter_reference_t<Iterator>>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			static_assert(isNothrowShiftable);
			if (shift == 0)
				return;
			if constexpr (isNothrowRelocatable)
			{
				ObjectBuffer<Object, alignment> objectBuffer;
				Iterator iter = begin;
				Object* objectPtr = std::addressof(*iter);
				Relocate(memManager, *objectPtr, &objectBuffer);
				for (size_t i = 0; i < shift; ++i)
				{
					++iter;
					Object* nextObjectPtr = std::addressof(*iter);
					Relocate(memManager, *nextObjectPtr, objectPtr);
					objectPtr = nextObjectPtr;
				}
				Relocate(memManager, *&objectBuffer, objectPtr);
			}
			else
			{
				Iterator iter = begin;
				Object* objectPtr = std::addressof(*iter);
				for (size_t i = 0; i < shift; ++i)
				{
					++iter;
					Object* nextObjectPtr = std::addressof(*iter);
					std::iter_swap(objectPtr, nextObjectPtr);
					objectPtr = nextObjectPtr;
				}
			}
		}
	};
}

} // namespace momo
