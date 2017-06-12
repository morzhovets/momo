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
	class BucketOpenN1
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 128);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef BucketBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const uint8_t emptyHash = 255;

	public:
		BucketOpenN1() MOMO_NOEXCEPT
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
			return Bounds(&mItems[0], pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			uint8_t hashByte = pvGetHashByte(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashes[i] == hashByte && pred(*&mItems[i]))
					return &mItems[i];
			}
			return nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return pvGetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return (mState & 128) != (uint8_t)0;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(params.GetMemManager(), &mItems[0], pvGetCount());
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = &mItems[count];
			itemCreator(pitem);
			mHashes[count] = pvGetHashByte(hashCode);
			++mState;
			mState |= (uint8_t)(IsFull() ? 128 : 0);
			return pitem;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = iter - &mItems[0];
			MOMO_ASSERT(index < count);
			itemReplacer(*&mItems[count - 1], *&mItems[index]);
			mHashes[index] = mHashes[count - 1];
			mHashes[count - 1] = emptyHash;
			--mState;
			return &mItems[index];
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(mState & 127);
		}

		static uint8_t pvGetHashByte(size_t hashCode) MOMO_NOEXCEPT
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyHash) >> 24);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			mState = (uint8_t)0;
			std::fill_n(mHashes, maxCount, (uint8_t)emptyHash);
		}

	private:
		uint8_t mState;
		uint8_t mHashes[maxCount];
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 3>
struct HashBucketOpenN1 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 4) * 3;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	template<typename ItemTraits>
	using Bucket = internal::BucketOpenN1<ItemTraits, maxCount>;
};

} // namespace momo
