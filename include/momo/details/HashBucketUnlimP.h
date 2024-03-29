/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/details/HashBucketUnlimP.h

  namespace momo:
    class HashBucketUnlimP

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_DETAILS_HASH_BUCKET_UNLIMP
#define MOMO_INCLUDE_GUARD_DETAILS_HASH_BUCKET_UNLIMP

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

		static const bool isTriviallyRelocatable = false;

	public:
		static void Destroy(MemManager& /*memManager*/, Item* /*items*/, size_t /*count*/) noexcept
		{
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
		static const size_t maxCount = UIntConst::maxSize;

		static const bool isNothrowAddableIfNothrowCreatable = false;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		//typedef Item* Iterator;
		typedef typename ArrayBucket::Bounds Bounds;
		typedef typename Bounds::Iterator Iterator;

		typedef typename ArrayBucket::Params Params;

	public:
		explicit BucketUnlimP() noexcept
		{
		}

		BucketUnlimP(const BucketUnlimP&) = delete;

		~BucketUnlimP() = default;

		BucketUnlimP& operator=(const BucketUnlimP&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return mArrayBucket.GetBounds();
		}

		template<bool first, typename ItemPredicate>
		MOMO_FORCEINLINE Iterator Find(Params& params,
			const ItemPredicate& itemPred, size_t /*hashCode*/)
		{
			for (Item& item : GetBounds(params))
			{
				if (itemPred(item))
					return std::addressof(item);
			}
			return nullptr;
		}

		bool IsFull() const noexcept
		{
			return false;
		}

		bool WasFull() const noexcept
		{
			return false;
		}

		size_t GetMaxProbe(size_t /*logBucketCount*/) const noexcept
		{
			return 0;
		}

		void Clear(Params& params) noexcept
		{
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
			size_t index = UIntMath<>::Dist(bounds.GetBegin(), iter);
			MOMO_ASSERT(index < count);
			std::forward<ItemReplacer>(itemReplacer)(bounds[count - 1], bounds[index]);
			mArrayBucket.RemoveBack(params);
			return GetBounds(params).GetBegin() + index;
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t /*bucketCount*/, size_t /*probe*/) noexcept
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

#endif // MOMO_INCLUDE_GUARD_DETAILS_HASH_BUCKET_UNLIMP
