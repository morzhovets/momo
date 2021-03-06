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
	MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

	static const size_t blockAlignment = tBlockAlignment;
	MOMO_STATIC_ASSERT(0 < blockAlignment && blockAlignment <= 1024);

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
class MemPool : private TParams, private internal::MemManagerWrapper<TMemManager>
{
public:
	typedef TParams Params;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<Params>::value);
	MOMO_STATIC_ASSERT(std::is_nothrow_move_assignable<Params>::value);

private:
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;
	typedef internal::MemManagerWrapper<MemManager> MemManagerWrapper;

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

	static const uintptr_t nullPtr = internal::UIntConst::nullPtr;

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
		MemManagerWrapper(std::move(memManager)),
		mFreeBufferHead(nullPtr),
		mAllocCount(0),
		mCachedCount(0),
		mCacheHead(nullptr)
	{
		pvCheckParams();
	}

	MemPool(MemPool&& memPool) noexcept
		: Params(std::move(memPool.pvGetParams())),
		MemManagerWrapper(std::move(memPool.pvGetMemManagerWrapper())),
		mFreeBufferHead(memPool.mFreeBufferHead),
		mAllocCount(memPool.mAllocCount),
		mCachedCount(memPool.mCachedCount),
		mCacheHead(memPool.mCacheHead)
	{
		memPool.mFreeBufferHead = nullPtr;
		memPool.mAllocCount = 0;
		memPool.mCachedCount = 0;
		memPool.mCacheHead = nullptr;
	}

	MemPool(const MemPool&) = delete;

	~MemPool() noexcept
	{
		MOMO_EXTRA_CHECK(mAllocCount == 0);
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
		std::swap(pvGetMemManagerWrapper(), memPool.pvGetMemManagerWrapper());
		std::swap(mFreeBufferHead, memPool.mFreeBufferHead);
		std::swap(mAllocCount, memPool.mAllocCount);
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
		return static_cast<const MemManagerWrapper&>(*this).GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return pvGetMemManagerWrapper().GetMemManager();
	}

	template<typename ResObject = void>
	MOMO_NODISCARD ResObject* Allocate()
	{
		void* block;
		if (pvUseCache() && mCachedCount > 0)
		{
			block = mCacheHead;
			mCacheHead = internal::PtrCaster::FromBuffer(block);
			--mCachedCount;
		}
		else
		{
			if (Params::blockCount > 1)
				block = internal::PtrCaster::FromUInt(pvNewBlock());
			else if (pvGetAlignmentAddend() == 0)
				block = MemManagerProxy::Allocate(GetMemManager(), pvGetBufferSize0());
			else
				block = internal::PtrCaster::FromUInt(pvNewBlock1());
		}
		++mAllocCount;
		return static_cast<ResObject*>(block);
	}

	void Deallocate(void* block) noexcept
	{
		MOMO_ASSERT(block != nullptr);
		MOMO_ASSERT(mAllocCount > 0);
		if (pvUseCache())
		{
			if (mCachedCount >= Params::cachedFreeBlockCount)
				pvFlushDeallocate();
			internal::PtrCaster::ToBuffer(mCacheHead, block);
			mCacheHead = block;
			++mCachedCount;
		}
		else
		{
			pvDeleteBlock(block);
		}
		--mAllocCount;
	}

	size_t GetAllocateCount() const noexcept
	{
		return mAllocCount;
	}

	bool CanDeallocateAll() const noexcept
	{
		return Params::blockCount > 1;
	}

	void DeallocateAll() noexcept
	{
		MOMO_EXTRA_CHECK(CanDeallocateAll());
		if (mFreeBufferHead == nullPtr)
			return;
		while (true)
		{
			BufferPointers& pointers = pvGetBufferPointers(mFreeBufferHead);
			if (pointers.prevBuffer != nullPtr)
				pvDeleteBuffer(pointers.prevBuffer);
			else if (pointers.nextBuffer != nullPtr)
				pvDeleteBuffer(pointers.nextBuffer);
			else
				break;
		}
		pvDeleteBuffer(mFreeBufferHead);
		mFreeBufferHead = nullPtr;
		mAllocCount = 0;
		mCachedCount = 0;
		mCacheHead = nullptr;
	}

	void MergeFrom(MemPool& memPool)
	{
		if (this == &memPool)
			return;
		MOMO_CHECK(Params::blockSize == memPool.GetBlockSize());
		MOMO_CHECK(Params::blockAlignment == memPool.GetBlockAlignment());
		MOMO_CHECK(Params::blockCount == memPool.GetBlockCount());
		MOMO_CHECK(MemManagerProxy::IsEqual(GetMemManager(), memPool.GetMemManager()));
		if (memPool.pvUseCache())
			memPool.pvFlushDeallocate();
		mAllocCount += memPool.mAllocCount;
		memPool.mAllocCount = 0;
		if (memPool.mFreeBufferHead == nullPtr)
			return;
		if (mFreeBufferHead == nullPtr)
		{
			mFreeBufferHead = memPool.mFreeBufferHead;
			memPool.mFreeBufferHead = nullPtr;
			return;
		}
		while (true)
		{
			uintptr_t buffer = pvGetBufferPointers(memPool.mFreeBufferHead).prevBuffer;
			if (buffer == nullPtr)
				break;
			BufferPointers& pointers = pvGetBufferPointers(buffer);
			if (pointers.prevBuffer != nullPtr)
				pvGetBufferPointers(pointers.prevBuffer).nextBuffer = pointers.nextBuffer;
			pvGetBufferPointers(pointers.nextBuffer).prevBuffer = pointers.prevBuffer;
			pointers.prevBuffer = pvGetBufferPointers(mFreeBufferHead).prevBuffer;
			pointers.nextBuffer = mFreeBufferHead;
			if (pointers.prevBuffer != nullPtr)
				pvGetBufferPointers(pointers.prevBuffer).nextBuffer = buffer;
			pvGetBufferPointers(mFreeBufferHead).prevBuffer = buffer;
		}
		uintptr_t buffer = mFreeBufferHead;
		while (true)
		{
			BufferPointers& pointers = pvGetBufferPointers(buffer);
			if (pointers.nextBuffer == nullPtr)
				break;
			buffer = pointers.nextBuffer;
		}
		pvGetBufferPointers(buffer).nextBuffer = memPool.mFreeBufferHead;
		pvGetBufferPointers(memPool.mFreeBufferHead).prevBuffer = buffer;
		memPool.mFreeBufferHead = nullPtr;
	}

private:
	Params& pvGetParams() noexcept
	{
		return *this;
	}

	MemManagerWrapper& pvGetMemManagerWrapper() noexcept
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
		size_t maxBlockSize = (internal::UIntConst::maxSize
			- 2 - 3 * sizeof(void*) - 4 * Params::blockAlignment) / Params::blockCount;
		if (Params::blockSize > maxBlockSize)
			throw std::length_error("Invalid block size");
	}

	bool pvUseCache() const noexcept
	{
		return Params::cachedFreeBlockCount > 0 && Params::blockSize >= sizeof(void*);	//?
	}

	MOMO_NOINLINE void pvFlushDeallocate() noexcept
	{
		while (mCachedCount > 0)
		{
			void* block = mCacheHead;
			mCacheHead = internal::PtrCaster::FromBuffer(block);
			pvDeleteBlock(block);
			--mCachedCount;
		}
	}

	void pvDeleteBlock(void* block) noexcept
	{
		if (Params::blockCount > 1)
			pvDeleteBlock(internal::PtrCaster::ToUInt(block));
		else if (pvGetAlignmentAddend() == 0)
			MemManagerProxy::Deallocate(GetMemManager(), block, pvGetBufferSize0());
		else
			pvDeleteBlock1(internal::PtrCaster::ToUInt(block));
	}

	size_t pvGetAlignmentAddend() const noexcept
	{
		static const size_t maxAllocAlignment = internal::UIntConst::maxAllocAlignment;
		size_t addend = Params::blockAlignment;
		if (SMath::HasSingleBit(Params::blockAlignment))
			addend -= std::minmax(size_t{maxAllocAlignment}, size_t{Params::blockAlignment}).first;
		else
			addend -= SMath::GCD(maxAllocAlignment, Params::blockAlignment);
		return addend;
	}

	size_t pvGetBufferSize0() const noexcept
	{
		return std::minmax(size_t{Params::blockSize}, size_t{Params::blockAlignment}).second;
	}

	uintptr_t pvNewBlock1()
	{
		uintptr_t begin = internal::PtrCaster::ToUInt(
			MemManagerProxy::Allocate(GetMemManager(), pvGetBufferSize1()));
		uintptr_t block = PMath::Ceil(begin, uintptr_t{Params::blockAlignment});
		pvGetBufferBegin1(block) = begin;
		return block;
	}

	void pvDeleteBlock1(uintptr_t block) noexcept
	{
		uintptr_t begin = pvGetBufferBegin1(block);
		MemManagerProxy::Deallocate(GetMemManager(), internal::PtrCaster::FromUInt(begin),
			pvGetBufferSize1());
	}

	size_t pvGetBufferSize1() const noexcept
	{
		size_t bufferUsefulSize = Params::blockSize + pvGetAlignmentAddend();
		return SMath::Ceil(bufferUsefulSize, sizeof(void*)) + sizeof(void*);
	}

	uintptr_t& pvGetBufferBegin1(uintptr_t block) noexcept
	{
		return *internal::PtrCaster::FromUInt<uintptr_t>(
			PMath::Ceil(block + uintptr_t{Params::blockSize}, uintptr_t{sizeof(void*)}));
	}

	uintptr_t pvNewBlock()
	{
		if (mFreeBufferHead == nullPtr)
			mFreeBufferHead = pvNewBuffer();
		BufferBytes& bytes = pvGetBufferBytes(mFreeBufferHead);
		BufferPointers& pointers = pvGetBufferPointers(mFreeBufferHead);
		if (bytes.freeBlockCount == int8_t{1} && pointers.nextBuffer == nullPtr)
		{
			pointers.nextBuffer = pvNewBuffer();
			pvGetBufferPointers(pointers.nextBuffer).prevBuffer = mFreeBufferHead;
		}
		uintptr_t block = pvGetBlock(mFreeBufferHead, bytes.firstFreeBlockIndex);
		bytes.firstFreeBlockIndex = pvGetNextFreeBlockIndex(block);
		--bytes.freeBlockCount;
		if (bytes.freeBlockCount == int8_t{0})
			mFreeBufferHead = pointers.nextBuffer;
		return block;
	}

	void pvDeleteBlock(uintptr_t block) noexcept
	{
		uintptr_t buffer;
		int8_t blockIndex = pvGetBlockIndex(block, buffer);
		BufferBytes& bytes = pvGetBufferBytes(buffer);
		pvGetNextFreeBlockIndex(block) = bytes.firstFreeBlockIndex;
		bytes.firstFreeBlockIndex = blockIndex;
		++bytes.freeBlockCount;
		size_t freeBlockCount = static_cast<size_t>(bytes.freeBlockCount);
		if (freeBlockCount == 1)
			pvMoveBuffer(buffer);
		if (freeBlockCount == Params::blockCount)
		{
			bool del = true;
			if (buffer == mFreeBufferHead)
			{
				uintptr_t nextBuffer = pvGetBufferPointers(buffer).nextBuffer;
				del = (nextBuffer != nullPtr);
				if (del)
					mFreeBufferHead = nextBuffer;
			}
			if (del)
				pvDeleteBuffer(buffer);
		}
	}

	int8_t& pvGetNextFreeBlockIndex(uintptr_t block) noexcept
	{
		return *internal::PtrCaster::FromUInt<int8_t>(block);
	}

	uintptr_t pvGetBlock(uintptr_t buffer, int8_t index) const noexcept
	{
		return static_cast<uintptr_t>(static_cast<intptr_t>(buffer)
			+ intptr_t{index} * static_cast<intptr_t>(Params::blockSize)
			+ (static_cast<intptr_t>(Params::blockAlignment) & -intptr_t{index >= 0}));
	}

	int8_t pvGetBlockIndex(uintptr_t block, uintptr_t& buffer) const noexcept
	{
		const uintptr_t uipBlockSize = uintptr_t{Params::blockSize};
		const uintptr_t uipBlockAlignment = uintptr_t{Params::blockAlignment};
		MOMO_ASSERT(block % uipBlockAlignment == 0);
		intptr_t dir = static_cast<intptr_t>(((block % uipBlockSize) / uipBlockAlignment) % 2);
		intptr_t index = static_cast<intptr_t>((block / uipBlockSize) % uintptr_t{Params::blockCount})
			- (static_cast<intptr_t>(Params::blockCount) & (dir - 1));
		buffer = static_cast<uintptr_t>(static_cast<intptr_t>(block)
			- index * static_cast<intptr_t>(uipBlockSize)
			- (static_cast<intptr_t>(uipBlockAlignment) & -dir));
		return static_cast<int8_t>(index);
	}

	MOMO_NOINLINE uintptr_t pvNewBuffer()
	{
		const uintptr_t uipBlockSize = uintptr_t{Params::blockSize};
		const uintptr_t uipBlockAlignment = uintptr_t{Params::blockAlignment};
		uintptr_t begin = internal::PtrCaster::ToUInt(
			MemManagerProxy::Allocate(GetMemManager(), pvGetBufferSize()));
		uintptr_t block = PMath::Ceil(begin, uipBlockAlignment);
		block += (block % uipBlockSize) % (2 * uipBlockAlignment);
		if ((block + uipBlockAlignment) % uipBlockSize == 0)
			block += uipBlockAlignment;
		if ((block / uipBlockSize) % uintptr_t{Params::blockCount} == 0)
			block += uipBlockAlignment;
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
			block = pvGetBlock(buffer, blockIndex);
		}
		pvGetNextFreeBlockIndex(block) = int8_t{-128};
		return buffer;
	}

	MOMO_NOINLINE void pvDeleteBuffer(uintptr_t buffer) noexcept
	{
		BufferPointers& pointers = pvGetBufferPointers(buffer);
		if (pointers.prevBuffer != nullPtr)
			pvGetBufferPointers(pointers.prevBuffer).nextBuffer = pointers.nextBuffer;
		if (pointers.nextBuffer != nullPtr)
			pvGetBufferPointers(pointers.nextBuffer).prevBuffer = pointers.prevBuffer;
		MemManagerProxy::Deallocate(GetMemManager(),
			internal::PtrCaster::FromUInt(pointers.begin), pvGetBufferSize());
	}

	void pvMoveBuffer(uintptr_t buffer) noexcept	//?
	{
		BufferPointers& headPointers = pvGetBufferPointers(mFreeBufferHead);
		MOMO_ASSERT(headPointers.prevBuffer != nullPtr);
		if (buffer != headPointers.prevBuffer)
		{
			BufferPointers& pointers = pvGetBufferPointers(buffer);
			MOMO_ASSERT(pointers.nextBuffer != nullPtr);
			if (pointers.prevBuffer != nullPtr)
				pvGetBufferPointers(pointers.prevBuffer).nextBuffer = pointers.nextBuffer;
			pvGetBufferPointers(pointers.nextBuffer).prevBuffer = pointers.prevBuffer;
			pointers.prevBuffer = headPointers.prevBuffer;
			pointers.nextBuffer = mFreeBufferHead;
			headPointers.prevBuffer = buffer;
			pvGetBufferPointers(pointers.prevBuffer).nextBuffer = buffer;
		}
		mFreeBufferHead = buffer;
	}

	size_t pvGetBufferSize() const noexcept
	{
		size_t bufferUsefulSize = Params::blockCount * Params::blockSize + pvGetAlignmentAddend()
			+ (2 + (Params::blockSize / Params::blockAlignment) % 2) * Params::blockAlignment;
		return SMath::Ceil(bufferUsefulSize, sizeof(void*)) + 3 * sizeof(void*)
			+ ((Params::blockAlignment <= 2) ? 2 : 0);
	}

	int8_t& pvGetFirstBlockIndex(uintptr_t buffer) noexcept
	{
		return *internal::PtrCaster::FromUInt<int8_t>(buffer);
	}

	BufferBytes& pvGetBufferBytes(uintptr_t buffer) noexcept
	{
		if (Params::blockAlignment > 2)
		{
			return *internal::PtrCaster::FromUInt<BufferBytes>(buffer + 1);
		}
		else
		{
			return *internal::PtrCaster::Shift<BufferBytes>(
				&pvGetBufferPointers(buffer), sizeof(BufferPointers));
		}
	}

	BufferPointers& pvGetBufferPointers(uintptr_t buffer) noexcept
	{
		size_t offset = Params::blockCount;
		offset -= static_cast<size_t>(-pvGetFirstBlockIndex(buffer));	// gcc warning
		return *internal::PtrCaster::FromUInt<BufferPointers>(
			PMath::Ceil(buffer + uintptr_t{Params::blockAlignment + Params::blockSize * offset},
			uintptr_t{sizeof(void*)}));
	}

private:
	uintptr_t mFreeBufferHead;
	size_t mAllocCount;
	size_t mCachedCount;
	void* mCacheHead;
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

		static const uint32_t nullPtr = UIntConst::max32;

	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		typedef Array<char*, MemManager, ArrayItemTraits<char*, MemManager>,
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
			char* buffer = mBuffers[block / blockCount];
			void* realPtr = buffer + (size_t{block} % blockCount) * mBlockSize;
			return static_cast<ResObject*>(realPtr);
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
			uint32_t nextBlock = 0;
			std::memcpy(&nextBlock, realPtr, sizeof(uint32_t));
			return nextBlock;
		}

		static void pvSetNextBlock(uint32_t nextBlock, void* realPtr) noexcept
		{
			std::memcpy(realPtr, &nextBlock, sizeof(uint32_t));
		}

		void pvNewBuffer()
		{
			size_t bufferCount = mBuffers.GetCount();
			if (bufferCount >= mMaxBufferCount)
				throw std::length_error("Invalid buffer count");
			mBuffers.Reserve(bufferCount + 1);
			char* buffer = MemManagerProxy::template Allocate<char>(GetMemManager(),
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
			for (char* buffer : mBuffers)
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
