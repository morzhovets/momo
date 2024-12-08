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
		if (blockCount == 1)
			return std::minmax(blockSize, size_t{1}).second;
		return std::minmax(internal::UIntMath<>::Ceil(blockSize, blockAlignment),
			2 * blockAlignment).second;
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

template<conceptMemPoolParams TParams = MemPoolParams<>,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TSettings = MemPoolSettings>
class MemPool : private TParams
{
public:
	typedef TParams Params;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

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
			allocCount(std::exchange(data.allocCount, 0))
		{
		}

		Data(const Data&) = delete;

		~Data() noexcept = default;

		Data& operator=(Data&& data) noexcept
		{
			MemManagerProxy::Assign(std::move(static_cast<MemManager&>(data)), *this);
			allocCount = std::exchange(data.allocCount, 0);
			return *this;
		}

		Data& operator=(const Data&) = delete;

	public:
		size_t allocCount;
	};

	struct BufferBytes
	{
		int8_t firstFreeBlockIndex;
		int8_t freeBlockCount;
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
		: Params(std::move(params)),
		mData(std::move(memManager)),
		mFreeBufferHead(nullptr),
		mCachedCount(0),
		mCacheHead(nullptr)
	{
		pvCheckParams();
	}

	MemPool(MemPool&& memPool) noexcept
		: Params(std::move(memPool.pvGetParams())),
		mData(std::move(memPool.mData)),
		mFreeBufferHead(std::exchange(memPool.mFreeBufferHead, nullptr)),
		mCachedCount(std::exchange(memPool.mCachedCount, 0)),
		mCacheHead(std::exchange(memPool.mCacheHead, nullptr))
	{
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
		pvGetParams() = std::move(memPool.pvGetParams());
		mData = std::move(memPool.mData);
		mFreeBufferHead = std::exchange(memPool.mFreeBufferHead, nullptr);
		mCachedCount = std::exchange(memPool.mCachedCount, 0);
		mCacheHead = std::exchange(memPool.mCacheHead, nullptr);
		return *this;
	}

	MemPool& operator=(const MemPool&) = delete;

	//void Swap(MemPool& memPool) noexcept
	//{
	//	std::swap(pvGetParams(), memPool.pvGetParams());
	//	std::swap(mData, memPool.mData);
	//	std::swap(mFreeBufferHead, memPool.mFreeBufferHead);
	//	std::swap(mCachedCount, memPool.mCachedCount);
	//	std::swap(mCacheHead, memPool.mCacheHead);
	//}

	//MOMO_FRIEND_SWAP(MemPool)

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
				block = MemManagerProxy::Allocate(GetMemManager(), pvGetBufferSize0());
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
			if (mCachedCount >= Params::GetCachedFreeBlockCount())
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
		return Params::GetBlockCount() > 1;
	}

	void DeallocateAll() noexcept
	{
		MOMO_EXTRA_CHECK(CanDeallocateAll());
		if (mFreeBufferHead == nullptr)
			return;
		while (true)
		{
			Byte* prevBuffer = pvGetPrevBuffer(mFreeBufferHead);
			if (prevBuffer == nullptr)
				break;
			pvDeleteBuffer(prevBuffer);
		}
		while (mFreeBufferHead != nullptr)
		{
			Byte* buffer = mFreeBufferHead;
			mFreeBufferHead = pvGetNextBuffer(buffer);
			pvDeleteBuffer(buffer);
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
			pvDeallocateIf(FastCopyableFunctor<BlockFilter>(blockFilter));
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
		if (memPool.mFreeBufferHead == nullptr)
			return;
		if (mFreeBufferHead == nullptr)
		{
			mFreeBufferHead = memPool.mFreeBufferHead;
			memPool.mFreeBufferHead = nullptr;
			return;
		}
		while (true)
		{
			Byte* buffer = pvGetPrevBuffer(memPool.mFreeBufferHead);
			if (buffer == nullptr)
				break;
			Byte* prevBuffer = pvGetPrevBuffer(buffer);
			Byte* nextBuffer = memPool.mFreeBufferHead;
			if (prevBuffer != nullptr)
				pvSetNextBuffer(prevBuffer, nextBuffer);
			pvSetPrevBuffer(nextBuffer, prevBuffer);
			prevBuffer = pvGetPrevBuffer(mFreeBufferHead);
			nextBuffer = mFreeBufferHead;
			pvSetPrevBuffer(buffer, prevBuffer);
			pvSetNextBuffer(buffer, nextBuffer);
			if (prevBuffer != nullptr)
				pvSetNextBuffer(prevBuffer, nextBuffer);
			pvSetPrevBuffer(nextBuffer, prevBuffer);
		}
		Byte* buffer = mFreeBufferHead;
		while (true)
		{
			Byte* nextBuffer = pvGetNextBuffer(buffer);
			if (nextBuffer == nullptr)
				break;
			buffer = nextBuffer;
		}
		pvSetNextBuffer(buffer, memPool.mFreeBufferHead);
		pvSetPrevBuffer(memPool.mFreeBufferHead, buffer);
		memPool.mFreeBufferHead = nullptr;
	}

private:
	Params& pvGetParams() noexcept
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
			throw std::length_error("Invalid block size");
	}

	bool pvUseCache() const noexcept
	{
		return Params::GetCachedFreeBlockCount() > 0
			&& Params::GetBlockSize() >= sizeof(Byte*);	//?
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
		if (Params::GetBlockCount() > 1)
			pvDeleteBlock(block);
		else if (pvGetAlignmentAddend() == 0)
			MemManagerProxy::Deallocate(GetMemManager(), block, pvGetBufferSize0());
		else
			pvDeleteBlock1(block);
	}

	size_t pvGetAlignmentAddend() const noexcept
	{
		const size_t blockAlignment = Params::GetBlockAlignment();
		return blockAlignment - std::minmax(size_t{internal::UIntConst::maxAllocAlignment},
			size_t{1} << std::countr_zero(blockAlignment)).first;
	}

	size_t pvGetBufferSize0() const noexcept
	{
		return std::minmax(Params::GetBlockSize(), Params::GetBlockAlignment()).second;
	}

	Byte* pvNewBlock1()
	{
		const uintptr_t uipBlockAlignment = uintptr_t{Params::GetBlockAlignment()};
		Byte* buffer = MemManagerProxy::template Allocate<Byte>(
			GetMemManager(), pvGetBufferSize1());
		uintptr_t uipBuffer = internal::PtrCaster::ToUInt(buffer);
		uintptr_t uipBlock = internal::UIntMath<uintptr_t>::Ceil(uipBuffer, uipBlockAlignment);
		size_t offset = static_cast<size_t>(uipBlock - uipBuffer);
		MOMO_ASSERT(offset < 256);
		Byte* block = buffer + offset;
		internal::MemCopyer::ToBuffer(static_cast<uint8_t>(offset),
			block + Params::GetBlockSize());
		return block;
	}

	void pvDeleteBlock1(Byte* block) noexcept
	{
		size_t offset = size_t{internal::MemCopyer::FromBuffer<uint8_t>(
			block + Params::GetBlockSize())};
		Byte* buffer = block - offset;
		MemManagerProxy::Deallocate(GetMemManager(), buffer, pvGetBufferSize1());
	}

	size_t pvGetBufferSize1() const noexcept
	{
		return Params::GetBlockSize() + pvGetAlignmentAddend() + 1;
	}

	Byte* pvNewBlock()
	{
		if (mFreeBufferHead == nullptr)
			mFreeBufferHead = pvNewBuffer();
		BufferBytes bytes = pvGetBufferBytes(mFreeBufferHead);
		Byte* nextBuffer = pvGetNextBuffer(mFreeBufferHead);
		if (bytes.freeBlockCount == int8_t{1} && nextBuffer == nullptr)
		{
			nextBuffer = pvNewBuffer();
			pvSetNextBuffer(mFreeBufferHead, nextBuffer);
			pvSetPrevBuffer(nextBuffer, mFreeBufferHead);
		}
		Byte* block = pvGetBlock(mFreeBufferHead, bytes.firstFreeBlockIndex);
		bytes.firstFreeBlockIndex = pvGetNextFreeBlockIndex(block);
		--bytes.freeBlockCount;
		pvSetBufferBytes(mFreeBufferHead, bytes);
		if (bytes.freeBlockCount == int8_t{0})
			mFreeBufferHead = nextBuffer;
		return block;
	}

	void pvDeleteBlock(Byte* block) noexcept
	{
		Byte* buffer;
		int8_t blockIndex = pvGetBlockIndex(block, buffer);
		pvDeleteBlock(block, buffer, blockIndex);
	}

	void pvDeleteBlock(Byte* block, Byte* buffer, int8_t blockIndex) noexcept
	{
		BufferBytes bytes = pvGetBufferBytes(buffer);
		pvSetNextFreeBlockIndex(block, bytes.firstFreeBlockIndex);
		bytes.firstFreeBlockIndex = blockIndex;
		++bytes.freeBlockCount;
		pvSetBufferBytes(buffer, bytes);
		size_t freeBlockCount = static_cast<size_t>(bytes.freeBlockCount);
		if (freeBlockCount == 1)
			pvMoveBufferToHead(buffer);
		if (freeBlockCount == Params::GetBlockCount())
		{
			bool del = true;
			if (buffer == mFreeBufferHead)
			{
				Byte* nextBuffer = pvGetNextBuffer(buffer);
				del = (nextBuffer != nullptr);
				if (del)
					mFreeBufferHead = nextBuffer;
			}
			if (del)
				pvDeleteBuffer(buffer);
		}
	}

	int8_t pvGetNextFreeBlockIndex(Byte* block) const noexcept
	{
		return internal::MemCopyer::FromBuffer<int8_t>(block);
	}

	void pvSetNextFreeBlockIndex(Byte* block, int8_t nextFreeBlockIndex) noexcept
	{
		internal::MemCopyer::ToBuffer(nextFreeBlockIndex, block);
	}

	Byte* pvGetBlock(Byte* buffer, int8_t index) const noexcept
	{
		return buffer + ptrdiff_t{index} * static_cast<ptrdiff_t>(Params::GetBlockSize())
			+ (static_cast<ptrdiff_t>(Params::GetBlockAlignment()) & -ptrdiff_t{index >= 0});
	}

	int8_t pvGetBlockIndex(Byte* block, Byte*& buffer) const noexcept
	{
		const uintptr_t uipBlockSize = uintptr_t{Params::GetBlockSize()};
		const uintptr_t uipBlockAlignment = uintptr_t{Params::GetBlockAlignment()};
		const uintptr_t uipBlockCount = uintptr_t{Params::GetBlockCount()};
		uintptr_t uipBlock = internal::PtrCaster::ToUInt(block);
		MOMO_ASSERT(uipBlock % uipBlockAlignment == 0);
		ptrdiff_t dir = static_cast<ptrdiff_t>(((uipBlock % uipBlockSize) / uipBlockAlignment) % 2);
		ptrdiff_t index = static_cast<ptrdiff_t>((uipBlock / uipBlockSize) % uipBlockCount)
			- (static_cast<ptrdiff_t>(uipBlockCount) & (dir - 1));
		buffer = block - index * static_cast<ptrdiff_t>(uipBlockSize)
			- (static_cast<ptrdiff_t>(uipBlockAlignment) & -dir);
		return static_cast<int8_t>(index);
	}

	MOMO_NOINLINE Byte* pvNewBuffer()
	{
		const uintptr_t uipBlockSize = uintptr_t{Params::GetBlockSize()};
		const uintptr_t uipBlockAlignment = uintptr_t{Params::GetBlockAlignment()};
		const uintptr_t uipBlockCount = uintptr_t{Params::GetBlockCount()};
		Byte* begin = MemManagerProxy::template Allocate<Byte>(
			GetMemManager(), pvGetBufferSize());
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
		Byte* buffer;
		int8_t blockIndex = pvGetBlockIndex(block, buffer);
		pvSetFirstBlockIndex(buffer, blockIndex);
		BufferBytes bytes;
		bytes.firstFreeBlockIndex = blockIndex;
		bytes.freeBlockCount = static_cast<int8_t>(uipBlockCount);
		pvSetBufferBytes(buffer, bytes);
		pvSetPrevBuffer(buffer, nullptr);
		pvSetNextBuffer(buffer, nullptr);
		pvSetBeginOffset(buffer, static_cast<uint16_t>(beginOffset));
		for (uintptr_t i = 1; i < uipBlockCount; ++i)
		{
			++blockIndex;
			pvSetNextFreeBlockIndex(block, blockIndex);
			block = pvGetBlock(buffer, blockIndex);
		}
		pvSetNextFreeBlockIndex(block, int8_t{-128});
		return buffer;
	}

	MOMO_NOINLINE void pvDeleteBuffer(Byte* buffer) noexcept
	{
		MOMO_ASSERT(buffer != mFreeBufferHead);
		Byte* prevBuffer = pvGetPrevBuffer(buffer);
		Byte* nextBuffer = pvGetNextBuffer(buffer);
		if (prevBuffer != nullptr)
			pvSetNextBuffer(prevBuffer, nextBuffer);
		if (nextBuffer != nullptr)
			pvSetPrevBuffer(nextBuffer, prevBuffer);
		Byte* begin = pvGetBlock(buffer, pvGetFirstBlockIndex(buffer))
			- size_t{pvGetBeginOffset(buffer)};
		MemManagerProxy::Deallocate(GetMemManager(), begin, pvGetBufferSize());
	}

	void pvMoveBufferToHead(Byte* buffer) noexcept
	{
		Byte* headPrevBuffer = pvGetPrevBuffer(mFreeBufferHead);
		MOMO_ASSERT(headPrevBuffer != nullptr);
		if (buffer != headPrevBuffer)
		{
			Byte* prevBuffer = pvGetPrevBuffer(buffer);
			Byte* nextBuffer = pvGetNextBuffer(buffer);
			MOMO_ASSERT(nextBuffer != nullptr);
			pvSetPrevBuffer(nextBuffer, prevBuffer);
			if (prevBuffer != nullptr)
				pvSetNextBuffer(prevBuffer, nextBuffer);
			pvSetPrevBuffer(buffer, headPrevBuffer);
			pvSetNextBuffer(buffer, mFreeBufferHead);
			pvSetPrevBuffer(mFreeBufferHead, buffer);
			pvSetNextBuffer(headPrevBuffer, buffer);
		}
		mFreeBufferHead = buffer;
	}

	size_t pvGetBufferSize() const noexcept
	{
		const size_t blockSize = Params::GetBlockSize();
		const size_t blockAlignment = Params::GetBlockAlignment();
		return Params::GetBlockCount() * blockSize + pvGetAlignmentAddend()
			+ (2 + (blockSize / blockAlignment) % 2) * blockAlignment
			+ (pvIsBufferBytesNear() ? 0 : sizeof(BufferBytes))
			+ 2 * sizeof(Byte*) + sizeof(uint16_t);
	}

	template<internal::conceptPredicate<void*> BlockFilter>
	void pvDeallocateIf(FastCopyableFunctor<BlockFilter> blockFilter)
	{
		Byte* buffer = mFreeBufferHead;
		while (true)
		{
			Byte* nextBuffer = pvGetNextBuffer(buffer);
			pvDeleteBlocks(buffer, blockFilter);
			if (nextBuffer == nullptr)
				break;
			buffer = nextBuffer;
		}
		buffer = pvGetPrevBuffer(mFreeBufferHead);
		while (buffer != nullptr)
		{
			Byte* prevBuffer = pvGetPrevBuffer(buffer);
			pvDeleteBlocks(buffer, blockFilter);
			buffer = prevBuffer;
		}
	}

	template<internal::conceptPredicate<void*> BlockFilter>
	void pvDeleteBlocks(Byte* buffer, FastCopyableFunctor<BlockFilter> blockFilter)
	{
		int8_t firstBlockIndex = pvGetFirstBlockIndex(buffer);
		uint8_t freeBlockBits[16] = {};
		BufferBytes bytes = pvGetBufferBytes(buffer);
		int8_t freeBlockIndex = bytes.firstFreeBlockIndex;
		for (int8_t i = 0; i < bytes.freeBlockCount; ++i)
		{
			internal::UIntMath<uint8_t>::SetBit(freeBlockBits,
				static_cast<size_t>(freeBlockIndex - firstBlockIndex));
			freeBlockIndex = pvGetNextFreeBlockIndex(pvGetBlock(buffer, freeBlockIndex));
		}
		for (size_t i = 0; i < Params::GetBlockCount(); ++i)
		{
			if (internal::UIntMath<uint8_t>::GetBit(freeBlockBits, i))
				continue;
			int8_t blockIndex = firstBlockIndex + static_cast<int8_t>(i);
			Byte* block = pvGetBlock(buffer, blockIndex);
			if (!blockFilter(static_cast<void*>(block)))
				continue;
			pvDeleteBlock(block, buffer, blockIndex);
			--mData.allocCount;
		}
	}

	int8_t pvGetFirstBlockIndex(Byte* buffer) const noexcept
	{
		return internal::MemCopyer::FromBuffer<int8_t>(buffer);
	}

	void pvSetFirstBlockIndex(Byte* buffer, int8_t firstBlockIndex) noexcept
	{
		return internal::MemCopyer::ToBuffer(firstBlockIndex, buffer);
	}

	BufferBytes pvGetBufferBytes(Byte* buffer) noexcept
	{
		return internal::MemCopyer::FromBuffer<BufferBytes>(pvGetBufferBytesPosition(buffer));
	}

	void pvSetBufferBytes(Byte* buffer, BufferBytes bufferBytes) noexcept
	{
		return internal::MemCopyer::ToBuffer(bufferBytes, pvGetBufferBytesPosition(buffer));
	}

	Byte* pvGetBufferBytesPosition(Byte* buffer) const noexcept
	{
		return pvIsBufferBytesNear() ? buffer + 1 : pvGetBlocksEndPosition(buffer);
	}

	Byte* pvGetPrevBuffer(Byte* buffer) const noexcept
	{
		return internal::MemCopyer::FromBuffer<Byte*>(pvGetPrevBufferPosition(buffer));
	}

	void pvSetPrevBuffer(Byte* buffer, Byte* prevBuffer) noexcept
	{
		return internal::MemCopyer::ToBuffer(prevBuffer, pvGetPrevBufferPosition(buffer));
	}

	Byte* pvGetPrevBufferPosition(Byte* buffer) const noexcept
	{
		return pvGetBlocksEndPosition(buffer) + (pvIsBufferBytesNear() ? 0 : sizeof(BufferBytes));
	}

	Byte* pvGetNextBuffer(Byte* buffer) const noexcept
	{
		return internal::MemCopyer::FromBuffer<Byte*>(pvGetNextBufferPosition(buffer));
	}

	void pvSetNextBuffer(Byte* buffer, Byte* nextBuffer) noexcept
	{
		return internal::MemCopyer::ToBuffer(nextBuffer, pvGetNextBufferPosition(buffer));
	}

	Byte* pvGetNextBufferPosition(Byte* buffer) const noexcept
	{
		return pvGetPrevBufferPosition(buffer) + sizeof(Byte*);
	}

	uint16_t pvGetBeginOffset(Byte* buffer) noexcept
	{
		return internal::MemCopyer::FromBuffer<uint16_t>(pvGetBeginOffsetPosition(buffer));
	}

	void pvSetBeginOffset(Byte* buffer, uint16_t beginOffset) noexcept
	{
		return internal::MemCopyer::ToBuffer(beginOffset, pvGetBeginOffsetPosition(buffer));
	}

	Byte* pvGetBeginOffsetPosition(Byte* buffer) const noexcept
	{
		return pvGetNextBufferPosition(buffer) + sizeof(Byte*);
	}

	Byte* pvGetBlocksEndPosition(Byte* buffer) const noexcept
	{
		return buffer + Params::GetBlockAlignment() + Params::GetBlockSize()
			* (Params::GetBlockCount() - static_cast<size_t>(-pvGetFirstBlockIndex(buffer)));
	}

	bool pvIsBufferBytesNear() const noexcept
	{
		return Params::GetBlockAlignment() >= sizeof(BufferBytes) + 1;
	}

private:
	Data mData;
	Byte* mFreeBufferHead;
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
			if (mLazyHead != nullptr)
			{
				void* block = mLazyHead.exchange(nullptr);
				if (block != nullptr)
				{
					pvFlushDeallocate(MemCopyer::FromBuffer<void*>(block));
					return static_cast<ResObject*>(block);
				}
			}
			return mMemPool.template Allocate<ResObject>();
		}

		void Deallocate(void* block) noexcept
		{
			if (mLazyHead != nullptr)
				pvFlushDeallocate(mLazyHead.exchange(nullptr));
			mMemPool.Deallocate(block);
		}

		void DeallocateLazy(void* block) noexcept
		{
			while (true)
			{
				void* lazyHead = mLazyHead;
				MemCopyer::ToBuffer(lazyHead, block);
				if (mLazyHead.compare_exchange_weak(lazyHead, block))
					break;
			}
		}

	private:
		void pvFlushDeallocate(void* block) noexcept
		{
			while (block != nullptr)
			{
				void* nextBlock = MemCopyer::FromBuffer<void*>(block);
				mMemPool.Deallocate(block);
				block = nextBlock;
			}
		}

	private:
		MemPool mMemPool;
		std::atomic<void*> mLazyHead;
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

		typedef Array<std::byte*, MemManager, ArrayItemTraits<std::byte*, MemManager>,
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
			std::byte* buffer = mBuffers[block / blockCount];
			void* realPtr = buffer + (size_t{block} % blockCount) * mBlockSize;
			return static_cast<ResObject*>(realPtr);
		}

		[[nodiscard]] uint32_t Allocate()
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
			std::byte* buffer = MemManagerProxy::template Allocate<std::byte>(GetMemManager(),
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
			for (std::byte* buffer : mBuffers)
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
