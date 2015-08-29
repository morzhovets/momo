/**********************************************************\

  momo/MemPool.h

  namespace momo:
    class MemPool

\**********************************************************/

#pragma once

#include "MemManager.h"
#include "Array.h"

namespace momo
{

template<size_t tBlockCount = MOMO_DEFAULT_MEM_POOL_BLOCK_COUNT,
	typename TMemManager = MemManagerDefault>
class MemPool : private internal::MemManagerWrapper<TMemManager>
{
public:
	typedef TMemManager MemManager;

	static const size_t blockCount = tBlockCount;

	static const size_t minBlockSize = sizeof(void*);
	static const size_t maxBlockSize = (SIZE_MAX - 2 * sizeof(void*)) / blockCount;
	MOMO_STATIC_ASSERT(minBlockSize <= maxBlockSize);

private:
	typedef internal::MemManagerWrapper<MemManager> MemManagerWrapper;

public:
	explicit MemPool(size_t blockSize, MemManager&& memManager = MemManager())
		: MemManagerWrapper(std::move(memManager)),
		mBlockHead(nullptr),
		mBufferHead(nullptr),
		mBlockSize(blockSize),
		mAllocCount(0)
	{
		if (mBlockSize < minBlockSize)
			mBlockSize = minBlockSize;
		if (mBlockSize > maxBlockSize)
			throw std::length_error("momo::MemPool length error");
	}

	MemPool(MemPool&& memPool) MOMO_NOEXCEPT
		: MemManagerWrapper(std::move(memPool._GetMemManagerWrapper())),
		mBlockHead(memPool.mBlockHead),
		mBufferHead(memPool.mBufferHead),
		mBlockSize(memPool.mBlockSize),
		mAllocCount(memPool.mAllocCount)
	{
		memPool.mBlockHead = nullptr;
		memPool.mBufferHead = nullptr;
		memPool.mAllocCount = 0;
	}

	~MemPool() MOMO_NOEXCEPT
	{
		assert(mAllocCount == 0);
		_Clear();
	}

	MemPool& operator=(MemPool&& memPool) MOMO_NOEXCEPT
	{
		MemPool(std::move(memPool)).Swap(*this);
		return *this;
	}

	void Swap(MemPool& memPool) MOMO_NOEXCEPT
	{
		std::swap(_GetMemManagerWrapper(), memPool._GetMemManagerWrapper());
		std::swap(mBlockHead, memPool.mBlockHead);
		std::swap(mBufferHead, memPool.mBufferHead);
		std::swap(mBlockSize, memPool.mBlockSize);
		std::swap(mAllocCount, memPool.mAllocCount);
	}

	MOMO_FRIEND_SWAP(MemPool)

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	size_t GetBlockSize() const MOMO_NOEXCEPT
	{
		return mBlockSize;
	}

	void* GetMemory()
	{
		if (mBlockHead == nullptr)
			_NewBuffer(GetMemManager().Allocate(_GetBufferSize()));
		void* ptr = mBlockHead;
		mBlockHead = *(void**)mBlockHead;
		++mAllocCount;
		return ptr;
	}

	void FreeMemory(void* ptr) MOMO_NOEXCEPT
	{
		assert(ptr != nullptr);
		assert(mAllocCount > 0);
		*(void**)ptr = mBlockHead;
		mBlockHead = ptr;
		--mAllocCount;
		if (mAllocCount == 0)
			_Shrink();
	}

	void FreeAll() MOMO_NOEXCEPT	//?
	{
		if (mAllocCount > 0)
			_Shrink();
		mAllocCount = 0;
	}

private:
	const MemManagerWrapper& _GetMemManagerWrapper() const MOMO_NOEXCEPT
	{
		return *this;
	}

	MemManagerWrapper& _GetMemManagerWrapper() MOMO_NOEXCEPT
	{
		return *this;
	}

	void _NewBuffer(void* buffer) MOMO_NOEXCEPT
	{
		for (size_t i = 0; i < blockCount; ++i)
		{
			char* ptr = (char*)buffer + mBlockSize * i;
			*(void**)ptr = (i + 1 < blockCount) ? ptr + mBlockSize : nullptr;
		}
		_GetNextBuffer(buffer) = mBufferHead;
		mBufferHead = buffer;
		mBlockHead = buffer;
	}

	void _Shrink() MOMO_NOEXCEPT
	{
		void* bufferNext = _GetNextBuffer(mBufferHead);
		if (bufferNext != nullptr)
		{
			void* bufferHead = mBufferHead;
			mBufferHead = bufferNext;
			_Clear();
			_NewBuffer(bufferHead);
		}
	}

	void _Clear() MOMO_NOEXCEPT
	{
		MemManager& memManager = GetMemManager();
		size_t bufferSize = _GetBufferSize();
		while (mBufferHead != nullptr)
		{
			void* buffer = mBufferHead;
			mBufferHead = _GetNextBuffer(mBufferHead);
			memManager.Deallocate(buffer, bufferSize);
		}
	}

	void*& _GetNextBuffer(void* buffer) MOMO_NOEXCEPT
	{
		return *((void**)((char*)buffer + _GetBufferSize()) - 1);
	}

	size_t _GetBufferSize() const MOMO_NOEXCEPT
	{
		return ((blockCount * mBlockSize - 1) / sizeof(void*) + 2) * sizeof(void*);
	}

private:
	MOMO_DISABLE_COPY_CONSTRUCTOR(MemPool);
	MOMO_DISABLE_COPY_OPERATOR(MemPool);

private:
	void* mBlockHead;
	void* mBufferHead;
	size_t mBlockSize;
	size_t mAllocCount;
};

template<typename TMemManager>
class MemPool<0, TMemManager> : private internal::MemManagerWrapper<TMemManager>
{
public:
	typedef TMemManager MemManager;

	static const size_t blockCount = 0;

	static const size_t minBlockSize = 1;
	static const size_t maxBlockSize = SIZE_MAX;

private:
	typedef internal::MemManagerWrapper<MemManager> MemManagerWrapper;

public:
	explicit MemPool(size_t blockSize, MemManager&& memManager = MemManager())
		: MemManagerWrapper(std::move(memManager)),
		mBlockSize(blockSize)
	{
		if (mBlockSize < minBlockSize)
			mBlockSize = minBlockSize;
		_GetAllocCount() = 0;
	}

	MemPool(MemPool&& memPool) MOMO_NOEXCEPT
		: MemManagerWrapper(std::move(memPool._GetMemManagerWrapper())),
		mBlockSize(memPool.mBlockSize)
	{
		_GetAllocCount() = memPool._GetAllocCount();
		memPool._GetAllocCount() = 0;
	}

	~MemPool() MOMO_NOEXCEPT
	{
		assert(_GetAllocCount() == 0);
	}

	MemPool& operator=(MemPool&& memPool) MOMO_NOEXCEPT
	{
		MemPool(std::move(memPool)).Swap(*this);
		return *this;
	}

	void Swap(MemPool& memPool) MOMO_NOEXCEPT
	{
		std::swap(_GetMemManagerWrapper(), memPool._GetMemManagerWrapper());
		std::swap(mBlockSize, memPool.mBlockSize);
		std::swap(_GetAllocCount(), memPool._GetAllocCount());
	}

	MOMO_FRIEND_SWAP(MemPool)

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	size_t GetBlockSize() const MOMO_NOEXCEPT
	{
		return mBlockSize;
	}

	void* GetMemory()
	{
		void* ptr = GetMemManager().Allocate(mBlockSize);
		++_GetAllocCount();
		return ptr;
	}

	void FreeMemory(void* ptr) MOMO_NOEXCEPT
	{
		assert(ptr != nullptr);
		assert(_GetAllocCount() > 0);
		GetMemManager().Deallocate(ptr, mBlockSize);
		--_GetAllocCount();
	}

private:
	const MemManagerWrapper& _GetMemManagerWrapper() const MOMO_NOEXCEPT
	{
		return *this;
	}

	MemManagerWrapper& _GetMemManagerWrapper() MOMO_NOEXCEPT
	{
		return *this;
	}

	size_t& _GetAllocCount() MOMO_NOEXCEPT
	{
#ifndef NDEBUG
		return mAllocCount;
#else
		static size_t allocCount = 0;
		return allocCount;
#endif
	}

private:
	MOMO_DISABLE_COPY_CONSTRUCTOR(MemPool);
	MOMO_DISABLE_COPY_OPERATOR(MemPool);

private:
	size_t mBlockSize;
#ifndef NDEBUG
	size_t mAllocCount;
#endif
};

namespace internal
{
	template<size_t tBlockCount, typename TMemManager>
	class MemPoolUInt32
	{
	public:
		typedef TMemManager MemManager;

		static const size_t blockCount = tBlockCount;
		MOMO_STATIC_ASSERT(blockCount > 0);

		static const size_t minBlockSize = sizeof(uint32_t);
		static const size_t maxBlockSize = SIZE_MAX / blockCount;
		MOMO_STATIC_ASSERT(minBlockSize <= maxBlockSize);

		static const uint32_t nullPtr = UINT32_MAX;

	private:
		typedef Array<void*, MemManager> Buffers;

	public:
		MemPoolUInt32(size_t blockSize, MemManager&& memManager, size_t maxTotalBlockCount)
			: mBuffers(std::move(memManager)),
			mBlockHead(nullPtr),
			mMaxBufferCount(maxTotalBlockCount / blockCount),
			mBlockSize(blockSize),
			mAllocCount(0)
		{
			assert(maxTotalBlockCount < (size_t)UINT32_MAX);
			if (mBlockSize < minBlockSize)
				mBlockSize = minBlockSize;
			if (mBlockSize > maxBlockSize)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
		}

		~MemPoolUInt32() MOMO_NOEXCEPT
		{
			assert(mAllocCount == 0);
			_Clear();
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return mBuffers.GetMemManager();
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return mBuffers.GetMemManager();
		}

		const void* GetRealPointer(uint32_t ptr) const MOMO_NOEXCEPT
		{
			return _GetRealPointer(ptr);
		}

		void* GetRealPointer(uint32_t ptr) MOMO_NOEXCEPT
		{
			return _GetRealPointer(ptr);
		}

		uint32_t GetMemory()
		{
			if (mBlockHead == nullPtr)
				_NewBuffer();
			uint32_t ptr = mBlockHead;
			mBlockHead = *(uint32_t*)GetRealPointer(mBlockHead);
			++mAllocCount;
			return ptr;
		}

		void FreeMemory(uint32_t ptr) MOMO_NOEXCEPT
		{
			assert(ptr != nullPtr);
			assert(mAllocCount > 0);
			*(uint32_t*)GetRealPointer(ptr) = mBlockHead;
			mBlockHead = ptr;
			--mAllocCount;
			if (mAllocCount == 0)
				_Shrink();
		}

	private:
		void* _GetRealPointer(uint32_t ptr) const MOMO_NOEXCEPT
		{
			assert(ptr != nullPtr);
			char* buffer = (char*)mBuffers[ptr / blockCount];
			void* realPtr = buffer + (size_t)(ptr % blockCount) * mBlockSize;
			return realPtr;
		}

		void _NewBuffer()
		{
			size_t bufferCount = mBuffers.GetCount();
			if (bufferCount >= mMaxBufferCount)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
			mBuffers.Reserve(bufferCount + 1);
			char* buffer = (char*)GetMemManager().Allocate(_GetBufferSize());
			for (size_t i = 0; i < blockCount; ++i)
			{
				void* realPtr = buffer + mBlockSize * i;
				*(uint32_t*)realPtr = (i + 1 < blockCount)
					? (uint32_t)(bufferCount * blockCount + i + 1) : nullPtr;
			}
			mBlockHead = (uint32_t)(bufferCount * blockCount);
			mBuffers.AddBackNogrow(buffer);
		}

		void _Shrink() MOMO_NOEXCEPT
		{
			if (mBuffers.GetCount() > 2)
			{
				_Clear();
				mBlockHead = nullPtr;
				mBuffers.Clear(true);
			}
		}

		void _Clear() MOMO_NOEXCEPT
		{
			MemManager& memManager = GetMemManager();
			size_t bufferSize = _GetBufferSize();
			for (void* buffer : mBuffers)
				memManager.Deallocate(buffer, bufferSize);
		}

		size_t _GetBufferSize() const MOMO_NOEXCEPT
		{
			return blockCount * mBlockSize;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(MemPoolUInt32);
		MOMO_DISABLE_COPY_OPERATOR(MemPoolUInt32);

	private:
		Buffers mBuffers;
		uint32_t mBlockHead;
		size_t mMaxBufferCount;
		size_t mBlockSize;
		size_t mAllocCount;
	};
}

} // namespace momo
