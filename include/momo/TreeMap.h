/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/TreeMap.h

  namespace momo:
    class TreeMapKeyValueTraits
    class TreeMapSettings
    class TreeMapCore
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
	template<typename TKeyValueTraits>
	class TreeMapNestedSetItemTraits;

	template<typename TKeyValueTraits>
	requires (!TKeyValueTraits::useValuePtr)
	class TreeMapNestedSetItemTraits<TKeyValueTraits> : public MapNestedSetItemTraits<TKeyValueTraits>
	{
	private:
		typedef internal::MapNestedSetItemTraits<TKeyValueTraits> MapNestedSetItemTraits;

	protected:
		using typename MapNestedSetItemTraits::KeyValueTraits;
		using typename MapNestedSetItemTraits::Value;

	public:
		using typename MapNestedSetItemTraits::Item;
		using typename MapNestedSetItemTraits::MemManager;

		static const bool isNothrowShiftable = KeyValueTraits::isKeyNothrowShiftable
			&& KeyValueTraits::isValueNothrowShiftable;

	public:
		template<conceptIncIterator<Item> Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			IncIterator keyIter = [iter = begin] () mutable noexcept
				{ return (iter++)->template GetKeyPtr<true>(); };
			KeyValueTraits::ShiftKeyNothrow(memManager, keyIter, shift);
			IncIterator valueIter = [iter = begin] () mutable noexcept
				{ return (iter++)->template GetValuePtr<true>(); };
			KeyValueTraits::ShiftValueNothrow(memManager, valueIter, shift);
		}
	};

	template<typename TKeyValueTraits>
	requires (TKeyValueTraits::useValuePtr)
	class TreeMapNestedSetItemTraits<TKeyValueTraits> : public MapNestedSetItemTraits<TKeyValueTraits>
	{
	private:
		typedef internal::MapNestedSetItemTraits<TKeyValueTraits> MapNestedSetItemTraits;

	protected:
		using typename MapNestedSetItemTraits::KeyValueTraits;
		using typename MapNestedSetItemTraits::Value;

	public:
		using typename MapNestedSetItemTraits::Item;
		using typename MapNestedSetItemTraits::MemManager;

		static const bool isNothrowShiftable = KeyValueTraits::isKeyNothrowShiftable;

	public:
		template<conceptIncIterator<Item> Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			IncIterator keyIter = [iter = begin] () mutable noexcept
				{ return (iter++)->template GetKeyPtr<true>(); };
			KeyValueTraits::ShiftKeyNothrow(memManager, keyIter, shift);
			IncIterator valueIter = [iter = begin] () mutable noexcept
				{ return &(iter++)->GetValuePtr(); };
			ObjectManager<Value*, MemManager>::ShiftNothrow(memManager, valueIter, shift);
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
		static const bool allowExceptionSuppression = TreeMapSettings::allowExceptionSuppression;
	};
}

template<conceptObject TKey, conceptObject TValue,
	conceptMemManager TMemManager = MemManagerDefault,
	bool tUseValuePtr = false>
class TreeMapKeyValueTraits;

template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager>
class TreeMapKeyValueTraits<TKey, TValue, TMemManager, false>
	: public internal::MapKeyValueTraits<TKey, TValue, TMemManager, false>
{
private:
	typedef internal::MapKeyValueTraits<TKey, TValue, TMemManager, false> MapKeyValueTraits;

	using typename MapKeyValueTraits::KeyManager;
	using typename MapKeyValueTraits::ValueManager;

public:
	using typename MapKeyValueTraits::Key;
	using typename MapKeyValueTraits::Value;
	using typename MapKeyValueTraits::MemManager;

	static const bool isKeyNothrowShiftable = KeyManager::isNothrowShiftable;
	static const bool isValueNothrowShiftable = ValueManager::isNothrowShiftable;

public:
	template<internal::conceptIncIterator<Key> KeyIterator>
	static void ShiftKeyNothrow(MemManager& memManager, KeyIterator keyBegin,
		size_t shift) noexcept
	{
		KeyManager::ShiftNothrow(memManager, keyBegin, shift);
	}

	template<internal::conceptIncIterator<Value> ValueIterator>
	static void ShiftValueNothrow(MemManager& memManager, ValueIterator valueBegin,
		size_t shift) noexcept
	{
		ValueManager::ShiftNothrow(memManager, valueBegin, shift);
	}
};

template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager>
class TreeMapKeyValueTraits<TKey, TValue, TMemManager, true>
	: public internal::MapKeyValueTraits<TKey, TValue, TMemManager, true>
{
private:
	typedef internal::MapKeyValueTraits<TKey, TValue, TMemManager, true> MapKeyValueTraits;

	using typename MapKeyValueTraits::KeyManager;

public:
	using typename MapKeyValueTraits::Key;
	using typename MapKeyValueTraits::MemManager;

	static const bool isKeyNothrowShiftable = KeyManager::isNothrowShiftable;

public:
	template<internal::conceptIncIterator<Key> KeyIterator>
	static void ShiftKeyNothrow(MemManager& memManager, KeyIterator keyBegin,
		size_t shift) noexcept
	{
		KeyManager::ShiftNothrow(memManager, keyBegin, shift);
	}
};

class TreeMapSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
	static const bool allowExceptionSuppression = true;
};

/*!
	All `TreeMapCore` functions and constructors have strong exception safety,
	but not the following cases:
	1. Functions `Insert` receiving many items have basic exception safety.
	2. Function `Remove` receiving predicate has basic exception safety.
	3. Functions `MergeFrom` and `MergeTo` have basic exception safety.
	4. In case default `KeyValueTraits`: if insert/add function receiving
	argument `Key&& key` throws exception, this argument may be changed.
	5. In case default `KeyValueTraits`: if function `Remove` throws exception
	and `ObjectManager<Key, MemManager>::isNothrowAnywayAssignable` is false
	and `ObjectManager<Value, MemManager>::isNothrowAnywayAssignable` is false,
	removing value may be changed.
*/

template<typename TKeyValueTraits,
	conceptTreeTraits<typename TKeyValueTraits::Key> TTreeTraits
		= TreeTraits<typename TKeyValueTraits::Key>,
	typename TSettings = TreeMapSettings>
requires conceptMapKeyValueTraits<TKeyValueTraits, typename TKeyValueTraits::Key,
	typename TKeyValueTraits::Value, typename TKeyValueTraits::MemManager>
class MOMO_EMPTY_BASES TreeMapCore
	: public internal::Rangeable,
	public internal::Swappable<TreeMapCore>
{
public:
	typedef TKeyValueTraits KeyValueTraits;
	typedef TTreeTraits TreeTraits;
	typedef TSettings Settings;
	typedef typename KeyValueTraits::Key Key;
	typedef typename KeyValueTraits::Value Value;
	typedef typename KeyValueTraits::MemManager MemManager;

private:
	typedef internal::TreeMapNestedSetItemTraits<KeyValueTraits> TreeSetItemTraits;
	typedef typename TreeSetItemTraits::Item KeyValuePair;

	typedef internal::TreeMapNestedSetSettings<Settings> TreeSetSettings;

	typedef TreeSetCore<TreeSetItemTraits, TreeTraits, TreeSetSettings> TreeSet;

	typedef typename TreeSet::ConstIterator TreeSetConstIterator;

	typedef typename TreeSet::ExtractedItem TreeSetExtractedItem;

public:
	typedef internal::MapBidirectionalIterator<TreeSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef internal::MapExtractedPair<TreeSetExtractedItem,
		KeyValueTraits::useValuePtr> ExtractedPair;

private:
	typedef internal::MapValueReferencer<TreeMapCore, Iterator> ValueReferencer;

public:
	template<typename KeyReference>
	using ValueReference = ValueReferencer::template ValueReference<KeyReference>;

private:
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	template<typename KeyArg>
	using IsValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>;

	struct ConstIteratorProxy : private ConstIterator
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetSetIterator)
	};

	struct ExtractedPairProxy : private ExtractedPair
	{
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetSetExtractedItem)
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetValueMemPool)
	};

public:
	TreeMapCore()
		: TreeMapCore(TreeTraits())
	{
	}

	explicit TreeMapCore(const TreeTraits& treeTraits, MemManager memManager = MemManager())
		: mTreeSet(treeTraits, std::move(memManager))
	{
	}

	template<internal::conceptMapArgIterator<Key> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit TreeMapCore(ArgIterator begin, ArgSentinel end,
		const TreeTraits& treeTraits = TreeTraits(), MemManager memManager = MemManager())
		: TreeMapCore(treeTraits, std::move(memManager))
	{
		Insert(std::move(begin), std::move(end));
	}

	template<typename Pair = std::pair<Key, Value>>
	TreeMapCore(std::initializer_list<Pair> pairs)
		: TreeMapCore(pairs, TreeTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit TreeMapCore(std::initializer_list<Pair> pairs, const TreeTraits& treeTraits,
		MemManager memManager = MemManager())
		: TreeMapCore(pairs.begin(), pairs.end(), treeTraits, std::move(memManager))
	{
	}

	TreeMapCore(TreeMapCore&& treeMap) noexcept
		: mTreeSet(std::move(treeMap.mTreeSet))
	{
	}

	TreeMapCore(const TreeMapCore& treeMap)
		: mTreeSet(treeMap.mTreeSet)
	{
	}

	explicit TreeMapCore(const TreeMapCore& treeMap, MemManager memManager)
		: mTreeSet(treeMap.mTreeSet, std::move(memManager))
	{
	}

	~TreeMapCore() noexcept = default;

	TreeMapCore& operator=(TreeMapCore&& treeMap) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(treeMap), *this);
	}

	TreeMapCore& operator=(const TreeMapCore& treeMap)
	{
		return internal::ContainerAssigner::Copy(treeMap, *this);
	}

	void Swap(TreeMapCore& treeMap) noexcept
	{
		mTreeSet.Swap(treeMap.mTreeSet);
	}

	ConstIterator GetBegin() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetBegin());
	}

	Iterator GetBegin() noexcept
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.GetBegin());
	}

	ConstIterator GetEnd() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetEnd());
	}

	Iterator GetEnd() noexcept
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.GetEnd());
	}

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
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetLowerBound(key));
	}

	Iterator GetLowerBound(const Key& key)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.GetLowerBound(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	ConstIterator GetLowerBound(const KeyArg& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetLowerBound(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	Iterator GetLowerBound(const KeyArg& key)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.GetLowerBound(key));
	}

	ConstIterator GetUpperBound(const Key& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetUpperBound(key));
	}

	Iterator GetUpperBound(const Key& key)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.GetUpperBound(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	ConstIterator GetUpperBound(const KeyArg& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetUpperBound(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	Iterator GetUpperBound(const KeyArg& key)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.GetUpperBound(key));
	}

	ConstIterator Find(const Key& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	ConstIterator Find(const KeyArg& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	Iterator Find(const KeyArg& key)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.Find(key));
	}

	bool ContainsKey(const Key& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	bool ContainsKey(const KeyArg& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	size_t GetKeyCount(const Key& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	size_t GetKeyCount(const KeyArg& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	template<internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult InsertCrt(Key&& key, ValueCreator valueCreator)
	{
		return pvInsert(std::move(key), FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	InsertResult InsertVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return InsertCrt(std::move(key),
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

	template<internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult InsertCrt(const Key& key, ValueCreator valueCreator)
	{
		return pvInsert(key, FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	InsertResult InsertVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return InsertCrt(key,
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
		if constexpr (KeyValueTraits::useValuePtr)
		{
			if (ExtractedPairProxy::GetValueMemPool(extPair) != &mTreeSet.GetMemManager().GetMemPool())
			{
				auto itemCreator = [this, &extPair] (KeyValuePair* newItem)
				{
					auto pairRemover = [this, newItem] (Key& key, Value& value)
					{
						KeyValuePair::template CreateRelocate<KeyValueTraits>(
							newItem, nullptr, mTreeSet.GetMemManager(), key, value);
					};
					extPair.Remove(pairRemover);
				};
				typename TreeSet::InsertResult res =
					mTreeSet.template InsertCrt<decltype(itemCreator), false>(
					std::as_const(extPair.GetKey()), std::move(itemCreator));
				return { internal::ProxyConstructor<Iterator>(res.position), res.inserted };
			}
		}
		typename TreeSet::InsertResult res =
			mTreeSet.Insert(std::move(ExtractedPairProxy::GetSetExtractedItem(extPair)));
		return { internal::ProxyConstructor<Iterator>(res.position), res.inserted };
	}

	template<internal::conceptMapArgIterator<Key> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		if (begin == end)
			return 0;
		const TreeTraits& treeTraits = GetTreeTraits();
		MemManager& memManager = GetMemManager();
		size_t initCount = GetCount();
		ArgIterator iter = std::move(begin);
		auto pair0 = internal::MapArgReferencer<>::GetReferencePair(iter);
		typedef decltype(pair0.first) KeyArg;
		typedef decltype(pair0.second) ValueArg;
		Iterator pos = InsertVar(std::forward<KeyArg>(pair0.first),
			std::forward<ValueArg>(pair0.second)).position;
		for (++iter; iter != end; ++iter)
		{
			auto pair = internal::MapArgReferencer<>::GetReferencePair(iter);
			const Key& key = pair.first;
			if (treeTraits.IsLess(key, pos->key) || !pvIsGreater(std::next(pos), key))
			{
				pos = InsertVar(std::forward<KeyArg>(pair.first),
					std::forward<ValueArg>(pair.second)).position;
			}
			else if (TreeTraits::multiKey || treeTraits.IsLess(pos->key, key))
			{
				pos = pvAdd<false>(std::next(pos), std::forward<KeyArg>(pair.first),
					FastMovableFunctor(ValueCreator<ValueArg>(memManager,
						std::forward<ValueArg>(pair.second))));
			}
		}
		return GetCount() - initCount;
	}

	template<typename Pair = std::pair<Key, Value>>
	size_t Insert(std::initializer_list<Pair> pairs)
	{
		return Insert(pairs.begin(), pairs.end());
	}

	template<internal::conceptMapPairCreator<Key, Value> PairCreator,
		bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, PairCreator pairCreator)
	{
		return pvAdd<extraCheck>(iter, FastMovableFunctor(std::forward<PairCreator>(pairCreator)));
	}

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, Key&& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(iter, std::move(key),
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
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

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Iterator AddCrt(ConstIterator iter, const Key& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(iter, key,
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
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
		if constexpr (KeyValueTraits::useValuePtr)
		{
			if (ExtractedPairProxy::GetValueMemPool(extPair) != &mTreeSet.GetMemManager().GetMemPool())
			{
				auto pairCreator = [this, &extPair] (Key* newKey, Value* newValue)
				{
					auto pairRemover = [this, newKey, newValue] (Key& key, Value& value)
					{
						KeyValueTraits::Relocate(nullptr, &GetMemManager(),
							key, value, newKey, newValue);
					};
					extPair.Remove(pairRemover);
				};
				return pvAdd<true>(iter, FastMovableFunctor(std::move(pairCreator)));
			}
		}
		return internal::ProxyConstructor<Iterator>(mTreeSet.Add(ConstIteratorProxy::GetSetIterator(iter),
			std::move(ExtractedPairProxy::GetSetExtractedItem(extPair))));
	}

	template<typename ValueArg = Value>
	requires requires { typename ValueCreator<ValueArg>; }
	InsertResult InsertOrAssign(Key&& key, ValueArg&& valueArg)
		requires (!TreeTraits::multiKey)
	{
		return pvInsertOrAssign(std::move(key), std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg = Value>
	requires requires { typename ValueCreator<ValueArg>; }
	InsertResult InsertOrAssign(const Key& key, ValueArg&& valueArg)
		requires (!TreeTraits::multiKey)
	{
		return pvInsertOrAssign(key, std::forward<ValueArg>(valueArg));
	}

	ValueReference<Key&&> operator[](Key&& key)
		requires (!TreeTraits::multiKey)
	{
		Iterator iter = GetLowerBound(std::as_const(key));
		return !pvIsGreater(iter, std::as_const(key))
			? ValueReferencer::template GetReference<Key&&>(*this, iter)
			: ValueReferencer::template GetReference<Key&&>(*this, iter, std::move(key));
	}

	ValueReference<const Key&> operator[](const Key& key)
		requires (!TreeTraits::multiKey)
	{
		Iterator iter = GetLowerBound(key);
		return !pvIsGreater(iter, key)
			? ValueReferencer::template GetReference<const Key&>(*this, iter)
			: ValueReferencer::template GetReference<const Key&>(*this, iter, key);
	}

	Iterator Remove(ConstIterator iter)
	{
		return internal::ProxyConstructor<Iterator>(
			mTreeSet.Remove(ConstIteratorProxy::GetSetIterator(iter)));
	}

	Iterator Remove(Iterator iter)
	{
		return Remove(static_cast<ConstIterator>(iter));
	}

	Iterator Remove(ConstIterator iter, ExtractedPair& extPair)
	{
		Iterator resIter = internal::ProxyConstructor<Iterator>(mTreeSet.Remove(
			ConstIteratorProxy::GetSetIterator(iter), ExtractedPairProxy::GetSetExtractedItem(extPair)));
		if constexpr (KeyValueTraits::useValuePtr)
			ExtractedPairProxy::GetValueMemPool(extPair) = &mTreeSet.GetMemManager().GetMemPool();
		return resIter;
	}

	Iterator Remove(ConstIterator begin, ConstIterator end)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.Remove(
			ConstIteratorProxy::GetSetIterator(begin), ConstIteratorProxy::GetSetIterator(end)));
	}

	size_t Remove(const Key& key)
	{
		return mTreeSet.Remove(key);
	}

	template<internal::conceptMapPairPredicate<Key, Value> PairFilter>
	size_t Remove(PairFilter pairFilter)
	{
		auto itemFilter = [fastPairFilter = FastCopyableFunctor(pairFilter)] (const KeyValuePair& item)
			{ return fastPairFilter(item.GetKey(), std::as_const(item.GetValue())); };
		return mTreeSet.Remove(itemFilter);
	}

	ExtractedPair Extract(ConstIterator iter)
	{
		return ExtractedPair(*this, iter);
	}

	template<typename KeyArg,
		bool extraCheck = true>
	void ResetKey(ConstIterator iter, KeyArg&& keyArg)
	{
		mTreeSet.template ResetKey<KeyArg, extraCheck>(ConstIteratorProxy::GetSetIterator(iter),
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
		return internal::ProxyConstructor<Iterator>(ConstIteratorProxy::GetSetIterator(iter));
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		mTreeSet.CheckIterator(ConstIteratorProxy::GetSetIterator(iter), allowEmpty);
	}

private:
	bool pvIsGreater(ConstIterator iter, const Key& key) const
	{
		return iter == GetEnd() || GetTreeTraits().IsLess(key, iter->key);
	}

	template<typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult pvInsert(RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mTreeSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		typename TreeSet::InsertResult res = mTreeSet.template InsertCrt<decltype(itemCreator), false>(
			std::as_const(key), std::move(itemCreator));
		return { internal::ProxyConstructor<Iterator>(res.position), res.inserted };
	}

	template<bool extraCheck, internal::conceptMapPairCreator<Key, Value> PairCreator>
	Iterator pvAdd(ConstIterator iter, FastMovableFunctor<PairCreator> pairCreator)
	{
		auto itemCreator = [this, pairCreator = std::move(pairCreator)] (KeyValuePair* newItem) mutable
			{ std::construct_at(newItem, mTreeSet.GetMemManager(), std::move(pairCreator)); };
		return internal::ProxyConstructor<Iterator>(mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetSetIterator(iter), std::move(itemCreator)));
	}

	template<bool extraCheck, typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	Iterator pvAdd(ConstIterator iter, RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mTreeSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		return internal::ProxyConstructor<Iterator>(mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstIteratorProxy::GetSetIterator(iter), std::move(itemCreator)));
	}

	template<typename RKey, typename ValueArg>
	InsertResult pvInsertOrAssign(RKey&& key, ValueArg&& valueArg)
	{
		MemManager& memManager = GetMemManager();
		InsertResult res = pvInsert(std::forward<RKey>(key),
			FastMovableFunctor(ValueCreator<ValueArg>(memManager, std::forward<ValueArg>(valueArg))));
		if (!res.inserted)
			KeyValueTraits::AssignValue(memManager, std::forward<ValueArg>(valueArg), res.position->value);
		return res;
	}

private:
	TreeSet mTreeSet;
};

template<conceptObject TKey, conceptObject TValue,
	conceptTreeTraits<TKey> TTreeTraits = TreeTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault>
using TreeMap = TreeMapCore<TreeMapKeyValueTraits<TKey, TValue, TMemManager>, TTreeTraits>;

template<conceptObject TKey, conceptObject TValue>
using TreeMultiMap = TreeMap<TKey, TValue, TreeTraits<TKey, true>>;

} // namespace momo
