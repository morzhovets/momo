/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MemPool.h

  namespace momo:
    class MemPoolConst
    concept conceptMemPoolParams
    class MemPoolParams
    class MemPoolParamsStatic
    class MemPoolSettings
    class MemPool

\**********************************************************/

#pragma once

#include "MemManager.h"
#include "FunctionUtility.h"
#include "Array.h"

#include <atomic>

namespace momo
{

class MemPoolConst
{
public:
	static const size_t defaultBlockCount = MOMO_DEFAULT_MEM_POOL_BLOCK_COUNT;
	static const size_t defaultCachedFreeBlockCount = MOMO_DEFAULT_MEM_POOL_CACHED_FREE_BLOCK_COUNT;

public:
	static constexpr size_t GetBlockAlignment(size_t blockSize) noexcept
	{
		size_t alignment = internal::UIntConst::maxAlignment;
		while (alignment > blockSize && alignment > 1)
			alignment /= 2;
		return alignment;
	}

	static constexpr size_t CorrectBlockSize(size_t blockSize, size_t blockAlignment,
		size_t blockCount) noexcept
	{
		typedef internal::UIntMath<> SMath;
		if (blockCount == 1)
			return SMath::Max(blockSize, 1);
		return SMath::Max(SMath::Ceil(blockSize, blockAlignment), 2 * blockAlignment);
	}

	static constexpr bool CheckBlockCount(size_t blockCount) noexcept
	{
		return 0 < blockCount && blockCount < 128;
	}

	static constexpr bool CheckBlockAlignment(size_t blockAlignment) noexcept
	{
		return 0 < blockAlignment && blockAlignment <= 256;
	}
};

template<typename MemPoolParams>
concept conceptMemPoolParams =
	std::is_nothrow_destructible_v<MemPoolParams> &&
	std::is_nothrow_move_constructible_v<MemPoolParams> &&
	std::is_nothrow_move_assignable_v<MemPoolParams> &&
	requires (const MemPoolParams& params)
	{
		{ params.GetBlockSize() } -> std::same_as<size_t>;
		{ params.GetBlockAlignment() } -> std::same_as<size_t>;
		{ params.GetBlockCount() } -> std::same_as<size_t>;
		{ params.GetCachedFreeBlockCount() } -> std::same_as<size_t>;
	};

template<size_t tBlockCount = MemPoolConst::defaultBlockCount,
	size_t tCachedFreeBlockCount = MemPoolConst::defaultCachedFreeBlockCount>
requires (MemPoolConst::CheckBlockCount(tBlockCount))
class MemPoolParams
{
public:
	static const size_t blockCount = tBlockCount;

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;

public:
	explicit MemPoolParams(size_t blockSize) noexcept
		: MemPoolParams(blockSize, MemPoolConst::GetBlockAlignment(blockSize))
	{
	}

	explicit MemPoolParams(size_t blockSize, size_t blockAlignment) noexcept
	{
		MOMO_ASSERT(blockAlignment > 0);
		mBlockSize = MemPoolConst::CorrectBlockSize(blockSize, blockAlignment, blockCount);
		mBlockAlignment = blockAlignment;
	}

	size_t GetBlockSize() const noexcept
	{
		return mBlockSize;
	}

	size_t GetBlockAlignment() const noexcept
	{
		return mBlockAlignment;
	}

	static constexpr size_t GetBlockCount() noexcept
	{
		return blockCount;
	}

	static constexpr size_t GetCachedFreeBlockCount() noexcept
	{
		return cachedFreeBlockCount;
	}

private:
	size_t mBlockSize;
	size_t mBlockAlignment;
};

template<size_t tBlockSize,
	size_t tBlockAlignment = MemPoolConst::GetBlockAlignment(tBlockSize),
	size_t tBlockCount = MemPoolConst::defaultBlockCount,
	size_t tCachedFreeBlockCount = MemPoolConst::defaultCachedFreeBlockCount>
requires (MemPoolConst::CheckBlockCount(tBlockCount) &&
	MemPoolConst::CheckBlockAlignment(tBlockAlignment))
class MemPoolParamsStatic
{
public:
	static const size_t blockCount = tBlockCount;

	static const size_t blockAlignment = tBlockAlignment;

	static const size_t blockSize = MemPoolConst::CorrectBlockSize(tBlockSize,
		blockAlignment, blockCount);

	static const size_t cachedFreeBlockCount = tCachedFreeBlockCount;

public:
	explicit MemPoolParamsStatic() noexcept = default;

	static constexpr size_t GetBlockSize() noexcept
	{
		return blockSize;
	}

	static constexpr size_t GetBlockAlignment() noexcept
	{
		return blockAlignment;
	}

	static constexpr size_t GetBlockCount() noexcept
	{
		return blockCount;
	}

	static constexpr size_t GetCachedFreeBlockCount() noexcept
	{
		return cachedFreeBlockCount;
	}
};

class MemPoolSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
};

namespace internal
{
	template<conceptMemPoolParams TParams>
	class MemChunker : public TParams
	{
	public:
		typedef TParams Params;

	private:
		typedef std::byte Byte;

		struct ChunkBytes
		{
			int8_t firstFreeBlockIndex;
			int8_t freeBlockCount;
		};

	public:
		explicit MemChunker(Params&& params)
			: Params(std::move(params))
		{
		}

		template<conceptMemManager MemManager>
		Byte* NewChunk(MemManager& memManager) const
		{
			Byte* begin = MemManagerProxy<MemManager>::template Allocate<Byte>(
				memManager, pvGetChunkSize());
			return pvNewChunk(begin);
		}

		template<conceptMemManager MemManager>
		void DeleteChunk(MemManager& memManager, Byte* chunk) const noexcept
		{
			Byte* begin = pvGetBlock(chunk, pvGetFirstBlockIndex(chunk))
				- size_t{pvGetBeginOffset(chunk)};
			MemManagerProxy<MemManager>::Deallocate(memManager, begin, pvGetChunkSize());
		}

		Byte* NewBlock(Byte* chunk, size_t& freeBlockCount) const noexcept
		{
			Byte* bytesPos = pvGetChunkBytesPosition(chunk);
			ChunkBytes bytes = pvGetChunkBytes(bytesPos);
			MOMO_ASSERT(bytes.freeBlockCount > 0);
			Byte* block = pvGetBlock(chunk, bytes.firstFreeBlockIndex);
			bytes.firstFreeBlockIndex = pvGetNextFreeBlockIndex(block);
			--bytes.freeBlockCount;
			pvSetChunkBytes(bytesPos, bytes);
			freeBlockCount = static_cast<size_t>(bytes.freeBlockCount);
			return block;
		}

		void DeleteBlock(Byte* block, Byte*& chunk, size_t& freeBlockCount) const noexcept
		{
			int8_t blockIndex = pvGetBlockIndex(block, chunk);
			freeBlockCount = static_cast<size_t>(pvDeleteBlock(block, chunk, blockIndex));
		}

		template<conceptPredicate<Byte*> BlockFilter>
		void DeleteBlocks(Byte* chunk, FastCopyableFunctor<BlockFilter> blockFilter) const
		{
			int8_t firstBlockIndex = pvGetFirstBlockIndex(chunk);
			uint8_t freeBlockBits[16] = {};
			ChunkBytes bytes = pvGetChunkBytes(pvGetChunkBytesPosition(chunk));
			int8_t freeBlockIndex = bytes.firstFreeBlockIndex;
			for (int8_t i = 0; i < bytes.freeBlockCount; ++i)
			{
				UIntMath<uint8_t>::SetBit(freeBlockBits,
					static_cast<size_t>(freeBlockIndex - firstBlockIndex));
				freeBlockIndex = pvGetNextFreeBlockIndex(pvGetBlock(chunk, freeBlockIndex));
			}
			for (size_t i = 0; i < Params::GetBlockCount(); ++i)
			{
				if (UIntMath<uint8_t>::GetBit(freeBlockBits, i))
					continue;
				int8_t blockIndex = firstBlockIndex + static_cast<int8_t>(i);
				Byte* block = pvGetBlock(chunk, blockIndex);
				if (blockFilter(block))
					pvDeleteBlock(block, chunk, blockIndex);
			}
		}

		Byte* GetPrevChunk(Byte* chunk) const noexcept
		{
			return MemCopyer::FromBuffer<Byte*>(pvGetPrevChunkPosition(chunk));
		}

		void SetPrevChunk(Byte* chunk, Byte* prevChunk) const noexcept
		{
			return MemCopyer::ToBuffer(prevChunk, pvGetPrevChunkPosition(chunk));
		}

		Byte* GetNextChunk(Byte* chunk) const noexcept
		{
			return MemCopyer::FromBuffer<Byte*>(pvGetNextChunkPosition(chunk));
		}

		void SetNextChunk(Byte* chunk, Byte* nextChunk) const noexcept
		{
			return MemCopyer::ToBuffer(nextChunk, pvGetNextChunkPosition(chunk));
		}

	private:
		size_t pvGetAlignmentAddend() const noexcept
		{
			const size_t blockAlignment = Params::GetBlockAlignment();
			return blockAlignment - UIntMath<>::Min(UIntConst::maxAllocAlignment,
				size_t{1} << std::countr_zero(blockAlignment));
		}

		static int8_t pvGetNextFreeBlockIndex(Byte* block) noexcept
		{
			return MemCopyer::FromBuffer<int8_t>(block);
		}

		static void pvSetNextFreeBlockIndex(Byte* block, int8_t nextFreeBlockIndex) noexcept
		{
			MemCopyer::ToBuffer(nextFreeBlockIndex, block);
		}

		Byte* pvGetBlock(Byte* chunk, int8_t index) const noexcept
		{
			return chunk + ptrdiff_t{index} * static_cast<ptrdiff_t>(Params::GetBlockSize())
				+ (static_cast<ptrdiff_t>(Params::GetBlockAlignment()) & -ptrdiff_t{index >= 0});
		}

		int8_t pvGetBlockIndex(Byte* block, Byte*& chunk) const noexcept
		{
			const uintptr_t uipBlockSize = uintptr_t{Params::GetBlockSize()};
			const uintptr_t uipBlockAlignment = uintptr_t{Params::GetBlockAlignment()};
			const uintptr_t uipBlockCount = uintptr_t{Params::GetBlockCount()};
			uintptr_t uipBlock = PtrCaster::ToUInt(block);
			MOMO_ASSERT(uipBlock % uipBlockAlignment == 0);
			ptrdiff_t dir = static_cast<ptrdiff_t>(((uipBlock % uipBlockSize) / uipBlockAlignment) % 2);
			ptrdiff_t index = static_cast<ptrdiff_t>((uipBlock / uipBlockSize) % uipBlockCount)
				- (static_cast<ptrdiff_t>(uipBlockCount) & (dir - 1));
			chunk = block - index * static_cast<ptrdiff_t>(uipBlockSize)
				- (static_cast<ptrdiff_t>(uipBlockAlignment) & -dir);
			return static_cast<int8_t>(index);
		}

		Byte* pvNewChunk(Byte* begin) const noexcept
		{
			const uintptr_t uipBlockSize = uintptr_t{Params::GetBlockSize()};
			const uintptr_t uipBlockAlignment = uintptr_t{Params::GetBlockAlignment()};
			const uintptr_t uipBlockCount = uintptr_t{Params::GetBlockCount()};
			uintptr_t uipBegin = PtrCaster::ToUInt(begin);
			uintptr_t uipBlock = UIntMath<uintptr_t>::Ceil(uipBegin, uipBlockAlignment);
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
			SetPrevChunk(chunk, nullptr);
			SetNextChunk(chunk, nullptr);
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

		int8_t pvDeleteBlock(Byte* block, Byte* chunk, int8_t blockIndex) const noexcept
		{
			Byte* bytesPos = pvGetChunkBytesPosition(chunk);
			ChunkBytes bytes = pvGetChunkBytes(bytesPos);
			pvSetNextFreeBlockIndex(block, bytes.firstFreeBlockIndex);
			bytes.firstFreeBlockIndex = blockIndex;
			++bytes.freeBlockCount;
			pvSetChunkBytes(bytesPos, bytes);
			return bytes.freeBlockCount;
		}

		size_t pvGetChunkSize() const noexcept
		{
			const size_t blockSize = Params::GetBlockSize();
			const size_t blockAlignment = Params::GetBlockAlignment();
			return Params::GetBlockCount() * blockSize + pvGetAlignmentAddend()
				+ (2 + (blockSize / blockAlignment) % 2) * blockAlignment
				+ (pvIsChunkBytesNear() ? 0 : sizeof(ChunkBytes))
				+ 2 * sizeof(Byte*) + sizeof(uint16_t);
		}

		static int8_t pvGetFirstBlockIndex(Byte* chunk) noexcept
		{
			return MemCopyer::FromBuffer<int8_t>(chunk);
		}

		static void pvSetFirstBlockIndex(Byte* chunk, int8_t firstBlockIndex) noexcept
		{
			MemCopyer::ToBuffer(firstBlockIndex, chunk);
		}

		ChunkBytes pvGetChunkBytes(Byte* chunkBytesPos) const noexcept
		{
			return MemCopyer::FromBuffer<ChunkBytes>(chunkBytesPos);
		}

		void pvSetChunkBytes(Byte* chunkBytesPos, ChunkBytes chunkBytes) const noexcept
		{
			MemCopyer::ToBuffer(chunkBytes, chunkBytesPos);
		}

		Byte* pvGetChunkBytesPosition(Byte* chunk) const noexcept
		{
			return pvIsChunkBytesNear() ? chunk + 1 : pvGetBlocksEndPosition(chunk);
		}

		Byte* pvGetPrevChunkPosition(Byte* chunk) const noexcept
		{
			return pvGetBlocksEndPosition(chunk) + (pvIsChunkBytesNear() ? 0 : sizeof(ChunkBytes));
		}

		Byte* pvGetNextChunkPosition(Byte* chunk) const noexcept
		{
			return pvGetPrevChunkPosition(chunk) + sizeof(Byte*);
		}

		uint16_t pvGetBeginOffset(Byte* chunk) const noexcept
		{
			return MemCopyer::FromBuffer<uint16_t>(pvGetBeginOffsetPosition(chunk));
		}

		void pvSetBeginOffset(Byte* chunk, uint16_t beginOffset) const noexcept
		{
			MemCopyer::ToBuffer(beginOffset, pvGetBeginOffsetPosition(chunk));
		}

		Byte* pvGetBeginOffsetPosition(Byte* chunk) const noexcept
		{
			return pvGetNextChunkPosition(chunk) + sizeof(Byte*);
		}

		Byte* pvGetBlocksEndPosition(Byte* chunk) const noexcept
		{
			return chunk + Params::GetBlockAlignment() + Params::GetBlockSize()
				* (Params::GetBlockCount() - static_cast<size_t>(-pvGetFirstBlockIndex(chunk)));
		}

		bool pvIsChunkBytesNear() const noexcept
		{
			return Params::GetBlockAlignment() >= sizeof(ChunkBytes) + 1;
		}
	};
}

template<conceptMemPoolParams TParams = MemPoolParams<>,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TSettings = MemPoolSettings>
class MOMO_EMPTY_BASES MemPool
	: private internal::MemChunker<TParams>,
	public internal::Swappable<MemPool>
{
public:
	typedef TParams Params;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

private:
	typedef internal::MemChunker<Params> Chunker;
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
			allocCount(std::exchange(data.allocCount, 0))
		{
		}

		Data(const Data&) = delete;

		~Data() noexcept = default;

		Data& operator=(const Data&) = delete;

		void Swap(Data& data) noexcept
		{
			if (this != &data)	//?
				MemManagerProxy::Swap(*this, data);
			std::swap(allocCount, data.allocCount);
		}

	public:
		size_t allocCount;
	};

	typedef std::byte Byte;

public:
	explicit MemPool()	// vs clang
		: MemPool(MemManager())
	{
	}

	explicit MemPool(MemManager memManager)
		: MemPool(Params(), std::move(memManager))
	{
	}

	explicit MemPool(Params params, MemManager memManager = MemManager())
		: Chunker(std::move(params)),
		mData(std::move(memManager)),
		mFreeChunkHead(nullptr),
		mCachedCount(0),
		mCacheHead(nullptr)
	{
		pvCheckParams();
	}

	MemPool(MemPool&& memPool) noexcept
		: Chunker(std::move(memPool.pvGetChunker())),
		mData(std::move(memPool.mData)),
		mFreeChunkHead(std::exchange(memPool.mFreeChunkHead, nullptr)),
		mCachedCount(std::exchange(memPool.mCachedCount, 0)),
		mCacheHead(std::exchange(memPool.mCacheHead, nullptr))
	{
	}

	MemPool(const MemPool&) = delete;

	~MemPool() noexcept
	{
		pvDestroy();
	}

	MemPool& operator=(MemPool&& memPool) noexcept
	{
		if (this != &memPool)
		{
			Swap(memPool);
			memPool.pvDestroy();
		}
		return *this;
	}

	MemPool& operator=(const MemPool&) = delete;

	void Swap(MemPool& memPool) noexcept
	{
		std::swap(pvGetChunker(), memPool.pvGetChunker());
		mData.Swap(memPool.mData);
		std::swap(mFreeChunkHead, memPool.mFreeChunkHead);
		std::swap(mCachedCount, memPool.mCachedCount);
		std::swap(mCacheHead, memPool.mCacheHead);
	}

	size_t GetBlockSize() const noexcept
	{
		return Params::GetBlockSize();
	}

	size_t GetBlockAlignment() const noexcept
	{
		return Params::GetBlockAlignment();
	}

	size_t GetBlockCount() const noexcept
	{
		return Params::GetBlockCount();
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
	[[nodiscard]] ResObject* Allocate()
	{
		return internal::PtrCaster::FromBytePtr<ResObject>(pvAllocate());
	}

	template<typename Object>
	void Deallocate(Object* ptr) noexcept
	{
		//MOMO_ASSERT(ptr != nullptr);
		pvDeallocate(internal::PtrCaster::ToBytePtr(ptr));
	}

	size_t GetAllocateCount() const noexcept
	{
		return mData.allocCount;
	}

	bool CanDeallocateAll() const noexcept
	{
		return Params::GetBlockCount() > 1;
	}

	void DeallocateAll() noexcept
	{
		MOMO_EXTRA_CHECK(CanDeallocateAll());
		if (mFreeChunkHead == nullptr)
			return;
		while (true)
		{
			Byte* prevChunk = Chunker::GetPrevChunk(mFreeChunkHead);
			if (prevChunk == nullptr)
				break;
			pvDeleteChunk(prevChunk);
		}
		while (mFreeChunkHead != nullptr)
		{
			Byte* chunk = mFreeChunkHead;
			mFreeChunkHead = Chunker::GetNextChunk(chunk);
			pvDeleteChunk(chunk);
		}
		mData.allocCount = 0;
		mCachedCount = 0;
		mCacheHead = nullptr;
	}

	template<internal::conceptPredicate<void*> BlockFilter>
	void DeallocateIf(BlockFilter blockFilter)
	{
		MOMO_EXTRA_CHECK(CanDeallocateAll());
		if (pvUseCache())
			pvFlushDeallocate();
		if (mData.allocCount > 0)
			pvDeallocateIf(FastCopyableFunctor(blockFilter));
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
			Byte* chunk = Chunker::GetPrevChunk(memPool.mFreeChunkHead);
			if (chunk == nullptr)
				break;
			Byte* prevChunk = Chunker::GetPrevChunk(chunk);
			Byte* nextChunk = memPool.mFreeChunkHead;
			if (prevChunk != nullptr)
				Chunker::SetNextChunk(prevChunk, nextChunk);
			Chunker::SetPrevChunk(nextChunk, prevChunk);
			prevChunk = Chunker::GetPrevChunk(mFreeChunkHead);
			nextChunk = mFreeChunkHead;
			Chunker::SetPrevChunk(chunk, prevChunk);
			Chunker::SetNextChunk(chunk, nextChunk);
			if (prevChunk != nullptr)
				Chunker::SetNextChunk(prevChunk, nextChunk);
			Chunker::SetPrevChunk(nextChunk, prevChunk);
		}
		Byte* chunk = mFreeChunkHead;
		while (true)
		{
			Byte* nextChunk = Chunker::GetNextChunk(chunk);
			if (nextChunk == nullptr)
				break;
			chunk = nextChunk;
		}
		Chunker::SetNextChunk(chunk, memPool.mFreeChunkHead);
		Chunker::SetPrevChunk(memPool.mFreeChunkHead, chunk);
		memPool.mFreeChunkHead = nullptr;
	}

private:
	Chunker& pvGetChunker() noexcept
	{
		return *this;
	}

	void pvCheckParams() const
	{
		const size_t blockSize = Params::GetBlockSize();
		const size_t blockAlignment = Params::GetBlockAlignment();
		const size_t blockCount = Params::GetBlockCount();
		MOMO_CHECK(MemPoolConst::CheckBlockCount(blockCount));
		MOMO_CHECK(MemPoolConst::CheckBlockAlignment(blockAlignment));
		MOMO_CHECK(blockSize > 0);
		MOMO_CHECK(blockCount == 1 || blockSize % blockAlignment == 0);
		MOMO_CHECK(blockCount == 1 || blockSize / blockAlignment >= 2);
		if (blockSize > internal::UIntConst::maxSize / blockCount)	//?
			MOMO_THROW(std::length_error("Invalid block size"));
	}

	void pvDestroy() noexcept
	{
		MOMO_EXTRA_CHECK(mData.allocCount == 0);
		if (CanDeallocateAll())
			DeallocateAll();
		else if (pvUseCache())
			pvFlushDeallocate();
		mData.allocCount = 0;
		mFreeChunkHead = nullptr;
	}

	bool pvUseCache() const noexcept
	{
		return Params::GetCachedFreeBlockCount() > 0
			&& Params::GetBlockSize() >= sizeof(Byte*);	//?
	}

	void* pvAllocate()
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
			if (Params::GetBlockCount() > 1)
				block = pvNewBlock();
			else if (pvGetAlignmentAddend() == 0)
				block = MemManagerProxy::Allocate(GetMemManager(), pvGetChunkSize0());
			else
				block = pvNewBlock1();
		}
		++mData.allocCount;
		return block;
	}

	void pvDeallocate(Byte* block) noexcept
	{
		MOMO_ASSERT(mData.allocCount > 0);
		if (pvUseCache())
		{
			if (mCachedCount >= Params::GetCachedFreeBlockCount())
				pvFlushDeallocate();
			internal::MemCopyer::ToBuffer(mCacheHead, block);
			mCacheHead = block;
			++mCachedCount;
		}
		else
		{
			pvDeallocateNoCache(block);
		}
		--mData.allocCount;
	}

	MOMO_NOINLINE void pvFlushDeallocate() noexcept
	{
		for (size_t i = 0; i < mCachedCount; ++i)
		{
			Byte* block = mCacheHead;
			mCacheHead = internal::MemCopyer::FromBuffer<Byte*>(block);
			pvDeallocateNoCache(block);
		}
		mCachedCount = 0;
	}

	void pvDeallocateNoCache(Byte* block) noexcept
	{
		if (Params::GetBlockCount() > 1)
			pvDeleteBlock(block);
		else if (pvGetAlignmentAddend() == 0)
			MemManagerProxy::Deallocate(GetMemManager(), block, pvGetChunkSize0());
		else
			pvDeleteBlock1(block);
	}

	size_t pvGetAlignmentAddend() const noexcept
	{
		const size_t blockAlignment = Params::GetBlockAlignment();
		return blockAlignment - internal::UIntMath<>::Min(internal::UIntConst::maxAllocAlignment,
			size_t{1} << std::countr_zero(blockAlignment));
	}

	size_t pvGetChunkSize0() const noexcept
	{
		return internal::UIntMath<>::Max(Params::GetBlockSize(), Params::GetBlockAlignment());
	}

	Byte* pvNewBlock1()
	{
		const uintptr_t uipBlockAlignment = uintptr_t{Params::GetBlockAlignment()};
		Byte* chunk = MemManagerProxy::template Allocate<Byte>(
			GetMemManager(), pvGetChunkSize1());
		uintptr_t uipChunk = internal::PtrCaster::ToUInt(chunk);
		uintptr_t uipBlock = internal::UIntMath<uintptr_t>::Ceil(uipChunk, uipBlockAlignment);
		size_t offset = static_cast<size_t>(uipBlock - uipChunk);
		MOMO_ASSERT(offset < 256);
		Byte* block = chunk + offset;
		internal::MemCopyer::ToBuffer(static_cast<uint8_t>(offset),
			block + Params::GetBlockSize());
		return block;
	}

	void pvDeleteBlock1(Byte* block) noexcept
	{
		size_t offset = size_t{internal::MemCopyer::FromBuffer<uint8_t>(
			block + Params::GetBlockSize())};
		Byte* chunk = block - offset;
		MemManagerProxy::Deallocate(GetMemManager(), chunk, pvGetChunkSize1());
	}

	size_t pvGetChunkSize1() const noexcept
	{
		return Params::GetBlockSize() + pvGetAlignmentAddend() + 1;
	}

	Byte* pvNewBlock()
	{
		if (mFreeChunkHead == nullptr)
			mFreeChunkHead = pvNewChunk();
		size_t freeBlockCount;
		Byte* block = Chunker::NewBlock(mFreeChunkHead, freeBlockCount);
		if (freeBlockCount == 0)
		{
			Byte* nextChunk = Chunker::GetNextChunk(mFreeChunkHead);
			if (nextChunk == nullptr)
			{
				internal::Finalizer fin(&MemPool::template pvDeleteBlock<false>, *this, block);
				nextChunk = pvNewChunk();
				fin.Detach();
				Chunker::SetNextChunk(mFreeChunkHead, nextChunk);
				Chunker::SetPrevChunk(nextChunk, mFreeChunkHead);
			}
			mFreeChunkHead = nextChunk;
		}
		return block;
	}

	template<bool processChunks = true>
	void pvDeleteBlock(Byte* block) noexcept
	{
		Byte* chunk;
		size_t freeBlockCount;
		Chunker::DeleteBlock(block, chunk, freeBlockCount);
		if constexpr (processChunks)
		{
			if (freeBlockCount == 1)
				pvMoveChunkToHead(chunk);
			if (freeBlockCount == Params::GetBlockCount())
			{
				bool del = true;
				if (chunk == mFreeChunkHead)
				{
					Byte* nextChunk = Chunker::GetNextChunk(chunk);
					del = (nextChunk != nullptr);
					if (del)
						mFreeChunkHead = nextChunk;
				}
				if (del)
					pvDeleteChunk(chunk);
			}
		}
	}

	MOMO_NOINLINE Byte* pvNewChunk()
	{
		return Chunker::NewChunk(GetMemManager());
	}

	MOMO_NOINLINE void pvDeleteChunk(Byte* chunk) noexcept
	{
		MOMO_ASSERT(chunk != mFreeChunkHead);
		Byte* prevChunk = Chunker::GetPrevChunk(chunk);
		Byte* nextChunk = Chunker::GetNextChunk(chunk);
		if (prevChunk != nullptr)
			Chunker::SetNextChunk(prevChunk, nextChunk);
		if (nextChunk != nullptr)
			Chunker::SetPrevChunk(nextChunk, prevChunk);
		Chunker::DeleteChunk(GetMemManager(), chunk);
	}

	MOMO_NOINLINE void pvMoveChunkToHead(Byte* chunk) noexcept
	{
		Byte* headPrevChunk = Chunker::GetPrevChunk(mFreeChunkHead);
		MOMO_ASSERT(headPrevChunk != nullptr);
		if (chunk != headPrevChunk)
		{
			Byte* prevChunk = Chunker::GetPrevChunk(chunk);
			Byte* nextChunk = Chunker::GetNextChunk(chunk);
			MOMO_ASSERT(nextChunk != nullptr);
			Chunker::SetPrevChunk(nextChunk, prevChunk);
			if (prevChunk != nullptr)
				Chunker::SetNextChunk(prevChunk, nextChunk);
			Chunker::SetPrevChunk(chunk, headPrevChunk);
			Chunker::SetNextChunk(chunk, mFreeChunkHead);
			Chunker::SetPrevChunk(mFreeChunkHead, chunk);
			Chunker::SetNextChunk(headPrevChunk, chunk);
		}
		mFreeChunkHead = chunk;
	}

	template<internal::conceptPredicate<void*> BlockFilter>
	void pvDeallocateIf(FastCopyableFunctor<BlockFilter> blockFilter)
	{
		Byte* chunk = mFreeChunkHead;
		while (true)
		{
			Byte* nextChunk = Chunker::GetNextChunk(chunk);
			pvDeleteBlocks(chunk, blockFilter);
			if (nextChunk == nullptr)
				break;
			chunk = nextChunk;
		}
		chunk = Chunker::GetPrevChunk(mFreeChunkHead);
		while (chunk != nullptr)
		{
			Byte* prevChunk = Chunker::GetPrevChunk(chunk);
			pvDeleteBlocks(chunk, blockFilter);
			chunk = prevChunk;
		}
	}

	template<internal::conceptPredicate<void*> BlockFilter>
	void pvDeleteBlocks(Byte* chunk, FastCopyableFunctor<BlockFilter> blockFilter)
	{
		auto chunkerBlockFilter = [this, blockFilter] (Byte* block)
		{
			bool res = blockFilter(static_cast<void*>(block));
			if (res)
				--mData.allocCount;
			return res;
		};
		Chunker::DeleteBlocks(chunk, FastCopyableFunctor(chunkerBlockFilter));
	}

private:
	Data mData;
	Byte* mFreeChunkHead;
	size_t mCachedCount;
	Byte* mCacheHead;
};

namespace internal
{
	template<typename MemPoolParams>
	concept conceptMemPoolParamsBlockSizeAlignment = conceptMemPoolParams<MemPoolParams> &&
		requires (size_t blockSize, size_t blockAlignment)
		{
			MemPoolParams(blockSize);
			MemPoolParams(blockSize, blockAlignment);
			typename std::integral_constant<size_t, MemPoolParams::blockCount>;
			typename std::integral_constant<size_t, MemPoolParams::cachedFreeBlockCount>;
		};

	class NestedMemPoolSettings : public MemPoolSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
	};

	template<conceptMemPoolParams TParams, conceptMemManager TMemManager>
	class MemPoolLazy
	{
	public:
		typedef TParams Params;
		typedef TMemManager MemManager;

	private:
		typedef momo::MemPool<Params, MemManager, NestedMemPoolSettings> MemPool;

	public:
		explicit MemPoolLazy(Params&& params, MemManager&& memManager)
			: mMemPool(std::move(params), std::move(memManager)),
			mLazyHead(nullptr)
		{
			MOMO_ASSERT(mMemPool.GetBlockSize() >= sizeof(void*));
		}

		MemPoolLazy(const MemPoolLazy&) = delete;

		~MemPoolLazy() noexcept
		{
			pvFlushDeallocate(mLazyHead);
		}

		MemPoolLazy& operator=(const MemPoolLazy&) = delete;

		template<typename ResObject = void>
		[[nodiscard]] ResObject* Allocate()
		{
			return PtrCaster::FromBytePtr<ResObject>(pvAllocate());
		}

		template<typename Object>
		void Deallocate(Object* ptr) noexcept
		{
			pvDeallocate(PtrCaster::ToBytePtr(ptr));
		}

		template<typename Object>
		void DeallocateLazy(Object* ptr) noexcept
		{
			pvDeallocateLazy(PtrCaster::ToBytePtr(ptr));
		}

	private:
		void* pvAllocate()
		{
			if (mLazyHead != nullptr)
			{
				std::byte* block = mLazyHead.exchange(nullptr);
				if (block != nullptr)
				{
					pvFlushDeallocate(MemCopyer::FromBuffer<std::byte*>(block));
					return block;
				}
			}
			return mMemPool.Allocate();
		}

		void pvDeallocate(std::byte* block) noexcept
		{
			if (mLazyHead != nullptr)
				pvFlushDeallocate(mLazyHead.exchange(nullptr));
			mMemPool.Deallocate(block);
		}

		void pvDeallocateLazy(std::byte* block) noexcept
		{
			while (true)
			{
				std::byte* lazyHead = mLazyHead;
				MemCopyer::ToBuffer(lazyHead, block);
				if (mLazyHead.compare_exchange_weak(lazyHead, block))
					break;
			}
		}

		void pvFlushDeallocate(std::byte* block) noexcept
		{
			while (block != nullptr)
			{
				std::byte* nextBlock = MemCopyer::FromBuffer<std::byte*>(block);
				mMemPool.Deallocate(block);
				block = nextBlock;
			}
		}

	private:
		MemPool mMemPool;
		std::atomic<std::byte*> mLazyHead;
	};

	template<conceptMemManager TBaseMemManager, conceptMemPoolParams TMemPoolParams>
	requires std::is_default_constructible_v<TMemPoolParams>
	class MemManagerPoolLazy : public TBaseMemManager
	{
	public:
		typedef TBaseMemManager BaseMemManager;
		typedef TMemPoolParams MemPoolParams;

		typedef MemPoolLazy<MemPoolParams, MemManagerPtr<BaseMemManager>> MemPool;

	public:
		MemManagerPoolLazy(BaseMemManager&& baseMemManager) noexcept
			: BaseMemManager(std::move(baseMemManager)),
			mMemPool(std::nullopt)
		{
		}

		MemManagerPoolLazy(MemManagerPoolLazy&& memManager) noexcept
			: BaseMemManager((memManager.mMemPool.reset(), std::move(memManager))),
			mMemPool(std::nullopt)
		{
		}

		MemManagerPoolLazy(const MemManagerPoolLazy& memManager)
			: BaseMemManager(memManager),
			mMemPool(std::nullopt)
		{
		}

		~MemManagerPoolLazy() noexcept
		{
			MOMO_ASSERT(!mMemPool.has_value());	//?
		}

		MemManagerPoolLazy& operator=(MemManagerPoolLazy&) = delete;

		MemPool& GetMemPool()
		{
			if (!mMemPool.has_value()) [[unlikely]]
				pvCreateMemPool();
			return mMemPool.value();
		}

	private:
		MOMO_NOINLINE void pvCreateMemPool()
		{
			mMemPool.emplace(MemPoolParams(), MemManagerPtr<BaseMemManager>(*this));
		}

	private:
		std::optional<MemPool> mMemPool;
	};

	template<size_t blockCount>
	concept conceptMemPoolUInt32BlockCount = (blockCount > 0);

	template<size_t tBlockCount, conceptMemManager TMemManager>
	requires conceptMemPoolUInt32BlockCount<tBlockCount>
	class MemPoolUInt32
	{
	public:
		typedef TMemManager MemManager;

		static const size_t blockCount = tBlockCount;

		static const uint32_t nullPtr = UIntConst::max32;

	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		typedef ArrayCore<ArrayItemTraits<std::byte*, MemManager>, NestedArraySettings<>> Chunks;

	public:
		explicit MemPoolUInt32(size_t blockSize, MemManager&& memManager, size_t maxTotalBlockCount)
			: mChunks(std::move(memManager)),
			mBlockHead(nullPtr),
			mMaxChunkCount(maxTotalBlockCount / blockCount),
			mBlockSize(internal::UIntMath<>::Max(blockSize, sizeof(uint32_t))),
			mAllocCount(0)
		{
			MOMO_ASSERT(maxTotalBlockCount < size_t{UIntConst::max32});
			if (mBlockSize > UIntConst::maxSize / blockCount)
				MOMO_THROW(std::length_error("Invalid block size"));
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
			return mChunks.GetMemManager();
		}

		MemManager& GetMemManager() noexcept
		{
			return mChunks.GetMemManager();
		}

		template<typename ResObject = void>
		ResObject* GetRealPointer(uint32_t block) noexcept
		{
			return PtrCaster::FromBytePtr<ResObject>(pvGetRealPointer(block));
		}

		[[nodiscard]] uint32_t Allocate()
		{
			if (mBlockHead == nullPtr)
				pvNewChunk();
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
			if (mAllocCount == 0 && mChunks.GetCount() > 2)
				pvClear();
		}

		void DeallocateAll() noexcept
		{
			pvClear();
			mAllocCount = 0;
		}

	private:
		std::byte* pvGetRealPointer(uint32_t block) noexcept
		{
			MOMO_ASSERT(block != nullPtr);
			std::byte* chunk = mChunks[block / blockCount];
			return chunk + (size_t{block} % blockCount) * mBlockSize;
		}

		static uint32_t pvGetNextBlock(void* realPtr) noexcept
		{
			return internal::MemCopyer::FromBuffer<uint32_t>(realPtr);
		}

		static void pvSetNextBlock(uint32_t nextBlock, void* realPtr) noexcept
		{
			internal::MemCopyer::ToBuffer(nextBlock, realPtr);
		}

		void pvNewChunk()
		{
			size_t chunkCount = mChunks.GetCount();
			if (chunkCount >= mMaxChunkCount)
				MOMO_THROW(std::length_error("Invalid chunk count"));
			mChunks.Reserve(chunkCount + 1);
			std::byte* chunk = MemManagerProxy::template Allocate<std::byte>(GetMemManager(),
				pvGetChunkSize());
			for (size_t i = 0; i < blockCount; ++i)
			{
				uint32_t nextBlock = (i + 1 < blockCount)
					? static_cast<uint32_t>(chunkCount * blockCount + i + 1) : nullPtr;
				pvSetNextBlock(nextBlock, chunk + mBlockSize * i);
			}
			mBlockHead = static_cast<uint32_t>(chunkCount * blockCount);
			mChunks.AddBackNogrow(chunk);
		}

		void pvClear() noexcept
		{
			MemManager& memManager = GetMemManager();
			size_t chunkSize = pvGetChunkSize();
			for (std::byte* chunk : mChunks)
				MemManagerProxy::Deallocate(memManager, chunk, chunkSize);
			mBlockHead = nullPtr;
			mChunks.Clear(true);
		}

		size_t pvGetChunkSize() const noexcept
		{
			return blockCount * mBlockSize;
		}

	private:
		Chunks mChunks;
		uint32_t mBlockHead;
		size_t mMaxChunkCount;
		size_t mBlockSize;
		size_t mAllocCount;
	};
}

} // namespace momo
