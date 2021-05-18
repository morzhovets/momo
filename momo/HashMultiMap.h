/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashMultiMap.h

  namespace momo:
    class HashMultiMapKeyValueTraits
    class HashMultiMapSettings
    class HashMultiMap
    class HashMultiMapOpen

\**********************************************************/

#pragma once

#include "HashMap.h"
#include "details/ArrayBucket.h"

namespace momo
{

namespace internal
{
	template<typename THashMapReference>
	class HashMultiMapKeyReference
	{
	protected:
		typedef THashMapReference HashMapReference;

		typedef decltype(std::declval<HashMapReference>().value.GetBounds()) Values;

	public:
		typedef typename HashMapReference::Key Key;

		typedef typename Values::Iterator Iterator;

		typedef HashMultiMapKeyReference<typename HashMapReference::ConstReference> ConstReference;

	private:
		struct ConstReferenceProxy : public ConstReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstReference)
		};

	public:
		HashMultiMapKeyReference() = delete;

		operator ConstReference() const noexcept
		{
			return ConstReferenceProxy(key, mValues);
		}

		Iterator GetBegin() const noexcept
		{
			return mValues.GetBegin();
		}

		Iterator GetEnd() const noexcept
		{
			return mValues.GetEnd();
		}

		MOMO_FRIENDS_SIZE_BEGIN_END_CONST(HashMultiMapKeyReference, Iterator)

		size_t GetCount() const noexcept
		{
			return mValues.GetCount();
		}

		typename std::iterator_traits<Iterator>::reference operator[](size_t index) const
		{
			//MOMO_CHECK(index < GetCount());	//?
			MOMO_ASSERT(index < GetCount());
			return mValues[index];
		}

	protected:
		explicit HashMultiMapKeyReference(const Key& key, Values values) noexcept
			: key(key),
			mValues(values)
		{
		}

		explicit HashMultiMapKeyReference(HashMapReference hashMapRef) noexcept
			: key(hashMapRef.key),
			mValues(hashMapRef.value.GetBounds())
		{
		}

	public:
		const Key& key;

	private:
		const Values mValues;
	};

	template<typename TKeyIterator>
	class HashMultiMapKeyBounds
	{
	protected:
		typedef TKeyIterator KeyIterator;

	public:
		typedef KeyIterator Iterator;

		typedef HashMultiMapKeyBounds<typename KeyIterator::ConstIterator> ConstBounds;

	public:
		explicit HashMultiMapKeyBounds() noexcept
			: mCount(0)
		{
		}

		operator ConstBounds() const noexcept
		{
			return ConstBounds(mBegin);
		}

		Iterator GetBegin() const noexcept
		{
			return mBegin;
		}

		Iterator GetEnd() const noexcept
		{
			return KeyIterator();
		}

		MOMO_FRIENDS_SIZE_BEGIN_END_CONST(HashMultiMapKeyBounds, KeyIterator)

		size_t GetCount() const noexcept	//?
		{
			return mCount;
		}

	protected:
		explicit HashMultiMapKeyBounds(KeyIterator begin, size_t count) noexcept
			: mBegin(begin),
			mCount(count)
		{
		}

	private:
		KeyIterator mBegin;
		size_t mCount;
	};

	template<typename TKey, typename TValue>
	class HashMultiMapReference
	{
	public:
		typedef TKey Key;
		typedef TValue Value;

		typedef HashMultiMapReference<Key, const Value> ConstReference;

	public:
		explicit HashMultiMapReference(const Key& key, Value& value) noexcept
			: key(key),
			value(value)
		{
		}

		operator ConstReference() const noexcept
		{
			return ConstReference(key, value);
		}

	public:
		const Key& key;
		Value& value;
	};

	template<typename TKeyIterator, typename TSettings>
	class HashMultiMapIterator : private VersionKeeper<TSettings, TSettings::checkValueVersion>
	{
	public:
		typedef TKeyIterator KeyIterator;

	protected:
		typedef TSettings Settings;

		typedef internal::VersionKeeper<Settings, Settings::checkValueVersion> VersionKeeper;

	private:
		typedef typename KeyIterator::Reference KeyReference;
		typedef typename KeyReference::Key Key;

	public:
		typedef typename KeyReference::Iterator ValueIterator;

	private:
		typedef typename std::iterator_traits<ValueIterator>::reference ValueReference;
		typedef typename std::remove_reference<ValueReference>::type Value;

	public:
		typedef HashMultiMapReference<Key, Value> Reference;
		typedef IteratorPointer<Reference> Pointer;

		typedef HashMultiMapIterator<typename KeyIterator::ConstIterator,
			Settings> ConstIterator;

	private:
		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit HashMultiMapIterator() noexcept
			: mValueIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mKeyIterator, mValueIterator, *this);
		}

		HashMultiMapIterator& operator++()
		{
			VersionKeeper::Check();
			MOMO_CHECK(mValueIterator != ValueIterator());
			++mValueIterator;
			pvMove();
			return *this;
		}

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(mValueIterator != ValueIterator());
			return Pointer(Reference(mKeyIterator->key, *mValueIterator));
		}

		friend bool operator==(HashMultiMapIterator iter1, HashMultiMapIterator iter2) noexcept
		{
			return iter1.mValueIterator == iter2.mValueIterator;
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashMultiMapIterator)

		KeyIterator GetKeyIterator() const noexcept
		{
			return mKeyIterator;
		}

		ValueIterator GetValueIterator() const noexcept
		{
			return mValueIterator;
		}

	protected:
		explicit HashMultiMapIterator(KeyIterator keyIter, ValueIterator valueIter,
			const size_t& version, bool move) noexcept
			: VersionKeeper(&version),
			mKeyIterator(keyIter),
			mValueIterator(valueIter)
		{
			if (move)
				pvMove();
		}

		explicit HashMultiMapIterator(KeyIterator keyIter, ValueIterator valueIter,
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mKeyIterator(keyIter),
			mValueIterator(valueIter)
		{
		}

		void ptCheck(const size_t& version) const
		{
			VersionKeeper::Check(&version);
			MOMO_CHECK(mValueIterator != ValueIterator());
		}

	private:
		void pvMove() noexcept
		{
			if (mValueIterator != mKeyIterator->GetEnd())
				return;
			++mKeyIterator;
			for (; !!mKeyIterator; ++mKeyIterator)
			{
				mValueIterator = mKeyIterator->GetBegin();
				if (mValueIterator != mKeyIterator->GetEnd())
					return;
			}
			mValueIterator = ValueIterator();
		}

	private:
		KeyIterator mKeyIterator;
		ValueIterator mValueIterator;
	};

	template<typename THashMultiMapKeyValueTraits>
	class HashMultiMapArrayBucketItemTraits
	{
	protected:
		typedef THashMultiMapKeyValueTraits HashMultiMapKeyValueTraits;

	public:
		typedef typename HashMultiMapKeyValueTraits::Value Item;
		typedef typename HashMultiMapKeyValueTraits::MemManager MemManager;

		static const size_t alignment = HashMultiMapKeyValueTraits::valueAlignment;

		static const bool isTriviallyRelocatable =
			HashMultiMapKeyValueTraits::isValueTriviallyRelocatable;

	public:
		static void Copy(MemManager& memManager, const Item& srcItem, Item* dstItem)
		{
			typename HashMultiMapKeyValueTraits::template ValueCreator<const Item&>(
				memManager, srcItem)(dstItem);
		}

		static void Destroy(MemManager& memManager, Item* items, size_t count) noexcept
		{
			HashMultiMapKeyValueTraits::DestroyValues(memManager, items, count);
		}

		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			HashMultiMapKeyValueTraits::RelocateCreateValues(memManager, srcItems, dstItems,
				count, std::forward<ItemCreator>(itemCreator), newItem);
		}
	};

	template<typename THashMultiMapKeyValueTraits, typename THashMultiMapSettings>
	class HashMultiMapNestedMapKeyValueTraits
	{
	protected:
		typedef THashMultiMapKeyValueTraits HashMultiMapKeyValueTraits;
		typedef THashMultiMapSettings HashMultiMapSettings;

	public:
		typedef typename HashMultiMapKeyValueTraits::Key Key;
		typedef typename HashMultiMapKeyValueTraits::MemManager MemManager;

		typedef ArrayBucket<HashMultiMapArrayBucketItemTraits<HashMultiMapKeyValueTraits>,
			HashMultiMapSettings::valueArrayMaxFastCount,
			typename HashMultiMapSettings::ValueArrayMemPoolParams,
			typename HashMultiMapSettings::ValueArraySettings> Value;

	private:
		typedef ObjectManager<Value, MemManager> ValueManager;

	public:
		static const size_t keyAlignment = HashMultiMapKeyValueTraits::keyAlignment;
		static const size_t valueAlignment = ValueManager::alignment;

		static const bool isKeyNothrowRelocatable =
			HashMultiMapKeyValueTraits::isKeyNothrowRelocatable;
		static const bool isValueNothrowRelocatable = ValueManager::isNothrowRelocatable;

		template<typename ValueArg>
		class ValueCreator : public ValueManager::template Creator<ValueArg>
		{
			MOMO_STATIC_ASSERT((std::is_same<ValueArg, Value>::value));

		private:
			typedef typename ValueManager::template Creator<ValueArg> BaseCreator;

		public:
			using BaseCreator::BaseCreator;
		};

	public:
		template<typename ValueCreator>
		static void Create(MemManager& memManager, Key&& key, ValueCreator&& valueCreator,
			Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] ()
				{ std::forward<ValueCreator>(valueCreator)(newValue); };
			HashMultiMapKeyValueTraits::MoveExecKey(memManager, std::move(key), newKey, func);
		}

		template<typename ValueCreator>
		static void Create(MemManager& memManager, const Key& key, ValueCreator&& valueCreator,
			Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] ()
				{ std::forward<ValueCreator>(valueCreator)(newValue); };
			HashMultiMapKeyValueTraits::CopyExecKey(memManager, key, newKey, func);
		}

		static void Destroy(MemManager* memManager, Key& key, Value& value) noexcept
		{
			MOMO_ASSERT(memManager != nullptr);
			HashMultiMapKeyValueTraits::DestroyKey(*memManager, key);
			ValueManager::Destroy(*memManager, value);
		}

		static void Relocate(MemManager* memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue)
		{
			MOMO_ASSERT(memManager != nullptr);
			HashMultiMapKeyValueTraits::RelocateKey(*memManager, srcKey, dstKey);
			ValueManager::Relocate(*memManager, srcValue, dstValue);
		}

		static void Replace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue)
		{
			HashMultiMapKeyValueTraits::AssignKey(memManager, std::move(srcKey), dstKey);	//?
			HashMultiMapKeyValueTraits::DestroyKey(memManager, srcKey);
			ValueManager::Replace(memManager, srcValue, dstValue);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void RelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, Func&& func)
		{
			HashMultiMapKeyValueTraits::RelocateExecKeys(memManager, srcKeyBegin, dstKeyBegin,
				count, std::forward<Func>(func));
			ValueManager::Relocate(memManager, srcValueBegin, dstValueBegin, count);
		}

		template<typename KeyArg>
		static void AssignKey(MemManager& memManager, KeyArg&& keyArg, Key& key)
		{
			HashMultiMapKeyValueTraits::AssignKey(memManager, std::forward<KeyArg>(keyArg), key);
		}
	};

	template<typename THashMultiMapSettings>
	class HashMultiMapNestedMapSettings : public HashMapSettings
	{
	protected:
		typedef THashMultiMapSettings HashMultiMapSettings;

	public:
		static const CheckMode checkMode = HashMultiMapSettings::checkMode;
		static const ExtraCheckMode extraCheckMode = HashMultiMapSettings::extraCheckMode;
		static const bool checkVersion = HashMultiMapSettings::checkKeyVersion;
	};
}

template<typename TKey, typename TValue, typename TMemManager>
class HashMultiMapKeyValueTraits
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

	static const bool isKeyNothrowRelocatable = KeyManager::isNothrowRelocatable;

	static const bool isValueTriviallyRelocatable = ValueManager::isTriviallyRelocatable;

	template<typename... ValueArgs>
	using ValueCreator = typename ValueManager::template Creator<ValueArgs...>;

public:
	template<typename Func>
	static void MoveExecKey(MemManager& memManager, Key&& srcKey, Key* dstKey, Func&& func)
	{
		KeyManager::MoveExec(memManager, std::move(srcKey), dstKey, std::forward<Func>(func));
	}

	template<typename Func>
	static void CopyExecKey(MemManager& memManager, const Key& srcKey, Key* dstKey, Func&& func)
	{
		KeyManager::CopyExec(memManager, srcKey, dstKey, std::forward<Func>(func));
	}

	static void DestroyKey(MemManager& memManager, Key& key) noexcept
	{
		KeyManager::Destroy(memManager, key);
	}

	static void DestroyValues(MemManager& memManager, Value* values, size_t count) noexcept
	{
		ValueManager::Destroy(memManager, values, count);
	}

	static void RelocateKey(MemManager& memManager, Key& srcKey, Key* dstKey)
		noexcept(isKeyNothrowRelocatable)
	{
		KeyManager::Relocate(memManager, srcKey, dstKey);
	}

	template<typename KeyIterator, typename Func>
	static void RelocateExecKeys(MemManager& memManager, KeyIterator srcKeyBegin,
		KeyIterator dstKeyBegin, size_t count, Func&& func)
	{
		KeyManager::RelocateExec(memManager, srcKeyBegin, dstKeyBegin, count,
			std::forward<Func>(func));
	}

	template<typename ValueCreator>
	static void RelocateCreateValues(MemManager& memManager, Value* srcValues, Value* dstValues,
		size_t count, ValueCreator&& valueCreator, Value* newValue)
	{
		ValueManager::RelocateCreate(memManager, srcValues, dstValues, count,
			std::forward<ValueCreator>(valueCreator), newValue);
	}

	template<typename KeyArg>
	static void AssignKey(MemManager& /*memManager*/, KeyArg&& keyArg, Key& key)
	{
		key = std::forward<KeyArg>(keyArg);
	}

	static void AssignAnywayValue(MemManager& memManager, Value& srcValue, Value& dstValue)
	{
		ValueManager::AssignAnyway(memManager, srcValue, dstValue);
	}
};

class HashMultiMapSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkKeyVersion = MOMO_CHECK_ITERATOR_VERSION;
	static const bool checkValueVersion = MOMO_CHECK_ITERATOR_VERSION;

	static const size_t valueArrayMaxFastCount = 7;
	typedef MemPoolParams<> ValueArrayMemPoolParams;
	typedef ArraySettings<> ValueArraySettings;
};

/*!
	All `HashMultiMap` functions and constructors have strong exception
	safety, but not the following cases:
	1. Functions `Add` receiving many items have basic exception safety.
	2. Function `Remove` receiving predicate have basic exception safety.
	3. In case default `KeyValueTraits`: If function `Add`, `AddVar` or
	`AddCrt` receiving argument `Key&& key` throws exception,
	this argument may be changed.
*/

template<typename TKey, typename TValue,
	typename THashTraits = HashTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TKeyValueTraits = HashMultiMapKeyValueTraits<TKey, TValue, TMemManager>,
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

	MOMO_STATIC_ASSERT(internal::ObjectAlignmenter<Key>::Check(KeyValueTraits::keyAlignment));
	MOMO_STATIC_ASSERT(internal::ObjectAlignmenter<Value>::Check(KeyValueTraits::valueAlignment));

private:
	typedef internal::HashMultiMapNestedMapKeyValueTraits<KeyValueTraits,
		Settings> HashMapKeyValueTraits;

	typedef typename HashMapKeyValueTraits::Value ValueArray;
	typedef typename ValueArray::Params ValueArrayParams;

	typedef internal::HashMultiMapNestedMapSettings<Settings> HashMapSettings;

	typedef momo::HashMap<Key, ValueArray, HashTraits, MemManager,
		HashMapKeyValueTraits, HashMapSettings> HashMap;

	class ValueCrew
	{
	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		class Data
		{
		public:
			explicit Data(MemManager& memManager)
				: valueVersion(0),
				valueArrayParams(memManager)
			{
			}

		public:
			size_t valueVersion;
			ValueArrayParams valueArrayParams;
		};

	public:
		explicit ValueCrew(MemManager& memManager)
		{
			mData = MemManagerProxy::template Allocate<Data>(memManager, sizeof(Data));
			try
			{
				::new(static_cast<void*>(mData)) Data(memManager);
			}
			catch (...)
			{
				MemManagerProxy::Deallocate(memManager, mData, sizeof(Data));
				throw;
			}
		}

		ValueCrew(ValueCrew&& crew) noexcept
			: mData(nullptr)
		{
			Swap(crew);
		}

		ValueCrew(const ValueCrew&) = delete;

		~ValueCrew() noexcept
		{
			MOMO_ASSERT(IsNull());
		}

		ValueCrew& operator=(const ValueCrew&) = delete;

		void Swap(ValueCrew& crew) noexcept
		{
			std::swap(mData, crew.mData);
		}

		void Destroy(MemManager& memManager) noexcept
		{
			MOMO_ASSERT(!IsNull());
			mData->~Data();
			MemManagerProxy::Deallocate(memManager, mData, sizeof(Data));
			mData = nullptr;
		}

		bool IsNull() const noexcept
		{
			return mData == nullptr;
		}

		const size_t& GetValueVersion() const noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->valueVersion;
		}

		size_t& GetValueVersion() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->valueVersion;
		}

		ValueArrayParams& GetValueArrayParams() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->valueArrayParams;
		}

	private:
		Data* mData;
	};

	typedef typename HashMap::Iterator HashMapIterator;

public:
	typedef internal::HashDerivedIterator<HashMapIterator,
		internal::HashMultiMapKeyReference> KeyIterator;
	typedef typename KeyIterator::ConstIterator ConstKeyIterator;

	typedef internal::HashMultiMapKeyBounds<KeyIterator> KeyBounds;
	typedef typename KeyBounds::ConstBounds ConstKeyBounds;

	typedef internal::HashMultiMapIterator<KeyIterator, Settings> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

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
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, Check, void)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct ConstKeyIteratorProxy : public ConstKeyIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstKeyIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstKeyIterator, GetBaseIterator,
			typename ConstKeyIterator::BaseIterator)
	};

	struct KeyIteratorProxy : public KeyIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(KeyIterator)
		MOMO_DECLARE_PROXY_FUNCTION(KeyIterator, GetBaseIterator, HashMapIterator)
	};

	struct ConstKeyBoundsProxy : public ConstKeyBounds
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstKeyBounds)
	};

	struct KeyBoundsProxy : public KeyBounds
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(KeyBounds)
	};

public:
	HashMultiMap()
		: HashMultiMap(HashTraits())
	{
	}

	explicit HashMultiMap(const HashTraits& hashTraits, MemManager memManager = MemManager())
		: mHashMap(hashTraits, std::move(memManager)),
		mValueCount(0),
		mValueCrew(GetMemManager())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	HashMultiMap(std::initializer_list<Pair> pairs)
		: HashMultiMap(pairs, HashTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit HashMultiMap(std::initializer_list<Pair> pairs, const HashTraits& hashTraits,
		MemManager memManager = MemManager())
		: HashMultiMap(hashTraits, std::move(memManager))
	{
		try
		{
			Add(pairs);
		}
		catch (...)
		{
			pvClearValueArrays();
			mValueCrew.Destroy(GetMemManager());
			throw;
		}
	}

	HashMultiMap(HashMultiMap&& hashMultiMap) noexcept
		: mHashMap(std::move(hashMultiMap.mHashMap)),
		mValueCount(hashMultiMap.mValueCount),
		mValueCrew(std::move(hashMultiMap.mValueCrew))
	{
		hashMultiMap.mValueCount = 0;
	}

	HashMultiMap(const HashMultiMap& hashMultiMap)
		: HashMultiMap(hashMultiMap, MemManager(hashMultiMap.GetMemManager()))
	{
	}

	explicit HashMultiMap(const HashMultiMap& hashMultiMap, MemManager memManager)
		: mHashMap(hashMultiMap.GetHashTraits(), std::move(memManager)),
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
			pvClearValueArrays();
			mValueCrew.Destroy(GetMemManager());
			throw;
		}
	}

	~HashMultiMap() noexcept
	{
		if (!mValueCrew.IsNull())
		{
			pvClearValueArrays();
			mValueCrew.Destroy(GetMemManager());
		}
	}

	HashMultiMap& operator=(HashMultiMap&& hashMultiMap) noexcept
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

	void Swap(HashMultiMap& hashMultiMap) noexcept
	{
		mHashMap.Swap(hashMultiMap.mHashMap);
		std::swap(mValueCount, hashMultiMap.mValueCount);
		mValueCrew.Swap(hashMultiMap.mValueCrew);
	}

	ConstIterator GetBegin() const noexcept
	{
		return pvMakeIterator<ConstIterator>(GetKeyBounds().GetBegin());
	}

	Iterator GetBegin() noexcept
	{
		return pvMakeIterator<Iterator>(GetKeyBounds().GetBegin());
	}

	ConstIterator GetEnd() const noexcept
	{
		return ConstIterator();
	}

	Iterator GetEnd() noexcept
	{
		return Iterator();
	}

	MOMO_FRIEND_SWAP(HashMultiMap)
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(HashMultiMap, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(HashMultiMap, Iterator)

	const HashTraits& GetHashTraits() const noexcept
	{
		return mHashMap.GetHashTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mHashMap.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mHashMap.GetMemManager();
	}

	size_t GetValueCount() const noexcept
	{
		return mValueCount;
	}

	void Clear() noexcept
	{
		if (!mValueCrew.IsNull())
		{
			pvClearValueArrays();
			mHashMap.Clear();
			mValueCount = 0;
			++mValueCrew.GetValueVersion();
		}
	}

	ConstKeyBounds GetKeyBounds() const noexcept
	{
		return ConstKeyBoundsProxy(ConstKeyIteratorProxy(mHashMap.GetBegin()), GetKeyCount());
	}

	KeyBounds GetKeyBounds() noexcept
	{
		return KeyBoundsProxy(KeyIteratorProxy(mHashMap.GetBegin()), GetKeyCount());
	}

	size_t GetKeyCount() const noexcept
	{
		return mHashMap.GetCount();
	}

	//void Shrink()
	//{
	//	HashMultiMap(*this).Swap(*this);
	//}

	MOMO_FORCEINLINE ConstKeyIterator Find(const Key& key) const
	{
		return ConstKeyIteratorProxy(mHashMap.Find(key));
	}

	MOMO_FORCEINLINE KeyIterator Find(const Key& key)
	{
		return KeyIteratorProxy(mHashMap.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstKeyIterator> Find(const KeyArg& key) const
	{
		return ConstKeyIteratorProxy(mHashMap.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	KeyIterator> Find(const KeyArg& key)
	{
		return KeyIteratorProxy(mHashMap.Find(key));
	}

	MOMO_FORCEINLINE bool ContainsKey(const Key& key) const
	{
		return mHashMap.ContainsKey(key);
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	bool> ContainsKey(const KeyArg& key) const
	{
		return mHashMap.ContainsKey(key);
	}

	template<typename ValueCreator>
	Iterator AddCrt(Key&& key, ValueCreator&& valueCreator)
	{
		return pvAdd(std::move(key), std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
	Iterator AddVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return pvAdd(std::move(key),
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
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
	Iterator AddCrt(const Key& key, ValueCreator&& valueCreator)
	{
		return pvAdd(key, std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
	Iterator AddVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return pvAdd(key,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
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
	Iterator AddCrt(ConstKeyIterator keyIter, ValueCreator&& valueCreator)
	{
		HashMapIterator hashMapIter = mHashMap.MakeMutableIterator(
			ConstKeyIteratorProxy::GetBaseIterator(keyIter));
		ValueArray& valueArray = hashMapIter->value;
		pvAddValue(valueArray, std::forward<ValueCreator>(valueCreator));
		return pvMakeIterator(KeyIteratorProxy(hashMapIter),
			valueArray.GetBounds().GetCount() - 1, false);
	}

	template<typename... ValueArgs>
	Iterator AddVar(ConstKeyIterator keyIter, ValueArgs&&... valueArgs)
	{
		return AddCrt(keyIter,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Iterator Add(ConstKeyIterator keyIter, Value&& value)
	{
		return AddVar(keyIter, std::move(value));
	}

	Iterator Add(ConstKeyIterator keyIter, const Value& value)
	{
		return AddVar(keyIter, value);
	}

	template<typename ArgIterator,
		typename = decltype(internal::MapPairConverter<ArgIterator>::Convert(*std::declval<ArgIterator>()))>
	void Add(ArgIterator begin, ArgIterator end)
	{
		for (ArgIterator iter = begin; iter != end; ++iter)
		{
			auto pair = internal::MapPairConverter<ArgIterator>::Convert(*iter);
			typedef decltype(pair.first) KeyArg;
			typedef decltype(pair.second) ValueArg;
			MOMO_STATIC_ASSERT((std::is_same<Key, typename std::decay<KeyArg>::type>::value));
			AddVar(std::forward<KeyArg>(pair.first), std::forward<ValueArg>(pair.second));
		}
	}

	template<typename Pair = std::pair<Key, Value>>
	void Add(std::initializer_list<Pair> pairs)
	{
		Add(pairs.begin(), pairs.end());
	}

	KeyIterator InsertKey(Key&& key)
	{
		return KeyIteratorProxy(mHashMap.Insert(std::move(key), ValueArray()).position);
	}

	KeyIterator InsertKey(const Key& key)
	{
		return KeyIteratorProxy(mHashMap.Insert(key, ValueArray()).position);
	}

	template<typename KeyCreator>
	KeyIterator AddKeyCrt(ConstKeyIterator keyIter, KeyCreator&& keyCreator)
	{
		auto pairCreator = [&keyCreator] (Key* newKey, ValueArray* newValueArray)
		{
			std::forward<KeyCreator>(keyCreator)(newKey);
			::new(static_cast<void*>(newValueArray)) ValueArray();
		};
		return KeyIteratorProxy(mHashMap.AddCrt(ConstKeyIteratorProxy::GetBaseIterator(keyIter),
			pairCreator));
	}

	//KeyIterator AddKey(ConstKeyIterator keyIter, Key&& key)
	//KeyIterator AddKey(ConstKeyIterator keyIter, const Key& key)

	Iterator Remove(ConstKeyIterator keyIter, size_t valueIndex)
	{
		MOMO_CHECK(valueIndex < keyIter->GetCount());
		return Remove(pvMakeIterator(keyIter, valueIndex, false));
	}

	Iterator Remove(ConstIterator iter)
	{
		ConstIteratorProxy::Check(iter, mValueCrew.GetValueVersion());
		HashMapIterator hashMapIter = mHashMap.MakeMutableIterator(
			ConstKeyIteratorProxy::GetBaseIterator(iter.GetKeyIterator()));
		ValueArray& valueArray = hashMapIter->value;
		typename ValueArray::Bounds valueBounds = valueArray.GetBounds();
		size_t valueIndex = internal::UIntMath<>::Dist<typename ConstIterator::ValueIterator>(
			valueBounds.GetBegin(), iter.GetValueIterator());
		KeyValueTraits::AssignAnywayValue(GetMemManager(), *(valueBounds.GetEnd() - 1),
			valueBounds[valueIndex]);
		valueArray.RemoveBack(mValueCrew.GetValueArrayParams());
		--mValueCount;
		++mValueCrew.GetValueVersion();
		return pvMakeIterator(KeyIteratorProxy(hashMapIter), valueIndex, true);
	}

	template<typename PairPredicate>
	internal::EnableIf<internal::IsInvocable<const PairPredicate&, bool, const Key&, const Value&>::value,
	size_t> Remove(const PairPredicate& pairPred)
	{
		size_t initValueCount = mValueCount;
		Iterator iter = GetBegin();
		while (!!iter)
		{
			if (pairPred(iter->key, static_cast<const Value&>(iter->value)))
				iter = Remove(iter);
			else
				++iter;
		}
		return initValueCount - mValueCount;
	}

	Iterator RemoveValues(ConstKeyIterator keyIter)
	{
		HashMapIterator hashMapIter = mHashMap.MakeMutableIterator(
			ConstKeyIteratorProxy::GetBaseIterator(keyIter));
		pvRemoveValues(hashMapIter->value);
		return pvMakeIterator<Iterator, KeyIterator>(KeyIteratorProxy(std::next(hashMapIter)));
	}

	KeyIterator RemoveKey(ConstKeyIterator keyIter)
	{
		HashMapIterator hashMapIter = mHashMap.MakeMutableIterator(
			ConstKeyIteratorProxy::GetBaseIterator(keyIter));
		ValueArray& valueArray = hashMapIter->value;
		ValueArray tempValueArray(std::move(valueArray));
		try
		{
			hashMapIter = mHashMap.Remove(hashMapIter);
		}
		catch (...)
		{
			valueArray = std::move(tempValueArray);
			throw;
		}
		pvRemoveValues(tempValueArray);
		return KeyIteratorProxy(hashMapIter);
	}

	size_t RemoveKey(const Key& key)
	{
		KeyIterator keyIter = Find(key);
		if (!keyIter)
			return 0;
		size_t valueCount = keyIter->GetCount();
		RemoveKey(keyIter);
		return valueCount;
	}

	template<typename KeyArg, bool extraCheck = true>
	void ResetKey(ConstKeyIterator keyIter, KeyArg&& keyArg)
	{
		mHashMap.template ResetKey<KeyArg, extraCheck>(
			ConstKeyIteratorProxy::GetBaseIterator(keyIter), std::forward<KeyArg>(keyArg));
	}

	ConstIterator MakeIterator(ConstKeyIterator keyIter, size_t valueIndex = 0) const
	{
		return pvMakeIterator<ConstIterator>(keyIter, valueIndex);
	}

	Iterator MakeIterator(KeyIterator keyIter, size_t valueIndex = 0)
	{
		return pvMakeIterator<Iterator>(keyIter, valueIndex);
	}

	Iterator MakeMutableIterator(ConstIterator iter)
	{
		if (!iter)
			return Iterator();
		ConstIteratorProxy::Check(iter, mValueCrew.GetValueVersion());
		KeyIterator keyIter = MakeMutableKeyIterator(iter.GetKeyIterator());
		size_t valueIndex = internal::UIntMath<>::Dist<typename ConstIterator::ValueIterator>(
			keyIter->GetBegin(), iter.GetValueIterator());
		return pvMakeIterator(keyIter, valueIndex, false);
	}

	KeyIterator MakeMutableKeyIterator(ConstKeyIterator keyIter)
	{
		return KeyIteratorProxy(mHashMap.MakeMutableIterator(
			ConstKeyIteratorProxy::GetBaseIterator(keyIter)));
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		CheckKeyIterator(iter.GetKeyIterator(), allowEmpty);
		if (!!iter)
			ConstIteratorProxy::Check(iter, mValueCrew.GetValueVersion());
	}

	void CheckKeyIterator(ConstKeyIterator keyIter, bool allowEmpty = true) const
	{
		mHashMap.CheckIterator(ConstKeyIteratorProxy::GetBaseIterator(keyIter), allowEmpty);
	}

private:
	void pvClearValueArrays() noexcept
	{
		ValueArrayParams& valueArrayParams = mValueCrew.GetValueArrayParams();
		for (typename HashMap::Iterator::Reference ref : mHashMap)
			ref.value.Clear(valueArrayParams);
		valueArrayParams.Clear();
	}

	template<typename Iterator, typename KeyIterator>
	Iterator pvMakeIterator(KeyIterator keyIter) const noexcept
	{
		if (!keyIter)
			return Iterator();
		return pvMakeIterator(keyIter, 0, true);
	}

	ConstIterator pvMakeIterator(ConstKeyIterator keyIter, size_t valueIndex,
		bool move) const noexcept
	{
		return ConstIteratorProxy(keyIter, keyIter->GetBegin() + valueIndex,
			mValueCrew.GetValueVersion(), move);
	}

	Iterator pvMakeIterator(KeyIterator keyIter, size_t valueIndex, bool move) const noexcept
	{
		return IteratorProxy(keyIter, keyIter->GetBegin() + valueIndex,
			mValueCrew.GetValueVersion(), move);
	}

	template<typename Iterator, typename KeyIterator>
	Iterator pvMakeIterator(KeyIterator keyIter, size_t valueIndex) const
	{
		if (!keyIter && valueIndex == 0)
			return Iterator();
		CheckKeyIterator(keyIter);
		MOMO_CHECK(valueIndex <= keyIter->GetCount());
		return pvMakeIterator(keyIter, valueIndex, true);
	}

	template<typename RKey, typename ValueCreator>
	Iterator pvAdd(RKey&& key, ValueCreator&& valueCreator)
	{
		KeyIterator keyIter = Find(static_cast<const Key&>(key));
		if (!!keyIter)
			return AddCrt(keyIter, std::forward<ValueCreator>(valueCreator));
		auto valuesCreator = [this, &valueCreator] (ValueArray* newValueArray)
		{
			ValueArray valueArray;
			this->pvAddValue(valueArray, std::forward<ValueCreator>(valueCreator));
			::new(static_cast<void*>(newValueArray)) ValueArray(std::move(valueArray));
		};
		keyIter = KeyIteratorProxy(mHashMap.template AddCrt<decltype(valuesCreator), false>(
			KeyIteratorProxy::GetBaseIterator(keyIter), std::forward<RKey>(key),
			std::move(valuesCreator)));
		return pvMakeIterator(keyIter, 0, false);
	}

	template<typename ValueCreator>
	void pvAddValue(ValueArray& valueArray, ValueCreator&& valueCreator)
	{
		valueArray.AddBackCrt(mValueCrew.GetValueArrayParams(),
			std::forward<ValueCreator>(valueCreator));
		++mValueCount;
		++mValueCrew.GetValueVersion();
	}

	void pvRemoveValues(ValueArray& valueArray) noexcept
	{
		size_t count = valueArray.GetBounds().GetCount();
		valueArray.RemoveAll(mValueCrew.GetValueArrayParams());
		mValueCount -= count;
		++mValueCrew.GetValueVersion();
	}

private:
	HashMap mHashMap;
	size_t mValueCount;
	ValueCrew mValueCrew;
};

template<typename TKey, typename TValue>
using HashMultiMapOpen = HashMultiMap<TKey, TValue, HashTraitsOpen<TKey>>;

namespace internal
{
	class NestedHashMultiMapSettings : public HashMultiMapSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkKeyVersion = false;
		static const bool checkValueVersion = false;
	};
}

} // namespace momo

namespace std
{
	template<typename KI, typename S>
	struct iterator_traits<momo::internal::HashMultiMapIterator<KI, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::HashMultiMapIterator<KI, S>,
			forward_iterator_tag>
	{
	};
} // namespace std
