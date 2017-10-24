/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOpenN.h

  namespace momo:
    struct HashBucketOpenN

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount>
	class BucketOpenN
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount <= 8);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef std::reverse_iterator<Item*> Iterator;
		typedef BucketBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const uint8_t emptyHash = (maxCount <= 4) ? 248 : 240;
		static const uint8_t maskCount = (maxCount <= 4) ? 3 : 7;

	public:
		BucketOpenN() MOMO_NOEXCEPT
		{
			pvSetEmpty();
		}

		BucketOpenN(const BucketOpenN&) = delete;

		~BucketOpenN() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetCount() == 0);
		}

		BucketOpenN& operator=(const BucketOpenN&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(Iterator(&mItems[0] + maxCount), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			uint8_t hashByte = pvGetHashByte(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashes[i] == hashByte && pred(*&mItems[i]))
					return Iterator(&mItems[i] + 1);
			}
			return Iterator(nullptr);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return pvGetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mState < emptyHash)
				return true;
			return (mState & (maskCount + 1)) != (uint8_t)0;
		}

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			return ((size_t)1 << logBucketCount) - 1;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			ItemTraits::Destroy(params.GetMemManager(), &mItems[0] + maxCount - count, count);
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = &mItems[maxCount - 1 - count];
			itemCreator(pitem);
			mHashes[maxCount - 1 - count] = pvGetHashByte(hashCode);
			if (count + 1 < maxCount)
				++mState;
			return Iterator(pitem + 1);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = iter - Iterator(&mItems[0] + maxCount);
			MOMO_ASSERT(index < count);
			itemReplacer(*&mItems[maxCount - count], *&mItems[maxCount - 1 - index]);
			mHashes[maxCount - 1 - index] = mHashes[maxCount - count];
			mHashes[maxCount - count] = emptyHash;
			if (count < maxCount)
				--mState;
			else
				mState = emptyHash + maskCount + (uint8_t)maxCount;
			return iter;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator /*iter*/,
			size_t /*bucketIndex*/, size_t /*logBucketCount*/, size_t /*newLogBucketCount*/)
		{
			return hashCodeFullGetter();
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			if (mState < emptyHash)
				return maxCount;
			return (size_t)(mState & maskCount);
		}

		static uint8_t pvGetHashByte(size_t hashCode) MOMO_NOEXCEPT
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyHash) >> 24);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			std::fill_n(mHashes, maxCount, (uint8_t)emptyHash);
		}

	private:
		union
		{
			uint8_t mState;
			uint8_t mHashes[maxCount];
		};
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 4>
struct HashBucketOpenN : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 8) * 5;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOpenN<ItemTraits, maxCount>;
};

} // namespace momo
