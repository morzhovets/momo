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

	template<typename TKey, typename TValue>
	class MapKeyValueTraits
	{
	public:
		typedef TKey Key;
		typedef TValue Value;

	private:
		typedef internal::ObjectManager<Key> KeyManager;
		typedef internal::ObjectManager<Value> ValueManager;

	public:
		static const size_t keyAlignment = KeyManager::alignment;
		static const size_t valueAlignment = ValueManager::alignment;

		template<typename... ValueArgs>
		using ValueCreator = typename ValueManager::template Creator<ValueArgs...>;

	public:
		template<typename ValueCreator>
		static void Create(Key&& key, const ValueCreator& valueCreator,
			Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] () { valueCreator(newValue); };
			KeyManager::MoveExec(std::move(key), newKey, func);
		}

		template<typename ValueCreator>
		static void Create(const Key& key, const ValueCreator& valueCreator,
			Key* newKey, Value* newValue)
		{
			auto func = [&valueCreator, newValue] () { valueCreator(newValue); };
			KeyManager::CopyExec(key, newKey, func);
		}

		static void Destroy(Key& key, Value& value) MOMO_NOEXCEPT
		{
			KeyManager::Destroy(key);
			ValueManager::Destroy(value);
		}

		static void Relocate(Key& srcKey, Value& srcValue, Key* dstKey, Value* dstValue)
		{
			_Relocate(srcKey, srcValue, dstKey, dstValue,
				BoolConstant<KeyManager::isNothrowRelocatable>(),
				BoolConstant<ValueManager::isNothrowRelocatable>());
		}

		static void Replace(Key& srcKey, Value& srcValue, Key& dstKey, Value& dstValue)
		{
			_Assign(std::move(srcKey), std::move(srcValue), dstKey, dstValue,
				BoolConstant<KeyManager::isNothrowAnywayAssignable>(),
				BoolConstant<ValueManager::isNothrowAnywayAssignable>());
			Destroy(srcKey, srcValue);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void RelocateExec(KeyIterator srcKeyBegin, ValueIterator srcValueBegin,
			KeyIterator dstKeyBegin, ValueIterator dstValueBegin, size_t count, const Func& func)
		{
			_RelocateExec(srcKeyBegin, srcValueBegin, dstKeyBegin, dstValueBegin, count, func,
				BoolConstant<KeyManager::isNothrowRelocatable>(),
				BoolConstant<ValueManager::isNothrowRelocatable>());
		}

		static void AssignKey(Key&& srcKey, Key& dstKey)
		{
			dstKey = std::move(srcKey);
		}

		static void AssignKey(const Key& srcKey, Key& dstKey)
		{
			dstKey = srcKey;
		}

#ifdef MOMO_USE_SAFE_MAP_BRACKETS
		template<typename ValueArg>
		static void AssignValue(ValueArg&& valueArg, Value& value)
		{
			value = std::forward<ValueArg>(valueArg);
		}
#endif

	private:
		template<bool isValueNothrowRelocatable>
		static void _Relocate(Key& srcKey, Value& srcValue, Key* dstKey, Value* dstValue,
			std::true_type /*isKeyNothrowRelocatable*/, BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::Relocate(srcValue, dstValue);
			KeyManager::Relocate(srcKey, dstKey);
		}

		static void _Relocate(Key& srcKey, Value& srcValue, Key* dstKey, Value* dstValue,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::Relocate(srcKey, dstKey);
			ValueManager::Relocate(srcValue, dstValue);
		}

		static void _Relocate(Key& srcKey, Value& srcValue, Key* dstKey, Value* dstValue,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::Copy(srcKey, dstKey);
			try
			{
				ValueManager::Relocate(srcValue, dstValue);
			}
			catch (...)
			{
				KeyManager::Destroy(*dstKey);
				throw;
			}
			KeyManager::Destroy(srcKey);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func,
			bool isValueNothrowRelocatable>
		static void _RelocateExec(KeyIterator srcKeyBegin, ValueIterator srcValueBegin,
			KeyIterator dstKeyBegin, ValueIterator dstValueBegin, size_t count, const Func& func,
			std::true_type /*isKeyNothrowRelocatable*/, BoolConstant<isValueNothrowRelocatable>)
		{
			ValueManager::RelocateExec(srcValueBegin, dstValueBegin, count, func);
			KeyManager::Relocate(srcKeyBegin, dstKeyBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void _RelocateExec(KeyIterator srcKeyBegin, ValueIterator srcValueBegin,
			KeyIterator dstKeyBegin, ValueIterator dstValueBegin, size_t count, const Func& func,
			std::false_type /*isKeyNothrowRelocatable*/, std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyManager::RelocateExec(srcKeyBegin, dstKeyBegin, count, func);
			ValueManager::Relocate(srcValueBegin, dstValueBegin, count);
		}

		template<typename KeyIterator, typename ValueIterator, typename Func>
		static void _RelocateExec(KeyIterator srcKeyBegin, ValueIterator srcValueBegin,
			KeyIterator dstKeyBegin, ValueIterator dstValueBegin, size_t count, const Func& func,
			std::false_type /*isKeyNothrowRelocatable*/, std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t keyIndex = 0;
			size_t valueIndex = 0;
			try
			{
				KeyIterator srcKeyIter = srcKeyBegin;
				KeyIterator dstKeyIter = dstKeyBegin;
				for (; keyIndex < count; ++keyIndex, ++srcKeyIter, ++dstKeyIter)
					KeyManager::Copy(*srcKeyIter, std::addressof(*dstKeyIter));
				ValueIterator srcValueIter = srcValueBegin;
				ValueIterator dstValueIter = dstValueBegin;
				for (; valueIndex < count; ++valueIndex, ++srcValueIter, ++dstValueIter)
					ValueManager::Copy(*srcValueIter, std::addressof(*dstValueIter));
				func();
			}
			catch (...)
			{
				for (KeyIterator itd = dstKeyBegin; keyIndex > 0; --keyIndex, ++itd)
					KeyManager::Destroy(*itd);
				for (ValueIterator itd = dstValueBegin; valueIndex > 0; --valueIndex, ++itd)
					ValueManager::Destroy(*itd);
				throw;
			}
			for (KeyIterator its = srcKeyBegin; keyIndex > 0; --keyIndex, ++its)
				KeyManager::Destroy(*its);
			for (ValueIterator its = srcValueBegin; valueIndex > 0; --valueIndex, ++its)
				ValueManager::Destroy(*its);
		}

		static void _Assign(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
			std::true_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::true_type /*isValueNothrowAnywayMoveAssignable*/) MOMO_NOEXCEPT
		{
			KeyManager::AssignNothrowAnyway(std::move(srcKey), dstKey);
			ValueManager::AssignNothrowAnyway(std::move(srcValue), dstValue);
		}

		static void _Assign(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
			std::true_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::false_type /*isValueNothrowAnywayMoveAssignable*/)
		{
			dstValue = std::move(srcValue);
			KeyManager::AssignNothrowAnyway(std::move(srcKey), dstKey);
		}

		static void _Assign(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
			std::false_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::true_type /*isValueNothrowAnywayMoveAssignable*/)
		{
			dstKey = std::move(srcKey);
			ValueManager::AssignNothrowAnyway(std::move(srcValue), dstValue);
		}

		static void _Assign(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
			std::false_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::false_type /*isValueNothrowAnywayMoveAssignable*/)
		{
			// basic exception safety
			dstValue = static_cast<const Value&>(srcValue);
			dstKey = std::move(srcKey);
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
		typedef TKeyValuePair KeyValuePair;

		typedef typename KeyValuePair::KeyValueTraits KeyValueTraits;
		typedef typename KeyValuePair::Key Key;
		typedef typename KeyValuePair::Value Value;
		typedef KeyValuePair Item;

	private:
		typedef internal::ObjectManager<Item> ItemManager;

	public:
		static const size_t alignment = ItemManager::alignment;

		template<typename ItemArg>
		class Creator
		{
			MOMO_STATIC_ASSERT((std::is_same<ItemArg, const Item&>::value));

		public:
			explicit Creator(const Item& item) MOMO_NOEXCEPT
				: mItem(item)
			{
			}

			void operator()(Item* newItem) const
			{
				KeyValueTraits::Create(*mItem.GetKeyPtr(),
					typename KeyValueTraits::template ValueCreator<const Value&>(*mItem.GetValuePtr()),
					newItem->GetKeyPtr(), newItem->GetValuePtr());
			}

		private:
			const Item& mItem;
		};

	public:
		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			return *item.GetKeyPtr();
		}

		static void Destroy(Item& item) MOMO_NOEXCEPT
		{
			KeyValueTraits::Destroy(*item.GetKeyPtr(), *item.GetValuePtr());
		}

		static void Relocate(Item& srcItem, Item* dstItem)
		{
			KeyValueTraits::Relocate(*srcItem.GetKeyPtr(), *srcItem.GetValuePtr(),
				dstItem->GetKeyPtr(), dstItem->GetValuePtr());
		}

		static void Replace(Item& srcItem, Item& dstItem)
		{
			KeyValueTraits::Replace(*srcItem.GetKeyPtr(), *srcItem.GetValuePtr(),
				*dstItem.GetKeyPtr(), *dstItem.GetValuePtr());
		}

		static void AssignKey(Key&& srcKey, Item& dstItem)
		{
			KeyValueTraits::AssignKey(std::move(srcKey), *dstItem.GetKeyPtr());
		}

		static void AssignKey(const Key& srcKey, Item& dstItem)
		{
			KeyValueTraits::AssignKey(srcKey, *dstItem.GetKeyPtr());
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
					KeyValueTraits::AssignValue(std::forward<ValueArg>(valueArg), mIter->value);
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
