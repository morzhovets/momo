/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketUnlimP.h

  namespace momo:
    class HashBucketUnlimP

\**********************************************************/

#pragma once

#include "ArrayBucket.h"

namespace momo
{

namespace internal
{
	template<typename TBucketUnlimPItemTraits>
	class BucketUnlimPArrayBucketItemTraits
	{
	protected:
		typedef TBucketUnlimPItemTraits BucketUnlimPItemTraits;

	public:
		typedef typename BucketUnlimPItemTraits::Item Item;
		typedef typename BucketUnlimPItemTraits::MemManager MemManager;

		static const size_t alignment = BucketUnlimPItemTraits::alignment;

	public:
		static void Destroy(MemManager& memManager, Item* items, size_t count) MOMO_NOEXCEPT
		{
			if (count != 1)
				BucketUnlimPItemTraits::Destroy(memManager, items, count);
		}

		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			BucketUnlimPItemTraits::RelocateCreate(memManager, srcItems, dstItems, count,
				std::forward<ItemCreator>(itemCreator), newItem);
		}
	};

	template<typename TItemTraits, size_t tMaxFastCount, typename TMemPoolParams,
		typename TArraySettings>
	class BucketUnlimP : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;
		typedef TArraySettings ArraySettings;

		static const size_t maxFastCount = tMaxFastCount;

	private:
		typedef BucketUnlimPArrayBucketItemTraits<ItemTraits> ArrayBucketItemTraits;

		typedef internal::ArrayBucket<ArrayBucketItemTraits, maxFastCount, MemPoolParams,
			ArraySettings> ArrayBucket;

	public:
		static const size_t maxCount = SIZE_MAX;

		static const bool isNothrowAddableIfNothrowCreatable = false;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		//typedef Item* Iterator;
		typedef typename ArrayBucket::Bounds Bounds;
		typedef typename Bounds::Iterator Iterator;

		typedef typename ArrayBucket::Params Params;

	public:
		explicit BucketUnlimP() MOMO_NOEXCEPT
		{
		}

		BucketUnlimP(const BucketUnlimP&) = delete;

		~BucketUnlimP() MOMO_NOEXCEPT
		{
		}

		BucketUnlimP& operator=(const BucketUnlimP&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return mArrayBucket.GetBounds();
		}

		template<typename Predicate>
		Iterator Find(Params& params, const Predicate& pred, size_t /*hashCode*/)
		{
			for (Item& item : GetBounds(params))
			{
				if (pred(item))
					return std::addressof(item);
			}
			return nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return false;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return false;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Bounds bounds = GetBounds(params);
			if (bounds.GetCount() == 1)
				ItemTraits::Destroy(params.GetMemManager(), bounds.GetBegin(), 1);
			mArrayBucket.Clear(params);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& params, ItemCreator&& itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			mArrayBucket.AddBackCrt(params, std::forward<ItemCreator>(itemCreator));
			return GetBounds(params).GetEnd() - 1;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, ItemReplacer&& itemReplacer)
		{
			Bounds bounds = GetBounds(params);
			size_t count = bounds.GetCount();
			size_t index = iter - bounds.GetBegin();
			MOMO_ASSERT(index < count);
			std::forward<ItemReplacer>(itemReplacer)(bounds[count - 1], bounds[index]);
			mArrayBucket.RemoveBack(params);
			return GetBounds(params).GetBegin() + index;
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t /*bucketCount*/, size_t /*probe*/) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(false);
			return bucketIndex;
		}

	private:
		ArrayBucket mArrayBucket;
	};
}

template<size_t tMaxFastCount = 7,
	typename TMemPoolParams = MemPoolParams<>,
	typename TArraySettings = ArraySettings<>>
class HashBucketUnlimP : public internal::HashBucketBase
{
public:
	static const size_t maxFastCount = tMaxFastCount;

	typedef TMemPoolParams MemPoolParams;
	typedef TArraySettings ArraySettings;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketUnlimP<ItemTraits, maxFastCount, MemPoolParams, ArraySettings>;
};

} // namespace momo
