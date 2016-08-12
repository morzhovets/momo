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

class MemManagerCpp
{
public:
	static const bool canReallocate = false;
	static const bool canReallocateInplace = false;

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
		return static_cast<ResType*>(newPtr);
	}
};

#ifdef MOMO_USE_MEM_MANAGER_WIN
class MemManagerWin
{
public:
	static const bool canReallocate = false;
	static const bool canReallocateInplace = true;

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

public:
	MemManagerStd()
	{
	}

	explicit MemManagerStd(const Allocator& alloc) MOMO_NOEXCEPT
		: CharAllocator(alloc)
	{
	}

	MemManagerStd(MemManagerStd&& memManager) MOMO_NOEXCEPT
		: CharAllocator(std::move(memManager._GetCharAllocator()))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: CharAllocator(std::allocator_traits<CharAllocator>
			::select_on_container_copy_construction(memManager._GetCharAllocator()))
	{
	}

	~MemManagerStd() MOMO_NOEXCEPT
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	template<typename ResType = void>
	ResType* Allocate(size_t size)
	{
		void* ptr = std::allocator_traits<CharAllocator>::allocate(_GetCharAllocator(), size);
		return static_cast<ResType*>(ptr);
	}

	void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
	{
		std::allocator_traits<CharAllocator>::deallocate(_GetCharAllocator(),
			static_cast<char*>(ptr), size);
	}

	Allocator GetAllocator() const MOMO_NOEXCEPT
	{
		return Allocator(_GetCharAllocator());
	}

private:
	const CharAllocator& _GetCharAllocator() const MOMO_NOEXCEPT
	{
		return *this;
	}

	CharAllocator& _GetCharAllocator() MOMO_NOEXCEPT
	{
		return *this;
	}
};

typedef MOMO_DEFAULT_MEM_MANAGER MemManagerDefault;

template<typename TItem>
class MemManagerStd<std::allocator<TItem>> : public MemManagerDefault
{
public:
	typedef std::allocator<TItem> Allocator;

public:
	explicit MemManagerStd(const Allocator& /*alloc*/ = Allocator()) MOMO_NOEXCEPT
	{
	}

	MemManagerStd(MemManagerStd&& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	MemManagerStd(const MemManagerStd& /*memManager*/) MOMO_NOEXCEPT
	{
	}

	~MemManagerStd() MOMO_NOEXCEPT
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	Allocator GetAllocator() const MOMO_NOEXCEPT
	{
		return Allocator();
	}
};

namespace internal
{
	class MemManagerDummy
	{
	public:
		static const bool canReallocate = false;
		static const bool canReallocateInplace = false;

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
	class MemManagerWrapper<TMemManager, true> : protected TMemManager
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

	template<typename TMemManager,
		bool tIsMemManagerEmpty = std::is_empty<TMemManager>::value
			&& std::is_nothrow_default_constructible<TMemManager>::value>
	class MemManagerPtr;

	template<typename TMemManager>
	class MemManagerPtr<TMemManager, true> : private TMemManager
	{
	public:
		typedef TMemManager MemManager;

		static const bool canReallocate = MemManager::canReallocate;
		static const bool canReallocateInplace = MemManager::canReallocateInplace;

	public:
		MemManagerPtr() MOMO_NOEXCEPT
		{
		}

		explicit MemManagerPtr(MemManager& /*memManager*/) MOMO_NOEXCEPT
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

		template<typename ResType = void>
		ResType* Allocate(size_t size)
		{
			return _GetMemManager().template Allocate<ResType>(size);
		}

		void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
		{
			return _GetMemManager().Deallocate(ptr, size);
		}

		template<typename ResType = void>
		ResType* Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return _GetMemManager().template Reallocate<ResType>(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
		{
			return _GetMemManager().ReallocateInplace(ptr, size, newSize);
		}

	private:
		MemManager& _GetMemManager() MOMO_NOEXCEPT
		{
			return *this;
		}
	};

	template<typename TMemManager>
	class MemManagerPtr<TMemManager, false>
	{
	public:
		typedef TMemManager MemManager;

		static const bool canReallocate = MemManager::canReallocate;
		static const bool canReallocateInplace = MemManager::canReallocateInplace;

	public:
		explicit MemManagerPtr(MemManager& memManager) MOMO_NOEXCEPT
			: mMemManager(memManager)
		{
		}

		MemManagerPtr(MemManagerPtr&& memManagerPtr) MOMO_NOEXCEPT
			: mMemManager(memManagerPtr.mMemManager)
		{
		}

		MemManagerPtr(const MemManagerPtr& memManagerPtr) MOMO_NOEXCEPT
			: mMemManager(memManagerPtr.mMemManager)
		{
		}

		~MemManagerPtr() MOMO_NOEXCEPT
		{
		}

		MemManagerPtr& operator=(const MemManagerPtr&) = delete;

		template<typename ResType = void>
		ResType* Allocate(size_t size)
		{
			return mMemManager.template Allocate<ResType>(size);
		}

		void Deallocate(void* ptr, size_t size) MOMO_NOEXCEPT
		{
			return mMemManager.Deallocate(ptr, size);
		}

		template<typename ResType = void>
		ResType* Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return mMemManager.template Reallocate<ResType>(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) MOMO_NOEXCEPT
		{
			return mMemManager.ReallocateInplace(ptr, size, newSize);
		}

	private:
		MemManager& mMemManager;
	};
}

} // namespace momo
