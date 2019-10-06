/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MemPool.h

  namespace momo:
    class MemPoolConst
    class MemPoolParams
    class MemPoolParamsStatic
    class MemPoolSettings
    class MemPool

\**********************************************************/

#pragma once

#include "MemManager.h"
#include "Array.h"

namespace momo
{

class MemPoolConst
{
public:
	static const size_t defaultBlockCount = MOMO_DEFAULT_MEM_POOL_BLOCK_COUNT;
	static const size_t defaultCachedFreeBlockCount = MOMO_DEFAULT_MEM_POOL_CACHED_FREE_BLOCK_COUNT;

public:
	static constexpr size_t GetBlockAlignment(size_t blockSize,
		size_t maxAlignment = MOMO_MAX_ALIGNMENT) noexcept
	{
		return (maxAlignment > blockSize && maxAlignment > 1)
			? GetBlockAlignment(blockSize, maxAlignment / 2) : maxAlignment;
	}
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
	explicit MemPoolParams(size_t blockSize) noexcept
		: blockSize(blockSize),
		blockAlignment(MOMO_MAX_ALIGNMENT)
	{
		while (blockAlignment > blockSize && blockAlignment > 1)
			blockAlignment /= 2;
		pvCorrectBlockSize();
	}

	explicit MemPoolParams(size_t blockSize, size_t blockAlignment) noexcept
		: blockSize(blockSize),
		blockAlignment(blockAlignment)
	{
		MOMO_ASSERT(blockAlignment > 0);
		pvCorrectBlockSize();
	}

	bool IsEqual(const MemPoolParams& params) const noexcept
	{
		return blockSize == params.blockSize && blockAlignment == params.blockAlignment;
	}

private:
	void pvCorrectBlockSize() noexcept
	{
		if (blockCount == 1)
		{
			if (blockSize == 0)
				blockSize = 1;
		}
		else
		{
			blockSize = (blockSize <= blockAlignment) ? 2 * blockAlignment
				: internal::UIntMath<>::Ceil(blockSize, blockAlignment);
		}
	}

protected:
	size_t blockSize;
	size_t blockAlignment;
};

template<size_t tBlockSize,
	size_t tBlockAlignment = MemPoolConst::GetBlockAlignment(tBlockSize),
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
		: ((tBlockSize <= blockAlignment) ? 2 * blockAlignment
			: internal::UIntMath<>::Ceil(tBlockSize, blockAlignment));

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;

public:
	explicit MemPoolParamsStatic() noexcept
	{
	}

	bool IsEqual(const MemPoolParamsStatic& /*params*/) const noexcept
	{
		return true;
	}
};

class MemPoolSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
};

template<typename TParams = MemPoolParams<>,
	typename TMemManager = MemManagerDefault,
	typename TSettings = MemPoolSettings>
class MemPool : private TParams
{
public:
	typedef TParams Params;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<Params>::value);
	MOMO_STATIC_ASSERT(std::is_nothrow_move_assignable<Params>::value);

private:
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef internal::NestedArrayIntCap<Params::cachedFreeBlockCount, void*,
		MemManager> CachedFreeBlocks;

	typedef internal::UIntMath<size_t> SMath;
	typedef internal::UIntMath<uintptr_t> PMath;

	struct BufferBytes
	{
		int8_t firstFreeBlockIndex;
		int8_t freeBlockCount;
	};

	struct BufferPointers
	{
		uintptr_t prevBuffer;
		uintptr_t nextBuffer;
		uintptr_t begin;
	};

	static const uintptr_t nullPtr = internal::UIntPtrConst::null;

	static const size_t maxAlignment = alignof(std::max_align_t);
	MOMO_STATIC_ASSERT((maxAlignment & (maxAlignment - 1)) == 0);

public:
	explicit MemPool(MemManager&& memManager = MemManager())
		: MemPool(Params(), std::move(memManager))
	{
	}

	explicit MemPool(const Params& params, MemManager&& memManager = MemManager())
		: Params(params),
		mBufferHead((pvCheckParams(), nullPtr)),	// check params before move memManager
		mAllocCount(0),
		mCachedFreeBlocks(std::move(memManager))
	{
	}

	MemPool(MemPool&& memPool) noexcept
		: Params(std::move(memPool.pvGetParams())),
		mBufferHead(memPool.mBufferHead),
		mAllocCount(memPool.mAllocCount),
		mCachedFreeBlocks(std::move(memPool.mCachedFreeBlocks))
	{
		memPool.mBufferHead = nullPtr;
		memPool.mAllocCount = 0;
	}

	MemPool(const MemPool&) = delete;

	~MemPool() noexcept
	{
		MOMO_EXTRA_CHECK(mAllocCount == 0);
		if (Params::cachedFreeBlockCount > 0)
			pvFlushDeallocate();
		MOMO_EXTRA_CHECK(mBufferHead == nullPtr);
	}

	MemPool& operator=(MemPool&& memPool) noexcept
	{
		MemPool(std::move(memPool)).Swap(*this);
		return *this;
	}

	MemPool& operator=(const MemPool&) = delete;

	void Swap(MemPool& memPool) noexcept
	{
		std::swap(pvGetParams(), memPool.pvGetParams());
		std::swap(mBufferHead, memPool.mBufferHead);
		std::swap(mAllocCount, memPool.mAllocCount);
		mCachedFreeBlocks.Swap(memPool.mCachedFreeBlocks);
	}

	MOMO_FRIEND_SWAP(MemPool)

	size_t GetBlockSize() const noexcept
	{
		return Params::blockSize;
	}

	size_t GetBlockAlignment() const noexcept
	{
		return Params::blockAlignment;
	}

	size_t GetBlockCount() const noexcept
	{
		return Params::blockCount;
	}

	const Params& GetParams() const noexcept
	{
		return *this;
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mCachedFreeBlocks.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mCachedFreeBlocks.GetMemManager();
	}

	template<typename ResObject = void>
	ResObject* Allocate()
	{
		void* pblock;
		if (Params::cachedFreeBlockCount > 0 && mCachedFreeBlocks.GetCount() > 0)
		{
			pblock = mCachedFreeBlocks.GetBackItem();
			mCachedFreeBlocks.RemoveBack();
		}
		else
		{
			if (Params::blockCount > 1)
				pblock = internal::BitCaster::ToPtr(pvNewBlock());
			else if (maxAlignment % Params::blockAlignment == 0)
				pblock = MemManagerProxy::Allocate(GetMemManager(), pvGetBufferSize0());
			else
				pblock = internal::BitCaster::ToPtr(pvNewBlock1());
		}
		++mAllocCount;
		return static_cast<ResObject*>(pblock);
	}

	void Deallocate(void* pblock) noexcept
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

	size_t GetAllocateCount() const noexcept
	{
		return mAllocCount;
	}

	void MergeFrom(MemPool& memPool)
	{
		if (this == &memPool)
			return;
		MOMO_CHECK(MemManagerProxy::IsEqual(GetMemManager(), memPool.GetMemManager()));
		MOMO_CHECK(static_cast<const Params&>(*this).IsEqual(memPool));
		if (Params::cachedFreeBlockCount > 0)
			memPool.pvFlushDeallocate();
		mAllocCount += memPool.mAllocCount;
		memPool.mAllocCount = 0;
		if (memPool.mBufferHead == nullPtr)
			return;
		if (mBufferHead == nullPtr)
		{
			std::swap(mBufferHead, memPool.mBufferHead);
			return;
		}
		uintptr_t buffer = mBufferHead;
		while (true)
		{
			BufferPointers& pointers = pvGetBufferPointers(buffer);
			if (pointers.nextBuffer == nullPtr)
				break;
			buffer = pointers.nextBuffer;
		}
		pvGetBufferPointers(buffer).nextBuffer = memPool.mBufferHead;
		pvGetBufferPointers(memPool.mBufferHead).prevBuffer = buffer;
		memPool.mBufferHead = nullPtr;
	}

private:
	Params& pvGetParams() noexcept
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

	void pvFlushDeallocate() noexcept
	{
		for (void* pblock : mCachedFreeBlocks)
			pvDeleteBlock(pblock);
		mCachedFreeBlocks.Clear();
	}

	void pvDeleteBlock(void* pblock) noexcept
	{
		if (Params::blockCount > 1)
			pvDeleteBlock(internal::BitCaster::ToUInt(pblock));
		else if (maxAlignment % Params::blockAlignment == 0)
			MemManagerProxy::Deallocate(GetMemManager(), pblock, pvGetBufferSize0());
		else
			pvDeleteBlock1(internal::BitCaster::ToUInt(pblock));
	}

	size_t pvGetBufferSize0() const noexcept
	{
		return std::minmax(size_t{Params::blockSize}, size_t{Params::blockAlignment}).second;
	}

	uintptr_t pvNewBlock1()
	{
		uintptr_t begin = internal::BitCaster::ToUInt(MemManagerProxy::Allocate(GetMemManager(),
			pvGetBufferSize1()));
		uintptr_t block = PMath::Ceil(begin, uintptr_t{Params::blockAlignment});
		pvGetBufferBegin1(block) = begin;
		return block;
	}

	void pvDeleteBlock1(uintptr_t block) noexcept
	{
		uintptr_t begin = pvGetBufferBegin1(block);
		MemManagerProxy::Deallocate(GetMemManager(), internal::BitCaster::ToPtr(begin),
			pvGetBufferSize1());
	}

	size_t pvGetBufferSize1() const noexcept
	{
		size_t bufferUsefulSize = Params::blockSize + Params::blockAlignment
			- SMath::GCD(maxAlignment, Params::blockAlignment);
		return SMath::Ceil(bufferUsefulSize, sizeof(void*)) + sizeof(void*);
	}

	uintptr_t& pvGetBufferBegin1(uintptr_t block) noexcept
	{
		return *internal::BitCaster::ToPtr<uintptr_t>(PMath::Ceil(block + Params::blockSize,
			uintptr_t{sizeof(void*)}));
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

	void pvDeleteBlock(uintptr_t block) noexcept
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

	size_t pvNewBlock(uintptr_t buffer, uintptr_t& block) noexcept
	{
		BufferBytes& bytes = pvGetBufferBytes(buffer);
		block = pvGetNextFreeBlockIndex(buffer, bytes.firstFreeBlockIndex);
		bytes.firstFreeBlockIndex = pvGetNextFreeBlockIndex(block);
		--bytes.freeBlockCount;
		return static_cast<size_t>(bytes.freeBlockCount);
	}

	size_t pvDeleteBlock(uintptr_t block, uintptr_t& buffer) noexcept
	{
		int8_t blockIndex = pvGetBlockIndex(block, buffer);
		BufferBytes& bytes = pvGetBufferBytes(buffer);
		pvGetNextFreeBlockIndex(block) = bytes.firstFreeBlockIndex;
		bytes.firstFreeBlockIndex = blockIndex;
		++bytes.freeBlockCount;
		return static_cast<size_t>(bytes.freeBlockCount);
	}

	int8_t& pvGetNextFreeBlockIndex(uintptr_t block) noexcept
	{
		return *internal::BitCaster::ToPtr<int8_t>(block);
	}

	uintptr_t pvGetNextFreeBlockIndex(uintptr_t buffer, int8_t index) const noexcept
	{
		return static_cast<uintptr_t>(static_cast<intptr_t>(buffer)
			+ intptr_t{index} * static_cast<intptr_t>(Params::blockSize)
			+ (static_cast<intptr_t>(Params::blockAlignment) & -intptr_t{index >= 0}));
	}

	int8_t pvGetBlockIndex(uintptr_t block, uintptr_t& buffer) const noexcept
	{
		MOMO_ASSERT(block % Params::blockAlignment == 0);
		size_t index = (block / Params::blockSize) % Params::blockCount;
		if (((block % Params::blockSize) / Params::blockAlignment) % 2 == 1)
		{
			buffer = block - index * Params::blockSize - Params::blockAlignment;
			return static_cast<int8_t>(index);
		}
		else
		{
			buffer = block + (Params::blockCount - index) * Params::blockSize;
			return static_cast<int8_t>(index) - static_cast<int8_t>(Params::blockCount);
		}
	}

	uintptr_t pvNewBuffer()
	{
		uintptr_t begin = internal::BitCaster::ToUInt(MemManagerProxy::Allocate(GetMemManager(),
			pvGetBufferSize()));
		uintptr_t block = PMath::Ceil(begin, uintptr_t{Params::blockAlignment});
		block += (block % Params::blockSize) % (2 * Params::blockAlignment);
		if ((block + Params::blockAlignment) % Params::blockSize == 0)
			block += Params::blockAlignment;
		if (((block / Params::blockSize) % Params::blockCount) == 0)
			block += Params::blockAlignment;
		uintptr_t buffer;
		int8_t blockIndex = pvGetBlockIndex(block, buffer);
		pvGetFirstBlockIndex(buffer) = blockIndex;
		BufferBytes& bytes = pvGetBufferBytes(buffer);
		bytes.firstFreeBlockIndex = blockIndex;
		bytes.freeBlockCount = static_cast<int8_t>(Params::blockCount);
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
		pvGetNextFreeBlockIndex(block) = int8_t{-128};
		return buffer;
	}

	void pvRemoveBuffer(uintptr_t buffer, bool deallocate) noexcept
	{
		BufferPointers& pointers = pvGetBufferPointers(buffer);
		if (pointers.prevBuffer != nullPtr)
			pvGetBufferPointers(pointers.prevBuffer).nextBuffer = pointers.nextBuffer;
		if (pointers.nextBuffer != nullPtr)
			pvGetBufferPointers(pointers.nextBuffer).prevBuffer = pointers.prevBuffer;
		if (mBufferHead == buffer)
			mBufferHead = pointers.nextBuffer;
		if (deallocate)
		{
			MemManagerProxy::Deallocate(GetMemManager(),
				internal::BitCaster::ToPtr(pointers.begin), pvGetBufferSize());
		}
	}

	size_t pvGetBufferSize() const noexcept
	{
		size_t bufferUsefulSize = Params::blockCount * Params::blockSize
			+ (3 + (Params::blockSize / Params::blockAlignment) % 2) * Params::blockAlignment;
		if ((Params::blockAlignment & (Params::blockAlignment - 1)) == 0)
			bufferUsefulSize -= std::minmax(size_t{maxAlignment}, size_t{Params::blockAlignment}).first;
		else
			bufferUsefulSize -= SMath::GCD(maxAlignment, Params::blockAlignment);
		return SMath::Ceil(bufferUsefulSize, sizeof(void*)) + 3 * sizeof(void*)
			+ ((Params::blockAlignment <= 2) ? 2 : 0);
	}

	int8_t& pvGetFirstBlockIndex(uintptr_t buffer) noexcept
	{
		return *internal::BitCaster::ToPtr<int8_t>(buffer);
	}

	BufferBytes& pvGetBufferBytes(uintptr_t buffer) noexcept
	{
		if (Params::blockAlignment > 2)
		{
			return *internal::BitCaster::ToPtr<BufferBytes>(buffer + 1);
		}
		else
		{
			return *internal::BitCaster::PtrToPtr<BufferBytes>(
				&pvGetBufferPointers(buffer), sizeof(BufferPointers));
		}
	}

	BufferPointers& pvGetBufferPointers(uintptr_t buffer) noexcept
	{
		size_t offset = size_t{Params::blockCount}	// gcc
			- static_cast<size_t>(-pvGetFirstBlockIndex(buffer));
		return *internal::BitCaster::ToPtr<BufferPointers>(PMath::Ceil(buffer +
			Params::blockAlignment + Params::blockSize * offset, uintptr_t{sizeof(void*)}));
	}

private:
	uintptr_t mBufferHead;
	size_t mAllocCount;
	CachedFreeBlocks mCachedFreeBlocks;
};

namespace internal
{
	class NestedMemPoolSettings
	{
	public:
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
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		typedef Array<char*, MemManager, ArrayItemTraits<char*, MemManager>,
			NestedArraySettings<>> Buffers;

	public:
		explicit MemPoolUInt32(size_t blockSize, MemManager&& memManager, size_t maxTotalBlockCount)
			: mBuffers(std::move(memManager)),
			mBlockHead(nullPtr),
			mMaxBufferCount(maxTotalBlockCount / blockCount),
			mBlockSize(internal::UIntMath<>::Ceil(blockSize, sizeof(uint32_t))),
			mAllocCount(0)
		{
			MOMO_ASSERT(maxTotalBlockCount < size_t{UINT32_MAX});
			if (mBlockSize > SIZE_MAX / blockCount)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
		}

		MemPoolUInt32(const MemPoolUInt32&) = delete;

		~MemPoolUInt32() noexcept
		{
			MOMO_ASSERT(mAllocCount == 0);
			pvClear();
		}

		MemPoolUInt32& operator=(const MemPoolUInt32&) = delete;

		const MemManager& GetMemManager() const noexcept
		{
			return mBuffers.GetMemManager();
		}

		MemManager& GetMemManager() noexcept
		{
			return mBuffers.GetMemManager();
		}

		template<typename ResObject = void>
		ResObject* GetRealPointer(uint32_t ptr) noexcept
		{
			MOMO_ASSERT(ptr != nullPtr);
			char* buffer = mBuffers[ptr / blockCount];
			void* realPtr = buffer + size_t{ptr % blockCount} * mBlockSize;
			return static_cast<ResObject*>(realPtr);
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

		void Deallocate(uint32_t ptr) noexcept
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
		uint32_t& pvGetNextBlock(void* realPtr) noexcept
		{
			return *static_cast<uint32_t*>(realPtr);
		}

		void pvNewBuffer()
		{
			size_t bufferCount = mBuffers.GetCount();
			if (bufferCount >= mMaxBufferCount)
				throw std::length_error("momo::internal::MemPoolUInt32 length error");
			mBuffers.Reserve(bufferCount + 1);
			char* buffer = MemManagerProxy::template Allocate<char>(GetMemManager(),
				pvGetBufferSize());
			for (size_t i = 0; i < blockCount; ++i)
			{
				void* realPtr = buffer + mBlockSize * i;
				pvGetNextBlock(realPtr) = (i + 1 < blockCount)
					? static_cast<uint32_t>(bufferCount * blockCount + i + 1) : nullPtr;
			}
			mBlockHead = static_cast<uint32_t>(bufferCount * blockCount);
			mBuffers.AddBackNogrow(buffer);
		}

		void pvShrink() noexcept
		{
			if (mBuffers.GetCount() > 2)
			{
				pvClear();
				mBlockHead = nullPtr;
				mBuffers.Clear(true);
			}
		}

		void pvClear() noexcept
		{
			MemManager& memManager = GetMemManager();
			size_t bufferSize = pvGetBufferSize();
			for (char* buffer : mBuffers)
				MemManagerProxy::Deallocate(memManager, buffer, bufferSize);
		}

		size_t pvGetBufferSize() const noexcept
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
