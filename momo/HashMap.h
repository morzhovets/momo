/**********************************************************\

  momo/HashMap.h

  namespace momo:
    struct HashMapKeyValueTraits
    struct HashMapSettings
    class HashMap

\**********************************************************/

#pragma once

#include "HashSet.h"

namespace momo
{

namespace internal
{
	template<typename TKey, typename TValue, typename THashSetReference>
	class HashMapReference
	{
	public:
		typedef TKey Key;
		typedef TValue Value;
		typedef THashSetReference HashSetReference;

		typedef HashMapReference<Key, const Value, HashSetReference> ConstReference;

	public:
		HashMapReference(const Key& key, Value& value) MOMO_NOEXCEPT
			: key(key),
			value(value)
		{
		}

		explicit HashMapReference(HashSetReference hashSetRef) MOMO_NOEXCEPT
			: key(hashSetRef.GetKey()),
			value(hashSetRef.GetValue())
		{
		}

		operator ConstReference() const MOMO_NOEXCEPT
		{
			return ConstReference(key, value);
		}

	public:
		const Key& key;
		Value& value;
	};

	template<typename TBucketIterator, typename THashSetBucketBounds>
	class HashMapBucketBounds
	{
	public:
		typedef TBucketIterator BucketIterator;
		typedef THashSetBucketBounds HashSetBucketBounds;

		typedef BucketIterator Iterator;

		typedef HashMapBucketBounds<typename BucketIterator::ConstIterator,
			HashSetBucketBounds> ConstBounds;

	public:
		HashMapBucketBounds() MOMO_NOEXCEPT
		{
		}

		HashMapBucketBounds(HashSetBucketBounds hashSetBucketBounds) MOMO_NOEXCEPT
			: mHashSetBucketBounds(hashSetBucketBounds)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBounds(mHashSetBucketBounds);
		}

		BucketIterator GetBegin() const MOMO_NOEXCEPT
		{
			return BucketIterator(mHashSetBucketBounds.GetBegin());
		}

		BucketIterator GetEnd() const MOMO_NOEXCEPT
		{
			return BucketIterator(mHashSetBucketBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const HashMapBucketBounds&, BucketIterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mHashSetBucketBounds.GetCount();
		}

	private:
		HashSetBucketBounds mHashSetBucketBounds;
	};
}

template<typename TKey, typename TValue>
struct HashMapKeyValueTraits
{
	typedef TKey Key;
	typedef TValue Value;

	typedef internal::ObjectManager<Key> KeyManager;
	typedef internal::ObjectManager<Value> ValueManager;

	static const size_t keyAlignment = KeyManager::alignment;
	static const size_t valueAlignment = ValueManager::alignment;

	static const bool isKeyNothrowRelocatable = KeyManager::isNothrowRelocatable;
	static const bool isValueNothrowRelocatable = ValueManager::isNothrowRelocatable;

#ifndef MOMO_USE_SAFE_MAP_BRACKETS
	typedef typename ValueManager::Creator ValueCreator;
#endif

	typedef typename ValueManager::MoveCreator MoveValueCreator;
	typedef typename ValueManager::CopyCreator CopyValueCreator;

	static void CreateKey(const Key& key, void* pkey)
	{
		MOMO_STATIC_ASSERT(!isKeyNothrowRelocatable);
		KeyManager::Create(key, pkey);
	}

	template<typename ValueCreator>
	static void CreatePair(Key&& key, const ValueCreator& valueCreator, void* pkey, void* pvalue)
	{
		KeyManager::CreatePair(std::move(key), valueCreator, pkey, pvalue);
	}

	template<typename ValueCreator>
	static void CreatePair(const Key& key, const ValueCreator& valueCreator,
		void* pkey, void* pvalue)
	{
		KeyManager::CreatePair(key, valueCreator, pkey, pvalue);
	}

	static void DestroyKey(Key& key) MOMO_NOEXCEPT
	{
		KeyManager::Destroy(key);
	}

	static void DestroyValue(Value& value) MOMO_NOEXCEPT
	{
		ValueManager::Destroy(value);
	}

	static void RelocateKeyNothrow(Key& srcKey, Key* dstKey) MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT(isKeyNothrowRelocatable);
		KeyManager::Relocate(std::addressof(srcKey), dstKey, 1);
	}

	static void RelocateValueNothrow(Value& srcValue, Value* dstValue) MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT(isValueNothrowRelocatable);
		ValueManager::Relocate(std::addressof(srcValue), dstValue, 1);
	}

	static void AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue)
	{
		_AssignPair(std::move(srcKey), std::move(srcValue), dstKey, dstValue,
			internal::BoolConstant<KeyManager::isNothrowAnywayMoveAssignable>(),
			internal::BoolConstant<ValueManager::isNothrowAnywayMoveAssignable>());
	}

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
	static void AssignValue(Value&& srcValue, Value& dstValue)
	{
		dstValue = std::move(srcValue);
	}

	static void AssignValue(const Value& srcValue, Value& dstValue)
	{
		dstValue = srcValue;
	}
#endif

private:
	template<bool isValueNothrowAnywayMoveAssignable>
	static void _AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
		std::true_type /*isKeyNothrowAnywayMoveAssignable*/,
		internal::BoolConstant<isValueNothrowAnywayMoveAssignable>)
	{
		dstValue = std::move(srcValue);
		KeyManager::AssignNothrowAnyway(std::move(srcKey), dstKey);
	}

	static void _AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
		std::false_type /*isKeyNothrowAnywayMoveAssignable*/,
		std::true_type /*isValueNothrowAnywayMoveAssignable*/)
	{
		dstKey = std::move(srcKey);
		ValueManager::AssignNothrowAnyway(std::move(srcValue), dstValue);
	}

	// basic exception safety
	static void _AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
		std::false_type /*isKeyNothrowAnywayMoveAssignable*/,
		std::false_type /*isValueNothrowAnywayMoveAssignable*/)
	{
		dstValue = (const Value&)srcValue;
		dstKey = std::move(srcKey);
	}
};

struct HashMapSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
};

template<typename TKey, typename TValue,
	typename THashTraits = HashTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TKeyValueTraits = HashMapKeyValueTraits<TKey, TValue>,
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
	class KeyValuePair
	{
	public:
		template<typename KeyValueCreator>
		explicit KeyValuePair(const KeyValueCreator& keyValueCreator)
		{
			keyValueCreator(&mKeyBuffer, &mValueBuffer);
		}

		template<typename ValueCreator>
		KeyValuePair(Key&& key, const ValueCreator& valueCreator)
		{
			KeyValueTraits::CreatePair(std::move(key), valueCreator, &mKeyBuffer, &mValueBuffer);
		}

		template<typename ValueCreator>
		KeyValuePair(const Key& key, const ValueCreator& valueCreator)
		{
			KeyValueTraits::CreatePair(key, valueCreator, &mKeyBuffer, &mValueBuffer);
		}

		KeyValuePair(KeyValuePair&& pair)
		{
			KeyValueTraits::CreatePair(std::move(pair.GetKey()),
				typename KeyValueTraits::MoveValueCreator(std::move(pair.GetValue())),
				&mKeyBuffer, &mValueBuffer);
		}

		KeyValuePair(const KeyValuePair& pair)
		{
			KeyValueTraits::CreatePair(pair.GetKey(),
				typename KeyValueTraits::CopyValueCreator((const Value&)pair.GetValue()),
				&mKeyBuffer, &mValueBuffer);
		}

		~KeyValuePair() MOMO_NOEXCEPT
		{
			KeyValueTraits::DestroyKey(GetKey());
			KeyValueTraits::DestroyValue(GetValue());
		}

		const Key& GetKey() const MOMO_NOEXCEPT
		{
			return *&mKeyBuffer;
		}

		Key& GetKey() MOMO_NOEXCEPT
		{
			return *&mKeyBuffer;
		}

		Value& GetValue() const MOMO_NOEXCEPT
		{
			return *&mValueBuffer;
		}

		template<typename PairCreator>
		static void RelocateAddBack(KeyValuePair* srcPairs, KeyValuePair* dstPairs,
			size_t srcCount, const PairCreator& pairCreator)
		{
			_RelocateAddBack(srcPairs, dstPairs, srcCount, pairCreator,
				internal::BoolConstant<KeyValueTraits::isKeyNothrowRelocatable>(),
				internal::BoolConstant<KeyValueTraits::isValueNothrowRelocatable>());
		}

		static void Assign(KeyValuePair&& srcPair, KeyValuePair& dstPair)
		{
			KeyValueTraits::AssignPair(std::move(srcPair.GetKey()), std::move(srcPair.GetValue()),
				dstPair.GetKey(), dstPair.GetValue());
		}

	private:
		template<typename PairCreator>
		static void _RelocateAddBack(KeyValuePair* srcPairs, KeyValuePair* dstPairs,
			size_t srcCount, const PairCreator& pairCreator,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			pairCreator(dstPairs + srcCount);
			for (size_t i = 0; i < srcCount; ++i)	//?
			{
				KeyValuePair& srcPair = srcPairs[i];
				KeyValuePair& dstPair = dstPairs[i];
				KeyValueTraits::RelocateKeyNothrow(srcPair.GetKey(), &dstPair.mKeyBuffer);
				KeyValueTraits::RelocateValueNothrow(srcPair.GetValue(), &dstPair.mValueBuffer);
			}
		}

		template<typename PairCreator>
		static void _RelocateAddBack(KeyValuePair* srcPairs, KeyValuePair* dstPairs,
			size_t srcCount, const PairCreator& pairCreator,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				for (; index < srcCount; ++index)
				{
					typename KeyValueTraits::CopyValueCreator(srcPairs[index].GetValue())
						(&dstPairs[index].mValueBuffer);
				}
				pairCreator(dstPairs + srcCount);
			}
			catch (...)
			{
				for (size_t i = 0; i < index; ++i)
					KeyValueTraits::DestroyValue(dstPairs[i].GetValue());
				throw;
			}
			for (size_t i = 0; i < srcCount; ++i)
			{
				KeyValuePair& srcPair = srcPairs[i];
				KeyValueTraits::RelocateKeyNothrow(srcPair.GetKey(), &dstPairs[i].mKeyBuffer);
				KeyValueTraits::DestroyValue(srcPair.GetValue());
			}
		}

		template<typename PairCreator>
		static void _RelocateAddBack(KeyValuePair* srcPairs, KeyValuePair* dstPairs,
			size_t srcCount, const PairCreator& pairCreator,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				for (; index < srcCount; ++index)
				{
					KeyValueTraits::CreateKey((const Key&)srcPairs[index].GetKey(),
						&dstPairs[index].mKeyBuffer);
				}
				pairCreator(dstPairs + srcCount);
			}
			catch (...)
			{
				for (size_t i = 0; i < index; ++i)
					KeyValueTraits::DestroyKey(dstPairs[i].GetKey());
				throw;
			}
			for (size_t i = 0; i < srcCount; ++i)
			{
				KeyValuePair& srcPair = srcPairs[i];
				KeyValueTraits::DestroyKey(srcPair.GetKey());
				KeyValueTraits::RelocateValueNothrow(srcPair.GetValue(),
					&dstPairs[i].mValueBuffer);
			}
		}

		template<typename PairCreator>
		static void _RelocateAddBack(KeyValuePair* srcPairs, KeyValuePair* dstPairs,
			size_t srcCount, const PairCreator& pairCreator,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t keyIndex = 0;
			size_t valueIndex = 0;
			try
			{
				for (; keyIndex < srcCount; ++keyIndex)
				{
					KeyValueTraits::CreateKey((const Key&)srcPairs[keyIndex].GetKey(),
						&dstPairs[keyIndex].mKeyBuffer);
				}
				for (; valueIndex < srcCount; ++valueIndex)
				{
					typename KeyValueTraits::CopyValueCreator(srcPairs[valueIndex].GetValue())
						(&dstPairs[valueIndex].mValueBuffer);
				}
				pairCreator(dstPairs + srcCount);
			}
			catch (...)
			{
				for (size_t i = 0; i < keyIndex; ++i)
					KeyValueTraits::DestroyKey(dstPairs[i].GetKey());
				for (size_t i = 0; i < valueIndex; ++i)
					KeyValueTraits::DestroyValue(dstPairs[i].GetValue());
				throw;
			}
			for (size_t i = 0; i < srcCount; ++i)
				srcPairs[i].~KeyValuePair();
		}

	private:
		MOMO_DISABLE_COPY_OPERATOR(KeyValuePair);

	private:
		internal::ObjectBuffer<Key, KeyValueTraits::keyAlignment> mKeyBuffer;
		mutable internal::ObjectBuffer<Value, KeyValueTraits::valueAlignment> mValueBuffer;
	};

	struct HashSetItemTraits
	{
		typedef typename HashMap::Key Key;
		typedef KeyValuePair Item;

		typedef internal::ObjectManager<Item> ItemManager;

		static const size_t alignment = ItemManager::alignment;

		typedef typename ItemManager::MoveCreator MoveCreator;
		typedef typename ItemManager::CopyCreator CopyCreator;

		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			return item.GetKey();
		}

		static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
		{
			ItemManager::Destroy(items, count);
		}

		static void Assign(Item&& srcItem, Item& dstItem)
		{
			KeyValuePair::Assign(std::move(srcItem), dstItem);
		}

		template<typename ItemCreator>
		static void RelocateAddBack(Item* srcItems, Item* dstItems, size_t srcCount,
			const ItemCreator& itemCreator)
		{
			KeyValuePair::RelocateAddBack(srcItems, dstItems, srcCount, itemCreator);
		}
	};

	struct HashSetSettings : public momo::HashSetSettings
	{
		static const CheckMode checkMode = Settings::checkMode;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
	};

	typedef momo::HashSet<Key, HashTraits, MemManager, HashSetItemTraits, HashSetSettings> HashSet;

	typedef typename HashSet::ConstIterator HashSetConstIterator;
	typedef typename HashSetConstIterator::Reference HashSetConstReference;
	typedef typename HashSet::ConstBucketBounds HashSetConstBucketBounds;
	typedef typename HashSetConstBucketBounds::Iterator HashSetConstBucketIterator;

	typedef internal::HashMapReference<Key, Value, HashSetConstReference> Reference;

	typedef internal::HashDerivedIterator<HashSetConstBucketIterator, Reference,
		HashSetConstBucketIterator> BucketIterator;

public:
	typedef internal::HashDerivedIterator<HashSetConstIterator, Reference> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::HashInsertResult<Iterator> InsertResult;

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
private:
	template<typename RKey>
	class ValueReference
	{
	public:
		ValueReference(HashMap& hashMap, RKey&& key)
			: mHashMap(hashMap),
			mKey(std::forward<RKey>(key)),
			mIter(hashMap.Find((const Key&)key))
		{
		}

		ValueReference(ValueReference&& valueRef)
			: mHashMap(valueRef.mHashMap),
			mKey(std::forward<RKey>(valueRef.mKey)),
			mIter(valueRef.mIter)
		{
		}

		ValueReference& operator=(ValueReference&& valueRef)
		{
			return _Assign(std::move(valueRef.Get()));
		}

		ValueReference& operator=(const ValueReference& valueRef)
		{
			return _Assign(valueRef.Get());
		}

		ValueReference& operator=(Value&& value)
		{
			return _Assign(std::move(value));
		}

		ValueReference& operator=(const Value& value)
		{
			return _Assign(value);
		}

		operator Value&()
		{
			return Get();
		}

		Value& Get()
		{
			return mIter->value;
		}

	private:
		template<typename RValue>
		ValueReference& _Assign(RValue&& value)
		{
			if (!!mIter)
				KeyValueTraits::AssignValue(std::forward<RValue>(value), mIter->value);
			else
				mIter = mHashMap.Add(mIter, std::forward<RKey>(mKey), std::forward<RValue>(value));
			return *this;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(ValueReference);

	private:
		HashMap& mHashMap;
		RKey&& mKey;
		Iterator mIter;
	};

public:
	typedef ValueReference<Key&&> ValueReferenceRKey;
	typedef ValueReference<const Key&> ValueReferenceCKey;
#else
	typedef Value& ValueReferenceRKey;
	typedef Value& ValueReferenceCKey;
#endif

	typedef internal::HashMapBucketBounds<BucketIterator, HashSetConstBucketBounds> BucketBounds;
	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

public:
	explicit HashMap(const HashTraits& hashTraits = HashTraits(),
		MemManager&& memManager = MemManager())
		: mHashSet(hashTraits, std::move(memManager))
	{
	}

	HashMap(HashMap&& hashMap) MOMO_NOEXCEPT
		: mHashSet(std::move(hashMap.mHashSet))
	{
	}

	HashMap(const HashMap& hashMap)
		: mHashSet(hashMap.mHashSet)
	{
	}

	~HashMap() MOMO_NOEXCEPT
	{
	}

	HashMap& operator=(HashMap&& hashMap) MOMO_NOEXCEPT
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

	void Swap(HashMap& hashMap) MOMO_NOEXCEPT
	{
		mHashSet.Swap(hashMap.mHashSet);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		return ConstIterator(mHashSet.GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return Iterator(mHashSet.GetBegin());
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator();
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator();
	}

	MOMO_FRIEND_SWAP(HashMap)
	MOMO_FRIENDS_BEGIN_END(const HashMap&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(HashMap&, Iterator)

	const HashTraits& GetHashTraits() const MOMO_NOEXCEPT
	{
		return mHashSet.GetHashTraits();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mHashSet.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mHashSet.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mHashSet.GetCount();
	}

	bool IsEmpty() const MOMO_NOEXCEPT
	{
		return mHashSet.IsEmpty();
	}

	void Clear() MOMO_NOEXCEPT
	{
		mHashSet.Clear();
	}

	size_t GetCapacity() const MOMO_NOEXCEPT
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
		return ConstIterator(mHashSet.Find(key));
	}

	Iterator Find(const Key& key)
	{
		return Iterator(mHashSet.Find(key));
	}

	bool HasKey(const Key& key) const
	{
		return mHashSet.HasKey(key);
	}

	template<typename ValueCreator>
	InsertResult InsertCrt(Key&& key, const ValueCreator& valueCreator)
	{
		return _Insert(std::move(key), valueCreator);
	}

	InsertResult Insert(Key&& key, Value&& value)
	{
		return _Insert(std::move(key),
			typename KeyValueTraits::MoveValueCreator(std::move(value)));
	}

	InsertResult Insert(Key&& key, const Value& value)
	{
		return _Insert(std::move(key), typename KeyValueTraits::CopyValueCreator(value));
	}

	template<typename ValueCreator>
	InsertResult InsertCrt(const Key& key, const ValueCreator& valueCreator)
	{
		return _Insert(key, valueCreator);
	}

	InsertResult Insert(const Key& key, Value&& value)
	{
		return _Insert(key, typename KeyValueTraits::MoveValueCreator(std::move(value)));
	}

	InsertResult Insert(const Key& key, const Value& value)
	{
		return _Insert(key, typename KeyValueTraits::CopyValueCreator(value));
	}

	template<typename Iterator>
	size_t InsertKV(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->key);
		MOMO_CHECK_TYPE(Value, begin->value);
		auto insertFunc = [this] (Iterator iter)
			{ return Insert(iter->key, iter->value); };
		return _Insert(begin, end, insertFunc);
	}

	template<typename Iterator>
	size_t InsertFS(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->first);
		MOMO_CHECK_TYPE(Value, begin->second);
		auto insertFunc = [this] (Iterator iter)
			{ return Insert(iter->first, iter->second); };
		return _Insert(begin, end, insertFunc);
	}

#ifdef MOMO_USE_INIT_LISTS
	size_t Insert(std::initializer_list<std::pair<Key, Value>> keyValuePairs)
	{
		return InsertFS(keyValuePairs.begin(), keyValuePairs.end());
	}
#endif

	template<typename KeyValueCreator>
	Iterator AddCrt(ConstIterator iter, const KeyValueCreator& keyValueCreator)
	{
		auto pairCreator = [&keyValueCreator] (void* ppair)
			{ new(ppair) KeyValuePair(keyValueCreator); };
		Iterator resIter = Iterator(mHashSet.AddCrt(iter.GetBaseIterator(), pairCreator));
		MOMO_EXTRA_CHECK(resIter == Find(resIter->key));
		return resIter;
	}

	Iterator Add(ConstIterator iter, Key&& key, Value&& value)
	{
		return _Add(iter, std::move(key),
			typename KeyValueTraits::MoveValueCreator(std::move(value)));
	}

	Iterator Add(ConstIterator iter, Key&& key, const Value& value)
	{
		return _Add(iter, std::move(key), typename KeyValueTraits::CopyValueCreator(value));
	}

	Iterator Add(ConstIterator iter, const Key& key, Value&& value)
	{
		return _Add(iter, key, typename KeyValueTraits::MoveValueCreator(std::move(value)));
	}

	Iterator Add(ConstIterator iter, const Key& key, const Value& value)
	{
		return _Add(iter, key, typename KeyValueTraits::CopyValueCreator(value));
	}

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
	ValueReferenceRKey operator[](Key&& key)
	{
		return ValueReferenceRKey(*this, std::move(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		return ValueReferenceCKey(*this, key);
	}
#else
	ValueReferenceRKey operator[](Key&& key)
	{
		return _Insert(std::move(key),
			typename KeyValueTraits::ValueCreator()).iterator->value;
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		return _Insert(key, typename KeyValueTraits::ValueCreator()).iterator->value;
	}
#endif

	// (!KeyManager::isNothrowAnywayMoveAssignable
	// && !ValueManager::isNothrowAnywayMoveAssignable) -> basic exception safety
	Iterator Remove(ConstIterator iter)
	{
		return Iterator(mHashSet.Remove(iter.GetBaseIterator()));
	}

	// (!KeyManager::isNothrowAnywayMoveAssignable
	// && !ValueManager::isNothrowAnywayMoveAssignable) -> basic exception safety
	bool Remove(const Key& key)
	{
		return mHashSet.Remove(key);
	}

	size_t GetBucketCount() const MOMO_NOEXCEPT
	{
		return mHashSet.GetBucketCount();
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		return ConstBucketBounds(mHashSet.GetBucketBounds(bucketIndex));
	}

	BucketBounds GetBucketBounds(size_t bucketIndex)
	{
		return BucketBounds(mHashSet.GetBucketBounds(bucketIndex));
	}

	size_t GetBucketIndex(const Key& key) const
	{
		return mHashSet.GetBucketIndex(key);
	}

private:
	template<typename RKey, typename ValueCreator>
	InsertResult _Insert(RKey&& key, const ValueCreator& valueCreator)
	{
		Iterator iter = Find((const Key&)key);
		if (!!iter)
			return InsertResult(iter, false);
		iter = _Add(iter, std::forward<RKey>(key), valueCreator);
		return InsertResult(iter, true);
	}

	template<typename Iterator, typename InsertFunc>
	size_t _Insert(Iterator begin, Iterator end, InsertFunc insertFunc)
	{
		size_t count = 0;
		for (Iterator iter = begin; iter != end; ++iter)
			count += insertFunc(iter).inserted ? 1 : 0;
		return count;
	}

	template<typename RKey, typename ValueCreator>
	Iterator _Add(ConstIterator iter, RKey&& key, const ValueCreator& valueCreator)
	{
		auto pairCreator = [&key, &valueCreator] (void* ppair)
			{ new(ppair) KeyValuePair(std::forward<RKey>(key), valueCreator); };
		return Iterator(mHashSet.AddCrt(iter.GetBaseIterator(), pairCreator));
	}

private:
	HashSet mHashSet;
};

} // namespace momo
