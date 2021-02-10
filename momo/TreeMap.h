/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/TreeMap.h

  namespace momo:
    class TreeMapKeyValueTraits
    class TreeMapSettings
    class TreeMap
    class TreeMultiMap

\**********************************************************/

#pragma once

#include "TreeSet.h"
#include "MapUtility.h"

namespace momo
{

namespace internal
{
	template<typename TTreeSetIterator,
		bool tIsConst = false>
	class TreeMapIterator
	{
	protected:
		typedef TTreeSetIterator TreeSetIterator;

		static const bool isConst = tIsConst;

	public:
		typedef MapReference<typename TreeSetIterator::Reference, isConst> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef TreeMapIterator<TreeSetIterator, true> ConstIterator;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit TreeMapIterator() noexcept
			: mTreeSetIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mTreeSetIterator);
		}

		TreeMapIterator& operator++()
		{
			++mTreeSetIterator;
			return *this;
		}

		TreeMapIterator& operator--()
		{
			--mTreeSetIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mTreeSetIterator));
		}

		friend bool operator==(TreeMapIterator iter1, TreeMapIterator iter2) noexcept
		{
			return iter1.mTreeSetIterator == iter2.mTreeSetIterator;
		}

		MOMO_MORE_TREE_ITERATOR_OPERATORS(TreeMapIterator)

	protected:
		explicit TreeMapIterator(TreeSetIterator treeSetIter) noexcept
			: mTreeSetIterator(treeSetIter)
		{
		}

		TreeSetIterator ptGetTreeSetIterator() const noexcept
		{
			return mTreeSetIterator;
		}

	private:
		TreeSetIterator mTreeSetIterator;
	};

	template<typename TKeyValueTraits>
	class TreeMapNestedSetItemTraits : public MapNestedSetItemTraits<TKeyValueTraits>
	{
	private:
		typedef internal::MapNestedSetItemTraits<TKeyValueTraits> MapNestedSetItemTraits;

	protected:
		using typename MapNestedSetItemTraits::KeyValueTraits;
		using typename MapNestedSetItemTraits::Key;
		using typename MapNestedSetItemTraits::Value;

	public:
		using typename MapNestedSetItemTraits::Item;
		using typename MapNestedSetItemTraits::MemManager;

		static const bool isNothrowShiftable = KeyValueTraits::isKeyNothrowShiftable
			&& KeyValueTraits::isValueNothrowShiftable;

	public:
		template<typename Iterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			auto func = [&itemCreator, newItem] ()
				{ std::forward<ItemCreator>(itemCreator)(newItem); };
			KeyValueTraits::RelocateExec(memManager,
				MapKeyIterator<Iterator>(srcBegin), MapValueIterator<Iterator>(srcBegin),
				MapKeyIterator<Iterator>(dstBegin), MapValueIterator<Iterator>(dstBegin),
				count, func);
		}

		template<typename Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			KeyValueTraits::ShiftKeyNothrow(memManager, MapKeyIterator<Iterator>(begin), shift);
			KeyValueTraits::ShiftValueNothrow(memManager, MapValueIterator<Iterator>(begin), shift);
		}
	};

	template<typename TTreeMapSettings>
	class TreeMapNestedSetSettings //: public TreeSetSettings
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
private:
	typedef internal::MapKeyValueTraits<TKey, TValue, TMemManager> MapKeyValueTraits;

public:
	using typename MapKeyValueTraits::Key;
	using typename MapKeyValueTraits::Value;
	using typename MapKeyValueTraits::MemManager;

private:
	typedef internal::ObjectManager<Key, MemManager> KeyManager;
	typedef internal::ObjectManager<Value, MemManager> ValueManager;

public:
	static const bool isKeyNothrowShiftable = KeyManager::isNothrowShiftable;
	static const bool isValueNothrowShiftable = ValueManager::isNothrowShiftable;

public:
	template<typename KeyIterator>
	static void ShiftKeyNothrow(MemManager& memManager, KeyIterator keyBegin,
		size_t shift) noexcept
	{
		KeyManager::ShiftNothrow(memManager, keyBegin, shift);
	}

	template<typename ValueIterator>
	static void ShiftValueNothrow(MemManager& memManager, ValueIterator valueBegin,
		size_t shift) noexcept
	{
		ValueManager::ShiftNothrow(memManager, valueBegin, shift);
	}
};

class TreeMapSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
};

/*!
	All `TreeMap` functions and constructors have strong exception safety,
	but not the following cases:
	1. Functions `Insert` receiving many items have basic exception safety.
	2. Function `Remove` receiving predicate have basic exception safety.
	3. Functions `MergeFrom` and `MergeTo` have basic exception safety.
	4. In case default `KeyValueTraits`: if insert/add function receiving
	argument `Key&& key` throws exception, this argument may be changed.
	5. In case default `KeyValueTraits`: if function `Remove` throws exception
	and `ObjectManager<Key, MemManager>::isNothrowAnywayAssignable` is false
	and `ObjectManager<Value, MemManager>::isNothrowAnywayAssignable` is false,
	removing value may be changed.
*/

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
	typedef internal::TreeMapNestedSetItemTraits<KeyValueTraits> TreeSetItemTraits;
	typedef typename TreeSetItemTraits::Item KeyValuePair;

	typedef internal::TreeMapNestedSetSettings<Settings> TreeSetSettings;

	typedef momo::TreeSet<Key, TreeTraits, MemManager, TreeSetItemTraits, TreeSetSettings> TreeSet;

	typedef typename TreeSet::ConstIterator TreeSetConstIterator;

	typedef typename TreeSet::ExtractedItem TreeSetExtractedItem;

public:
	typedef internal::TreeMapIterator<TreeSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef internal::MapExtractedPair<TreeSetExtractedItem> ExtractedPair;

private:
	typedef internal::MapValueReferencer<TreeMap> ValueReferencer;

public:
	typedef typename ValueReferencer::template ValueReference<Key&&> ValueReferenceRKey;
	typedef typename ValueReferencer::template ValueReference<const Key&> ValueReferenceCKey;

private:
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	template<typename KeyArg>
	struct IsValidKeyArg : public TreeTraits::template IsValidKeyArg<KeyArg>
	{
	};

	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetTreeSetIterator, TreeSetConstIterator)
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

	explicit TreeMap(const TreeTraits& treeTraits, MemManager memManager = MemManager())
		: mTreeSet(treeTraits, std::move(memManager))
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	TreeMap(std::initializer_list<Pair> pairs)
		: TreeMap(pairs, TreeTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit TreeMap(std::initializer_list<Pair> pairs, const TreeTraits& treeTraits,
		MemManager memManager = MemManager())
		: TreeMap(treeTraits, std::move(memManager))
	{
		Insert(pairs);
	}

	TreeMap(TreeMap&& treeMap) noexcept
		: mTreeSet(std::move(treeMap.mTreeSet))
	{
	}

	TreeMap(const TreeMap& treeMap)
		: mTreeSet(treeMap.mTreeSet)
	{
	}

	explicit TreeMap(const TreeMap& treeMap, MemManager memManager)
		: mTreeSet(treeMap.mTreeSet, std::move(memManager))
	{
	}

	~TreeMap() = default;

	TreeMap& operator=(TreeMap&& treeMap) noexcept
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

	void Swap(TreeMap& treeMap) noexcept
	{
		mTreeSet.Swap(treeMap.mTreeSet);
	}

	ConstIterator GetBegin() const noexcept
	{
		return ConstIteratorProxy(mTreeSet.GetBegin());
	}

	Iterator GetBegin() noexcept
	{
		return IteratorProxy(mTreeSet.GetBegin());
	}

	ConstIterator GetEnd() const noexcept
	{
		return ConstIteratorProxy(mTreeSet.GetEnd());
	}

	Iterator GetEnd() noexcept
	{
		return IteratorProxy(mTreeSet.GetEnd());
	}

	MOMO_FRIEND_SWAP(TreeMap)
	MOMO_FRIENDS_BEGIN_END(const TreeMap&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(TreeMap&, Iterator)

	const TreeTraits& GetTreeTraits() const noexcept
	{
		return mTreeSet.GetTreeTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mTreeSet.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mTreeSet.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mTreeSet.GetCount();
	}

	bool IsEmpty() const noexcept
	{
		return mTreeSet.IsEmpty();
	}

	void Clear() noexcept
	{
		mTreeSet.Clear();
	}

	ConstIterator GetLowerBound(const Key& key) const
	{
		return ConstIteratorProxy(mTreeSet.GetLowerBound(key));
	}

	Iterator GetLowerBound(const Key& key)
	{
		return IteratorProxy(mTreeSet.GetLowerBound(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstIterator> GetLowerBound(
		const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeSet.GetLowerBound(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, Iterator> GetLowerBound(const KeyArg& key)
	{
		return IteratorProxy(mTreeSet.GetLowerBound(key));
	}

	ConstIterator GetUpperBound(const Key& key) const
	{
		return ConstIteratorProxy(mTreeSet.GetUpperBound(key));
	}

	Iterator GetUpperBound(const Key& key)
	{
		return IteratorProxy(mTreeSet.GetUpperBound(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstIterator> GetUpperBound(
		const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeSet.GetUpperBound(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, Iterator> GetUpperBound(const KeyArg& key)
	{
		return IteratorProxy(mTreeSet.GetUpperBound(key));
	}

	ConstIterator Find(const Key& key) const
	{
		return ConstIteratorProxy(mTreeSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return IteratorProxy(mTreeSet.Find(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstIterator> Find(const KeyArg& key) const
	{
		return ConstIteratorProxy(mTreeSet.Find(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, Iterator> Find(const KeyArg& key)
	{
		return IteratorProxy(mTreeSet.Find(key));
	}

	bool ContainsKey(const Key& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, bool> ContainsKey(const KeyArg& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	size_t GetKeyCount(const Key& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, size_t> GetKeyCount(const KeyArg& key) const
	{
		return mTreeSet.GetKeyCount(key);
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
		typename TreeSet::InsertResult res =
			mTreeSet.Insert(std::move(ExtractedPairProxy::GetSetExtractedItem(extPair)));
		return { IteratorProxy(res.iterator), res.inserted };
	}

	template<typename ArgIterator,
		typename = decltype(internal::MapPairConverter<ArgIterator>::Convert(*std::declval<ArgIterator>()))>
	size_t Insert(ArgIterator begin, ArgIterator end)
	{
		if (begin == end)
			return 0;
		const TreeTraits& treeTraits = GetTreeTraits();
		MemManager& memManager = GetMemManager();
		ArgIterator iter = begin;
		auto pair0 = internal::MapPairConverter<ArgIterator>::Convert(*iter);
		typedef decltype(pair0.first) KeyArg;
		typedef decltype(pair0.second) ValueArg;
		MOMO_STATIC_ASSERT((std::is_same<Key, typename std::decay<KeyArg>::type>::value));
		InsertResult res = InsertVar(std::forward<KeyArg>(pair0.first),
			std::forward<ValueArg>(pair0.second));
		size_t count = res.inserted ? 1 : 0;
		++iter;
		for (; iter != end; ++iter)
		{
			auto pair = internal::MapPairConverter<ArgIterator>::Convert(*iter);
			const Key& key = pair.first;
			const Key& prevKey = res.iterator->key;
			if (treeTraits.IsLess(key, prevKey) || !pvIsGreater(std::next(res.iterator), key))
			{
				res = InsertVar(std::forward<KeyArg>(pair.first), std::forward<ValueArg>(pair.second));
			}
			else if (TreeTraits::multiKey || treeTraits.IsLess(prevKey, key))
			{
				res.iterator = pvAdd<false>(std::next(res.iterator), std::forward<KeyArg>(pair.first),
					ValueCreator<ValueArg>(memManager, std::forward<ValueArg>(pair.second)));
				res.inserted = true;
			}
			else
			{
				res.inserted = false;
			}
			count += res.inserted ? 1 : 0;
		}
		return count;
	}

	template<typename Pair = std::pair<Key, Value>>
	size_t Insert(std::initializer_list<Pair> pairs)
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
		return IteratorProxy(mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetTreeSetIterator(iter), std::move(itemCreator)));
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
		return IteratorProxy(mTreeSet.Add(ConstIteratorProxy::GetTreeSetIterator(iter),
			std::move(ExtractedPairProxy::GetSetExtractedItem(extPair))));
	}

	ValueReferenceRKey operator[](Key&& key)
	{
		MOMO_STATIC_ASSERT(!TreeTraits::multiKey);
		Iterator iter = GetLowerBound(static_cast<const Key&>(key));
		return !pvIsGreater(iter, static_cast<const Key&>(key))
			? ValueReferencer::template GetReference<Key&&>(*this, iter)
			: ValueReferencer::template GetReference<Key&&>(*this, iter, std::move(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		MOMO_STATIC_ASSERT(!TreeTraits::multiKey);
		Iterator iter = GetLowerBound(key);
		return !pvIsGreater(iter, key)
			? ValueReferencer::template GetReference<const Key&>(*this, iter)
			: ValueReferencer::template GetReference<const Key&>(*this, iter, key);
	}

	Iterator Remove(ConstIterator iter)
	{
		return IteratorProxy(mTreeSet.Remove(ConstIteratorProxy::GetTreeSetIterator(iter)));
	}

	Iterator Remove(ConstIterator iter, ExtractedPair& extPair)
	{
		return IteratorProxy(mTreeSet.Remove(ConstIteratorProxy::GetTreeSetIterator(iter),
			ExtractedPairProxy::GetSetExtractedItem(extPair)));
	}

	Iterator Remove(ConstIterator begin, ConstIterator end)
	{
		return IteratorProxy(mTreeSet.Remove(ConstIteratorProxy::GetTreeSetIterator(begin),
			ConstIteratorProxy::GetTreeSetIterator(end)));
	}

	size_t Remove(const Key& key)
	{
		return mTreeSet.Remove(key);
	}

	template<typename PairPredicate>
	internal::EnableIf<internal::IsInvocable<const PairPredicate&, bool, const Key&, const Value&>::value,
		size_t>
	Remove(const PairPredicate& pairPred)
	{
		auto itemPred = [&pairPred] (const KeyValuePair& item)
			{ return pairPred(*item.GetKeyPtr(), *static_cast<const Value*>(item.GetValuePtr())); };
		return mTreeSet.Remove(itemPred);
	}

	ExtractedPair Extract(ConstIterator iter)
	{
		return ExtractedPair(*this, iter);	// need RVO for exception safety
	}

	template<typename KeyArg, bool extraCheck = true>
	void ResetKey(ConstIterator iter, KeyArg&& keyArg)
	{
		mTreeSet.template ResetKey<KeyArg, extraCheck>(ConstIteratorProxy::GetTreeSetIterator(iter),
			std::forward<KeyArg>(keyArg));
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
		return IteratorProxy(ConstIteratorProxy::GetTreeSetIterator(iter));
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		mTreeSet.CheckIterator(ConstIteratorProxy::GetTreeSetIterator(iter), allowEmpty);
	}

private:
	bool pvIsGreater(ConstIterator iter, const Key& key) const
	{
		return iter == GetEnd() || GetTreeTraits().IsLess(key, iter->key);
	}

	template<typename RKey, typename ValueCreator>
	InsertResult pvInsert(RKey&& key, ValueCreator&& valueCreator)
	{
		auto itemCreator = [this, &key, &valueCreator] (KeyValuePair* newItem)
		{
			KeyValueTraits::Create(GetMemManager(), std::forward<RKey>(key),
				std::forward<ValueCreator>(valueCreator), newItem->GetKeyPtr(),
				newItem->GetValuePtr());
		};
		typename TreeSet::InsertResult res = mTreeSet.InsertCrt(
			static_cast<const Key&>(key), itemCreator);
		return { IteratorProxy(res.iterator), res.inserted };
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
		return IteratorProxy(mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetTreeSetIterator(iter), std::move(itemCreator)));
	}

private:
	TreeSet mTreeSet;
};

template<typename TKey, typename TValue>
using TreeMultiMap = TreeMap<TKey, TValue, TreeTraits<TKey, true>>;

} // namespace momo

namespace std
{
	template<typename SI, bool c>
	struct iterator_traits<momo::internal::TreeMapIterator<SI, c>>
		: public momo::internal::IteratorTraitsStd<momo::internal::TreeMapIterator<SI, c>,
			bidirectional_iterator_tag>
	{
	};
} // namespace std
