/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashSet.h

  namespace momo:
    class HashListSetItemTraits
    class HashListSetSettings
    class HashListSetCore
    class HashListSet

\**********************************************************/

#pragma once

#include "HashSet.h"
#include "SemiList.h"

namespace momo
{

namespace internal
{
	template<typename TIterator>
	class HashListSetPosition : public PositionBase
	{
	public:
		typedef TIterator Iterator;

		typedef typename Iterator::Reference Reference;
		typedef typename Iterator::Pointer Pointer;

		typedef HashListSetPosition ConstPosition;

	private:
		struct IteratorProxy : private Iterator
		{
			MOMO_DECLARE_PROXY_FUNCTION(Iterator, IsEmpty)
		};

	public:
		explicit HashListSetPosition() noexcept
			: mHashCode(0)
		{
		}

		//operator ConstPosition() const noexcept

		operator Iterator() const noexcept
		{
			return mIterator;
		}

		Pointer operator->() const
		{
			return mIterator.operator->();
		}

		explicit operator bool() const noexcept
		{
			return !IteratorProxy::IsEmpty(mIterator);
		}

		friend bool operator==(HashListSetPosition pos1, HashListSetPosition pos2) noexcept
		{
			return pos1.mIterator == pos2.mIterator;
		}

	protected:
		explicit HashListSetPosition(Iterator iter, size_t hashCode) noexcept
			: mIterator(iter),
			mHashCode(hashCode)
		{
		}

		size_t ptGetHashCode() const noexcept
		{
			return mHashCode;
		}

	private:
		Iterator mIterator;
		size_t mHashCode;
	};

	template<typename THashSetBucketIterator>
	class HashListSetBucketIterator : public ForwardIteratorBase
	{
	protected:
		typedef THashSetBucketIterator HashSetBucketIterator;

	private:
		typedef std::decay_t<decltype(*HashSetBucketIterator())> HashListSetIterator;

	public:
		typedef typename HashListSetIterator::Reference Reference;
		typedef typename HashListSetIterator::Pointer Pointer;

		typedef HashListSetBucketIterator ConstIterator;

	public:
		explicit HashListSetBucketIterator() noexcept
			: mHashSetBucketIterator()
		{
		}

		//operator ConstIterator() const noexcept

		HashListSetBucketIterator& operator++()
		{
			++mHashSetBucketIterator;
			return *this;
		}

		using ForwardIteratorBase::operator++;

		Pointer operator->() const
		{
			return mHashSetBucketIterator->operator->();
		}

		friend bool operator==(HashListSetBucketIterator iter1, HashListSetBucketIterator iter2) noexcept
		{
			return iter1.mHashSetBucketIterator == iter2.mHashSetBucketIterator;
		}

	protected:
		explicit HashListSetBucketIterator(HashSetBucketIterator hashSetIter) noexcept
			: mHashSetBucketIterator(hashSetIter)
		{
		}

	private:
		HashSetBucketIterator mHashSetBucketIterator;
	};

	template<typename THashSetBucketBounds>
	class HashListSetBucketBounds
	{
	protected:
		typedef THashSetBucketBounds HashSetBucketBounds;

	public:
		typedef HashListSetBucketIterator<typename HashSetBucketBounds::Iterator> Iterator;

		typedef HashListSetBucketBounds ConstBounds;

	public:
		explicit HashListSetBucketBounds() noexcept
		{
		}

		//operator ConstBounds() const noexcept

		Iterator GetBegin() const noexcept
		{
			return ProxyConstructor<Iterator>(mHashSetBucketBounds.GetBegin());
		}

		Iterator GetEnd() const noexcept
		{
			return ProxyConstructor<Iterator>(mHashSetBucketBounds.GetEnd());
		}

		size_t GetCount() const noexcept
		{
			return mHashSetBucketBounds.GetCount();
		}

	protected:
		explicit HashListSetBucketBounds(HashSetBucketBounds hashSetBounds) noexcept
			: mHashSetBucketBounds(hashSetBounds)
		{
		}

	private:
		HashSetBucketBounds mHashSetBucketBounds;
	};

	template<typename KeyArg>
	struct HashListSetCodeKeyArg
	{
		size_t hashCode;
		const KeyArg& key;
	};

	template<typename Key>
	struct HashListSetCodeKeyPtr
	{
		size_t hashCode;
		const Key* keyPtr;
	};

	template<typename TKey, typename TBaseHashTraits>
	class HashListSetNestedHashTraits
	{
	protected:
		typedef TBaseHashTraits BaseHashTraits;

	public:
		typedef TKey Key;

		static const bool isFastNothrowHashable = BaseHashTraits::isFastNothrowHashable;

		template<typename ItemTraits>
		using Bucket = typename BaseHashTraits::template Bucket<ItemTraits>;	//?

		template<typename KeyArg>
		class IsValidKeyArg : public std::false_type
		{
		};

		template<typename KeyArg>
		class IsValidKeyArg<HashListSetCodeKeyArg<KeyArg>>
			: public BaseHashTraits::template IsValidKeyArg<KeyArg>
		{
		};

		template<std::same_as<Key> KeyArg>	// gcc
		class IsValidKeyArg<HashListSetCodeKeyArg<KeyArg>> : public std::true_type
		{
		};

		template<std::same_as<Key> KeyArg>
		class IsValidKeyArg<HashListSetCodeKeyPtr<KeyArg>> : public std::true_type
		{
		};

	public:
		explicit HashListSetNestedHashTraits(const BaseHashTraits& baseHashTraits)
			: mBaseHashTraits(baseHashTraits)
		{
		}

		size_t CalcCapacity(size_t bucketCount, size_t bucketMaxItemCount) const noexcept
		{
			return mBaseHashTraits.CalcCapacity(bucketCount, bucketMaxItemCount);
		}

		size_t GetBucketCountShift(size_t bucketCount, size_t bucketMaxItemCount) const noexcept
		{
			return mBaseHashTraits.GetBucketCountShift(bucketCount, bucketMaxItemCount);
		}

		size_t GetLogStartBucketCount() const noexcept
		{
			return mBaseHashTraits.GetLogStartBucketCount();
		}

		size_t GetHashCode(const Key& key) const
		{
			return mBaseHashTraits.GetHashCode(key);
		}

		template<typename KeyArg>
		size_t GetHashCode(HashListSetCodeKeyArg<KeyArg> key) const noexcept
		{
			return key.hashCode;
		}

		size_t GetHashCode(HashListSetCodeKeyPtr<Key> key) const noexcept
		{
			return key.hashCode;
		}

		bool IsEqual(const Key& key1, const Key& key2) const
		{
			return mBaseHashTraits.IsEqual(key1, key2);
		}

		template<typename KeyArg>
		bool IsEqual(HashListSetCodeKeyArg<KeyArg> key1, const Key& key2) const
		{
			return mBaseHashTraits.IsEqual(key1.key, key2);
		}

		bool IsEqual(HashListSetCodeKeyPtr<Key> key1, const Key& key2) const noexcept
		{
			return key1.keyPtr == std::addressof(key2);
		}

		const BaseHashTraits& GetBaseHashTraits() const noexcept
		{
			return mBaseHashTraits;
		}

	private:
		MOMO_NO_UNIQUE_ADDRESS BaseHashTraits mBaseHashTraits;
	};

	template<typename THashListSetItemTraits, typename THashSetItem>
	class HashListSetNestedHashSetItemTraits
		: public HashSetItemTraits<THashSetItem, typename THashListSetItemTraits::MemManager>
	{
	protected:
		typedef THashListSetItemTraits HashListSetItemTraits;
		typedef THashSetItem HashSetItem;

	public:
		typedef typename HashListSetItemTraits::Key Key;

	public:
		static const Key& GetKey(HashSetItem hashSetItem) noexcept
		{
			return HashListSetItemTraits::GetKey(*hashSetItem);
		}
	};

	template<typename THashListSetItemTraits>
	class HashListSetNestedSemiListItemTraits
	{
	protected:
		typedef THashListSetItemTraits HashListSetItemTraits;

	public:
		typedef typename HashListSetItemTraits::Item Item;
		typedef MemManagerPtr<typename HashListSetItemTraits::MemManager> MemManager;

	public:
		static void Destroy(MemManager& memManager, Item& item) noexcept
		{
			HashListSetItemTraits::Destroy(&memManager.GetBaseMemManager(), item);
		}
	};

	template<typename THashListSetSettings>
	class HashListSetNestedSemiListSettings : public SemiListSettings
	{
	protected:
		typedef THashListSetSettings HashListSetSettings;

	public:
		static const CheckMode checkMode = HashListSetSettings::checkMode;
		//static const bool allowExceptionSuppression = HashListSetSettings::allowExceptionSuppression;
	};
}

template<conceptObject TKey,
	conceptMemManager TMemManager = MemManagerDefault>
class HashListSetItemTraits
{
public:
	typedef TKey Key;
	typedef TMemManager MemManager;
	typedef Key Item;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	static const size_t alignment = ItemManager::alignment;	//?

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

public:
	static const Key& GetKey(const Item& item) noexcept
	{
		return item;
	}

	template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
	static void Destroy(MemManagerOrNullPtr memManager, Item& item) noexcept
	{
		ItemManager::Destroy(memManager, item);
	}
};

class HashListSetSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool allowExceptionSuppression = true;
};

template<typename TItemTraits,
	conceptHashTraits<typename TItemTraits::Key> THashTraits = HashTraits<typename TItemTraits::Key>,
	typename TSettings = HashListSetSettings>
requires conceptSetItemTraits<TItemTraits, typename TItemTraits::Key, typename TItemTraits::MemManager>
class MOMO_EMPTY_BASES HashListSetCore
	: public internal::Rangeable,
	public internal::Swappable<HashListSetCore>
{
public:
	typedef TItemTraits ItemTraits;
	typedef THashTraits HashTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Key Key;
	typedef typename ItemTraits::Item Item;
	typedef typename ItemTraits::MemManager MemManager;

private:
	typedef SemiListCore<internal::HashListSetNestedSemiListItemTraits<ItemTraits>,
		internal::HashListSetNestedSemiListSettings<Settings>> List;
	typedef typename List::MemManager MemManagerPtr;	//?

	typedef internal::HashListSetNestedHashTraits<Key, HashTraits> NestedHashTraits;
	typedef HashSetCore<internal::HashListSetNestedHashSetItemTraits<ItemTraits, typename List::ConstIterator>,
		NestedHashTraits, internal::NestedHashSetSettings<Settings::allowExceptionSuppression>> HashSet;

public:
	typedef typename List::ConstIterator Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::HashListSetPosition<Iterator> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef internal::SetExtractedItem<ItemTraits, Settings> ExtractedItem;	//?

	typedef internal::HashListSetBucketBounds<typename HashSet::ConstBucketBounds> ConstBucketBounds;

	static const size_t bucketMaxItemCount = HashSet::bucketMaxItemCount;

private:
	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	template<typename KeyArg>
	using IsValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>;

	struct ConstPositionProxy : private ConstPosition
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetHashCode)
	};

public:
	HashListSetCore()
		: HashListSetCore(HashTraits())
	{
	}

	explicit HashListSetCore(const HashTraits& hashTraits, MemManager memManager = MemManager())
		: mHashSet(NestedHashTraits(hashTraits), std::move(memManager)),
		mList(MemManagerPtr(GetMemManager()))
	{
	}

	template<internal::conceptSetArgIterator<Item> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit HashListSetCore(ArgIterator begin, ArgSentinel end,
		const HashTraits& hashTraits = HashTraits(), MemManager memManager = MemManager())
		: HashListSetCore(hashTraits, std::move(memManager))
	{
		Insert(std::move(begin), std::move(end));
	}

	HashListSetCore(std::initializer_list<Item> items)
		: HashListSetCore(items, HashTraits())
	{
	}

	explicit HashListSetCore(std::initializer_list<Item> items, const HashTraits& hashTraits,
		MemManager memManager = MemManager())
		: HashListSetCore(items.begin(), items.end(), hashTraits, std::move(memManager))
	{
	}

	HashListSetCore(HashListSetCore&& hashListSet) noexcept
		: mHashSet(std::move(hashListSet.mHashSet)),
		mList(std::move(hashListSet.mList))
	{
	}

	HashListSetCore(const HashListSetCore& hashListSet)
		: HashListSetCore(hashListSet, MemManager(hashListSet.GetMemManager()))
	{
	}

	explicit HashListSetCore(const HashListSetCore& hashListSet, MemManager memManager)
		: HashListSetCore(hashListSet.GetBegin(), hashListSet.GetEnd(),
			hashListSet.GetHashTraits(), std::move(memManager))
	{
	}

	~HashListSetCore() noexcept = default;

	HashListSetCore& operator=(HashListSetCore&& hashListSet) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(hashListSet), *this);
	}

	HashListSetCore& operator=(const HashListSetCore& hashListSet)
	{
		return internal::ContainerAssigner::Copy(hashListSet, *this);
	}

	void Swap(HashListSetCore& hashListSet) noexcept
	{
		mHashSet.Swap(hashListSet.mHashSet);
		mList.Swap(hashListSet.mList);
	}

	Iterator GetBegin() const noexcept
	{
		return mList.GetBegin();
	}

	Iterator GetEnd() const noexcept
	{
		return mList.GetEnd();
	}

	const HashTraits& GetHashTraits() const noexcept
	{
		return mHashSet.GetHashTraits().GetBaseHashTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mHashSet.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mHashSet.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mHashSet.GetCount();
	}

	bool IsEmpty() const noexcept
	{
		return mHashSet.IsEmpty();
	}

	void Clear(bool shrink = true) noexcept
	{
		mHashSet.Clear(shrink);
		mList.Clear();
	}

	size_t GetCapacity() const noexcept
	{
		return mHashSet.GetCapacity();
	}

	void Reserve(size_t capacity)
	{
		mHashSet.Reserve(capacity);
	}

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
			{
				Creator<Item&&>(GetMemManager(), std::move(item))(newItem);
				ItemTraits::Destroy(nullptr, item);
			};
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
	Iterator AddCrt(ConstPosition pos, ItemCreator itemCreator)
	{
		return pvAdd<extraCheck>(pos, FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	requires requires { typename Creator<ItemArgs...>; }
	Iterator AddVar(ConstPosition pos, ItemArgs&&... itemArgs)
	{
		return AddCrt(pos,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	Iterator Add(ConstPosition pos, Item&& item)
	{
		return AddVar(pos, std::move(item));
	}

	Iterator Add(ConstPosition pos, const Item& item)
	{
		return AddVar(pos, item);
	}

	//Iterator Add(ConstPosition pos, ExtractedItem&& extItem)

	Iterator Remove(ConstIterator iter)
	{
		size_t hashCode = GetHashTraits().GetHashCode(ItemTraits::GetKey(*iter));
		ConstPosition pos = internal::ProxyConstructor<ConstPosition>(iter, hashCode);
		return Remove(pos);
	}

	Iterator Remove(ConstPosition pos)
	{
		size_t hashCode = ConstPositionProxy::GetHashCode(pos);
		ConstIterator iter = pos;
		const Key* keyPtr = std::addressof(ItemTraits::GetKey(*iter));
		mHashSet.Remove(mHashSet.Find(internal::HashListSetCodeKeyPtr(hashCode, keyPtr)));
		return mList.Remove(iter);
	}

	//Iterator Remove(ConstIterator iter, ExtractedItem& extItem)

	Iterator Remove(ConstIterator begin, ConstIterator end)
	{
		Iterator iter = begin;
		while (iter != end)
			iter = Remove(iter);
		return iter;
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
		ConstIterator iter = GetBegin();
		ConstIterator end = GetEnd();
		while (iter != end)
		{
			if (itemFilter(*iter))	//?
				iter = Remove(iter);
			else
				++iter;
		}
		return initCount - GetCount();
	}

	//ExtractedItem Extract(ConstPosition pos)

	//template<typename KeyArg,
	//	bool extraCheck = true>
	//void ResetKey(ConstPosition pos, KeyArg&& keyArg)

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

	void MergeTo(HashListSetCore& dstHashListSet)
	{
		if (this == &dstHashListSet)
			return;
		pvMergeTo(dstHashListSet);
	}

	size_t GetBucketCount() const noexcept
	{
		return mHashSet.GetBucketCount();
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		return internal::ProxyConstructor<ConstBucketBounds>(mHashSet.GetBucketBounds(bucketIndex));
	}

	size_t GetBucketIndex(const Key& key) const
	{
		return mHashSet.GetBucketIndex(key);
	}

	//Position MakePosition(size_t hashCode) const noexcept

	//void CheckIterator(ConstIterator iter, bool allowEmpty = true) const

private:
	template<bool extraCheck, internal::conceptObjectCreator<Item> ItemCreator>
	InsertResult pvInsert(const Key& key, FastMovableFunctor<ItemCreator> itemCreator)
	{
		Position pos = pvFind(key);
		Iterator iter = pos;
		if (!!pos)
			return { iter, false };
		iter = pvAdd<extraCheck>(pos, std::move(itemCreator));
		return { iter, true };
	}

	template<bool extraCheck, internal::conceptObjectCreator<Item> ItemCreator>
	Iterator pvAdd(ConstPosition pos, FastMovableFunctor<ItemCreator> itemCreator)
	{
		auto hashSetItemCreator = [this, itemCreator = std::move(itemCreator)] (auto* hashSetItem) mutable
		{
			mList.AddBackCrt(std::move(itemCreator));
			std::construct_at(hashSetItem, std::prev(mList.GetEnd()));
		};
		return *mHashSet.AddCrt(mHashSet.MakePosition(ConstPositionProxy::GetHashCode(pos)),
			std::move(hashSetItemCreator));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE ConstPosition pvFind(const KeyArg& key) const
	{
		size_t hashCode = GetHashTraits().GetHashCode(key);
		auto hashSetPosition = mHashSet.Find(internal::HashListSetCodeKeyArg(hashCode, key));
		return internal::ProxyConstructor<ConstPosition>(
			!!hashSetPosition ? *hashSetPosition : GetEnd(), hashCode);
	}

	template<typename Set>
	void pvMergeTo(Set& dstSet)
	{
		ConstIterator iter = GetBegin();
		ConstIterator end = GetEnd();
		while (iter != end)
		{
			if (dstSet.Insert(*iter).inserted)
				iter = Remove(iter);
			else
				++iter;
		}
	}

private:
	HashSet mHashSet;
	List mList;
};

template<conceptObject TKey,
	conceptHashTraits<TKey> THashTraits = HashTraitsOpen<TKey>,
	conceptMemManager TMemManager = MemManagerDefault>
using HashListSet = HashListSetCore<HashListSetItemTraits<TKey, TMemManager>, THashTraits>;

} // namespace momo

namespace std
{
	template<typename BI>
	struct iterator_traits<momo::internal::HashListSetBucketIterator<BI>>
		: public momo::internal::IteratorTraitsStd<momo::internal::HashListSetBucketIterator<BI>,
			forward_iterator_tag>
	{
	};
} // namespace std
