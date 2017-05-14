/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/MemPool.h

  namespace momo:
    struct MemPoolConst
    class MemPoolParams
    class MemPoolParamsStatic
    struct MemPoolSettings
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
	static const size_t defaultCachedFreeBlockCount = MOMO_DEFAULT_MEM_POOL_CACHED_FREE_BLOCK_COUNT;

	template<size_t blockSize, size_t maxAlignment = MOMO_MAX_ALIGNMENT>
	struct BlockAlignmenter
	{
		static const size_t blockAlignment = (blockSize >= maxAlignment)
			? maxAlignment : BlockAlignmenter<blockSize, maxAlignment / 2>::blockAlignment;
	};

	template<size_t blockSize>
	struct BlockAlignmenter<blockSize, 1>
	{
		static const size_t blockAlignment = 1;
	};
};

template<size_t tBlockCount = MemPoolConst::defaultBlockCount,
	size_t tCachedFreeBlockCount = MemPoolConst::defaultCachedFreeBlockCount>
class MemPoolParams
{
public:
	static const size_t blockCount = tBlockCount;
	MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;

public:
	explicit MemPoolParams(size_t blockSize) MOMO_NOEXCEPT
		: blockSize(blockSize),
		blockAlignment(MOMO_MAX_ALIGNMENT)
	{
		while (blockAlignment > blockSize && blockAlignment > 1)
			blockAlignment /= 2;
		pvCorrectBlockSize();
	}

	MemPoolParams(size_t blockSize, size_t blockAlignment) MOMO_NOEXCEPT
		: blockSize(blockSize),
		blockAlignment(blockAlignment)
	{
		MOMO_ASSERT(blockAlignment > 0);
		pvCorrectBlockSize();
	}

private:
	void pvCorrectBlockSize() MOMO_NOEXCEPT
	{
		if (blockCount == 1)
		{
			if (blockSize == 0)
				blockSize = 1;
		}
		else
		{
			blockSize = (blockSize <= blockAlignment) ? 2 * blockAlignment
				: internal::UIntMath<size_t>::Ceil(blockSize, blockAlignment);
		}
	}

protected:
	size_t blockSize;
	size_t blockAlignment;
};

template<size_t tBlockSize,
	size_t tBlockAlignment = MemPoolConst::BlockAlignmenter<tBlockSize>::blockAlignment,
	size_t tBlockCount = MemPoolConst::defaultBlockCount,
	size_t tCachedFreeBlockCount = MemPoolConst::defaultCachedFreeBlockCount>
class MemPoolParamsStatic
{
public:
	static const size_t blockCount = tBlockCount;
	MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

	static const size_t blockAlignment = tBlockAlignment;
	MOMO_STATIC_ASSERT(0 < blockAlignment && blockAlignment <= 1024);

	static const size_t blockSize = (blockCount == 1)
		? ((tBlockSize > 0) ? tBlockSize : 1)
		: ((tBlockSize <= blockAlignment)
			? 2 * blockAlignment
			: ((tBlockSize - 1) / blockAlignment + 1) * blockAlignment);

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;
};

struct MemPoolSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
};

template<typename TParams = MemPoolParams<>,
	typename TMemManager = MemManagerDefault,
	typename TSettings = MemPoolSettings>
class MemPool : private TParams, private internal::MemManagerWrapper<TMemManager>
{
public:
	typedef TParams Params;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<Params>::value);
	MOMO_STATIC_ASSERT(std::is_nothrow_move_assignable<Params>::value);

private:
	typedef internal::MemManagerWrapper<MemManager> MemManagerWrapper;

	typedef internal::NestedArrayIntCap<Params::cachedFreeBlockCount, void*,
		internal::MemManagerDummy> CachedFreeBlocks;

	typedef internal::UIntMath<size_t> SMath;
	typedef internal::UIntMath<uintptr_t> PMath;

	struct BufferChars
	{
		signed char firstFreeBlockIndex;
		signed char freeBlockCount;
	};

	struct BufferPointers
	{
		uintptr_t prevBuffer;
		uintptr_t nextBuffer;
		uintptr_t begin;
	};

	static const uintptr_t nullPtr = internal::UIntPtrConst::null;

	static const size_t maxAlignment = std::alignment_of<std::max_align_t>::value;
	MOMO_STATIC_ASSERT((maxAlignment & (maxAlignment - 1)) == 0);

public:
	explicit MemPool(MemManager&& memManager = MemManager())
		: MemPool(Params(), std::move(memManager))
	{
	}

	explicit MemPool(const Params& params, MemManager&& memManager = MemManager())
		: Params(params),
		MemManagerWrapper((pvCheckParams(), std::move(memManager))),
		mBufferHead(nullPtr),
		mAllocCount(0)
	{
	}

	MemPool(MemPool&& memPool) MOMO_NOEXCEPT
		: Params(std::move(memPool.pvGetParams())),
		MemManagerWrapper(std::move(memPool.pvGetMemManagerWrapper())),
		mBufferHead(memPool.mBufferHead),
		mAllocCount(memPool.mAllocCount),
		mCachedFreeBlocks(std::move(memPool.mCachedFreeBlocks))
	{
		memPool.mBufferHead = nullPtr;
		memPool.mAllocCount = 0;
	}

	MemPool(const MemPool&) = delete;

	~MemPool() MOMO_NOEXCEPT
	{
		MOMO_EXTRA_CHECK(mAllocCount == 0);
		if (Params::cachedFreeBlockCount > 0)
			pvFlushDeallocate();
		MOMO_EXTRA_CHECK(mBufferHead == nullPtr);
	}

	MemPool& operator=(MemPool&& memPool) MOMO_NOEXCEPT
	{
		MemPool(std::move(memPool)).Swap(*this);
		return *this;
	}

	MemPool& operator=(const MemPool&) = delete;

	void Swap(MemPool& memPool) MOMO_NOEXCEPT
	{
		std::swap(pvGetMemManagerWrapper(), memPool.pvGetMemManagerWrapper());
		std::swap(pvGetParams(), memPool.pvGetParams());
		std::swap(mBufferHead, memPool.mBufferHead);
		std::swap(mAllocCount, memPool.mAllocCount);
		mCachedFreeBlocks.Swap(memPool.mCachedFreeBlocks);
	}

	MOMO_FRIEND_SWAP(MemPool)

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
		return pvGetMemManagerWrapper().GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return pvGetMemManagerWrapper().GetMemManager();
	}

	template<typename ResType = void>
	ResType* Allocate()
	{
		ResType* pblock;
		if (Params::cachedFreeBlockCount > 0 && mCachedFreeBlocks.GetCount() > 0)
		{
			pblock = static_cast<ResType*>(mCachedFreeBlocks.GetBackItem());
			mCachedFreeBlocks.RemoveBack();
		}
		else
		{
			if (Params::blockCount > 1)
				pblock = reinterpret_cast<ResType*>(pvNewBlock());
			else if (maxAlignment % Params::blockAlignment == 0)
				pblock = GetMemManager().template Allocate<ResType>(pvGetBufferSize0());
			else
				pblock = reinterpret_cast<ResType*>(pvNewBlock1());
		}
		++mAllocCount;
		return pblock;
	}

	void Deallocate(void* pblock) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(pblock != nullptr);
		MOMO_ASSERT(mAllocCount > 0);
		if (Params::cachedFreeBlockCount > 0)
		{
			if (mCachedFreeBlocks.GetCount() == Params::cachedFreeBlockCount)
				pvFlushDeallocate();
			mCachedFreeBlocks.AddBackNogrow(pblock);
		}
		else
		{
			pvDeleteBlock(pblock);
		}
		--mAllocCount;
	}

private:
	Params& pvGetParams() MOMO_NOEXCEPT
	{
		return *this;
	}

	const MemManagerWrapper& pvGetMemManagerWrapper() const MOMO_NOEXCEPT
	{
		return *this;
	}

	MemManagerWrapper& pvGetMemManagerWrapper() MOMO_NOEXCEPT
	{
		return *this;
	}

	void pvCheckParams() const
	{
		MOMO_CHECK(0 < Params::blockCount && Params::blockCount < 128);
		MOMO_CHECK(0 < Params::blockAlignment && Params::blockAlignment <= 1024);
		MOMO_CHECK(Params::blockSize > 0);
		MOMO_CHECK(Params::blockCount == 1 || Params::blockSize % Params::blockAlignment == 0);
		MOMO_CHECK(Params::blockCount == 1 || Params::blockSize / Params::blockAlignment >= 2);
		size_t maxBlockSize =
			(SIZE_MAX - 2 - 3 * sizeof(void*) - 4 * Params::blockAlignment) / Params::blockCount;
		if (Params::blockSize > maxBlockSize)
			throw std::length_error("momo::MemPool length error");
	}

	void pvFlushDeallocate() MOMO_NOEXCEPT
	{
		for (void* pblock : mCachedFreeBlocks)
			pvDeleteBlock(pblock);
		mCachedFreeBlocks.Clear();
	}

	void pvDeleteBlock(void* pblock) MOMO_NOEXCEPT
	{
		if (Params::blockCount > 1)
			pvDeleteBlock(reinterpret_cast<uintptr_t>(pblock));
		else if (maxAlignment % Params::blockAlignment == 0)
			GetMemManager().Deallocate(pblock, pvGetBufferSize0());
		else
			pvDeleteBlock1(reinterpret_cast<uintptr_t>(pblock));
	}

	size_t pvGetBufferSize0() const MOMO_NOEXCEPT
	{
		return std::minmax((size_t)Params::blockSize, (size_t)Params::blockAlignment).second;	// gcc & llvm
	}

	uintptr_t pvNewBlock1()
	{
		uintptr_t begin = reinterpret_cast<uintptr_t>(GetMemManager().Allocate(pvGetBufferSize1()));
		uintptr_t block = PMath::Ceil(begin, (uintptr_t)Params::blockAlignment);
		pvGetBufferBegin1(block) = begin;
		return block;
	}

	void pvDeleteBlock1(uintptr_t block) MOMO_NOEXCEPT
	{
		uintptr_t begin = pvGetBufferBegin1(block);
		GetMemManager().Deallocate(reinterpret_cast<void*>(begin), pvGetBufferSize1());
	}

	size_t pvGetBufferSize1() const MOMO_NOEXCEPT
	{
		size_t bufferUsefulSize = Params::blockSize + Params::blockAlignment
			- SMath::GCD(maxAlignment, Params::blockAlignment);
		return SMath::Ceil(bufferUsefulSize, sizeof(void*)) + sizeof(void*);
	}

	uintptr_t& pvGetBufferBegin1(uintptr_t block) MOMO_NOEXCEPT
	{
		return *reinterpret_cast<uintptr_t*>(PMath::Ceil(block + Params::blockSize,
			(uintptr_t)sizeof(void*)));
	}

	uintptr_t pvNewBlock()
	{
		if (mBufferHead == nullPtr)
			mBufferHead = pvNewBuffer();
		uintptr_t block;
		size_t freeBlockCount = pvNewBlock(mBufferHead, block);
		if (freeBlockCount == 0)
			pvRemoveBuffer(mBufferHead, false);
		return block;
	}

	void pvDeleteBlock(uintptr_t block) MOMO_NOEXCEPT
	{
		uintptr_t buffer;
		size_t freeBlockCount = pvDeleteBlock(block, buffer);
		if (freeBlockCount == 1)
		{
			BufferPointers& pointers = pvGetBufferPointers(buffer);
			pointers.prevBuffer = nullPtr;
			pointers.nextBuffer = mBufferHead;
			if (mBufferHead != nullPtr)
				pvGetBufferPointers(mBufferHead).prevBuffer = buffer;
			mBufferHead = buffer;
		}
		if (freeBlockCount == Params::blockCount)
			pvRemoveBuffer(buffer, true);
	}

	size_t pvNewBlock(uintptr_t buffer, uintptr_t& block) MOMO_NOEXCEPT
	{
		BufferChars& chars = pvGetBufferChars(buffer);
		block = pvGetNextFreeBlockIndex(buffer, chars.firstFreeBlockIndex);
		chars.firstFreeBlockIndex = pvGetNextFreeBlockIndex(block);
		--chars.freeBlockCount;
		return (size_t)chars.freeBlockCount;
	}

	size_t pvDeleteBlock(uintptr_t block, uintptr_t& buffer) MOMO_NOEXCEPT
	{
		signed char blockIndex = pvGetBlockIndex(block, buffer);
		BufferChars& chars = pvGetBufferChars(buffer);
		pvGetNextFreeBlockIndex(block) = chars.firstFreeBlockIndex;
		chars.firstFreeBlockIndex = blockIndex;
		++chars.freeBlockCount;
		return (size_t)chars.freeBlockCount;
	}

	signed char& pvGetNextFreeBlockIndex(uintptr_t block) MOMO_NOEXCEPT
	{
		return *reinterpret_cast<signed char*>(block);
	}

	uintptr_t pvGetNextFreeBlockIndex(uintptr_t buffer, signed char index) const MOMO_NOEXCEPT
	{
		return buffer + (intptr_t)index * (intptr_t)Params::blockSize
			+ ((intptr_t)Params::blockAlignment & -(intptr_t)(index >= 0));
	}

	signed char pvGetBlockIndex(uintptr_t block, uintptr_t& buffer) const MOMO_NOEXCEPT
	{
		MOMO_ASSERT(block % Params::blockAlignment == 0);
		size_t index = (block / Params::blockSize) % Params::blockCount;
		if (((block % Params::blockSize) / Params::blockAlignment) % 2 == 1)
		{
			buffer = block - index * Params::blockSize - Params::blockAlignment;
			return (signed char)index;
		}
		else
		{
			buffer = block + (Params::blockCount - index) * Params::blockSize;
			return (signed char)index - (signed char)Params::blockCount;
		}
	}

	uintptr_t pvNewBuffer()
	{
		uintptr_t begin = reinterpret_cast<uintptr_t>(GetMemManager().Allocate(pvGetBufferSize()));
		uintptr_t block = PMath::Ceil(begin, (uintptr_t)Params::blockAlignment);
		block += (block % Params::blockSize) % (2 * Params::blockAlignment);
		if ((block + Params::blockAlignment) % Params::blockSize == 0)
			block += Params::blockAlignment;
		if (((block / Params::blockSize) % Params::blockCount) == 0)
			block += Params::blockAlignment;
		uintptr_t buffer;
		signed char blockIndex = pvGetBlockIndex(block, buffer);
		pvGetFirstBlockIndex(buffer) = blockIndex;
		BufferChars& chars = pvGetBufferChars(buffer);
		chars.firstFreeBlockIndex = blockIndex;
		chars.freeBlockCount = (signed char)Params::blockCount;
		BufferPointers& pointers = pvGetBufferPointers(buffer);
		pointers.prevBuffer = nullPtr;
		pointers.nextBuffer = nullPtr;
		pointers.begin = begin;
		for (size_t i = 1; i < Params::blockCount; ++i)
		{
			++blockIndex;
			pvGetNextFreeBlockIndex(block) = blockIndex;
			block = pvGetNextFreeBlockIndex(buffer, blockIndex);
		}
		pvGetNextFreeBlockIndex(block) = (signed char)(-128);
		return buffer;
	}

	void pvRemoveBuffer(uintptr_t buffer, bool deallocate) MOMO_NOEXCEPT
	{
		BufferPointers& pointers = pvGetBufferPointers(buffer);
		if (pointers.prevBuffer != nullPtr)
			pvGetBufferPointers(pointers.prevBuffer).nextBuffer = pointers.nextBuffer;
		if (pointers.nextBuffer != nullPtr)
			pvGetBufferPointers(pointers.nextBuffer).prevBuffer = pointers.prevBuffer;
		if (mBufferHead == buffer)
			mBufferHead = pointers.nextBuffer;
		if (deallocate)
			GetMemManager().Deallocate(reinterpret_cast<void*>(pointers.begin), pvGetBufferSize());
	}

	size_t pvGetBufferSize() const MOMO_NOEXCEPT
	{
		size_t bufferUsefulSize = Params::blockCount * Params::blockSize
			+ (3 + (Params::blockSize / Params::blockAlignment) % 2) * Params::blockAlignment;
		if ((Params::blockAlignment & (Params::blockAlignment - 1)) == 0)
			bufferUsefulSize -= std::minmax((size_t)maxAlignment, (size_t)Params::blockAlignment).first;	// gcc & llvm
		else
			bufferUsefulSize -= SMath::GCD(maxAlignment, Params::blockAlignment);
		return SMath::Ceil(bufferUsefulSize, sizeof(void*)) + 3 * sizeof(void*)
			+ ((Params::blockAlignment <= 2) ? 2 : 0);
	}

	signed char& pvGetFirstBlockIndex(uintptr_t buffer) MOMO_NOEXCEPT
	{
		return *reinterpret_cast<signed char*>(buffer);
	}

	BufferChars& pvGetBufferChars(uintptr_t buffer) MOMO_NOEXCEPT
	{
		if (Params::blockAlignment > 2)
			return *reinterpret_cast<BufferChars*>(buffer + 1);
		else
			return *reinterpret_cast<BufferChars*>(&pvGetBufferPointers(buffer) + 1);
	}

	BufferPointers& pvGetBufferPointers(uintptr_t buffer) MOMO_NOEXCEPT
	{
		size_t offset = Params::blockCount - (size_t)(-pvGetFirstBlockIndex(buffer));
		return *reinterpret_cast<BufferPointers*>(PMath::Ceil(buffer + Params::blockAlignment
			+ Params::blockSize * offset, (uintptr_t)sizeof(void*)));
	}

private:
	uintptr_t mBufferHead;
	size_t mAllocCount;
	CachedFreeBlocks mCachedFreeBlocks;
};

namespace internal
{
	struct NestedMemPoolSettings
	{
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
	};

	template<size_t tBlockCount, typename TMemManager>
	class MemPoolUInt32
	{
	public:
		typedef TMemManager MemManager;

		static const size_t blockCount = tBlockCount;
		MOMO_STATIC_ASSERT(blockCount > 0);

		static const uint32_t nullPtr = UINT32_MAX;

	private:
		typedef Array<char*, MemManager, ArrayItemTraits<char*, MemManager>,
			NestedArraySettings<>> Buffers;

	public:
		MemPoolUInt32(size_t blockSize, MemManager&& memManager, size_t maxTotalBlockCount)
			: mBuffers(std::move(memManager)),
			mBlockHead(nullPtr),
			mMaxBufferCount(maxTotalBlockCount / blockCount),
			mBlockSize(internal::UIntMath<size_t>::Ceil(blockSize, sizeof(uint32_t))),
			mAllocCount(0)
		{
			MOMO_ASSERT(maxTotalBlockCount < (size_t)UINT32_MAX);
			if (mBlockSize > SIZE_MAX / blockCount)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
		}

		MemPoolUInt32(const MemPoolUInt32&) = delete;

		~MemPoolUInt32() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mAllocCount == 0);
			pvClear();
		}

		MemPoolUInt32& operator=(const MemPoolUInt32&) = delete;

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return mBuffers.GetMemManager();
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return mBuffers.GetMemManager();
		}

		template<typename ResType = void>
		ResType* GetRealPointer(uint32_t ptr) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(ptr != nullPtr);
			char* buffer = mBuffers[ptr / blockCount];
			void* realPtr = buffer + (size_t)(ptr % blockCount) * mBlockSize;
			return static_cast<ResType*>(realPtr);
		}

		uint32_t Allocate()
		{
			if (mBlockHead == nullPtr)
				pvNewBuffer();
			uint32_t ptr = mBlockHead;
			mBlockHead = pvGetNextBlock(GetRealPointer(mBlockHead));
			++mAllocCount;
			return ptr;
		}

		void Deallocate(uint32_t ptr) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(ptr != nullPtr);
			MOMO_ASSERT(mAllocCount > 0);
			pvGetNextBlock(GetRealPointer(ptr)) = mBlockHead;
			mBlockHead = ptr;
			--mAllocCount;
			if (mAllocCount == 0)
				pvShrink();
		}

	private:
		uint32_t& pvGetNextBlock(void* realPtr) MOMO_NOEXCEPT
		{
			return *static_cast<uint32_t*>(realPtr);
		}

		void pvNewBuffer()
		{
			size_t bufferCount = mBuffers.GetCount();
			if (bufferCount >= mMaxBufferCount)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
			mBuffers.Reserve(bufferCount + 1);
			char* buffer = GetMemManager().template Allocate<char>(pvGetBufferSize());
			for (size_t i = 0; i < blockCount; ++i)
			{
				void* realPtr = buffer + mBlockSize * i;
				pvGetNextBlock(realPtr) = (i + 1 < blockCount)
					? (uint32_t)(bufferCount * blockCount + i + 1) : nullPtr;
			}
			mBlockHead = (uint32_t)(bufferCount * blockCount);
			mBuffers.AddBackNogrow(buffer);
		}

		void pvShrink() MOMO_NOEXCEPT
		{
			if (mBuffers.GetCount() > 2)
			{
				pvClear();
				mBlockHead = nullPtr;
				mBuffers.Clear(true);
			}
		}

		void pvClear() MOMO_NOEXCEPT
		{
			MemManager& memManager = GetMemManager();
			size_t bufferSize = pvGetBufferSize();
			for (char* buffer : mBuffers)
				memManager.Deallocate(buffer, bufferSize);
		}

		size_t pvGetBufferSize() const MOMO_NOEXCEPT
		{
			return blockCount * mBlockSize;
		}

	private:
		Buffers mBuffers;
		uint32_t mBlockHead;
		size_t mMaxBufferCount;
		size_t mBlockSize;
		size_t mAllocCount;
	};
}

} // namespace momo
