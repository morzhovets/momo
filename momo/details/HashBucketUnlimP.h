/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketUnlimP.h

  namespace momo:
    struct HashBucketUnlimP

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
			size_t count, const ItemCreator& itemCreator, Item* newItem)
		{
			BucketUnlimPItemTraits::RelocateCreate(memManager, srcItems, dstItems, count,
				itemCreator, newItem);
		}
	};

	template<typename TItemTraits, size_t tMaxFastCount, typename TMemPoolParams,
		typename TArraySettings>
	class BucketUnlimP
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;
		typedef TArraySettings ArraySettings;
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		static const size_t maxFastCount = tMaxFastCount;

	private:
		typedef BucketUnlimPArrayBucketItemTraits<ItemTraits> ArrayBucketItemTraits;

		typedef internal::ArrayBucket<ArrayBucketItemTraits, maxFastCount, MemPoolParams,
			ArraySettings> ArrayBucket;

	public:
		typedef typename ArrayBucket::Params Params;

		typedef typename ArrayBucket::ConstBounds ConstBounds;
		typedef typename ArrayBucket::Bounds Bounds;

	public:
		BucketUnlimP() MOMO_NOEXCEPT
		{
		}

		BucketUnlimP(const BucketUnlimP&) = delete;

		~BucketUnlimP() MOMO_NOEXCEPT
		{
		}

		BucketUnlimP& operator=(const BucketUnlimP&) = delete;

		ConstBounds GetBounds(const Params& /*params*/) const MOMO_NOEXCEPT
		{
			return mArrayBucket.GetBounds();
		}

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return mArrayBucket.GetBounds();
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
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			mArrayBucket.AddBackCrt(params, itemCreator);
			return GetBounds(params).GetEnd() - 1;
		}

		void DecCount(Params& params) MOMO_NOEXCEPT
		{
			mArrayBucket.RemoveBack(params);
		}

	private:
		ArrayBucket mArrayBucket;
	};
}

template<size_t tMaxFastCount = 7,
	typename TMemPoolParams = MemPoolParams<>,
	typename TArraySettings = ArraySettings<>>
struct HashBucketUnlimP : public internal::HashBucketBase<SIZE_MAX>
{
	static const size_t maxFastCount = tMaxFastCount;

	typedef TMemPoolParams MemPoolParams;
	typedef TArraySettings ArraySettings;

	static size_t GetBucketIndex(size_t hashCode, size_t bucketCount, size_t probe) MOMO_NOEXCEPT
	{
		(void)probe;
		MOMO_ASSERT(probe == 0);
		return hashCode & (bucketCount - 1);
	}

	template<typename ItemTraits>
	using Bucket = internal::BucketUnlimP<ItemTraits, maxFastCount, MemPoolParams, ArraySettings>;
};

} // namespace momo
