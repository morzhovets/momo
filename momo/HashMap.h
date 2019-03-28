/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashMap.h

  namespace momo:
    class HashMapKeyValueTraits
    class HashMapSettings
    class HashMap
    class HashMapOpen

  All `HashMap` functions and constructors have strong exception safety,
  but not the following cases:
  1. Functions `Insert` receiving many items have basic exception safety.
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
	private:
		typedef internal::MapNestedSetItemTraits<TKeyValuePair> MapNestedSetItemTraits;

	protected:
		using typename MapNestedSetItemTraits::KeyValueTraits;
		using typename MapNestedSetItemTraits::Key;
		using typename MapNestedSetItemTraits::Value;

	public:
		using typename MapNestedSetItemTraits::Item;
		using typename MapNestedSetItemTraits::MemManager;

	public:
		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			auto func = [&itemCreator, newItem] ()
				{ std::forward<ItemCreator>(itemCreator)(newItem); };
			KeyValueTraits::RelocateExec(memManager,
				MapKeyIterator<Item*, Key>(srcItems), MapValueIterator<Item*, Value>(srcItems),
				MapKeyIterator<Item*, Key>(dstItems), MapValueIterator<Item*, Value>(dstItems),
				count, func);
		}
	};

	template<typename THashMapSettings>
	class HashMapNestedSetSettings //: public HashSetSettings
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

class HashMapSettings
{
public:
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
	: private internal::Swappable<HashMap<TKey, TValue, THashTraits, TMemManager,
		TKeyValueTraits, TSettings>>
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

	static const size_t bucketMaxItemCount = HashSet::bucketMaxItemCount;

private:
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	template<typename KeyArg>
	struct IsValidKeyArg : public HashTraits::template IsValidKeyArg<KeyArg>
	{
	};

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

	HashMap(std::initializer_list<std::pair<Key, Value>> pairs,
		const HashTraits& hashTraits = HashTraits(), MemManager&& memManager = MemManager())
		: HashMap(hashTraits, std::move(memManager))
	{
		Insert(pairs);
	}

	HashMap(HashMap&& hashMap) noexcept
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

	~HashMap() noexcept
	{
	}

	HashMap& operator=(HashMap&& hashMap) noexcept
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

	void Swap(HashMap& hashMap) noexcept
	{
		mHashSet.Swap(hashMap.mHashSet);
	}

	ConstIterator GetBegin() const noexcept
	{
		return ConstIteratorProxy(mHashSet.GetBegin());
	}

	Iterator GetBegin() noexcept
	{
		return IteratorProxy(mHashSet.GetBegin());
	}

	ConstIterator GetEnd() const noexcept
	{
		return ConstIterator();
	}

	Iterator GetEnd() noexcept
	{
		return Iterator();
	}

	MOMO_FRIENDS_BEGIN_END(const HashMap&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(HashMap&, Iterator)

	const HashTraits& GetHashTraits() const noexcept
	{
		return mHashSet.GetHashTraits();
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
	}

	size_t GetCapacity() const noexcept
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

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstIterator> Find(const KeyArg& key) const
	{
		return ConstIteratorProxy(mHashSet.Find(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, Iterator> Find(const KeyArg& key)
	{
		return IteratorProxy(mHashSet.Find(key));
	}

	ConstIterator Find(const Key& key, size_t hashCode) const
	{
		return ConstIteratorProxy(mHashSet.Find(key, hashCode));
	}

	Iterator Find(const Key& key, size_t hashCode)
	{
		return IteratorProxy(mHashSet.Find(key, hashCode));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstIterator> Find(const KeyArg& key,
		size_t hashCode) const
	{
		return ConstIteratorProxy(mHashSet.Find(key, hashCode));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, Iterator> Find(const KeyArg& key,
		size_t hashCode)
	{
		return IteratorProxy(mHashSet.Find(key, hashCode));
	}

	bool ContainsKey(const Key& key) const
	{
		return mHashSet.ContainsKey(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, bool> ContainsKey(const KeyArg& key) const
	{
		return mHashSet.ContainsKey(key);
	}

	template<typename ValueCreator>
	InsertResult InsertCrt(Key&& key, ValueCreator&& valueCreator)
	{
		return pvInsert(std::move(key), std::forward<ValueCreator>(valueCreator));
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
	InsertResult InsertCrt(const Key& key, ValueCreator&& valueCreator)
	{
		return pvInsert(key, std::forward<ValueCreator>(valueCreator));
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
		return { IteratorProxy(res.iterator), res.inserted };
	}

	template<typename ArgIterator>
	size_t Insert(ArgIterator begin, ArgIterator end)
	{
		size_t count = 0;
		for (ArgIterator iter = begin; iter != end; ++iter)
		{
			auto pair = internal::MapPairConverter<ArgIterator>::Convert(*iter);
			typedef decltype(pair.first) KeyArg;
			typedef decltype(pair.second) ValueArg;
			MOMO_STATIC_ASSERT((std::is_same<Key, typename std::decay<KeyArg>::type>::value));
			InsertResult res = InsertVar(std::forward<KeyArg>(pair.first),
				std::forward<ValueArg>(pair.second));
			count += res.inserted ? 1 : 0;
		}
		return count;
	}

	size_t Insert(std::initializer_list<std::pair<Key, Value>> pairs)	//?
	{
		return Insert(pairs.begin(), pairs.end());
	}

	template<typename PairCreator, bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, PairCreator&& pairCreator)
	{
		auto itemCreator = [&pairCreator] (KeyValuePair* newItem)
		{
			std::forward<PairCreator>(pairCreator)(newItem->GetKeyPtr(), newItem->GetValuePtr());
		};
		return IteratorProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetBaseIterator(iter), std::move(itemCreator)));
	}

	template<typename ValueCreator, bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, Key&& key, ValueCreator&& valueCreator)
	{
		return pvAdd<extraCheck>(iter, std::move(key), std::forward<ValueCreator>(valueCreator));
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
	Iterator AddCrt(ConstIterator iter, const Key& key, ValueCreator&& valueCreator)
	{
		return pvAdd<extraCheck>(iter, key, std::forward<ValueCreator>(valueCreator));
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

	template<typename KeyArg, bool extraCheck = true>
	void ResetKey(ConstIterator iter, KeyArg&& keyArg)
	{
		mHashSet.template ResetKey<KeyArg, extraCheck>(ConstIteratorProxy::GetBaseIterator(iter),
			std::forward<KeyArg>(keyArg));
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

	size_t GetBucketCount() const noexcept
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

	ConstIterator MakeIterator(size_t hashCode) const noexcept
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
	InsertResult pvInsert(RKey&& key, ValueCreator&& valueCreator)
	{
		Iterator iter = Find(static_cast<const Key&>(key));
		if (!!iter)
			return { iter, false };
		iter = pvAdd<false>(iter, std::forward<RKey>(key),
			std::forward<ValueCreator>(valueCreator));
		return { iter, true };
	}

	template<bool extraCheck, typename RKey, typename ValueCreator>
	Iterator pvAdd(ConstIterator iter, RKey&& key, ValueCreator&& valueCreator)
	{
		auto itemCreator = [this, &key, &valueCreator] (KeyValuePair* newItem)
		{
			KeyValueTraits::Create(GetMemManager(), std::forward<RKey>(key),
				std::forward<ValueCreator>(valueCreator), newItem->GetKeyPtr(),
				newItem->GetValuePtr());
		};
		return IteratorProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetBaseIterator(iter), std::move(itemCreator)));
	}

private:
	HashSet mHashSet;
};

template<typename TKey, typename TValue>
using HashMapOpen = HashMap<TKey, TValue, HashTraitsOpen<TKey>>;

namespace internal
{
	class NestedHashMapSettings : public HashMapSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = false;
	};
}

} // namespace momo
