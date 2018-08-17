/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOpenN1.h

  namespace momo:
    struct HashBucketOpenN1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount>
	class BucketOpenN1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 128);

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef std::reverse_iterator<Item*> Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const uint8_t emptyShortHash = 255;
		static const uint8_t maskCount = 127;

	public:
		explicit BucketOpenN1() MOMO_NOEXCEPT
		{
			pvSetEmpty();
		}

		BucketOpenN1(const BucketOpenN1&) = delete;

		~BucketOpenN1() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetCount() == 0);
		}

		BucketOpenN1& operator=(const BucketOpenN1&) = delete;

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
			++mState;
			mState |= (uint8_t)(IsFull() ? maskCount + 1 : 0);
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
			--mState;
			return iter;
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(mState & maskCount);
		}

		static uint8_t pvGetShortHash(size_t hashCode) MOMO_NOEXCEPT
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyShortHash) >> 24);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			mState = (uint8_t)0;
			std::fill_n(mShortHashes, maxCount, (uint8_t)emptyShortHash);
		}

	private:
		uint8_t mState;
		uint8_t mShortHashes[maxCount];
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 3>
struct HashBucketOpenN1 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 3) * 2;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOpenN1<ItemTraits, maxCount>;
};

} // namespace momo
