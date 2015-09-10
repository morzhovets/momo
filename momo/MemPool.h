/**********************************************************\

  momo/MemPool.h

  namespace momo:
    struct MemPoolConst
    class MemPool2Params
    class MemPool2ParamsVarSize
    class MemPool2Settings
    class MemPool2
    class MemPool

\**********************************************************/

#pragma once

#include "MemManager.h"
#include "Array.h"

namespace momo
{

struct MemPoolConst
{
	static const size_t defaultBlockCount = MOMO_DEFAULT_MEM_POOL_BLOCK_COUNT;
};

template<size_t tBlockSize,
	size_t tBlockAlignment = MOMO_MAX_ALIGNMENT,	//?
	size_t tBlockCount = MemPoolConst::defaultBlockCount>
class MemPool2Params
{
public:
	static const size_t blockCount = tBlockCount;
	MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

	static const size_t blockAlignment = tBlockAlignment;
	MOMO_STATIC_ASSERT(0 < blockAlignment && blockAlignment <= 1024);

	static const size_t blockSize = (tBlockSize <= blockAlignment) ? 2 * blockAlignment
		: ((tBlockSize - 1) / blockAlignment + 1) * blockAlignment;
};

template<size_t tBlockAlignment = MOMO_MAX_ALIGNMENT,
	size_t tBlockCount = MemPoolConst::defaultBlockCount>
class MemPool2ParamsVarSize
{
public:
	static const size_t blockCount = tBlockCount;
	MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

	static const size_t blockAlignment = tBlockAlignment;
	MOMO_STATIC_ASSERT(0 < blockAlignment && blockAlignment <= 1024);

public:
	explicit MemPool2ParamsVarSize(size_t blockSize) MOMO_NOEXCEPT
		: blockSize((blockSize <= blockAlignment) ? 2 * blockAlignment
			: internal::UIntMath<size_t>::Ceil(blockSize, blockAlignment))
	{
	}

protected:
	size_t blockSize;
};

struct MemPool2Settings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
};

template<typename TParams = MemPool2ParamsVarSize<>,
	typename TMemManager = MemManagerDefault,
	typename TSettings = MemPool2Settings>
class MemPool2 : private TParams, private internal::MemManagerWrapper<TMemManager>
{
public:
	typedef TParams Params;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	//MOMO_STATIC_ASSERT(std::is_trivially_copyable<Params>::value);
	//MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<Params>::value);
	//MOMO_STATIC_ASSERT(std::is_nothrow_move_assignable<Params>::value);

private:
	typedef internal::MemManagerWrapper<MemManager> MemManagerWrapper;

	typedef internal::UIntMath<size_t> SMath;
	typedef internal::UIntMath<uintptr_t> PMath;

	static const uintptr_t nullPtr = (uintptr_t)nullptr;

public:
	explicit MemPool2(const Params& params = Params(), MemManager&& memManager = MemManager())
		: Params(params),
		MemManagerWrapper(std::move(memManager)),
		mBufferHead(nullPtr),
		mFreeBlockCount(0)
	{
		_CheckParams();
	}

	MemPool2(MemPool2&& memPool) MOMO_NOEXCEPT
		: Params(std::move(memPool._GetParams())),
		MemManagerWrapper(std::move(memPool._GetMemManagerWrapper())),
		mBufferHead(memPool.mBufferHead),
		mFreeBlockCount(memPool.mFreeBlockCount)
	{
		memPool.mBufferHead = nullPtr;
		memPool.mFreeBlockCount = 0;
	}

	MemPool2& operator=(MemPool2&& memPool) MOMO_NOEXCEPT
	{
		MemPool2(std::move(memPool)).Swap(*this);
		return *this;
	}

	void Swap(MemPool2& memPool) MOMO_NOEXCEPT
	{
		std::swap(_GetMemManagerWrapper(), memPool._GetMemManagerWrapper());
		std::swap(_GetParams(), memPool._GetParams());
		std::swap(mBufferHead, memPool.mBufferHead);
		std::swap(mFreeBlockCount, memPool.mFreeBlockCount);
	}

	MOMO_FRIEND_SWAP(MemPool2)

	~MemPool2() MOMO_NOEXCEPT
	{
		if (mBufferHead != nullPtr)
		{
			MOMO_EXTRA_CHECK(_GetPrevBuffer(mBufferHead) == nullPtr);
			MOMO_EXTRA_CHECK(_GetNextBuffer(mBufferHead) == nullPtr);
			MOMO_EXTRA_CHECK(mFreeBlockCount == Params::blockCount);
			_FreeBuffer(mBufferHead);
		}
	}

	size_t GetBlockSize() const MOMO_NOEXCEPT
	{
		return Params::blockSize;
	}

	size_t GetBlockAlignment() const MOMO_NOEXCEPT
	{
		return Params::blockAlignment;
	}

	size_t GetBlockCount() const MOMO_NOEXCEPT
	{
		return Params::blockCount;
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return _GetMemManagerWrapper().GetMemManager();
	}

	void* GetMemory()
	{
		if (mBufferHead == nullPtr)
		{
			mBufferHead = _NewBuffer();
			mFreeBlockCount += Params::blockCount;
		}
		uintptr_t block = _NewBlock(mBufferHead);
		--mFreeBlockCount;
		if ((size_t)_GetFreeBlockCount(mBufferHead) == 0)
			_RemoveBuffer(mBufferHead);
		return (void*)block;
	}

	void FreeMemory(void* pblock) MOMO_NOEXCEPT
	{
		assert(pblock != nullptr);
		uintptr_t block = (uintptr_t)pblock;
		uintptr_t buffer = _GetBuffer(block);
		_FreeBlock(buffer, block);
		++mFreeBlockCount;
		size_t freeBlockCount = (size_t)_GetFreeBlockCount(buffer);
		if (freeBlockCount == Params::blockCount && mFreeBlockCount >= 2 * Params::blockCount)
		{
			_RemoveBuffer(buffer);
			_FreeBuffer(buffer);
			mFreeBlockCount -= Params::blockCount;
		}
		else if (freeBlockCount == 1)
		{
			_GetPrevBuffer(buffer) = nullPtr;
			_GetNextBuffer(buffer) = mBufferHead;
			if (mBufferHead != nullPtr)
				_GetPrevBuffer(mBufferHead) = buffer;
			mBufferHead = buffer;
		}
	}

private:
	Params& _GetParams() MOMO_NOEXCEPT
	{
		return *this;
	}

	const MemManagerWrapper& _GetMemManagerWrapper() const MOMO_NOEXCEPT
	{
		return *this;
	}

	MemManagerWrapper& _GetMemManagerWrapper() MOMO_NOEXCEPT
	{
		return *this;
	}

	void _CheckParams() const
	{
		MOMO_CHECK(0 < Params::blockCount && Params::blockCount < 128);
		MOMO_CHECK(0 < Params::blockAlignment && Params::blockAlignment <= 1024);
		MOMO_CHECK(Params::blockSize % Params::blockAlignment == 0);
		MOMO_CHECK(Params::blockSize / Params::blockAlignment >= 2);
		size_t maxBlockSize = (SIZE_MAX - 2 - 3 * sizeof(void*) - 4 * Params::blockAlignment)
			/ Params::blockCount;
		if (Params::blockSize > maxBlockSize)
			throw std::length_error("momo::MemPool length error");
	}

	uintptr_t _GetBuffer(uintptr_t block) const MOMO_NOEXCEPT
	{
		assert(block % Params::blockAlignment == 0);
		uintptr_t buffer = PMath::Ceil(block, Params::blockSize * Params::blockCount)
			+ (block % Params::blockSize);
		if (((block % Params::blockSize) / Params::blockAlignment) % 2 == 1)
			buffer -= Params::blockSize * Params::blockCount + Params::blockAlignment;
		return buffer;
	}

	uintptr_t _NewBlock(uintptr_t buffer) MOMO_NOEXCEPT
	{
		signed char& firstFreeBlockIndex = _GetFirstFreeBlockIndex(buffer);
		uintptr_t block = _GetBlock(buffer, firstFreeBlockIndex);
		firstFreeBlockIndex = _GetNextFreeBlockIndex(block);
		--_GetFreeBlockCount(buffer);
		return block;
	}

	void _FreeBlock(uintptr_t buffer, uintptr_t block) MOMO_NOEXCEPT
	{
		signed char& firstFreeBlockIndex = _GetFirstFreeBlockIndex(buffer);
		_GetNextFreeBlockIndex(block) = firstFreeBlockIndex;
		firstFreeBlockIndex = _GetBlockIndex(buffer, block);
		++_GetFreeBlockCount(buffer);
	}

	signed char& _GetNextFreeBlockIndex(uintptr_t block) MOMO_NOEXCEPT
	{
		return *(signed char*)block;
	}

	uintptr_t _GetBlock(uintptr_t buffer, signed char index) const MOMO_NOEXCEPT
	{
		if (index < 0)
			return buffer - (size_t)(-index) * Params::blockSize;
		else
			return buffer + (size_t)index * Params::blockSize + Params::blockAlignment;
	}

	signed char _GetBlockIndex(uintptr_t buffer, uintptr_t block) const MOMO_NOEXCEPT
	{
		if (block < buffer)
			return -(signed char)((buffer - block) / Params::blockSize);
		else
			return (signed char)((block - buffer) / Params::blockSize);
	}

	uintptr_t _NewBuffer()
	{
		uintptr_t begin = (uintptr_t)GetMemManager().Allocate(_GetBufferSize());
		uintptr_t block = PMath::Ceil(begin, (uintptr_t)Params::blockAlignment);
		if (((block % Params::blockSize) / Params::blockAlignment) % 2 == 1)
			block += Params::blockAlignment;
		if ((block + Params::blockAlignment) % Params::blockSize == 0)
			block += Params::blockAlignment;
		if (((block / Params::blockSize) % Params::blockCount) == 0)
			block += Params::blockAlignment;
		uintptr_t buffer = _GetBuffer(block);
		signed char index = _GetBlockIndex(buffer, block);
		_GetFirstBlockIndex(buffer) = index;
		_GetFirstFreeBlockIndex(buffer) = index;
		_GetFreeBlockCount(buffer) = (signed char)Params::blockCount;
		_GetPrevBuffer(buffer) = nullPtr;
		_GetNextBuffer(buffer) = nullPtr;
		_GetBufferBegin(buffer) = begin;
		for (size_t i = 1; i < Params::blockCount; ++i)
		{
			++index;
			_GetNextFreeBlockIndex(block) = index;
			block = _GetBlock(buffer, index);
		}
		_GetNextFreeBlockIndex(block) = (signed char)(-128);
		return buffer;
	}

	void _FreeBuffer(uintptr_t buffer) MOMO_NOEXCEPT
	{
		GetMemManager().Deallocate((void*)_GetBufferBegin(buffer), _GetBufferSize());
	}

	void _RemoveBuffer(uintptr_t buffer) MOMO_NOEXCEPT
	{
		uintptr_t prevBuffer = _GetPrevBuffer(buffer);
		uintptr_t nextBuffer = _GetNextBuffer(buffer);
		if (prevBuffer != nullPtr)
			_GetNextBuffer(prevBuffer) = nextBuffer;
		if (nextBuffer != nullPtr)
			_GetPrevBuffer(nextBuffer) = prevBuffer;
		if (mBufferHead == buffer)
			mBufferHead = nextBuffer;
	}

	size_t _GetBufferSize() const MOMO_NOEXCEPT
	{
		size_t bufferUsefulSize = Params::blockCount * Params::blockSize
			+ (3 + (Params::blockSize / Params::blockAlignment) % 2) * Params::blockAlignment;
		static const size_t maxAlignment = std::alignment_of<std::max_align_t>::value;
		if ((Params::blockAlignment & (Params::blockAlignment - 1)) == 0)
			bufferUsefulSize -= std::minmax(maxAlignment, (size_t)Params::blockAlignment).first;	// gcc & llvm
		else
			bufferUsefulSize -= SMath::GCD(maxAlignment, Params::blockAlignment);
		size_t bufferSize = SMath::Ceil(bufferUsefulSize, sizeof(void*)) + 3 * sizeof(void*)
			+ ((Params::blockAlignment <= 2) ? 2 : 0);
		return bufferSize;
	}

	signed char& _GetFreeBlockCount(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return _GetBufferChars(buffer)[0];
	}

	signed char& _GetFirstFreeBlockIndex(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return _GetBufferChars(buffer)[1];
	}

	signed char& _GetFirstBlockIndex(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return *(signed char*)buffer;
	}

	uintptr_t& _GetPrevBuffer(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return _GetBufferPointers(buffer)[0];
	}

	uintptr_t& _GetNextBuffer(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return _GetBufferPointers(buffer)[1];
	}

	uintptr_t& _GetBufferBegin(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return _GetBufferPointers(buffer)[2];
	}

	signed char* _GetBufferChars(uintptr_t buffer) MOMO_NOEXCEPT
	{
		if (Params::blockAlignment > 2)
			return (signed char*)(buffer + 1);
		else
			return (signed char*)(_GetBufferPointers(buffer) + 3);
	}

	uintptr_t* _GetBufferPointers(uintptr_t buffer) MOMO_NOEXCEPT
	{
		size_t offset = Params::blockCount - (size_t)(-_GetFirstBlockIndex(buffer));
		uintptr_t ptrs = buffer + Params::blockAlignment + Params::blockSize * offset;
		ptrs = PMath::Ceil(ptrs, (uintptr_t)sizeof(void*));
		return (uintptr_t*)ptrs;
	}

private:
	MOMO_DISABLE_COPY_CONSTRUCTOR(MemPool2);
	MOMO_DISABLE_COPY_OPERATOR(MemPool2);

private:
	uintptr_t mBufferHead;
	size_t mFreeBlockCount;
};

template<size_t tBlockCount = MemPoolConst::defaultBlockCount,
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
		return internal::UIntMath<size_t>::Ceil(blockCount * mBlockSize, sizeof(void*))
			+ sizeof(void*);
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
