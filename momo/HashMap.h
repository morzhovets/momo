/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashMap.h

  namespace momo:
    class HashMapKeyValueTraits
    struct HashMapSettings
    class HashMap
    class HashMapOpen

  All `HashMap` functions and constructors have strong exception safety,
  but not the following cases:
  1. Functions `Insert`, `InsertKV`, `InsertFS` receiving many items have
    basic exception safety.
  2. Functions `MergeFrom` and `MergeTo` have basic exception safety.
  3. If constructor receiving many items throws exception, input argument
    `memManager` may be changed.
  4. In case default `KeyValueTraits`: if insert/add function receiving
    argument `Key&& key` throws exception, this argument may be changed.
  5. In case default `KeyValueTraits`: if function `Remove` throws exception
    and `ObjectManager<Key, MemManager>::isNothrowAnywayAssignable` is false
    and `ObjectManager<Value, MemManager>::isNothrowAnywayAssignable` is false,
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
	class HashMapNestedSetItemTraits : public MapNestedSetItemTraits<TKeyValuePair>
	{
	protected:
		typedef TKeyValuePair KeyValuePair;
		typedef typename KeyValuePair::KeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::Value Value;

	public:
		typedef KeyValuePair Item;
		typedef typename KeyValueTraits::MemManager MemManager;

	public:
		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, const ItemCreator& itemCreator, Item* newItem)
		{
			auto func = [&itemCreator, newItem] () { itemCreator(newItem); };
			KeyValueTraits::RelocateExec(memManager,
				MapKeyIterator<Item*, Key>(srcItems), MapValueIterator<Item*, Value>(srcItems),
				MapKeyIterator<Item*, Key>(dstItems), MapValueIterator<Item*, Value>(dstItems),
				count, func);
		}
	};

	template<typename THashMapSettings>
	struct HashMapNestedSetSettings //: public HashSetSettings
	{
	protected:
		typedef THashMapSettings HashMapSettings;

	public:
		static const CheckMode checkMode = HashMapSettings::checkMode;
		static const ExtraCheckMode extraCheckMode = HashMapSettings::extraCheckMode;
		static const bool checkVersion = HashMapSettings::checkVersion;

		static const bool overloadIfCannotGrow = HashMapSettings::overloadIfCannotGrow;
	};
}

template<typename TKey, typename TValue, typename TMemManager>
class HashMapKeyValueTraits : public internal::MapKeyValueTraits<TKey, TValue, TMemManager>
{
};

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
	typename TKeyValueTraits = HashMapKeyValueTraits<TKey, TValue, TMemManager>,
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

	typedef typename HashSet::ExtractedItem HashSetExtractedItem;

	typedef typename HashSet::ConstBucketBounds HashSetConstBucketBounds;
	typedef internal::HashDerivedIterator<typename HashSetConstBucketBounds::Iterator,
		Reference> BucketIterator;

	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

public:
	typedef internal::HashDerivedIterator<HashSetConstIterator, Reference> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef typename ValueReferencer::template ValueReference<Key&&> ValueReferenceRKey;
	typedef typename ValueReferencer::template ValueReference<const Key&> ValueReferenceCKey;

	typedef internal::MapExtractedPair<HashSetExtractedItem> ExtractedPair;

	typedef internal::HashDerivedBucketBounds<BucketIterator,
		HashSetConstBucketBounds> BucketBounds;
	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

private:
	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator, HashSetConstIterator)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct ExtractedPairProxy : private ExtractedPair
	{
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetSetExtractedItem, HashSetExtractedItem&)
	};

	struct ConstBucketBoundsProxy : public ConstBucketBounds
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstBucketBounds)
	};

	struct BucketBoundsProxy : public BucketBounds
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(BucketBounds)
	};

public:
	HashMap()
		: HashMap(HashTraits())
	{
	}

	explicit HashMap(const HashTraits& hashTraits, MemManager&& memManager = MemManager())
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

	HashMap(const HashMap& hashMap, MemManager&& memManager)
		: mHashSet(hashMap.mHashSet, std::move(memManager))
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
		return ConstIteratorProxy(mHashSet.GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return IteratorProxy(mHashSet.GetBegin());
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
		return ConstIteratorProxy(mHashSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return IteratorProxy(mHashSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type Find(const KeyArg& key) const
	{
		return ConstIteratorProxy(mHashSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type Find(const KeyArg& key)
	{
		return IteratorProxy(mHashSet.Find(key));
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
		return pvInsert(std::move(key), valueCreator);
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return pvInsert(std::move(key),
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
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
		return pvInsert(key, valueCreator);
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return pvInsert(key,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	InsertResult Insert(const Key& key, Value&& value)
	{
		return InsertVar(key, std::move(value));
	}

	InsertResult Insert(const Key& key, const Value& value)
	{
		return InsertVar(key, value);
	}

	InsertResult Insert(ExtractedPair&& extPair)
	{
		typename HashSet::InsertResult res =
			mHashSet.Insert(std::move(ExtractedPairProxy::GetSetExtractedItem(extPair)));
		return InsertResult(IteratorProxy(res.iterator), res.inserted);
	}

	template<typename ArgIterator>
	size_t InsertKV(ArgIterator begin, ArgIterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->key);
		auto insertFunc = [this] (ArgIterator iter)
			{ return InsertVar(iter->key, iter->value); };
		return pvInsert(begin, end, insertFunc);
	}

	template<typename ArgIterator>
	size_t InsertFS(ArgIterator begin, ArgIterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->first);
		auto insertFunc = [this] (ArgIterator iter)
			{ return InsertVar(iter->first, iter->second); };
		return pvInsert(begin, end, insertFunc);
	}

	size_t Insert(std::initializer_list<std::pair<Key, Value>> keyValuePairs)
	{
		return InsertFS(keyValuePairs.begin(), keyValuePairs.end());
	}

	template<typename PairCreator, bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, const PairCreator& pairCreator)
	{
		auto itemCreator = [&pairCreator] (KeyValuePair* newItem)
			{ pairCreator(newItem->GetKeyPtr(), newItem->GetValuePtr()); };
		return IteratorProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetBaseIterator(iter), itemCreator));
	}

	template<typename ValueCreator, bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, Key&& key, const ValueCreator& valueCreator)
	{
		return pvAdd<extraCheck>(iter, std::move(key), valueCreator);
	}

	template<typename... ValueArgs>
	Iterator AddVar(ConstIterator iter, Key&& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(iter, std::move(key),
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstIterator iter, Key&& key, Value&& value)
	{
		return AddVar(iter, std::move(key), std::move(value));
	}

	Iterator Add(ConstIterator iter, Key&& key, const Value& value)
	{
		return AddVar(iter, std::move(key), value);
	}

	template<typename ValueCreator, bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, const Key& key, const ValueCreator& valueCreator)
	{
		return pvAdd<extraCheck>(iter, key, valueCreator);
	}

	template<typename... ValueArgs>
	Iterator AddVar(ConstIterator iter, const Key& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(iter, key,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstIterator iter, const Key& key, Value&& value)
	{
		return AddVar(iter, key, std::move(value));
	}

	Iterator Add(ConstIterator iter, const Key& key, const Value& value)
	{
		return AddVar(iter, key, value);
	}

	Iterator Add(ConstIterator iter, ExtractedPair&& extPair)
	{
		return IteratorProxy(mHashSet.Add(ConstIteratorProxy::GetBaseIterator(iter),
			std::move(ExtractedPairProxy::GetSetExtractedItem(extPair))));
	}

	ValueReferenceRKey operator[](Key&& key)
	{
		Iterator iter = Find(static_cast<const Key&>(key));
		return !!iter ? ValueReferencer::template GetReference<Key&&>(*this, iter)
			: ValueReferencer::template GetReference<Key&&>(*this, iter, std::move(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		Iterator iter = Find(key);
		return !!iter ? ValueReferencer::template GetReference<const Key&>(*this, iter)
			: ValueReferencer::template GetReference<const Key&>(*this, iter, key);
	}

	Iterator Remove(ConstIterator iter)
	{
		return IteratorProxy(mHashSet.Remove(ConstIteratorProxy::GetBaseIterator(iter)));
	}

	Iterator Remove(ConstIterator iter, ExtractedPair& extPair)
	{
		return IteratorProxy(mHashSet.Remove(ConstIteratorProxy::GetBaseIterator(iter),
			ExtractedPairProxy::GetSetExtractedItem(extPair)));
	}

	bool Remove(const Key& key)
	{
		return mHashSet.Remove(key);
	}

	ExtractedPair Extract(ConstIterator iter)
	{
		return ExtractedPair(*this, iter);	// need RVO for exception safety
	}

	void ResetKey(ConstIterator iter, Key&& newKey)
	{
		mHashSet.ResetKey(ConstIteratorProxy::GetBaseIterator(iter), std::move(newKey));
	}

	void ResetKey(ConstIterator iter, const Key& newKey)
	{
		mHashSet.ResetKey(ConstIteratorProxy::GetBaseIterator(iter), newKey);
	}

	template<typename RMap>
	void MergeFrom(RMap&& srcMap)
	{
		srcMap.MergeTo(mHashSet);
	}

	template<typename Map>
	void MergeTo(Map& dstMap)
	{
		dstMap.MergeFrom(mHashSet);
	}

	size_t GetBucketCount() const MOMO_NOEXCEPT
	{
		return mHashSet.GetBucketCount();
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		return ConstBucketBoundsProxy(mHashSet.GetBucketBounds(bucketIndex));
	}

	BucketBounds GetBucketBounds(size_t bucketIndex)
	{
		return BucketBoundsProxy(mHashSet.GetBucketBounds(bucketIndex));
	}

	size_t GetBucketIndex(const Key& key) const
	{
		return mHashSet.GetBucketIndex(key);
	}

	ConstIterator MakeIterator(size_t hashCode) const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mHashSet.MakeIterator(hashCode));
	}

	Iterator MakeMutableIterator(ConstIterator iter)
	{
		CheckIterator(iter);
		return IteratorProxy(ConstIteratorProxy::GetBaseIterator(iter));
	}

	void CheckIterator(ConstIterator iter) const
	{
		mHashSet.CheckIterator(ConstIteratorProxy::GetBaseIterator(iter));
	}

private:
	template<typename RKey, typename ValueCreator>
	InsertResult pvInsert(RKey&& key, const ValueCreator& valueCreator)
	{
		Iterator iter = Find(static_cast<const Key&>(key));
		if (!!iter)
			return InsertResult(iter, false);
		iter = pvAdd<false>(iter, std::forward<RKey>(key), valueCreator);
		return InsertResult(iter, true);
	}

	template<typename ArgIterator, typename InsertFunc>
	size_t pvInsert(ArgIterator begin, ArgIterator end, InsertFunc insertFunc)
	{
		size_t count = 0;
		for (ArgIterator iter = begin; iter != end; ++iter)
			count += insertFunc(iter).inserted ? 1 : 0;
		return count;
	}

	template<bool extraCheck, typename RKey, typename ValueCreator>
	Iterator pvAdd(ConstIterator iter, RKey&& key, const ValueCreator& valueCreator)
	{
		auto itemCreator = [this, &key, &valueCreator] (KeyValuePair* newItem)
		{
			KeyValueTraits::Create(GetMemManager(), std::forward<RKey>(key), valueCreator,
				newItem->GetKeyPtr(), newItem->GetValuePtr());
		};
		return IteratorProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetBaseIterator(iter), itemCreator));
	}

private:
	HashSet mHashSet;
};

template<typename TKey, typename TValue>
using HashMapOpen = HashMap<TKey, TValue, HashTraitsOpen<TKey>>;

} // namespace momo
