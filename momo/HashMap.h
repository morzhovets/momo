/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashMap.h

  namespace momo:
    struct HashMapKeyValueTraits
    struct HashMapSettings
    class HashMap

  All `HashMap` functions and constructors have strong exception safety,
  but not the following cases:
  1. Functions `Insert`, `InsertKV`, `InsertFS` receiving many items have
    basic exception safety.
  2. If constructor receiving many items throws exception, input argument
    `memManager` may be changed.
  3. In case default `KeyValueTraits`: if function `Remove` throws exception and
    `ObjectManager<Key>::isNothrowAnywayMoveAssignable` is false and
    `ObjectManager<Value>::isNothrowAnywayMoveAssignable` is false,
    removing value may be changed.

\**********************************************************/

#pragma once

#include "HashSet.h"
#include "MapUtility.h"

namespace momo
{

namespace internal
{
	template<typename TKeyValuePair>
	struct HashMapNestedSetItemTraits
	{
		typedef TKeyValuePair KeyValuePair;

		typedef typename KeyValuePair::Key Key;
		typedef KeyValuePair Item;

		typedef internal::ObjectManager<Item> ItemManager;

		static const size_t alignment = ItemManager::alignment;

		template<typename ItemArg>
		class Creator : public ItemManager::template Creator<ItemArg>
		{
			MOMO_STATIC_ASSERT((std::is_same<ItemArg, Item>::value
				|| std::is_same<ItemArg, const Item&>::value));

		private:
			typedef typename ItemManager::template Creator<ItemArg> BaseCreator;

		public:
			//using BaseCreator::BaseCreator;	// vs2013
			explicit Creator(ItemArg&& itemArg)
				: BaseCreator(std::forward<ItemArg>(itemArg))
			{
			}
		};

		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			return item.GetKey();
		}

		static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
		{
			ItemManager::Destroy(items, count);
		}

		static void Assign(Item&& srcItem, Item& dstItem)
		{
			KeyValuePair::Assign(std::move(srcItem), dstItem);
		}

		template<typename ItemCreator>
		static void RelocateCreate(Item* srcItems, Item* dstItems, size_t count,
			const ItemCreator& itemCreator, void* pitem)
		{
			KeyValuePair::RelocateCreate(srcItems, dstItems, count, itemCreator, pitem);
		}
	};

	template<typename THashMapSettings>
	struct HashMapNestedSetSettings : public HashSetSettings
	{
		typedef THashMapSettings HashMapSettings;

		static const CheckMode checkMode = HashMapSettings::checkMode;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = HashMapSettings::checkVersion;

		static const bool overloadIfCannotGrow = HashMapSettings::overloadIfCannotGrow;
	};
}

template<typename TKey, typename TValue>
using HashMapKeyValueTraits = internal::MapKeyValueTraits<TKey, TValue>;

struct HashMapSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;

	static const bool overloadIfCannotGrow = true;
};

template<typename TKey, typename TValue,
	typename THashTraits = HashTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TKeyValueTraits = HashMapKeyValueTraits<TKey, TValue>,
	typename TSettings = HashMapSettings>
class HashMap
{
public:
	typedef TKey Key;
	typedef TValue Value;
	typedef THashTraits HashTraits;
	typedef TMemManager MemManager;
	typedef TKeyValueTraits KeyValueTraits;
	typedef TSettings Settings;

private:
	typedef internal::MapKeyValuePair<KeyValueTraits> KeyValuePair;

	typedef internal::HashMapNestedSetItemTraits<KeyValuePair> HashSetItemTraits;
	typedef internal::HashMapNestedSetSettings<Settings> HashSetSettings;

	typedef momo::HashSet<Key, HashTraits, MemManager, HashSetItemTraits, HashSetSettings> HashSet;

	typedef typename HashSet::ConstIterator HashSetConstIterator;
	typedef typename HashSetConstIterator::Reference HashSetConstReference;

	typedef internal::MapReference<Key, Value, HashSetConstReference> Reference;

	typedef internal::MapValueReferencer<HashMap> ValueReferencer;

	typedef typename HashSet::ConstBucketBounds HashSetConstBucketBounds;
	typedef internal::HashDerivedIterator<typename HashSetConstBucketBounds::Iterator,
		Reference> BucketIterator;

	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

public:
	typedef internal::HashDerivedIterator<HashSetConstIterator, Reference> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef typename ValueReferencer::ValueReferenceRKey ValueReferenceRKey;
	typedef typename ValueReferencer::ValueReferenceCKey ValueReferenceCKey;

	typedef internal::HashDerivedBucketBounds<BucketIterator,
		HashSetConstBucketBounds> BucketBounds;
	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

public:
	explicit HashMap(const HashTraits& hashTraits = HashTraits(),
		MemManager&& memManager = MemManager())
		: mHashSet(hashTraits, std::move(memManager))
	{
	}

	HashMap(std::initializer_list<std::pair<Key, Value>> keyValuePairs,
		const HashTraits& hashTraits = HashTraits(), MemManager&& memManager = MemManager())
		: HashMap(hashTraits, std::move(memManager))
	{
		Insert(keyValuePairs);
	}

	HashMap(HashMap&& hashMap) MOMO_NOEXCEPT
		: mHashSet(std::move(hashMap.mHashSet))
	{
	}

	HashMap(const HashMap& hashMap)
		: mHashSet(hashMap.mHashSet)
	{
	}

	~HashMap() MOMO_NOEXCEPT
	{
	}

	HashMap& operator=(HashMap&& hashMap) MOMO_NOEXCEPT
	{
		HashMap(std::move(hashMap)).Swap(*this);
		return *this;
	}

	HashMap& operator=(const HashMap& hashMap)
	{
		if (this != &hashMap)
			HashMap(hashMap).Swap(*this);
		return *this;
	}

	void Swap(HashMap& hashMap) MOMO_NOEXCEPT
	{
		mHashSet.Swap(hashMap.mHashSet);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		return ConstIterator(mHashSet.GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return Iterator(mHashSet.GetBegin());
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator();
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator();
	}

	MOMO_FRIEND_SWAP(HashMap)
	MOMO_FRIENDS_BEGIN_END(const HashMap&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(HashMap&, Iterator)

	const HashTraits& GetHashTraits() const MOMO_NOEXCEPT
	{
		return mHashSet.GetHashTraits();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mHashSet.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mHashSet.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mHashSet.GetCount();
	}

	bool IsEmpty() const MOMO_NOEXCEPT
	{
		return mHashSet.IsEmpty();
	}

	void Clear(bool shrink = true) MOMO_NOEXCEPT
	{
		mHashSet.Clear(shrink);
	}

	size_t GetCapacity() const MOMO_NOEXCEPT
	{
		return mHashSet.GetCapacity();
	}

	void Reserve(size_t capacity)
	{
		mHashSet.Reserve(capacity);
	}

	void Shrink()
	{
		mHashSet.Shrink();
	}

	ConstIterator Find(const Key& key) const
	{
		return ConstIterator(mHashSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return Iterator(mHashSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type Find(const KeyArg& key) const
	{
		return ConstIterator(mHashSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type Find(const KeyArg& key)
	{
		return Iterator(mHashSet.Find(key));
	}

	bool HasKey(const Key& key) const
	{
		return mHashSet.HasKey(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, bool>::type HasKey(const KeyArg& key) const
	{
		return mHashSet.HasKey(key);
	}

	template<typename ValueCreator>
	InsertResult InsertCrt(Key&& key, const ValueCreator& valueCreator)
	{
		return _Insert(std::move(key), valueCreator);
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return _Insert(std::move(key),
			ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	InsertResult Insert(Key&& key, Value&& value)
	{
		return InsertVar(std::move(key), std::move(value));
	}

	InsertResult Insert(Key&& key, const Value& value)
	{
		return InsertVar(std::move(key), value);
	}

	template<typename ValueCreator>
	InsertResult InsertCrt(const Key& key, const ValueCreator& valueCreator)
	{
		return _Insert(key, valueCreator);
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return _Insert(key, ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	InsertResult Insert(const Key& key, Value&& value)
	{
		return InsertVar(key, std::move(value));
	}

	InsertResult Insert(const Key& key, const Value& value)
	{
		return InsertVar(key, value);
	}

	template<typename Iterator>
	size_t InsertKV(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->key);
		auto insertFunc = [this] (Iterator iter)
			{ return InsertVar(iter->key, iter->value); };
		return _Insert(begin, end, insertFunc);
	}

	template<typename Iterator>
	size_t InsertFS(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->first);
		auto insertFunc = [this] (Iterator iter)
			{ return InsertVar(iter->first, iter->second); };
		return _Insert(begin, end, insertFunc);
	}

	size_t Insert(std::initializer_list<std::pair<Key, Value>> keyValuePairs)
	{
		return InsertFS(keyValuePairs.begin(), keyValuePairs.end());
	}

	template<typename ValueCreator>
	Iterator AddCrt(ConstIterator iter, Key&& key, const ValueCreator& valueCreator)
	{
		return _Add(iter, std::move(key), valueCreator, true);
	}

	template<typename... ValueArgs>
	Iterator AddVar(ConstIterator iter, Key&& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(iter, std::move(key),
			ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstIterator iter, Key&& key, Value&& value)
	{
		return AddVar(iter, std::move(key), std::move(value));
	}

	Iterator Add(ConstIterator iter, Key&& key, const Value& value)
	{
		return AddVar(iter, std::move(key), value);
	}

	template<typename ValueCreator>
	Iterator AddCrt(ConstIterator iter, const Key& key, const ValueCreator& valueCreator)
	{
		return _Add(iter, key, valueCreator, true);
	}

	template<typename... ValueArgs>
	Iterator AddVar(ConstIterator iter, const Key& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(iter, key,
			ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstIterator iter, const Key& key, Value&& value)
	{
		return AddVar(iter, key, std::move(value));
	}

	Iterator Add(ConstIterator iter, const Key& key, const Value& value)
	{
		return AddVar(iter, key, value);
	}

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
	ValueReferenceRKey operator[](Key&& key)
	{
		Iterator iter = Find(static_cast<const Key&>(key));
		return ValueReferenceRKey(*this, iter, !!iter ? nullptr : std::addressof(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		Iterator iter = Find(key);
		return ValueReferenceCKey(*this, iter, !!iter ? nullptr : std::addressof(key));
	}
#else
	ValueReferenceRKey operator[](Key&& key)
	{
		return _Insert(std::move(key), ValueCreator<>()).iterator->value;
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		return _Insert(key, ValueCreator<>()).iterator->value;
	}
#endif

	Iterator Remove(ConstIterator iter)
	{
		return Iterator(mHashSet.Remove(iter.GetBaseIterator()));
	}

	bool Remove(const Key& key)
	{
		return mHashSet.Remove(key);
	}

	void MergeFrom(HashMap& srcHashMap)
	{
		mHashSet.MergeFrom(srcHashMap.mHashSet);
	}

	void MergeTo(HashMap& dstHashMap)
	{
		mHashSet.MergeTo(dstHashMap.mHashSet);
	}

	size_t GetBucketCount() const MOMO_NOEXCEPT
	{
		return mHashSet.GetBucketCount();
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		return ConstBucketBounds(mHashSet.GetBucketBounds(bucketIndex));
	}

	BucketBounds GetBucketBounds(size_t bucketIndex)
	{
		return BucketBounds(mHashSet.GetBucketBounds(bucketIndex));
	}

	size_t GetBucketIndex(const Key& key) const
	{
		return mHashSet.GetBucketIndex(key);
	}

private:
	bool _ExtraCheck(ConstIterator iter, const Key& key) const
	{
		if (!!iter)
			return false;
		return iter.GetBaseIterator().GetHashCode() == GetHashTraits().GetHashCode(key);
	}

	template<typename RKey, typename ValueCreator>
	InsertResult _Insert(RKey&& key, const ValueCreator& valueCreator)
	{
		Iterator iter = Find(static_cast<const Key&>(key));
		if (!!iter)
			return InsertResult(iter, false);
		iter = _Add(iter, std::forward<RKey>(key), valueCreator, false);
		return InsertResult(iter, true);
	}

	template<typename Iterator, typename InsertFunc>
	size_t _Insert(Iterator begin, Iterator end, InsertFunc insertFunc)
	{
		size_t count = 0;
		for (Iterator iter = begin; iter != end; ++iter)
			count += insertFunc(iter).inserted ? 1 : 0;
		return count;
	}

	template<typename RKey, typename ValueCreator>
	Iterator _Add(ConstIterator iter, RKey&& key, const ValueCreator& valueCreator,
		bool extraCheck)
	{
		(void)extraCheck;
		MOMO_EXTRA_CHECK(!extraCheck || _ExtraCheck(iter, static_cast<const Key&>(key)));
		auto pairCreator = [&key, &valueCreator] (void* ppair)
			{ new(ppair) KeyValuePair(std::forward<RKey>(key), valueCreator); };
		return Iterator(mHashSet.AddCrt(iter.GetBaseIterator(), pairCreator));
	}

private:
	HashSet mHashSet;
};

} // namespace momo
