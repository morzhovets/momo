/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MapUtility.h

  namespace momo:
    concept conceptMapKeyValueTraits

\**********************************************************/

#pragma once

#include "ObjectManager.h"
#include "IteratorUtility.h"
#include "MemPool.h"

namespace momo
{

template<typename MapKeyValueTraits, typename Key, typename Value, typename MemManager>
concept conceptMapKeyValueTraits =
	std::is_same_v<typename MapKeyValueTraits::Key, Key> &&
	std::is_same_v<typename MapKeyValueTraits::Value, Value> &&
	std::is_same_v<typename MapKeyValueTraits::MemManager, MemManager> &&
	requires (Key& key, Value& value, MemManager& memManager)
	{
		typename std::bool_constant<MapKeyValueTraits::useValuePtr>;
		typename std::integral_constant<size_t, MapKeyValueTraits::keyAlignment>;
		typename std::integral_constant<size_t, MapKeyValueTraits::valueAlignment>;
		{ MapKeyValueTraits::DestroyKey(&memManager, key) } noexcept;
		{ MapKeyValueTraits::DestroyValue(&memManager, value) } noexcept;
	} &&
	internal::ObjectAlignmenter<Key>::Check(MapKeyValueTraits::keyAlignment) &&
	internal::ObjectAlignmenter<Value>::Check(MapKeyValueTraits::valueAlignment);

namespace internal
{
	template<typename TSetReference,
		bool tIsConst = false>
	class MapReference
	{
	protected:
		typedef TSetReference SetReference;
		typedef std::decay_t<SetReference> KeyValuePair;

		static const bool isConst = tIsConst;

	public:
		typedef typename KeyValuePair::Key Key;
		typedef std::conditional_t<isConst, const typename KeyValuePair::Value,
			typename KeyValuePair::Value> Value;

		typedef MapReference<SetReference, true> ConstReference;

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

	template<typename TMapReference>
	class MapReferenceStd
		: public std::pair<const typename TMapReference::Key&, typename TMapReference::Value&>
	{
	protected:
		typedef TMapReference MapReference;

	public:
		typedef typename MapReference::Key Key;
		typedef typename MapReference::Value Value;

		typedef MapReferenceStd<typename MapReference::ConstReference> ConstReference;

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

		bool operator==(const MapReferenceStd& ref) const
		{
			return RefPair::first == ref.first && RefPair::second == ref.second;
		}

		bool operator==(const std::pair<const Key, Value>& pair) const
		{
			return RefPair::first == pair.first && RefPair::second == pair.second;
		}

		//operator<=>

	protected:
		explicit MapReferenceStd(MapReference mapRef) noexcept
			: RefPair(mapRef.key, mapRef.value)
		{
		}
	};

	template<typename TSetIterator,
		bool tIsConst = false>
	class MapBidirectionalIterator
	{
	protected:
		typedef TSetIterator SetIterator;

		static const bool isConst = tIsConst;

	public:
		typedef MapReference<typename SetIterator::Reference, isConst> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef MapBidirectionalIterator<SetIterator, true> ConstIterator;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit MapBidirectionalIterator() noexcept
			: mSetIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mSetIterator);
		}

		MapBidirectionalIterator& operator++()
		{
			++mSetIterator;
			return *this;
		}

		MapBidirectionalIterator& operator--()
		{
			--mSetIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mSetIterator));
		}

		friend bool operator==(MapBidirectionalIterator iter1,
			MapBidirectionalIterator iter2) noexcept
		{
			return iter1.mSetIterator == iter2.mSetIterator;
		}

		MOMO_MORE_BIDIRECTIONAL_ITERATOR_OPERATORS(MapBidirectionalIterator)

	protected:
		explicit MapBidirectionalIterator(SetIterator setIter) noexcept
			: mSetIterator(setIter)
		{
		}

		SetIterator ptGetSetIterator() const noexcept
		{
			return mSetIterator;
		}

	private:
		SetIterator mSetIterator;
	};

	template<typename TSetIterator,
		bool tIsConst = false>
	class MapForwardIterator
	{
	protected:
		typedef TSetIterator SetIterator;

		static const bool isConst = tIsConst;

	public:
		typedef MapReference<std::iter_reference_t<SetIterator>, isConst> Reference;
		typedef IteratorPointer<Reference> Pointer;

		typedef MapForwardIterator<SetIterator, true> ConstIterator;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit MapForwardIterator() noexcept
			: mSetIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mSetIterator);
		}

		MapForwardIterator& operator++()
		{
			++mSetIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mSetIterator));
		}

		friend bool operator==(MapForwardIterator iter1, MapForwardIterator iter2) noexcept
		{
			return iter1.mSetIterator == iter2.mSetIterator;
		}

		MOMO_MORE_FORWARD_ITERATOR_OPERATORS(MapForwardIterator)

	protected:
		explicit MapForwardIterator(SetIterator setIter) noexcept
			: mSetIterator(setIter)
		{
		}

		SetIterator ptGetSetIterator() const noexcept
		{
			return mSetIterator;
		}

	private:
		SetIterator mSetIterator;
	};

	template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager>
	class MapKeyValueTraitsBase
	{
	public:
		typedef TKey Key;
		typedef TValue Value;
		typedef TMemManager MemManager;

	protected:
		typedef ObjectManager<Key, MemManager> KeyManager;
		typedef ObjectManager<Value, MemManager> ValueManager;

	public:
		static const size_t keyAlignment = KeyManager::alignment;
		static const size_t valueAlignment = ValueManager::alignment;

		static const bool isKeyNothrowRelocatable = KeyManager::isNothrowRelocatable;

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

		static void DestroyKey(MemManager* memManager, Key& key) noexcept
		{
			KeyManager::Destroyer::Destroy(memManager, key);
		}

		static void DestroyValue(MemManager* memManager, Value& value) noexcept
		{
			ValueManager::Destroyer::Destroy(memManager, value);
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
	};

	template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager,
		bool tUseValuePtr = false>
	class MapKeyValueTraits;

	template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager>
	class MapKeyValueTraits<TKey, TValue, TMemManager, false>
		: public MapKeyValueTraitsBase<TKey, TValue, TMemManager>
	{
	private:
		typedef internal::MapKeyValueTraitsBase<TKey, TValue, TMemManager> MapKeyValueTraitsBase;

	public:
		using typename MapKeyValueTraitsBase::Key;
		using typename MapKeyValueTraitsBase::Value;
		using typename MapKeyValueTraitsBase::MemManager;

		static const bool useValuePtr = false;

	private:
		using typename MapKeyValueTraitsBase::KeyManager;
		using typename MapKeyValueTraitsBase::ValueManager;

	public:
		using MapKeyValueTraitsBase::isKeyNothrowRelocatable;
		static const bool isValueNothrowRelocatable = ValueManager::isNothrowRelocatable;

	public:
		static void Relocate(MemManager* memManager, Key& srcKey, Value& srcValue,
			Key* dstKey, Value* dstValue)
		{
			if constexpr (isKeyNothrowRelocatable)
			{
				ValueManager::Relocator::Relocate(memManager, srcValue, dstValue);
				KeyManager::Relocator::Relocate(memManager, srcKey, dstKey);
			}
			else if constexpr (isValueNothrowRelocatable)
			{
				KeyManager::Relocator::Relocate(memManager, srcKey, dstKey);
				ValueManager::Relocator::Relocate(memManager, srcValue, dstValue);
			}
			else
			{
				if (memManager != nullptr)
					KeyManager::Copy(*memManager, srcKey, dstKey);
				else
					std::construct_at(dstKey, std::as_const(srcKey));	//?
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
		}

		static void Replace(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue)
		{
			if constexpr (KeyManager::isNothrowAnywayAssignable)
			{
				ValueManager::Replace(memManager, srcValue, dstValue);
				KeyManager::Replace(memManager, srcKey, dstKey);
			}
			else if constexpr (ValueManager::isNothrowAnywayAssignable)
			{
				KeyManager::Replace(memManager, srcKey, dstKey);
				ValueManager::Replace(memManager, srcValue, dstValue);
			}
			else
			{
				pvReplaceUnsafe(memManager, srcKey, srcValue, dstKey, dstValue);
			}
		}

		static void ReplaceRelocate(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& midKey, Value& midValue, Key* dstKey, Value* dstValue)
		{
			MOMO_ASSERT(std::addressof(srcKey) != std::addressof(midKey));
			MOMO_ASSERT(std::addressof(srcValue) != std::addressof(midValue));
			if constexpr (isKeyNothrowRelocatable)
			{
				ValueManager::ReplaceRelocate(memManager, srcValue, midValue, dstValue);
				KeyManager::ReplaceRelocate(memManager, srcKey, midKey, dstKey);
			}
			else if constexpr (isValueNothrowRelocatable)
			{
				KeyManager::ReplaceRelocate(memManager, srcKey, midKey, dstKey);
				ValueManager::ReplaceRelocate(memManager, srcValue, midValue, dstValue);
			}
			else if constexpr (KeyManager::isNothrowAnywayAssignable)
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
			else if constexpr (ValueManager::isNothrowAnywayAssignable)
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
			else
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
		}

		template<typename SrcKeyIterator, typename SrcValueIterator,
			typename DstKeyIterator, typename DstValueIterator, typename Func>
		static void RelocateExec(MemManager& memManager,
			SrcKeyIterator srcKeyBegin, SrcValueIterator srcValueBegin,
			DstKeyIterator dstKeyBegin, DstValueIterator dstValueBegin, size_t count, Func&& func)
		{
			if constexpr (isKeyNothrowRelocatable)
			{
				ValueManager::RelocateExec(memManager, srcValueBegin, dstValueBegin, count,
					std::forward<Func>(func));
				KeyManager::Relocate(memManager, srcKeyBegin, dstKeyBegin, count);
			}
			else if constexpr (isValueNothrowRelocatable)
			{
				KeyManager::RelocateExec(memManager, srcKeyBegin, dstKeyBegin, count,
					std::forward<Func>(func));
				ValueManager::Relocate(memManager, srcValueBegin, dstValueBegin, count);
			}
			else
			{
				size_t keyIndex = 0;
				size_t valueIndex = 0;
				try
				{
					SrcKeyIterator srcKeyIter = srcKeyBegin;
					DstKeyIterator dstKeyIter = dstKeyBegin;
					for (; keyIndex < count; ++keyIndex, (void)++srcKeyIter, (void)++dstKeyIter)
						KeyManager::Copy(memManager, *srcKeyIter, std::addressof(*dstKeyIter));
					SrcValueIterator srcValueIter = srcValueBegin;
					DstValueIterator dstValueIter = dstValueBegin;
					for (; valueIndex < count; ++valueIndex, (void)++srcValueIter, (void)++dstValueIter)
						ValueManager::Copy(memManager, *srcValueIter, std::addressof(*dstValueIter));
					std::forward<Func>(func)();
				}
				catch (...)
				{
					for (DstKeyIterator itd = dstKeyBegin; keyIndex > 0; --keyIndex, (void)++itd)
						KeyManager::Destroy(memManager, *itd);
					for (DstValueIterator itd = dstValueBegin; valueIndex > 0; --valueIndex, (void)++itd)
						ValueManager::Destroy(memManager, *itd);
					throw;
				}
				for (SrcKeyIterator its = srcKeyBegin; keyIndex > 0; --keyIndex, (void)++its)
					KeyManager::Destroy(memManager, *its);
				for (SrcValueIterator its = srcValueBegin; valueIndex > 0; --valueIndex, (void)++its)
					ValueManager::Destroy(memManager, *its);
			}
		}

	private:
		static void pvReplaceUnsafe(MemManager& memManager, Key& srcKey, Value& srcValue,
			Key& dstKey, Value& dstValue)
		{
			// basic exception safety
			dstValue = srcValue;	//?
			dstKey = std::move(srcKey);
			KeyManager::Destroy(memManager, srcKey);
			ValueManager::Destroy(memManager, srcValue);
		}
	};

	template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager>
	class MapKeyValueTraits<TKey, TValue, TMemManager, true>
		: public MapKeyValueTraitsBase<TKey, TValue, TMemManager>
	{
	private:
		typedef internal::MapKeyValueTraitsBase<TKey, TValue, TMemManager> MapKeyValueTraitsBase;

	public:
		using typename MapKeyValueTraitsBase::Key;
		using typename MapKeyValueTraitsBase::Value;
		using typename MapKeyValueTraitsBase::MemManager;

		static const bool useValuePtr = true;

	private:
		using typename MapKeyValueTraitsBase::KeyManager;
		using typename MapKeyValueTraitsBase::ValueManager;

	public:
		static_assert(sizeof(Value) >= sizeof(void*));
		typedef MemPoolParamsStatic<sizeof(Value),
			MapKeyValueTraitsBase::valueAlignment> ValueMemPoolParams;

	public:
		static void RelocateKey(MemManager* memManager, Key& srcKey, Key* dstKey)
		{
			KeyManager::Relocator::Relocate(memManager, srcKey, dstKey);
		}

		static void ReplaceKey(MemManager& memManager, Key& srcKey, Key& dstKey)
		{
			KeyManager::Replace(memManager, srcKey, dstKey);
		}

		static void ReplaceRelocateKeys(MemManager& memManager, Key& srcKey,
			Key& midKey, Key* dstKey)
		{
			KeyManager::ReplaceRelocate(memManager, srcKey, midKey, dstKey);
		}

		template<typename SrcKeyIterator, typename DstKeyIterator, typename Func>
		static void RelocateExecKeys(MemManager& memManager,
			SrcKeyIterator srcKeyBegin, DstKeyIterator dstKeyBegin, size_t count, Func&& func)
		{
			KeyManager::RelocateExec(memManager, srcKeyBegin, dstKeyBegin, count,
				std::forward<Func>(func));
		}
	};

	template<typename TKey, typename TValue, size_t tKeyAlignment, size_t tValueAlignment>
	class MapKeyValuePair
	{
	public:
		typedef TKey Key;
		typedef TValue Value;

	protected:
		static const size_t keyAlignment = tKeyAlignment;
		static const size_t valueAlignment = tValueAlignment;

	public:
		MapKeyValuePair() = delete;

		MapKeyValuePair(const MapKeyValuePair&) = delete;

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
		ObjectBuffer<Key, keyAlignment> mKeyBuffer;
		mutable ObjectBuffer<Value, valueAlignment> mValueBuffer;
	};

	template<typename TKey, typename TValue, size_t tKeyAlignment>
	class MapKeyValuePtrPair
	{
	public:
		typedef TKey Key;
		typedef TValue Value;

	protected:
		static const size_t keyAlignment = tKeyAlignment;

	public:
		MapKeyValuePtrPair() = delete;

		MapKeyValuePtrPair(const MapKeyValuePtrPair&) = delete;

		~MapKeyValuePtrPair() = delete;

		MapKeyValuePtrPair& operator=(const MapKeyValuePtrPair&) = delete;

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
			return mValuePtr;
		}

		Value*& GetValuePtr() noexcept
		{
			return mValuePtr;
		}

	private:
		ObjectBuffer<Key, keyAlignment> mKeyBuffer;
		Value* mValuePtr;
	};

	template<typename TItemTraits, typename TRKey, typename TValueCreator>
	class MapNestedSetItemCreator : private TItemTraits
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TRKey RKey;
		typedef TValueCreator ValueCreator;

		using typename ItemTraits::Item;
		using typename ItemTraits::MemManager;

	private:
		using typename ItemTraits::KeyValueTraits;
		using typename ItemTraits::Value;

	public:
		explicit MapNestedSetItemCreator(MemManager& memManager,
			RKey&& key, ValueCreator&& valueCreator) noexcept
			: mMemManager(memManager),
			mKey(std::forward<RKey>(key)),
			mValueCreator(std::forward<ValueCreator>(valueCreator))
		{
		}

		void operator()(Item* newItem) &&
		{
			if constexpr (KeyValueTraits::useValuePtr)
			{
				Value* valuePtr = mMemManager.GetMemPool().template Allocate<Value>();
				newItem->GetValuePtr() = valuePtr;
				try
				{
					pvCreate(newItem);
				}
				catch (...)
				{
					mMemManager.GetMemPool().Deallocate(valuePtr);
					throw;
				}
			}
			else
			{
				pvCreate(newItem);
			}
		}

	private:
		void pvCreate(Item* newItem)
		{
			KeyValueTraits::Create(mMemManager, std::forward<RKey>(mKey),
				std::forward<ValueCreator>(mValueCreator), newItem->GetKeyPtr(),
				newItem->GetValuePtr());
		}

	private:
		MemManager& mMemManager;
		RKey&& mKey;
		ValueCreator&& mValueCreator;
	};

	template<typename TKeyValueTraits>
	class MapNestedSetItemTraits;

	template<typename TKeyValueTraits>
	requires (!TKeyValueTraits::useValuePtr)
	class MapNestedSetItemTraits<TKeyValueTraits>
	{
	protected:
		typedef TKeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Value Value;

	public:
		typedef typename KeyValueTraits::Key Key;
		typedef typename KeyValueTraits::MemManager MemManager;

		typedef MapKeyValuePair<Key, Value,
			KeyValueTraits::keyAlignment, KeyValueTraits::valueAlignment> Item;

		static const size_t alignment = ObjectAlignmenter<Item>::alignment;

		static const bool isNothrowRelocatable =
			KeyValueTraits::isKeyNothrowRelocatable && KeyValueTraits::isValueNothrowRelocatable;

		template<typename ItemArg>
		requires std::is_same_v<ItemArg, const Item&>
		class Creator
		{
		public:
			explicit Creator(MemManager& memManager, const Item& item) noexcept
				: mMemManager(memManager),
				mItem(item)
			{
			}

			void operator()(Item* newItem) &&
			{
				typedef typename KeyValueTraits::template ValueCreator<const Value&> ValueCreator;
				ValueCreator valueCreator(mMemManager, *mItem.GetValuePtr());
				MapNestedSetItemCreator<MapNestedSetItemTraits, const Key&, ValueCreator>(
					mMemManager, *mItem.GetKeyPtr(), std::move(valueCreator))(newItem);
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
			KeyValueTraits::DestroyKey(memManager, *item.GetKeyPtr());
			KeyValueTraits::DestroyValue(memManager, *item.GetValuePtr());
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

		template<typename SrcIterator, typename DstIterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			auto srcKeyGen = [srcIter = srcBegin] () mutable { return ptGenerateKeyPtr(srcIter); };
			auto dstKeyGen = [dstIter = dstBegin] () mutable { return ptGenerateKeyPtr(dstIter); };
			auto srcValueGen = [srcIter = srcBegin] () mutable { return ptGenerateValuePtr(srcIter); };
			auto dstValueGen = [dstIter = dstBegin] () mutable { return ptGenerateValuePtr(dstIter); };
			auto func = [&itemCreator, newItem] ()
				{ std::forward<ItemCreator>(itemCreator)(newItem); };
			KeyValueTraits::RelocateExec(memManager,
				InputIterator(srcKeyGen), InputIterator(srcValueGen),
				InputIterator(dstKeyGen), InputIterator(dstValueGen), count, func);
		}

		template<typename KeyArg>
		static void AssignKey(MemManager& memManager, KeyArg&& keyArg, Item& item)
		{
			KeyValueTraits::AssignKey(memManager, std::forward<KeyArg>(keyArg), *item.GetKeyPtr());
		}

	protected:
		template<typename Iterator>
		static Key* ptGenerateKeyPtr(Iterator& iter) noexcept
		{
			Key* keyPtr = iter->GetKeyPtr();
			++iter;
			return keyPtr;
		}

		template<typename Iterator>
		static Value* ptGenerateValuePtr(Iterator& iter) noexcept
		{
			Value* valuePtr = iter->GetValuePtr();
			++iter;
			return valuePtr;
		}
	};

	template<typename TKeyValueTraits>
	requires (TKeyValueTraits::useValuePtr)
	class MapNestedSetItemTraits<TKeyValueTraits>
	{
	protected:
		typedef TKeyValueTraits KeyValueTraits;
		typedef typename KeyValueTraits::Value Value;

	public:
		typedef typename KeyValueTraits::Key Key;
		typedef MemManagerPoolLazy<typename KeyValueTraits::MemManager,
			typename KeyValueTraits::ValueMemPoolParams> MemManager;

		typedef MapKeyValuePtrPair<Key, Value, KeyValueTraits::keyAlignment> Item;

		static const size_t alignment = ObjectAlignmenter<Item>::alignment;

		static const bool isNothrowRelocatable = KeyValueTraits::isKeyNothrowRelocatable;

		template<typename ItemArg>
		requires std::is_same_v<ItemArg, const Item&>
		class Creator
		{
		public:
			explicit Creator(MemManager& memManager, const Item& item) noexcept
				: mMemManager(memManager),
				mItem(item)
			{
			}

			void operator()(Item* newItem) &&
			{
				typedef typename KeyValueTraits::template ValueCreator<const Value&> ValueCreator;
				ValueCreator valueCreator(mMemManager, *mItem.GetValuePtr());
				MapNestedSetItemCreator<MapNestedSetItemTraits, const Key&, ValueCreator>(
					mMemManager, *mItem.GetKeyPtr(), std::move(valueCreator))(newItem);
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
			KeyValueTraits::DestroyKey(memManager, *item.GetKeyPtr());
			Value* valuePtr = item.GetValuePtr();
			KeyValueTraits::DestroyValue(memManager, *valuePtr);
			if (memManager != nullptr) [[likely]]
				memManager->GetMemPool().Deallocate(valuePtr);
		}

		static void Relocate(MemManager* memManager, Item& srcItem, Item* dstItem)
		{
			KeyValueTraits::RelocateKey(memManager, *srcItem.GetKeyPtr(), dstItem->GetKeyPtr());
			dstItem->GetValuePtr() = srcItem.GetValuePtr();
		}

		static void Replace(MemManager& memManager, Item& srcItem, Item& dstItem)
		{
			KeyValueTraits::ReplaceKey(memManager, *srcItem.GetKeyPtr(), *dstItem.GetKeyPtr());
			Value*& dstValuePtr = dstItem.GetValuePtr();
			KeyValueTraits::DestroyValue(&memManager, *dstValuePtr);
			memManager.GetMemPool().Deallocate(dstValuePtr);
			dstValuePtr = srcItem.GetValuePtr();
		}

		static void ReplaceRelocate(MemManager& memManager, Item& srcItem, Item& midItem,
			Item* dstItem)
		{
			KeyValueTraits::ReplaceRelocateKeys(memManager, *srcItem.GetKeyPtr(),
				*midItem.GetKeyPtr(), dstItem->GetKeyPtr());
			dstItem->GetValuePtr() = midItem.GetValuePtr();
			midItem.GetValuePtr() = srcItem.GetValuePtr();
		}

		template<typename SrcIterator, typename DstIterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			auto srcKeyGen = [srcIter = srcBegin] () mutable { return ptGenerateKeyPtr(srcIter); };
			auto dstKeyGen = [dstIter = dstBegin] () mutable { return ptGenerateKeyPtr(dstIter); };
			auto func = [&itemCreator, newItem] ()
				{ std::forward<ItemCreator>(itemCreator)(newItem); };
			KeyValueTraits::RelocateExecKeys(memManager,
				InputIterator(srcKeyGen), InputIterator(dstKeyGen), count, func);
			auto srcValueGen = [srcIter = srcBegin] () mutable
				{ return ptGenerateValuePtrPtr(srcIter); };
			auto dstValueGen = [dstIter = dstBegin] () mutable
				{ return ptGenerateValuePtrPtr(dstIter); };
			ObjectManager<Value*, MemManager>::Relocate(memManager,
				InputIterator(srcValueGen), InputIterator(dstValueGen), count);
		}

		template<typename KeyArg>
		static void AssignKey(MemManager& memManager, KeyArg&& keyArg, Item& item)
		{
			KeyValueTraits::AssignKey(memManager, std::forward<KeyArg>(keyArg), *item.GetKeyPtr());
		}

	protected:
		template<typename Iterator>
		static Key* ptGenerateKeyPtr(Iterator& iter) noexcept
		{
			Key* keyPtr = iter->GetKeyPtr();
			++iter;
			return keyPtr;
		}

		template<typename Iterator>
		static Value** ptGenerateValuePtrPtr(Iterator& iter) noexcept
		{
			Value** valuePtrPtr = &iter->GetValuePtr();
			++iter;
			return valuePtrPtr;
		}
	};

	template<typename TSetExtractedItem, bool tUseValuePtr>
	class MapExtractedPair;

	template<typename TSetExtractedItem>
	class MapExtractedPair<TSetExtractedItem, false>
	{
	protected:
		typedef TSetExtractedItem SetExtractedItem;
		typedef typename SetExtractedItem::Item KeyValuePair;

		static const bool useValuePtr = false;

	public:
		typedef typename KeyValuePair::Key Key;
		typedef typename KeyValuePair::Value Value;

	public:
		explicit MapExtractedPair() = default;

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

		~MapExtractedPair() = default;

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

		template<std::invocable<Key&, Value&> PairRemover>
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

	template<typename TSetExtractedItem>
	class MapExtractedPair<TSetExtractedItem, true>
	{
	protected:
		typedef TSetExtractedItem SetExtractedItem;
		typedef typename SetExtractedItem::Item KeyValuePair;

		static const bool useValuePtr = true;

		typedef typename SetExtractedItem::ItemTraits::MemManager::MemPool ValueMemPool;

	public:
		typedef typename KeyValuePair::Key Key;
		typedef typename KeyValuePair::Value Value;

	public:
		explicit MapExtractedPair()
			: mValueMemPool(nullptr)
		{
		}

		template<typename Map>
		explicit MapExtractedPair(Map& map, typename Map::ConstIterator iter)
			: mValueMemPool(nullptr)
		{
			map.Remove(iter, *this);
		}

		MapExtractedPair(MapExtractedPair&& extractedPair)
			noexcept(noexcept(SetExtractedItem(std::declval<SetExtractedItem&&>())))
			: mSetExtractedItem(std::move(extractedPair.mSetExtractedItem)),
			mValueMemPool(extractedPair.mValueMemPool)
		{
		}

		MapExtractedPair(const MapExtractedPair&) = delete;

		~MapExtractedPair() noexcept
		{
			Clear();
		}

		MapExtractedPair& operator=(const MapExtractedPair&) = delete;

		bool IsEmpty() const noexcept
		{
			return mSetExtractedItem.IsEmpty();
		}

		void Clear() noexcept
		{
			if (!IsEmpty())
			{
				MOMO_ASSERT(mValueMemPool != nullptr);
				Value* valuePtr = mSetExtractedItem.GetItem().GetValuePtr();
				mSetExtractedItem.Clear();
				mValueMemPool->DeallocateLazy(valuePtr);
			}
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

		template<std::invocable<Key&, Value&> PairRemover>
		void Remove(PairRemover&& pairRemover)
		{
			auto itemRemover = [this, &pairRemover] (KeyValuePair& item)
			{
				MOMO_ASSERT(mValueMemPool != nullptr);
				Value* valuePtr = mSetExtractedItem.GetItem().GetValuePtr();
				std::forward<PairRemover>(pairRemover)(*item.GetKeyPtr(), *valuePtr);
				mValueMemPool->DeallocateLazy(valuePtr);
			};
			mSetExtractedItem.Remove(itemRemover);
		}

	protected:
		SetExtractedItem& ptGetSetExtractedItem() noexcept
		{
			return mSetExtractedItem;
		}

		ValueMemPool*& ptGetValueMemPool() noexcept
		{
			return mValueMemPool;
		}

	private:
		SetExtractedItem mSetExtractedItem;
		ValueMemPool* mValueMemPool;
	};

	template<typename TMap, typename TPosition>
	class MapValueReferencer
	{
	public:
		typedef TMap Map;
		typedef TPosition Position;

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

			typedef typename Map::Settings Settings;

		public:
			typedef const Value& ConstReference;

		private:
			typedef std::add_pointer_t<KeyReference> KeyPointer;

		public:
			ValueReference() = delete;

			ValueReference(const ValueReference& valueRef) noexcept
				: mMap(valueRef.mMap),
				mPosition(valueRef.mPosition),
				mKeyPtr(valueRef.mKeyPtr)
			{
			}

			~ValueReference() = default;

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
				return mPosition->value;
			}

		protected:
			explicit ValueReference(Map& map, Position pos) noexcept
				: mMap(map),
				mPosition(pos),
				mKeyPtr(nullptr)
			{
			}

			explicit ValueReference(Map& map, Position pos, KeyReference keyRef) noexcept
				: mMap(map),
				mPosition(pos),
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
						std::forward<ValueArg>(valueArg), mPosition->value);
				}
				else
				{
					typename KeyValueTraits::template ValueCreator<ValueArg> valueCreator(
						mMap.GetMemManager(), std::forward<ValueArg>(valueArg));
					mPosition = mMap.AddCrt(mPosition, std::forward<KeyReference>(*mKeyPtr),
						std::move(valueCreator));
				}
				mKeyPtr = nullptr;
				return *this;
			}

		private:
			Map& mMap;
			Position mPosition;
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
		static ValueReference<KeyReference> GetReference(Map& map, Position pos) noexcept
		{
			return ValueReferenceProxy<KeyReference>(map, pos);
		}

		template<typename KeyReference>
		static ValueReference<KeyReference> GetReference(Map& map, Position pos,
			KeyReference keyRef) noexcept
		{
			return ValueReferenceProxy<KeyReference>(map, pos,
				std::forward<KeyReference>(keyRef));
		}
#else
	public:
		template<typename KeyReference>
		using ValueReference = Value&;

	public:
		template<typename KeyReference>
		static ValueReference<KeyReference> GetReference(Map& /*map*/, Position pos) noexcept
		{
			return pos->value;
		}

		template<typename KeyReference>
		static ValueReference<KeyReference> GetReference(Map& map, Position pos,
			KeyReference keyRef)
		{
			typename KeyValueTraits::template ValueCreator<> valueCreator(map.GetMemManager());
			return map.AddCrt(pos, std::forward<KeyReference>(keyRef),
				std::move(valueCreator))->value;
		}
#endif
	};

	template<bool tAllowKeyValue = true>
	class MapArgReferencer
	{
	public:
		static const bool allowKeyValue = tAllowKeyValue;

	public:
		template<typename ArgIterator,
			typename KeyArg = decltype(std::declval<ArgIterator>()->key),
			typename ValueArg = decltype(std::declval<ArgIterator>()->value)>
		requires std::is_reference_v<KeyArg> && std::is_reference_v<ValueArg>
		static auto GetReferencePair(ArgIterator iter) noexcept requires allowKeyValue
		{
			return std::pair<KeyArg, ValueArg>(iter->key, iter->value);
		}

		template<typename ArgIterator,
			typename KeyArg = decltype(std::declval<ArgIterator>()->first),
			typename ValueArg = decltype(std::declval<ArgIterator>()->second)>
		requires std::is_reference_v<std::iter_reference_t<ArgIterator>> ||
			(std::is_reference_v<KeyArg> && std::is_reference_v<ValueArg>)
		static auto GetReferencePair(ArgIterator iter) noexcept
		{
			return pvGetReferencePair<KeyArg, ValueArg>(*iter);
		}

	private:
		template<typename KeyArg, typename ValueArg, typename Pair>
		static auto pvGetReferencePair(Pair&& pair) noexcept
		{
			return std::pair<KeyArg&&, ValueArg&&>(std::forward<KeyArg>(pair.first),
				std::forward<ValueArg>(pair.second));
		}

		template<typename KeyArg, typename ValueArg, typename Pair>
		static auto pvGetReferencePair(const Pair& pair) noexcept
		{
			return std::pair<const KeyArg&, const ValueArg&>(pair.first, pair.second);
		}
	};

	template<typename MapArgIterator, typename Key,
		bool allowKeyValue = true>
	concept conceptMapArgIterator = conceptInputIterator<MapArgIterator> &&
		std::is_same_v<Key, std::decay_t<decltype(
			MapArgReferencer<allowKeyValue>::GetReferencePair(std::declval<MapArgIterator>()).first)>>;
}

} // namespace momo

namespace std
{
	template<typename SI, bool c>
	struct iterator_traits<momo::internal::MapBidirectionalIterator<SI, c>>
		: public momo::internal::IteratorTraitsStd<momo::internal::MapBidirectionalIterator<SI, c>,
			bidirectional_iterator_tag>
	{
	};

	template<typename SI, bool c>
	struct iterator_traits<momo::internal::MapForwardIterator<SI, c>>
		: public momo::internal::IteratorTraitsStd<momo::internal::MapForwardIterator<SI, c>,
			forward_iterator_tag>
	{
	};
} // namespace std
