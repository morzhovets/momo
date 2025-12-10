/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashMap.h

  namespace momo:
    class HashMapKeyValueTraits
    class HashMapSettings
    class HashMapCore
    class HashMap
    class HashMapOpen

\**********************************************************/

#pragma once

#include "HashSet.h"
#include "MapUtility.h"

namespace momo
{

namespace internal
{
	template<typename THashSetBucketBounds,
		bool tIsConst = false>
	class HashMapBucketBounds : public Rangeable
	{
	protected:
		typedef THashSetBucketBounds HashSetBucketBounds;

		static const bool isConst = tIsConst;

	public:
		typedef MapForwardIterator<typename HashSetBucketBounds::Iterator, isConst> Iterator;

		typedef HashMapBucketBounds<HashSetBucketBounds, true> ConstBounds;

	private:
		struct IteratorProxy : public Iterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
		};

		struct ConstBoundsProxy : public ConstBounds
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstBounds)
		};

	public:
		explicit HashMapBucketBounds() noexcept
		{
		}

		operator ConstBounds() const noexcept
		{
			return ConstBoundsProxy(mHashSetBucketBounds);
		}

		Iterator GetBegin() const noexcept
		{
			return IteratorProxy(mHashSetBucketBounds.GetBegin());
		}

		Iterator GetEnd() const noexcept
		{
			return IteratorProxy(mHashSetBucketBounds.GetEnd());
		}

		size_t GetCount() const noexcept
		{
			return mHashSetBucketBounds.GetCount();
		}

	protected:
		explicit HashMapBucketBounds(HashSetBucketBounds hashSetBounds) noexcept
			: mHashSetBucketBounds(hashSetBounds)
		{
		}

	private:
		HashSetBucketBounds mHashSetBucketBounds;
	};

	template<typename TKeyValueTraits>
	class HashMapNestedSetItemTraits : public MapNestedSetItemTraits<TKeyValueTraits>
	{
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
		static const bool allowExceptionSuppression = HashMapSettings::allowExceptionSuppression;
	};
}

template<conceptObject TKey, conceptObject TValue,
	conceptMemManager TMemManager = MemManagerDefault,
	bool tUseValuePtr = false>
class HashMapKeyValueTraits
	: public internal::MapKeyValueTraits<TKey, TValue, TMemManager, tUseValuePtr>
{
};

class HashMapSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
	static const bool allowExceptionSuppression = true;
};

/*!
	All `HashMapCore` functions and constructors have strong exception safety,
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
	conceptHashTraits<typename TKeyValueTraits::Key> THashTraits
		= HashTraits<typename TKeyValueTraits::Key>,
	typename TSettings = HashMapSettings>
requires conceptMapKeyValueTraits<TKeyValueTraits, typename TKeyValueTraits::Key,
	typename TKeyValueTraits::Value, typename TKeyValueTraits::MemManager>
class MOMO_EMPTY_BASES HashMapCore
	: public internal::Rangeable,
	public internal::Swappable<HashMapCore>
{
public:
	typedef TKeyValueTraits KeyValueTraits;
	typedef THashTraits HashTraits;
	typedef TSettings Settings;
	typedef typename KeyValueTraits::Key Key;
	typedef typename KeyValueTraits::Value Value;
	typedef typename KeyValueTraits::MemManager MemManager;

private:
	typedef internal::HashMapNestedSetItemTraits<KeyValueTraits> HashSetItemTraits;
	typedef typename HashSetItemTraits::Item KeyValuePair;

	typedef internal::HashMapNestedSetSettings<Settings> HashSetSettings;

	typedef HashSetCore<HashSetItemTraits, HashTraits, HashSetSettings> HashSet;

	typedef typename HashSet::ConstIterator HashSetConstIterator;
	typedef typename HashSet::ConstPosition HashSetConstPosition;

	typedef typename HashSet::ExtractedItem HashSetExtractedItem;

public:
	typedef internal::MapForwardIterator<HashSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::MapPosition<HashSetConstPosition> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

	typedef internal::MapExtractedPair<HashSetExtractedItem,
		KeyValueTraits::useValuePtr> ExtractedPair;

	typedef internal::HashMapBucketBounds<typename HashSet::ConstBucketBounds> BucketBounds;
	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

	static const size_t bucketMaxItemCount = HashSet::bucketMaxItemCount;

private:
	typedef internal::MapValueReferencer<HashMapCore, Position> ValueReferencer;

public:
	template<typename KeyReference>
	using ValueReference = ValueReferencer::template ValueReference<KeyReference>;

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
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetSetIterator)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct ConstPositionProxy : public ConstPosition
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstPosition)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetSetPosition)
	};

	struct PositionProxy : public Position
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Position)
	};

	struct ExtractedPairProxy : private ExtractedPair
	{
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetSetExtractedItem)
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetValueMemPool)
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
	HashMapCore()
		: HashMapCore(HashTraits())
	{
	}

	explicit HashMapCore(const HashTraits& hashTraits, MemManager memManager = MemManager())
		: mHashSet(hashTraits, std::move(memManager))
	{
	}

	template<internal::conceptMapArgIterator<Key> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit HashMapCore(ArgIterator begin, ArgSentinel end,
		const HashTraits& hashTraits = HashTraits(), MemManager memManager = MemManager())
		: HashMapCore(hashTraits, std::move(memManager))
	{
		Insert(std::move(begin), std::move(end));
	}

	template<typename Pair = std::pair<Key, Value>>
	HashMapCore(std::initializer_list<Pair> pairs)
		: HashMapCore(pairs, HashTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit HashMapCore(std::initializer_list<Pair> pairs, const HashTraits& hashTraits,
		MemManager memManager = MemManager())
		: HashMapCore(pairs.begin(), pairs.end(), hashTraits, std::move(memManager))
	{
	}

	HashMapCore(HashMapCore&& hashMap) noexcept
		: mHashSet(std::move(hashMap.mHashSet))
	{
	}

	HashMapCore(const HashMapCore& hashMap)
		: mHashSet(hashMap.mHashSet)
	{
	}

	explicit HashMapCore(const HashMapCore& hashMap, MemManager memManager)
		: mHashSet(hashMap.mHashSet, std::move(memManager))
	{
	}

	~HashMapCore() noexcept = default;

	HashMapCore& operator=(HashMapCore&& hashMap) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(hashMap), *this);
	}

	HashMapCore& operator=(const HashMapCore& hashMap)
	{
		return internal::ContainerAssigner::Copy(hashMap, *this);
	}

	void Swap(HashMapCore& hashMap) noexcept
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

	//void Shrink()
	//{
	//	mHashSet.Shrink();
	//}

	MOMO_FORCEINLINE ConstPosition Find(const Key& key) const
	{
		return ConstPositionProxy(mHashSet.Find(key));
	}

	MOMO_FORCEINLINE Position Find(const Key& key)
	{
		return PositionProxy(mHashSet.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE ConstPosition Find(const KeyArg& key) const
	{
		return ConstPositionProxy(mHashSet.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE Position Find(const KeyArg& key)
	{
		return PositionProxy(mHashSet.Find(key));
	}

	MOMO_FORCEINLINE bool ContainsKey(const Key& key) const
	{
		return mHashSet.ContainsKey(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE bool ContainsKey(const KeyArg& key) const
	{
		return mHashSet.ContainsKey(key);
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
			if (ExtractedPairProxy::GetValueMemPool(extPair) != &mHashSet.GetMemManager().GetMemPool())
			{
				auto itemCreator = [this, &extPair] (KeyValuePair* newItem)
				{
					auto pairRemover = [this, newItem] (Key& key, Value& value)
					{
						KeyValuePair::template CreateRelocate<KeyValueTraits>(
							newItem, nullptr, mHashSet.GetMemManager(), key, value);
					};
					extPair.Remove(pairRemover);
				};
				typename HashSet::InsertResult res =
					mHashSet.template InsertCrt<decltype(itemCreator), false>(
					std::as_const(extPair.GetKey()), std::move(itemCreator));
				return { PositionProxy(res.position), res.inserted };
			}
		}
		typename HashSet::InsertResult res =
			mHashSet.Insert(std::move(ExtractedPairProxy::GetSetExtractedItem(extPair)));
		return { PositionProxy(res.position), res.inserted };
	}

	template<internal::conceptMapArgIterator<Key> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		size_t initCount = GetCount();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
		{
			auto pair = internal::MapArgReferencer<>::GetReferencePair(iter);
			InsertVar(std::forward<decltype(pair.first)>(pair.first),
				std::forward<decltype(pair.second)>(pair.second));
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
	Position AddCrt(ConstPosition pos, PairCreator pairCreator)
	{
		return pvAdd<extraCheck>(pos, FastMovableFunctor(std::forward<PairCreator>(pairCreator)));
	}

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, Key&& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, std::move(key),
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	Position AddVar(ConstPosition pos, Key&& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(pos, std::move(key),
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Position Add(ConstPosition pos, Key&& key, Value&& value)
	{
		return AddVar(pos, std::move(key), std::move(value));
	}

	Position Add(ConstPosition pos, Key&& key, const Value& value)
	{
		return AddVar(pos, std::move(key), value);
	}

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, const Key& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, key,
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	Position AddVar(ConstPosition pos, const Key& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(pos, key,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Position Add(ConstPosition pos, const Key& key, Value&& value)
	{
		return AddVar(pos, key, std::move(value));
	}

	Position Add(ConstPosition pos, const Key& key, const Value& value)
	{
		return AddVar(pos, key, value);
	}

	Position Add(ConstPosition pos, ExtractedPair&& extPair)
	{
		if constexpr (KeyValueTraits::useValuePtr)
		{
			if (ExtractedPairProxy::GetValueMemPool(extPair) != &mHashSet.GetMemManager().GetMemPool())
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
				return pvAdd<true>(pos, FastMovableFunctor(std::move(pairCreator)));
			}
		}
		return PositionProxy(mHashSet.Add(ConstPositionProxy::GetSetPosition(pos),
			std::move(ExtractedPairProxy::GetSetExtractedItem(extPair))));
	}

	template<typename ValueArg>
	requires requires { typename ValueCreator<ValueArg>; }
	InsertResult InsertOrAssign(Key&& key, ValueArg&& valueArg)
	{
		return pvInsertOrAssign(std::move(key), std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg>
	requires requires { typename ValueCreator<ValueArg>; }
	InsertResult InsertOrAssign(const Key& key, ValueArg&& valueArg)
	{
		return pvInsertOrAssign(key, std::forward<ValueArg>(valueArg));
	}

	MOMO_FORCEINLINE ValueReference<Key&&> operator[](Key&& key)
	{
		Position pos = Find(std::as_const(key));
		return !!pos ? ValueReferencer::template GetReference<Key&&>(*this, pos)
			: ValueReferencer::template GetReference<Key&&>(*this, pos, std::move(key));
	}

	MOMO_FORCEINLINE ValueReference<const Key&> operator[](const Key& key)
	{
		Position pos = Find(key);
		return !!pos ? ValueReferencer::template GetReference<const Key&>(*this, pos)
			: ValueReferencer::template GetReference<const Key&>(*this, pos, key);
	}

	Iterator Remove(ConstIterator iter)
	{
		return IteratorProxy(mHashSet.Remove(ConstIteratorProxy::GetSetIterator(iter)));
	}

	Iterator Remove(Iterator iter)
	{
		return Remove(static_cast<ConstIterator>(iter));
	}

	void Remove(ConstPosition pos)
	{
		Remove(static_cast<ConstIterator>(pos));
	}

	void Remove(Position pos)
	{
		Remove(static_cast<ConstIterator>(pos));
	}

	Iterator Remove(ConstIterator iter, ExtractedPair& extPair)
	{
		Iterator resIter = IteratorProxy(mHashSet.Remove(ConstIteratorProxy::GetSetIterator(iter),
			ExtractedPairProxy::GetSetExtractedItem(extPair)));
		if constexpr (KeyValueTraits::useValuePtr)
			ExtractedPairProxy::GetValueMemPool(extPair) = &mHashSet.GetMemManager().GetMemPool();
		return resIter;
	}

	bool Remove(const Key& key)
	{
		return mHashSet.Remove(key);
	}

	template<internal::conceptMapPairPredicate<Key, Value> PairFilter>
	size_t Remove(PairFilter pairFilter)
	{
		auto itemFilter = [fastPairFilter = FastCopyableFunctor(pairFilter)] (const KeyValuePair& item)
			{ return fastPairFilter(item.GetKey(), std::as_const(item.GetValue())); };
		return mHashSet.Remove(itemFilter);
	}

	ExtractedPair Extract(ConstPosition pos)
	{
		return ExtractedPair(*this, static_cast<ConstIterator>(pos));
	}

	template<typename KeyArg,
		bool extraCheck = true>
	void ResetKey(ConstPosition pos, KeyArg&& keyArg)
	{
		mHashSet.template ResetKey<KeyArg, extraCheck>(ConstPositionProxy::GetSetPosition(pos),
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

	ConstPosition MakePosition(size_t hashCode) const noexcept
	{
		return ConstPositionProxy(mHashSet.MakePosition(hashCode));
	}

	Iterator MakeMutableIterator(ConstIterator iter)
	{
		CheckIterator(iter);
		return IteratorProxy(ConstIteratorProxy::GetSetIterator(iter));
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		mHashSet.CheckIterator(ConstIteratorProxy::GetSetIterator(iter), allowEmpty);
	}

private:
	template<typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult pvInsert(RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mHashSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		typename HashSet::InsertResult res = mHashSet.template InsertCrt<decltype(itemCreator), false>(
			std::as_const(key), std::move(itemCreator));
		return { PositionProxy(res.position), res.inserted };
	}

	template<bool extraCheck, internal::conceptMapPairCreator<Key, Value> PairCreator>
	Position pvAdd(ConstPosition pos, FastMovableFunctor<PairCreator> pairCreator)
	{
		auto itemCreator = [this, pairCreator = std::move(pairCreator)] (KeyValuePair* newItem) mutable
			{ std::construct_at(newItem, mHashSet.GetMemManager(), std::move(pairCreator)); };
		return PositionProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetSetPosition(pos), std::move(itemCreator)));
	}

	template<bool extraCheck, typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	Position pvAdd(ConstPosition pos, RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mHashSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		return PositionProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetSetPosition(pos), std::move(itemCreator)));
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
	HashSet mHashSet;
};

template<conceptObject TKey, conceptObject TValue,
	conceptHashTraits<TKey> THashTraits = HashTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault>
using HashMap = HashMapCore<HashMapKeyValueTraits<TKey, TValue, TMemManager>, THashTraits>;

template<conceptObject TKey, conceptObject TValue>
using HashMapOpen = HashMap<TKey, TValue, HashTraitsOpen<TKey>>;

namespace internal
{
	template<bool tAllowExceptionSuppression>
	class NestedHashMapSettings : public HashMapSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = false;
		static const bool allowExceptionSuppression = tAllowExceptionSuppression;
	};
}

} // namespace momo
