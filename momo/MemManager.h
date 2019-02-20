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
    explicit UserMemManager(...);
    UserMemManager(UserMemManager&& memManager) noexcept;
    UserMemManager(const UserMemManager& memManager);
    ~UserMemManager() noexcept;
    UserMemManager& operator=(const UserMemManager&) = delete;

    void* Allocate(size_t size);

    void Deallocate(void* ptr, size_t size) noexcept;

    void* Reallocate(void* ptr, size_t size, size_t newSize);

    bool ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept;
  };

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename MemManager,
		typename = size_t>
	struct MemManagerPtrUsefulBitCount
	{
		static const size_t value = MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT;
	};

	template<typename MemManager>
	struct MemManagerPtrUsefulBitCount<MemManager, decltype(MemManager::ptrUsefulBitCount)>
	{
		static const size_t value = MemManager::ptrUsefulBitCount;
	};

	template<typename MemManager,
		typename = void*>
	struct MemManagerCanReallocate : public std::false_type
	{
	};

	template<typename MemManager>
	struct MemManagerCanReallocate<MemManager,
		decltype(std::declval<MemManager&>().Reallocate(nullptr, size_t{}, size_t{}))>
		: public std::true_type
	{
	};

	template<typename MemManager,
		typename = bool>
	struct MemManagerCanReallocateInplace : public std::false_type
	{
	};

	template<typename MemManager>
	struct MemManagerCanReallocateInplace<MemManager,
		decltype(std::declval<MemManager&>().ReallocateInplace(nullptr, size_t{}, size_t{}))>
		: public std::true_type
	{
	};

	template<typename TMemManager>
	class MemManagerProxy
	{
	public:
		typedef TMemManager MemManager;

		static const bool canReallocate = MemManagerCanReallocate<MemManager>::value;
		static const bool canReallocateInplace = MemManagerCanReallocateInplace<MemManager>::value;

		static const size_t ptrUsefulBitCount = MemManagerPtrUsefulBitCount<MemManager>::value;

	public:
		template<typename Result = void>
		static Result* Allocate(MemManager& memManager, size_t size)
		{
			MOMO_ASSERT(size > 0);
			void* ptr = memManager.Allocate(size);
			pvCheckPtr(ptr);
			return static_cast<Result*>(ptr);
		}

		static void Deallocate(MemManager& memManager, void* ptr, size_t size) noexcept
		{
			MOMO_ASSERT(ptr != nullptr && size > 0);
			memManager.Deallocate(ptr, size);
		}

		template<typename Result = void>
		static void* Reallocate(MemManager& memManager, void* ptr, size_t size, size_t newSize)
		{
			MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
			if (size == newSize)
				return static_cast<Result*>(ptr);
			void* newPtr = memManager.Reallocate(ptr, size, newSize);
			pvCheckPtr(newPtr);
			return static_cast<Result*>(newPtr);
		}

		static bool ReallocateInplace(MemManager& memManager, void* ptr, size_t size,
			size_t newSize) noexcept
		{
			MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
			if (size == newSize)
				return true;
			return memManager.ReallocateInplace(ptr, size, newSize);
		}

	private:
		template<size_t shift = ptrUsefulBitCount,
			typename std::enable_if<(shift < sizeof(void*) * 8), int>::type = 0>
		static void pvCheckPtr(void* ptr) noexcept
		{
			MOMO_ASSERT(reinterpret_cast<uintptr_t>(ptr) >> shift == (uintptr_t)0);
		}

		template<size_t shift = ptrUsefulBitCount,
			typename std::enable_if<(shift == sizeof(void*) * 8), int>::type = 0>
		static void pvCheckPtr(void* /*ptr*/) noexcept
		{
		}
	};
}

class MemManagerCpp
{
public:
	explicit MemManagerCpp() noexcept
	{
	}

	MemManagerCpp(MemManagerCpp&& /*memManager*/) noexcept
	{
	}

	MemManagerCpp(const MemManagerCpp& /*memManager*/) noexcept
	{
	}

	~MemManagerCpp() noexcept
	{
	}

	MemManagerCpp& operator=(const MemManagerCpp&) = delete;

	void* Allocate(size_t size)
	{
		return operator new(size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		(void)size;	// C++11
		operator delete(ptr);
	}
};

class MemManagerC
{
public:
	explicit MemManagerC() noexcept
	{
	}

	MemManagerC(MemManagerC&& /*memManager*/) noexcept
	{
	}

	MemManagerC(const MemManagerC& /*memManager*/) noexcept
	{
	}

	~MemManagerC() noexcept
	{
	}

	MemManagerC& operator=(const MemManagerC&) = delete;

	void* Allocate(size_t size)
	{
		void* ptr = malloc(size);
		if (ptr == nullptr)
			throw std::bad_alloc();
		return ptr;
	}

	void Deallocate(void* ptr, size_t /*size*/) noexcept
	{
		free(ptr);
	}

	void* Reallocate(void* ptr, size_t /*size*/, size_t newSize)
	{
		void* newPtr = realloc(ptr, newSize);
		if (newPtr == nullptr)
			throw std::bad_alloc();
		return newPtr;
	}
};

#ifdef MOMO_USE_MEM_MANAGER_WIN
class MemManagerWin
{
public:
	explicit MemManagerWin() noexcept
	{
	}

	MemManagerWin(MemManagerWin&& /*memManager*/) noexcept
	{
	}

	MemManagerWin(const MemManagerWin& /*memManager*/) noexcept
	{
	}

	~MemManagerWin() noexcept
	{
	}

	MemManagerWin& operator=(const MemManagerWin&) = delete;

	void* Allocate(size_t size)
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

	//void* Reallocate(void* ptr, size_t /*size*/, size_t newSize)
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

template<typename TAllocator>
class MemManagerStd : private std::allocator_traits<TAllocator>::template rebind_alloc<char>
{
public:
	typedef TAllocator Allocator;
	typedef typename std::allocator_traits<Allocator>::template rebind_alloc<char> CharAllocator;

	//MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<CharAllocator>::value);

public:
	explicit MemManagerStd() noexcept(noexcept(CharAllocator()))
	{
	}

	explicit MemManagerStd(const Allocator& alloc) noexcept
		: CharAllocator(alloc)
	{
	}

	MemManagerStd(MemManagerStd&& memManager) noexcept
		: CharAllocator(std::move(memManager.GetCharAllocator()))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: CharAllocator(std::allocator_traits<CharAllocator>
			::select_on_container_copy_construction(memManager.GetCharAllocator()))
	{
	}

	~MemManagerStd() noexcept
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	void* Allocate(size_t size)
	{
		return std::allocator_traits<CharAllocator>::allocate(GetCharAllocator(), size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		std::allocator_traits<CharAllocator>::deallocate(GetCharAllocator(),
			static_cast<char*>(ptr), size);
	}

	const CharAllocator& GetCharAllocator() const noexcept
	{
		return *this;
	}

	CharAllocator& GetCharAllocator() noexcept
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
		noexcept(noexcept(MemManagerDefault()))
	{
	}

	MemManagerStd(MemManagerStd&& memManager) noexcept
		: MemManagerDefault(std::move(memManager))
	{
	}

	MemManagerStd(const MemManagerStd& memManager)
		: MemManagerDefault(memManager)
	{
	}

	~MemManagerStd() noexcept
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	const CharAllocator& GetCharAllocator() const noexcept
	{
		return *this;
	}

	CharAllocator& GetCharAllocator() noexcept
	{
		return *this;
	}
};

namespace internal
{
	class MemManagerDummy
	{
	public:
		explicit MemManagerDummy() noexcept
		{
		}

		MemManagerDummy(MemManagerDummy&& /*memManager*/) noexcept
		{
		}

		MemManagerDummy(const MemManagerDummy& /*memManager*/) noexcept
		{
		}

		~MemManagerDummy() noexcept
		{
		}

		MemManagerDummy& operator=(const MemManagerDummy&) = delete;

		//void* Allocate(size_t size);

		void Deallocate(void* /*ptr*/, size_t /*size*/) noexcept
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
		explicit MemManagerWrapper(MemManager&& memManager) noexcept
			: mMemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&& memManagerWrapper) noexcept
			: mMemManager(std::move(memManagerWrapper.mMemManager))
		{
		}

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() noexcept
		{
		}

		MemManagerWrapper& operator=(MemManagerWrapper&& memManagerWrapper) noexcept
		{
			if (this != &memManagerWrapper)
			{
				mMemManager.~MemManager();
				new(&mMemManager) MemManager(std::move(memManagerWrapper.mMemManager));
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
	class MemManagerWrapper<TMemManager, true> : protected TMemManager	// vs
	{
	public:
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

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

		~MemManagerWrapper() noexcept
		{
		}

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

	template<typename TBaseMemManager,
		bool tIsEmpty = std::is_empty<TBaseMemManager>::value
			&& std::is_nothrow_default_constructible<TBaseMemManager>::value>
	class MemManagerPtr;

	template<typename TBaseMemManager>
	class MemManagerPtr<TBaseMemManager, true> : private TBaseMemManager
	{
	public:
		typedef TBaseMemManager BaseMemManager;

		static const size_t ptrUsefulBitCount = MemManagerProxy<BaseMemManager>::ptrUsefulBitCount;

	public:
		explicit MemManagerPtr() noexcept
		{
		}

		explicit MemManagerPtr(BaseMemManager& /*memManager*/) noexcept
		{
		}

		MemManagerPtr(MemManagerPtr&& /*memManagerPtr*/) noexcept
		{
		}

		MemManagerPtr(const MemManagerPtr& /*memManagerPtr*/) noexcept
		{
		}

		~MemManagerPtr() noexcept
		{
		}

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

		typename std::conditional<MemManagerProxy<BaseMemManager>::canReallocate, void*, void>::type
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return GetBaseMemManager().Reallocate(ptr, size, newSize);
		}

		typename std::conditional<MemManagerProxy<BaseMemManager>::canReallocate, bool, void>::type
		ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept
		{
			return GetBaseMemManager().ReallocateInplace(ptr, size, newSize);
		}
	};

	template<typename TBaseMemManager>
	class MemManagerPtr<TBaseMemManager, false>
	{
	public:
		typedef TBaseMemManager BaseMemManager;

		static const size_t ptrUsefulBitCount = MemManagerProxy<BaseMemManager>::ptrUsefulBitCount;

	public:
		explicit MemManagerPtr(BaseMemManager& memManager) noexcept
			: mBaseMemManager(memManager)
		{
		}

		MemManagerPtr(MemManagerPtr&& memManagerPtr) noexcept
			: mBaseMemManager(memManagerPtr.mBaseMemManager)
		{
		}

		MemManagerPtr(const MemManagerPtr& memManagerPtr) noexcept
			: mBaseMemManager(memManagerPtr.mBaseMemManager)
		{
		}

		~MemManagerPtr() noexcept
		{
		}

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

		typename std::conditional<MemManagerProxy<BaseMemManager>::canReallocate, void*, void>::type
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return mBaseMemManager.Reallocate(ptr, size, newSize);
		}

		typename std::conditional<MemManagerProxy<BaseMemManager>::canReallocate, bool, void>::type
		ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept
		{
			return mBaseMemManager.ReallocateInplace(ptr, size, newSize);
		}

	private:
		BaseMemManager& mBaseMemManager;
	};
}

} // namespace momo
