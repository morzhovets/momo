/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MemManager.h

  namespace momo:
    concept conceptMemManager
    class MemManagerCpp
    class MemManagerC
    class MemManagerWin
    class MemManagerStdByte
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

#ifdef MOMO_USE_MEM_MANAGER_WIN
#include <Windows.h>
#endif

namespace momo
{

template<typename MemManager>
concept conceptMemManager =
	std::is_nothrow_destructible_v<MemManager> &&
	std::is_nothrow_move_constructible_v<MemManager> &&
	requires (MemManager& memManager, void* ptr, size_t size)
	{
		{ memManager.Allocate(size) } -> std::same_as<void*>;
		{ memManager.Deallocate(ptr, size) } noexcept -> std::same_as<void>;
	};

namespace internal
{
	template<typename MemManager>
	concept conceptMemManagerWithReallocate = conceptMemManager<MemManager> &&
		requires (MemManager& memManager, void* ptr, size_t size)
			{ { memManager.Reallocate(ptr, size, size) } -> std::same_as<void*>; };

	template<typename MemManager>
	concept conceptMemManagerWithReallocateInplace = conceptMemManager<MemManager> &&
		requires (MemManager& memManager, void* ptr, size_t size)
			{ { memManager.ReallocateInplace(ptr, size, size) } noexcept -> std::same_as<bool>; };

	template<typename MemManager>
	concept conceptMemManagerWithIsEqual = conceptMemManager<MemManager> &&
		requires (const MemManager& memManager)
			{ { memManager.IsEqual(memManager) } noexcept -> std::same_as<bool>; };

	template<typename MemManager>
	concept conceptMemManagerWithPtrUsefulBitCount = conceptMemManager<MemManager> &&
		requires { typename std::integral_constant<size_t, MemManager::ptrUsefulBitCount>; };

	template<typename Allocator>
	concept conceptAllocator =
		requires (Allocator& alloc, typename Allocator::value_type* ptr, size_t count)
		{
			{ alloc.allocate(count) } -> std::same_as<typename Allocator::value_type*>;
			{ alloc.deallocate(ptr, count) } -> std::same_as<void>;
		};

	template<typename Allocator>
	concept conceptByteAllocator = conceptAllocator<Allocator> &&
		std::is_same_v<typename Allocator::value_type, std::byte>;

	template<typename Allocator, typename Object, typename... ObjectArgs>
	concept conceptByteAllocatorWithConstruct = conceptByteAllocator<Allocator> &&
		requires (Allocator& alloc, Object* newObject, ObjectArgs&&... objectArgs)
		{
			{ alloc.construct(newObject, std::forward<ObjectArgs>(objectArgs)...) };
		};
}

//! `MemManagerCpp` uses `new` and `delete`
class MemManagerCpp
{
public:
	explicit MemManagerCpp() noexcept = default;

	MemManagerCpp(MemManagerCpp&&) noexcept = default;

	MemManagerCpp(const MemManagerCpp&) noexcept = default;

	~MemManagerCpp() noexcept = default;

	MemManagerCpp& operator=(const MemManagerCpp&) = delete;

	[[nodiscard]] void* Allocate(size_t size)
	{
		return operator new(size);
	}

	void Deallocate(void* ptr, [[maybe_unused]] size_t size) noexcept
	{
#ifdef __cpp_sized_deallocation	// clang
		operator delete(ptr, size);
#else
		operator delete(ptr);
#endif
	}
};

//! `MemManagerC` uses `malloc`, `free` and `realloc`
class MemManagerC
{
public:
	explicit MemManagerC() noexcept = default;

	MemManagerC(MemManagerC&&) noexcept = default;

	MemManagerC(const MemManagerC&) noexcept = default;

	~MemManagerC() noexcept = default;

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
	explicit MemManagerWin() noexcept = default;

	MemManagerWin(MemManagerWin&&) noexcept = default;

	MemManagerWin(const MemManagerWin&) noexcept = default;

	~MemManagerWin() noexcept = default;

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

	[[nodiscard]] void* Reallocate(void* ptr, size_t /*size*/, size_t newSize)
	{
		void* newPtr = HeapReAlloc(GetProcessHeap(), 0, ptr, newSize);
		if (newPtr == nullptr)
			throw std::bad_alloc();
		return newPtr;
	}

	[[nodiscard]] bool ReallocateInplace(void* ptr, size_t /*size*/, size_t newSize) noexcept
	{
		void* newPtr = HeapReAlloc(GetProcessHeap(), HEAP_REALLOC_IN_PLACE_ONLY,
			ptr, newSize);
		MOMO_ASSERT(newPtr == ptr || newPtr == nullptr);
		return newPtr == ptr;
	}
};
#endif

//! `MemManagerStdByte` uses `ByteAllocator::allocate` and `deallocate`
template<internal::conceptByteAllocator TByteAllocator>
class MemManagerStdByte : private TByteAllocator
{
public:
	typedef TByteAllocator ByteAllocator;

private:
	typedef std::allocator_traits<ByteAllocator> ByteAllocatorTraits;

public:
	explicit MemManagerStdByte() = default;

	template<internal::conceptAllocator Allocator>
	requires (std::is_same_v<Allocator,
		typename ByteAllocatorTraits::template rebind_alloc<typename Allocator::value_type>>)
	explicit MemManagerStdByte(const Allocator& alloc) noexcept
		: ByteAllocator(alloc)
	{
	}

	MemManagerStdByte(MemManagerStdByte&&) noexcept = default;

	MemManagerStdByte(const MemManagerStdByte& memManager)
		: ByteAllocator(ByteAllocatorTraits::select_on_container_copy_construction(
			memManager.GetByteAllocator()))
	{
	}

	~MemManagerStdByte() noexcept = default;

	MemManagerStdByte& operator=(const MemManagerStdByte&) = delete;

	[[nodiscard]] void* Allocate(size_t size)
	{
		return ByteAllocatorTraits::allocate(GetByteAllocator(), size);
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		ByteAllocatorTraits::deallocate(GetByteAllocator(), static_cast<std::byte*>(ptr), size);
	}

	bool IsEqual(const MemManagerStdByte& memManager) const noexcept
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

//! `MemManagerStd` uses `Allocator<std::byte>::allocate` and `deallocate`
template<internal::conceptAllocator TAllocator>
using MemManagerStd =
	MemManagerStdByte<typename std::allocator_traits<TAllocator>::template rebind_alloc<std::byte>>;

//! `MemManagerDefault` is defined in UserSettings.h
typedef MOMO_DEFAULT_MEM_MANAGER MemManagerDefault;

#ifdef MOMO_USE_DEFAULT_MEM_MANAGER_IN_STD
//! `MemManagerStdByte<std::allocator<std::byte>>` is same as `MemManagerDefault`
template<>
class MemManagerStdByte<std::allocator<std::byte>> : public MemManagerDefault
{
public:
	typedef std::allocator<std::byte> ByteAllocator;

public:
	explicit MemManagerStdByte() = default;

	template<typename Object>
	explicit MemManagerStdByte(const std::allocator<Object>& /*alloc*/)
		noexcept(noexcept(MemManagerDefault()))
	{
	}

	MemManagerStdByte(MemManagerStdByte&&) noexcept = default;

	MemManagerStdByte(const MemManagerStdByte&) = default;

	~MemManagerStdByte() noexcept = default;

	MemManagerStdByte& operator=(const MemManagerStdByte&) = delete;

	const ByteAllocator& GetByteAllocator() const noexcept
	{
		return pvGetByteAllocator();
	}

	ByteAllocator& GetByteAllocator() noexcept
	{
		return pvGetByteAllocator();
	}

private:
	static ByteAllocator& pvGetByteAllocator() noexcept
	{
		static ByteAllocator byteAllocator;
		return byteAllocator;
	}
};
#endif

namespace internal
{
	template<conceptMemManager TMemManager>
	class MemManagerProxy
	{
	public:
		typedef TMemManager MemManager;

	private:
		static constexpr size_t pvGetPtrUsefulBitCount() noexcept
		{
			if constexpr (conceptMemManagerWithPtrUsefulBitCount<MemManager>)
			{
				return MemManager::ptrUsefulBitCount;
			}
			else
			{
#ifdef MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT
				return MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT;
#else
				return sizeof(void*) * 8;
#endif
			}
		}

	public:
		static const bool canReallocate = conceptMemManagerWithReallocate<MemManager>;
		static const bool canReallocateInplace = conceptMemManagerWithReallocateInplace<MemManager>;

		static const size_t ptrUsefulBitCount = pvGetPtrUsefulBitCount();

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
			requires canReallocate
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
			size_t newSize) noexcept requires canReallocateInplace
		{
			MOMO_ASSERT(ptr != nullptr && size > 0 && newSize > 0);
			if (size == newSize)
				return true;
			return memManager.ReallocateInplace(ptr, size, newSize);
		}

		static bool IsEqual(const MemManager& memManager1, const MemManager& memManager2) noexcept
		{
			if constexpr (conceptMemManagerWithIsEqual<MemManager>)
				return memManager1.IsEqual(memManager2);
			else
				return &memManager1 == &memManager2 || std::is_empty_v<MemManager>;	//?
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

	private:
		static void pvCheckBits([[maybe_unused]] void* ptr) noexcept
		{
			if constexpr (ptrUsefulBitCount < sizeof(void*) * 8)
				MOMO_ASSERT(PtrCaster::ToUInt(ptr) >> ptrUsefulBitCount == uintptr_t{0});
		}
	};

	template<conceptMemManager TMemManager>
	class MemManagerWrapper
	{
	public:
		typedef TMemManager MemManager;

	public:
		explicit MemManagerWrapper(MemManager&& memManager) noexcept
			: mMemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&&) noexcept = default;

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() noexcept = default;

		MemManagerWrapper& operator=(MemManagerWrapper&& memManagerWrapper) noexcept
		{
			if (this != &memManagerWrapper)
			{
				std::destroy_at(&mMemManager);
				std::construct_at(&mMemManager, std::move(memManagerWrapper.mMemManager));
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

	template<conceptMemManager TMemManager>
	requires (std::is_empty_v<TMemManager>)
	class MemManagerWrapper<TMemManager> : private TMemManager
	{
	public:
		typedef TMemManager MemManager;

	public:
		explicit MemManagerWrapper(MemManager&& memManager) noexcept
			: MemManager(std::move(memManager))
		{
		}

		MemManagerWrapper(MemManagerWrapper&&) noexcept = default;

		MemManagerWrapper(const MemManagerWrapper&) = delete;

		~MemManagerWrapper() noexcept = default;

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
		explicit MemManagerDummy() noexcept = default;

		MemManagerDummy(MemManagerDummy&&) noexcept = default;

		MemManagerDummy(const MemManagerDummy&) noexcept = default;

		~MemManagerDummy() noexcept = default;

		MemManagerDummy& operator=(const MemManagerDummy&) = delete;

		[[noreturn]] void* Allocate(size_t /*size*/)
		{
			MOMO_ASSERT(false);
			throw std::bad_alloc();
		}

		void Deallocate(void* /*ptr*/, size_t /*size*/) noexcept
		{
			MOMO_ASSERT(false);
		}
	};

	template<conceptMemManager TBaseMemManager>
	class MemManagerPtr
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

		MemManagerPtr(MemManagerPtr&&) noexcept = default;

		MemManagerPtr(const MemManagerPtr&) noexcept = default;

		~MemManagerPtr() noexcept = default;

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

		void* Reallocate(void* ptr, size_t size, size_t newSize)
			requires BaseMemManagerProxy::canReallocate
		{
			return mBaseMemManager.Reallocate(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept
			requires BaseMemManagerProxy::canReallocateInplace
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

	template<conceptMemManager TBaseMemManager>
	requires conceptSmallAndTriviallyCopyable<TBaseMemManager>
	class MemManagerPtr<TBaseMemManager> : private TBaseMemManager
	{
	public:
		typedef TBaseMemManager BaseMemManager;

	private:
		typedef MemManagerProxy<BaseMemManager> BaseMemManagerProxy;

	public:
		static const size_t ptrUsefulBitCount = BaseMemManagerProxy::ptrUsefulBitCount;

	public:
		explicit MemManagerPtr(BaseMemManager& baseMemManager) noexcept
			: BaseMemManager(std::as_const(baseMemManager))
		{
		}

		MemManagerPtr(MemManagerPtr&&) noexcept = default;

		MemManagerPtr(const MemManagerPtr&) noexcept = default;

		~MemManagerPtr() noexcept = default;

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

		void* Reallocate(void* ptr, size_t size, size_t newSize)
			requires BaseMemManagerProxy::canReallocate
		{
			return GetBaseMemManager().Reallocate(ptr, size, newSize);
		}

		bool ReallocateInplace(void* ptr, size_t size, size_t newSize) noexcept
			requires BaseMemManagerProxy::canReallocateInplace
		{
			return GetBaseMemManager().ReallocateInplace(ptr, size, newSize);
		}

		bool IsEqual(const MemManagerPtr& memManager) const noexcept
		{
			return BaseMemManagerProxy::IsEqual(*this, memManager);
		}
	};
}

} // namespace momo
