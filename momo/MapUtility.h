/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

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
		MapReference(const Key& key, Value& value) MOMO_NOEXCEPT
			: key(key),
			value(value)
		{
		}

		operator ConstReference() const MOMO_NOEXCEPT
		{
			return ConstReference(key, value);
		}

	protected:
		explicit MapReference(SetReference setRef) MOMO_NOEXCEPT
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
		MapReferenceStd(const Key& key, Value& value) MOMO_NOEXCEPT
			: RefPair(key, value)
		{
		}

		operator ConstReference() MOMO_NOEXCEPT
		{
			return ConstReference(this->first, this->second);
		}

		template<typename First, typename Second>
		bool operator==(const std::pair<First, Second>& pair) const
		{
			return this->first == pair.first && this->second == pair.second;
		}

		template<typename First, typename Second>
		bool operator!=(const std::pair<First, Second>& pair) const
		{
			return !(*this == pair);
		}

		//? <, >, <=, >=

	protected:
		explicit MapReferenceStd(HashMapReference ref) MOMO_NOEXCEPT
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

	public:
		explicit MapKeyIterator(SetIterator setIterator) MOMO_NOEXCEPT
			: mSetIterator(setIterator)
		{
		}

		MapKeyIterator& operator++() MOMO_NOEXCEPT
		{
			++mSetIterator;
			return *this;
		}

		Key& operator*() const MOMO_NOEXCEPT
		{
			return *mSetIterator->GetKeyPtr();
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

	public:
		explicit MapValueIterator(SetIterator setIterator) MOMO_NOEXCEPT
			: mSetIterator(setIterator)
		{
		}

		MapValueIterator& operator++() MOMO_NOEXCEPT
		{
			++mSetIterator;
			return *this;
		}

		Value& operator*() const MOMO_NOEXCEPT
		{
			return *mSetIterator->GetValuePtr();
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

		template<typename... ValueArgs>
		using ValueCreator = typename ValueManager::template Creator<ValueArgs...>;

	public:
		template<typename ValueCreator>
		static void Create(MemManager& memManager, Key&& key,
			const ValueCreator& valueCreator, Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] () { valueCreator(newValue); };
			KeyManager::MoveExec(memManager, std::move(key), newKey, func);
		}

		template<typename ValueCreator>
		static void Create(MemManager& memManager, const Key& key,
			const ValueCreator& valueCreator, Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] () { valueCreator(newValue); };
			KeyManager::CopyExec(memManager, key, newKey, func);
		}

		static void Destroy(MemManager& memManager, Key& key, Value& value) MOMO_NOEXCEPT
		{
			KeyManager::Destroy(memManager, key);
			ValueManager::Destroy(memManager, value);
		}

		static void Relocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue)
		{
			pvRelocate(memManager, srcKey, srcValue, dstKey, dstValue,
				BoolConstant<KeyManager::isNothrowRelocatable>(),
				BoolConstant<ValueManager::isNothrowRelocatable>());
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
				BoolConstant<KeyManager::isNothrowRelocatable>(),
				BoolConstant<ValueManager::isNothrowRelocatable>(),
				BoolConstant<KeyManager::isNothrowAnywayAssignable>(),
				BoolConstant<ValueManager::isNothrowAnywayAssignable>());
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void RelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, const Func& func)
		{
			pvRelocateExec(memManager, srcKeyBegin, srcValueBegin, dstKeyBegin, dstValueBegin,
				count, func, BoolConstant<KeyManager::isNothrowRelocatable>(),
				BoolConstant<ValueManager::isNothrowRelocatable>());
		}

		static void AssignKey(MemManager& /*memManager*/, Key&& srcKey, Key& dstKey)
		{
			dstKey = std::move(srcKey);
		}

		static void AssignKey(MemManager& /*memManager*/, const Key& srcKey, Key& dstKey)
		{
			dstKey = srcKey;
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
		static void pvRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::true_type /*isKeyNothrowRelocatable*/,
			BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::Relocate(memManager, srcValue, dstValue);
			KeyManager::Relocate(memManager, srcKey, dstKey);
		}

		static void pvRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::Relocate(memManager, srcKey, dstKey);
			ValueManager::Relocate(memManager, srcValue, dstValue);
		}

		static void pvRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::Copy(memManager, srcKey, dstKey);
			try
			{
				ValueManager::Relocate(memManager, srcValue, dstValue);
			}
			catch (...)
			{
				KeyManager::Destroy(memManager, *dstKey);
				throw;
			}
			KeyManager::Destroy(memManager, srcKey);
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
			size_t count, const Func& func, std::true_type /*isKeyNothrowRelocatable*/,
			BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::RelocateExec(memManager, srcValueBegin, dstValueBegin, count, func);
			KeyManager::Relocate(memManager, srcKeyBegin, dstKeyBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, const Func& func, std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::RelocateExec(memManager, srcKeyBegin, dstKeyBegin, count, func);
			ValueManager::Relocate(memManager, srcValueBegin, dstValueBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void pvRelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, const Func& func, std::false_type /*isKeyNothrowRelocatable*/,
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
				func();
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

		const Key* GetKeyPtr() const MOMO_NOEXCEPT
		{
			return &mKeyBuffer;
		}

		Key* GetKeyPtr() MOMO_NOEXCEPT
		{
			return &mKeyBuffer;
		}

		Value* GetValuePtr() const MOMO_NOEXCEPT
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

		template<typename ItemArg>
		class Creator
		{
			MOMO_STATIC_ASSERT((std::is_same<ItemArg, const Item&>::value));

		public:
			Creator(MemManager& memManager, const Item& item) MOMO_NOEXCEPT
				: mMemManager(memManager),
				mItem(item)
			{
			}

			void operator()(Item* newItem) const
			{
				typename KeyValueTraits::template ValueCreator<const Value&> valueCreator(
					mMemManager, *mItem.GetValuePtr());
				KeyValueTraits::Create(mMemManager, *mItem.GetKeyPtr(), valueCreator,
					newItem->GetKeyPtr(), newItem->GetValuePtr());
			}

		private:
			MemManager& mMemManager;
			const Item& mItem;
		};

	public:
		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			return *item.GetKeyPtr();
		}

		static void Destroy(MemManager& memManager, Item& item) MOMO_NOEXCEPT
		{
			KeyValueTraits::Destroy(memManager, *item.GetKeyPtr(), *item.GetValuePtr());
		}

		static void Relocate(MemManager& memManager, Item& srcItem, Item* dstItem)
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

		static void AssignKey(MemManager& memManager, Key&& srcKey, Item& dstItem)
		{
			KeyValueTraits::AssignKey(memManager, std::move(srcKey), *dstItem.GetKeyPtr());
		}

		static void AssignKey(MemManager& memManager, const Key& srcKey, Item& dstItem)
		{
			KeyValueTraits::AssignKey(memManager, srcKey, *dstItem.GetKeyPtr());
		}
	};

	template<typename TMap>
	class MapValueReferencer
	{
	public:
		typedef TMap Map;
		typedef typename Map::Key Key;
		typedef typename Map::Value Value;
		typedef typename Map::KeyValueTraits KeyValueTraits;
		typedef typename Map::Iterator Iterator;
		typedef typename Map::Settings Settings;

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
	private:
		template<typename RKey, typename PKey>
		class ValueReference
		{
		public:
			ValueReference(Map& map, Iterator iter, PKey pkey) MOMO_NOEXCEPT
				: mMap(map),
				mIter(iter),
				mKeyPtr(pkey)
			{
			}

			ValueReference(ValueReference&& valueRef) MOMO_NOEXCEPT
				: mMap(valueRef.mMap),
				mIter(valueRef.mIter),
				mKeyPtr(valueRef.mKeyPtr)
			{
			}

			ValueReference(const ValueReference&) = delete;

			~ValueReference() MOMO_NOEXCEPT
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
					mIter = mMap.AddCrt(mIter, std::forward<RKey>(*mKeyPtr), valueCreator);
				}
				mKeyPtr = nullptr;
				return *this;
			}

		private:
			Map& mMap;
			Iterator mIter;
			PKey mKeyPtr;
		};

	public:
		typedef ValueReference<Key&&, Key*> ValueReferenceRKey;
		typedef ValueReference<const Key&, const Key*> ValueReferenceCKey;
#else
		typedef Value& ValueReferenceRKey;
		typedef Value& ValueReferenceCKey;
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
		MapExtractedPair() MOMO_NOEXCEPT
		{
		}

		template<typename Map>
		MapExtractedPair(Map& map, typename Map::ConstIterator iter)
		{
			map.Remove(iter, *this);
		}

		MapExtractedPair(MapExtractedPair&& extractedPair) //MOMO_NOEXCEPT_IF
			: mSetExtractedItem(std::move(extractedPair.mSetExtractedItem))
		{
		}

		MapExtractedPair(const MapExtractedPair&) = delete;

		~MapExtractedPair() MOMO_NOEXCEPT
		{
		}

		MapExtractedPair& operator=(const MapExtractedPair&) = delete;

		bool IsEmpty() const MOMO_NOEXCEPT
		{
			return mSetExtractedItem.IsEmpty();
		}

		void Clear() MOMO_NOEXCEPT
		{
			mSetExtractedItem.Clear();
		}

		const MemManager& GetMemManager() const
		{
			return mSetExtractedItem.GetMemManager();
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
		void Set(MemManager& memManager, const PairCreator& pairCreator)
		{
			auto itemCreator = [&pairCreator] (KeyValuePair* newItem)
				{ pairCreator(newItem->GetKeyPtr(), newItem->GetValuePtr()); };
			mSetExtractedItem.Set(memManager, itemCreator);
		}

		template<typename PairRemover>
		void Reset(const PairRemover& pairRemover)
		{
			auto itemRemover = [&pairRemover] (KeyValuePair& item)
				{ pairRemover(*item.GetKeyPtr(), *item.GetValuePtr()); };
			mSetExtractedItem.Reset(itemRemover);
		}

	protected:
		SetExtractedItem& ptGetSetExtractedItem() MOMO_NOEXCEPT
		{
			return mSetExtractedItem;
		}

	private:
		SetExtractedItem mSetExtractedItem;
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
