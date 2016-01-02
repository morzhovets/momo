/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashMultiMap.h

  namespace momo:
    struct HashMultiMapKeyValueTraits
    struct HashMultiMapSettings
    class HashMultiMap

\**********************************************************/

#pragma once

#include "HashMap.h"
#include "details/ArrayBucket.h"

namespace momo
{

namespace internal
{
	template<typename TKey, typename TValues, typename THashMapReference>
	class HashMultiMapKeyReference
	{
	public:
		typedef TKey Key;
		typedef TValues Values;
		typedef THashMapReference HashMapReference;

		typedef HashMultiMapKeyReference<Key, typename Values::ConstBounds,
			typename HashMapReference::ConstReference> ConstReference;

	public:
		HashMultiMapKeyReference(const Key& key, const Values& values) MOMO_NOEXCEPT
			: key(key),
			values(values)
		{
		}

		explicit HashMultiMapKeyReference(HashMapReference hashMapRef) MOMO_NOEXCEPT
			: key(hashMapRef.key),
			values(hashMapRef.value.GetBounds())
		{
		}

		operator ConstReference() const MOMO_NOEXCEPT
		{
			return ConstReference(key, values);
		}

	public:
		const Key& key;
		const Values values;
	};

	template<typename TKeyIterator>
	class HashMultiMapKeyBounds
	{
	public:
		typedef TKeyIterator KeyIterator;

		typedef KeyIterator Iterator;

		typedef HashMultiMapKeyBounds<typename KeyIterator::ConstIterator> ConstBounds;

	public:
		HashMultiMapKeyBounds() MOMO_NOEXCEPT
		{
		}

		explicit HashMultiMapKeyBounds(KeyIterator begin) MOMO_NOEXCEPT
			: mBegin(begin)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBounds(mBegin);
		}

		KeyIterator GetBegin() const MOMO_NOEXCEPT
		{
			return mBegin;
		}

		KeyIterator GetEnd() const MOMO_NOEXCEPT
		{
			return KeyIterator();
		}

		MOMO_FRIENDS_BEGIN_END(const HashMultiMapKeyBounds&, KeyIterator)

	private:
		KeyIterator mBegin;
	};

	template<typename TKeyIterator, typename TValue, typename TSettings>
	class HashMultiMapIterator
	{
	public:
		typedef TKeyIterator KeyIterator;
		typedef TValue Value;
		typedef TSettings Settings;

		typedef typename KeyIterator::Reference::Key Key;

		typedef HashMultiMapIterator<typename KeyIterator::ConstIterator,
			const Value, Settings> ConstIterator;

		class Reference
		{
		public:
			typedef typename ConstIterator::Reference ConstReference;

		public:
			Reference(const Key& key, Value& value) MOMO_NOEXCEPT
				: key(key),
				value(value)
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

		typedef HashPointer<Reference> Pointer;

	public:
		HashMultiMapIterator() MOMO_NOEXCEPT
			: mValuePtr(nullptr),
			mHashMultiMapVersion(nullptr),
			mVersion(0)
		{
		}

		HashMultiMapIterator(KeyIterator keyIter, Value* pvalue, const size_t& version,
			bool move) MOMO_NOEXCEPT
			: mKeyIterator(keyIter),
			mValuePtr(pvalue),
			mHashMultiMapVersion(&version),
			mVersion(version)
		{
			if (move)
				_Move();
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			if (mValuePtr != nullptr)
				return ConstIterator(mKeyIterator, mValuePtr, *mHashMultiMapVersion, false);
			else
				return ConstIterator();
		}

		HashMultiMapIterator& operator++()
		{
			_Check();
			++mValuePtr;
			_Move();
			return *this;
		}

		Pointer operator->() const
		{
			_Check();
			return Pointer(Reference(mKeyIterator->key, *mValuePtr));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mValuePtr == iter.GetValuePtr();
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashMultiMapIterator)

		KeyIterator GetKeyIterator() const MOMO_NOEXCEPT
		{
			return mKeyIterator;
		}

		Value* GetValuePtr() const MOMO_NOEXCEPT
		{
			return mValuePtr;
		}

		void Check(const size_t& version) const
		{
			(void)version;
			MOMO_CHECK(mHashMultiMapVersion == &version);
			MOMO_CHECK(mVersion == version);
			MOMO_CHECK(mValuePtr != nullptr);
		}

	private:
		void _Move() MOMO_NOEXCEPT
		{
			if (mValuePtr != mKeyIterator->values.GetEnd())
				return;
			++mKeyIterator;
			for (; !!mKeyIterator; ++mKeyIterator)
			{
				auto values = mKeyIterator->values;
				mValuePtr = values.GetBegin();
				if (mValuePtr != values.GetEnd())
					return;
			}
			mValuePtr = nullptr;
		}

		void _Check() const
		{
			MOMO_CHECK(mValuePtr != nullptr);
			MOMO_CHECK(*mHashMultiMapVersion == mVersion);
		}

	private:
		KeyIterator mKeyIterator;
		Value* mValuePtr;
		const size_t* mHashMultiMapVersion;
		size_t mVersion;
	};
}

template<typename TKey, typename TValue>
struct HashMultiMapKeyValueTraits
{
	typedef TKey Key;
	typedef TValue Value;

	typedef internal::ObjectManager<Key> KeyManager;
	typedef internal::ObjectManager<Value> ValueManager;

	static const size_t keyAlignment = KeyManager::alignment;
	static const size_t valueAlignment = ValueManager::alignment;

	static const bool isKeyNothrowRelocatable = KeyManager::isNothrowRelocatable;

	typedef typename ValueManager::MoveCreator MoveValueCreator;
	typedef typename ValueManager::CopyCreator CopyValueCreator;

	static void CreateKey(const Key& key, void* pkey)
	{
		KeyManager::Create(key, pkey);
	}

	template<typename ValueCreator>
	static void CreatePair(Key&& key, const ValueCreator& valueCreator,
		void* pkey, void* pvalue)
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

	static void DestroyValues(Value* values, size_t count) MOMO_NOEXCEPT
	{
		ValueManager::Destroy(values, count);
	}

	static void RelocateKeyNothrow(Key& srcKey, Key* dstKey) MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT(isKeyNothrowRelocatable);
		KeyManager::Relocate(std::addressof(srcKey), dstKey, 1);
	}

	static void AssignKey(Key&& srcKey, Key& dstKey)
	{
		dstKey = std::move(srcKey);
	}

	static void AssignValue(Value&& srcValue, Value& dstValue)
	{
		dstValue = std::move(srcValue);
	}

	template<typename ValueCreator>
	static void RelocateAddBackValue(Value* srcValues, Value* dstValues, size_t srcCount,
		const ValueCreator& valueCreator)
	{
		ValueManager::RelocateAddBack(srcValues, dstValues, srcCount, valueCreator);
	}
};

struct HashMultiMapSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	//? ValueArray
};

template<typename TKey, typename TValue,
	typename THashTraits = HashTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TKeyValueTraits = HashMultiMapKeyValueTraits<TKey, TValue>,
	typename TSettings = HashMultiMapSettings>
class HashMultiMap
{
public:
	typedef TKey Key;
	typedef TValue Value;
	typedef THashTraits HashTraits;
	typedef TMemManager MemManager;
	typedef TKeyValueTraits KeyValueTraits;
	typedef TSettings Settings;

private:
	struct ArrayBucketItemTraits
	{
		typedef typename HashMultiMap::Value Item;

		static const size_t alignment = KeyValueTraits::valueAlignment;

		static void Create(const Item& item, void* pitem)
		{
			(typename KeyValueTraits::CopyValueCreator(item))(pitem);
		}

		static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
		{
			KeyValueTraits::DestroyValues(items, count);
		}

		template<typename ItemCreator>
		static void RelocateAddBack(Item* srcItems, Item* dstItems, size_t srcCount,
			const ItemCreator& itemCreator)
		{
			KeyValueTraits::RelocateAddBackValue(srcItems, dstItems, srcCount, itemCreator);
		}
	};

	struct ValueArraySettings : public ArraySettings<>
	{
		static const CheckMode checkMode = CheckMode::assertion;
	};

	typedef internal::ArrayBucket<ArrayBucketItemTraits, MemManager, 7,
		MemPoolConst::defaultBlockCount, ValueArraySettings> ValueArray;

	typedef typename ValueArray::Params ValueArrayParams;

	class ValueCrew
	{
	private:
		struct Data
		{
			size_t valueVersion;
			ValueArrayParams valueArrayParams;
		};

	public:
		ValueCrew(MemManager& memManager)
		{
			mData = (Data*)memManager.Allocate(sizeof(Data));
			mData->valueVersion = 0;
			try
			{
				new(&mData->valueArrayParams) ValueArrayParams(memManager);
			}
			catch (...)
			{
				memManager.Deallocate(mData, sizeof(Data));
				throw;
			}
		}

		ValueCrew(ValueCrew&& crew) MOMO_NOEXCEPT
			: mData(nullptr)
		{
			Swap(crew);
		}

		~ValueCrew() MOMO_NOEXCEPT
		{
			assert(IsNull());
		}

		ValueCrew& operator=(ValueCrew&& crew) MOMO_NOEXCEPT
		{
			ValueCrew(std::move(crew)).Swap(*this);
			return *this;
		}

		void Swap(ValueCrew& crew) MOMO_NOEXCEPT
		{
			std::swap(mData, crew.mData);
		}

		void Destroy(MemManager& memManager) MOMO_NOEXCEPT
		{
			assert(!IsNull());
			mData->~Data();
			memManager.Deallocate(mData, sizeof(Data));
			mData = nullptr;
		}

		bool IsNull() const MOMO_NOEXCEPT
		{
			return mData == nullptr;
		}

		const size_t& GetValueVersion() const MOMO_NOEXCEPT
		{
			assert(!IsNull());
			return mData->valueVersion;
		}

		size_t& GetValueVersion() MOMO_NOEXCEPT
		{
			assert(!IsNull());
			return mData->valueVersion;
		}

		const ValueArrayParams& GetValueArrayParams() const MOMO_NOEXCEPT
		{
			assert(!IsNull());
			return mData->valueArrayParams;
		}

		ValueArrayParams& GetValueArrayParams() MOMO_NOEXCEPT
		{
			assert(!IsNull());
			return mData->valueArrayParams;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(ValueCrew);
		MOMO_DISABLE_COPY_OPERATOR(ValueCrew);

	private:
		Data* mData;
	};

	struct HashMapKeyValueTraits
	{
		typedef typename HashMultiMap::Key Key;
		typedef typename HashMultiMap::ValueArray Value;

		typedef internal::ObjectManager<Value> ValueManager;	//?

		static const bool isKeyNothrowRelocatable = KeyValueTraits::isKeyNothrowRelocatable;
		static const bool isValueNothrowRelocatable = ValueManager::isNothrowRelocatable;

		static const size_t keyAlignment = KeyValueTraits::keyAlignment;
		static const size_t valueAlignment = ValueManager::alignment;

		typedef typename ValueManager::MoveCreator MoveValueCreator;

		static void CreateKey(const Key& key, void* pkey)
		{
			MOMO_STATIC_ASSERT(!isKeyNothrowRelocatable);
			KeyValueTraits::CreateKey(key, pkey);
		}

		template<typename ValueCreator>
		static void CreatePair(Key&& key, const ValueCreator& valueCreator,
			void* pkey, void* pvalue)
		{
			KeyValueTraits::CreatePair(std::move(key), valueCreator, pkey, pvalue);
		}

		template<typename ValueCreator>
		static void CreatePair(const Key& key, const ValueCreator& valueCreator,
			void* pkey, void* pvalue)
		{
			KeyValueTraits::CreatePair(key, valueCreator, pkey, pvalue);
		}

		static void DestroyKey(Key& key) MOMO_NOEXCEPT
		{
			KeyValueTraits::DestroyKey(key);
		}

		static void DestroyValue(Value& value) MOMO_NOEXCEPT
		{
			ValueManager::Destroy(value);
		}

		static void RelocateKeyNothrow(Key& srcKey, Key* dstKey) MOMO_NOEXCEPT
		{
			KeyValueTraits::RelocateKeyNothrow(srcKey, dstKey);
		}

		static void RelocateValueNothrow(Value& srcValue, Value* dstValue) MOMO_NOEXCEPT
		{
			ValueManager::Relocate(std::addressof(srcValue), dstValue, 1);
		}

		static void AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue)
		{
			KeyValueTraits::AssignKey(std::move(srcKey), dstKey);
			dstValue = std::move(srcValue);
		}
	};

	struct HashMapSettings : public momo::HashMapSettings
	{
		static const CheckMode checkMode = Settings::checkMode;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
	};

	typedef momo::HashMap<Key, ValueArray, HashTraits, MemManager,
		HashMapKeyValueTraits, HashMapSettings> HashMap;

	typedef typename HashMap::Iterator HashMapIterator;
	typedef typename HashMapIterator::Reference HashMapReference;

public:
	typedef typename ValueArray::ConstBounds ConstValueBounds;
	typedef typename ValueArray::Bounds ValueBounds;

private:
	typedef internal::HashMultiMapKeyReference<Key, ValueBounds, HashMapReference> KeyReference;

public:
	typedef internal::HashDerivedIterator<HashMapIterator, KeyReference> KeyIterator;
	typedef typename KeyIterator::ConstIterator ConstKeyIterator;

	typedef internal::HashMultiMapKeyBounds<KeyIterator> KeyBounds;
	typedef typename KeyBounds::ConstBounds ConstKeyBounds;

	typedef internal::HashMultiMapIterator<KeyIterator, Value, Settings> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

public:
	explicit HashMultiMap(const HashTraits& hashTraits = HashTraits(),
		MemManager&& memManager = MemManager())
		: mHashMap(hashTraits, std::move(memManager)),
		mValueCount(0),
		mValueCrew(GetMemManager())
	{
	}

	HashMultiMap(HashMultiMap&& hashMultiMap) MOMO_NOEXCEPT
		: mHashMap(std::move(hashMultiMap.mHashMap)),
		mValueCount(hashMultiMap.mValueCount),
		mValueCrew(std::move(hashMultiMap.mValueCrew))
	{
		hashMultiMap.mValueCount = 0;
	}

	HashMultiMap(const HashMultiMap& hashMultiMap)
		: mHashMap(hashMultiMap.GetHashTraits(), MemManager(hashMultiMap.GetMemManager())),
		mValueCount(hashMultiMap.mValueCount),
		mValueCrew(GetMemManager())
	{
		ValueArrayParams& valueArrayParams = mValueCrew.GetValueArrayParams();
		try
		{
			mHashMap.Reserve(hashMultiMap.mHashMap.GetCount());
			for (typename HashMap::ConstIterator::Reference ref : hashMultiMap.mHashMap)
				mHashMap.Insert(ref.key, ValueArray(valueArrayParams, ref.value));
		}
		catch (...)
		{
			for (typename HashMap::Iterator::Reference ref : mHashMap)
				ref.value.Clear(valueArrayParams);
			mValueCrew.Destroy(GetMemManager());
			throw;
		}
	}

	~HashMultiMap() MOMO_NOEXCEPT
	{
		if (mValueCrew.IsNull())	//?
			return;
		ValueArrayParams& valueArrayParams = mValueCrew.GetValueArrayParams();
		for (typename HashMap::Iterator::Reference ref : mHashMap)
			ref.value.Clear(valueArrayParams);
		mValueCrew.Destroy(GetMemManager());
	}

	HashMultiMap& operator=(HashMultiMap&& hashMultiMap) MOMO_NOEXCEPT
	{
		HashMultiMap(std::move(hashMultiMap)).Swap(*this);
		return *this;
	}

	HashMultiMap& operator=(const HashMultiMap& hashMultiMap)
	{
		if (this != &hashMultiMap)
			HashMultiMap(hashMultiMap).Swap(*this);
		return *this;
	}

	void Swap(HashMultiMap& hashMultiMap) MOMO_NOEXCEPT
	{
		mHashMap.Swap(hashMultiMap.mHashMap);
		std::swap(mValueCount, hashMultiMap.mValueCount);
		mValueCrew.Swap(hashMultiMap.mValueCrew);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		return _MakeIterator<ConstIterator>(GetKeyBounds().GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return _MakeIterator<Iterator>(GetKeyBounds().GetBegin());
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator();
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator();
	}

	MOMO_FRIEND_SWAP(HashMultiMap)
	MOMO_FRIENDS_BEGIN_END(const HashMultiMap&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(HashMultiMap&, Iterator)

	const HashTraits& GetHashTraits() const MOMO_NOEXCEPT
	{
		return mHashMap.GetHashTraits();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mHashMap.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mHashMap.GetMemManager();
	}

	size_t GetValueCount() const MOMO_NOEXCEPT
	{
		return mValueCount;
	}

	void Clear() MOMO_NOEXCEPT
	{
		if (mValueCrew.IsNull())	//?
			return;
		ValueArrayParams& valueArrayParams = mValueCrew.GetValueArrayParams();
		for (typename HashMap::Iterator::Reference ref : mHashMap)
			ref.value.Clear(valueArrayParams);
		mHashMap.Clear();
		mValueCount = 0;
		++mValueCrew.GetValueVersion();
	}

	ConstKeyBounds GetKeyBounds() const MOMO_NOEXCEPT
	{
		return ConstKeyBounds(ConstKeyIterator(mHashMap.GetBegin()));
	}

	KeyBounds GetKeyBounds() MOMO_NOEXCEPT
	{
		return KeyBounds(KeyIterator(mHashMap.GetBegin()));
	}

	size_t GetKeyCount() const MOMO_NOEXCEPT
	{
		return mHashMap.GetCount();
	}

	void Shrink()
	{
		HashMultiMap(*this).Swap(*this);
	}

	ConstKeyIterator Find(const Key& key) const
	{
		return ConstKeyIterator(mHashMap.Find(key));
	}

	KeyIterator Find(const Key& key)
	{
		return KeyIterator(mHashMap.Find(key));
	}

	bool HasKey(const Key& key) const
	{
		return mHashMap.HasKey(key);
	}

	template<typename ValueCreator>
	Iterator AddCrt(Key&& key, const ValueCreator& valueCreator)
	{
		return _Add(std::move(key), valueCreator);
	}

	Iterator Add(Key&& key, Value&& value)
	{
		return _Add(std::move(key), typename KeyValueTraits::MoveValueCreator(std::move(value)));
	}

	Iterator Add(Key&& key, const Value& value)
	{
		return _Add(std::move(key), typename KeyValueTraits::CopyValueCreator(value));
	}

	template<typename ValueCreator>
	Iterator AddCrt(const Key& key, const ValueCreator& valueCreator)
	{
		return _Add(key, valueCreator);
	}

	Iterator Add(const Key& key, Value&& value)
	{
		return _Add(key, typename KeyValueTraits::MoveValueCreator(std::move(value)));
	}

	Iterator Add(const Key& key, const Value& value)
	{
		return _Add(key, typename KeyValueTraits::CopyValueCreator(value));
	}

	template<typename ValueCreator>
	Iterator AddCrt(KeyIterator keyIter, const ValueCreator& valueCreator)
	{
		ValueArray& valueArray = keyIter.GetBaseIterator()->value;
		_AddValue(valueArray, valueCreator);
		return _MakeIterator<Iterator>(keyIter, valueArray.GetBounds().GetEnd() - 1, false);
	}

	Iterator Add(KeyIterator keyIter, Value&& value)
	{
		return AddCrt(keyIter, typename KeyValueTraits::MoveValueCreator(std::move(value)));
	}

	Iterator Add(KeyIterator keyIter, const Value& value)
	{
		return AddCrt(keyIter, typename KeyValueTraits::CopyValueCreator(value));
	}

	template<typename Iterator>
	void AddKV(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->key);
		MOMO_CHECK_TYPE(Value, begin->value);
		for (Iterator iter = begin; iter != end; ++iter)
			Add(iter->key, iter->value);
	}

	template<typename Iterator>
	void AddFS(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->first);
		MOMO_CHECK_TYPE(Value, begin->second);
		for (Iterator iter = begin; iter != end; ++iter)
			Add(iter->first, iter->second);
	}

#ifdef MOMO_USE_INIT_LISTS
	void Add(std::initializer_list<std::pair<Key, Value>> keyValuePairs)
	{
		AddFS(keyValuePairs.begin(), keyValuePairs.end());
	}
#endif

	KeyIterator InsertKey(Key&& key)
	{
		return KeyIterator(mHashMap.Insert(std::move(key), ValueArray()).iterator);
	}

	KeyIterator InsertKey(const Key& key)
	{
		return KeyIterator(mHashMap.Insert(key, ValueArray()).iterator);
	}

	Iterator Remove(KeyIterator keyIter, size_t valueIndex)
	{
		ValueBounds valueBounds = keyIter->values;
		MOMO_CHECK(valueIndex < valueBounds.GetCount());
		Iterator iter = _MakeIterator<Iterator>(keyIter,
			valueBounds.GetBegin() + valueIndex, false);
		return Remove(iter);
	}

	Iterator Remove(Iterator iter)
	{
		iter.Check(mValueCrew.GetValueVersion());
		KeyIterator keyIter = iter.GetKeyIterator();
		ValueArray& valueArray = keyIter.GetBaseIterator()->value;
		Value& value = iter->value;
		typename ValueArray::Bounds valueBounds = valueArray.GetBounds();
		size_t valueIndex = std::addressof(value) - valueBounds.GetBegin();
		KeyValueTraits::AssignValue(std::move(*(valueBounds.GetEnd() - 1)), value);
		valueArray.RemoveBack(mValueCrew.GetValueArrayParams());
		--mValueCount;
		++mValueCrew.GetValueVersion();
		return _MakeIterator<Iterator>(keyIter,
			valueArray.GetBounds().GetBegin() + valueIndex, true);
	}

	Iterator RemoveValues(KeyIterator keyIter)
	{
		ValueArray& valueArray = keyIter.GetBaseIterator()->value;
		_RemoveValues(valueArray);
		return _MakeIterator<Iterator>(std::next(keyIter));
	}

	Iterator RemoveKey(KeyIterator keyIter)
	{
		ValueArray& valueArray = keyIter.GetBaseIterator()->value;
		ValueArray tempValueArray(std::move(valueArray));
		HashMapIterator hashMapIter;
		try
		{
			hashMapIter = mHashMap.Remove(keyIter.GetBaseIterator());
		}
		catch (...)
		{
			valueArray = std::move(tempValueArray);
			throw;
		}
		_RemoveValues(tempValueArray);
		return _MakeIterator<Iterator>(KeyIterator(hashMapIter));
	}

	size_t RemoveKey(const Key& key)
	{
		KeyIterator keyIter = Find(key);
		if (!keyIter)
			return 0;
		size_t valueCount = keyIter->values.GetCount();
		RemoveKey(keyIter);
		return valueCount;
	}

	ConstIterator MakeIterator(ConstKeyIterator keyIter, size_t valueIndex) const	//?
	{
		ConstValueBounds valueBounds = keyIter->values;
		MOMO_CHECK(valueIndex < valueBounds.GetCount());
		return _MakeIterator<ConstIterator>(keyIter, valueBounds.GetBegin() + valueIndex, false);
	}

	Iterator MakeIterator(KeyIterator keyIter, size_t valueIndex)	//?
	{
		ValueBounds valueBounds = keyIter->values;
		MOMO_CHECK(valueIndex < valueBounds.GetCount());
		return _MakeIterator<Iterator>(keyIter, valueBounds.GetBegin() + valueIndex, false);
	}

private:
	template<typename Iterator, typename KeyIterator>
	Iterator _MakeIterator(KeyIterator keyIter) const MOMO_NOEXCEPT
	{
		if (!keyIter)
			return Iterator();
		return _MakeIterator<Iterator>(keyIter, keyIter->values.GetBegin(), true);
	}

	template<typename Iterator, typename KeyIterator, typename Value>
	Iterator _MakeIterator(KeyIterator keyIter, Value* pvalue, bool move) const MOMO_NOEXCEPT
	{
		return Iterator(keyIter, pvalue, mValueCrew.GetValueVersion(), move);
	}

	template<typename RKey, typename ValueCreator>
	Iterator _Add(RKey&& key, const ValueCreator& valueCreator)
	{
		KeyIterator keyIter = Find((const Key&)key);
		if (!!keyIter)
			return AddCrt(keyIter, valueCreator);
		auto keyValuesCreator = [this, &key, &valueCreator] (void* pkey, void* pvalues)
		{
			auto keyValueCreator = [&key, &valueCreator, pkey] (void* pvalue)
			{
				KeyValueTraits::CreatePair(std::forward<RKey>(key),
					valueCreator, pkey, pvalue);
			};
			ValueArray valueArray;
			this->_AddValue(valueArray, keyValueCreator);
			new(pvalues) ValueArray(std::move(valueArray));
		};
		keyIter = KeyIterator(mHashMap.AddCrt(keyIter.GetBaseIterator(), keyValuesCreator));
		return _MakeIterator<Iterator>(keyIter, keyIter->values.GetBegin(), false);
	}

	template<typename ValueCreator>
	void _AddValue(ValueArray& valueArray, const ValueCreator& valueCreator)
	{
		valueArray.AddBackCrt(mValueCrew.GetValueArrayParams(), valueCreator);
		++mValueCount;
		++mValueCrew.GetValueVersion();
	}

	void _RemoveValues(ValueArray& valueArray) MOMO_NOEXCEPT
	{
		mValueCount -= valueArray.GetBounds().GetCount();
		++mValueCrew.GetValueVersion();
		valueArray.Clear(mValueCrew.GetValueArrayParams());
	}

private:
	HashMap mHashMap;
	size_t mValueCount;
	ValueCrew mValueCrew;
};

} // namespace momo

namespace std
{
	template<typename KI, typename V, typename S>
	struct iterator_traits<momo::internal::HashMultiMapIterator<KI, V, S>>
	{
		typedef forward_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::HashMultiMapIterator<KI, V, S>::Pointer pointer;
		typedef typename momo::internal::HashMultiMapIterator<KI, V, S>::Reference reference;
		typedef reference value_type;	//?
	};
} // namespace std
