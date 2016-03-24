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
		template<typename Arg>
		static void AssignValue(Arg&& arg, Value& value)
		{
			value = std::forward<Arg>(arg);
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

		MapKeyValuePair(MapKeyValuePair&& pair)
		{
			_Create(std::move(pair.GetKey()), ValueCreator<Value>(std::move(pair.GetValue())),
				IsKeyNothrowMoveConstructible());
		}

		MapKeyValuePair(const MapKeyValuePair& pair)
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

		static void Assign(MapKeyValuePair&& srcPair, MapKeyValuePair& dstPair)
		{
			KeyValueTraits::AssignPair(std::move(srcPair.GetKey()), std::move(srcPair.GetValue()),
				dstPair.GetKey(), dstPair.GetValue());
		}

		static void SwapNothrowAnyway(MapKeyValuePair& pair1, MapKeyValuePair& pair2) MOMO_NOEXCEPT
		{
			KeyValueTraits::SwapKeysNothrowAnyway(pair1.GetKey(), pair2.GetKey());
			KeyValueTraits::SwapValuesNothrowAnyway(pair1.GetValue(), pair2.GetValue());
		}

		template<typename Iterator, typename PairCreator>
		static void RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const PairCreator& pairCreator, void* ppair)
		{
			_RelocateCreate(srcBegin, dstBegin, count, pairCreator, ppair,
				internal::BoolConstant<KeyValueTraits::isKeyNothrowRelocatable>(),
				internal::BoolConstant<KeyValueTraits::isValueNothrowRelocatable>());
		}

	private:
		template<typename ValueCreator>
		void _Create(Key&& key, const ValueCreator& valueCreator,
			std::true_type /*isKeyNothrowMoveConstructible*/)
		{
			valueCreator(&mValueBuffer);
			KeyValueTraits::CreateKeyNothrow(std::move(key), &mKeyBuffer);
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
			KeyValueTraits::CreateKey(key, &mKeyBuffer);
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

		template<typename Iterator, typename PairCreator>
		static void _RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const PairCreator& pairCreator, void* ppair,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			pairCreator(ppair);
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
			const PairCreator& pairCreator, void* ppair,
			std::true_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				for (Iterator its = srcBegin, itd = dstBegin; index < count; ++index, ++its, ++itd)
					ValueCreator<const Value&>(its->GetValue())(&itd->mValueBuffer);
				pairCreator(ppair);
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
			const PairCreator& pairCreator, void* ppair,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::true_type /*isValueNothrowRelocatable*/)
		{
			size_t index = 0;
			try
			{
				for (Iterator its = srcBegin, itd = dstBegin; index < count; ++index, ++its, ++itd)
					KeyValueTraits::CreateKey(static_cast<const Key&>(its->GetKey()), &itd->mKeyBuffer);
				pairCreator(ppair);
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
			const PairCreator& pairCreator, void* ppair,
			std::false_type /*isKeyNothrowRelocatable*/,
			std::false_type /*isValueNothrowRelocatable*/)
		{
			size_t keyIndex = 0;
			size_t valueIndex = 0;
			try
			{
				for (Iterator its = srcBegin, itd = dstBegin; keyIndex < count; ++keyIndex, ++its, ++itd)
					KeyValueTraits::CreateKey(static_cast<const Key&>(its->GetKey()), &itd->mKeyBuffer);
				for (Iterator its = srcBegin, itd = dstBegin; valueIndex < count; ++valueIndex, ++its, ++itd)
					ValueCreator<const Value&>(its->GetValue())(&itd->mValueBuffer);
				pairCreator(ppair);
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
			ValueReference(Map& map, Iterator iter, PKey pkey)
				: mMap(map),
				mIter(iter),
				mKeyPtr(pkey)
			{
			}

			ValueReference(ValueReference&& valueRef)
				: mMap(valueRef.mMap),
				mIter(valueRef.mIter),
				mKeyPtr(valueRef.mKeyPtr)
			{
			}

			ValueReference(const ValueReference&) = delete;

			ValueReference& operator=(ValueReference&& valueRef)
			{
				return _Assign(std::move(valueRef.Get()));
			}

			ValueReference& operator=(const ValueReference& valueRef)
			{
				return _Assign(valueRef.Get());
			}

			template<typename Arg>
			ValueReference& operator=(Arg&& arg)
			{
				return _Assign(std::forward<Arg>(arg));
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
			template<typename Arg>
			ValueReference& _Assign(Arg&& arg)
			{
				if (mKeyPtr == nullptr)
				{
					KeyValueTraits::AssignValue(std::forward<Arg>(arg), mIter->value);
				}
				else
				{
					mIter = mMap.AddCrt(mIter, std::forward<RKey>(*mKeyPtr),
						typename KeyValueTraits::template ValueCreator<Arg>(std::forward<Arg>(arg)));
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
