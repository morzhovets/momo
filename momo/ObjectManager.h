/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/ObjectManager.h

  namespace momo:
    concept conceptObject
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
concept conceptObject = std::is_object_v<Object> &&
	!std::is_const_v<Object> && !std::is_volatile_v<Object>;

namespace internal
{
	template<conceptMemManager MemManager, conceptObject Object, typename... ObjectArgs>
	struct HasCustomConstructor : public std::false_type
	{
	};

	template<conceptObject Object, typename... ObjectArgs,
		conceptAllocatorWithConstruct<Object, ObjectArgs...> Allocator>
	struct HasCustomConstructor<MemManagerStd<Allocator>, Object, ObjectArgs...>
		: public std::true_type
	{
	};
}

template<conceptObject Object>
struct IsTriviallyRelocatable : public std::bool_constant<MOMO_IS_TRIVIALLY_RELOCATABLE(Object)>
{
};

template<conceptObject Object, conceptMemManager MemManager>
struct IsNothrowMoveConstructible
	: public std::bool_constant<std::is_nothrow_move_constructible_v<Object>
		&& !internal::HasCustomConstructor<MemManager, Object, Object&&>::value>
{
};

template<conceptObject TObject, conceptMemManager TMemManager>
class ObjectDestroyer
{
public:
	typedef TObject Object;
	typedef TMemManager MemManager;

	static const bool isNothrowDestructible = std::is_nothrow_destructible_v<Object>;

public:
	static void Destroy(MemManager* /*memManager*/, Object& object) noexcept
		requires isNothrowDestructible
	{
		if constexpr (!std::is_trivially_destructible_v<Object>)
			object.~Object();
	}
};

template<conceptObject TObject, conceptMemManager TMemManager>
class ObjectRelocator
{
public:
	typedef TObject Object;
	typedef TMemManager MemManager;

private:
	typedef ObjectDestroyer<Object, MemManager> Destroyer;

public:
	static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;

	static const bool isRelocatable = isTriviallyRelocatable
		|| (std::is_move_constructible_v<Object> && Destroyer::isNothrowDestructible);

	static const bool isNothrowRelocatable = isRelocatable
		&& (isTriviallyRelocatable || std::is_nothrow_move_constructible_v<Object>
			|| MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object));

public:
	static void Relocate(MemManager* memManager, Object& srcObject,
		Object* dstObject) noexcept(isNothrowRelocatable) requires isRelocatable
	{
		MOMO_ASSERT(std::addressof(srcObject) != dstObject);
		if constexpr (!isTriviallyRelocatable ||
			(std::is_nothrow_move_constructible_v<Object> && Destroyer::isNothrowDestructible))
		{
			std::construct_at(dstObject, std::move(srcObject));
			Destroyer::Destroy(memManager, srcObject);
		}
		else
		{
			std::memcpy(dstObject, std::addressof(srcObject), sizeof(Object));
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
	requires (ObjectAlignmenter<TObject>::Check(tAlignment))
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t alignment = tAlignment;

	public:
		explicit ObjectBuffer() noexcept = default;

		ObjectBuffer(const ObjectBuffer&) = delete;

		~ObjectBuffer() noexcept = default;

		ObjectBuffer& operator=(const ObjectBuffer&) = delete;

		const Object* operator&() const noexcept
		{
			return reinterpret_cast<const Object*>(mBuffer);
		}

		Object* operator&() noexcept
		{
			return reinterpret_cast<Object*>(mBuffer);
		}

	private:
		alignas(alignment) std::byte mBuffer[sizeof(Object)];
	};

	template<conceptMemManager TMemManager, conceptObject Object, typename... ObjectArgs>
	class ObjectCreatorAllocator : private std::allocator<std::byte>
	{
	public:
		typedef TMemManager MemManager;
		typedef std::allocator<std::byte> ByteAllocator;

	public:
		explicit ObjectCreatorAllocator(MemManager& /*memManager*/) noexcept
		{
		}

		ByteAllocator& GetByteAllocator() noexcept
		{
			return *this;
		}
	};

	template<conceptAllocator Allocator, conceptObject Object, typename... ObjectArgs>
	requires (HasCustomConstructor<MemManagerStd<Allocator>, Object, ObjectArgs...>::value)
	class ObjectCreatorAllocator<MemManagerStd<Allocator>, Object, ObjectArgs...>
		: private MemManagerPtr<MemManagerStd<Allocator>>
	{
	public:
		typedef MemManagerStd<Allocator> MemManager;
		typedef typename MemManager::ByteAllocator ByteAllocator;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

	public:
		explicit ObjectCreatorAllocator(MemManager& memManager) noexcept
			: MemManagerPtr(memManager)
		{
		}

		ByteAllocator& GetByteAllocator() noexcept
		{
			return MemManagerPtr::GetBaseMemManager().GetByteAllocator();
		}
	};

	template<typename ObjectArg>
	concept conceptPassingByValue =
		std::is_trivially_destructible_v<ObjectArg> &&
		std::is_trivially_move_constructible_v<ObjectArg> &&
		std::is_trivially_copy_constructible_v<ObjectArg> &&
		sizeof(ObjectArg) <= sizeof(void*);

	template<typename ObjectArg, size_t index>
	class ObjectCreatorArg
	{
	public:
		explicit ObjectCreatorArg(ObjectArg&& objectArg) noexcept
			: mObjectArg(std::forward<ObjectArg>(objectArg))
		{
		}

		ObjectArg&& Get() && noexcept
		{
			return std::forward<ObjectArg>(mObjectArg);
		}

	private:
		ObjectArg&& mObjectArg;
	};

	template<typename ObjectArg, size_t index>
	requires (!std::is_reference_v<ObjectArg> && conceptPassingByValue<ObjectArg>)
	class ObjectCreatorArg<ObjectArg, index>
	{
	public:
		explicit ObjectCreatorArg(ObjectArg objectArg) noexcept
			: mObjectArg(objectArg)
		{
		}

		ObjectArg Get() && noexcept
		{
			return mObjectArg;
		}

	private:
		[[no_unique_address]] ObjectArg mObjectArg;
	};

	template<typename ObjectArg, size_t index>
	class ObjectCreatorArg<ObjectArg&&, index> : public ObjectCreatorArg<ObjectArg, index>
	{
	private:
		typedef ObjectCreatorArg<ObjectArg, index> ObjectCreatorArgBase;

	public:
		using ObjectCreatorArgBase::ObjectCreatorArgBase;
	};

	template<conceptObject TObject, conceptMemManager TMemManager,
		typename Indexes, typename... ObjectArgs>
	class ObjectCreator;

	template<conceptObject TObject, conceptMemManager TMemManager,
		typename... ObjectArgs, size_t... indexes>
	requires (std::is_constructible_v<TObject, ObjectArgs...>
		|| HasCustomConstructor<TMemManager, TObject, ObjectArgs...>::value)
	class ObjectCreator<TObject, TMemManager, std::index_sequence<indexes...>, ObjectArgs...>
		: private ObjectCreatorAllocator<TMemManager, TObject, ObjectArgs...>,
		private ObjectCreatorArg<ObjectArgs, indexes>...
	{
	public:
		typedef TObject Object;
		typedef TMemManager MemManager;

	private:
		typedef ObjectCreatorAllocator<MemManager, Object, ObjectArgs...> CreatorAllocator;

	public:
		explicit ObjectCreator(MemManager& memManager, ObjectArgs&&... objectArgs) noexcept
			: CreatorAllocator(memManager),
			ObjectCreatorArg<ObjectArgs, indexes>(std::forward<ObjectArgs>(objectArgs))...
		{
		}

		explicit ObjectCreator(MemManager& memManager, std::tuple<ObjectArgs...>&& objectArgs) noexcept
			: ObjectCreator(memManager,
				std::forward<ObjectArgs>(std::get<indexes>(std::move(objectArgs)))...)
		{
		}

		ObjectCreator(ObjectCreator&&) noexcept = default;

		ObjectCreator(const ObjectCreator&) = delete;

		~ObjectCreator() noexcept = default;

		ObjectCreator& operator=(const ObjectCreator&) = delete;

		void operator()(Object* newObject) &&
		{
			std::allocator_traits<typename CreatorAllocator::ByteAllocator>::construct(
				CreatorAllocator::GetByteAllocator(), newObject,
				std::forward<ObjectArgs>(
					std::move(*static_cast<ObjectCreatorArg<ObjectArgs, indexes>*>(this)).Get())...);
		}
	};

	template<conceptObject TObject, conceptMemManager TMemManager>
	requires (conceptPassingByValue<TObject> &&
		!HasCustomConstructor<TMemManager, TObject, const TObject&>::value)
	class ObjectCreator<TObject, TMemManager, std::index_sequence<0>, const TObject&>
		: public ObjectCreator<TObject, TMemManager, std::index_sequence<0>, TObject>
	{
	private:
		typedef ObjectCreator<TObject, TMemManager, std::index_sequence<0>, TObject> ObjectCreatorBase;

	public:
		using typename ObjectCreatorBase::Object;
		using typename ObjectCreatorBase::MemManager;

	public:
		explicit ObjectCreator(MemManager& memManager, const Object& objectArg) noexcept
			: ObjectCreatorBase(memManager, objectArg)
		{
		}

		explicit ObjectCreator(MemManager& memManager, std::tuple<const Object&>&& objectArg) noexcept
			: ObjectCreatorBase(memManager, std::get<0>(objectArg))
		{
		}
	};

	template<typename TBaseCreator>
	class FastMovableCreator
	{
	public:
		typedef TBaseCreator BaseCreator;

	public:
		explicit FastMovableCreator(BaseCreator&& baseCreator) noexcept
			: mBaseCreator(std::move(baseCreator))
		{
		}

		FastMovableCreator(FastMovableCreator&&) noexcept = default;

		FastMovableCreator(const FastMovableCreator&) = delete;

		~FastMovableCreator() noexcept = default;

		FastMovableCreator& operator=(const FastMovableCreator&) = delete;

		template<typename... Objects>
		void operator()(Objects*... newObjects) &&
		{
			std::move(mBaseCreator)(newObjects...);
		}

	private:
		BaseCreator&& mBaseCreator;
	};

	template<typename TBaseCreator>
	requires (std::is_trivially_destructible_v<TBaseCreator> &&
		std::is_trivially_move_constructible_v<TBaseCreator> &&
		sizeof(TBaseCreator) <= 3 * sizeof(void*))
	class FastMovableCreator<TBaseCreator> : public TBaseCreator
	{
	public:
		typedef TBaseCreator BaseCreator;

	public:
		explicit FastMovableCreator(BaseCreator&& baseCreator) noexcept
			: BaseCreator(std::move(baseCreator))
		{
		}

		FastMovableCreator(FastMovableCreator&&) noexcept = default;

		FastMovableCreator(const FastMovableCreator&) = delete;

		~FastMovableCreator() noexcept = default;

		FastMovableCreator& operator=(const FastMovableCreator&) = delete;

		template<typename... Objects>
		void operator()(Objects*... newObjects) &&
		{
			std::move(*static_cast<BaseCreator*>(this))(newObjects...);
		}
	};

	template<conceptObject TObject, conceptMemManager TMemManager>
	class ObjectManager
	{
	public:
		typedef TObject Object;
		typedef TMemManager MemManager;

		typedef ObjectDestroyer<Object, MemManager> Destroyer;
		typedef ObjectRelocator<Object, MemManager> Relocator;

		template<typename... Args>
		using Creator = ObjectCreator<Object, MemManager, std::index_sequence_for<Args...>, Args...>;

		static const bool isTriviallyRelocatable = Relocator::isTriviallyRelocatable;

		static const bool isNothrowRelocatable = Relocator::isNothrowRelocatable;

		static const bool isNothrowMoveConstructible =
			IsNothrowMoveConstructible<Object, MemManager>::value;

		static const bool isNothrowSwappable = std::is_nothrow_swappable_v<Object>;

		static const bool isNothrowAnywayAssignable =
			std::is_nothrow_move_assignable_v<Object> || isNothrowSwappable || isNothrowRelocatable;

		static const bool isNothrowShiftable = isNothrowRelocatable || isNothrowSwappable;

		static const size_t alignment = ObjectAlignmenter<Object>::alignment;

	private:
		static const bool isNothrowDestructible = Destroyer::isNothrowDestructible;

		static const bool isRelocatable = Relocator::isRelocatable;

		static const bool isMoveConstructible = std::is_move_constructible_v<Object>
			|| HasCustomConstructor<MemManager, Object, Object&&>::value;

		static const bool isCopyConstructible = std::is_copy_constructible_v<Object>
			|| HasCustomConstructor<MemManager, Object, const Object&>::value;

		static const bool isAnywayAssignable = std::is_move_assignable_v<Object>
			|| isNothrowSwappable || isNothrowRelocatable;

	public:
		static void Move(MemManager& memManager, Object&& srcObject, Object* dstObject)
			noexcept(isNothrowMoveConstructible) requires isMoveConstructible
		{
			Creator<Object&&>(memManager, std::move(srcObject))(dstObject);
		}

		static void Copy(MemManager& memManager, const Object& srcObject, Object* dstObject)
			requires isCopyConstructible
		{
			Creator<const Object&>(memManager, srcObject)(dstObject);
		}

		template<std::invocable Func>
		static void MoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject, Func func)
			requires isMoveConstructible && isNothrowDestructible
		{
			if constexpr (isNothrowMoveConstructible)
			{
				std::move(func)();
				Move(memManager, std::move(srcObject), dstObject);
			}
			else
			{
				Move(memManager, std::move(srcObject), dstObject);
				try
				{
					std::move(func)();
				}
				catch (...)
				{
					// srcObject has been changed!
					Destroy(memManager, *dstObject);
					throw;
				}
			}
		}

		template<std::invocable Func>
		static void CopyExec(MemManager& memManager, const Object& srcObject, Object* dstObject,
			Func func) requires isCopyConstructible && isNothrowDestructible
		{
			Copy(memManager, srcObject, dstObject);
			try
			{
				std::move(func)();
			}
			catch (...)
			{
				Destroy(memManager, *dstObject);
				throw;
			}
		}

		static void Destroy(MemManager& memManager, Object& object) noexcept
			requires isNothrowDestructible
		{
			Destroyer::Destroy(&memManager, object);
		}

		template<conceptIteratorWithReference<std::input_iterator_tag, Object&> Iterator>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
			requires isNothrowDestructible
		{
			Iterator iter = begin;
			for (size_t i = 0; i < count; ++i, (void)++iter)
				Destroy(memManager, *iter);
		}

		static void Relocate(MemManager& memManager, Object& srcObject, Object* dstObject)
			noexcept(isNothrowRelocatable) requires isRelocatable
		{
			Relocator::Relocate(&memManager, srcObject, dstObject);
		}

		static void AssignAnyway(MemManager& memManager, Object& srcObject, Object& dstObject)
			noexcept(isNothrowAnywayAssignable) requires isAnywayAssignable
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
			else
			{
				dstObject = std::move(srcObject);
			}
		}

		static void Replace(MemManager& memManager, Object& srcObject, Object& dstObject)
			noexcept(isNothrowAnywayAssignable)
			requires isAnywayAssignable && isNothrowDestructible
		{
			AssignAnyway(memManager, srcObject, dstObject);
			Destroy(memManager, srcObject);
		}

		static void ReplaceRelocate(MemManager& memManager, Object& srcObject, Object& midObject,
			Object* dstObject) noexcept(isNothrowRelocatable)
			requires isNothrowRelocatable ||
				(isNothrowAnywayAssignable && isMoveConstructible && isNothrowDestructible) ||
				(isAnywayAssignable && isCopyConstructible && isNothrowDestructible)
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

		template<conceptIteratorWithReference<std::input_iterator_tag, Object&> SrcIterator,
			conceptIteratorWithReference<std::input_iterator_tag, Object&> DstIterator>
		static void Relocate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count) noexcept(isNothrowRelocatable)
			requires isNothrowRelocatable ||
				(isCopyConstructible && isMoveConstructible && isNothrowDestructible)
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

		template<conceptIteratorWithReference<std::input_iterator_tag, Object&> SrcIterator,
			conceptIteratorWithReference<std::input_iterator_tag, Object&> DstIterator,
			std::invocable<Object*> ObjectCreator>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, ObjectCreator objectCreator, Object* newObject)
			requires isNothrowRelocatable || (isCopyConstructible && isNothrowDestructible)
		{
			auto func = [objectCreator = std::move(objectCreator), newObject] () mutable
				{ std::move(objectCreator)(newObject); };
			RelocateExec(memManager, srcBegin, dstBegin, count, std::move(func));
		}

		template<conceptIteratorWithReference<std::input_iterator_tag, Object&> SrcIterator,
			conceptIteratorWithReference<std::input_iterator_tag, Object&> DstIterator,
			std::invocable Func>
		static void RelocateExec(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, Func func)
			requires isNothrowRelocatable || (isCopyConstructible && isNothrowDestructible)
		{
			if constexpr (isNothrowRelocatable)
			{
				std::move(func)();
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
					std::move(func)();
				}
				catch (...)
				{
					Destroy(memManager, dstBegin, index);
					throw;
				}
				Destroy(memManager, srcBegin, count);
			}
		}

		template<conceptIteratorWithReference<std::input_iterator_tag, Object&> Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
			requires isNothrowShiftable
		{
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
