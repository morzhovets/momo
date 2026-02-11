/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashListMap.h

  namespace momo:
    class HashListMapKeyValueTraits
    class HashListMapSettings
    class HashListMapCore
    class HashListMap

\**********************************************************/

#pragma once

#include "HashListSet.h"
#include "HashMap.h"

namespace momo
{

namespace internal
{
	template<typename TKeyValueTraits>
	class HashListMapNestedSetItemTraits
	{
	protected:
		typedef TKeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Value Value;

	public:
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::MemManager MemManager;

		typedef MapKeyValuePair<Key, Value,
			KeyValueTraits::keyAlignment, KeyValueTraits::valueAlignment> Item;
		static_assert(std::is_trivially_destructible_v<Item>);

		static const size_t alignment = ObjectAlignmenter<Item>::alignment;

		template<typename ItemArg>
		requires std::is_same_v<ItemArg, const Item&>
		class Creator
		{
		public:
			explicit Creator(MemManager& memManager, const Item& item) noexcept
				: mMemManager(memManager),
				mItem(item)
			{
			}

			void operator()(Item* newItem) &&
			{
				typedef typename KeyValueTraits::template ValueCreator<const Value&> ValueCreator;
				Item::template Create<KeyValueTraits>(newItem, mMemManager, mItem.GetKey(),
					FastMovableFunctor(ValueCreator(mMemManager, std::as_const(mItem.GetValue()))));
			}

		private:
			MemManager& mMemManager;
			const Item& mItem;
		};

	public:
		static const Key& GetKey(const Item& item) noexcept
		{
			return item.GetKey();
		}

		template<conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
		static void Destroy(MemManagerOrNullPtr memManager, Item& item) noexcept
		{
			KeyValueTraits::DestroyKey(memManager, item.GetKey());
			KeyValueTraits::DestroyValue(memManager, item.GetValue());
		}
	};

	template<typename THashListMapSettings>
	class HashListMapNestedSetSettings //: public HashListSetSettings
	{
	protected:
		typedef THashListMapSettings HashListMapSettings;

	public:
		static const CheckMode checkMode = HashListMapSettings::checkMode;
		static const ExtraCheckMode extraCheckMode = HashListMapSettings::extraCheckMode;
		static const bool allowExceptionSuppression = HashListMapSettings::allowExceptionSuppression;
	};
}

template<conceptObject TKey, conceptObject TValue,
	conceptMemManager TMemManager = MemManagerDefault>
class HashListMapKeyValueTraits
{
public:
	typedef TKey Key;
	typedef TValue Value;
	typedef TMemManager MemManager;

private:
	typedef internal::ObjectManager<Key, MemManager> KeyManager;
	typedef internal::ObjectManager<Value, MemManager> ValueManager;

public:
	static const size_t keyAlignment = KeyManager::alignment;
	static const size_t valueAlignment = ValueManager::alignment;

	template<typename... ValueArgs>
	using ValueCreator = typename ValueManager::template Creator<ValueArgs...>;

public:
	template<internal::conceptObjectCreator<Value> ValueCreator>
	static void Create(MemManager& memManager, Key&& key,
		FastMovableFunctor<ValueCreator> valueCreator, Key* newKey, Value* newValue)
	{
		KeyManager::MoveExec(memManager, std::move(key), newKey,
			FastMovableFunctor(internal::ObjectCreateExecutor(std::move(valueCreator), newValue)));
	}

	template<internal::conceptObjectCreator<Value> ValueCreator>
	static void Create(MemManager& memManager, const Key& key,
		FastMovableFunctor<ValueCreator> valueCreator, Key* newKey, Value* newValue)
	{
		KeyManager::CopyExec(memManager, key, newKey,
			FastMovableFunctor(internal::ObjectCreateExecutor(std::move(valueCreator), newValue)));
	}

	template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
	static void DestroyKey(MemManagerOrNullPtr memManager, Key& key) noexcept
	{
		KeyManager::Destroy(memManager, key);
	}

	template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
	static void DestroyValue(MemManagerOrNullPtr memManager, Value& value) noexcept
	{
		ValueManager::Destroy(memManager, value);
	}

	template<typename ValueArg>
	static void AssignValue(MemManager& /*memManager*/, ValueArg&& valueArg, Value& value)
	{
		value = std::forward<ValueArg>(valueArg);
	}
};

class HashListMapSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool allowExceptionSuppression = true;
};

template<typename TKeyValueTraits,
	conceptHashTraits<typename TKeyValueTraits::Key> THashTraits
		= HashTraits<typename TKeyValueTraits::Key>,
	typename TSettings = HashListMapSettings>
requires conceptMapKeyValueTraits<TKeyValueTraits, typename TKeyValueTraits::Key,
	typename TKeyValueTraits::Value, typename TKeyValueTraits::MemManager>
class MOMO_EMPTY_BASES HashListMapCore
	: public internal::Rangeable,
	public internal::Swappable<HashListMapCore>
{
public:
	typedef TKeyValueTraits KeyValueTraits;
	typedef THashTraits HashTraits;
	typedef TSettings Settings;
	typedef typename KeyValueTraits::Key Key;
	typedef typename KeyValueTraits::Value Value;
	typedef typename KeyValueTraits::MemManager MemManager;

private:
	typedef internal::HashListMapNestedSetItemTraits<KeyValueTraits> HashListSetItemTraits;
	typedef typename HashListSetItemTraits::Item KeyValuePair;

	typedef internal::HashListMapNestedSetSettings<Settings> HashListSetSettings;

	typedef HashListSetCore<HashListSetItemTraits, HashTraits, HashListSetSettings> HashListSet;

	typedef typename HashListSet::ConstIterator HashListSetConstIterator;
	typedef typename HashListSet::ConstPosition HashListSetConstPosition;

	typedef typename HashListSet::ExtractedItem HashListSetExtractedItem;

public:
	typedef internal::MapPosition<HashListSetConstPosition> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef typename Position::Iterator Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::InsertResult<Iterator> InsertResult;

	typedef internal::MapExtractedPair<HashListSetExtractedItem, false> ExtractedPair;

	typedef internal::HashMapBucketBounds<typename HashListSet::ConstBucketBounds> BucketBounds;
	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

	static const size_t bucketMaxItemCount = HashListSet::bucketMaxItemCount;

private:
	typedef internal::MapValueReferencer<HashListMapCore, Position, false> ValueReferencer;

public:
	template<typename KeyReference>
	using ValueReference = ValueReferencer::template ValueReference<KeyReference>;

private:
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	template<typename KeyArg>
	using IsValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>;

	struct ConstIteratorProxy : private ConstIterator
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetSetIterator)
	};

	struct ConstPositionProxy : private ConstPosition
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetSetPosition)
	};

	struct ExtractedPairProxy : private ExtractedPair
	{
		MOMO_DECLARE_PROXY_FUNCTION(ExtractedPair, GetSetExtractedItem)
	};

public:
	HashListMapCore()
		: HashListMapCore(HashTraits())
	{
	}

	explicit HashListMapCore(const HashTraits& hashTraits, MemManager memManager = MemManager())
		: mHashListSet(hashTraits, std::move(memManager))
	{
	}

	template<internal::conceptMapArgIterator<Key> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit HashListMapCore(ArgIterator begin, ArgSentinel end,
		const HashTraits& hashTraits = HashTraits(), MemManager memManager = MemManager())
		: HashListMapCore(hashTraits, std::move(memManager))
	{
		Insert(std::move(begin), std::move(end));
	}

	template<typename Pair = std::pair<Key, Value>>
	HashListMapCore(std::initializer_list<Pair> pairs)
		: HashListMapCore(pairs, HashTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit HashListMapCore(std::initializer_list<Pair> pairs, const HashTraits& hashTraits,
		MemManager memManager = MemManager())
		: HashListMapCore(pairs.begin(), pairs.end(), hashTraits, std::move(memManager))
	{
	}

	HashListMapCore(HashListMapCore&& hashListMap) noexcept
		: mHashListSet(std::move(hashListMap.mHashListSet))
	{
	}

	HashListMapCore(const HashListMapCore& hashListMap)
		: mHashListSet(hashListMap.mHashListSet)
	{
	}

	explicit HashListMapCore(const HashListMapCore& hashListMap, MemManager memManager)
		: mHashListSet(hashListMap.mHashListSet, std::move(memManager))
	{
	}

	~HashListMapCore() noexcept = default;

	HashListMapCore& operator=(HashListMapCore&& hashListMap) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(hashListMap), *this);
	}

	HashListMapCore& operator=(const HashListMapCore& hashListMap)
	{
		return internal::ContainerAssigner::Copy(hashListMap, *this);
	}

	void Swap(HashListMapCore& hashListMap) noexcept
	{
		mHashListSet.Swap(hashListMap.mHashListSet);
	}

	ConstIterator GetBegin() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(mHashListSet.GetBegin());
	}

	Iterator GetBegin() noexcept
	{
		return internal::ProxyConstructor<Iterator>(mHashListSet.GetBegin());
	}

	ConstIterator GetEnd() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(mHashListSet.GetEnd());
	}

	Iterator GetEnd() noexcept
	{
		return internal::ProxyConstructor<Iterator>(mHashListSet.GetEnd());
	}

	const HashTraits& GetHashTraits() const noexcept
	{
		return mHashListSet.GetHashTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mHashListSet.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mHashListSet.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mHashListSet.GetCount();
	}

	bool IsEmpty() const noexcept
	{
		return mHashListSet.IsEmpty();
	}

	void Clear(bool shrink = true) noexcept
	{
		mHashListSet.Clear(shrink);
	}

	size_t GetCapacity() const noexcept
	{
		return mHashListSet.GetCapacity();
	}

	void Reserve(size_t capacity)
	{
		mHashListSet.Reserve(capacity);
	}

	MOMO_FORCEINLINE ConstPosition Find(const Key& key) const
	{
		return internal::ProxyConstructor<ConstPosition>(mHashListSet.Find(key));
	}

	MOMO_FORCEINLINE Position Find(const Key& key)
	{
		return internal::ProxyConstructor<Position>(mHashListSet.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE ConstPosition Find(const KeyArg& key) const
	{
		return internal::ProxyConstructor<ConstPosition>(mHashListSet.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE Position Find(const KeyArg& key)
	{
		return internal::ProxyConstructor<Position>(mHashListSet.Find(key));
	}

	MOMO_FORCEINLINE bool ContainsKey(const Key& key) const
	{
		return mHashListSet.ContainsKey(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE bool ContainsKey(const KeyArg& key) const
	{
		return mHashListSet.ContainsKey(key);
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
		typename HashListSet::InsertResult res =
			mHashListSet.Insert(std::move(ExtractedPairProxy::GetSetExtractedItem(extPair)));
		return { internal::ProxyConstructor<Position>(res.position), res.inserted };
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
	Iterator AddCrt(ConstPosition pos, PairCreator pairCreator)
	{
		return pvAdd<extraCheck>(pos, FastMovableFunctor(std::forward<PairCreator>(pairCreator)));
	}

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Iterator AddCrt(ConstPosition pos, Key&& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, std::move(key),
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	Iterator AddVar(ConstPosition pos, Key&& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(pos, std::move(key),
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstPosition pos, Key&& key, Value&& value)
	{
		return AddVar(pos, std::move(key), std::move(value));
	}

	Iterator Add(ConstPosition pos, Key&& key, const Value& value)
	{
		return AddVar(pos, std::move(key), value);
	}

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Iterator AddCrt(ConstPosition pos, const Key& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, key,
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	Iterator AddVar(ConstPosition pos, const Key& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(pos, key,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstPosition pos, const Key& key, Value&& value)
	{
		return AddVar(pos, key, std::move(value));
	}

	Iterator Add(ConstPosition pos, const Key& key, const Value& value)
	{
		return AddVar(pos, key, value);
	}

	//Iterator Add(ConstPosition pos, ExtractedPair&& extPair)

	template<typename ValueArg = Value>
	requires requires { typename ValueCreator<ValueArg>; }
	InsertResult InsertOrAssign(Key&& key, ValueArg&& valueArg)
	{
		return pvInsertOrAssign(std::move(key), std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg = Value>
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
		return internal::ProxyConstructor<Iterator>(
			mHashListSet.Remove(ConstIteratorProxy::GetSetIterator(iter)));
	}

	Iterator Remove(Iterator iter)
	{
		return Remove(static_cast<ConstIterator>(iter));
	}

	Iterator Remove(ConstPosition pos)
	{
		return Remove(static_cast<ConstIterator>(pos));
	}

	Iterator Remove(Position pos)
	{
		return Remove(static_cast<ConstIterator>(pos));
	}

	//Iterator Remove(ConstIterator iter, ExtractedPair& extPair)

	Iterator Remove(ConstIterator begin, ConstIterator end)
	{
		return internal::ProxyConstructor<Iterator>(mHashListSet.Remove(
			ConstIteratorProxy::GetSetIterator(begin), ConstIteratorProxy::GetSetIterator(end)));
	}

	bool Remove(const Key& key)
	{
		return mHashListSet.Remove(key);
	}

	template<internal::conceptMapPairPredicate<Key, Value> PairFilter>
	size_t Remove(PairFilter pairFilter)
	{
		auto itemFilter = [fastPairFilter = FastCopyableFunctor(pairFilter)] (const KeyValuePair& item)
			{ return fastPairFilter(item.GetKey(), std::as_const(item.GetValue())); };
		return mHashListSet.Remove(itemFilter);
	}

	//ExtractedPair Extract(ConstPosition pos)

	//template<typename KeyArg,
	//	bool extraCheck = true>
	//void ResetKey(ConstPosition pos, KeyArg&& keyArg)

	template<typename RMap>
	void MergeFrom(RMap&& srcMap)
	{
		srcMap.MergeTo(mHashListSet);
	}

	template<typename Map>
	void MergeTo(Map& dstMap)
	{
		dstMap.MergeFrom(mHashListSet);
	}

	size_t GetBucketCount() const noexcept
	{
		return mHashListSet.GetBucketCount();
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		return internal::ProxyConstructor<ConstBucketBounds>(mHashListSet.GetBucketBounds(bucketIndex));
	}

	BucketBounds GetBucketBounds(size_t bucketIndex)
	{
		return internal::ProxyConstructor<BucketBounds>(mHashListSet.GetBucketBounds(bucketIndex));
	}

	size_t GetBucketIndex(const Key& key) const
	{
		return mHashListSet.GetBucketIndex(key);
	}

	//ConstPosition MakePosition(size_t hashCode) const noexcept

	Iterator MakeMutableIterator(ConstIterator iter)
	{
		return internal::ProxyConstructor<Iterator>(ConstIteratorProxy::GetSetIterator(iter));
	}

	//void CheckIterator(ConstIterator iter, bool allowEmpty = true) const

private:
	template<typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult pvInsert(RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mHashListSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		typename HashListSet::InsertResult res = mHashListSet.template InsertCrt<decltype(itemCreator), false>(
			std::as_const(key), std::move(itemCreator));
		return { internal::ProxyConstructor<Iterator>(res.position), res.inserted };
	}

	template<bool extraCheck, internal::conceptMapPairCreator<Key, Value> PairCreator>
	Iterator pvAdd(ConstPosition pos, FastMovableFunctor<PairCreator> pairCreator)
	{
		auto itemCreator = [this, pairCreator = std::move(pairCreator)] (KeyValuePair* newItem) mutable
			{ std::construct_at(newItem, mHashListSet.GetMemManager(), std::move(pairCreator)); };
		return internal::ProxyConstructor<Iterator>(
			mHashListSet.template AddCrt<decltype(itemCreator), extraCheck>(
				ConstPositionProxy::GetSetPosition(pos), std::move(itemCreator)));
	}

	template<bool extraCheck, typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	Iterator pvAdd(ConstPosition pos, RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mHashListSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		return internal::ProxyConstructor<Iterator>(
			mHashListSet.template AddCrt<decltype(itemCreator), extraCheck>(
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
	HashListSet mHashListSet;
};

template<conceptObject TKey, conceptObject TValue,
	conceptHashTraits<TKey> THashTraits = HashTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault>
using HashListMap = HashListMapCore<HashListMapKeyValueTraits<TKey, TValue, TMemManager>, THashTraits>;

} // namespace momo
