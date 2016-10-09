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
		typedef TSetReference SetReference;

		typedef MapReference<Key, const Value, SetReference> ConstReference;

	public:
		MapReference(const Key& key, Value& value) MOMO_NOEXCEPT
			: key(key),
			value(value)
		{
		}

		explicit MapReference(SetReference setRef) MOMO_NOEXCEPT
			: key(*setRef.GetKeyPtr()),
			value(*setRef.GetValuePtr())
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

	template<typename TKey, typename TValue, typename THashMapReference>
	class MapReferenceStd : public std::pair<const TKey&, TValue&>
	{
	public:
		typedef TKey Key;
		typedef TValue Value;
		typedef THashMapReference HashMapReference;

		typedef MapReferenceStd<Key, const Value,
			typename HashMapReference::ConstReference> ConstReference;

	private:
		typedef std::pair<const Key&, Value&> RefPair;

	public:
		MapReferenceStd(const Key& key, Value& value) MOMO_NOEXCEPT
			: RefPair(key, value)
		{
		}

		explicit MapReferenceStd(HashMapReference ref) MOMO_NOEXCEPT
			: RefPair(ref.key, ref.value)
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
			_Relocate(memManager, srcKey, srcValue, dstKey, dstValue,
				BoolConstant<KeyManager::isNothrowRelocatable>(),
				BoolConstant<ValueManager::isNothrowRelocatable>());
		}

		static void Replace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue)
		{
			_Replace(memManager, srcKey, srcValue, dstKey, dstValue,
				BoolConstant<KeyManager::isNothrowAnywayAssignable>(),
				BoolConstant<ValueManager::isNothrowAnywayAssignable>());
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void RelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, const Func& func)
		{
			_RelocateExec(memManager, srcKeyBegin, srcValueBegin, dstKeyBegin, dstValueBegin,
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
		static void _Relocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::true_type /*isKeyNothrowRelocatable*/,
			BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::Relocate(memManager, srcValue, dstValue);
			KeyManager::Relocate(memManager, srcKey, dstKey);
		}

		static void _Relocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue, std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::Relocate(memManager, srcKey, dstKey);
			ValueManager::Relocate(memManager, srcValue, dstValue);
		}

		static void _Relocate(MemManager& memManager, Key& srcKey, Value& srcValue,
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

		template<typename KeyIterator, typename ValueIterator, typename Func,
			bool isValueNothrowRelocatable>
		static void _RelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, const Func& func, std::true_type /*isKeyNothrowRelocatable*/,
			BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::RelocateExec(memManager, srcValueBegin, dstValueBegin, count, func);
			KeyManager::Relocate(memManager, srcKeyBegin, dstKeyBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void _RelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
			ValueIterator srcValueBegin, KeyIterator dstKeyBegin, ValueIterator dstValueBegin,
			size_t count, const Func& func, std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::RelocateExec(memManager, srcKeyBegin, dstKeyBegin, count, func);
			ValueManager::Relocate(memManager, srcValueBegin, dstValueBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void _RelocateExec(MemManager& memManager, KeyIterator srcKeyBegin,
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

		template<bool isValueNothrowAnywayMoveAssignable>
		static void _Replace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue, std::true_type /*isKeyNothrowAnywayMoveAssignable*/,
			BoolConstant<isValueNothrowAnywayMoveAssignable>)
		{
			ValueManager::Replace(memManager, srcValue, dstValue);
			KeyManager::Replace(memManager, srcKey, dstKey);
		}

		static void _Replace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue, std::false_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::true_type /*isValueNothrowAnywayMoveAssignable*/)
		{
			KeyManager::Replace(memManager, srcKey, dstKey);
			ValueManager::Replace(memManager, srcValue, dstValue);
		}

		static void _Replace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue, std::false_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::false_type /*isValueNothrowAnywayMoveAssignable*/)
		{
			// basic exception safety
			dstValue = static_cast<const Value&>(srcValue);
			dstKey = std::move(srcKey);
			KeyManager::Destroy(memManager, srcKey);
			ValueManager::Destroy(memManager, srcValue);
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
	public:
		typedef TKeyValuePair Item;
		typedef typename Item::KeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::Value Value;
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
				return _Assign(valueRef.Get());
			}

			template<typename ValueArg>
			ValueReference& operator=(ValueArg&& valueArg)
			{
				return _Assign(std::forward<ValueArg>(valueArg));
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
			ValueReference& _Assign(ValueArg&& valueArg)
			{
				if (mKeyPtr == nullptr)
				{
					KeyValueTraits::AssignValue(mMap.GetMemManager(),
						std::forward<ValueArg>(valueArg), mIter->value);
				}
				else
				{
					mIter = mMap.AddCrt(mIter, std::forward<RKey>(*mKeyPtr),
						typename KeyValueTraits::template ValueCreator<ValueArg>(
						std::forward<ValueArg>(valueArg)));
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
