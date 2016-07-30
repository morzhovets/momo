/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/BucketUnlimP.h

  namespace momo:
    struct HashBucketUnlimP

\**********************************************************/

#pragma once

#include "ArrayBucket.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tMaxFastCount, typename TMemPoolParams, typename TArraySettings>
	class BucketUnlimP
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef TMemPoolParams MemPoolParams;
		typedef TArraySettings ArraySettings;
		typedef typename ItemTraits::Item Item;

		static const size_t maxFastCount = tMaxFastCount;

	private:
		struct ArrayBucketItemTraits
		{
			typedef typename ItemTraits::Item Item;

			static const size_t alignment = ItemTraits::alignment;

			static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
			{
				ItemTraits::Destroy(items, count);
			}

			static void Relocate(Item* /*srcItems*/, Item* /*dstItems*/, size_t /*count*/)
			{
				MOMO_ASSERT(false);
			}

			template<typename ItemCreator>
			static void RelocateCreate(Item* srcItems, Item* dstItems, size_t count,
				const ItemCreator& itemCreator, void* pitem)
			{
				ItemTraits::RelocateCreate(srcItems, dstItems, count, itemCreator, pitem);
			}
		};

		typedef momo::internal::ArrayBucket<ArrayBucketItemTraits, MemManager,
			maxFastCount, MemPoolParams, ArraySettings> ArrayBucket;

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
			mArrayBucket.Clear(params);
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			mArrayBucket.AddBackCrt(params, itemCreator);
			return mArrayBucket.GetBounds().GetEnd() - 1;
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
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

	template<typename ItemTraits, typename MemManager>
	using Bucket = internal::BucketUnlimP<ItemTraits, MemManager,
		maxFastCount, MemPoolParams, ArraySettings>;
};

} // namespace momo
