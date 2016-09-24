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
			: key(setRef.GetKey()),
			value(setRef.GetValue())
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

	template<typename TKey, typename TValue>
	struct MapKeyValueTraits
	{
		typedef TKey Key;
		typedef TValue Value;

		typedef internal::ObjectManager<Key> KeyManager;
		typedef internal::ObjectManager<Value> ValueManager;

		static const size_t keyAlignment = KeyManager::alignment;
		static const size_t valueAlignment = ValueManager::alignment;

		static const bool isKeyNothrowMoveConstructible = KeyManager::isNothrowMoveConstructible;
		static const bool isKeyNothrowRelocatable = KeyManager::isNothrowRelocatable;
		static const bool isValueNothrowRelocatable = ValueManager::isNothrowRelocatable;

		template<typename... ValueArgs>
		using ValueCreator = typename ValueManager::template Creator<ValueArgs...>;

		static void MoveKey(Key&& srcKey, Key* dstKey)
			MOMO_NOEXCEPT_IF(isKeyNothrowMoveConstructible)
		{
			KeyManager::Move(std::move(srcKey), dstKey);
		}

		static void CopyKey(const Key& srcKey, Key* dstKey)
		{
			KeyManager::Copy(srcKey, dstKey);
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
			KeyManager::Relocate(srcKey, dstKey);
		}

		static void RelocateValueNothrow(Value& srcValue, Value* dstValue) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(isValueNothrowRelocatable);
			ValueManager::Relocate(srcValue, dstValue);
		}

		static void ReplacePair(Key& srcKey, Value& srcValue, Key& dstKey, Value& dstValue)
		{
			_AssignPair(std::move(srcKey), std::move(srcValue), dstKey, dstValue,
				BoolConstant<KeyManager::isNothrowAnywayAssignable>(),
				BoolConstant<ValueManager::isNothrowAnywayAssignable>());
			DestroyKey(srcKey);
			DestroyValue(srcValue);
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
		static void _AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
			std::true_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::true_type /*isValueNothrowAnywayMoveAssignable*/) MOMO_NOEXCEPT
		{
			KeyManager::AssignNothrowAnyway(std::move(srcKey), dstKey);
			ValueManager::AssignNothrowAnyway(std::move(srcValue), dstValue);
		}

		static void _AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
			std::true_type /*isKeyNothrowAnywayMoveAssignable*/,
			std::false_type /*isValueNothrowAnywayMoveAssignable*/)
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

		static void _AssignPair(Key&& srcKey, Value&& srcValue, Key& dstKey, Value& dstValue,
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

	private:
		typedef internal::BoolConstant<KeyValueTraits::isKeyNothrowMoveConstructible>
			IsKeyNothrowMoveConstructible;

		template<typename... ValueArgs>
		using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	public:
		template<typename ValueCreator>
		MapKeyValuePair(Key&& key, const ValueCreator& valueCreator)
		{
			_Create(std::move(key), valueCreator, IsKeyNothrowMoveConstructible());
		}

		template<typename ValueCreator>
		MapKeyValuePair(const Key& key, const ValueCreator& valueCreator)
		{
			_Create(key, valueCreator);
		}

		MapKeyValuePair(MapKeyValuePair&& pair) = delete;

		MapKeyValuePair(const MapKeyValuePair& pair)	//?
		{
			_Create(pair.GetKey(), ValueCreator<const Value&>(pair.GetValue()));
		}

		~MapKeyValuePair() MOMO_NOEXCEPT
		{
			KeyValueTraits::DestroyKey(GetKey());
			KeyValueTraits::DestroyValue(GetValue());
		}

		MapKeyValuePair& operator=(const MapKeyValuePair&) = delete;

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

		static void Relocate(MapKeyValuePair& srcPair, MapKeyValuePair* dstPair)
		{
			_Relocate(srcPair, dstPair, BoolConstant<KeyValueTraits::isKeyNothrowRelocatable>(),
				BoolConstant<KeyValueTraits::isValueNothrowRelocatable>());
		}

		static void Replace(MapKeyValuePair& srcPair, MapKeyValuePair& dstPair)
		{
			KeyValueTraits::ReplacePair(srcPair.GetKey(), srcPair.GetValue(), dstPair.GetKey(),
				dstPair.GetValue());
		}

		static void AssignKey(Key&& srcKey, MapKeyValuePair& dstPair)
		{
			KeyValueTraits::AssignKey(std::move(srcKey), dstPair.GetKey());
		}

		static void AssignKey(const Key& srcKey, MapKeyValuePair& dstPair)
		{
			KeyValueTraits::AssignKey(srcKey, dstPair.GetKey());
		}

		static void SwapNothrowAnyway(MapKeyValuePair& pair1, MapKeyValuePair& pair2) MOMO_NOEXCEPT
		{
			KeyValueTraits::SwapKeysNothrowAnyway(pair1.GetKey(), pair2.GetKey());
			KeyValueTraits::SwapValuesNothrowAnyway(pair1.GetValue(), pair2.GetValue());
		}

		template<typename Iterator, typename PairCreator>
		static void RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const PairCreator& pairCreator, MapKeyValuePair* newPair)
		{
			_RelocateCreate(srcBegin, dstBegin, count, pairCreator, newPair,
				BoolConstant<KeyValueTraits::isKeyNothrowRelocatable>(),
				BoolConstant<KeyValueTraits::isValueNothrowRelocatable>());
		}

	private:
		template<typename ValueCreator>
		void _Create(Key&& key, const ValueCreator& valueCreator,
			std::true_type /*isKeyNothrowMoveConstructible*/)
		{
			valueCreator(&mValueBuffer);
			KeyValueTraits::MoveKey(std::move(key), &mKeyBuffer);
		}

		template<typename ValueCreator>
		void _Create(Key&& key, const ValueCreator& valueCreator,
			std::false_type /*isKeyNothrowMoveConstructible*/)
		{
			_Create(static_cast<const Key&>(key), valueCreator);
		}

		template<typename ValueCreator>
		void _Create(const Key& key, const ValueCreator& valueCreator)
		{
			KeyValueTraits::CopyKey(key, &mKeyBuffer);
			try
			{
				valueCreator(&mValueBuffer);
			}
			catch (...)
			{
				KeyValueTraits::DestroyKey(*&mKeyBuffer);
				throw;
			}
		}

		static void _Relocate(MapKeyValuePair& srcPair, MapKeyValuePair* dstPair,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/) MOMO_NOEXCEPT
		{
			KeyValueTraits::RelocateKeyNothrow(srcPair.GetKey(), &dstPair->mKeyBuffer);
			KeyValueTraits::RelocateValueNothrow(srcPair.GetValue(), &dstPair->mValueBuffer);
		}

		static void _Relocate(MapKeyValuePair& srcPair, MapKeyValuePair* dstPair,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			ValueCreator<Value>(std::move(srcPair.GetValue()))(&dstPair->mValueBuffer);
			KeyValueTraits::DestroyValue(srcPair.GetValue());
			KeyValueTraits::RelocateKeyNothrow(srcPair.GetKey(), &dstPair->mKeyBuffer);
		}

		static void _Relocate(MapKeyValuePair& srcPair, MapKeyValuePair* dstPair,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			KeyValueTraits::MoveKey(static_cast<const Key&>(srcPair.GetKey()),
				&dstPair->mKeyBuffer);
			KeyValueTraits::DestroyKey(srcPair.GetKey());
			KeyValueTraits::RelocateValueNothrow(srcPair.GetValue(), &dstPair->mValueBuffer);
		}

		static void _Relocate(MapKeyValuePair& srcPair, MapKeyValuePair* dstPair,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			KeyValueTraits::CopyKey(static_cast<const Key&>(srcPair.GetKey()),
				&dstPair->mKeyBuffer);
			try
			{
				ValueCreator<Value>(std::move(srcPair.GetValue()))(&dstPair->mValueBuffer);
			}
			catch (...)
			{
				KeyValueTraits::DestroyKey(&dstPair->mKeyBuffer);
				throw;
			}
			KeyValueTraits::DestroyKey(srcPair.GetKey());
			KeyValueTraits::DestroyValue(srcPair.GetValue());
		}

		template<typename Iterator, typename PairCreator>
		static void _RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const PairCreator& pairCreator, MapKeyValuePair* newPair,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			pairCreator(newPair);
			Iterator srcIter = srcBegin;
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, ++srcIter, ++dstIter)
			{
				KeyValueTraits::RelocateKeyNothrow(srcIter->GetKey(), &dstIter->mKeyBuffer);
				KeyValueTraits::RelocateValueNothrow(srcIter->GetValue(), &dstIter->mValueBuffer);
			}
		}

		template<typename Iterator, typename PairCreator>
		static void _RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const PairCreator& pairCreator, MapKeyValuePair* newPair,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				for (Iterator its = srcBegin, itd = dstBegin; index < count; ++index, ++its, ++itd)
					ValueCreator<const Value&>(its->GetValue())(&itd->mValueBuffer);
				pairCreator(newPair);
			}
			catch (...)
			{
				for (Iterator itd = dstBegin; index > 0; --index, ++itd)
					KeyValueTraits::DestroyValue(itd->GetValue());
				throw;
			}
			Iterator srcIter = srcBegin;
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, ++srcIter, ++dstIter)
			{
				KeyValueTraits::RelocateKeyNothrow(srcIter->GetKey(), &dstIter->mKeyBuffer);
				KeyValueTraits::DestroyValue(srcIter->GetValue());
			}
		}

		template<typename Iterator, typename PairCreator>
		static void _RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const PairCreator& pairCreator, MapKeyValuePair* newPair,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				for (Iterator its = srcBegin, itd = dstBegin; index < count; ++index, ++its, ++itd)
					KeyValueTraits::CopyKey(static_cast<const Key&>(its->GetKey()), &itd->mKeyBuffer);
				pairCreator(newPair);
			}
			catch (...)
			{
				for (Iterator itd = dstBegin; index > 0; --index, ++itd)
					KeyValueTraits::DestroyKey(itd->GetKey());
				throw;
			}
			Iterator srcIter = srcBegin;
			Iterator dstIter = dstBegin;
			for (size_t i = 0; i < count; ++i, ++srcIter, ++dstIter)
			{
				KeyValueTraits::DestroyKey(srcIter->GetKey());
				KeyValueTraits::RelocateValueNothrow(srcIter->GetValue(), &dstIter->mValueBuffer);
			}
		}

		template<typename Iterator, typename PairCreator>
		static void _RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const PairCreator& pairCreator, MapKeyValuePair* newPair,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t keyIndex = 0;
			size_t valueIndex = 0;
			try
			{
				for (Iterator its = srcBegin, itd = dstBegin; keyIndex < count; ++keyIndex, ++its, ++itd)
					KeyValueTraits::CopyKey(static_cast<const Key&>(its->GetKey()), &itd->mKeyBuffer);
				for (Iterator its = srcBegin, itd = dstBegin; valueIndex < count; ++valueIndex, ++its, ++itd)
					ValueCreator<const Value&>(its->GetValue())(&itd->mValueBuffer);
				pairCreator(newPair);
			}
			catch (...)
			{
				for (Iterator itd = dstBegin; keyIndex > 0; --keyIndex, ++itd)
					KeyValueTraits::DestroyKey(itd->GetKey());
				for (Iterator itd = dstBegin; valueIndex > 0; --valueIndex, ++itd)
					KeyValueTraits::DestroyValue(itd->GetValue());
				throw;
			}
			Iterator srcIter = srcBegin;
			for (size_t i = 0; i < count; ++i, ++srcIter)
				srcIter->~MapKeyValuePair();
		}

	private:
		internal::ObjectBuffer<Key, KeyValueTraits::keyAlignment> mKeyBuffer;
		mutable internal::ObjectBuffer<Value, KeyValueTraits::valueAlignment> mValueBuffer;
	};

	template<typename TKeyValuePair>
	struct MapNestedSetItemTraits
	{
		typedef TKeyValuePair KeyValuePair;

		typedef typename KeyValuePair::Key Key;
		typedef KeyValuePair Item;

		typedef internal::ObjectManager<Item> ItemManager;

		static const size_t alignment = ItemManager::alignment;

		template<typename ItemArg>
		class Creator : public ItemManager::template Creator<ItemArg>
		{
			MOMO_STATIC_ASSERT((std::is_same<ItemArg, const Item&>::value));

		private:
			typedef typename ItemManager::template Creator<ItemArg> BaseCreator;

		public:
			//using BaseCreator::BaseCreator;	// vs2013
			explicit Creator(ItemArg&& itemArg)
				: BaseCreator(std::forward<ItemArg>(itemArg))
			{
			}
		};

		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			return item.GetKey();
		}

		static void Destroy(Item& item) MOMO_NOEXCEPT
		{
			ItemManager::Destroy(item);
		}

		static void Relocate(Item& srcItem, Item* dstItem)
		{
			KeyValuePair::Relocate(srcItem, dstItem);
		}

		static void Replace(Item& srcItem, Item& dstItem)
		{
			KeyValuePair::Replace(srcItem, dstItem);
		}

		static void AssignKey(Key&& srcKey, Item& dstItem)
		{
			KeyValuePair::AssignKey(std::move(srcKey), dstItem);
		}

		static void AssignKey(const Key& srcKey, Item& dstItem)
		{
			KeyValuePair::AssignKey(srcKey, dstItem);
		}
	};

	template<typename TMap>
	struct MapValueReferencer
	{
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
