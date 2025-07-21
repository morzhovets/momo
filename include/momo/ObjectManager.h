/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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
	template<typename MemManagerOrNullPtr, typename MemManager>
	concept conceptMemManagerOrNullPtr = conceptMemManager<MemManager> &&
		std::convertible_to<MemManagerOrNullPtr, MemManager*>;

	template<conceptMemManager MemManager, conceptObject Object, typename... ObjectArgs>
	struct HasCustomConstructor : public std::false_type
	{
	};

	template<conceptObject Object, typename... ObjectArgs,
		conceptByteAllocatorWithConstruct<Object, ObjectArgs...> ByteAllocator>
	struct HasCustomConstructor<MemManagerStdByte<ByteAllocator>, Object, ObjectArgs...>
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
	template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
	static void Destroy(MemManagerOrNullPtr /*memManager*/, Object& object) noexcept
		requires isNothrowDestructible
	{
		object.~Object();
	}
};

template<conceptObject TObject, conceptMemManager TMemManager>
class ObjectRelocator
{
public:
	typedef TObject Object;
	typedef TMemManager MemManager;

public:
	static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;

	static const bool isRelocatable = isTriviallyRelocatable
		|| (std::is_move_constructible_v<Object> && std::is_nothrow_destructible_v<Object>);

	static const bool isNothrowRelocatable = isRelocatable
		&& (isTriviallyRelocatable || std::is_nothrow_move_constructible_v<Object>
			|| (MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object)));

public:
	template<internal::conceptMemManagerOrNullPtr<MemManager> SrcMemManagerOrNullPtr,
		internal::conceptMemManagerOrNullPtr<MemManager> DstMemManagerOrNullPtr>
	static void Relocate(SrcMemManagerOrNullPtr /*srcMemManager*/, DstMemManagerOrNullPtr /*dstMemManager*/,
		Object& srcObject, Object* dstObject) noexcept(isNothrowRelocatable) requires isRelocatable
	{
		MOMO_ASSERT(std::addressof(srcObject) != dstObject);
		if constexpr (isTriviallyRelocatable &&
			!(std::is_nothrow_move_constructible_v<Object> && std::is_nothrow_destructible_v<Object>))	//?
		{
			std::memcpy(dstObject, std::addressof(srcObject), sizeof(Object));
		}
		else
		{
			std::construct_at(dstObject, std::move(srcObject));
			srcObject.~Object();
		}
	}

	// optional function:
	//template<internal::conceptIncIterator<Object> SrcIterator,
	//	internal::conceptIncIterator<Object> DstIterator>
	//static void RelocateNothrow(MemManager& memManager,
	//	SrcIterator srcBegin, DstIterator dstBegin, size_t count) noexcept

	static void RelocateNothrow(MemManager& /*memManager*/, Object* srcObjects, Object* dstObjects,
		size_t count) noexcept requires isTriviallyRelocatable
	{
		if (count > 0)
			std::memcpy(dstObjects, srcObjects, sizeof(Object) * count);
	}
};

namespace internal
{
	template<typename Creator, typename Object>
	concept conceptObjectCreator = conceptMoveFunctor<Creator, void, Object*>;

	template<typename Creator, typename Object>
	concept conceptObjectMultiCreator = conceptConstFunctor<Creator, void, Object*>;

	template<typename Remover, typename Object>
	concept conceptObjectRemover = conceptMoveFunctor<Remover, void, Object&>;

	template<typename Replacer, typename Object>
	concept conceptObjectReplacer = conceptMoveFunctor<Replacer, void, Object&, Object&>;

	template<typename Predicate, typename Object>
	concept conceptObjectPredicate = conceptPredicate<Predicate, const Object&>;

	template<typename Iterator, typename Object>
	concept conceptIncIterator =
		requires (Iterator iter)
		{
			{ std::to_address(iter++) } -> std::same_as<Object*>;
			{ *iter++ } -> std::same_as<Object&>;
		};

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

	template<conceptObject TObject, size_t tAlignment,
		size_t tCount = 1>
	requires (ObjectAlignmenter<TObject>::Check(tAlignment) && tCount > 0)
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t alignment = tAlignment;
		static const size_t count = tCount;

	public:
		explicit ObjectBuffer() noexcept = default;

		ObjectBuffer(const ObjectBuffer&) = delete;

		~ObjectBuffer() noexcept = default;

		ObjectBuffer& operator=(const ObjectBuffer&) = delete;

		template<bool isWithinLifetime = false>
		const Object* GetPtr() const noexcept
		{
			return PtrCaster::FromBytePtr<Object, isWithinLifetime, count == 1>(mBuffer);
		}

		template<bool isWithinLifetime = false>
		Object* GetPtr() noexcept
		{
			return PtrCaster::FromBytePtr<Object, isWithinLifetime, count == 1>(mBuffer);
		}

		const Object& Get() const noexcept
			requires (count == 1)
		{
			return *GetPtr<true>();
		}

		Object& Get() noexcept
			requires (count == 1)
		{
			return *GetPtr<true>();
		}

	private:
		alignas(alignment) std::byte mBuffer[sizeof(Object) * count];
	};

	template<conceptMemManager TMemManager, conceptObject TObject, typename... ObjectArgs>
	class ObjectCreatorBase
	{
	protected:
		typedef TMemManager MemManager;
		typedef TObject Object;

	protected:
		explicit ObjectCreatorBase(MemManager& /*memManager*/) noexcept
		{
		}

		void ptCreate(Object* newObject, ObjectArgs&&... objectArgs)
		{
			std::construct_at(newObject, std::forward<ObjectArgs>(objectArgs)...);
		}
	};

	template<conceptByteAllocator ByteAllocator, conceptObject TObject, typename... ObjectArgs>
	requires (HasCustomConstructor<MemManagerStdByte<ByteAllocator>, TObject, ObjectArgs...>::value)
	class ObjectCreatorBase<MemManagerStdByte<ByteAllocator>, TObject, ObjectArgs...>
		: private MemManagerPtr<MemManagerStdByte<ByteAllocator>>
	{
	protected:
		typedef MemManagerStdByte<ByteAllocator> MemManager;
		typedef TObject Object;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

	protected:
		explicit ObjectCreatorBase(MemManager& memManager) noexcept
			: MemManagerPtr(memManager)
		{
		}

		void ptCreate(Object* newObject, ObjectArgs&&... objectArgs)
		{
			MemManagerPtr::GetBaseMemManager().GetByteAllocator().construct(newObject,
				std::forward<ObjectArgs>(objectArgs)...);
		}
	};

	template<typename ObjectArg, size_t index>
	class ObjectCreatorArg
	{
	protected:
		explicit ObjectCreatorArg(ObjectArg&& objectArg) noexcept
			: mObjectArg(std::forward<ObjectArg>(objectArg))
		{
		}

		ObjectArg&& ptGet() noexcept
		{
			return std::forward<ObjectArg>(mObjectArg);
		}

	private:
		ObjectArg&& mObjectArg;
	};

	template<typename ObjectArg, size_t index>
	requires (!std::is_reference_v<ObjectArg> && conceptSmallAndTriviallyCopyable<ObjectArg>)
	class ObjectCreatorArg<ObjectArg, index>
	{
	protected:
		explicit ObjectCreatorArg(ObjectArg objectArg) noexcept
			: mObjectArg(objectArg)
		{
		}

		ObjectArg ptGet() noexcept
		{
			return mObjectArg;
		}

	private:
		MOMO_NO_UNIQUE_ADDRESS ObjectArg mObjectArg;
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
	class MOMO_EMPTY_BASES ObjectCreator;

	template<conceptObject TObject, conceptMemManager TMemManager,
		typename... ObjectArgs, size_t... indexes>
	requires (std::is_constructible_v<TObject, ObjectArgs...>
		|| HasCustomConstructor<TMemManager, TObject, ObjectArgs...>::value)
	class MOMO_EMPTY_BASES ObjectCreator<
		TObject, TMemManager, std::index_sequence<indexes...>, ObjectArgs...>
		: private ObjectCreatorBase<TMemManager, TObject, ObjectArgs...>,
		private ObjectCreatorArg<ObjectArgs, indexes>...
	{
	private:
		typedef ObjectCreatorBase<TMemManager, TObject, ObjectArgs...> CreatorBase;

	public:
		using typename CreatorBase::Object;
		using typename CreatorBase::MemManager;

	public:
		explicit ObjectCreator(MemManager& memManager, ObjectArgs&&... objectArgs) noexcept
			: CreatorBase(memManager),
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
			CreatorBase::ptCreate(newObject,
				std::forward<ObjectArgs>(ObjectCreatorArg<ObjectArgs, indexes>::ptGet())...);
		}
	};

	template<conceptObject TObject, conceptMemManager TMemManager>
	requires (conceptSmallAndTriviallyCopyable<TObject> &&
		!HasCustomConstructor<TMemManager, TObject, const TObject&>::value)
	class MOMO_EMPTY_BASES ObjectCreator<TObject, TMemManager, std::index_sequence<0>, const TObject&>
		: public ObjectCreator<TObject, TMemManager, std::index_sequence<0>, TObject>
	{
	private:
		typedef ObjectCreator<TObject, TMemManager, std::index_sequence<0>, TObject> Creator;

	public:
		using typename Creator::Object;
		using typename Creator::MemManager;

	public:
		explicit ObjectCreator(MemManager& memManager, const Object& objectArg) noexcept
			: Creator(memManager, objectArg)
		{
		}

		explicit ObjectCreator(MemManager& memManager, std::tuple<const Object&>&& objectArg) noexcept
			: Creator(memManager, std::get<0>(objectArg))
		{
		}
	};

	template<conceptObject TObject, conceptMemManager TMemManager,
		conceptMemManagerOrNullPtr<TMemManager> TMemManagerOrNullPtr>
	class ObjectDestroyFinalizer
	{
	public:
		typedef TObject Object;
		typedef TMemManager MemManager;
		typedef TMemManagerOrNullPtr MemManagerOrNullPtr;

	public:
		[[nodiscard]] explicit ObjectDestroyFinalizer(MemManagerOrNullPtr memManager,
			Object* objectPtr) noexcept
			: mMemManager(memManager),
			mObjectPtr(objectPtr)
		{
		}

		ObjectDestroyFinalizer(ObjectDestroyFinalizer&& fin) noexcept
			: mMemManager(fin.mMemManager),
			mObjectPtr(std::exchange(fin.mObjectPtr, nullptr))
		{
		}

		ObjectDestroyFinalizer(const ObjectDestroyFinalizer&) = delete;

		~ObjectDestroyFinalizer() noexcept
		{
			if (mObjectPtr != nullptr)
				ObjectDestroyer<Object, MemManager>::Destroy(mMemManager, *mObjectPtr);
		}

		ObjectDestroyFinalizer& operator=(const ObjectDestroyFinalizer&) = delete;

		MemManagerOrNullPtr GetMemManager() const noexcept
		{
			return mMemManager;
		}

		Object* GetPtr() const noexcept
		{
			return mObjectPtr;
		}

		void ResetPtr(Object* objectPtr = nullptr) noexcept
		{
			mObjectPtr = objectPtr;
		}

	private:
		MemManagerOrNullPtr mMemManager;
		Object* mObjectPtr;
	};

	template<conceptObject TObject, conceptMemManager TMemManager>
	class ObjectManager
	{
	public:
		typedef TObject Object;
		typedef TMemManager MemManager;

	private:
		typedef ObjectDestroyer<Object, MemManager> Destroyer;
		typedef ObjectRelocator<Object, MemManager> Relocator;

	public:
		template<typename... Args>
		using Creator = ObjectCreator<Object, MemManager, std::index_sequence_for<Args...>, Args...>;

		template<conceptMemManagerOrNullPtr<TMemManager> MemManagerOrNullPtr = MemManager*>
		using DestroyFinalizer = ObjectDestroyFinalizer<Object, MemManager, MemManagerOrNullPtr>;

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
		static void Copy(MemManager* memManager, const Object& srcObject, Object* dstObject)
			requires isCopyConstructible
		{
			MOMO_ASSERT(memManager != nullptr);
			Creator<const Object&>(*memManager, srcObject)(dstObject);
		}

		static void Copy(std::nullptr_t /*memManager*/, const Object& srcObject, Object* dstObject)
			requires std::is_copy_constructible_v<Object>	//?
		{
			std::construct_at(dstObject, srcObject);
		}

		template<conceptExecutor Executor>
		static void MoveExec(MemManager& memManager, Object&& srcObject, Object* dstObject,
			FastMovableFunctor<Executor> exec) requires isMoveConstructible && isNothrowDestructible
		{
			if constexpr (isNothrowMoveConstructible)
			{
				std::move(exec)();
				Creator<Object&&>(memManager, std::move(srcObject))(dstObject);
			}
			else
			{
				Creator<Object&&>(memManager, std::move(srcObject))(dstObject);
				DestroyFinalizer<> dstFin(&memManager, dstObject);
				std::move(exec)();	//?
				dstFin.ResetPtr();
			}
		}

		template<conceptExecutor Executor>
		static void CopyExec(MemManager& memManager, const Object& srcObject, Object* dstObject,
			FastMovableFunctor<Executor> exec) requires isCopyConstructible && isNothrowDestructible
		{
			Copy(&memManager, srcObject, dstObject);
			DestroyFinalizer<> dstFin(&memManager, dstObject);
			std::move(exec)();
			dstFin.ResetPtr();
		}

		template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
		static void Destroy(MemManagerOrNullPtr memManager, Object& object) noexcept
			requires isNothrowDestructible
		{
			Destroyer::Destroy(memManager, object);
		}

		template<conceptIncIterator<Object> Iterator>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
			requires isNothrowDestructible
		{
			Iterator iter = begin;
			for (size_t i = 0; i < count; ++i)
				Destroy(&memManager, *iter++);
		}

		template<internal::conceptMemManagerOrNullPtr<MemManager> SrcMemManagerOrNullPtr,
			internal::conceptMemManagerOrNullPtr<MemManager> DstMemManagerOrNullPtr>
		static void Relocate(SrcMemManagerOrNullPtr srcMemManager, DstMemManagerOrNullPtr dstMemManager,
			Object& srcObject, Object* dstObject) noexcept(isNothrowRelocatable) requires isRelocatable
		{
			Relocator::Relocate(srcMemManager, dstMemManager, srcObject, dstObject);
		}

		static void Relocate(MemManager& memManager, Object& srcObject, Object* dstObject)
			noexcept(isNothrowRelocatable) requires isRelocatable
		{
			Relocate(&memManager, &memManager, srcObject, dstObject);
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
					Relocate(memManager, dstObject, objectBuffer.GetPtr());
					Relocate(memManager, srcObject, std::addressof(dstObject));
					Relocate(memManager, objectBuffer.Get(), std::addressof(srcObject));
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
			Destroy(&memManager, srcObject);
		}

		static void ReplaceRelocate(MemManager& memManager, Object& srcObject, Object& midObject,
			Object* dstObject) noexcept(isNothrowRelocatable)
			requires isNothrowRelocatable ||
				(isNothrowAnywayAssignable && isNothrowDestructible && std::is_move_constructible_v<Object>) ||
				(isAnywayAssignable && isNothrowDestructible && std::is_copy_constructible_v<Object>)
		{
			MOMO_ASSERT(std::addressof(srcObject) != std::addressof(midObject));
			if constexpr (isNothrowRelocatable)
			{
				Relocate(&memManager, nullptr, midObject, dstObject);
				Relocate(memManager, srcObject, std::addressof(midObject));
			}
			else if constexpr (isNothrowAnywayAssignable)
			{
				std::construct_at(dstObject, std::move(midObject));
				Replace(memManager, srcObject, midObject);
			}
			else
			{
				Copy(nullptr, midObject, dstObject);
				DestroyFinalizer<std::nullptr_t> dstFin(nullptr, dstObject);
				Replace(memManager, srcObject, midObject);
				dstFin.ResetPtr();
			}
		}

		template<conceptIncIterator<Object> SrcIterator, conceptIncIterator<Object> DstIterator>
		static void Relocate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count) noexcept(isNothrowRelocatable)
			requires isNothrowRelocatable ||
				(isCopyConstructible && isMoveConstructible && isNothrowDestructible)
		{
			if constexpr (requires { {
				Relocator::RelocateNothrow(memManager, srcBegin, dstBegin, count) } noexcept; })
			{
				Relocator::RelocateNothrow(memManager, srcBegin, dstBegin, count);
			}
			else
			{
				SrcIterator srcIter = srcBegin;
				DstIterator dstIter = dstBegin;
				if constexpr (isNothrowRelocatable)
				{
					for (size_t i = 0; i < count; ++i)
						Relocate(memManager, *srcIter++, std::to_address(dstIter++));
				}
				else
				{
					if (count > 0)
					{
						Object& srcObject0 = *srcIter++;
						Object& dstObject0 = *dstIter++;
						RelocateCreate(memManager, srcIter, dstIter, count - 1,
							FastMovableFunctor(Creator<Object&&>(memManager, std::move(srcObject0))),
							std::addressof(dstObject0));
						Destroy(&memManager, srcObject0);
					}
				}
			}
		}

		template<conceptIncIterator<Object> SrcIterator, conceptIncIterator<Object> DstIterator,
			conceptObjectCreator<Object> ObjectCreator>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, FastMovableFunctor<ObjectCreator> objectCreator, Object* newObject)
			requires isNothrowRelocatable || (isCopyConstructible && isNothrowDestructible)
		{
			auto exec = [objectCreator = std::move(objectCreator), newObject] () mutable
				{ std::move(objectCreator)(newObject); };
			RelocateExec(memManager, srcBegin, dstBegin, count, FastMovableFunctor(std::move(exec)));
		}

		template<conceptIncIterator<Object> SrcIterator, conceptIncIterator<Object> DstIterator,
			conceptExecutor Executor>
		static void RelocateExec(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, FastMovableFunctor<Executor> exec)
			requires isNothrowRelocatable || (isCopyConstructible && isNothrowDestructible)
		{
			if constexpr (isNothrowRelocatable)
			{
				std::move(exec)();
				Relocate(memManager, srcBegin, dstBegin, count);
			}
			else
			{
				size_t index = 0;
				for (Finalizer fin = [&memManager, dstBegin, &index] () noexcept
						{ Destroy(memManager, dstBegin, index); };
					fin; fin.Detach())
				{
					SrcIterator srcIter = srcBegin;
					DstIterator dstIter = dstBegin;
					for (; index < count; ++index)
						Copy(&memManager, *srcIter++, std::to_address(dstIter++));
					std::move(exec)();
				}
				Destroy(memManager, srcBegin, count);
			}
		}

		template<conceptIncIterator<Object> Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
			requires isNothrowShiftable
		{
			if (shift > 0)
				pvShiftNothrow(memManager, begin, shift);
		}

	private:
		template<conceptIncIterator<Object> Iterator>
		static void pvShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			Iterator iter = begin;
			if constexpr (isNothrowRelocatable)
			{
				ObjectBuffer<Object, alignment> objectBuffer;
				Object* objectPtr = std::to_address(iter++);
				Relocate(memManager, *objectPtr, objectBuffer.GetPtr());
				for (size_t i = 0; i < shift; ++i)
				{
					Object* nextObjectPtr = std::to_address(iter++);
					Relocate(memManager, *nextObjectPtr, objectPtr);
					objectPtr = nextObjectPtr;
				}
				Relocate(memManager, objectBuffer.Get(), objectPtr);
			}
			else
			{
				Object* objectPtr = std::to_address(iter++);
				for (size_t i = 0; i < shift; ++i)
				{
					Object* nextObjectPtr = std::to_address(iter++);
					std::iter_swap(objectPtr, nextObjectPtr);
					objectPtr = nextObjectPtr;
				}
			}
		}
	};
}

} // namespace momo
