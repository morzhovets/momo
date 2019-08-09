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

    void* Reallocate(void* ptr, size_t size, size_t newSize); // optional

    bool ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept; // optional

    bool IsEqual(const UserMemManager& memManager) const noexcept; // optional
  };

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

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
	typedef typename std::allocator_traits<Allocator>::template rebind_alloc<char> ByteAllocator;

	//MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<ByteAllocator>::value);

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

	~MemManagerStd() noexcept
	{
	}

	MemManagerStd& operator=(const MemManagerStd&) = delete;

	void* Allocate(size_t size)
	{
		return std::allocator_traits<ByteAllocator>::allocate(GetByteAllocator(), size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		std::allocator_traits<ByteAllocator>::deallocate(GetByteAllocator(),
			static_cast<char*>(ptr), size);
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

typedef MOMO_DEFAULT_MEM_MANAGER MemManagerDefault;

template<typename Item>
class MemManagerStd<std::allocator<Item>>
	: private std::allocator<char>, public MemManagerDefault
{
public:
	typedef std::allocator<Item> Allocator;
	typedef std::allocator<char> ByteAllocator;

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

	~MemManagerStd() noexcept
	{
	}

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
			static const size_t value = MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT;
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
			return pvIsEqual(memManager1, memManager2, HasIsEqual<MemManager>());
		}

	private:
		template<size_t shift = ptrUsefulBitCount>
		static EnableIf<(shift < sizeof(void*) * 8)> pvCheckBits(void* ptr) noexcept
		{
			MOMO_ASSERT(BitCaster::ToUInt(ptr) >> shift == (uintptr_t)0);
		}

		template<size_t shift = ptrUsefulBitCount>
		static EnableIf<(shift == sizeof(void*) * 8)> pvCheckBits(void* /*ptr*/) noexcept
		{
		}

		static bool pvIsEqual(const MemManager& memManager1, const MemManager& memManager2,
			std::true_type /*hasIsEqual*/) noexcept
		{
			return memManager1.IsEqual(memManager2);
		}

		static bool pvIsEqual(const MemManager& memManager1, const MemManager& memManager2,
			std::false_type /*hasIsEqual*/) noexcept
		{
			return &memManager1 == &memManager2 || std::is_empty<MemManager>::value;
		}
	};

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

		typename std::conditional<BaseMemManagerProxy::canReallocate, void*, void>::type
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return GetBaseMemManager().Reallocate(ptr, size, newSize);
		}

		typename std::conditional<BaseMemManagerProxy::canReallocateInplace, bool, void>::type
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

		typename std::conditional<BaseMemManagerProxy::canReallocate, void*, void>::type
		Reallocate(void* ptr, size_t size, size_t newSize)
		{
			return mBaseMemManager.Reallocate(ptr, size, newSize);
		}

		typename std::conditional<BaseMemManagerProxy::canReallocateInplace, bool, void>::type
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
