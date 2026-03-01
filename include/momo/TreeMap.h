/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/TreeMap.h

  namespace momo:
    class TreeMapKeyValueTraits
    class TreeMapSettings
    class TreeMapCore
    class TreeMap
    class TreeMultiMap

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_TREE_MAP
#define MOMO_INCLUDE_GUARD_TREE_MAP

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

	public:
		explicit TreeMapIterator() noexcept
			: mTreeSetIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ProxyConstructor<ConstIterator>(mTreeSetIterator);
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
			return Pointer(ProxyConstructor<Reference>(*mTreeSetIterator));
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
		using typename MapNestedSetItemTraits::Value;

	public:
		using typename MapNestedSetItemTraits::Key;
		using typename MapNestedSetItemTraits::Item;
		using typename MapNestedSetItemTraits::MemManager;

		static const bool isNothrowShiftable = KeyValueTraits::isKeyNothrowShiftable
			&& KeyValueTraits::isValueNothrowShiftable;

	public:
		template<typename Iterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, (void)++dstIter)
				Item::Create(dstIter.operator->());	//?
			auto fin = Catcher::Finalize(&TreeMapNestedSetItemTraits::template pvDestroy<Iterator>,
				dstBegin, count);
			KeyValueTraits::RelocateExec(memManager,
				MapKeyIterator<Iterator>(srcBegin), MapValueIterator<Iterator>(srcBegin),
				MapKeyIterator<Iterator>(dstBegin), MapValueIterator<Iterator>(dstBegin), count,
				ObjectCreateExecutor<Item, ItemCreator>(std::forward<ItemCreator>(itemCreator), newItem));
			fin.Detach();
			pvDestroy(srcBegin, count);
		}

		template<typename Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			KeyValueTraits::ShiftKeyNothrow(memManager, MapKeyIterator<Iterator>(begin), shift);
			KeyValueTraits::ShiftValueNothrow(memManager, MapValueIterator<Iterator>(begin), shift);
		}

	private:
		template<typename Iterator>
		static void pvDestroy(Iterator begin, size_t count) noexcept
		{
			Iterator iter = begin;
			for (size_t i = 0; i < count; ++i, (void)++iter)
				Item::Destroy(*iter);
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

template<typename TKey, typename TValue,
	typename TMemManager = MemManagerDefault>
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
	typename TTreeTraits = TreeTraits<typename TKeyValueTraits::Key>,
	typename TSettings = TreeMapSettings>
class TreeMapCore
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
	typedef internal::TreeMapIterator<TreeSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef internal::MapExtractedPair<TreeSetExtractedItem> ExtractedPair;

private:
	typedef internal::MapValueReferencer<TreeMapCore> ValueReferencer;

public:
	template<typename KeyReference>
	using ValueReference = typename ValueReferencer::template ValueReference<KeyReference>;

private:
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	template<typename KeyArg>
	struct IsValidKeyArg : public TreeTraits::template IsValidKeyArg<KeyArg>
	{
	};

	struct ConstIteratorProxy : private ConstIterator
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetTreeSetIterator)
	};

	struct ExtractedPairProxy : private ExtractedPair
	{
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetSetExtractedItem)
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

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(internal::MapPairConverter<ArgIterator>::Convert(*std::declval<ArgIterator>()))>
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

	~TreeMapCore() = default;

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

	MOMO_FRIEND_SWAP(TreeMapCore)
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(TreeMapCore, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(TreeMapCore, Iterator)

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
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstIterator> GetLowerBound(const KeyArg& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetLowerBound(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	Iterator> GetLowerBound(const KeyArg& key)
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
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstIterator> GetUpperBound(const KeyArg& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.GetUpperBound(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	Iterator> GetUpperBound(const KeyArg& key)
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
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstIterator> Find(const KeyArg& key) const
	{
		return internal::ProxyConstructor<ConstIterator>(mTreeSet.Find(key));
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	Iterator> Find(const KeyArg& key)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.Find(key));
	}

	bool ContainsKey(const Key& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	bool> ContainsKey(const KeyArg& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	size_t GetKeyCount(const Key& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	size_t> GetKeyCount(const KeyArg& key) const
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
		return { internal::ProxyConstructor<Iterator>(res.position), res.inserted };
	}

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(internal::MapPairConverter<ArgIterator>::Convert(*std::declval<ArgIterator>()))>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		if (begin == end)
			return 0;
		const TreeTraits& treeTraits = GetTreeTraits();
		MemManager& memManager = GetMemManager();
		size_t initCount = GetCount();
		ArgIterator iter = std::move(begin);
		auto pair0 = internal::MapPairConverter<ArgIterator>::Convert(*iter);
		typedef decltype(pair0.first) KeyArg;
		typedef decltype(pair0.second) ValueArg;
		MOMO_STATIC_ASSERT(std::is_same<Key, typename std::decay<KeyArg>::type>::value);
		Iterator pos = InsertVar(std::forward<KeyArg>(pair0.first),
			std::forward<ValueArg>(pair0.second)).position;
		for (++iter; iter != end; ++iter)
		{
			auto pair = internal::MapPairConverter<ArgIterator>::Convert(*iter);
			const Key& key = pair.first;
			if (treeTraits.IsLess(key, pos->key) || !pvIsGreater(std::next(pos), key))
			{
				pos = InsertVar(std::forward<KeyArg>(pair.first),
					std::forward<ValueArg>(pair.second)).position;
			}
			else if (TreeTraits::multiKey || treeTraits.IsLess(pos->key, key))
			{
				pos = pvAdd<false>(std::next(pos), std::forward<KeyArg>(pair.first),
					ValueCreator<ValueArg>(memManager, std::forward<ValueArg>(pair.second)));
			}
		}
		return GetCount() - initCount;
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
			{ KeyValuePair::Create(newItem, std::forward<PairCreator>(pairCreator)); };
		return internal::ProxyConstructor<Iterator>(
			mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
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
		return internal::ProxyConstructor<Iterator>(mTreeSet.Add(
			ConstIteratorProxy::GetTreeSetIterator(iter),
			std::move(ExtractedPairProxy::GetSetExtractedItem(extPair))));
	}

	ValueReference<Key&&> operator[](Key&& key)
	{
		MOMO_STATIC_ASSERT(!TreeTraits::multiKey);
		Iterator iter = GetLowerBound(static_cast<const Key&>(key));
		return !pvIsGreater(iter, static_cast<const Key&>(key))
			? ValueReferencer::template GetReference<Key&&>(*this, iter)
			: ValueReferencer::template GetReference<Key&&>(*this, iter, std::move(key));
	}

	ValueReference<const Key&> operator[](const Key& key)
	{
		MOMO_STATIC_ASSERT(!TreeTraits::multiKey);
		Iterator iter = GetLowerBound(key);
		return !pvIsGreater(iter, key)
			? ValueReferencer::template GetReference<const Key&>(*this, iter)
			: ValueReferencer::template GetReference<const Key&>(*this, iter, key);
	}

	Iterator Remove(ConstIterator iter)
	{
		return internal::ProxyConstructor<Iterator>(
			mTreeSet.Remove(ConstIteratorProxy::GetTreeSetIterator(iter)));
	}

	Iterator Remove(Iterator iter)
	{
		return Remove(static_cast<ConstIterator>(iter));
	}

	Iterator Remove(ConstIterator iter, ExtractedPair& extPair)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.Remove(
			ConstIteratorProxy::GetTreeSetIterator(iter),
			ExtractedPairProxy::GetSetExtractedItem(extPair)));
	}

	Iterator Remove(ConstIterator begin, ConstIterator end)
	{
		return internal::ProxyConstructor<Iterator>(mTreeSet.Remove(
			ConstIteratorProxy::GetTreeSetIterator(begin),
			ConstIteratorProxy::GetTreeSetIterator(end)));
	}

	size_t Remove(const Key& key)
	{
		return mTreeSet.Remove(key);
	}

	template<typename PairFilter>
	internal::EnableIf<internal::IsInvocable<const PairFilter&, bool, const Key&, const Value&>::value,
	size_t> Remove(const PairFilter& pairFilter)
	{
		auto itemFilter = [&pairFilter] (const KeyValuePair& item)
			{ return pairFilter(item.GetKey(), static_cast<const Value&>(item.GetValue())); };
		return mTreeSet.Remove(itemFilter);
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
		return internal::ProxyConstructor<Iterator>(ConstIteratorProxy::GetTreeSetIterator(iter));
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
			KeyValuePair::template Create<KeyValueTraits>(newItem, GetMemManager(),
				std::forward<RKey>(key), std::forward<ValueCreator>(valueCreator));
		};
		typename TreeSet::InsertResult res = mTreeSet.template InsertCrt<decltype(itemCreator), false>(
			static_cast<const Key&>(key), std::move(itemCreator));
		return { internal::ProxyConstructor<Iterator>(res.position), res.inserted };
	}

	template<bool extraCheck, typename RKey, typename ValueCreator>
	Iterator pvAdd(ConstIterator iter, RKey&& key, ValueCreator&& valueCreator)
	{
		auto itemCreator = [this, &key, &valueCreator] (KeyValuePair* newItem)
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, GetMemManager(),
				std::forward<RKey>(key), std::forward<ValueCreator>(valueCreator));
		};
		return internal::ProxyConstructor<Iterator>(
			mTreeSet.template AddCrt<decltype(itemCreator), extraCheck>(
				ConstIteratorProxy::GetTreeSetIterator(iter), std::move(itemCreator)));
	}

private:
	TreeSet mTreeSet;
};

template<typename TKey, typename TValue,
	typename TTreeTraits = TreeTraits<TKey>,
	typename TMemManager = MemManagerDefault>
using TreeMap = TreeMapCore<TreeMapKeyValueTraits<TKey, TValue, TMemManager>, TTreeTraits>;

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

#endif // MOMO_INCLUDE_GUARD_TREE_MAP
