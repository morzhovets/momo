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
	class BucketOpenN : public BucketBase<tMaxCount>
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount <= 8);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef std::reverse_iterator<Item*> Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

		static const bool isNothrowAddableIfNothrowCreatable = true;

	private:
		static const uint8_t emptyShortHash = (maxCount <= 4) ? 248 : 240;
		static const uint8_t maskCount = (maxCount <= 4) ? 3 : 7;

	public:
		explicit BucketOpenN() MOMO_NOEXCEPT
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
			uint8_t shortHash = pvGetShortHash(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mShortHashes[i] == shortHash && pred(*&mItems[i]))
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
			if (mState < emptyShortHash)
				return true;
			return (mState & (maskCount + 1)) != (uint8_t)0;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			ItemTraits::Destroy(params.GetMemManager(), &mItems[0] + maxCount - count, count);
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
			MOMO_NOEXCEPT_IF(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = &mItems[maxCount - 1 - count];
			std::forward<ItemCreator>(itemCreator)(pitem);
			mShortHashes[maxCount - 1 - count] = pvGetShortHash(hashCode);
			if (count + 1 < maxCount)
				++mState;
			return Iterator(pitem + 1);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = iter - Iterator(&mItems[0] + maxCount);
			MOMO_ASSERT(index < count);
			std::forward<ItemReplacer>(itemReplacer)(*&mItems[maxCount - count],
				*&mItems[maxCount - 1 - index]);
			mShortHashes[maxCount - 1 - index] = mShortHashes[maxCount - count];
			mShortHashes[maxCount - count] = emptyShortHash;
			if (count < maxCount)
				--mState;
			else
				mState = emptyShortHash + maskCount + (uint8_t)maxCount;
			return iter;
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			if (mState < emptyShortHash)
				return maxCount;
			return (size_t)(mState & maskCount);
		}

		static uint8_t pvGetShortHash(size_t hashCode) MOMO_NOEXCEPT
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyShortHash) >> 24);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			std::fill_n(mShortHashes, maxCount, (uint8_t)emptyShortHash);
		}

	private:
		union
		{
			uint8_t mState;
			uint8_t mShortHashes[maxCount];
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
