/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/MemManager.h

  namespace momo:
    class MemManagerCpp
    class MemManagerC
    class MemManagerWin
    class MemManagerStd
    class MemManagerDefault

\**********************************************************/

/*!
	\file
	\code{.cpp}
	// template for user MemManager:
	class UserMemManager
	{
	public:
		explicit UserMemManager(...);
		UserMemManager(UserMemManager&& memManager) noexcept;
		UserMemManager(const UserMemManager& memManager);
		~UserMemManager() noexcept;
		UserMemManager& operator=(const UserMemManager&) = delete;

		void* Allocate(size_t size);

		void Deallocate(void* ptr, size_t size) noexcept;

		void* Reallocate(void* ptr, size_t size, size_t newSize); // optional

		void* ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept; // optional

		bool IsEqual(const UserMemManager& memManager) const noexcept; // optional
	};
	\endcode
*/

#ifndef MOMO_INCLUDE_GUARD_MEM_MANAGER
#define MOMO_INCLUDE_GUARD_MEM_MANAGER

#include "Utility.h"

#ifdef MOMO_USE_MEM_MANAGER_WIN
# include <Windows.h>
#endif

namespace momo
{

//! `MemManagerCpp` uses `new` and `delete`
class MemManagerCpp
{
public:
	explicit MemManagerCpp() noexcept
	{
	}

	MemManagerCpp(MemManagerCpp&&) = default;

	MemManagerCpp(const MemManagerCpp&) = default;

	~MemManagerCpp() = default;

	MemManagerCpp& operator=(const MemManagerCpp&) = delete;

	MOMO_NODISCARD void* Allocate(size_t size)
	{
		return operator new(size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
#ifdef __cpp_sized_deallocation	//?
		operator delete(ptr, size);
#else
		(void)size;
		operator delete(ptr);
#endif
	}
};

//! `MemManagerC` uses `malloc`, `free` and `realloc`
class MemManagerC
{
public:
	explicit MemManagerC() noexcept
	{
	}

	MemManagerC(MemManagerC&&) = default;

	MemManagerC(const MemManagerC&) = default;

	~MemManagerC() = default;

	MemManagerC& operator=(const MemManagerC&) = delete;

	MOMO_NODISCARD void* Allocate(size_t size)
	{
		void* ptr = std::malloc(size);
		if (ptr == nullptr)
			MOMO_THROW(std::bad_alloc());
		return ptr;
	}

	void Deallocate(void* ptr, size_t /*size*/) noexcept
	{
		std::free(ptr);
	}

	MOMO_NODISCARD void* Reallocate(void* ptr, size_t /*size*/, size_t newSize)
	{
		void* newPtr = std::realloc(ptr, newSize);
		if (newPtr == nullptr)
			MOMO_THROW(std::bad_alloc());
		return newPtr;
	}
};

#ifdef MOMO_USE_MEM_MANAGER_WIN
//! `MemManagerWin` uses `HeapAlloc`, `HeapFree` and `HeapReAlloc`
class MemManagerWin
{
public:
	explicit MemManagerWin() noexcept
	{
	}

	MemManagerWin(MemManagerWin&&) = default;

	MemManagerWin(const MemManagerWin&) = default;

	~MemManagerWin() = default;

	MemManagerWin& operator=(const MemManagerWin&) = delete;

	MOMO_NODISCARD void* Allocate(size_t size)
	{
		void* ptr = HeapAlloc(GetProcessHeap(), 0, size);
		if (ptr == nullptr)
			MOMO_THROW(std::bad_alloc());
		return ptr;
	}

	void Deallocate(void* ptr, size_t /*size*/) noexcept
	{
		HeapFree(GetProcessHeap(), 0, ptr);
	}

	MOMO_NODISCARD void* Reallocate(void* ptr, size_t /*size*/, size_t newSize)
	{
		void* newPtr = HeapReAlloc(GetProcessHeap(), 0, ptr, newSize);
		if (newPtr == nullptr)
			MOMO_THROW(std::bad_alloc());
		return newPtr;
	}

	MOMO_NODISCARD void* ReallocateInplace(void* ptr, size_t /*size*/, size_t newSize) noexcept
	{
		return HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY, ptr, newSize);
	}
};
#endif // MOMO_USE_MEM_MANAGER_WIN

//! `MemManagerStd` uses `allocator<unsigned char>::allocate` and `deallocate`
template<typename TAllocator>
class MOMO_EMPTY_BASES MemManagerStd
	: private std::allocator_traits<TAllocator>::template rebind_alloc<internal::Byte>
{
public:
	typedef TAllocator Allocator;
	typedef typename std::allocator_traits<Allocator>
		::template rebind_alloc<internal::Byte> ByteAllocator;

private:
	typedef std::allocator_traits<ByteAllocator> ByteAllocatorTraits;

public:
	explicit MemManagerStd() noexcept(std::is_nothrow_default_constructible<ByteAllocator>::value)
	{
	}

	explicit MemManagerStd(const Allocator& alloc) noexcept
		: ByteAllocator(alloc)
	{
	}

	MemManagerStd(MemManagerStd&& memManager) noexcept
		: ByteAllocator(std::move(memManager.GetByteAllocator()))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: ByteAllocator(ByteAllocatorTraits::select_on_container_copy_construction(
			memManager.GetByteAllocator()))
	{
	}

	~MemManagerStd() = default;

	template<bool enabled = std::is_nothrow_move_assignable<ByteAllocator>::value
		|| ByteAllocatorTraits::propagate_on_container_move_assignment::value
		|| ByteAllocatorTraits::propagate_on_container_copy_assignment::value
		|| ByteAllocatorTraits::propagate_on_container_swap::value>
	internal::EnableIf<enabled, MemManagerStd&> operator=(MemManagerStd&& memManager) noexcept
	{
		static const bool isNothrowMoveAssignable =
			std::is_nothrow_move_assignable<ByteAllocator>::value
			|| ByteAllocatorTraits::propagate_on_container_move_assignment::value;
		pvAssign(memManager.GetByteAllocator(), GetByteAllocator(),
			internal::BoolConstant<isNothrowMoveAssignable>(),
			typename ByteAllocatorTraits::propagate_on_container_copy_assignment(),
			typename ByteAllocatorTraits::propagate_on_container_swap());
		return *this;
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	MOMO_NODISCARD void* Allocate(size_t size)
	{
		return ByteAllocatorTraits::allocate(GetByteAllocator(), size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		ByteAllocatorTraits::deallocate(GetByteAllocator(),
			static_cast<internal::Byte*>(ptr), size);
	}

	bool IsEqual(const MemManagerStd& memManager) const noexcept
	{
		return GetByteAllocator() == memManager.GetByteAllocator();
	}

	const ByteAllocator& GetByteAllocator() const noexcept
	{
		return *this;
	}

	ByteAllocator& GetByteAllocator() noexcept
	{
		return *this;
	}

private:
	template<bool isNothrowCopyAssignable, bool isNothrowSwappable>
	static void pvAssign(ByteAllocator& srcAlloc, ByteAllocator& dstAlloc,
		std::true_type /*isNothrowMoveAssignable*/, internal::BoolConstant<isNothrowCopyAssignable>,
		internal::BoolConstant<isNothrowSwappable>) noexcept
	{
		dstAlloc = std::move(srcAlloc);
	}

	template<bool isNothrowSwappable>
	static void pvAssign(ByteAllocator& srcAlloc, ByteAllocator& dstAlloc,
		std::false_type /*isNothrowMoveAssignable*/, std::true_type /*isNothrowCopyAssignable*/,
		internal::BoolConstant<isNothrowSwappable>) noexcept
	{
		dstAlloc = static_cast<const ByteAllocator&>(srcAlloc);
	}

	static void pvAssign(ByteAllocator& srcAlloc, ByteAllocator& dstAlloc,
		std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isNothrowCopyAssignable*/,
		std::true_type /*isNothrowSwappable*/) noexcept
	{
		std::iter_swap(&dstAlloc, &srcAlloc);
	}
};

//! `MemManagerDefault` is defined in UserSettings.h
typedef MOMO_DEFAULT_MEM_MANAGER MemManagerDefault;

#ifdef MOMO_USE_DEFAULT_MEM_MANAGER_IN_STD
//! `MemManagerStd<std::allocator<...>>` is same as `MemManagerDefault`
template<typename Object>
class MOMO_EMPTY_BASES MemManagerStd<std::allocator<Object>>
	: private std::allocator<internal::Byte>,
	public MemManagerDefault
{
public:
	typedef std::allocator<Object> Allocator;
	typedef std::allocator<internal::Byte> ByteAllocator;

public:
	explicit MemManagerStd(const Allocator& /*alloc*/ = Allocator())
		noexcept(noexcept(MemManagerDefault()))
	{
	}

	MemManagerStd(MemManagerStd&& memManager) noexcept
		: ByteAllocator(),
		MemManagerDefault(std::move(memManager))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: ByteAllocator(),
		MemManagerDefault(memManager)
	{
	}

	~MemManagerStd() = default;

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	const ByteAllocator& GetByteAllocator() const noexcept
	{
		return *this;
	}

	ByteAllocator& GetByteAllocator() noexcept
	{
		return *this;
	}
};
#endif // MOMO_USE_DEFAULT_MEM_MANAGER_IN_STD

namespace internal
{
	template<typename TMemManager>
	class MemManagerProxy
	{
	public:
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	private:
		template<typename MemManager,
			typename = void*>
		struct CanReallocate : public std::false_type
		{
		};

		template<typename MemManager>
		struct CanReallocate<MemManager,
			decltype(std::declval<MemManager&>().Reallocate(nullptr, size_t{}, size_t{}))>
			: public std::true_type
		{
		};

		template<typename MemManager,
			typename = void*>
		struct CanReallocateInplace : public std::false_type
		{
		};

		template<typename MemManager>
		struct CanReallocateInplace<MemManager,
			decltype(std::declval<MemManager&>().ReallocateInplace(nullptr, size_t{}, size_t{}))>
			: public std::true_type
		{
		};

		template<typename MemManager,
			typename = size_t>
		struct PtrUsefulBitCount
		{
#ifdef MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT
			static const size_t value = MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT;
#else
			static const size_t value = sizeof(void*) * 8;
#endif
		};

		template<typename MemManager>
		struct PtrUsefulBitCount<MemManager, decltype(MemManager::ptrUsefulBitCount)>
		{
			static const size_t value = MemManager::ptrUsefulBitCount;
		};

		template<typename MemManager,
			typename = bool>
		struct HasIsEqual : public std::false_type
		{
		};

		template<typename MemManager>
		struct HasIsEqual<MemManager,
			decltype(std::declval<const MemManager&>().IsEqual(std::declval<const MemManager&>()))>
			: public std::true_type
		{
		};

	public:
		static const bool canReallocate = CanReallocate<MemManager>::value;
		static const bool canReallocateInplace = CanReallocateInplace<MemManager>::value;

		static const size_t ptrUsefulBitCount = PtrUsefulBitCount<MemManager>::value;

	public:
		template<typename ResObject = void>
		static ResObject* Allocate(MemManager& memManager, size_t size)
		{
			MOMO_ASSERT(size > 0);
			void* ptr = memManager.Allocate(size);
			MOMO_ASSERT(ptr != nullptr);
			pvCheckBits(ptr);
			return PtrCaster::FromBytePtr<ResObject>(ptr);
		}

		template<typename ResObject, typename... ResObjectArgs>
		static ResObject* AllocateCreate(MemManager& memManager, ResObjectArgs&&... resObjectArgs)
		{
			void* resObjectPtr = Allocate(memManager, sizeof(ResObject));
			try
			{
				return ::new(resObjectPtr) ResObject(std::forward<ResObjectArgs>(resObjectArgs)...);
			}
			catch (...)
			{
				memManager.Deallocate(resObjectPtr, sizeof(ResObject));
				throw;
			}
		}

		template<typename Object>
		static void Deallocate(MemManager& memManager, Object* ptr, size_t size) noexcept
		{
			MOMO_ASSERT(ptr != nullptr && size > 0);
			memManager.Deallocate(PtrCaster::ToBytePtr(ptr), size);
		}

		template<typename Object>
		static Object* Reallocate(MemManager& memManager, Object* ptr, size_t size, size_t newSize)
		{
			MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
			if (size == newSize)
				return ptr;
			void* newPtr = memManager.Reallocate(PtrCaster::ToBytePtr(ptr), size, newSize);
			MOMO_ASSERT(newPtr != nullptr);
			pvCheckBits(newPtr);
			return PtrCaster::FromBytePtr<Object>(newPtr);
		}

		template<typename Object>
		static Object* ReallocateInplace(MemManager& memManager, Object* ptr, size_t size,
			size_t newSize) noexcept
		{
			MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
			if (size == newSize)
				return ptr;
			void* newPtr = memManager.ReallocateInplace(PtrCaster::ToBytePtr(ptr), size, newSize);
			if (newPtr == nullptr)
				return nullptr;
			MOMO_ASSERT(newPtr == ptr);
			return PtrCaster::FromBytePtr<Object>(newPtr);
		}

		static bool IsEqual(const MemManager& memManager1, const MemManager& memManager2) noexcept
		{
			if (&memManager1 == &memManager2 || std::is_empty<MemManager>::value)
				return true;
			return pvIsEqual(memManager1, memManager2, HasIsEqual<MemManager>());
		}

		static void Swap(MemManager& memManager1, MemManager& memManager2) noexcept
		{
			MOMO_ASSERT(&memManager1 != &memManager2);
			if (!std::is_empty<MemManager>::value)
			{
				MemManager tempMemManager(std::move(memManager1));
				Assign(std::move(memManager2), memManager1);
				Assign(std::move(tempMemManager), memManager2);
			}
		}

		static void Assign(MemManager&& srcMemManager, MemManager& dstMemManager) noexcept
		{
			MOMO_ASSERT(&srcMemManager != &dstMemManager);
			if (!std::is_empty<MemManager>::value)
			{
				pvAssign(std::move(srcMemManager), dstMemManager,
					std::is_nothrow_move_assignable<MemManager>());
			}
		}

	private:
		template<size_t shift = ptrUsefulBitCount>
		static EnableIf<(shift < sizeof(void*) * 8)>
		pvCheckBits(void* ptr) noexcept
		{
			(void)ptr;
			MOMO_ASSERT(PtrCaster::ToUInt(ptr) >> shift == uintptr_t{0});
		}

		template<size_t shift = ptrUsefulBitCount>
		static EnableIf<(shift == sizeof(void*) * 8)>
		pvCheckBits(void* /*ptr*/) noexcept
		{
		}

		static bool pvIsEqual(const MemManager& memManager1, const MemManager& memManager2,
			std::true_type /*hasIsEqual*/) noexcept
		{
			return memManager1.IsEqual(memManager2);
		}

		static bool pvIsEqual(const MemManager& /*memManager1*/, const MemManager& /*memManager2*/,
			std::false_type /*hasIsEqual*/) noexcept
		{
			return false;
		}

		static void pvAssign(MemManager&& srcMemManager, MemManager& dstMemManager,
			std::true_type /*isNothrowMoveAssignable*/) noexcept
		{
			dstMemManager = std::move(srcMemManager);
		}

		static void pvAssign(MemManager&& srcMemManager, MemManager& dstMemManager,
			std::false_type /*isNothrowMoveAssignable*/) noexcept
		{
			dstMemManager.~MemManager();
			::new(static_cast<void*>(&dstMemManager)) MemManager(std::move(srcMemManager));
		}
	};

	class MemManagerDummy
	{
	public:
		explicit MemManagerDummy() noexcept
		{
		}

		MemManagerDummy(MemManagerDummy&&) = default;

		MemManagerDummy(const MemManagerDummy&) = default;

		~MemManagerDummy() = default;

		MemManagerDummy& operator=(const MemManagerDummy&) = delete;

		void* Allocate(size_t /*size*/)
		{
			MOMO_ASSERT(false);
			MOMO_THROW(std::bad_alloc());
		}

		void Deallocate(void* /*ptr*/, size_t /*size*/) noexcept
		{
			MOMO_ASSERT(false);
		}
	};

	template<typename TBaseMemManager,
		bool tIsEmpty = std::is_empty<TBaseMemManager>::value
			&& std::is_nothrow_default_constructible<TBaseMemManager>::value>
	class MemManagerPtr;

	template<typename TBaseMemManager>
	class MemManagerPtr<TBaseMemManager, true> : private TBaseMemManager
	{
	public:
		typedef TBaseMemManager BaseMemManager;

	private:
		typedef MemManagerProxy<BaseMemManager> BaseMemManagerProxy;

	public:
		static const size_t ptrUsefulBitCount = BaseMemManagerProxy::ptrUsefulBitCount;

	public:
		explicit MemManagerPtr(BaseMemManager& /*baseMemManager*/) noexcept
		{
		}

		MemManagerPtr(MemManagerPtr&& /*memManager*/) noexcept
			: BaseMemManager()
		{
		}

		MemManagerPtr(const MemManagerPtr& /*memManager*/) noexcept
			: BaseMemManager()
		{
		}

		~MemManagerPtr() = default;

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		BaseMemManager& GetBaseMemManager() noexcept
		{
			return *this;
		}

		void* Allocate(size_t size)
		{
			return GetBaseMemManager().Allocate(size);
		}

		void Deallocate(void* ptr, size_t size) noexcept
		{
			GetBaseMemManager().Deallocate(ptr, size);
		}

		typename std::conditional<BaseMemManagerProxy::canReallocate, void*, void>::type
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return GetBaseMemManager().Reallocate(ptr, size, newSize);
		}

		typename std::conditional<BaseMemManagerProxy::canReallocateInplace, void*, void>::type
		ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept
		{
			return GetBaseMemManager().ReallocateInplace(ptr, size, newSize);
		}

		bool IsEqual(const MemManagerPtr& /*memManager*/) const noexcept
		{
			return true;
		}
	};

	template<typename TBaseMemManager>
	class MemManagerPtr<TBaseMemManager, false>
	{
	public:
		typedef TBaseMemManager BaseMemManager;

	private:
		typedef MemManagerProxy<BaseMemManager> BaseMemManagerProxy;

	public:
		static const size_t ptrUsefulBitCount = BaseMemManagerProxy::ptrUsefulBitCount;

	public:
		explicit MemManagerPtr(BaseMemManager& baseMemManager) noexcept
			: mBaseMemManager(&baseMemManager)
		{
		}

		MemManagerPtr(MemManagerPtr&&) = default;

		MemManagerPtr(const MemManagerPtr&) = default;

		~MemManagerPtr() = default;

		MemManagerPtr& operator=(MemManagerPtr&&) = default;

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		BaseMemManager& GetBaseMemManager() noexcept
		{
			return *mBaseMemManager;
		}

		void* Allocate(size_t size)
		{
			return mBaseMemManager->Allocate(size);
		}

		void Deallocate(void* ptr, size_t size) noexcept
		{
			mBaseMemManager->Deallocate(ptr, size);
		}

		typename std::conditional<BaseMemManagerProxy::canReallocate, void*, void>::type
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return mBaseMemManager->Reallocate(ptr, size, newSize);
		}

		typename std::conditional<BaseMemManagerProxy::canReallocateInplace, void*, void>::type
		ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept
		{
			return mBaseMemManager->ReallocateInplace(ptr, size, newSize);
		}

		bool IsEqual(const MemManagerPtr& memManager) const noexcept
		{
			return BaseMemManagerProxy::IsEqual(*mBaseMemManager, *memManager.mBaseMemManager);
		}

	private:
		BaseMemManager* mBaseMemManager;
	};
}

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_MEM_MANAGER
