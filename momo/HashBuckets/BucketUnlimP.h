/**********************************************************\

  momo/HashBuckets/BucketUnlimP.h

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
		size_t tMaxFastCount, size_t tMemPoolBlockCount, typename TArraySettings>
	class BucketUnlimP
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef TArraySettings ArraySettings;
		typedef typename ItemTraits::Item Item;

		static const size_t maxFastCount = tMaxFastCount;
		static const size_t memPoolBlockCount = tMemPoolBlockCount;

	private:
		typedef momo::internal::ArrayBucket<ItemTraits, MemManager,
			maxFastCount, memPoolBlockCount, ArraySettings> ArrayBucket;

	public:
		typedef typename ArrayBucket::Params Params;

		typedef typename ArrayBucket::ConstBounds ConstBounds;
		typedef typename ArrayBucket::Bounds Bounds;

	public:
		BucketUnlimP() MOMO_NOEXCEPT
		{
		}

		~BucketUnlimP() MOMO_NOEXCEPT
		{
		}

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
		void AddBackEmpl(Params& params, const ItemCreator& itemCreator)
		{
			mArrayBucket.AddBackEmpl(params, itemCreator);
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			mArrayBucket.RemoveBack(params);
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketUnlimP);
		MOMO_DISABLE_COPY_OPERATOR(BucketUnlimP);

	private:
		ArrayBucket mArrayBucket;
	};
}

template<size_t tMaxFastCount = 7,
	size_t tMemPoolBlockCount = 32,
	typename TArraySettings = ArraySettings<>>
struct HashBucketUnlimP : public internal::HashBucketBase<SIZE_MAX>
{
	static const size_t maxFastCount = tMaxFastCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	typedef TArraySettings ArraySettings;

	static size_t GetBucketIndex(size_t hashCode, size_t bucketCount, size_t probe) MOMO_NOEXCEPT
	{
		assert(probe == 0);
		return hashCode & (bucketCount - 1);
	}

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketUnlimP<ItemTraits, MemManager,
			maxFastCount, memPoolBlockCount, ArraySettings> Bucket;
	};
};

} // namespace momo
