/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/ArrayBucket.h

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tMaxFastCount, typename TMemPoolParams, typename TArraySettings>
	class ArrayBucket
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef TMemPoolParams MemPoolParams;
		typedef TArraySettings ArraySettings;
		typedef typename ItemTraits::Item Item;

		static const size_t maxFastCount = tMaxFastCount;
		MOMO_STATIC_ASSERT(0 < maxFastCount && maxFastCount < 16);

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		struct ArrayItemTraits
		{
			typedef typename ItemTraits::Item Item;

			static const bool isTriviallyRelocatable = false;	//?

			template<typename ItemArg>
			class Creator
			{
				MOMO_STATIC_ASSERT((std::is_same<ItemArg, const Item&>::value));

			public:
				explicit Creator(const Item& item) MOMO_NOEXCEPT
					: mItem(item)
				{
				}

				void operator()(void* pitem) const
				{
					ItemTraits::Create(mItem, pitem);
				}

			private:
				const Item& mItem;
			};

			static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
			{
				ItemTraits::Destroy(items, count);
			}

			static void Relocate(Item* srcItems, Item* dstItems, size_t count)
			{
				ItemTraits::Relocate(srcItems, dstItems, count);
			}

			template<typename ItemCreator>
			static void RelocateCreate(Item* srcItems, Item* dstItems, size_t count,
				const ItemCreator& itemCreator, void* pitem)
			{
				ItemTraits::RelocateCreate(srcItems, dstItems, count, itemCreator, pitem);
			}
		};

		typedef momo::Array<Item, MemManagerPtr, ArrayItemTraits, ArraySettings> Array;

		static const size_t arrayAlignment = std::alignment_of<Array>::value;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, unsigned char*> Memory;

	public:
		class Params
		{
		private:
			typedef momo::Array<MemPool, MemManagerDummy, momo::ArrayItemTraits<MemPool>,
				momo::ArraySettings<maxFastCount>> MemPools;

		public:
			explicit Params(MemManager& memManager)
				: mArrayMemPool(MemPoolParams(sizeof(Array) + arrayAlignment, arrayAlignment),
					MemManagerPtr(memManager))
			{
				for (size_t i = 1; i <= maxFastCount; ++i)
				{
					size_t blockSize = i * sizeof(Item) + ItemTraits::alignment;
					mFastMemPools.AddBackNogrow(
						MemPool(MemPoolParams(blockSize, ItemTraits::alignment),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemPool& GetFastMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mFastMemPools[memPoolIndex - 1];
			}

			MemPool& GetArrayMemPool() MOMO_NOEXCEPT
			{
				return mArrayMemPool;
			}

		private:
			MemPools mFastMemPools;
			MemPool mArrayMemPool;
		};

	public:
		ArrayBucket() MOMO_NOEXCEPT
			: mPtr(nullptr)
		{
		}

		ArrayBucket(ArrayBucket&& bucket) MOMO_NOEXCEPT
			: mPtr(nullptr)
		{
			Swap(bucket);
		}

		ArrayBucket(const ArrayBucket& bucket) = delete;

		ArrayBucket(Params& params, const ArrayBucket& bucket)
		{
			ConstBounds bounds = bucket.GetBounds();
			size_t count = bounds.GetCount();
			if (count == 0)
			{
				mPtr = nullptr;
			}
			else if (count <= maxFastCount)
			{
				size_t memPoolIndex = _GetFastMemPoolIndex(count);
				Memory memory(params.GetFastMemPool(memPoolIndex));
				Item* items = _GetFastItems(memory.GetPointer());
				size_t index = 0;
				try
				{
					for (; index < count; ++index)
						ItemTraits::Create(bounds[index], items + index);
				}
				catch (...)
				{
					ItemTraits::Destroy(items, index);
					throw;
				}
				_Set(memory.Extract(), _MakeState(memPoolIndex, count));
			}
			else
			{
				MemPool& arrayMemPool = params.GetArrayMemPool();
				Memory memory(arrayMemPool);
				new(&_GetArray(memory.GetPointer())) Array(bounds.GetBegin(), bounds.GetEnd(),
					MemManagerPtr(arrayMemPool.GetMemManager()));
				_Set(memory.Extract(), (unsigned char)0);
			}
		}

		~ArrayBucket() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mPtr == nullptr);
		}

		ArrayBucket& operator=(ArrayBucket&& bucket) MOMO_NOEXCEPT
		{
			ArrayBucket(std::move(bucket)).Swap(*this);
			return *this;
		}

		ArrayBucket& operator=(const ArrayBucket& bucket) = delete;

		void Swap(ArrayBucket& bucket) MOMO_NOEXCEPT
		{
			std::swap(mPtr, bucket.mPtr);
		}

		ConstBounds GetBounds() const MOMO_NOEXCEPT
		{
			return _GetBounds();
		}

		Bounds GetBounds() MOMO_NOEXCEPT
		{
			return _GetBounds();
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
				return;
			size_t memPoolIndex = _GetMemPoolIndex();
			if (memPoolIndex > 0)
			{
				ItemTraits::Destroy(_GetFastItems(), _GetFastCount());
				params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
			}
			else
			{
				_GetArray().~Array();
				params.GetArrayMemPool().Deallocate(mPtr);
			}
			mPtr = nullptr;
		}

		template<typename ItemCreator>
		void AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			if (mPtr == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetFastMemPoolIndex(newCount);
				Memory memory(params.GetFastMemPool(newMemPoolIndex));
				itemCreator(_GetFastItems(memory.GetPointer()));
				_Set(memory.Extract(), _MakeState(newMemPoolIndex, newCount));
			}
			else
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				if (memPoolIndex > 0)
				{
					size_t count = _GetFastCount();
					MOMO_ASSERT(count <= memPoolIndex);
					if (count == memPoolIndex)
					{
						size_t newCount = count + 1;
						Item* items = _GetFastItems();
						if (newCount <= maxFastCount)
						{
							size_t newMemPoolIndex = _GetFastMemPoolIndex(newCount);
							Memory memory(params.GetFastMemPool(newMemPoolIndex));
							Item* newItems = _GetFastItems(memory.GetPointer());
							ItemTraits::RelocateCreate(items, newItems, count,
								itemCreator, newItems + count);
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							_Set(memory.Extract(), _MakeState(newMemPoolIndex, newCount));
						}
						else
						{
							MemPool& arrayMemPool = params.GetArrayMemPool();
							Memory memory(arrayMemPool);
							Array array = Array::CreateCap(maxFastCount * 2,
								MemManagerPtr(arrayMemPool.GetMemManager()));
							Item* newItems = array.GetItems();
							ItemTraits::RelocateCreate(items, newItems, count,
								itemCreator, newItems + count);
							array.SetCountCrt(newCount, [] (void* /*pitem*/) { });
							new(&_GetArray(memory.GetPointer())) Array(std::move(array));
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							_Set(memory.Extract(), (unsigned char)0);
						}
					}
					else
					{
						itemCreator(_GetFastItems() + count);
						++*mPtr;
					}
				}
				else
				{
					_GetArray().AddBackCrt(itemCreator);
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = GetBounds().GetCount();
			MOMO_ASSERT(count > 0);
			if (count == 1)
				return Clear(params);
			if (_GetMemPoolIndex() > 0)
			{
				ItemTraits::Destroy(_GetFastItems() + count - 1, 1);
				--*mPtr;
			}
			else
			{
				_GetArray().RemoveBack();
			}
		}

		void Shrink(Params& /*params*/) MOMO_NOEXCEPT
		{
			if (mPtr != nullptr && _GetMemPoolIndex() == 0)
			{
				try
				{
					Array& array = _GetArray();
					if (array.GetCount() < array.GetCapacity() / 2)
						array.Shrink();
				}
				catch (...)
				{
					// no throw!
				}
			}
		}

	private:
		void _Set(unsigned char* ptr, unsigned char state) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(ptr != nullptr);
			mPtr = ptr;
			*mPtr = state;
		}

		static unsigned char _MakeState(size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			return (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetFastMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxFastCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mPtr != nullptr);
			return (size_t)(*mPtr >> 4);
		}

		size_t _GetFastCount() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(_GetMemPoolIndex() > 0);
			return (size_t)(*mPtr & 15);
		}

		Item* _GetFastItems() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(_GetMemPoolIndex() > 0);
			return _GetFastItems(mPtr);
		}

		static Item* _GetFastItems(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return (Item*)(ptr + ItemTraits::alignment);
		}

		Array& _GetArray() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(_GetMemPoolIndex() == 0);
			return _GetArray(mPtr);
		}

		static Array& _GetArray(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return *(Array*)(ptr + arrayAlignment);
		}

		Bounds _GetBounds() const MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
			{
				return Bounds();
			}
			else if (_GetMemPoolIndex() > 0)
			{
				return Bounds(_GetFastItems(), _GetFastCount());
			}
			else
			{
				Array& array = _GetArray();
				return Bounds(array.GetItems(), array.GetCount());
			}
		}

	private:
		unsigned char* mPtr;
	};
}

} // namespace momo
