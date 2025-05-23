/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/HashMap.h

  namespace momo:
    class HashMapKeyValueTraits
    class HashMapSettings
    class HashMap
    class HashMapOpen

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_HASH_MAP
#define MOMO_INCLUDE_GUARD_HASH_MAP

#include "HashSet.h"
#include "MapUtility.h"

namespace momo
{

namespace internal
{
	template<typename THashSetIterator,
		bool tIsConst = false>
	class HashMapIterator
	{
	protected:
		typedef THashSetIterator HashSetIterator;

		static const bool isConst = tIsConst;

	public:
		typedef MapReference<typename std::iterator_traits<HashSetIterator>::reference,
			isConst> Reference;
		typedef IteratorPointer<Reference> Pointer;

		typedef HashMapIterator<HashSetIterator, true> ConstIterator;

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
		explicit HashMapIterator() noexcept
			: mHashSetIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mHashSetIterator);
		}

		HashMapIterator& operator++()
		{
			++mHashSetIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mHashSetIterator));
		}

		friend bool operator==(HashMapIterator iter1, HashMapIterator iter2) noexcept
		{
			return iter1.mHashSetIterator == iter2.mHashSetIterator;
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashMapIterator)

	protected:
		explicit HashMapIterator(HashSetIterator hashSetIter) noexcept
			: mHashSetIterator(hashSetIter)
		{
		}

		HashSetIterator ptGetHashSetIterator() const noexcept
		{
			return mHashSetIterator;
		}

	private:
		HashSetIterator mHashSetIterator;
	};

	template<typename THashSetPosition,
		bool tIsConst = false>
	class HashMapPosition
	{
	protected:
		typedef THashSetPosition HashSetPosition;

		static const bool isConst = tIsConst;

	public:
		typedef HashMapIterator<typename HashSetPosition::Iterator, isConst> Iterator;

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
			MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetHashSetIterator)
		};

	public:
		explicit HashMapPosition() noexcept
			: mHashSetPosition()
		{
		}

		template<typename ArgIterator,
			typename = EnableIf<std::is_convertible<ArgIterator, Iterator>::value>>
		HashMapPosition(ArgIterator iter) noexcept
			: mHashSetPosition(IteratorProxy::GetHashSetIterator(static_cast<Iterator>(iter)))
		{
		}

		operator ConstPosition() const noexcept
		{
			return ConstPositionProxy(mHashSetPosition);
		}

		template<typename ResIterator,
			typename = EnableIf<std::is_convertible<Iterator, ResIterator>::value>>
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

		MOMO_MORE_HASH_POSITION_OPERATORS(HashMapPosition)

		friend bool operator==(HashMapPosition pos1, Iterator iter2) noexcept
		{
			return pos1 == static_cast<HashMapPosition>(iter2);
		}

		friend bool operator!=(HashMapPosition pos1, Iterator iter2) noexcept
		{
			return !(pos1 == iter2);
		}

		friend bool operator==(Iterator iter1, HashMapPosition pos2) noexcept
		{
			return pos2 == iter1;
		}

		friend bool operator!=(Iterator iter1, HashMapPosition pos2) noexcept
		{
			return !(pos2 == iter1);
		}

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
		typedef HashMapIterator<typename HashSetBucketBounds::Iterator, isConst> Iterator;

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

		MOMO_FRIENDS_SIZE_BEGIN_END_CONST(HashMapBucketBounds, Iterator)

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
	private:
		typedef internal::MapNestedSetItemTraits<TKeyValueTraits> MapNestedSetItemTraits;

	protected:
		using typename MapNestedSetItemTraits::KeyValueTraits;
		using typename MapNestedSetItemTraits::Value;

	public:
		using typename MapNestedSetItemTraits::Key;
		using typename MapNestedSetItemTraits::Item;
		using typename MapNestedSetItemTraits::MemManager;

	public:
		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			for (size_t i = 0; i < count; ++i)
				Item::Create(dstItems + i);
			auto exec = [&itemCreator, newItem] ()
				{ std::forward<ItemCreator>(itemCreator)(newItem); };
			try
			{
				KeyValueTraits::RelocateExec(memManager,
					MapKeyIterator<Item*>(srcItems), MapValueIterator<Item*>(srcItems),
					MapKeyIterator<Item*>(dstItems), MapValueIterator<Item*>(dstItems),
					count, exec);
			}
			catch (...)
			{
				for (size_t i = 0; i < count; ++i)
					Item::Destroy(dstItems[i]);
				throw;
			}
			for (size_t i = 0; i < count; ++i)
				Item::Destroy(srcItems[i]);
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

/*!
	All `HashMap` functions and constructors have strong exception safety,
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
	typedef internal::HashMapNestedSetItemTraits<KeyValueTraits> HashSetItemTraits;
	typedef typename HashSetItemTraits::Item KeyValuePair;

	typedef internal::HashMapNestedSetSettings<Settings> HashSetSettings;

	typedef momo::HashSet<Key, HashTraits, MemManager, HashSetItemTraits, HashSetSettings> HashSet;

	typedef typename HashSet::ConstIterator HashSetConstIterator;
	typedef typename HashSet::ConstPosition HashSetConstPosition;

	typedef typename HashSet::ExtractedItem HashSetExtractedItem;

public:
	typedef internal::HashMapIterator<HashSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::HashMapPosition<HashSetConstPosition> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

	typedef internal::MapExtractedPair<HashSetExtractedItem> ExtractedPair;

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
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetHashSetIterator)
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

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(internal::MapPairConverter<ArgIterator>::Convert(*std::declval<ArgIterator>()))>
	explicit HashMap(ArgIterator begin, ArgSentinel end,
		const HashTraits& hashTraits = HashTraits(), MemManager memManager = MemManager())
		: HashMap(hashTraits, std::move(memManager))
	{
		Insert(std::move(begin), std::move(end));
	}

	template<typename Pair = std::pair<Key, Value>>
	HashMap(std::initializer_list<Pair> pairs)
		: HashMap(pairs, HashTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit HashMap(std::initializer_list<Pair> pairs, const HashTraits& hashTraits,
		MemManager memManager = MemManager())
		: HashMap(pairs.begin(), pairs.end(), hashTraits, std::move(memManager))
	{
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

	~HashMap() = default;

	HashMap& operator=(HashMap&& hashMap) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(hashMap), *this);
	}

	HashMap& operator=(const HashMap& hashMap)
	{
		return internal::ContainerAssigner::Copy(hashMap, *this);
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
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(HashMap, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(HashMap, Iterator)

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

	MOMO_FORCEINLINE ConstPosition Find(const Key& key) const
	{
		return ConstPositionProxy(mHashSet.Find(key));
	}

	MOMO_FORCEINLINE Position Find(const Key& key)
	{
		return PositionProxy(mHashSet.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstPosition> Find(const KeyArg& key) const
	{
		return ConstPositionProxy(mHashSet.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	Position> Find(const KeyArg& key)
	{
		return PositionProxy(mHashSet.Find(key));
	}

	MOMO_FORCEINLINE bool ContainsKey(const Key& key) const
	{
		return mHashSet.ContainsKey(key);
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	bool> ContainsKey(const KeyArg& key) const
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
		return { PositionProxy(res.position), res.inserted };
	}

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(internal::MapPairConverter<ArgIterator>::Convert(*std::declval<ArgIterator>()))>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		size_t initCount = GetCount();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
		{
			auto pair = internal::MapPairConverter<ArgIterator>::Convert(*iter);
			typedef decltype(pair.first) KeyArg;
			typedef decltype(pair.second) ValueArg;
			MOMO_STATIC_ASSERT(std::is_same<Key, typename std::decay<KeyArg>::type>::value);
			InsertVar(std::forward<KeyArg>(pair.first), std::forward<ValueArg>(pair.second));
		}
		return GetCount() - initCount;
	}

	template<typename Pair = std::pair<Key, Value>>
	size_t Insert(std::initializer_list<Pair> pairs)
	{
		return Insert(pairs.begin(), pairs.end());
	}

	template<typename PairCreator, bool extraCheck = true>
	Position AddCrt(ConstPosition pos, PairCreator&& pairCreator)
	{
		auto itemCreator = [&pairCreator] (KeyValuePair* newItem)
			{ KeyValuePair::Create(newItem, std::forward<PairCreator>(pairCreator)); };
		return PositionProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetHashSetPosition(pos), std::move(itemCreator)));
	}

	template<typename ValueCreator, bool extraCheck = true>
	Position AddCrt(ConstPosition pos, Key&& key, ValueCreator&& valueCreator)
	{
		return pvAdd<extraCheck>(pos, std::move(key), std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
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

	template<typename ValueCreator, bool extraCheck = true>
	Position AddCrt(ConstPosition pos, const Key& key, ValueCreator&& valueCreator)
	{
		return pvAdd<extraCheck>(pos, key, std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
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
		return PositionProxy(mHashSet.Add(ConstPositionProxy::GetHashSetPosition(pos),
			std::move(ExtractedPairProxy::GetSetExtractedItem(extPair))));
	}

	MOMO_FORCEINLINE ValueReferenceRKey operator[](Key&& key)
	{
		Position pos = Find(static_cast<const Key&>(key));
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
		return IteratorProxy(mHashSet.Remove(ConstIteratorProxy::GetHashSetIterator(iter)));
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
		return IteratorProxy(mHashSet.Remove(ConstIteratorProxy::GetHashSetIterator(iter),
			ExtractedPairProxy::GetSetExtractedItem(extPair)));
	}

	bool Remove(const Key& key)
	{
		return mHashSet.Remove(key);
	}

	template<typename PairFilter>
	internal::EnableIf<internal::IsInvocable<const PairFilter&, bool, const Key&, const Value&>::value,
	size_t> Remove(const PairFilter& pairFilter)
	{
		auto itemFilter = [&pairFilter] (const KeyValuePair& item)
			{ return pairFilter(item.GetKey(), static_cast<const Value&>(item.GetValue())); };
		return mHashSet.Remove(itemFilter);
	}

	ExtractedPair Extract(ConstPosition pos)
	{
		return ExtractedPair(*this, static_cast<ConstIterator>(pos));	// need RVO for exception safety
	}

	template<typename KeyArg, bool extraCheck = true>
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
		return IteratorProxy(ConstIteratorProxy::GetHashSetIterator(iter));
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		mHashSet.CheckIterator(ConstIteratorProxy::GetHashSetIterator(iter), allowEmpty);
	}

private:
	template<typename RKey, typename ValueCreator>
	InsertResult pvInsert(RKey&& key, ValueCreator&& valueCreator)
	{
		auto itemCreator = [this, &key, &valueCreator] (KeyValuePair* newItem)
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, GetMemManager(),
				std::forward<RKey>(key), std::forward<ValueCreator>(valueCreator));
		};
		typename HashSet::InsertResult res = mHashSet.template InsertCrt<decltype(itemCreator), false>(
			static_cast<const Key&>(key), std::move(itemCreator));
		return { IteratorProxy(res.position), res.inserted };
	}

	template<bool extraCheck, typename RKey, typename ValueCreator>
	Position pvAdd(ConstPosition pos, RKey&& key, ValueCreator&& valueCreator)
	{
		auto itemCreator = [this, &key, &valueCreator] (KeyValuePair* newItem)
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, GetMemManager(),
				std::forward<RKey>(key), std::forward<ValueCreator>(valueCreator));
		};
		return PositionProxy(mHashSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetHashSetPosition(pos), std::move(itemCreator)));
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

namespace std
{
	template<typename SI, bool c>
	struct iterator_traits<momo::internal::HashMapIterator<SI, c>>
		: public momo::internal::IteratorTraitsStd<momo::internal::HashMapIterator<SI, c>,
			forward_iterator_tag>
	{
	};
} // namespace std

#endif // MOMO_INCLUDE_GUARD_HASH_MAP
