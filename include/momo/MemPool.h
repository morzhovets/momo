/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/MemPool.h

  namespace momo:
    class MemPoolConst
    class MemPoolParams
    class MemPoolParamsStatic
    class MemPoolSettings
    class MemPool

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_MEM_POOL
#define MOMO_INCLUDE_GUARD_MEM_POOL

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
		size_t maxAlignment = momo::internal::UIntConst::maxAlignment) noexcept	// c++/cli
	{
		return (maxAlignment > blockSize && maxAlignment > 1)
			? GetBlockAlignment(blockSize, maxAlignment / 2) : maxAlignment;
	}

	static constexpr size_t CorrectBlockSize(size_t blockSize, size_t blockAlignment,
		size_t blockCount) noexcept
	{
		return (blockCount == 1) ? ((blockSize > 0) ? blockSize : 1)
			: ((blockSize <= blockAlignment) ? 2 * blockAlignment
				: internal::UIntMath<>::Ceil(blockSize, blockAlignment));
	}

	static constexpr bool CheckBlockCount(size_t blockCount) noexcept
	{
		return 0 < blockCount && blockCount < 128;
	}

	static constexpr bool CheckBlockAlignment(size_t blockAlignment) noexcept
	{
		return 0 < blockAlignment && blockAlignment <= 1024;
	}
};

template<size_t tBlockCount = MemPoolConst::defaultBlockCount,
	size_t tCachedFreeBlockCount = MemPoolConst::defaultCachedFreeBlockCount>
class MemPoolParams
{
public:
	static const size_t blockCount = tBlockCount;
	MOMO_STATIC_ASSERT(MemPoolConst::CheckBlockCount(blockCount));

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;

public:
	explicit MemPoolParams(size_t blockSize) noexcept
		: MemPoolParams(blockSize, MemPoolConst::GetBlockAlignment(blockSize))
	{
	}

	explicit MemPoolParams(size_t blockSize, size_t blockAlignment) noexcept
	{
		MOMO_ASSERT(blockAlignment > 0);
		this->blockSize = MemPoolConst::CorrectBlockSize(blockSize, blockAlignment, blockCount);
		this->blockAlignment = blockAlignment;
	}

	size_t GetBlockSize() const noexcept
	{
		return blockSize;
	}

	size_t GetBlockAlignment() const noexcept
	{
		return blockAlignment;
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
	MOMO_STATIC_ASSERT(MemPoolConst::CheckBlockCount(blockCount));

	static const size_t blockAlignment = tBlockAlignment;
	MOMO_STATIC_ASSERT(MemPoolConst::CheckBlockAlignment(blockAlignment));

	static const size_t blockSize = MemPoolConst::CorrectBlockSize(tBlockSize,
		blockAlignment, blockCount);

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;

public:
	explicit MemPoolParamsStatic() noexcept
	{
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

	class Data : public MemManager
	{
	public:
		explicit Data(MemManager&& memManager) noexcept
			: MemManager(std::move(memManager)),
			allocCount(0)
		{
		}

		Data(Data&& data) noexcept
			: MemManager(std::move(static_cast<MemManager&>(data))),
			allocCount(data.allocCount)
		{
			data.allocCount = 0;
		}

		Data(const Data&) = delete;

		~Data() = default;

		void Swap(Data& data) noexcept
		{
			if (!MemManagerProxy::IsEqual(*this, data))
			{
				MemManager memManager(std::move(static_cast<MemManager&>(*this)));
				MemManagerProxy::Assign(std::move(static_cast<MemManager&>(data)), *this);
				MemManagerProxy::Assign(std::move(memManager), data);
			}
			std::swap(allocCount, data.allocCount);
		}

		Data& operator=(const Data&) = delete;

	public:
		size_t allocCount;
	};

	struct ChunkBytes
	{
		int8_t firstFreeBlockIndex;
		int8_t freeBlockCount;
	};

	typedef internal::Byte Byte;

public:
	explicit MemPool()	// vs clang
		: MemPool(MemManager())
	{
	}

	explicit MemPool(MemManager memManager)
		: MemPool(Params(), std::move(memManager))
	{
	}

	explicit MemPool(const Params& params, MemManager memManager = MemManager())
		: Params(params),
		mData(std::move(memManager)),
		mFreeChunkHead(nullptr),
		mCachedCount(0),
		mCacheHead(nullptr)
	{
		pvCheckParams();
	}

	MemPool(MemPool&& memPool) noexcept
		: Params(std::move(memPool.pvGetParams())),
		mData(std::move(memPool.mData)),
		mFreeChunkHead(memPool.mFreeChunkHead),
		mCachedCount(memPool.mCachedCount),
		mCacheHead(memPool.mCacheHead)
	{
		memPool.mFreeChunkHead = nullptr;
		memPool.mCachedCount = 0;
		memPool.mCacheHead = nullptr;
	}

	MemPool(const MemPool&) = delete;

	~MemPool() noexcept
	{
		MOMO_EXTRA_CHECK(mData.allocCount == 0);
		if (CanDeallocateAll())
			DeallocateAll();
		else if (pvUseCache())
			pvFlushDeallocate();
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
		mData.Swap(memPool.mData);
		std::swap(mFreeChunkHead, memPool.mFreeChunkHead);
		std::swap(mCachedCount, memPool.mCachedCount);
		std::swap(mCacheHead, memPool.mCacheHead);
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
		return mData;
	}

	MemManager& GetMemManager() noexcept
	{
		return mData;
	}

	template<typename ResObject = void>
	MOMO_NODISCARD ResObject* Allocate()
	{
		void* block;
		if (pvUseCache() && mCachedCount > 0)
		{
			block = mCacheHead;
			mCacheHead = internal::MemCopyer::FromBuffer<Byte*>(block);
			--mCachedCount;
		}
		else
		{
			if (Params::blockCount > 1)
				block = pvNewBlock();
			else if (pvGetAlignmentAddend() == 0)
				block = MemManagerProxy::Allocate(GetMemManager(), pvGetChunkSize0());
			else
				block = pvNewBlock1();
		}
		++mData.allocCount;
		return internal::PtrCaster::FromBytePtr<ResObject>(block);
	}

	template<typename Object>
	void Deallocate(Object* ptr) noexcept
	{
		MOMO_ASSERT(ptr != nullptr);
		MOMO_ASSERT(mData.allocCount > 0);
		Byte* block = internal::PtrCaster::ToBytePtr(ptr);
		if (pvUseCache())
		{
			if (mCachedCount >= Params::cachedFreeBlockCount)
				pvFlushDeallocate();
			internal::MemCopyer::ToBuffer(mCacheHead, block);
			mCacheHead = block;
			++mCachedCount;
		}
		else
		{
			pvDeallocate(block);
		}
		--mData.allocCount;
	}

	size_t GetAllocateCount() const noexcept
	{
		return mData.allocCount;
	}

	bool CanDeallocateAll() const noexcept
	{
		return Params::blockCount > 1;
	}

	void DeallocateAll() noexcept
	{
		MOMO_EXTRA_CHECK(CanDeallocateAll());
		if (mFreeChunkHead == nullptr)
			return;
		while (true)
		{
			Byte* prevChunk = pvGetPrevChunk(mFreeChunkHead);
			if (prevChunk == nullptr)
				break;
			pvDeleteChunk(prevChunk);
		}
		while (mFreeChunkHead != nullptr)
		{
			Byte* chunk = mFreeChunkHead;
			mFreeChunkHead = pvGetNextChunk(chunk);
			pvDeleteChunk(chunk);
		}
		mData.allocCount = 0;
		mCachedCount = 0;
		mCacheHead = nullptr;
	}

	template<typename BlockFilter>	// bool BlockFilter(void*)
	void DeallocateIf(const BlockFilter& blockFilter)
	{
		MOMO_EXTRA_CHECK(CanDeallocateAll());
		if (pvUseCache())
			pvFlushDeallocate();
		if (mData.allocCount == 0)
			return;
		Byte* chunk = mFreeChunkHead;
		while (true)
		{
			Byte* nextChunk = pvGetNextChunk(chunk);
			pvDeleteBlocks(chunk, blockFilter);
			if (nextChunk == nullptr)
				break;
			chunk = nextChunk;
		}
		chunk = pvGetPrevChunk(mFreeChunkHead);
		while (chunk != nullptr)
		{
			Byte* prevChunk = pvGetPrevChunk(chunk);
			pvDeleteBlocks(chunk, blockFilter);
			chunk = prevChunk;
		}
	}

	void MergeFrom(MemPool& memPool)
	{
		if (this == &memPool)
			return;
		MOMO_CHECK(GetBlockSize() == memPool.GetBlockSize());
		MOMO_CHECK(GetBlockAlignment() == memPool.GetBlockAlignment());
		MOMO_CHECK(GetBlockCount() == memPool.GetBlockCount());
		MOMO_CHECK(MemManagerProxy::IsEqual(GetMemManager(), memPool.GetMemManager()));
		if (memPool.pvUseCache())
			memPool.pvFlushDeallocate();
		mData.allocCount += memPool.mData.allocCount;
		memPool.mData.allocCount = 0;
		if (memPool.mFreeChunkHead == nullptr)
			return;
		if (mFreeChunkHead == nullptr)
		{
			mFreeChunkHead = memPool.mFreeChunkHead;
			memPool.mFreeChunkHead = nullptr;
			return;
		}
		while (true)
		{
			Byte* chunk = pvGetPrevChunk(memPool.mFreeChunkHead);
			if (chunk == nullptr)
				break;
			Byte* prevChunk = pvGetPrevChunk(chunk);
			Byte* nextChunk = memPool.mFreeChunkHead;
			if (prevChunk != nullptr)
				pvSetNextChunk(prevChunk, nextChunk);
			pvSetPrevChunk(nextChunk, prevChunk);
			prevChunk = pvGetPrevChunk(mFreeChunkHead);
			nextChunk = mFreeChunkHead;
			pvSetPrevChunk(chunk, prevChunk);
			pvSetNextChunk(chunk, nextChunk);
			if (prevChunk != nullptr)
				pvSetNextChunk(prevChunk, nextChunk);
			pvSetPrevChunk(nextChunk, prevChunk);
		}
		Byte* chunk = mFreeChunkHead;
		while (true)
		{
			Byte* nextChunk = pvGetNextChunk(chunk);
			if (nextChunk == nullptr)
				break;
			chunk = nextChunk;
		}
		pvSetNextChunk(chunk, memPool.mFreeChunkHead);
		pvSetPrevChunk(memPool.mFreeChunkHead, chunk);
		memPool.mFreeChunkHead = nullptr;
	}

private:
	Params& pvGetParams() noexcept
	{
		return *this;
	}

	void pvCheckParams() const
	{
		MOMO_CHECK(MemPoolConst::CheckBlockCount(Params::blockCount));
		MOMO_CHECK(MemPoolConst::CheckBlockAlignment(Params::blockAlignment));
		MOMO_CHECK(Params::blockSize > 0);
		MOMO_CHECK(Params::blockCount == 1 || Params::blockSize % Params::blockAlignment == 0);
		MOMO_CHECK(Params::blockCount == 1 || Params::blockSize / Params::blockAlignment >= 2);
		if (Params::blockSize > internal::UIntConst::maxSize / Params::blockCount)	//?
			throw std::length_error("Invalid block size");
	}

	bool pvUseCache() const noexcept
	{
		return Params::cachedFreeBlockCount > 0 && Params::blockSize >= sizeof(Byte*);	//?
	}

	MOMO_NOINLINE void pvFlushDeallocate() noexcept
	{
		for (size_t i = 0; i < mCachedCount; ++i)
		{
			Byte* block = mCacheHead;
			mCacheHead = internal::MemCopyer::FromBuffer<Byte*>(block);
			pvDeallocate(block);
		}
		mCachedCount = 0;
	}

	void pvDeallocate(Byte* block) noexcept
	{
		if (Params::blockCount > 1)
			pvDeleteBlock(block);
		else if (pvGetAlignmentAddend() == 0)
			MemManagerProxy::Deallocate(GetMemManager(), block, pvGetChunkSize0());
		else
			pvDeleteBlock1(block);
	}

	size_t pvGetAlignmentAddend() const noexcept
	{
		return Params::blockAlignment - std::minmax(size_t{internal::UIntConst::maxAllocAlignment},
			Params::blockAlignment & (~Params::blockAlignment + 1)).first;
	}

	size_t pvGetChunkSize0() const noexcept
	{
		return std::minmax(size_t{Params::blockSize}, size_t{Params::blockAlignment}).second;
	}

	Byte* pvNewBlock1()
	{
		const uintptr_t uipBlockAlignment = uintptr_t{Params::blockAlignment};
		Byte* chunk = MemManagerProxy::template Allocate<Byte>(
			GetMemManager(), pvGetChunkSize1());
		uintptr_t uipChunk = internal::PtrCaster::ToUInt(chunk);
		uintptr_t uipBlock = internal::UIntMath<uintptr_t>::Ceil(uipChunk, uipBlockAlignment);
		size_t offset = static_cast<size_t>(uipBlock - uipChunk);
		MOMO_ASSERT(offset < 256);
		Byte* block = chunk + offset;
		internal::MemCopyer::ToBuffer(static_cast<uint8_t>(offset),
			block + Params::blockSize);
		return block;
	}

	void pvDeleteBlock1(Byte* block) noexcept
	{
		size_t offset = size_t{internal::MemCopyer::FromBuffer<uint8_t>(
			block + Params::blockSize)};
		Byte* chunk = block - offset;
		MemManagerProxy::Deallocate(GetMemManager(), chunk, pvGetChunkSize1());
	}

	size_t pvGetChunkSize1() const noexcept
	{
		return Params::blockSize + pvGetAlignmentAddend() + 1;
	}

	Byte* pvNewBlock()
	{
		if (mFreeChunkHead == nullptr)
			mFreeChunkHead = pvNewChunk();
		Byte* bytesPos = pvGetChunkBytesPosition(mFreeChunkHead);
		ChunkBytes bytes = pvGetChunkBytes(bytesPos);
		Byte* nextChunk = pvGetNextChunk(mFreeChunkHead);
		if (bytes.freeBlockCount == int8_t{1} && nextChunk == nullptr)
		{
			nextChunk = pvNewChunk();
			pvSetNextChunk(mFreeChunkHead, nextChunk);
			pvSetPrevChunk(nextChunk, mFreeChunkHead);
		}
		Byte* block = pvGetBlock(mFreeChunkHead, bytes.firstFreeBlockIndex);
		bytes.firstFreeBlockIndex = pvGetNextFreeBlockIndex(block);
		--bytes.freeBlockCount;
		pvSetChunkBytes(bytesPos, bytes);
		if (bytes.freeBlockCount == int8_t{0})
			mFreeChunkHead = nextChunk;
		return block;
	}

	void pvDeleteBlock(Byte* block) noexcept
	{
		Byte* chunk;
		int8_t blockIndex = pvGetBlockIndex(block, chunk);
		pvDeleteBlock(block, chunk, blockIndex);
	}

	void pvDeleteBlock(Byte* block, Byte* chunk, int8_t blockIndex) noexcept
	{
		Byte* bytesPos = pvGetChunkBytesPosition(chunk);
		ChunkBytes bytes = pvGetChunkBytes(bytesPos);
		pvSetNextFreeBlockIndex(block, bytes.firstFreeBlockIndex);
		bytes.firstFreeBlockIndex = blockIndex;
		++bytes.freeBlockCount;
		pvSetChunkBytes(bytesPos, bytes);
		size_t freeBlockCount = static_cast<size_t>(bytes.freeBlockCount);
		if (freeBlockCount == 1)
			pvMoveChunkToHead(chunk);
		if (freeBlockCount == Params::blockCount)
		{
			bool del = true;
			if (chunk == mFreeChunkHead)
			{
				Byte* nextChunk = pvGetNextChunk(chunk);
				del = (nextChunk != nullptr);
				if (del)
					mFreeChunkHead = nextChunk;
			}
			if (del)
				pvDeleteChunk(chunk);
		}
	}

	static int8_t pvGetNextFreeBlockIndex(Byte* block) noexcept
	{
		return internal::MemCopyer::FromBuffer<int8_t>(block);
	}

	static void pvSetNextFreeBlockIndex(Byte* block, int8_t nextFreeBlockIndex) noexcept
	{
		internal::MemCopyer::ToBuffer(nextFreeBlockIndex, block);
	}

	Byte* pvGetBlock(Byte* chunk, int8_t index) const noexcept
	{
		return chunk + ptrdiff_t{index} * static_cast<ptrdiff_t>(Params::blockSize)
			+ (static_cast<ptrdiff_t>(Params::blockAlignment) & -ptrdiff_t{index >= 0});
	}

	int8_t pvGetBlockIndex(Byte* block, Byte*& chunk) const noexcept
	{
		const uintptr_t uipBlockSize = uintptr_t{Params::blockSize};
		const uintptr_t uipBlockAlignment = uintptr_t{Params::blockAlignment};
		const uintptr_t uipBlockCount = uintptr_t{Params::blockCount};
		uintptr_t uipBlock = internal::PtrCaster::ToUInt(block);
		MOMO_ASSERT(uipBlock % uipBlockAlignment == 0);
		ptrdiff_t dir = static_cast<ptrdiff_t>(((uipBlock % uipBlockSize) / uipBlockAlignment) % 2);
		ptrdiff_t index = static_cast<ptrdiff_t>((uipBlock / uipBlockSize) % uipBlockCount)
			- (static_cast<ptrdiff_t>(uipBlockCount) & (dir - 1));
		chunk = block - index * static_cast<ptrdiff_t>(uipBlockSize)
			- (static_cast<ptrdiff_t>(uipBlockAlignment) & -dir);
		return static_cast<int8_t>(index);
	}

	MOMO_NOINLINE Byte* pvNewChunk()
	{
		const uintptr_t uipBlockSize = uintptr_t{Params::blockSize};
		const uintptr_t uipBlockAlignment = uintptr_t{Params::blockAlignment};
		const uintptr_t uipBlockCount = uintptr_t{Params::blockCount};
		Byte* begin = MemManagerProxy::template Allocate<Byte>(
			GetMemManager(), pvGetChunkSize());
		uintptr_t uipBegin = internal::PtrCaster::ToUInt(begin);
		uintptr_t uipBlock = internal::UIntMath<uintptr_t>::Ceil(uipBegin, uipBlockAlignment);
		uipBlock += (uipBlock % uipBlockSize) % (2 * uipBlockAlignment);
		if ((uipBlock + uipBlockAlignment) % uipBlockSize == 0)
			uipBlock += uipBlockAlignment;
		if ((uipBlock / uipBlockSize) % uipBlockCount == 0)
			uipBlock += uipBlockAlignment;
		size_t beginOffset = static_cast<size_t>(uipBlock - uipBegin);
		MOMO_ASSERT(beginOffset < (1 << 16));
		Byte* block = begin + beginOffset;
		Byte* chunk;
		int8_t blockIndex = pvGetBlockIndex(block, chunk);
		pvSetFirstBlockIndex(chunk, blockIndex);
		ChunkBytes bytes;
		bytes.firstFreeBlockIndex = blockIndex;
		bytes.freeBlockCount = static_cast<int8_t>(uipBlockCount);
		pvSetChunkBytes(pvGetChunkBytesPosition(chunk), bytes);
		pvSetPrevChunk(chunk, nullptr);
		pvSetNextChunk(chunk, nullptr);
		pvSetBeginOffset(chunk, static_cast<uint16_t>(beginOffset));
		for (uintptr_t i = 1; i < uipBlockCount; ++i)
		{
			++blockIndex;
			pvSetNextFreeBlockIndex(block, blockIndex);
			block = pvGetBlock(chunk, blockIndex);
		}
		pvSetNextFreeBlockIndex(block, int8_t{-128});
		return chunk;
	}

	MOMO_NOINLINE void pvDeleteChunk(Byte* chunk) noexcept
	{
		MOMO_ASSERT(chunk != mFreeChunkHead);
		Byte* prevChunk = pvGetPrevChunk(chunk);
		Byte* nextChunk = pvGetNextChunk(chunk);
		if (prevChunk != nullptr)
			pvSetNextChunk(prevChunk, nextChunk);
		if (nextChunk != nullptr)
			pvSetPrevChunk(nextChunk, prevChunk);
		Byte* begin = pvGetBlock(chunk, pvGetFirstBlockIndex(chunk))
			- size_t{pvGetBeginOffset(chunk)};
		MemManagerProxy::Deallocate(GetMemManager(), begin, pvGetChunkSize());
	}

	MOMO_NOINLINE void pvMoveChunkToHead(Byte* chunk) noexcept
	{
		Byte* headPrevChunk = pvGetPrevChunk(mFreeChunkHead);
		MOMO_ASSERT(headPrevChunk != nullptr);
		if (chunk != headPrevChunk)
		{
			Byte* prevChunk = pvGetPrevChunk(chunk);
			Byte* nextChunk = pvGetNextChunk(chunk);
			MOMO_ASSERT(nextChunk != nullptr);
			pvSetPrevChunk(nextChunk, prevChunk);
			if (prevChunk != nullptr)
				pvSetNextChunk(prevChunk, nextChunk);
			pvSetPrevChunk(chunk, headPrevChunk);
			pvSetNextChunk(chunk, mFreeChunkHead);
			pvSetPrevChunk(mFreeChunkHead, chunk);
			pvSetNextChunk(headPrevChunk, chunk);
		}
		mFreeChunkHead = chunk;
	}

	size_t pvGetChunkSize() const noexcept
	{
		return Params::blockCount * Params::blockSize + pvGetAlignmentAddend()
			+ (2 + (Params::blockSize / Params::blockAlignment) % 2) * Params::blockAlignment
			+ (pvIsChunkBytesNear() ? 0 : sizeof(ChunkBytes))
			+ 2 * sizeof(Byte*) + sizeof(uint16_t);
	}

	template<typename BlockFilter>
	void pvDeleteBlocks(Byte* chunk, const BlockFilter& blockFilter)
	{
		int8_t firstBlockIndex = pvGetFirstBlockIndex(chunk);
		uint8_t freeBlockBits[16] = {};
		ChunkBytes bytes = pvGetChunkBytes(pvGetChunkBytesPosition(chunk));
		int8_t freeBlockIndex = bytes.firstFreeBlockIndex;
		for (int8_t i = 0; i < bytes.freeBlockCount; ++i)
		{
			internal::UIntMath<uint8_t>::SetBit(freeBlockBits,
				static_cast<size_t>(freeBlockIndex - firstBlockIndex));
			freeBlockIndex = pvGetNextFreeBlockIndex(pvGetBlock(chunk, freeBlockIndex));
		}
		for (size_t i = 0; i < Params::blockCount; ++i)
		{
			if (internal::UIntMath<uint8_t>::GetBit(freeBlockBits, i))
				continue;
			int8_t blockIndex = firstBlockIndex + static_cast<int8_t>(i);
			Byte* block = pvGetBlock(chunk, blockIndex);
			if (!blockFilter(static_cast<void*>(block)))
				continue;
			pvDeleteBlock(block, chunk, blockIndex);
			--mData.allocCount;
		}
	}

	static int8_t pvGetFirstBlockIndex(Byte* chunk) noexcept
	{
		return internal::MemCopyer::FromBuffer<int8_t>(chunk);
	}

	static void pvSetFirstBlockIndex(Byte* chunk, int8_t firstBlockIndex) noexcept
	{
		internal::MemCopyer::ToBuffer(firstBlockIndex, chunk);
	}

	ChunkBytes pvGetChunkBytes(Byte* chunkBytesPos) const noexcept
	{
		return internal::MemCopyer::FromBuffer<ChunkBytes>(chunkBytesPos);
	}

	void pvSetChunkBytes(Byte* chunkBytesPos, ChunkBytes chunkBytes) noexcept
	{
		internal::MemCopyer::ToBuffer(chunkBytes, chunkBytesPos);
	}

	Byte* pvGetChunkBytesPosition(Byte* chunk) const noexcept
	{
		return pvIsChunkBytesNear() ? chunk + 1 : pvGetBlocksEndPosition(chunk);
	}

	Byte* pvGetPrevChunk(Byte* chunk) const noexcept
	{
		return internal::MemCopyer::FromBuffer<Byte*>(pvGetPrevChunkPosition(chunk));
	}

	void pvSetPrevChunk(Byte* chunk, Byte* prevChunk) noexcept
	{
		internal::MemCopyer::ToBuffer(prevChunk, pvGetPrevChunkPosition(chunk));
	}

	Byte* pvGetPrevChunkPosition(Byte* chunk) const noexcept
	{
		return pvGetBlocksEndPosition(chunk) + (pvIsChunkBytesNear() ? 0 : sizeof(ChunkBytes));
	}

	Byte* pvGetNextChunk(Byte* chunk) const noexcept
	{
		return internal::MemCopyer::FromBuffer<Byte*>(pvGetNextChunkPosition(chunk));
	}

	void pvSetNextChunk(Byte* chunk, Byte* nextChunk) noexcept
	{
		internal::MemCopyer::ToBuffer(nextChunk, pvGetNextChunkPosition(chunk));
	}

	Byte* pvGetNextChunkPosition(Byte* chunk) const noexcept
	{
		return pvGetPrevChunkPosition(chunk) + sizeof(Byte*);
	}

	uint16_t pvGetBeginOffset(Byte* chunk) const noexcept
	{
		return internal::MemCopyer::FromBuffer<uint16_t>(pvGetBeginOffsetPosition(chunk));
	}

	void pvSetBeginOffset(Byte* chunk, uint16_t beginOffset) noexcept
	{
		internal::MemCopyer::ToBuffer(beginOffset, pvGetBeginOffsetPosition(chunk));
	}

	Byte* pvGetBeginOffsetPosition(Byte* chunk) const noexcept
	{
		return pvGetNextChunkPosition(chunk) + sizeof(Byte*);
	}

	Byte* pvGetBlocksEndPosition(Byte* chunk) const noexcept
	{
		size_t blockIndex = Params::blockCount;
		blockIndex -= static_cast<size_t>(-pvGetFirstBlockIndex(chunk));	// gcc
		return chunk + Params::blockAlignment + Params::blockSize * blockIndex;
	}

	bool pvIsChunkBytesNear() const noexcept
	{
		return Params::blockAlignment >= sizeof(ChunkBytes) + 1;
	}

private:
	Data mData;
	Byte* mFreeChunkHead;
	size_t mCachedCount;
	Byte* mCacheHead;
};

namespace internal
{
	class NestedMemPoolSettings : public MemPoolSettings
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

		static const uint32_t nullPtr = UIntConst::max32;

	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		typedef Array<Byte*, MemManager, ArrayItemTraits<Byte*, MemManager>,
			NestedArraySettings<>> Buffers;

	public:
		explicit MemPoolUInt32(size_t blockSize, MemManager&& memManager, size_t maxTotalBlockCount)
			: mBuffers(std::move(memManager)),
			mBlockHead(nullPtr),
			mMaxBufferCount(maxTotalBlockCount / blockCount),
			mBlockSize(std::minmax(blockSize, sizeof(uint32_t)).second),
			mAllocCount(0)
		{
			MOMO_ASSERT(maxTotalBlockCount < size_t{UIntConst::max32});
			if (mBlockSize > UIntConst::maxSize / blockCount)
				throw std::length_error("Invalid block size");
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
		ResObject* GetRealPointer(uint32_t block) noexcept
		{
			MOMO_ASSERT(block != nullPtr);
			Byte* buffer = mBuffers[block / blockCount];
			Byte* bytePtr = buffer + (size_t{block} % blockCount) * mBlockSize;
			return PtrCaster::FromBytePtr<ResObject>(bytePtr);
		}

		uint32_t Allocate()
		{
			if (mBlockHead == nullPtr)
				pvNewBuffer();
			uint32_t block = mBlockHead;
			mBlockHead = pvGetNextBlock(GetRealPointer(mBlockHead));
			++mAllocCount;
			return block;
		}

		void Deallocate(uint32_t block) noexcept
		{
			MOMO_ASSERT(block != nullPtr);
			MOMO_ASSERT(mAllocCount > 0);
			pvSetNextBlock(mBlockHead, GetRealPointer(block));
			mBlockHead = block;
			--mAllocCount;
			if (mAllocCount == 0 && mBuffers.GetCount() > 2)
				pvClear();
		}

		void DeallocateAll() noexcept
		{
			pvClear();
			mAllocCount = 0;
		}

	private:
		static uint32_t pvGetNextBlock(void* realPtr) noexcept
		{
			return internal::MemCopyer::FromBuffer<uint32_t>(realPtr);
		}

		static void pvSetNextBlock(uint32_t nextBlock, void* realPtr) noexcept
		{
			internal::MemCopyer::ToBuffer(nextBlock, realPtr);
		}

		void pvNewBuffer()
		{
			size_t bufferCount = mBuffers.GetCount();
			if (bufferCount >= mMaxBufferCount)
				throw std::length_error("Invalid buffer count");
			mBuffers.Reserve(bufferCount + 1);
			Byte* buffer = MemManagerProxy::template Allocate<Byte>(GetMemManager(),
				pvGetBufferSize());
			for (size_t i = 0; i < blockCount; ++i)
			{
				uint32_t nextBlock = (i + 1 < blockCount)
					? static_cast<uint32_t>(bufferCount * blockCount + i + 1) : nullPtr;
				pvSetNextBlock(nextBlock, buffer + mBlockSize * i);
			}
			mBlockHead = static_cast<uint32_t>(bufferCount * blockCount);
			mBuffers.AddBackNogrow(buffer);
		}

		void pvClear() noexcept
		{
			MemManager& memManager = GetMemManager();
			size_t bufferSize = pvGetBufferSize();
			for (Byte* buffer : mBuffers)
				MemManagerProxy::Deallocate(memManager, buffer, bufferSize);
			mBlockHead = nullPtr;
			mBuffers.Clear(true);
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

#endif // MOMO_INCLUDE_GUARD_MEM_POOL
