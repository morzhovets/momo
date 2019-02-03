/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MapUtility.h

\**********************************************************/

#pragma once

#include "ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TKey, typename TValue, typename TSetReference>
	class MapReference
	{
	public:
		typedef TKey Key;
		typedef TValue Value;

	protected:
		typedef TSetReference SetReference;

	public:
		typedef MapReference<Key, const Value, SetReference> ConstReference;

	public:
		explicit MapReference(const Key& key, Value& value) noexcept
			: key(key),
			value(value)
		{
		}

		operator ConstReference() const noexcept
		{
			return ConstReference(key, value);
		}

	protected:
		explicit MapReference(SetReference setRef) noexcept
			: key(*setRef.GetKeyPtr()),
			value(*setRef.GetValuePtr())
		{
		}

	public:
		const Key& key;
		Value& value;
	};

	template<typename TKey, typename TValue, typename THashMapReference>
	class MapReferenceStd : public std::pair<const TKey&, TValue&>
	{
	public:
		typedef TKey Key;
		typedef TValue Value;

	protected:
		typedef THashMapReference HashMapReference;

	public:
		typedef MapReferenceStd<Key, const Value,
			typename HashMapReference::ConstReference> ConstReference;

	private:
		typedef std::pair<const Key&, Value&> RefPair;

	public:
		explicit MapReferenceStd(const Key& key, Value& value) noexcept
			: RefPair(key, value)
		{
		}

		operator ConstReference() noexcept
		{
			return ConstReference(RefPair::first, RefPair::second);
		}

		template<typename First, typename Second>
		bool operator==(const std::pair<First, Second>& pair) const
		{
			return RefPair::first == pair.first && RefPair::second == pair.second;
		}

		template<typename First, typename Second>
		bool operator!=(const std::pair<First, Second>& pair) const
		{
			return !(*this == pair);
		}

		//? <, >, <=, >=

	protected:
		explicit MapReferenceStd(HashMapReference ref) noexcept
			: RefPair(ref.key, ref.value)
		{
		}
	};

	template<typename TSetIterator, typename TKey>
	class MapKeyIterator
	{
	public:
		typedef TSetIterator SetIterator;
		typedef TKey Key;

		typedef Key& Reference;
		typedef Key* Pointer;

	public:
		explicit MapKeyIterator(SetIterator setIterator) noexcept
			: mSetIterator(setIterator)
		{
		}

		MapKeyIterator& operator++() noexcept
		{
			++mSetIterator;
			return *this;
		}

		Pointer operator->() const noexcept
		{
			return mSetIterator->GetKeyPtr();
		}

		Reference operator*() const noexcept
		{
			return *operator->();
		}

	private:
		SetIterator mSetIterator;
	};

	template<typename TSetIterator, typename TValue>
	class MapValueIterator
	{
	public:
		typedef TSetIterator SetIterator;
		typedef TValue Value;

		typedef Value& Reference;
		typedef Value* Pointer;

	public:
		explicit MapValueIterator(SetIterator setIterator) noexcept
			: mSetIterator(setIterator)
		{
		}

		MapValueIterator& operator++() noexcept
		{
			++mSetIterator;
			return *this;
		}

		Pointer operator->() const noexcept
		{
			return mSetIterator->GetValuePtr();
		}

		Reference operator*() const noexcept
		{
			return *operator->();
		}

	private:
		SetIterator mSetIterator;
	};

	template<typename TKey, typename TValue, typename TMemManager>
	class MapKeyValueTraits
	{
	public:
		typedef TKey Key;
		typedef TValue Value;
		typedef TMemManager MemManager;

	private:
		typedef ObjectManager<Key, MemManager> KeyManager;
		typedef ObjectManager<Value, MemManager> ValueManager;

	public:
		static const size_t keyAlignment = KeyManager::alignment;
		static const size_t valueAlignment = ValueManager::alignment;

		static const bool isKeyNothrowRelocatable = KeyManager::isNothrowRelocatable;
		static const bool isValueNothrowRelocatable = ValueManager::isNothrowRelocatable;

		template<typename... ValueArgs>
		using ValueCreator = typename ValueManager::template Creator<ValueArgs...>;

	public:
		template<typename ValueCreator>
		static void Create(MemManager& memManager, Key&& key,
			ValueCreator&& valueCreator, Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] ()
				{ std::forward<ValueCreator>(valueCreator)(newValue); };
			KeyManager::MoveExec(memManager, std::move(key), newKey, func);
		}

		template<typename ValueCreator>
		static void Create(MemManager& memManager, const Key& key,
			ValueCreator&& valueCreator, Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] ()
				{ std::forward<ValueCreator>(valueCreator)(newValue); };
			KeyManager::CopyExec(memManager, key, newKey, func);
		}

		static void Destroy(MemManager* memManager, Key& key, Value& value) noexcept
		{
			KeyManager::Destroyer::Destroy(memManager, key);
			ValueManager::Destroyer::Destroy(memManager, value);
		}

		static void Relocate(MemManager* memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue)
		{
			pvRelocate(memManager, srcKey, srcValue, dstKey, dstValue,
				BoolConstant<isKeyNothrowRelocatable>(),
				BoolConstant<isValueNothrowRelocatable>());
		}

		static void Replace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue)
		{
			pvReplace(memManager, srcKey, srcValue, dstKey, dstValue,
				BoolConstant<KeyManager::isNothrowAnywayAssignable>(),
				BoolConstant<ValueManager::isNothrowAnywayAssignable>());
		}

		static void ReplaceRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& midKey, Value& midValue, Key* dstKey, Value* dstValue)
		{
			MOMO_ASSERT(std::addressof(srcKey) != std::addressof(midKey));
			MOMO_ASSERT(std::addressof(srcValue) != std::addressof(midValue));
			pvReplaceRelocate(memManager, srcKey, srcValue, midKey, midValue, dstKey, dstValue,
				BoolConstant<isKeyNothrowRelocatable>(), BoolConstant<isValueNothrowRelocatable>(),
				BoolConstant<KeyManager::isNothrowAnywayAssignable>(),
				BoolConstant<ValueManager::isNothrowAnywayAssignable>());
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void RelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, Func&& func)
		{
			pvRelocateExec(memManager, srcKeyBegin, srcValueBegin, dstKeyBegin, dstValueBegin,
				count, std::forward<Func>(func), BoolConstant<isKeyNothrowRelocatable>(),
				BoolConstant<isValueNothrowRelocatable>());
		}

		template<typename KeyArg>
		static void AssignKey(MemManager& /*memManager*/, KeyArg&& keyArg, Key& key)
		{
			key = std::forward<KeyArg>(keyArg);
		}

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
		template<typename ValueArg>
		static void AssignValue(MemManager& /*memManager*/, ValueArg&& valueArg, Value& value)
		{
			value = std::forward<ValueArg>(valueArg);
		}
#endif

	private:
		template<bool isValueNothrowRelocatable>
		static void pvRelocate(MemManager* memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::true_type /*isKeyNothrowRelocatable*/,
			BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::Relocator::Relocate(memManager, srcValue, dstValue);
			KeyManager::Relocator::Relocate(memManager, srcKey, dstKey);
		}

		static void pvRelocate(MemManager* memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::Relocator::Relocate(memManager, srcKey, dstKey);
			ValueManager::Relocator::Relocate(memManager, srcValue, dstValue);
		}

		static void pvRelocate(MemManager* memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			if (memManager != nullptr)
				KeyManager::Copy(*memManager, srcKey, dstKey);
			else
				new(dstKey) Key(std::move(srcKey));	//? basic exception safety
			try
			{
				ValueManager::Relocator::Relocate(memManager, srcValue, dstValue);
			}
			catch (...)
			{
				KeyManager::Destroyer::Destroy(memManager, *dstKey);
				throw;
			}
			KeyManager::Destroyer::Destroy(memManager, srcKey);
		}

		template<bool isValueNothrowAnywayAssignable>
		static void pvReplace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue, std::true_type /*isKeyNothrowAnywayAssignable*/,
			BoolConstant<isValueNothrowAnywayAssignable>)
		{
			ValueManager::Replace(memManager, srcValue, dstValue);
			KeyManager::Replace(memManager, srcKey, dstKey);
		}

		static void pvReplace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue, std::false_type /*isKeyNothrowAnywayAssignable*/,
			std::true_type /*isValueNothrowAnywayAssignable*/)
		{
			KeyManager::Replace(memManager, srcKey, dstKey);
			ValueManager::Replace(memManager, srcValue, dstValue);
		}

		static void pvReplace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue, std::false_type /*isKeyNothrowAnywayAssignable*/,
			std::false_type /*isValueNothrowAnywayAssignable*/)
		{
			pvReplaceUnsafe(memManager, srcKey, srcValue, dstKey, dstValue);
		}

		static void pvReplaceUnsafe(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue)
		{
			// basic exception safety
			dstValue = srcValue;
			dstKey = std::move(srcKey);
			KeyManager::Destroy(memManager, srcKey);
			ValueManager::Destroy(memManager, srcValue);
		}

		template<bool isValueNothrowRelocatable, bool isKeyNothrowAnywayAssignable,
			bool isValueNothrowAnywayAssignable>
		static void pvReplaceRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& midKey, Value& midValue, Key* dstKey, Value* dstValue,
			std::true_type /*isKeyNothrowRelocatable*/, BoolConstant<isValueNothrowRelocatable>,
			BoolConstant<isKeyNothrowAnywayAssignable>, BoolConstant<isValueNothrowAnywayAssignable>)
		{
			ValueManager::ReplaceRelocate(memManager, srcValue, midValue, dstValue);
			KeyManager::ReplaceRelocate(memManager, srcKey, midKey, dstKey);
		}

		template<bool isKeyNothrowAnywayAssignable, bool isValueNothrowAnywayAssignable>
		static void pvReplaceRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& midKey, Value& midValue, Key* dstKey, Value* dstValue,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/,
			BoolConstant<isKeyNothrowAnywayAssignable>, BoolConstant<isValueNothrowAnywayAssignable>)
		{
			KeyManager::ReplaceRelocate(memManager, srcKey, midKey, dstKey);
			ValueManager::ReplaceRelocate(memManager, srcValue, midValue, dstValue);
		}

		template<bool isValueNothrowAnywayAssignable>
		static void pvReplaceRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& midKey, Value& midValue, Key* dstKey, Value* dstValue,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/,
			std::true_type /*isKeyNothrowAnywayAssignable*/,
			BoolConstant<isValueNothrowAnywayAssignable>)
		{
			KeyManager::Copy(memManager, midKey, dstKey);
			try
			{
				ValueManager::ReplaceRelocate(memManager, srcValue, midValue, dstValue);
			}
			catch (...)
			{
				KeyManager::Destroy(memManager, *dstKey);
				throw;
			}
			KeyManager::Replace(memManager, srcKey, midKey);
		}

		static void pvReplaceRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& midKey, Value& midValue, Key* dstKey, Value* dstValue,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/,
			std::false_type /*isKeyNothrowAnywayAssignable*/,
			std::true_type /*isValueNothrowAnywayAssignable*/)
		{
			ValueManager::Copy(memManager, midValue, dstValue);
			try
			{
				KeyManager::ReplaceRelocate(memManager, srcKey, midKey, dstKey);
			}
			catch (...)
			{
				ValueManager::Destroy(memManager, *dstValue);
				throw;
			}
			ValueManager::Replace(memManager, srcValue, midValue);
		}

		static void pvReplaceRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& midKey, Value& midValue, Key* dstKey, Value* dstValue,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/,
			std::false_type /*isKeyNothrowAnywayAssignable*/,
			std::false_type /*isValueNothrowAnywayAssignable*/)
		{
			KeyManager::Copy(memManager, midKey, dstKey);
			try
			{
				ValueManager::Copy(memManager, midValue, dstValue);
				try
				{
					pvReplaceUnsafe(memManager, srcKey, srcValue, midKey, midValue);
				}
				catch (...)
				{
					ValueManager::Destroy(memManager, *dstValue);
					throw;
				}
			}
			catch (...)
			{
				KeyManager::Destroy(memManager, *dstKey);
				throw;
			}
		}

		template<typename KeyIterator, typename ValueIterator, typename Func,
			bool isValueNothrowRelocatable>
		static void pvRelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, Func&& func, std::true_type /*isKeyNothrowRelocatable*/,
			BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::RelocateExec(memManager, srcValueBegin, dstValueBegin, count,
				std::forward<Func>(func));
			KeyManager::Relocate(memManager, srcKeyBegin, dstKeyBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, Func&& func, std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::RelocateExec(memManager, srcKeyBegin, dstKeyBegin, count,
				std::forward<Func>(func));
			ValueManager::Relocate(memManager, srcValueBegin, dstValueBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, Func&& func, std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t keyIndex = 0;
			size_t valueIndex = 0;
			try
			{
				KeyIterator srcKeyIter = srcKeyBegin;
				KeyIterator dstKeyIter = dstKeyBegin;
				for (; keyIndex < count; ++keyIndex, ++srcKeyIter, ++dstKeyIter)
					KeyManager::Copy(memManager, *srcKeyIter, std::addressof(*dstKeyIter));
				ValueIterator srcValueIter = srcValueBegin;
				ValueIterator dstValueIter = dstValueBegin;
				for (; valueIndex < count; ++valueIndex, ++srcValueIter, ++dstValueIter)
					ValueManager::Copy(memManager, *srcValueIter, std::addressof(*dstValueIter));
				std::forward<Func>(func)();
			}
			catch (...)
			{
				for (KeyIterator itd = dstKeyBegin; keyIndex > 0; --keyIndex, ++itd)
					KeyManager::Destroy(memManager, *itd);
				for (ValueIterator itd = dstValueBegin; valueIndex > 0; --valueIndex, ++itd)
					ValueManager::Destroy(memManager, *itd);
				throw;
			}
			for (KeyIterator its = srcKeyBegin; keyIndex > 0; --keyIndex, ++its)
				KeyManager::Destroy(memManager, *its);
			for (ValueIterator its = srcValueBegin; valueIndex > 0; --valueIndex, ++its)
				ValueManager::Destroy(memManager, *its);
		}
	};

	template<typename TKeyValueTraits>
	class MapKeyValuePair
	{
	public:
		typedef TKeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::Value Value;

	public:
		MapKeyValuePair() = delete;

		MapKeyValuePair(const MapKeyValuePair& pair) = delete;

		~MapKeyValuePair() = delete;

		MapKeyValuePair& operator=(const MapKeyValuePair&) = delete;

		const Key* GetKeyPtr() const noexcept
		{
			return &mKeyBuffer;
		}

		Key* GetKeyPtr() noexcept
		{
			return &mKeyBuffer;
		}

		Value* GetValuePtr() const noexcept
		{
			return &mValueBuffer;
		}

	private:
		ObjectBuffer<Key, KeyValueTraits::keyAlignment> mKeyBuffer;
		mutable ObjectBuffer<Value, KeyValueTraits::valueAlignment> mValueBuffer;
	};

	template<typename TKeyValuePair>
	class MapNestedSetItemTraits
	{
	protected:
		typedef TKeyValuePair KeyValuePair;	//?
		typedef typename KeyValuePair::KeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::Value Value;

	public:
		typedef KeyValuePair Item;
		typedef typename KeyValueTraits::MemManager MemManager;

	private:
		typedef ObjectManager<Item, MemManager> ItemManager;

	public:
		static const size_t alignment = ItemManager::alignment;

		static const bool isNothrowRelocatable =
			KeyValueTraits::isKeyNothrowRelocatable && KeyValueTraits::isValueNothrowRelocatable;

		template<typename ItemArg>
		class Creator
		{
			MOMO_STATIC_ASSERT((std::is_same<ItemArg, const Item&>::value));

		public:
			explicit Creator(MemManager& memManager, const Item& item) noexcept
				: mMemManager(memManager),
				mItem(item)
			{
			}

			void operator()(Item* newItem) &&
			{
				typename KeyValueTraits::template ValueCreator<const Value&> valueCreator(
					mMemManager, *mItem.GetValuePtr());
				KeyValueTraits::Create(mMemManager, *mItem.GetKeyPtr(), std::move(valueCreator),
					newItem->GetKeyPtr(), newItem->GetValuePtr());
			}

		private:
			MemManager& mMemManager;
			const Item& mItem;
		};

	public:
		static const Key& GetKey(const Item& item) noexcept
		{
			return *item.GetKeyPtr();
		}

		static void Destroy(MemManager* memManager, Item& item) noexcept
		{
			KeyValueTraits::Destroy(memManager, *item.GetKeyPtr(), *item.GetValuePtr());
		}

		static void Relocate(MemManager* memManager, Item& srcItem, Item* dstItem)
		{
			KeyValueTraits::Relocate(memManager, *srcItem.GetKeyPtr(), *srcItem.GetValuePtr(),
				dstItem->GetKeyPtr(), dstItem->GetValuePtr());
		}

		static void Replace(MemManager& memManager, Item& srcItem, Item& dstItem)
		{
			KeyValueTraits::Replace(memManager, *srcItem.GetKeyPtr(), *srcItem.GetValuePtr(),
				*dstItem.GetKeyPtr(), *dstItem.GetValuePtr());
		}

		static void ReplaceRelocate(MemManager& memManager, Item& srcItem, Item& midItem,
			Item* dstItem)
		{
			KeyValueTraits::ReplaceRelocate(memManager, *srcItem.GetKeyPtr(), *srcItem.GetValuePtr(),
				*midItem.GetKeyPtr(), *midItem.GetValuePtr(),
				dstItem->GetKeyPtr(), dstItem->GetValuePtr());
		}

		template<typename KeyArg>
		static void AssignKey(MemManager& memManager, KeyArg&& keyArg, Item& item)
		{
			KeyValueTraits::AssignKey(memManager, std::forward<KeyArg>(keyArg), *item.GetKeyPtr());
		}
	};

	template<typename TMap>
	class MapValueReferencer
	{
	public:
		typedef TMap Map;

		typedef typename Map::Iterator Iterator;

	private:
		typedef typename Map::Value Value;
		typedef typename Map::KeyValueTraits KeyValueTraits;

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
	public:
		template<typename TKeyReference>
		class ValueReference
		{
		protected:
			typedef TKeyReference KeyReference;

		public:
			typedef const Value& ConstReference;

		private:
			typedef typename Map::Settings Settings;

			typedef typename std::add_pointer<KeyReference>::type KeyPointer;

		public:
			ValueReference() = delete;

			ValueReference(const ValueReference& valueRef) noexcept
				: mMap(valueRef.mMap),
				mIter(valueRef.mIter),
				mKeyPtr(valueRef.mKeyPtr)
			{
			}

			~ValueReference() noexcept
			{
			}

			ValueReference& operator=(const ValueReference& valueRef)
			{
				return pvAssign(valueRef.Get());
			}

			template<typename ValueArg>
			ValueReference& operator=(ValueArg&& valueArg)
			{
				return pvAssign(std::forward<ValueArg>(valueArg));
			}

			operator Value&()
			{
				return Get();
			}

			Value& Get()
			{
				MOMO_CHECK(mKeyPtr == nullptr);
				return mIter->value;
			}

		protected:
			explicit ValueReference(Map& map, Iterator iter) noexcept
				: mMap(map),
				mIter(iter),
				mKeyPtr(nullptr)
			{
			}

			explicit ValueReference(Map& map, Iterator iter, KeyReference keyRef) noexcept
				: mMap(map),
				mIter(iter),
				mKeyPtr(std::addressof(keyRef))
			{
			}

		private:
			template<typename ValueArg>
			ValueReference& pvAssign(ValueArg&& valueArg)
			{
				if (mKeyPtr == nullptr)
				{
					KeyValueTraits::AssignValue(mMap.GetMemManager(),
						std::forward<ValueArg>(valueArg), mIter->value);
				}
				else
				{
					typename KeyValueTraits::template ValueCreator<ValueArg> valueCreator(
						mMap.GetMemManager(), std::forward<ValueArg>(valueArg));
					mIter = mMap.AddCrt(mIter, std::forward<KeyReference>(*mKeyPtr),
						std::move(valueCreator));
				}
				mKeyPtr = nullptr;
				return *this;
			}

		private:
			Map& mMap;
			Iterator mIter;
			KeyPointer mKeyPtr;
		};

	private:
		template<typename KeyReference>
		struct ValueReferenceProxy : public ValueReference<KeyReference>
		{
			typedef typename MapValueReferencer::template ValueReference<KeyReference> ValueReference;	// gcc
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ValueReference)
		};

	public:
		template<typename KeyReference>
		static ValueReference<KeyReference> GetReference(Map& map, Iterator iter) noexcept
		{
			return ValueReferenceProxy<KeyReference>(map, iter);
		}

		template<typename KeyReference>
		static ValueReference<KeyReference> GetReference(Map& map, Iterator iter,
			KeyReference keyRef) noexcept
		{
			return ValueReferenceProxy<KeyReference>(map, iter,
				std::forward<KeyReference>(keyRef));
		}
#else
	public:
		template<typename KeyReference>
		using ValueReference = Value&;

	public:
		template<typename KeyReference>
		static ValueReference<KeyReference> GetReference(Map& /*map*/, Iterator iter) noexcept
		{
			return iter->value;
		}

		template<typename KeyReference>
		static ValueReference<KeyReference> GetReference(Map& map, Iterator iter,
			KeyReference keyRef)
		{
			typename KeyValueTraits::template ValueCreator<> valueCreator(map.GetMemManager());
			return map.AddCrt(iter, std::forward<KeyReference>(keyRef),
				std::move(valueCreator))->value;
		}
#endif
	};

	template<typename TSetExtractedItem>
	class MapExtractedPair
	{
	protected:
		typedef TSetExtractedItem SetExtractedItem;
		typedef typename SetExtractedItem::Item KeyValuePair;

	public:
		typedef typename KeyValuePair::Key Key;
		typedef typename KeyValuePair::Value Value;
		typedef typename SetExtractedItem::MemManager MemManager;

	public:
		explicit MapExtractedPair() noexcept
		{
		}

		template<typename Map>
		explicit MapExtractedPair(Map& map, typename Map::ConstIterator iter)
		{
			map.Remove(iter, *this);
		}

		MapExtractedPair(MapExtractedPair&& extractedPair)
			noexcept(noexcept(SetExtractedItem(std::declval<SetExtractedItem&&>())))
			: mSetExtractedItem(std::move(extractedPair.mSetExtractedItem))
		{
		}

		MapExtractedPair(const MapExtractedPair&) = delete;

		~MapExtractedPair() noexcept
		{
		}

		MapExtractedPair& operator=(const MapExtractedPair&) = delete;

		bool IsEmpty() const noexcept
		{
			return mSetExtractedItem.IsEmpty();
		}

		void Clear() noexcept
		{
			mSetExtractedItem.Clear();
		}

		const Key& GetKey() const
		{
			return *mSetExtractedItem.GetItem().GetKeyPtr();
		}

		Key& GetKey()
		{
			return *mSetExtractedItem.GetItem().GetKeyPtr();
		}

		const Value& GetValue() const
		{
			return *mSetExtractedItem.GetItem().GetValuePtr();
		}

		Value& GetValue()
		{
			return *mSetExtractedItem.GetItem().GetValuePtr();
		}

		template<typename PairCreator>
		void Create(PairCreator&& pairCreator)
		{
			auto itemCreator = [&pairCreator] (KeyValuePair* newItem)
			{
				std::forward<PairCreator>(pairCreator)(newItem->GetKeyPtr(),
					newItem->GetValuePtr());
			};
			mSetExtractedItem.Create(itemCreator);
		}

		template<typename PairRemover>
		void Remove(PairRemover&& pairRemover)
		{
			auto itemRemover = [&pairRemover] (KeyValuePair& item)
			{
				std::forward<PairRemover>(pairRemover)(*item.GetKeyPtr(), *item.GetValuePtr());
			};
			mSetExtractedItem.Remove(itemRemover);
		}

	protected:
		SetExtractedItem& ptGetSetExtractedItem() noexcept
		{
			return mSetExtractedItem;
		}

	private:
		SetExtractedItem mSetExtractedItem;
	};

	template<typename TIterator>
	class MapPairConverter
	{
	public:
		typedef TIterator Iterator;

	public:
		template<typename Pair,
			typename KeyArg = decltype(std::declval<Pair>().key),
			typename ValueArg = decltype(std::declval<Pair>().value)>
		static std::pair<KeyArg, ValueArg> Convert(const Pair& pair) noexcept
		{
			MOMO_STATIC_ASSERT(std::is_reference<KeyArg>::value && std::is_reference<ValueArg>::value);
			return std::pair<KeyArg, ValueArg>(pair.key, pair.value);
		}

		template<typename KeyArg, typename ValueArg>
		static std::pair<KeyArg&&, ValueArg&&> Convert(
			std::pair<KeyArg, ValueArg>&& pair) noexcept
		{
			MOMO_STATIC_ASSERT((std::is_reference<KeyArg>::value && std::is_reference<ValueArg>::value)
				|| std::is_reference<typename std::iterator_traits<Iterator>::reference>::value);
			return std::pair<KeyArg&&, ValueArg&&>(std::forward<KeyArg>(pair.first),
				std::forward<ValueArg>(pair.second));
		}

		template<typename KeyArg, typename ValueArg>
		static std::pair<const KeyArg&, const ValueArg&> Convert(
			const std::pair<KeyArg, ValueArg>& pair) noexcept
		{
			MOMO_STATIC_ASSERT((std::is_reference<KeyArg>::value && std::is_reference<ValueArg>::value)
				|| std::is_reference<typename std::iterator_traits<Iterator>::reference>::value);
			return std::pair<const KeyArg&, const ValueArg&>(pair.first, pair.second);
		}
	};
}

} // namespace momo

namespace std
{
	template<typename SI, typename K>
	struct iterator_traits<momo::internal::MapKeyIterator<SI, K>> : public iterator_traits<K*>
	{
		typedef forward_iterator_tag iterator_category;
	};

	template<typename SI, typename V>
	struct iterator_traits<momo::internal::MapValueIterator<SI, V>> : public iterator_traits<V*>
	{
		typedef forward_iterator_tag iterator_category;
	};
} // namespace std
