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
  2. Functions `Merge` and `ExtractAll` have basic exception safety.
  3. If constructor receiving many items throws exception, input argument
    `memManager` may be changed.
  4. In case default `KeyValueTraits`: if insert/add function receiving
    argument `Key&& key` throws exception, this argument may be changed.
  5. In case default `KeyValueTraits`: if function `Remove` throws exception
    and `ObjectManager<Key>::isNothrowAnywayAssignable` is false
    and `ObjectManager<Value>::isNothrowAnywayAssignable` is false,
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
	public:
		typedef TKeyValuePair Item;
		typedef typename Item::KeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::Value Value;
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
	struct TreeMapNestedSetSettings : public TreeSetSettings
	{
		typedef TTreeMapSettings TreeMapSettings;

		static const CheckMode checkMode = TreeMapSettings::checkMode;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
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
	typedef internal::ObjectManager<Key> KeyManager;
	typedef internal::ObjectManager<Value> ValueManager;

public:
	static const bool isKeyNothrowShiftable = KeyManager::isNothrowShiftable;
	static const bool isValueNothrowShiftable = ValueManager::isNothrowShiftable;

public:
	template<typename KeyIterator>
	static void ShiftKeyNothrow(MemManager& /*memManager*/, KeyIterator keyBegin,
		size_t shift) MOMO_NOEXCEPT
	{
		KeyManager::ShiftNothrow(keyBegin, shift);
	}

	template<typename ValueIterator>
	static void ShiftValueNothrow(MemManager& /*memManager*/, ValueIterator valueBegin,
		size_t shift) MOMO_NOEXCEPT
	{
		ValueManager::ShiftNothrow(valueBegin, shift);
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

public:
	typedef internal::TreeDerivedIterator<TreeSetConstIterator, Reference> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef typename ValueReferencer::ValueReferenceRKey ValueReferenceRKey;
	typedef typename ValueReferencer::ValueReferenceCKey ValueReferenceCKey;

public:
	explicit TreeMap(const TreeTraits& treeTraits = TreeTraits(),
		MemManager&& memManager = MemManager())
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
		return ConstIterator(mTreeSet.GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return Iterator(mTreeSet.GetBegin());
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator(mTreeSet.GetEnd());
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator(mTreeSet.GetEnd());
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
		return ConstIterator(mTreeSet.LowerBound(key));
	}

	Iterator LowerBound(const Key& key)
	{
		return Iterator(mTreeSet.LowerBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type LowerBound(const KeyArg& key) const
	{
		return ConstIterator(mTreeSet.LowerBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type LowerBound(const KeyArg& key)
	{
		return Iterator(mTreeSet.LowerBound(key));
	}

	ConstIterator UpperBound(const Key& key) const
	{
		return ConstIterator(mTreeSet.UpperBound(key));
	}

	Iterator UpperBound(const Key& key)
	{
		return Iterator(mTreeSet.UpperBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type UpperBound(const KeyArg& key) const
	{
		return ConstIterator(mTreeSet.UpperBound(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type UpperBound(const KeyArg& key)
	{
		return Iterator(mTreeSet.UpperBound(key));
	}

	ConstIterator Find(const Key& key) const
	{
		return ConstIterator(mTreeSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return Iterator(mTreeSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type Find(const KeyArg& key) const
	{
		return ConstIterator(mTreeSet.Find(key));
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, Iterator>::type Find(const KeyArg& key)
	{
		return Iterator(mTreeSet.Find(key));
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

	template<typename ArgIterator>
	size_t InsertKV(ArgIterator begin, ArgIterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->key);
		auto insertFunc = [this] (ArgIterator iter)
			{ return InsertVar(iter->key, iter->value); };
		return _Insert(begin, end, insertFunc);
	}

	template<typename ArgIterator>
	size_t InsertFS(ArgIterator begin, ArgIterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->first);
		auto insertFunc = [this] (ArgIterator iter)
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
		Iterator iter = LowerBound(static_cast<const Key&>(key));
		return ValueReferenceRKey(*this, iter,
			_IsEqual(iter, static_cast<const Key&>(key)) ? nullptr : std::addressof(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		Iterator iter = LowerBound(key);
		return ValueReferenceCKey(*this, iter,
			_IsEqual(iter, key) ? nullptr : std::addressof(key));
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
		return Iterator(mTreeSet.Remove(iter.GetBaseIterator()));
	}

	bool Remove(const Key& key)
	{
		return mTreeSet.Remove(key);
	}

	void ResetKey(ConstIterator iter, Key&& newKey)
	{
		mTreeSet.ResetKey(iter.GetBaseIterator(), std::move(newKey));
	}

	void ResetKey(ConstIterator iter, const Key& newKey)
	{
		mTreeSet.ResetKey(iter.GetBaseIterator(), newKey);
	}

	template<typename RMap>
	void Merge(RMap&& srcMap)
	{
		typedef typename std::decay<RMap>::type Map;
		MOMO_STATIC_ASSERT((std::is_same<Key, typename Map::Key>::value));
		MOMO_STATIC_ASSERT((std::is_same<Value, typename Map::Value>::value));
		auto relocateFunc = [this, &srcMap] (Key& key, Value& value)
		{
			Insert(std::move(key), std::move(value));	//?
			Map::KeyValueTraits::Destroy(srcMap.GetMemManager(), key, value);
		};
		srcMap.ExtractAll(relocateFunc);
	}

	template<typename RelocateFunc>
	void ExtractAll(const RelocateFunc& relocateFunc)
	{
		auto setRelocateFunc = [&relocateFunc] (KeyValuePair& pair)
			{ relocateFunc(*pair.GetKeyPtr(), *pair.GetValuePtr()); };
		mTreeSet.ExtractAll(setRelocateFunc);
	}

private:
	bool _IsEqual(ConstIterator iter, const Key& key) const
	{
		return iter != GetEnd() && !GetTreeTraits().IsLess(key, iter->key);
	}

	bool _ExtraCheck(ConstIterator iter, const Key& key) const
	{
		const TreeTraits& treeTraits = GetTreeTraits();
		return (iter == GetBegin() || treeTraits.IsLess(std::prev(iter)->key, key))
			&& (iter == GetEnd() || treeTraits.IsLess(key, iter->key));
	}

	template<typename RKey, typename ValueCreator>
	InsertResult _Insert(RKey&& key, const ValueCreator& valueCreator)
	{
		Iterator iter = LowerBound(static_cast<const Key&>(key));
		if (_IsEqual(iter, static_cast<const Key&>(key)))
			return InsertResult(iter, false);
		iter = _Add(iter, std::forward<RKey>(key), valueCreator, false);
		return InsertResult(iter, true);
	}

	template<typename ArgIterator, typename InsertFunc>
	size_t _Insert(ArgIterator begin, ArgIterator end, InsertFunc insertFunc)
	{
		size_t count = 0;
		for (ArgIterator iter = begin; iter != end; ++iter)
			count += insertFunc(iter).inserted ? 1 : 0;
		return count;
	}

	template<typename RKey, typename ValueCreator>
	Iterator _Add(ConstIterator iter, RKey&& key, const ValueCreator& valueCreator,
		bool extraCheck)
	{
		(void)extraCheck;
		MOMO_EXTRA_CHECK(!extraCheck || _ExtraCheck(iter, static_cast<const Key&>(key)));
		auto pairCreator = [this, &key, &valueCreator] (KeyValuePair* newPair)
		{
			KeyValueTraits::Create(/*GetMemManager(), */std::forward<RKey>(key), valueCreator,
				newPair->GetKeyPtr(), newPair->GetValuePtr());
		};
		return Iterator(mTreeSet.AddCrt(iter.GetBaseIterator(), pairCreator));
	}

private:
	TreeSet mTreeSet;
};

} // namespace momo
