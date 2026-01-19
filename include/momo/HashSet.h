/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashSet.h

  namespace momo:
    class HashSetItemTraits
    class HashSetSettings
    class HashSetCore
    class HashSet
    class HashSetOpen

\**********************************************************/

#pragma once

#include "HashTraits.h"
#include "SetUtility.h"
#include "IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TBucket>
	requires (std::is_trivially_destructible_v<TBucket>)
	class HashSetBuckets : public Rangeable
	{
	public:
		typedef TBucket Bucket;
		typedef typename Bucket::MemManager MemManager;
		typedef typename Bucket::Params BucketParams;

		static const size_t maxBucketCount = UIntConst::maxSize / sizeof(Bucket);

	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	public:
		HashSetBuckets() = delete;

		HashSetBuckets(const HashSetBuckets&) = delete;

		HashSetBuckets& operator=(const HashSetBuckets&) = delete;

		static HashSetBuckets* Create(MemManager& memManager, size_t logBucketCount,
			BucketParams* bucketParams)
		{
			size_t bucketCount = size_t{1} << logBucketCount;
			if (bucketCount > maxBucketCount)
				MOMO_THROW(std::bad_array_new_length());
			size_t bufferSize = pvGetBufferSize(logBucketCount);
			void* buffer = MemManagerProxy::Allocate(memManager, bufferSize);
			HashSetBuckets* resBuckets = ::new(buffer) HashSetBuckets(logBucketCount);
			for (Finalizer fin(&MemManagerProxy::template Deallocate<void>, memManager, buffer, bufferSize);
				fin; fin.Detach())
			{
				std::uninitialized_default_construct_n(
					resBuckets->pvGetBuckets<false>(), bucketCount);
				resBuckets->mBucketParams = (bucketParams != nullptr) ? bucketParams
					: MemManagerProxy::template AllocateCreate<BucketParams>(memManager, memManager);
			}
			return resBuckets;
		}

		void Destroy(MemManager& memManager, bool destroyBucketParams) noexcept
		{
			MOMO_ASSERT(mNextBuckets == nullptr);
			if (destroyBucketParams)
			{
				mBucketParams->Clear();
				std::destroy_at(mBucketParams);
				MemManagerProxy::Deallocate(memManager, mBucketParams, sizeof(BucketParams));
			}
			MemManagerProxy::Deallocate(memManager, this, pvGetBufferSize(GetLogCount()));
		}

		Bucket* GetBegin() noexcept
		{
			return pvGetBuckets();
		}

		Bucket* GetEnd() noexcept
		{
			return pvGetBuckets() + GetCount();
		}

		HashSetBuckets* GetNextBuckets() noexcept
		{
			return mNextBuckets;
		}

		HashSetBuckets* ExtractNextBuckets() noexcept
		{
			HashSetBuckets* nextBuckets = mNextBuckets;
			mNextBuckets = nullptr;
			return nextBuckets;
		}

		void SetNextBuckets(HashSetBuckets* nextBuckets) noexcept
		{
			MOMO_ASSERT(mNextBuckets == nullptr);
			mNextBuckets = nextBuckets;
		}

		size_t GetCount() const noexcept
		{
			return size_t{1} << mLogCount;
		}

		size_t GetLogCount() const noexcept
		{
			return mLogCount;
		}

		Bucket& operator[](size_t index) noexcept
		{
			MOMO_ASSERT(index < GetCount());
			return pvGetBuckets()[index];
		}

		BucketParams& GetBucketParams() noexcept
		{
			return *mBucketParams;
		}

	private:
		explicit HashSetBuckets(size_t logBucketCount) noexcept
			: mLogCount(logBucketCount),
			mNextBuckets(nullptr),
			mBucketParams(nullptr)
		{
		}

		~HashSetBuckets() noexcept = default;

		template<bool isWithinLifetime = true>
		Bucket* pvGetBuckets() noexcept
		{
			return PtrCaster::FromBytePtr<Bucket, isWithinLifetime>(
				PtrCaster::ToBytePtr(this) + pvGetBucketOffset());
		}

		static size_t pvGetBufferSize(size_t logBucketCount) noexcept
		{
			return pvGetBucketOffset() + (sizeof(Bucket) << logBucketCount);
		}

		static consteval size_t pvGetBucketOffset() noexcept
		{
			return sizeof(HashSetBuckets);
		}

	private:
		size_t mLogCount;
		HashSetBuckets* mNextBuckets;
		union
		{
			BucketParams* mBucketParams;
			ObjectBuffer<std::byte[alignof(Bucket)], alignof(Bucket)> mAlignedBuffer;
		};
		//Bucket[]
	};

	template<typename TBucket, typename TSettings>
	class HashSetIterator;

	template<typename TBucket, typename TSettings>
	class MOMO_EMPTY_BASES HashSetPosition
		: private VersionKeeper<TSettings>,
		public ForwardIteratorBase
	{
	protected:
		typedef TBucket Bucket;
		typedef TSettings Settings;

		typedef typename Bucket::Iterator BucketIterator;

	private:
		typedef typename Bucket::Item Item;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef HashSetPosition ConstPosition;

		typedef HashSetIterator<Bucket, Settings> Iterator;

	public:
		explicit HashSetPosition() noexcept
			: mBucketIterator(),
			mIndexCode(0)
		{
		}

		//operator ConstPosition() const noexcept

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(mBucketIterator != BucketIterator());
			return std::to_address(mBucketIterator);
		}

		friend bool operator==(HashSetPosition pos1, HashSetPosition pos2) noexcept
		{
			return pos1.mBucketIterator == pos2.mBucketIterator;
		}

	protected:
		explicit HashSetPosition(size_t indexCode, BucketIterator bucketIter,
			const size_t* version) noexcept
			: VersionKeeper(version),
			mBucketIterator(bucketIter),
			mIndexCode(indexCode)
		{
		}

		size_t ptGetBucketIndex() const noexcept
		{
			//MOMO_ASSERT(mBucketIterator != BucketIterator());	//?
			return mIndexCode;
		}

		size_t ptGetHashCode() const noexcept
		{
			MOMO_ASSERT(mBucketIterator == BucketIterator());
			return mIndexCode;
		}

		BucketIterator ptGetBucketIterator() const noexcept
		{
			return mBucketIterator;
		}

		void ptReset(size_t bucketIndex, BucketIterator bucketIter) noexcept
		{
			mIndexCode = bucketIndex;
			mBucketIterator = bucketIter;
		}

		void ptCheck(const size_t* version, bool allowEmpty) const
		{
			VersionKeeper::Check(version, allowEmpty);
		}

	private:
		BucketIterator mBucketIterator;
		size_t mIndexCode;
	};

	template<typename TBucket, typename TSettings>
	class HashSetIterator : public HashSetPosition<TBucket, TSettings>
	{
	public:
		typedef HashSetPosition<TBucket, TSettings> Position;

	protected:
		using typename Position::Bucket;
		using typename Position::Settings;

		using typename Position::BucketIterator;

		typedef HashSetBuckets<Bucket> Buckets;

	public:
		using typename Position::Reference;
		using typename Position::Pointer;

		typedef HashSetIterator ConstIterator;

	private:
		//using typename Position::ConstPosition;
		using typename Position::Iterator;	//?

		typedef typename Bucket::Bounds BucketBounds;

	public:
		explicit HashSetIterator() noexcept
			: mBuckets(nullptr)
		{
		}

		HashSetIterator(Position pos) noexcept
			: Position(pos),
			mBuckets(nullptr)
		{
		}

		//operator ConstIterator() const noexcept

		HashSetIterator& operator++()
		{
			Position::operator->();	// check
			if (ptIsMovable())
				pvInc();
			else
				*this = HashSetIterator();
			return *this;
		}

		using ForwardIteratorBase::operator++;

	protected:
		explicit HashSetIterator(Buckets& buckets, size_t bucketIndex,
			BucketIterator bucketIter, const size_t* version) noexcept
			: Position(bucketIndex, bucketIter, version),
			mBuckets(&buckets)
		{
			pvInc();
		}

		bool ptIsMovable() const noexcept
		{
			MOMO_ASSERT(Position::ptGetBucketIterator() != BucketIterator());
			return mBuckets != nullptr;
		}

	private:
		void pvInc() noexcept
		{
			BucketIterator bucketIter = Position::ptGetBucketIterator();
			size_t bucketIndex = Position::ptGetBucketIndex();
			if (bucketIter != pvGetBucketBounds(bucketIndex).GetBegin())
				Position::ptReset(bucketIndex, std::prev(bucketIter));
			else
				pvMove();
		}

		void pvMove() noexcept
		{
			size_t bucketCount = mBuckets->GetCount();
			size_t bucketIndex = Position::ptGetBucketIndex();
			while (true)
			{
				++bucketIndex;
				if (bucketIndex >= bucketCount)
					break;
				BucketBounds bounds = pvGetBucketBounds(bucketIndex);
				if (bounds.GetCount() > 0)
				{
					Position::ptReset(bucketIndex, std::prev(bounds.GetEnd()));
					return;
				}
			}
			Buckets* nextBuckets = mBuckets->GetNextBuckets();
			if (nextBuckets != nullptr)
			{
				mBuckets = nextBuckets;
				Position::ptReset(0, pvGetBucketBounds(0).GetEnd());
				return pvInc();	//?
			}
			*this = HashSetIterator();
		}

		BucketBounds pvGetBucketBounds(size_t bucketIndex) const noexcept
		{
			return (*mBuckets)[bucketIndex].GetBounds(mBuckets->GetBucketParams());
		}

	private:
		Buckets* mBuckets;
	};

	template<typename THashSetItemTraits>
	class HashSetBucketItemTraits
	{
	protected:
		typedef THashSetItemTraits HashSetItemTraits;

	public:
		typedef typename HashSetItemTraits::Item Item;
		typedef typename HashSetItemTraits::MemManager MemManager;

		static const size_t alignment = HashSetItemTraits::alignment;

	public:
		template<conceptObjectCreator<Item> ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
		{
			HashSetItemTraits::RelocateCreate(memManager, srcItems, dstItems, count,
				std::move(itemCreator), newItem);
		}
	};
}

template<conceptObject TKey,
	conceptMemManager TMemManager = MemManagerDefault>
class HashSetItemTraits : public internal::SetItemTraits<TKey, TMemManager>
{
private:
	typedef internal::SetItemTraits<TKey, TMemManager> SetItemTraits;

public:
	using typename SetItemTraits::Item;
	using typename SetItemTraits::MemManager;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	template<internal::conceptObjectCreator<Item> ItemCreator>
	static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
		size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcItems, dstItems, count,
			std::move(itemCreator), newItem);
	}
};

class HashSetSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
	static const bool allowExceptionSuppression = true;
};

/*!
	All `HashSetCore` functions and constructors have strong exception safety,
	but not the following cases:
	1. Functions `Insert` receiving many items have basic exception safety.
	2. Function `Remove` receiving predicate has basic exception safety.
	3. Functions `MergeFrom` and `MergeTo` have basic exception safety.
*/

template<typename TItemTraits,
	conceptHashTraits<typename TItemTraits::Key> THashTraits = HashTraits<typename TItemTraits::Key>,
	typename TSettings = HashSetSettings>
requires conceptSetItemTraits<TItemTraits, typename TItemTraits::Key, typename TItemTraits::MemManager>
class MOMO_EMPTY_BASES HashSetCore
	: public internal::Rangeable,
	public internal::Swappable<HashSetCore>
{
public:
	typedef TItemTraits ItemTraits;
	typedef THashTraits HashTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Key Key;
	typedef typename ItemTraits::Item Item;
	typedef typename ItemTraits::MemManager MemManager;

private:
	typedef internal::SetCrew<HashTraits, MemManager, Settings::checkVersion> Crew;

	typedef internal::HashSetBucketItemTraits<ItemTraits> BucketItemTraits;

	typedef typename HashTraits::template Bucket<BucketItemTraits> Bucket;

	typedef typename Bucket::Params BucketParams;

	typedef typename Bucket::Iterator BucketIterator;
	typedef typename Bucket::Bounds BucketBounds;

	typedef internal::HashSetBuckets<Bucket> Buckets;

	static const bool allowExceptionSuppression = internal::Catcher::allowExceptionSuppression<Settings>;

public:
	typedef internal::HashSetIterator<Bucket, Settings> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef typename Iterator::Position Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

	typedef internal::SetExtractedItem<ItemTraits, Settings> ExtractedItem;

	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

	static const size_t bucketMaxItemCount = Bucket::maxCount;

private:
	static const bool areItemsNothrowRelocatable = HashTraits::isFastNothrowHashable
		&& ItemTraits::isNothrowRelocatable && Bucket::isNothrowAddableIfNothrowCreatable;

	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	template<typename KeyArg>
	using IsValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>;

	struct ConstIteratorProxy : private ConstIterator
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, IsMovable)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct ConstPositionProxy : private ConstPosition
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetBucketIndex)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetHashCode)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetBucketIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, Reset)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, Check)
	};

	struct PositionProxy : public Position
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Position)
	};

	template<typename KeyArg>
	class ItemFindPredicate
	{
	public:
		explicit ItemFindPredicate(const KeyArg& key, const HashTraits& hashTraits) noexcept
			: mKey(key),
			mHashTraits(hashTraits)
		{
		}

		bool operator()(const Item& item) const
		{
			return mHashTraits.IsEqual(mKey, ItemTraits::GetKey(item));
		}

	private:
		const KeyArg& mKey;
		const HashTraits& mHashTraits;
	};

	template<typename KeyArg>
	requires requires (const KeyArg& key1, const Key& key2)
		{ { HashTraits::IsEqual(key1, key2) } -> std::convertible_to<bool>; }
	class ItemFindPredicate<KeyArg>
	{
	public:
		explicit ItemFindPredicate(const KeyArg& key, const HashTraits& /*hashTraits*/) noexcept
			: mKey(key)
		{
		}

		bool operator()(const Item& item) const
		{
			return HashTraits::IsEqual(mKey, ItemTraits::GetKey(item));
		}

	private:
		const KeyArg& mKey;
	};

public:
	HashSetCore()
		: HashSetCore(HashTraits())
	{
	}

	explicit HashSetCore(const HashTraits& hashTraits, MemManager memManager = MemManager())
		: mCrew(hashTraits, std::move(memManager)),
		mCount(0),
		mCapacity(0),
		mBuckets(nullptr)
	{
	}

	template<internal::conceptSetArgIterator<Item> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit HashSetCore(ArgIterator begin, ArgSentinel end,
		const HashTraits& hashTraits = HashTraits(), MemManager memManager = MemManager())
		: HashSetCore(hashTraits, std::move(memManager))
	{
		Insert(std::move(begin), std::move(end));
	}

	HashSetCore(std::initializer_list<Item> items)
		: HashSetCore(items, HashTraits())
	{
	}

	explicit HashSetCore(std::initializer_list<Item> items, const HashTraits& hashTraits,
		MemManager memManager = MemManager())
		: HashSetCore(items.begin(), items.end(), hashTraits, std::move(memManager))
	{
	}

	HashSetCore(HashSetCore&& hashSet) noexcept
		: mCrew(std::move(hashSet.mCrew)),
		mCount(std::exchange(hashSet.mCount, 0)),
		mCapacity(std::exchange(hashSet.mCapacity, 0)),
		mBuckets(std::exchange(hashSet.mBuckets, nullptr))
	{
	}

	HashSetCore(const HashSetCore& hashSet)
		: HashSetCore(hashSet, MemManager(hashSet.GetMemManager()))
	{
	}

	explicit HashSetCore(const HashSetCore& hashSet, MemManager memManager)
		: HashSetCore(hashSet.GetHashTraits(), std::move(memManager))
	{
		if (hashSet.IsEmpty())
			return;
		const HashTraits& hashTraits = GetHashTraits();
		size_t logBucketCount = hashTraits.GetLogStartBucketCount();
		size_t capacity;
		while (true)
		{
			capacity = hashTraits.CalcCapacity(size_t{1} << logBucketCount, bucketMaxItemCount);
			if (capacity >= hashSet.mCount)
				break;
			++logBucketCount;
		}
		mBuckets = Buckets::Create(GetMemManager(), logBucketCount, nullptr);
		mCapacity = capacity;
		for (const Item& item : hashSet)
		{
			size_t hashCode = hashTraits.GetHashCode(ItemTraits::GetKey(item));
			pvAddInternal(*mBuckets, hashCode,
				FastMovableFunctor(Creator<const Item&>(GetMemManager(), item)));
		}
		mCount = hashSet.mCount;
	}

	~HashSetCore() noexcept
	{
		pvDestroy();
	}

	HashSetCore& operator=(HashSetCore&& hashSet) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(hashSet), *this);
	}

	HashSetCore& operator=(const HashSetCore& hashSet)
	{
		return internal::ContainerAssigner::Copy(hashSet, *this);
	}

	void Swap(HashSetCore& hashSet) noexcept
	{
		mCrew.Swap(hashSet.mCrew);
		std::swap(mCount, hashSet.mCount);
		std::swap(mCapacity, hashSet.mCapacity);
		std::swap(mBuckets, hashSet.mBuckets);
	}

	Iterator GetBegin() const noexcept
	{
		if (mCount == 0)
			return Iterator();
		return IteratorProxy(*mBuckets, size_t{0},
			mBuckets->GetBegin()->GetBounds(mBuckets->GetBucketParams()).GetEnd(),
			mCrew.GetVersion());
	}

	Iterator GetEnd() const noexcept
	{
		return Iterator();
	}

	const HashTraits& GetHashTraits() const noexcept
	{
		return mCrew.GetContainerTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mCrew.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mCrew.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mCount;
	}

	bool IsEmpty() const noexcept
	{
		return mCount == 0;
	}

	void Clear(bool shrink = true) noexcept
	{
		if (mBuckets == nullptr)
			return;
		if (shrink)
		{
			pvDestroy();
			mBuckets = nullptr;
			mCapacity = 0;
		}
		else
		{
			pvClear(*mBuckets);
			pvDestroy(mBuckets->ExtractNextBuckets(), false);
			mBuckets->GetBucketParams().Clear();
		}
		mCount = 0;
		mCrew.IncVersion();
	}

	size_t GetCapacity() const noexcept
	{
		return mCapacity;
	}

	void Reserve(size_t capacity)
	{
		if (capacity <= mCapacity)
			return;
		const HashTraits& hashTraits = GetHashTraits();
		size_t newLogBucketCount = pvGetNewLogBucketCount();
		size_t newCapacity;
		while (true)
		{
			newCapacity = hashTraits.CalcCapacity(size_t{1} << newLogBucketCount,
				bucketMaxItemCount);
			if (newCapacity >= capacity)
				break;
			++newLogBucketCount;
		}
		Buckets* newBuckets = Buckets::Create(GetMemManager(), newLogBucketCount,
			(mBuckets != nullptr) ? &mBuckets->GetBucketParams() : nullptr);
		newBuckets->SetNextBuckets(mBuckets);
		mBuckets = newBuckets;
		mCapacity = newCapacity;
		pvRelocateItems();
		mCrew.IncVersion();
	}

	//void Shrink()
	//{
	//	HashSetCore(*this).Swap(*this);
	//}

	MOMO_FORCEINLINE ConstPosition Find(const Key& key) const
	{
		return pvFind(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE ConstPosition Find(const KeyArg& key) const
	{
		return pvFind(key);
	}

	MOMO_FORCEINLINE bool ContainsKey(const Key& key) const
	{
		return !!pvFind(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE bool ContainsKey(const KeyArg& key) const
	{
		return !!pvFind(key);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator,
		bool extraCheck = true>
	InsertResult InsertCrt(const Key& key, ItemCreator itemCreator)
	{
		return pvInsert<extraCheck>(key, FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	requires requires { typename Creator<ItemArgs...>; }
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		const Key& key = ItemTraits::GetKey(std::as_const(item));
		return InsertCrt<Creator<Item&&>, false>(key,
			Creator<Item&&>(GetMemManager(), std::move(item)));
	}

	InsertResult Insert(const Item& item)
	{
		return InsertCrt<Creator<const Item&>, false>(ItemTraits::GetKey(item),
			Creator<const Item&>(GetMemManager(), item));
	}

	InsertResult Insert(ExtractedItem&& extItem)
	{
		auto itemCreator = [this, &extItem] (Item* newItem)
		{
			auto itemRemover = [this, newItem] (Item& item)
				{ ItemTraits::Relocate(nullptr, &GetMemManager(), item, newItem); };
			extItem.Remove(itemRemover);
		};
		return pvInsert<false>(ItemTraits::GetKey(extItem.GetItem()),
			FastMovableFunctor(std::move(itemCreator)));
	}

	template<internal::conceptSetArgIterator<Item> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		size_t initCount = GetCount();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
		{
			auto&& ref = *iter;
			InsertVar(ItemTraits::GetKey(std::as_const(ref)), std::forward<decltype(ref)>(ref));
		}
		return GetCount() - initCount;
	}

	size_t Insert(std::initializer_list<Item> items)
	{
		return Insert(items.begin(), items.end());
	}

	template<internal::conceptObjectCreator<Item> ItemCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, ItemCreator itemCreator)
	{
		return pvAdd<extraCheck>(pos, FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	requires requires { typename Creator<ItemArgs...>; }
	Position AddVar(ConstPosition pos, ItemArgs&&... itemArgs)
	{
		return AddCrt(pos,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	Position Add(ConstPosition pos, Item&& item)
	{
		return AddVar(pos, std::move(item));
	}

	Position Add(ConstPosition pos, const Item& item)
	{
		return AddVar(pos, item);
	}

	Position Add(ConstPosition pos, ExtractedItem&& extItem)
	{
		auto itemCreator = [this, &extItem] (Item* newItem)
		{
			auto itemRemover = [this, newItem] (Item& item)
				{ ItemTraits::Relocate(nullptr, &GetMemManager(), item, newItem); };
			extItem.Remove(itemRemover);
		};
		return pvAdd<true>(pos, FastMovableFunctor(std::move(itemCreator)));
	}

	Iterator Remove(ConstIterator iter)
	{
		auto itemReplacer = [this] (Item& srcItem, Item& dstItem)
			{ ItemTraits::Replace(GetMemManager(), srcItem, dstItem); };
		return pvRemove(iter, FastMovableFunctor(std::move(itemReplacer)));
	}

	void Remove(ConstPosition pos)
	{
		Remove(static_cast<ConstIterator>(pos));
	}

	Iterator Remove(ConstIterator iter, ExtractedItem& extItem)
	{
		Iterator resIter;
		auto itemCreator = [this, iter, &resIter] (Item* newItem)
		{
			auto itemReplacer = [this, newItem] (Item& srcItem, Item& dstItem)
			{
				MemManager& memManager = GetMemManager();
				if (std::addressof(srcItem) == std::addressof(dstItem))
					ItemTraits::Relocate(&memManager, nullptr, dstItem, newItem);
				else
					ItemTraits::ReplaceRelocate(memManager, srcItem, dstItem, newItem);
			};
			resIter = pvRemove(iter, FastMovableFunctor(std::move(itemReplacer)));
		};
		extItem.Create(itemCreator);
		return resIter;
	}

	bool Remove(const Key& key)
	{
		Position pos = pvFind(key);
		if (!pos)
			return false;
		Remove(pos);
		return true;
	}

	template<internal::conceptObjectPredicate<Item> ItemFilter>
	size_t Remove(ItemFilter itemFilter)
	{
		size_t initCount = GetCount();
		Iterator iter = GetBegin();
		while (!!iter)
		{
			if (itemFilter(*iter))	//?
				iter = Remove(iter);
			else
				++iter;
		}
		return initCount - GetCount();
	}

	ExtractedItem Extract(ConstPosition pos)
	{
		return ExtractedItem(*this, static_cast<ConstIterator>(pos));
	}

	template<typename KeyArg,
		bool extraCheck = true>
	void ResetKey(ConstPosition pos, KeyArg&& keyArg)
	{
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), false);
		BucketIterator bucketIter = ConstPositionProxy::GetBucketIterator(pos);
		MOMO_CHECK(bucketIter != BucketIterator());
		ItemTraits::AssignKey(GetMemManager(), std::forward<KeyArg>(keyArg), *bucketIter);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(pos));
	}

	template<typename RSet>
	void MergeFrom(RSet&& srcSet)
	{
		srcSet.MergeTo(*this);
	}

	template<typename Set>
	requires std::is_same_v<ItemTraits, typename Set::ItemTraits>
	void MergeTo(Set& dstSet)
	{
		pvMergeTo(dstSet);
	}

	void MergeTo(HashSetCore& dstHashSet)
	{
		if (this == &dstHashSet)
			return;
		pvMergeTo(dstHashSet);
	}

	size_t GetBucketCount() const noexcept
	{
		size_t bucketCount = 0;
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
			bucketCount += bkts->GetCount();
		return bucketCount;
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		MOMO_CHECK(bucketIndex < GetBucketCount());
		size_t curBucketIndex = bucketIndex;
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			BucketParams& bucketParams = bkts->GetBucketParams();
			size_t curBucketCount = bkts->GetCount();
			if (curBucketIndex < curBucketCount)
				return (*bkts)[curBucketIndex].GetBounds(bucketParams);
			curBucketIndex -= curBucketCount;
		}
		MOMO_ASSERT(false);
		return ConstBucketBounds();
	}

	size_t GetBucketIndex(const Key& key) const
	{
		MOMO_CHECK(mBuckets != nullptr);
		Position pos = pvFind(key);
		if (!!pos)
		{
			size_t bucketIndex = ConstPositionProxy::GetBucketIndex(pos);
			BucketIterator bucketIter = ConstPositionProxy::GetBucketIterator(pos);
			Buckets* buckets = pvFindBuckets(bucketIndex, bucketIter);
			for (Buckets* bkts = mBuckets; bkts != buckets; bkts = bkts->GetNextBuckets())
				bucketIndex += bkts->GetCount();
			return bucketIndex;
		}
		else
		{
			size_t hashCode = ConstPositionProxy::GetHashCode(pos);
			return Bucket::GetStartBucketIndex(hashCode, mBuckets->GetCount());	//?
		}
	}

	ConstPosition MakePosition(size_t hashCode) const noexcept
	{
		return pvMakePosition(hashCode, BucketIterator());
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		ConstPosition pos = iter;
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), allowEmpty);
	}

private:
	void pvDestroy() noexcept
	{
		pvDestroy(mBuckets, true);
	}

	void pvDestroy(Buckets* buckets, bool destroyBucketParams) noexcept
	{
		if (buckets == nullptr)
			return;
		pvClear(*buckets);
		pvDestroy(buckets->ExtractNextBuckets(), false);
		buckets->Destroy(GetMemManager(), destroyBucketParams);
	}

	void pvClear(Buckets& buckets) noexcept
	{
		MemManager& memManager = GetMemManager();
		BucketParams& bucketParams = buckets.GetBucketParams();
		for (Bucket& bucket : buckets)
		{
			for (Item& item : bucket.GetBounds(bucketParams))
				ItemTraits::Destroy(&memManager, item);
			bucket.Clear(bucketParams);
		}
	}

	size_t pvGetNewLogBucketCount() const
	{
		const HashTraits& hashTraits = GetHashTraits();
		if (mBuckets == nullptr)
			return hashTraits.GetLogStartBucketCount();
		size_t logBucketCount = mBuckets->GetLogCount();
		size_t shift = hashTraits.GetBucketCountShift(size_t{1} << logBucketCount,
			bucketMaxItemCount);
		MOMO_CHECK(shift > 0);
		return logBucketCount + shift;
	}

	Position pvMakePosition(size_t indexCode, BucketIterator bucketIter) const noexcept
	{
		return PositionProxy(indexCode, bucketIter, mCrew.GetVersion());
	}

	bool pvExtraCheck(ConstPosition pos) const noexcept
	{
		bool res = true;
		if constexpr (allowExceptionSuppression)
		{
			res = false;
			internal::Catcher::CatchAll([this, &res, pos] ()
				{ res = (pos == pvFind(ItemTraits::GetKey(*pos))); });
		}
		return res;
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE Position pvFind(const KeyArg& key) const
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t indexCode = hashTraits.GetHashCode(key);
		BucketIterator bucketIter = BucketIterator();
		if (mCount != 0) [[likely]]
		{
			Buckets* buckets = mBuckets;
			while (true)
			{
				bucketIter = pvFind(indexCode, *buckets,
					FastCopyableFunctor(ItemFindPredicate<KeyArg>(key, hashTraits)));
				if (bucketIter != BucketIterator() || areItemsNothrowRelocatable)
					break;
				buckets = buckets->GetNextBuckets();
				if (buckets == nullptr) [[likely]]
					break;
			}
		}
		return pvMakePosition(indexCode, bucketIter);
	}

	template<internal::conceptObjectPredicate<Item> ItemPredicate>
	MOMO_FORCEINLINE static BucketIterator pvFind(size_t& indexCode, Buckets& buckets,
		FastCopyableFunctor<ItemPredicate> itemPred)
	{
		size_t hashCode = indexCode;
		BucketParams& bucketParams = buckets.GetBucketParams();
		size_t bucketCount = buckets.GetCount();
		size_t bucketIndex = Bucket::GetStartBucketIndex(hashCode, bucketCount);
		Bucket* bucket = &buckets[bucketIndex];
		BucketIterator bucketIter = bucket->template Find<true>(bucketParams, itemPred, hashCode);
		if (bucketIter != BucketIterator())
		{
			indexCode = bucketIndex;
			return bucketIter;
		}
		size_t maxProbe = bucket->GetMaxProbe(buckets.GetLogCount());
		for (size_t probe = 1; bucket->WasFull() && probe <= maxProbe; ++probe)
		{
			bucketIndex = Bucket::GetNextBucketIndex(bucketIndex, hashCode, bucketCount, probe);
			bucket = &buckets[bucketIndex];
			bucketIter = bucket->template Find<false>(bucketParams, itemPred, hashCode);
			if (bucketIter != BucketIterator())
			{
				indexCode = bucketIndex;
				return bucketIter;
			}
		}
		return BucketIterator();
	}

	template<bool extraCheck, internal::conceptObjectCreator<Item> ItemCreator>
	InsertResult pvInsert(const Key& key, FastMovableFunctor<ItemCreator> itemCreator)
	{
		Position pos = pvFind(key);
		if (!!pos)
			return { pos, false };
		pos = pvAdd<extraCheck>(pos, std::move(itemCreator));
		return { pos, true };
	}

	template<bool extraCheck, internal::conceptObjectCreator<Item> ItemCreator>
	Position pvAdd(ConstPosition pos, FastMovableFunctor<ItemCreator> itemCreator)
	{
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), false);
		MOMO_CHECK(ConstPositionProxy::GetBucketIterator(pos) == BucketIterator());
		size_t hashCode = ConstPositionProxy::GetHashCode(pos);
		Position resPos;
		if (mCount < mCapacity)
			resPos = pvAddNogrow(*mBuckets, hashCode, std::move(itemCreator));
		else
			resPos = pvAddGrow(hashCode, std::move(itemCreator));
		if constexpr (allowExceptionSuppression)
		{
			if (mBuckets->GetNextBuckets() != nullptr)
				pvRelocateItems(resPos);
		}
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(resPos));
		return resPos;
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	Position pvAddNogrow(Buckets& buckets, size_t hashCode, FastMovableFunctor<ItemCreator> itemCreator)
	{
		auto [bucketIndex, bucketIter] = pvAddInternal(buckets, hashCode, std::move(itemCreator));
		++mCount;
		mCrew.IncVersion();
		return pvMakePosition(bucketIndex, bucketIter);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	MOMO_NOINLINE Position pvAddGrow(size_t hashCode, FastMovableFunctor<ItemCreator> itemCreator)
	{
		const HashTraits& hashTraits = GetHashTraits();
		MemManager& memManager = GetMemManager();
		size_t newLogBucketCount = pvGetNewLogBucketCount();
		size_t newCapacity = hashTraits.CalcCapacity(size_t{1} << newLogBucketCount, bucketMaxItemCount);
		MOMO_CHECK(newCapacity > mCount);
		Buckets* newBuckets = nullptr;
		auto newBucketsCreator = [this, &newBuckets, newLogBucketCount] ()
		{
			newBuckets = Buckets::Create(GetMemManager(), newLogBucketCount,
				(mBuckets != nullptr) ? &mBuckets->GetBucketParams() : nullptr);
		};
		if (mBuckets == nullptr || !allowExceptionSuppression)
			newBucketsCreator();
		else if constexpr (allowExceptionSuppression)
			internal::Catcher::CatchAll(newBucketsCreator);
		if (newBuckets == nullptr)
			return pvAddNogrow(*mBuckets, hashCode, std::move(itemCreator));
		internal::Finalizer newBucketsFin(&Buckets::Destroy, *newBuckets,
			memManager, mBuckets == nullptr);
		Position resPos;
		internal::ObjectBuffer<Item, ItemTraits::alignment> itemBuffer;
		if constexpr (allowExceptionSuppression)
			resPos = pvAddNogrow(*newBuckets, hashCode, std::move(itemCreator));
		else
			std::move(itemCreator)(itemBuffer.GetPtr());
		newBucketsFin.Detach();
		newBuckets->SetNextBuckets(mBuckets);
		mBuckets = newBuckets;
		mCapacity = newCapacity;
		if constexpr (!allowExceptionSuppression)
		{
			internal::Finalizer fin(&ItemTraits::template Destroy<MemManager*>,
				&memManager, itemBuffer.Get());
			pvRelocateItems();
			auto itemRelocateCreator = [&memManager, &itemBuffer] (Item* newItem)
				{ ItemTraits::Relocate(&memManager, &memManager, itemBuffer.Get(), newItem); };
			resPos = pvAddNogrow(*mBuckets, hashCode, FastMovableFunctor(std::move(itemRelocateCreator)));
			fin.Detach();
		}
		return resPos;
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	std::pair<size_t, BucketIterator> pvAddInternal(Buckets& buckets, size_t hashCode,
		FastMovableFunctor<ItemCreator> itemCreator)
	{
		size_t bucketCount = buckets.GetCount();
		size_t bucketIndex = Bucket::GetStartBucketIndex(hashCode, bucketCount);
		Bucket& startBucket = buckets[bucketIndex];
		Bucket* bucket = &startBucket;
		size_t probe = 0;
		while (bucket->IsFull())
		{
			++probe;
			if (probe >= bucketCount)
				MOMO_THROW(std::runtime_error("Hash table is full"));
			bucketIndex = Bucket::GetNextBucketIndex(bucketIndex, hashCode, bucketCount, probe);
			bucket = &buckets[bucketIndex];
		}
		BucketIterator bucketIter = bucket->AddCrt(buckets.GetBucketParams(),
			std::move(itemCreator), hashCode, buckets.GetLogCount(), probe);
		startBucket.UpdateMaxProbe(probe);
		return { bucketIndex, bucketIter };
	}

	template<internal::conceptObjectReplacer<Item> ItemReplacer>
	Iterator pvRemove(ConstIterator iter, FastMovableFunctor<ItemReplacer> itemReplacer)
	{
		MOMO_CHECK(mBuckets != nullptr);
		Position pos = iter;
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), false);
		BucketIterator bucketIter = ConstPositionProxy::GetBucketIterator(pos);
		MOMO_CHECK(bucketIter != BucketIterator());
		size_t bucketIndex = ConstPositionProxy::GetBucketIndex(pos);
		Buckets* buckets = pvFindBuckets(bucketIndex, bucketIter);
		Bucket& bucket = (*buckets)[bucketIndex];
		bucketIter = bucket.Remove(buckets->GetBucketParams(), bucketIter, std::move(itemReplacer));
		--mCount;
		mCrew.IncVersion();
		if (!ConstIteratorProxy::IsMovable(iter))
			return Iterator();
		return IteratorProxy(*buckets, bucketIndex, bucketIter, mCrew.GetVersion());
	}

	Buckets* pvFindBuckets(size_t bucketIndex, BucketIterator bucketIter) const
	{
		MOMO_ASSERT(mBuckets != nullptr);
		if (mBuckets->GetNextBuckets() == nullptr)
			return mBuckets;
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			if (bucketIndex >= bkts->GetCount())
				continue;
			Bucket& bucket = (*bkts)[bucketIndex];
			BucketBounds bucketBounds = bucket.GetBounds(bkts->GetBucketParams());
			std::less<BucketIterator> less;
			if (!less(bucketIter, bucketBounds.GetBegin()) && less(bucketIter, bucketBounds.GetEnd()))
				return bkts;
		}
		MOMO_ASSERT(false);
		return nullptr;
	}

	MOMO_NOINLINE void pvRelocateItems(Position& pos) noexcept
		requires allowExceptionSuppression
	{
		BucketParams& bucketParams = mBuckets->GetBucketParams();
		size_t bucketIndex = ConstPositionProxy::GetBucketIndex(pos);
		Bucket& bucket = (*mBuckets)[bucketIndex];
		ptrdiff_t itemIndex = std::distance(bucket.GetBounds(bucketParams).GetBegin(),
			ConstPositionProxy::GetBucketIterator(pos));
		pvRelocateItems();
		ConstPositionProxy::Reset(pos, bucketIndex,
			std::next(bucket.GetBounds(bucketParams).GetBegin(), itemIndex));
	}

	void pvRelocateItems() noexcept(areItemsNothrowRelocatable || allowExceptionSuppression)
	{
		MemManager& memManager = GetMemManager();
		while (true)
		{
			Buckets* buckets = mBuckets->GetNextBuckets();
			if (buckets == nullptr)
				break;
			bool done = true;
			if constexpr (areItemsNothrowRelocatable || !allowExceptionSuppression)
				pvRelocateItems(*buckets);
			else if constexpr (allowExceptionSuppression)
				done = internal::Catcher::CatchAll([this, buckets] () { pvRelocateItems(*buckets); });	//?
			if (!done)
				break;
			Buckets* nextBuckets = buckets->ExtractNextBuckets();
			mBuckets->ExtractNextBuckets()->Destroy(memManager, false);
			mBuckets->SetNextBuckets(nextBuckets);
		}
	}

	void pvRelocateItems(Buckets& buckets) noexcept(areItemsNothrowRelocatable)
	{
		const HashTraits& hashTraits = GetHashTraits();
		BucketParams& bucketParams = buckets.GetBucketParams();
		size_t bucketCount = buckets.GetCount();
		for (size_t i = 0; i < bucketCount; ++i)
		{
			Bucket& bucket = buckets[i];
			BucketBounds bucketBounds = bucket.GetBounds(bucketParams);
			BucketIterator bucketIter = bucketBounds.GetEnd();
			auto hashCodeFullGetter = [&hashTraits, &bucketIter] ()
				{ return hashTraits.GetHashCode(ItemTraits::GetKey(*bucketIter)); };
			for (size_t c = bucketBounds.GetCount(); c > 0; --c)
			{
				--bucketIter;
				size_t hashCode = bucket.GetHashCodePart(FastCopyableFunctor(hashCodeFullGetter),
					bucketIter, i, buckets.GetLogCount(), mBuckets->GetLogCount());
				auto itemReplacer = [this, hashCode] ([[maybe_unused]] Item& srcItem, Item& dstItem)
				{
					MOMO_ASSERT(std::addressof(srcItem) == std::addressof(dstItem));
					MemManager& memManager = GetMemManager();
					auto itemCreator = [&memManager, &dstItem] (Item* newItem)
						{ ItemTraits::Relocate(&memManager, &memManager, dstItem, newItem); };
					pvAddInternal(*mBuckets, hashCode, FastMovableFunctor(std::move(itemCreator)));
				};
				bucketIter = bucket.Remove(bucketParams, bucketIter,
					FastMovableFunctor(std::move(itemReplacer)));
			}
		}
	}

	template<typename Set>
	void pvMergeTo(Set& dstSet)
	{
		MemManager& memManager = GetMemManager();
		MemManager& dstMemManager = dstSet.GetMemManager();
		Iterator iter = GetBegin();
		while (!!iter)
		{
			auto itemCreator = [this, &memManager, &dstMemManager, &iter] (Item* newItem)
			{
				auto itemReplacer = [&memManager, &dstMemManager, newItem]
					([[maybe_unused]] Item& srcItem, Item& dstItem)
				{
					MOMO_ASSERT(std::addressof(srcItem) == std::addressof(dstItem));
					ItemTraits::Relocate(&memManager, &dstMemManager, dstItem, newItem);
				};
				iter = pvRemove(iter, FastMovableFunctor(std::move(itemReplacer)));
			};
			if (!dstSet.InsertCrt(ItemTraits::GetKey(*iter), std::move(itemCreator)).inserted)
				++iter;
		}
	}

private:
	MOMO_NO_UNIQUE_ADDRESS Crew mCrew;
	size_t mCount;
	size_t mCapacity;
	Buckets* mBuckets;
};

template<conceptObject TKey,
	conceptHashTraits<TKey> THashTraits = HashTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault>
using HashSet = HashSetCore<HashSetItemTraits<TKey, TMemManager>, THashTraits>;

template<conceptObject TKey>
using HashSetOpen = HashSet<TKey, HashTraitsOpen<TKey>>;

namespace internal
{
	template<bool tAllowExceptionSuppression>
	class NestedHashSetSettings : public HashSetSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = false;
		static const bool allowExceptionSuppression = tAllowExceptionSuppression;
	};
}

} // namespace momo

namespace std
{
	template<typename B, typename S>
	struct iterator_traits<momo::internal::HashSetIterator<B, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::HashSetIterator<B, S>,
			forward_iterator_tag>
	{
	};
} // namespace std
