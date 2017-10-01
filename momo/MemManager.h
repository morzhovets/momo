/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/MemManager.h

  namespace momo:
    class MemManagerCpp
    class MemManagerC
    class MemManagerWin
    class MemManagerStd
    class MemManagerDefault

  MemManagerCpp uses `new` and `delete`.
  MemManagerC uses `malloc`, `free` and `realloc`.
  MemManagerWin uses `HeapAlloc`, `HeapFree` and `HeapReAlloc`.
  MemManagerStd uses `allocator<char>::allocate` and `deallocate`.
  MemManagerDefault is defined in UserSettings.h.
  MemManagerStd<std::allocator<...>> is same as MemManagerDefault.

  // template for user MemManager:
  class UserMemManager
  {
  public:
    static const bool canReallocate = true;
    static const bool canReallocateInplace = true;

    static const size_t ptrUsefulBitCount = sizeof(void*) * 8;

  public:
    UserMemManager();
    UserMemManager(UserMemManager&& memManager) noexcept;
    UserMemManager(const UserMemManager& memManager);
    ~UserMemManager() noexcept;
    UserMemManager& operator=(const UserMemManager&) = delete;

    template<typename ResType = void>
    ResType* Allocate(size_t size);

    void Deallocate(void* ptr, size_t size) noexcept;

    template<typename ResType = void>
    ResType* Reallocate(void* ptr, size_t size, size_t newSize);

    bool ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept;
  };

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<size_t ptrUsefulBitCount>
	struct MemManagerCheckPtr
	{
		explicit MemManagerCheckPtr(void* ptr) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(ptrUsefulBitCount < sizeof(void*) * 8);
			MOMO_ASSERT((uintptr_t)ptr >> ptrUsefulBitCount == (uintptr_t)0);
		}
	};

	template<>
	struct MemManagerCheckPtr<sizeof(void*) * 8>
	{
		explicit MemManagerCheckPtr(void* /*ptr*/) MOMO_NOEXCEPT
		{
		}
	};
}

class MemManagerCpp
{
public:
	static const bool canReallocate = false;
	static const bool canReallocateInplace = false;

	static const size_t ptrUsefulBitCount = MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT;

public:
	MemManagerCpp() MOMO_NOEXCEPT
	{
	}

	MemManagerCpp(MemManagerCpp&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerCpp(const MemManagerCpp& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerCpp() MOMO_NOEXCEPT
	{
	}

	MemManagerCpp& operator=(const MemManagerCpp&) = delete;

	template<typename ResType = void>
	ResType* Allocate(size_t size)
	{
		MOMO_ASSERT(size > 0);
		void* ptr = operator new(size);
		(internal::MemManagerCheckPtr<ptrUsefulBitCount>)(ptr);
		return static_cast<ResType*>(ptr);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		(void)size;
		MOMO_ASSERT(ptr != nullptr && size > 0);
		operator delete(ptr);
	}
};

class MemManagerC
{
public:
	static const bool canReallocate = true;
	static const bool canReallocateInplace = false;

	static const size_t ptrUsefulBitCount = MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT;

public:
	MemManagerC() MOMO_NOEXCEPT
	{
	}

	MemManagerC(MemManagerC&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerC(const MemManagerC& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerC() MOMO_NOEXCEPT
	{
	}

	MemManagerC& operator=(const MemManagerC&) = delete;

	template<typename ResType = void>
	ResType* Allocate(size_t size)
	{
		MOMO_ASSERT(size > 0);
		void* ptr = malloc(size);
		if (ptr == nullptr)
			throw std::bad_alloc();
		(internal::MemManagerCheckPtr<ptrUsefulBitCount>)(ptr);
		return static_cast<ResType*>(ptr);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		(void)size;
		MOMO_ASSERT(ptr != nullptr && size > 0);
		free(ptr);
	}

	template<typename ResType = void>
	ResType* Reallocate(void* ptr, size_t size, size_t newSize)
	{
		MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
		if (size == newSize)
			return static_cast<ResType*>(ptr);
		void* newPtr = realloc(ptr, newSize);
		if (newPtr == nullptr)
			throw std::bad_alloc();
		(internal::MemManagerCheckPtr<ptrUsefulBitCount>)(newPtr);
		return static_cast<ResType*>(newPtr);
	}
};

#ifdef MOMO_USE_MEM_MANAGER_WIN
class MemManagerWin
{
public:
	static const bool canReallocate = false;
	static const bool canReallocateInplace = true;

	static const size_t ptrUsefulBitCount = MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT;

public:
	MemManagerWin() MOMO_NOEXCEPT
	{
	}

	MemManagerWin(MemManagerWin&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerWin(const MemManagerWin& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerWin() MOMO_NOEXCEPT
	{
	}

	MemManagerWin& operator=(const MemManagerWin&) = delete;

	template<typename ResType = void>
	ResType* Allocate(size_t size)
	{
		MOMO_ASSERT(size > 0);
		void* ptr = HeapAlloc(GetProcessHeap(), 0, size);
		if (ptr == nullptr)
			throw std::bad_alloc();
		(internal::MemManagerCheckPtr<ptrUsefulBitCount>)(ptr);
		return static_cast<ResType*>(ptr);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		(void)size;
		MOMO_ASSERT(ptr != nullptr && size > 0);
		HeapFree(GetProcessHeap(), 0, ptr);
	}

	//template<typename ResType = void>
	//void* Reallocate(void* ptr, size_t size, size_t newSize)
	//{
	//	MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
	//	if (size == newSize)
	//		return static_cast<ResType*>(ptr);
	//	void* newPtr = HeapReAlloc(GetProcessHeap(), 0, ptr, newSize);
	//	if (newPtr == nullptr)
	//		throw std::bad_alloc();
	//	(internal::MemManagerCheckPtr<ptrUsefulBitCount>)(newPtr);
	//	return static_cast<ResType*>(newPtr);
	//}

	bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
		if (size == newSize)
			return true;
		void* newPtr = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY,
			ptr, newSize);
		MOMO_ASSERT(newPtr == ptr || newPtr == nullptr);
		return newPtr == ptr;
	}
};
#endif

template<typename TAllocator>
class MemManagerStd : private std::allocator_traits<TAllocator>::template rebind_alloc<char>
{
public:
	typedef TAllocator Allocator;
	typedef typename std::allocator_traits<Allocator>::template rebind_alloc<char> CharAllocator;

	//MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<CharAllocator>::value);

	static const bool canReallocate = false;
	static const bool canReallocateInplace = false;

	static const size_t ptrUsefulBitCount = sizeof(void*) * 8;

public:
	MemManagerStd() MOMO_NOEXCEPT_IF(noexcept(CharAllocator()))
	{
	}

	explicit MemManagerStd(const Allocator& alloc) MOMO_NOEXCEPT
		: CharAllocator(alloc)
	{
	}

	MemManagerStd(MemManagerStd&& memManager) MOMO_NOEXCEPT
		: CharAllocator(std::move(memManager.GetCharAllocator()))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: CharAllocator(std::allocator_traits<CharAllocator>
			::select_on_container_copy_construction(memManager.GetCharAllocator()))
	{
	}

	~MemManagerStd() MOMO_NOEXCEPT
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	template<typename ResType = void>
	ResType* Allocate(size_t size)
	{
		void* ptr = std::allocator_traits<CharAllocator>::allocate(GetCharAllocator(), size);
		return static_cast<ResType*>(ptr);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		std::allocator_traits<CharAllocator>::deallocate(GetCharAllocator(),
			static_cast<char*>(ptr), size);
	}

	const CharAllocator& GetCharAllocator() const MOMO_NOEXCEPT
	{
		return *this;
	}

	CharAllocator& GetCharAllocator() MOMO_NOEXCEPT
	{
		return *this;
	}
};

typedef MOMO_DEFAULT_MEM_MANAGER MemManagerDefault;

template<typename Item>
class MemManagerStd<std::allocator<Item>>
	: private std::allocator<char>, public MemManagerDefault
{
public:
	typedef std::allocator<Item> Allocator;
	typedef std::allocator<char> CharAllocator;

public:
	explicit MemManagerStd(const Allocator& /*alloc*/ = Allocator())
		MOMO_NOEXCEPT_IF(noexcept(MemManagerDefault()))
	{
	}

	MemManagerStd(MemManagerStd&& memManager) MOMO_NOEXCEPT
		: MemManagerDefault(std::move(memManager))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: MemManagerDefault(memManager)
	{
	}

	~MemManagerStd() MOMO_NOEXCEPT
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	const CharAllocator& GetCharAllocator() const MOMO_NOEXCEPT
	{
		return *this;
	}

	CharAllocator& GetCharAllocator() MOMO_NOEXCEPT
	{
		return *this;
	}
};

namespace internal
{
	class MemManagerDummy
	{
	public:
		static const bool canReallocate = false;
		static const bool canReallocateInplace = false;

		static const size_t ptrUsefulBitCount = sizeof(void*) * 8;

	public:
		MemManagerDummy() MOMO_NOEXCEPT
		{
		}

		MemManagerDummy(MemManagerDummy&& /*memManager*/) MOMO_NOEXCEPT
		{
		}

		MemManagerDummy(const MemManagerDummy& /*memManager*/) MOMO_NOEXCEPT
		{
		}

		~MemManagerDummy() MOMO_NOEXCEPT
		{
		}

		MemManagerDummy& operator=(const MemManagerDummy&) = delete;

		//template<typename ResType = void>
		//ResType* Allocate(size_t size);

		void Deallocate(void* /*ptr*/, size_t /*size*/) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(false);
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

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	public:
		explicit MemManagerWrapper(MemManager&& memManager) MOMO_NOEXCEPT
			: mMemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&& memManagerWrapper) MOMO_NOEXCEPT
			: mMemManager(std::move(memManagerWrapper.mMemManager))
		{
		}

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() MOMO_NOEXCEPT
		{
		}

		MemManagerWrapper& operator=(MemManagerWrapper&& memManagerWrapper) MOMO_NOEXCEPT
		{
			if (this != &memManagerWrapper)
			{
				mMemManager.~MemManager();
				new(&mMemManager) MemManager(std::move(memManagerWrapper.mMemManager));
			}
			return *this;
		}

		MemManagerWrapper& operator=(const MemManagerWrapper&) = delete;

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return mMemManager;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return mMemManager;
		}

	private:
		MemManager mMemManager;
	};

	template<typename TMemManager>
	class MemManagerWrapper<TMemManager, true> : protected TMemManager	// vs
	{
	public:
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	public:
		explicit MemManagerWrapper(MemManager&& memManager) MOMO_NOEXCEPT
			: MemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&& memManagerWrapper) MOMO_NOEXCEPT
			: MemManager(std::move(memManagerWrapper.GetMemManager()))
		{
		}

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() MOMO_NOEXCEPT
		{
		}

		MemManagerWrapper& operator=(MemManagerWrapper&& /*memManagerWrapper*/) MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManagerWrapper& operator=(const MemManagerWrapper&) = delete;

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return *this;
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

		static const bool canReallocate = BaseMemManager::canReallocate;
		static const bool canReallocateInplace = BaseMemManager::canReallocateInplace;

		static const size_t ptrUsefulBitCount = BaseMemManager::ptrUsefulBitCount;

	public:
		MemManagerPtr() MOMO_NOEXCEPT
		{
		}

		explicit MemManagerPtr(BaseMemManager& /*memManager*/) MOMO_NOEXCEPT
		{
		}

		MemManagerPtr(MemManagerPtr&& /*memManagerPtr*/) MOMO_NOEXCEPT
		{
		}

		MemManagerPtr(const MemManagerPtr& /*memManagerPtr*/) MOMO_NOEXCEPT
		{
		}

		~MemManagerPtr() MOMO_NOEXCEPT
		{
		}

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		BaseMemManager& GetBaseMemManager() MOMO_NOEXCEPT
		{
			return *this;
		}

		template<typename ResType = void>
		ResType* Allocate(size_t size)
		{
			return GetBaseMemManager().template Allocate<ResType>(size);
		}

		void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
		{
			GetBaseMemManager().Deallocate(ptr, size);
		}

		template<typename ResType = void>
		ResType* Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return GetBaseMemManager().template Reallocate<ResType>(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
		{
			return GetBaseMemManager().ReallocateInplace(ptr, size, newSize);
		}
	};

	template<typename TBaseMemManager>
	class MemManagerPtr<TBaseMemManager, false>
	{
	public:
		typedef TBaseMemManager BaseMemManager;

		static const bool canReallocate = BaseMemManager::canReallocate;
		static const bool canReallocateInplace = BaseMemManager::canReallocateInplace;

		static const size_t ptrUsefulBitCount = BaseMemManager::ptrUsefulBitCount;

	public:
		explicit MemManagerPtr(BaseMemManager& memManager) MOMO_NOEXCEPT
			: mBaseMemManager(memManager)
		{
		}

		MemManagerPtr(MemManagerPtr&& memManagerPtr) MOMO_NOEXCEPT
			: mBaseMemManager(memManagerPtr.mBaseMemManager)
		{
		}

		MemManagerPtr(const MemManagerPtr& memManagerPtr) MOMO_NOEXCEPT
			: mBaseMemManager(memManagerPtr.mBaseMemManager)
		{
		}

		~MemManagerPtr() MOMO_NOEXCEPT
		{
		}

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		BaseMemManager& GetBaseMemManager() MOMO_NOEXCEPT
		{
			return mBaseMemManager;
		}

		template<typename ResType = void>
		ResType* Allocate(size_t size)
		{
			return mBaseMemManager.template Allocate<ResType>(size);
		}

		void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
		{
			mBaseMemManager.Deallocate(ptr, size);
		}

		template<typename ResType = void>
		ResType* Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return mBaseMemManager.template Reallocate<ResType>(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
		{
			return mBaseMemManager.ReallocateInplace(ptr, size, newSize);
		}

	private:
		BaseMemManager& mBaseMemManager;
	};
}

} // namespace momo
