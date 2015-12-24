/**********************************************************\

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
	class HashMultiMapIterator : private IteratorVersion<TSettings::checkValueVersion>
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

		typedef IteratorPointer<Reference> Pointer;

	private:
		typedef internal::IteratorVersion<Settings::checkValueVersion> IteratorVersion;

	public:
		HashMultiMapIterator() MOMO_NOEXCEPT
			: mValuePtr(nullptr)
		{
		}

		HashMultiMapIterator(KeyIterator keyIter, Value* pvalue, const size_t& version,
			bool move) MOMO_NOEXCEPT
			: IteratorVersion(version),
			mKeyIterator(keyIter),
			mValuePtr(pvalue)
		{
			if (move)
				_Move();
		}

		HashMultiMapIterator(KeyIterator keyIter, Value* pvalue,
			IteratorVersion version) MOMO_NOEXCEPT
			: IteratorVersion(version),
			mKeyIterator(keyIter),
			mValuePtr(pvalue)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mKeyIterator, mValuePtr, *this);
		}

		HashMultiMapIterator& operator++()
		{
			MOMO_CHECK(mValuePtr != nullptr);
			MOMO_CHECK(IteratorVersion::Check());
			++mValuePtr;
			_Move();
			return *this;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mValuePtr != nullptr);
			MOMO_CHECK(IteratorVersion::Check());
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
			MOMO_CHECK(mValuePtr != nullptr);
			MOMO_CHECK(IteratorVersion::Check(version));
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

	private:
		KeyIterator mKeyIterator;
		Value* mValuePtr;
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

	static const bool isKeyNothrowMoveConstructible = KeyManager::isNothrowMoveConstructible;
	static const bool isKeyNothrowRelocatable = KeyManager::isNothrowRelocatable;

	template<typename... ValueArgs>
	using ValueCreator = typename ValueManager::template Creator<ValueArgs...>;

	static void CreateKeyNothrow(Key&& key, void* pkey) MOMO_NOEXCEPT
	{
		KeyManager::CreateNothrow(std::move(key), pkey);
	}

	static void CreateKey(const Key& key, void* pkey)
	{
		KeyManager::Create(key, pkey);
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

	static void RelocateValues(Value* srcValues, Value* dstValues, size_t count)
	{
		ValueManager::Relocate(srcValues, dstValues, count);
	}

	template<typename ValueCreator>
	static void RelocateCreateValues(Value* srcValues, Value* dstValues, size_t count,
		const ValueCreator& valueCreator, void* pvalue)
	{
		ValueManager::RelocateCreate(srcValues, dstValues, count, valueCreator, pvalue);
	}
};

struct HashMultiMapSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION_VALUE;
	static const bool checkValueVersion = MOMO_CHECK_ITERATOR_VERSION_VALUE;
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
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	struct ArrayBucketItemTraits
	{
		typedef typename HashMultiMap::Value Item;

		static const size_t alignment = KeyValueTraits::valueAlignment;

		static void Create(const Item& item, void* pitem)
		{
			(ValueCreator<const Item&>(item))(pitem);
		}

		static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
		{
			KeyValueTraits::DestroyValues(items, count);
		}

		static void Relocate(Item* srcItems, Item* dstItems, size_t count)
		{
			KeyValueTraits::RelocateValues(srcItems, dstItems, count);
		}

		template<typename ItemCreator>
		static void RelocateCreate(Item* srcItems, Item* dstItems, size_t count,
			const ItemCreator& itemCreator, void* pitem)
		{
			KeyValueTraits::RelocateCreateValues(srcItems, dstItems, count, itemCreator, pitem);
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

		ValueCrew(const ValueCrew&) = delete;

		~ValueCrew() MOMO_NOEXCEPT
		{
			assert(IsNull());
		}

		ValueCrew& operator=(ValueCrew&& crew) MOMO_NOEXCEPT
		{
			ValueCrew(std::move(crew)).Swap(*this);
			return *this;
		}

		ValueCrew& operator=(const ValueCrew&) = delete;

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
		Data* mData;
	};

	struct HashMapKeyValueTraits
	{
		typedef typename HashMultiMap::Key Key;
		typedef typename HashMultiMap::ValueArray Value;

		typedef internal::ObjectManager<Value> ValueManager;	//?

		static const bool isKeyNothrowMoveConstructible = KeyValueTraits::isKeyNothrowMoveConstructible;
		static const bool isKeyNothrowRelocatable = KeyValueTraits::isKeyNothrowRelocatable;
		static const bool isValueNothrowRelocatable = ValueManager::isNothrowRelocatable;

		static const size_t keyAlignment = KeyValueTraits::keyAlignment;
		static const size_t valueAlignment = ValueManager::alignment;

		template<typename ValueArg>
		class ValueCreator : public ValueManager::template Creator<ValueArg>
		{
			MOMO_STATIC_ASSERT((std::is_same<ValueArg, Value>::value));

		private:
			typedef typename ValueManager::template Creator<ValueArg> BaseCreator;

		public:
			//using BaseCreator::BaseCreator;	// vs2013
			explicit ValueCreator(ValueArg&& valueArg)
				: BaseCreator(std::forward<ValueArg>(valueArg))
			{
			}
		};

		static void CreateKeyNothrow(Key&& key, void* pkey) MOMO_NOEXCEPT
		{
			KeyValueTraits::CreateKeyNothrow(std::move(key), pkey);
		}

		static void CreateKey(const Key& key, void* pkey)
		{
			KeyValueTraits::CreateKey(key, pkey);
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
		static const bool checkVersion = Settings::checkVersion;
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

	HashMultiMap(std::initializer_list<std::pair<Key, Value>> keyValuePairs,
		const HashTraits& hashTraits = HashTraits(), MemManager&& memManager = MemManager())
		: HashMultiMap(hashTraits, std::move(memManager))
	{
		try
		{
			Add(keyValuePairs);
		}
		catch (...)
		{
			_ClearValueArrays();
			mValueCrew.Destroy(GetMemManager());
			throw;
		}
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
		try
		{
			ValueArrayParams& valueArrayParams = mValueCrew.GetValueArrayParams();
			mHashMap.Reserve(hashMultiMap.mHashMap.GetCount());
			for (typename HashMap::ConstIterator::Reference ref : hashMultiMap.mHashMap)
				mHashMap.Insert(ref.key, ValueArray(valueArrayParams, ref.value));
		}
		catch (...)
		{
			_ClearValueArrays();
			mValueCrew.Destroy(GetMemManager());
			throw;
		}
	}

	~HashMultiMap() MOMO_NOEXCEPT
	{
		if (mValueCrew.IsNull())
			return;
		_ClearValueArrays();
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
		if (mValueCrew.IsNull())
			return;
		_ClearValueArrays();
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

	template<typename... ValueArgs>
	Iterator AddVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return _Add(std::move(key),
			ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(Key&& key, Value&& value)
	{
		return AddVar(std::move(key), std::move(value));
	}

	Iterator Add(Key&& key, const Value& value)
	{
		return AddVar(std::move(key), value);
	}

	template<typename ValueCreator>
	Iterator AddCrt(const Key& key, const ValueCreator& valueCreator)
	{
		return _Add(key, valueCreator);
	}

	template<typename... ValueArgs>
	Iterator AddVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return _Add(key, ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(const Key& key, Value&& value)
	{
		return AddVar(key, std::move(value));
	}

	Iterator Add(const Key& key, const Value& value)
	{
		return AddVar(key, value);
	}

	template<typename ValueCreator>
	Iterator AddCrt(KeyIterator keyIter, const ValueCreator& valueCreator)
	{
		ValueArray& valueArray = keyIter.GetBaseIterator()->value;
		_AddValue(valueArray, valueCreator);
		return _MakeIterator<Iterator>(keyIter, valueArray.GetBounds().GetEnd() - 1, false);
	}

	template<typename... ValueArgs>
	Iterator AddVar(KeyIterator keyIter, ValueArgs&&... valueArgs)
	{
		return AddCrt(keyIter, ValueCreator<ValueArgs...>(std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(KeyIterator keyIter, Value&& value)
	{
		return AddVar(keyIter, std::move(value));
	}

	Iterator Add(KeyIterator keyIter, const Value& value)
	{
		return AddVar(keyIter, value);
	}

	template<typename Iterator>
	void AddKV(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->key);
		for (Iterator iter = begin; iter != end; ++iter)
			AddVar(iter->key, iter->value);
	}

	template<typename Iterator>
	void AddFS(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Key, begin->first);
		for (Iterator iter = begin; iter != end; ++iter)
			AddVar(iter->first, iter->second);
	}

	void Add(std::initializer_list<std::pair<Key, Value>> keyValuePairs)
	{
		AddFS(keyValuePairs.begin(), keyValuePairs.end());
	}

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
		valueArray.Shrink(mValueCrew.GetValueArrayParams());
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
	void _ClearValueArrays() MOMO_NOEXCEPT
	{
		ValueArrayParams& valueArrayParams = mValueCrew.GetValueArrayParams();
		for (typename HashMap::Iterator::Reference ref : mHashMap)
			ref.value.Clear(valueArrayParams);
	}

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
		auto valuesCreator = [this, &valueCreator] (void* pvalues)
		{
			ValueArray valueArray;
			this->_AddValue(valueArray, valueCreator);
			new(pvalues) ValueArray(std::move(valueArray));
		};
		keyIter = KeyIterator(mHashMap.AddCrt(keyIter.GetBaseIterator(),
			std::forward<RKey>(key), valuesCreator));
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
