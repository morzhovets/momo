/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

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

		static const bool isTriviallyRelocatable = false;

	public:
		static void Destroy(MemManager& /*memManager*/, Item* /*items*/, size_t /*count*/) noexcept
		{
		}

		template<conceptObjectCreator<Item> ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
		{
			BucketUnlimPItemTraits::RelocateCreate(memManager, srcItems, dstItems, count,
				std::move(itemCreator), newItem);
		}
	};

	template<typename TItemTraits, size_t tMaxFastCount,
		conceptMemPoolParamsBlockSizeAlignment TMemPoolParams, typename TArraySettings>
	requires conceptArrayBucketMaxFastCount<tMaxFastCount>
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
		explicit BucketUnlimP() noexcept = default;

		BucketUnlimP(const BucketUnlimP&) = delete;

		~BucketUnlimP() noexcept = default;

		BucketUnlimP& operator=(const BucketUnlimP&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return mArrayBucket.GetBounds();
		}

		template<bool first, conceptObjectPredicate<Item> Predicate>
		MOMO_FORCEINLINE Iterator Find(Params& params,
			FastCopyableFunctor<Predicate> pred, size_t /*hashCode*/)
		{
			for (Item& item : GetBounds(params))
			{
				if (pred(std::as_const(item)))
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

		template<conceptObjectCreator<Item> ItemCreator>
		Iterator AddCrt(Params& params, FastMovableFunctor<ItemCreator> itemCreator,
			size_t /*hashCode*/, size_t /*logBucketCount*/, size_t /*probe*/)
		{
			mArrayBucket.AddBackCrt(params, std::move(itemCreator));
			return GetBounds(params).GetEnd() - 1;
		}

		template<conceptObjectReplacer<Item> ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, FastMovableFunctor<ItemReplacer> itemReplacer)
		{
			Bounds bounds = GetBounds(params);
			size_t count = bounds.GetCount();
			size_t index = UIntMath<>::Dist(bounds.GetBegin(), iter);
			MOMO_ASSERT(index < count);
			std::move(itemReplacer)(bounds[count - 1], bounds[index]);
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
	internal::conceptMemPoolParamsBlockSizeAlignment TMemPoolParams = MemPoolParams<>,
	typename TArraySettings = ArraySettings<>>
requires internal::conceptArrayBucketMaxFastCount<tMaxFastCount>
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
