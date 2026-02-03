/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/SemiList.h

  namespace momo:
    class SemiListItemTraits
    class SemiListSettings
    class SemiListCore
    class SemiList

\**********************************************************/

#pragma once

#include "ObjectManager.h"
#include "IteratorUtility.h"
#include "KeyUtility.h"

namespace momo
{

namespace internal
{
	template<typename TItem>
	class ListChunker
	{
	public:
		typedef TItem Item;

		static const size_t logItemCount = 3;	//?

		typedef std::byte* Chunk;
		typedef uintptr_t Block;
		typedef uint64_t State;

		static constexpr Chunk nullChunk = nullptr;
		static const Block nullBlock = UIntConst::nullPtr;

	private:
		static const size_t itemCount = size_t{1} << logItemCount;
		static const uintptr_t maskIndex = uintptr_t{itemCount - 1};

	public:
		template<conceptMemManager MemManager>
		static Chunk NewChunk(MemManager& memManager)
		{
			std::byte* begin = MemManagerProxy<MemManager>::template Allocate<std::byte>(
				memManager, pvGetChunkSize());
			return pvNewChunk(begin);
		}

		template<conceptMemManager MemManager>
		static void DeleteChunk(MemManager& memManager, Chunk chunk) noexcept
		{
			std::byte* begin = chunk - pvGetChunkHeadSize();
			MemManagerProxy<MemManager>::Deallocate(memManager, begin, pvGetChunkSize());
		}

		static Block NewBlock(Chunk chunk, size_t blockIndex) noexcept
		{
			MOMO_ASSERT(blockIndex < itemCount);
			State state = GetState(chunk);
			//MOMO_ASSERT
			state += State{1} << blockIndex;
			pvSetState(chunk, state);
			return GetBlock(chunk, blockIndex);
		}

		static void DeleteBlock(Block block) noexcept
		{
			Chunk chunk = GetChunk(block);
			size_t blockIndex = GetBlockIndex(block);
			State state = GetState(chunk);
			//MOMO_ASSERT
			state -= State{1} << blockIndex;
			pvSetState(chunk, state);
		}

		static Item* GetItemPtr(Block block) noexcept
		{
			Chunk chunk = GetChunk(block);
			size_t blockIndex = GetBlockIndex(block);
			return PtrCaster::FromBytePtr<Item>(chunk + blockIndex * sizeof(Item));
		}

		static Chunk GetChunk(Block block) noexcept
		{
			return PtrCaster::FromUInt<std::byte>(block & ~maskIndex);
		}

		static size_t GetBlockIndex(Block block) noexcept
		{
			return static_cast<size_t>(block & maskIndex);
		}

		static Block GetBlock(Chunk chunk, size_t blockIndex) noexcept
		{
			return PtrCaster::ToUInt(chunk) | uintptr_t{blockIndex};
		}

		static State GetState(Chunk chunk) noexcept
		{
			return MemCopyer::FromBuffer<State>(pvGetStatePosition(chunk));
		}

		static Chunk GetPrevChunk(Chunk chunk) noexcept
		{
			return MemCopyer::FromBuffer<Chunk>(pvGetPrevChunkPosition(chunk));
		}

		static void SetPrevChunk(Chunk chunk, Chunk prevChunk) noexcept
		{
			return MemCopyer::ToBuffer(prevChunk, pvGetPrevChunkPosition(chunk));
		}

		static Chunk GetNextChunk(Chunk chunk) noexcept
		{
			return MemCopyer::FromBuffer<Chunk>(pvGetNextChunkPosition(chunk));
		}

		static void SetNextChunk(Chunk chunk, Chunk nextChunk) noexcept
		{
			return MemCopyer::ToBuffer(nextChunk, pvGetNextChunkPosition(chunk));
		}

	private:
		static consteval size_t pvGetChunkHeadSize() noexcept
		{
			return UIntMath<>::Ceil(sizeof(State) + 2 * sizeof(Chunk),
				UIntConst::maxAllocAlignment);
		}

		static consteval size_t pvGetChunkSize() noexcept
		{
			return pvGetChunkHeadSize() + (sizeof(Item) << logItemCount);
		}

		static Chunk pvNewChunk(std::byte* begin) noexcept
		{
			Chunk chunk = begin + pvGetChunkHeadSize();
			pvSetState(chunk, State{0});
			SetPrevChunk(chunk, nullptr);
			SetNextChunk(chunk, nullptr);
			return chunk;
		}

		static void pvSetState(Chunk chunk, State state) noexcept
		{
			return MemCopyer::ToBuffer(state, pvGetStatePosition(chunk));
		}

		static std::byte* pvGetStatePosition(Chunk chunk) noexcept
		{
			return chunk - sizeof(State);
		}

		static std::byte* pvGetPrevChunkPosition(Chunk chunk) noexcept
		{
			return pvGetStatePosition(chunk) - 2 * sizeof(Chunk);
		}

		static std::byte* pvGetNextChunkPosition(Chunk chunk) noexcept
		{
			return pvGetStatePosition(chunk) - sizeof(Chunk);
		}
	};

	template<typename TQItem, typename TSettings>
	class SemiListIterator : public BidirectionalIteratorBase
	{
	protected:
		typedef TQItem QItem;
		typedef TSettings Settings;

	private:
		typedef typename Settings::template Chunker<std::remove_const_t<QItem>> Chunker;
		typedef typename Chunker::Chunk Chunk;
		typedef typename Chunker::Block Block;
		typedef typename Chunker::State State;	//?

	public:
		typedef QItem& Reference;
		typedef QItem* Pointer;

		typedef SemiListIterator<const QItem, Settings> ConstIterator;

	public:
		explicit SemiListIterator() noexcept
			: mBlock(Chunker::nullBlock)
		{
		}

		operator ConstIterator() const noexcept
		{
			return ProxyConstructor<ConstIterator>(mBlock);
		}

		SemiListIterator& operator++()
		{
			MOMO_CHECK(mBlock != Chunker::nullBlock);
			Chunk chunk = Chunker::GetChunk(mBlock);
			size_t blockIndex = Chunker::GetBlockIndex(mBlock);
			State state = Chunker::GetState(chunk);
			State state2 = (state >> (blockIndex + 1)) << (blockIndex + 1);
			if (state2 != State{0})
			{
				size_t nextBlockIndex = static_cast<size_t>(std::countr_zero(state2));
				mBlock = Chunker::GetBlock(chunk, nextBlockIndex);
			}
			else
			{
				Chunk nextChunk = Chunker::GetNextChunk(chunk);
				MOMO_CHECK(nextChunk != Chunker::nullChunk);
				State nextState = Chunker::GetState(nextChunk);
				if (nextState != State{0})
				{
					size_t nextBlockIndex = static_cast<size_t>(std::countr_zero(nextState));
					mBlock = Chunker::GetBlock(nextChunk, nextBlockIndex);
				}
				else
				{
					MOMO_ASSERT(Chunker::GetNextChunk(nextChunk) == Chunker::nullChunk);
					mBlock = Chunker::GetBlock(nextChunk, 0);
				}
			}
			return *this;
		}

		SemiListIterator& operator--()
		{
			MOMO_CHECK(mBlock != Chunker::nullBlock);
			Chunk chunk = Chunker::GetChunk(mBlock);
			size_t blockIndex = Chunker::GetBlockIndex(mBlock);
			State state = Chunker::GetState(chunk);
			State state2 = state & uintptr_t((size_t{1} << blockIndex) - 1);
			if (state2 != State{0})
			{
				size_t prevBlockIndex = static_cast<size_t>(std::bit_width(state2) - 1);
				mBlock = Chunker::GetBlock(chunk, prevBlockIndex);
			}
			else
			{
				Chunk prevChunk = Chunker::GetPrevChunk(chunk);
				MOMO_CHECK(prevChunk != Chunker::nullChunk);
				State prevState = Chunker::GetState(prevChunk);
				MOMO_ASSERT(prevState != State{0});
				size_t prevBlockIndex = static_cast<size_t>(std::bit_width(prevState) - 1);
				mBlock = Chunker::GetBlock(prevChunk, prevBlockIndex);
			}
			return *this;
		}

		using BidirectionalIteratorBase::operator++;
		using BidirectionalIteratorBase::operator--;

		Pointer operator->() const
		{
			MOMO_CHECK(!ptIsEmpty());
			return Chunker::GetItemPtr(mBlock);
		}

		friend bool operator==(SemiListIterator iter1, SemiListIterator iter2) noexcept
		{
			return iter1.mBlock == iter2.mBlock;
		}

	protected:
		explicit SemiListIterator(Block block) noexcept
			: mBlock(block)
		{
		}

		Block ptGetBlock() const noexcept
		{
			return mBlock;
		}

		bool ptIsEmpty() const noexcept
		{
			return mBlock == Chunker::nullBlock
				|| Chunker::GetState(Chunker::GetChunk(mBlock)) == State{0};
		}

	private:
		Block mBlock;
	};
}

template<conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault>
class SemiListItemTraits
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;

	//template<typename... ItemArgs>
	//using Creator = typename ItemManager::template Creator<ItemArgs...>;
	template<typename... ItemArgs>
	using Creator = internal::ObjectCreator<Item, MemManager, std::index_sequence_for<ItemArgs...>, ItemArgs...>;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	static void Destroy(MemManager& memManager, Item& item) noexcept
	{
		ItemManager::Destroy(&memManager, item);
	}
};

class SemiListSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	//static const bool allowExceptionSuppression = true;

	template<typename Item>
	using Chunker = internal::ListChunker<Item>;
};

template<typename TItemTraits,
	typename TSettings = SemiListSettings>
class SemiListCore
	: public internal::Swappable<SemiListCore>
{
public:
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Item Item;
	typedef typename ItemTraits::MemManager MemManager;

	typedef internal::SemiListIterator<Item, Settings> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

private:
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef typename Settings::template Chunker<Item> Chunker;
	typedef typename Chunker::Chunk Chunk;
	typedef typename Chunker::Block Block;
	typedef typename Chunker::State State;	//?

	static const size_t chunkItemCount = size_t{1} << Chunker::logItemCount;

	struct ConstIteratorProxy : private ConstIterator
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBlock)
	};

	struct IteratorProxy : private Iterator
	{
		MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetBlock)
	};

public:
	SemiListCore()
		: SemiListCore(MemManager())
	{
	}

	explicit SemiListCore(MemManager memManager)
		: mMemManager(std::move(memManager)),
		mCount(0)
	{
		mTailChunk = Chunker::NewChunk(mMemManager);	//?
		mHeadChunk = mTailChunk;
	}

	explicit SemiListCore(size_t count, MemManager memManager = MemManager())
		: SemiListCore(std::move(memManager))
	{
		SetCount(count);
	}

	explicit SemiListCore(size_t count, const Item& item, MemManager memManager = MemManager())
		: SemiListCore(std::move(memManager))
	{
		SetCount(count, item);
	}

	template<std::input_iterator ArgIterator, internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit SemiListCore(ArgIterator begin, ArgSentinel end, MemManager memManager = MemManager())
		: SemiListCore(std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<
			std::iter_reference_t<ArgIterator>> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
			pvAddBack(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
	}

	SemiListCore(std::initializer_list<Item> items)
		: SemiListCore(items, MemManager())
	{
	}

	explicit SemiListCore(std::initializer_list<Item> items, MemManager memManager)
		: SemiListCore(items.begin(), items.end(), std::move(memManager))
	{
	}

	SemiListCore(SemiListCore&& list) noexcept
		: mMemManager(std::move(list.mMemManager)),
		mHeadChunk(std::exchange(list.mHeadChunk, Chunker::nullChunk)),
		mTailChunk(std::exchange(list.mTailChunk, Chunker::nullChunk)),
		mCount(std::exchange(list.mCount, 0))
	{
	}

	SemiListCore(const SemiListCore& list)
		: SemiListCore(list, MemManager(list.GetMemManager()))
	{
	}

	explicit SemiListCore(const SemiListCore& list, MemManager memManager)
		: SemiListCore(list.GetBegin(), list.GetEnd(), std::move(memManager))
	{
	}

	~SemiListCore() noexcept
	{
		if (mTailChunk != Chunker::nullChunk)
		{
			pvDecCount(0);
			Chunker::DeleteChunk(mMemManager, mTailChunk);
		}
	}

	SemiListCore& operator=(SemiListCore&& list) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(list), *this);
	}

	SemiListCore& operator=(const SemiListCore& list)
	{
		return internal::ContainerAssigner::Copy(list, *this);
	}

	void Swap(SemiListCore& list) noexcept
	{
		if (this != &list)	//?
			MemManagerProxy::Swap(mMemManager, list.mMemManager);
		std::swap(mHeadChunk, list.mHeadChunk);
		std::swap(mTailChunk, list.mTailChunk);
		std::swap(mCount, list.mCount);
	}

	ConstIterator GetBegin() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(
			Chunker::GetBlock(mHeadChunk, pvGetBeginBlockIndex()));
	}

	Iterator GetBegin() noexcept
	{
		return internal::ProxyConstructor<Iterator>(
			Chunker::GetBlock(mHeadChunk, pvGetBeginBlockIndex()));
	}

	ConstIterator GetEnd() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(Chunker::GetBlock(mTailChunk, 0));
	}

	Iterator GetEnd() noexcept
	{
		return internal::ProxyConstructor<Iterator>(Chunker::GetBlock(mTailChunk, 0));
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mMemManager;
	}

	MemManager& GetMemManager() noexcept
	{
		return mMemManager;
	}

	size_t GetCount() const noexcept
	{
		return mCount;
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void SetCountCrt(size_t count, ItemMultiCreator itemMultiCreator)
	{
		pvSetCount(count, FastCopyableFunctor(itemMultiCreator));
	}

	void SetCount(size_t count)
	{
		typedef typename ItemTraits::template Creator<> ItemCreator;
		auto itemMultiCreator = [this] (Item* newItem)
			{ (ItemCreator(mMemManager))(newItem); };
		SetCountCrt(count, itemMultiCreator);
	}

	void SetCount(size_t count, const Item& item)
	{
		typedef typename ItemTraits::template Creator<const Item&> ItemCreator;
		auto itemMultiCreator = [this, &item] (Item* newItem)
			{ (ItemCreator(mMemManager, item))(newItem); };
		SetCountCrt(count, itemMultiCreator);
	}

	bool IsEmpty() const noexcept
	{
		return mCount == 0;
	}

	void Clear() noexcept
	{
		pvDecCount(0);
	}

	const Item& GetFrontItem() const
	{
		return *GetBegin();
	}

	Item& GetFrontItem()
	{
		return *GetBegin();
	}

	const Item& GetBackItem() const
	{
		return *std::prev(GetEnd());
	}

	Item& GetBackItem()
	{
		return *std::prev(GetEnd());
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddFrontCrt(ItemCreator itemCreator)
	{
		pvAddFront(FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	void AddFrontVar(ItemArgs&&... itemArgs)
	{
		AddFrontCrt(typename ItemTraits::template Creator<ItemArgs...>(
			mMemManager, std::forward<ItemArgs>(itemArgs)...));
	}

	void AddFront(Item&& item)
	{
		return AddFrontVar(std::move(item));
	}

	void AddFront(const Item& item)
	{
		return AddFrontVar(item);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackCrt(ItemCreator itemCreator)
	{
		pvAddBack(FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	void AddBackVar(ItemArgs&&... itemArgs)
	{
		AddBackCrt(typename ItemTraits::template Creator<ItemArgs...>(
			mMemManager, std::forward<ItemArgs>(itemArgs)...));
	}

	void AddBack(Item&& item)
	{
		return AddBackVar(std::move(item));
	}

	void AddBack(const Item& item)
	{
		return AddBackVar(item);
	}

	void RemoveFront()
	{
		Remove(GetBegin());
	}

	void RemoveBack()
	{
		Remove(std::prev(GetEnd()));
	}

	Iterator Remove(ConstIterator iter)
	{
		MOMO_CHECK(iter != ConstIterator() && iter != GetEnd());
		Block block = ConstIteratorProxy::GetBlock(iter);
		Iterator resIter = internal::ProxyConstructor<Iterator>(block);
		++resIter;
		Chunk chunk = Chunker::GetChunk(block);
		ItemTraits::Destroy(mMemManager, *Chunker::GetItemPtr(block));
		Chunker::DeleteBlock(block);
		if (Chunker::GetState(chunk) == State{0})
		{
			Chunk prevChunk = Chunker::GetPrevChunk(chunk);
			Chunk nextChunk = Chunker::GetNextChunk(chunk);
			if (prevChunk != Chunker::nullChunk)
				Chunker::SetNextChunk(prevChunk, nextChunk);
			Chunker::SetPrevChunk(nextChunk, prevChunk);
			if (chunk == mHeadChunk)
				mHeadChunk = nextChunk;
			Chunker::DeleteChunk(mMemManager, chunk);
		}
		--mCount;
		return resIter;
	}

	Iterator Remove(ConstIterator begin, ConstIterator end)
	{
		Iterator iter = internal::ProxyConstructor<Iterator>(ConstIteratorProxy::GetBlock(begin));
		while (iter != end)
			iter = Remove(iter);
		return iter;
	}

	template<internal::conceptObjectPredicate<Item> ItemFilter>
	size_t Remove(ItemFilter itemFilter)
	{
		size_t initCount = mCount;
		ConstIterator iter = GetBegin();
		ConstIterator end = GetEnd();
		while (iter != end)
		{
			if (itemFilter(*iter))
				iter = Remove(iter);
			else
				++iter;
		}
		return initCount - mCount;
	}

	template<internal::conceptEqualComparer<Item> ItemEqualComparer = std::equal_to<Item>>
	bool IsEqual(const SemiListCore& list,
		ItemEqualComparer itemEqualComp = ItemEqualComparer()) const
	{
		return std::equal(GetBegin(), GetEnd(), list.GetBegin(), list.GetEnd(),
			FastCopyableFunctor(itemEqualComp));
	}

	auto Compare(const SemiListCore& list) const
		requires requires { typename internal::TieThreeComparer<Item>; }
	{
		return Compare(list, internal::TieThreeComparer<Item>());
	}

	template<typename ItemThreeComparer>
	auto Compare(const SemiListCore& list, ItemThreeComparer itemThreeComp) const
	{
		return std::lexicographical_compare_three_way(GetBegin(), GetEnd(),
			list.GetBegin(), list.GetEnd(), FastCopyableFunctor(itemThreeComp));
	}

	//template<typename ItemArg = Item,
	//	internal::conceptEqualComparer<Item, ItemArg> ItemEqualComparer = std::equal_to<>>
	//bool Contains(const ItemArg& itemArg, ItemEqualComparer itemEqualComp = ItemEqualComparer()) const

private:
	size_t pvGetBeginBlockIndex() const noexcept
	{
		size_t blockIndex = 0;
		if (mHeadChunk != mTailChunk)
			blockIndex = static_cast<size_t>(std::countr_zero(Chunker::GetState(mHeadChunk)));
		return blockIndex;
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void pvSetCount(size_t count, FastCopyableFunctor<ItemMultiCreator> itemMultiCreator)
	{
		if (count <= mCount)
		{
			pvDecCount(count);
		}
		else
		{
			size_t initCount = mCount;
			for (internal::Finalizer fin(&SemiListCore::pvDecCount, *this, initCount); fin; fin.Detach())
			{
				while (mCount < count)
					pvAddBack(FastMovableFunctor(itemMultiCreator));
			}
		}
	}

	void pvDecCount(size_t count) noexcept
	{
		while (mCount > count)
			RemoveBack();
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddFront(FastMovableFunctor<ItemCreator> itemCreator)
	{
		size_t blockIndex = pvGetBeginBlockIndex();
		if (blockIndex == 0)
		{
			Chunk newChunk = pvAddGrow(chunkItemCount - 1, std::move(itemCreator));
			Chunker::SetPrevChunk(mHeadChunk, newChunk);
			Chunker::SetNextChunk(newChunk, mHeadChunk);
			mHeadChunk = newChunk;
		}
		else
		{
			pvAddNogrow(mHeadChunk, blockIndex - 1, std::move(itemCreator));
		}
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBack(FastMovableFunctor<ItemCreator> itemCreator)
	{
		if (mHeadChunk != mTailChunk)
		{
			Block block = IteratorProxy::GetBlock(std::prev(GetEnd()));
			Chunk chunk = Chunker::GetChunk(block);
			size_t blockIndex = Chunker::GetBlockIndex(block);
			if (blockIndex + 1 < chunkItemCount)
			{
				pvAddNogrow(chunk, blockIndex + 1, std::move(itemCreator));
			}
			else
			{
				Chunk newChunk = pvAddGrow(0, std::move(itemCreator));
				Chunker::SetNextChunk(chunk, newChunk);
				Chunker::SetPrevChunk(newChunk, chunk);
				Chunker::SetNextChunk(newChunk, mTailChunk);
				Chunker::SetPrevChunk(mTailChunk, newChunk);
			}
		}
		else
		{
			Chunk newChunk = pvAddGrow(0, std::move(itemCreator));
			Chunker::SetNextChunk(newChunk, mTailChunk);
			Chunker::SetPrevChunk(mTailChunk, newChunk);
			mHeadChunk = newChunk;
		}
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddNogrow(Chunk chunk, size_t blockIndex, FastMovableFunctor<ItemCreator> itemCreator)
	{
		Block block = Chunker::NewBlock(chunk, blockIndex);
		for (internal::Finalizer fin(&Chunker::DeleteBlock, block); fin; fin.Detach())
			std::move(itemCreator)(Chunker::GetItemPtr(block));
		++mCount;
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	Chunk pvAddGrow(size_t blockIndex, FastMovableFunctor<ItemCreator> itemCreator)
	{
		Chunk chunk = Chunker::NewChunk(mMemManager);
		internal::Finalizer fin(&Chunker::template DeleteChunk<MemManager>, mMemManager, chunk);
		pvAddNogrow(chunk, blockIndex, std::move(itemCreator));
		fin.Detach();
		return chunk;
	}

private:
	MOMO_NO_UNIQUE_ADDRESS MemManager mMemManager;
	Chunk mHeadChunk;
	Chunk mTailChunk;
	size_t mCount;
};

template<conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault>
using SemiList = SemiListCore<SemiListItemTraits<TItem, TMemManager>>;

} // namespace momo

namespace std
{
	template<typename I, typename S>
	struct iterator_traits<momo::internal::SemiListIterator<I, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::SemiListIterator<I, S>,
			bidirectional_iterator_tag>
	{
	};
} // namespace std
