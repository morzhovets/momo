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

\**********************************************************/

#pragma once

#include "HashSet.h"
#include "MapUtility.h"

namespace momo
{

namespace internal
{
	template<typename THashSetPosition,
		bool tIsConst = false>
	class HashMapPosition
	{
	protected:
		typedef THashSetPosition HashSetPosition;

		static const bool isConst = tIsConst;

	public:
		typedef MapForwardIterator<typename HashSetPosition::Iterator, isConst> Iterator;

		typedef typename Iterator::Reference Reference;
		typedef typename Iterator::Pointer Pointer;

		typedef HashMapPosition<HashSetPosition, true> ConstPosition;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstPositionProxy : public ConstPosition
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstPosition)
		};

		struct IteratorProxy : public Iterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
			MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetSetIterator)
		};

	public:
		explicit HashMapPosition() noexcept
			: mHashSetPosition()
		{
		}

		template<typename ArgIterator>
		requires std::is_convertible_v<ArgIterator, Iterator>
		HashMapPosition(ArgIterator iter) noexcept
			: mHashSetPosition(IteratorProxy::GetSetIterator(static_cast<Iterator>(iter)))
		{
		}

		operator ConstPosition() const noexcept
		{
			return ConstPositionProxy(mHashSetPosition);
		}

		template<typename ResIterator>
		requires std::is_convertible_v<Iterator, ResIterator>
		operator ResIterator() const noexcept
		{
			Iterator iter = IteratorProxy(mHashSetPosition);
			return static_cast<ResIterator>(iter);
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mHashSetPosition));
		}

		friend bool operator==(HashMapPosition pos1, HashMapPosition pos2) noexcept
		{
			return pos1.mHashSetPosition == pos2.mHashSetPosition;
		}

		friend bool operator==(HashMapPosition pos1, Iterator iter2) noexcept
		{
			return pos1 == static_cast<HashMapPosition>(iter2);
		}

		friend bool operator==(Iterator iter1, HashMapPosition pos2) noexcept
		{
			return pos2 == iter1;
		}

		MOMO_MORE_POSITION_OPERATORS(HashMapPosition)

	protected:
		explicit HashMapPosition(HashSetPosition hashSetPos) noexcept
			: mHashSetPosition(hashSetPos)
		{
		}

		HashSetPosition ptGetHashSetPosition() const noexcept
		{
			return mHashSetPosition;
		}

	private:
		HashSetPosition mHashSetPosition;
	};

	template<typename THashSetBucketBounds,
		bool tIsConst = false>
	class HashMapBucketBounds
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

		MOMO_FRIENDS_SIZE_BEGIN_END(HashMapBucketBounds)

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

		static const bool overloadIfCannotGrow = HashMapSettings::overloadIfCannotGrow;
	};
}

template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager,
	bool tUseValuePtr = false>
class HashMapKeyValueTraits : public internal::MapKeyValueTraits<TKey, TValue, TMemManager, tUseValuePtr>
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

/*!
	All `HashMap` functions and constructors have strong exception safety,
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

template<conceptObject TKey, conceptObject TValue,
	conceptHashTraits<TKey> THashTraits = HashTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault,
	conceptMapKeyValueTraits<TKey, TValue, TMemManager> TKeyValueTraits
		= HashMapKeyValueTraits<TKey, TValue, TMemManager>,
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
	typedef internal::HashMapNestedSetItemTraits<KeyValueTraits> HashSetItemTraits;
	typedef typename HashSetItemTraits::Item KeyValuePair;

	typedef internal::HashMapNestedSetSettings<Settings> HashSetSettings;

	typedef momo::HashSet<Key, HashTraits, typename HashSetItemTraits::MemManager,
		HashSetItemTraits, HashSetSettings> HashSet;

	typedef typename HashSet::ConstIterator HashSetConstIterator;
	typedef typename HashSet::ConstPosition HashSetConstPosition;

	typedef typename HashSet::ExtractedItem HashSetExtractedItem;

public:
	typedef internal::MapForwardIterator<HashSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::HashMapPosition<HashSetConstPosition> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

	typedef internal::MapExtractedPair<HashSetExtractedItem,
		KeyValueTraits::useValuePtr> ExtractedPair;

	typedef internal::HashMapBucketBounds<typename HashSet::ConstBucketBounds> BucketBounds;
	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

	static const size_t bucketMaxItemCount = HashSet::bucketMaxItemCount;

private:
	typedef internal::MapValueReferencer<HashMap, Position> ValueReferencer;

public:
	typedef typename ValueReferencer::template ValueReference<Key&&> ValueReferenceRKey;
	typedef typename ValueReferencer::template ValueReference<const Key&> ValueReferenceCKey;

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
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetHashSetPosition)
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
	HashMap()
		: HashMap(HashTraits())
	{
	}

	explicit HashMap(const HashTraits& hashTraits, MemManager memManager = MemManager())
		: mHashSet(hashTraits, std::move(memManager))
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	HashMap(std::initializer_list<Pair> pairs)
		: HashMap(pairs, HashTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit HashMap(std::initializer_list<Pair> pairs, const HashTraits& hashTraits,
		MemManager memManager = MemManager())
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

	explicit HashMap(const HashMap& hashMap, MemManager memManager)
		: mHashSet(hashMap.mHashSet, std::move(memManager))
	{
	}

	~HashMap() noexcept = default;

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

	MOMO_FRIEND_SWAP(HashMap)
	MOMO_FRIENDS_SIZE_BEGIN_END(HashMap)

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

	template<std::invocable<Value*> ValueCreator>
	InsertResult InsertCrt(Key&& key, ValueCreator valueCreator)
	{
		return pvInsert(std::move(key), internal::FastMovableCreator(std::move(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
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

	template<std::invocable<Value*> ValueCreator>
	InsertResult InsertCrt(const Key& key, ValueCreator valueCreator)
	{
		return pvInsert(key, internal::FastMovableCreator(std::move(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
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
		if constexpr (KeyValueTraits::useValuePtr)
		{
			if (ExtractedPairProxy::GetValueMemPool(extPair) != &mHashSet.GetMemManager().GetMemPool())
			{
				auto itemCreator = [this, &extPair] (KeyValuePair* newItem)
				{
					auto pairRemover = [this, newItem] (Key& key, Value& value)
					{
						newItem->template Relocate<KeyValueTraits>(mHashSet.GetMemManager(),
							key, value);
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

	template<internal::conceptMapArgIterator<Key> ArgIterator>
	size_t Insert(ArgIterator begin, ArgIterator end)
	{
		size_t count = 0;
		for (ArgIterator iter = begin; iter != end; ++iter)
		{
			auto pair = internal::MapArgReferencer<>::GetReferencePair(iter);
			InsertResult res = InsertVar(std::forward<decltype(pair.first)>(pair.first),
				std::forward<decltype(pair.second)>(pair.second));
			count += res.inserted ? 1 : 0;
		}
		return count;
	}

	template<typename Pair = std::pair<Key, Value>>
	size_t Insert(std::initializer_list<Pair> pairs)
	{
		return Insert(pairs.begin(), pairs.end());
	}

	template<std::invocable<Key*, Value*> PairCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, PairCreator pairCreator)
	{
		return pvAdd<extraCheck>(pos, internal::FastMovableCreator(std::move(pairCreator)));
	}

	template<std::invocable<Value*> ValueCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, Key&& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, std::move(key),
			internal::FastMovableCreator(std::move(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	Position AddVar(ConstPosition pos, Key&& key, ValueArgs&&... valueArgs)
	{
		return pvAdd<true>(pos, std::move(key),
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

	template<std::invocable<Value*> ValueCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, const Key& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, key, internal::FastMovableCreator(std::move(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	Position AddVar(ConstPosition pos, const Key& key, ValueArgs&&... valueArgs)
	{
		return pvAdd<true>(pos, key,
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
						{ KeyValueTraits::Relocate(&GetMemManager(), key, value, newKey, newValue); };
					extPair.Remove(pairRemover);
				};
				return pvAdd<true>(pos, std::move(pairCreator));
			}
		}
		return PositionProxy(mHashSet.Add(ConstPositionProxy::GetHashSetPosition(pos),
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

	MOMO_FORCEINLINE ValueReferenceRKey operator[](Key&& key)
	{
		Position pos = Find(std::as_const(key));
		return !!pos ? ValueReferencer::template GetReference<Key&&>(*this, pos)
			: ValueReferencer::template GetReference<Key&&>(*this, pos, std::move(key));
	}

	MOMO_FORCEINLINE ValueReferenceCKey operator[](const Key& key)
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

	template<typename PairPredicate>
	requires std::predicate<const PairPredicate&, const Key&, const Value&>
	size_t Remove(const PairPredicate& pairPred)
	{
		auto itemPred = [&pairPred] (const KeyValuePair& item)
			{ return pairPred(*item.GetKeyPtr(), std::as_const(*item.GetValuePtr())); };
		return mHashSet.Remove(itemPred);
	}

	ExtractedPair Extract(ConstPosition pos)
	{
		return ExtractedPair(*this, static_cast<ConstIterator>(pos));
	}

	template<typename KeyArg,
		bool extraCheck = true>
	void ResetKey(ConstPosition pos, KeyArg&& keyArg)
	{
		mHashSet.template ResetKey<KeyArg, extraCheck>(ConstPositionProxy::GetHashSetPosition(pos),
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
	template<typename RKey, typename ValueCreator>
	InsertResult pvInsert(RKey&& key, ValueCreator valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			newItem->template Create<KeyValueTraits>(mHashSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		typename HashSet::InsertResult res = mHashSet.template InsertCrt<decltype(itemCreator), false>(
			std::as_const(key), std::move(itemCreator));
		return { PositionProxy(res.position), res.inserted };
	}

	template<bool extraCheck, typename PairCreator>
	Position pvAdd(ConstPosition pos, PairCreator pairCreator)
	{
		auto itemCreator = [this, pairCreator = std::move(pairCreator)] (KeyValuePair* newItem) mutable
			{ newItem->Create(mHashSet.GetMemManager(), std::move(pairCreator)); };
		return PositionProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetHashSetPosition(pos), std::move(itemCreator)));
	}

	template<bool extraCheck, typename RKey, typename ValueCreator>
	Position pvAdd(ConstPosition pos, RKey&& key, ValueCreator valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			newItem->template Create<KeyValueTraits>(mHashSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		return PositionProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetHashSetPosition(pos), std::move(itemCreator)));
	}

	template<typename RKey, typename ValueArg>
	InsertResult pvInsertOrAssign(RKey&& key, ValueArg&& valueArg)
	{
		MemManager& memManager = GetMemManager();
		InsertResult res = pvInsert(std::forward<RKey>(key),
			ValueCreator<ValueArg>(memManager, std::forward<ValueArg>(valueArg)));
		if (!res.inserted)
			KeyValueTraits::AssignValue(memManager, std::forward<ValueArg>(valueArg), res.position->value);
		return res;
	}

private:
	HashSet mHashSet;
};

template<conceptObject TKey, conceptObject TValue>
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
