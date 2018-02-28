/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/TreeMap.h

  namespace momo:
    class TreeMapKeyValueTraits
    struct TreeMapSettings
    class TreeMap

  All `TreeMap` functions and constructors have strong exception safety,
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

#include "TreeSet.h"
#include "MapUtility.h"

namespace momo
{

namespace internal
{
	template<typename TKeyValuePair>
	class TreeMapNestedSetItemTraits : public MapNestedSetItemTraits<TKeyValuePair>
	{
	protected:
		typedef TKeyValuePair KeyValuePair;
		typedef typename KeyValuePair::KeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::Value Value;

	public:
		typedef KeyValuePair Item;
		typedef typename KeyValueTraits::MemManager MemManager;

		static const bool isNothrowShiftable = KeyValueTraits::isKeyNothrowShiftable
			&& KeyValueTraits::isValueNothrowShiftable;

	public:
		template<typename Iterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, const ItemCreator& itemCreator, Item* newItem)
		{
			auto func = [&itemCreator, newItem] () { itemCreator(newItem); };
			KeyValueTraits::RelocateExec(memManager,
				MapKeyIterator<Iterator, Key>(srcBegin), MapValueIterator<Iterator, Value>(srcBegin),
				MapKeyIterator<Iterator, Key>(dstBegin), MapValueIterator<Iterator, Value>(dstBegin),
				count, func);
		}

		template<typename Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) MOMO_NOEXCEPT
		{
			KeyValueTraits::ShiftKeyNothrow(memManager,
				MapKeyIterator<Iterator, Key>(begin), shift);
			KeyValueTraits::ShiftValueNothrow(memManager,
				MapValueIterator<Iterator, Value>(begin), shift);
		}
	};

	template<typename TTreeMapSettings>
	struct TreeMapNestedSetSettings //: public TreeSetSettings
	{
	protected:
		typedef TTreeMapSettings TreeMapSettings;

	public:
		static const CheckMode checkMode = TreeMapSettings::checkMode;
		static const ExtraCheckMode extraCheckMode = TreeMapSettings::extraCheckMode;
		static const bool checkVersion = TreeMapSettings::checkVersion;
	};
}

template<typename TKey, typename TValue, typename TMemManager>
class TreeMapKeyValueTraits : public internal::MapKeyValueTraits<TKey, TValue, TMemManager>
{
public:
	typedef TKey Key;
	typedef TValue Value;
	typedef TMemManager MemManager;

private:
	typedef internal::ObjectManager<Key, MemManager> KeyManager;
	typedef internal::ObjectManager<Value, MemManager> ValueManager;

public:
	static const bool isKeyNothrowShiftable = KeyManager::isNothrowShiftable;
	static const bool isValueNothrowShiftable = ValueManager::isNothrowShiftable;

public:
	template<typename KeyIterator>
	static void ShiftKeyNothrow(MemManager& memManager, KeyIterator keyBegin,
		size_t shift) MOMO_NOEXCEPT
	{
		KeyManager::ShiftNothrow(memManager, keyBegin, shift);
	}

	template<typename ValueIterator>
	static void ShiftValueNothrow(MemManager& memManager, ValueIterator valueBegin,
		size_t shift) MOMO_NOEXCEPT
	{
		ValueManager::ShiftNothrow(memManager, valueBegin, shift);
	}
};

struct TreeMapSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
};

template<typename TKey, typename TValue,
	typename TTreeTraits = TreeTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TKeyValueTraits = TreeMapKeyValueTraits<TKey, TValue, TMemManager>,
	typename TSettings = TreeMapSettings>
class TreeMap
{
public:
	typedef TKey Key;
	typedef TValue Value;
	typedef TTreeTraits TreeTraits;
	typedef TMemManager MemManager;
	typedef TKeyValueTraits KeyValueTraits;
	typedef TSettings Settings;

private:
	typedef internal::MapKeyValuePair<KeyValueTraits> KeyValuePair;

	typedef internal::TreeMapNestedSetItemTraits<KeyValuePair> TreeSetItemTraits;
	typedef internal::TreeMapNestedSetSettings<Settings> TreeSetSettings;

	typedef momo::TreeSet<Key, TreeTraits, MemManager, TreeSetItemTraits, TreeSetSettings> TreeSet;

	typedef typename TreeSet::ConstIterator TreeSetConstIterator;
	typedef typename TreeSetConstIterator::Reference TreeSetConstReference;

	typedef internal::MapReference<Key, Value, TreeSetConstReference> Reference;

	typedef internal::MapValueReferencer<TreeMap> ValueReferencer;

	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	typedef typename TreeSet::ExtractedItem TreeSetExtractedItem;

public:
	typedef internal::TreeDerivedIterator<TreeSetConstIterator, Reference> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef typename ValueReferencer::template ValueReference<Key&&> ValueReferenceRKey;
	typedef typename ValueReferencer::template ValueReference<const Key&> ValueReferenceCKey;

	typedef internal::MapExtractedPair<TreeSetExtractedItem> ExtractedPair;

private:
	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator, TreeSetConstIterator)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct ExtractedPairProxy : private ExtractedPair
	{
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetSetExtractedItem, TreeSetExtractedItem&)
	};

public:
	TreeMap()
		: TreeMap(TreeTraits())
	{
	}

	explicit TreeMap(const TreeTraits& treeTraits, MemManager&& memManager = MemManager())
		: mTreeSet(treeTraits, std::move(memManager))
	{
	}

	TreeMap(std::initializer_list<std::pair<Key, Value>> keyValuePairs,
		const TreeTraits& treeTraits = TreeTraits(), MemManager&& memManager = MemManager())
		: TreeMap(treeTraits, std::move(memManager))
	{
		Insert(keyValuePairs);
	}

	TreeMap(TreeMap&& treeMap) MOMO_NOEXCEPT
		: mTreeSet(std::move(treeMap.mTreeSet))
	{
	}

	TreeMap(const TreeMap& treeMap)
		: mTreeSet(treeMap.mTreeSet)
	{
	}

	TreeMap(const TreeMap& treeMap, MemManager&& memManager)
		: mTreeSet(treeMap.mTreeSet, std::move(memManager))
	{
	}

	~TreeMap() MOMO_NOEXCEPT
	{
	}

	TreeMap& operator=(TreeMap&& treeMap) MOMO_NOEXCEPT
	{
		TreeMap(std::move(treeMap)).Swap(*this);
		return *this;
	}

	TreeMap& operator=(const TreeMap& treeMap)
	{
		if (this != &treeMap)
			TreeMap(treeMap).Swap(*this);
		return *this;
	}

	void Swap(TreeMap& treeMap) MOMO_NOEXCEPT
	{
		mTreeSet.Swap(treeMap.mTreeSet);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mTreeSet.GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return IteratorProxy(mTreeSet.GetBegin());
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mTreeSet.GetEnd());
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return IteratorProxy(mTreeSet.GetEnd());
	}

	MOMO_FRIEND_SWAP(TreeMap)
	MOMO_FRIENDS_BEGIN_END(const TreeMap&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(TreeMap&, Iterator)

	const TreeTraits& GetTreeTraits() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetTreeTraits();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mTreeSet.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetCount();
	}

	bool IsEmpty() const MOMO_NOEXCEPT
	{
		return mTreeSet.IsEmpty();
	}

	void Clear() MOMO_NOEXCEPT
	{
		mTreeSet.Clear();
	}

	ConstIterator LowerBound(const Key& key) const
	{
		return ConstIteratorProxy(mTreeSet.LowerBound(key));
	}

	Iterator LowerBound(const Key& key)
	{
		return IteratorProxy(mTreeSet.LowerBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type LowerBound(const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeSet.LowerBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type LowerBound(const KeyArg& key)
	{
		return IteratorProxy(mTreeSet.LowerBound(key));
	}

	ConstIterator UpperBound(const Key& key) const
	{
		return ConstIteratorProxy(mTreeSet.UpperBound(key));
	}

	Iterator UpperBound(const Key& key)
	{
		return IteratorProxy(mTreeSet.UpperBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type UpperBound(const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeSet.UpperBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type UpperBound(const KeyArg& key)
	{
		return IteratorProxy(mTreeSet.UpperBound(key));
	}

	ConstIterator Find(const Key& key) const
	{
		return ConstIteratorProxy(mTreeSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return IteratorProxy(mTreeSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type Find(const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type Find(const KeyArg& key)
	{
		return IteratorProxy(mTreeSet.Find(key));
	}

	bool HasKey(const Key& key) const
	{
		return mTreeSet.HasKey(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, bool>::type HasKey(const KeyArg& key) const
	{
		return mTreeSet.HasKey(key);
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
		typename TreeSet::InsertResult res =
			mTreeSet.Insert(std::move(ExtractedPairProxy::GetSetExtractedItem(extPair)));
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
		return IteratorProxy(mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
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
		return IteratorProxy(mTreeSet.Add(ConstIteratorProxy::GetBaseIterator(iter),
			std::move(ExtractedPairProxy::GetSetExtractedItem(extPair))));
	}

	ValueReferenceRKey operator[](Key&& key)
	{
		Iterator iter = LowerBound(static_cast<const Key&>(key));
		return pvIsEqual(iter, static_cast<const Key&>(key))
			? ValueReferencer::template GetReference<Key&&>(*this, iter)
			: ValueReferencer::template GetReference<Key&&>(*this, iter, std::move(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		Iterator iter = LowerBound(key);
		return pvIsEqual(iter, key)
			? ValueReferencer::template GetReference<const Key&>(*this, iter)
			: ValueReferencer::template GetReference<const Key&>(*this, iter, key);
	}

	Iterator Remove(ConstIterator iter)
	{
		return IteratorProxy(mTreeSet.Remove(ConstIteratorProxy::GetBaseIterator(iter)));
	}

	Iterator Remove(ConstIterator iter, ExtractedPair& extPair)
	{
		return IteratorProxy(mTreeSet.Remove(ConstIteratorProxy::GetBaseIterator(iter),
			ExtractedPairProxy::GetSetExtractedItem(extPair)));
	}

	bool Remove(const Key& key)
	{
		return mTreeSet.Remove(key);
	}

	ExtractedPair Extract(ConstIterator iter)
	{
		return ExtractedPair(*this, iter);	// need RVO for exception safety
	}

	void ResetKey(ConstIterator iter, Key&& newKey)
	{
		mTreeSet.ResetKey(ConstIteratorProxy::GetBaseIterator(iter), std::move(newKey));
	}

	void ResetKey(ConstIterator iter, const Key& newKey)
	{
		mTreeSet.ResetKey(ConstIteratorProxy::GetBaseIterator(iter), newKey);
	}

	template<typename RMap>
	void MergeFrom(RMap&& srcMap)
	{
		srcMap.MergeTo(mTreeSet);
	}

	template<typename Map>
	void MergeTo(Map& dstMap)
	{
		dstMap.MergeFrom(mTreeSet);
	}

	Iterator MakeMutableIterator(ConstIterator iter)
	{
		CheckIterator(iter);
		return IteratorProxy(ConstIteratorProxy::GetBaseIterator(iter));
	}

	void CheckIterator(ConstIterator iter) const
	{
		mTreeSet.CheckIterator(ConstIteratorProxy::GetBaseIterator(iter));
	}

private:
	bool pvIsEqual(ConstIterator iter, const Key& key) const
	{
		return iter != GetEnd() && !GetTreeTraits().IsLess(key, iter->key);
	}

	template<typename RKey, typename ValueCreator>
	InsertResult pvInsert(RKey&& key, const ValueCreator& valueCreator)
	{
		Iterator iter = LowerBound(static_cast<const Key&>(key));
		if (pvIsEqual(iter, static_cast<const Key&>(key)))
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
		return IteratorProxy(mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetBaseIterator(iter), itemCreator));
	}

private:
	TreeSet mTreeSet;
};

} // namespace momo
