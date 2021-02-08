/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
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

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept; // optional

		bool IsEqual(const UserMemManager& memManager) const noexcept; // optional
	};
	\endcode
*/

#pragma once

#include "Utility.h"

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

	[[nodiscard]] void* Allocate(size_t size)
	{
		return operator new(size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		operator delete(ptr, size);
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

	[[nodiscard]] void* Allocate(size_t size)
	{
		void* ptr = std::malloc(size);
		if (ptr == nullptr)
			throw std::bad_alloc();
		return ptr;
	}

	void Deallocate(void* ptr, size_t /*size*/) noexcept
	{
		std::free(ptr);
	}

	[[nodiscard]] void* Reallocate(void* ptr, size_t /*size*/, size_t newSize)
	{
		void* newPtr = std::realloc(ptr, newSize);
		if (newPtr == nullptr)
			throw std::bad_alloc();
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

	[[nodiscard]] void* Allocate(size_t size)
	{
		void* ptr = HeapAlloc(GetProcessHeap(), 0, size);
		if (ptr == nullptr)
			throw std::bad_alloc();
		return ptr;
	}

	void Deallocate(void* ptr, size_t /*size*/) noexcept
	{
		HeapFree(GetProcessHeap(), 0, ptr);
	}

	//[[nodiscard]] void* Reallocate(void* ptr, size_t /*size*/, size_t newSize)
	//{
	//	void* newPtr = HeapReAlloc(GetProcessHeap(), 0, ptr, newSize);
	//	if (newPtr == nullptr)
	//		throw std::bad_alloc();
	//	return newPtr;
	//}

	bool ReallocateInplace(void* ptr, size_t /*size*/, size_t newSize) noexcept
	{
		void* newPtr = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY,
			ptr, newSize);
		MOMO_ASSERT(newPtr == ptr || newPtr == nullptr);
		return newPtr == ptr;
	}
};
#endif

//! `MemManagerStd` uses `allocator<std::byte>::allocate` and `deallocate`
template<typename TAllocator,
	bool tUseMemManagerDefault = true>
class MemManagerStd : private std::allocator_traits<TAllocator>::template rebind_alloc<std::byte>
{
public:
	typedef TAllocator Allocator;
	typedef typename std::allocator_traits<Allocator>::template rebind_alloc<std::byte> ByteAllocator;

	//static_assert(std::is_nothrow_move_constructible<ByteAllocator>::value);

public:
	explicit MemManagerStd() noexcept(noexcept(ByteAllocator()))
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
		: ByteAllocator(std::allocator_traits<ByteAllocator>
			::select_on_container_copy_construction(memManager.GetByteAllocator()))
	{
	}

	~MemManagerStd() = default;

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	[[nodiscard]] void* Allocate(size_t size)
	{
		return std::allocator_traits<ByteAllocator>::allocate(GetByteAllocator(), size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		std::allocator_traits<ByteAllocator>::deallocate(GetByteAllocator(),
			static_cast<std::byte*>(ptr), size);
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
};

//! `MemManagerDefault` is defined in UserSettings.h
typedef MOMO_DEFAULT_MEM_MANAGER MemManagerDefault;

//! `MemManagerStd<std::allocator<...>>` is same as `MemManagerDefault`
template<typename Item>
class MemManagerStd<std::allocator<Item>, true>
	: private std::allocator<std::byte>, public MemManagerDefault
{
public:
	typedef std::allocator<Item> Allocator;
	typedef std::allocator<std::byte> ByteAllocator;

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

namespace internal
{
	template<typename TMemManager>
	class MemManagerProxy
	{
	public:
		typedef TMemManager MemManager;

		static_assert(std::is_nothrow_move_constructible<MemManager>::value);

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
			typename = bool>
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
			return static_cast<ResObject*>(ptr);
		}

		static void Deallocate(MemManager& memManager, void* ptr, size_t size) noexcept
		{
			MOMO_ASSERT(ptr != nullptr && size > 0);
			memManager.Deallocate(ptr, size);
		}

		template<typename ResObject = void>
		static ResObject* Reallocate(MemManager& memManager, void* ptr, size_t size, size_t newSize)
		{
			MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
			if (size == newSize)
				return static_cast<ResObject*>(ptr);
			void* newPtr = memManager.Reallocate(ptr, size, newSize);
			MOMO_ASSERT(newPtr != nullptr);
			pvCheckBits(newPtr);
			return static_cast<ResObject*>(newPtr);
		}

		static bool ReallocateInplace(MemManager& memManager, void* ptr, size_t size,
			size_t newSize) noexcept
		{
			MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
			if (size == newSize)
				return true;
			return memManager.ReallocateInplace(ptr, size, newSize);
		}

		static bool IsEqual(const MemManager& memManager1, const MemManager& memManager2) noexcept
		{
			if constexpr (HasIsEqual<MemManager>::value)
				return memManager1.IsEqual(memManager2);
			else
				return &memManager1 == &memManager2 || std::is_empty<MemManager>::value;
		}

	private:
		static void pvCheckBits(void* ptr) noexcept
		{
			(void)ptr;
			if constexpr (ptrUsefulBitCount < sizeof(void*) * 8)
				MOMO_ASSERT(PtrCaster::ToUInt(ptr) >> ptrUsefulBitCount == uintptr_t{0});
		}
	};

	template<typename TMemManager,
		bool tIsEmpty = std::is_empty<TMemManager>::value>
	class MemManagerWrapper;

	template<typename TMemManager>
	class MemManagerWrapper<TMemManager, false>
	{
	public:
		typedef TMemManager MemManager;

		static_assert(std::is_nothrow_move_constructible<MemManager>::value);

	public:
		explicit MemManagerWrapper(MemManager&& memManager) noexcept
			: mMemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&& memManagerWrapper) noexcept
			: mMemManager(std::move(memManagerWrapper.mMemManager))
		{
		}

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() = default;

		MemManagerWrapper& operator=(MemManagerWrapper&& memManagerWrapper) noexcept
		{
			if (this != &memManagerWrapper)
			{
				mMemManager.~MemManager();
				new(static_cast<void*>(&mMemManager))
					MemManager(std::move(memManagerWrapper.mMemManager));
			}
			return *this;
		}

		MemManagerWrapper& operator=(const MemManagerWrapper&) = delete;

		const MemManager& GetMemManager() const noexcept
		{
			return mMemManager;
		}

		MemManager& GetMemManager() noexcept
		{
			return mMemManager;
		}

	private:
		MemManager mMemManager;
	};

	template<typename TMemManager>
	class MemManagerWrapper<TMemManager, true> : private TMemManager
	{
	public:
		typedef TMemManager MemManager;

		static_assert(std::is_nothrow_move_constructible<MemManager>::value);

	public:
		explicit MemManagerWrapper(MemManager&& memManager) noexcept
			: MemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&& memManagerWrapper) noexcept
			: MemManager(std::move(memManagerWrapper.GetMemManager()))
		{
		}

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() = default;

		MemManagerWrapper& operator=(MemManagerWrapper&& /*memManagerWrapper*/) noexcept
		{
			return *this;
		}

		MemManagerWrapper& operator=(const MemManagerWrapper&) = delete;

		const MemManager& GetMemManager() const noexcept
		{
			return *this;
		}

		MemManager& GetMemManager() noexcept
		{
			return *this;
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
			throw std::bad_alloc();
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
		//explicit MemManagerPtr() noexcept
		//{
		//}

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

		std::conditional_t<BaseMemManagerProxy::canReallocate, void*, void>
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return GetBaseMemManager().Reallocate(ptr, size, newSize);
		}

		std::conditional_t<BaseMemManagerProxy::canReallocateInplace, bool, void>
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
			: mBaseMemManager(baseMemManager)
		{
		}

		MemManagerPtr(MemManagerPtr&& memManager) noexcept
			: mBaseMemManager(memManager.mBaseMemManager)
		{
		}

		MemManagerPtr(const MemManagerPtr& memManager) noexcept
			: mBaseMemManager(memManager.mBaseMemManager)
		{
		}

		~MemManagerPtr() = default;

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		BaseMemManager& GetBaseMemManager() noexcept
		{
			return mBaseMemManager;
		}

		void* Allocate(size_t size)
		{
			return mBaseMemManager.Allocate(size);
		}

		void Deallocate(void* ptr, size_t size) noexcept
		{
			mBaseMemManager.Deallocate(ptr, size);
		}

		std::conditional_t<BaseMemManagerProxy::canReallocate, void*, void>
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return mBaseMemManager.Reallocate(ptr, size, newSize);
		}

		std::conditional_t<BaseMemManagerProxy::canReallocateInplace, bool, void>
		ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept
		{
			return mBaseMemManager.ReallocateInplace(ptr, size, newSize);
		}

		bool IsEqual(const MemManagerPtr& memManager) const noexcept
		{
			return BaseMemManagerProxy::IsEqual(mBaseMemManager, memManager.mBaseMemManager);
		}

	private:
		BaseMemManager& mBaseMemManager;
	};
}

} // namespace momo
