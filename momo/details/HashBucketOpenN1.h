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

		typedef BucketBounds<Item> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	public:
		BucketOpenN1() MOMO_NOEXCEPT
			: mState(0)
		{
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
		const Item* Find(Params& /*params*/, const Predicate& pred, size_t hashCode) const
		{
			uint8_t hashByte = pvGetHashByte(hashCode);
			size_t count = pvGetCount();
			for (size_t i = 0; i < count; ++i)
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
			mState = (uint8_t)0;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode)
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

		void AcceptRemove(Params& /*params*/, size_t index) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count > 0);
			mHashes[index] = mHashes[count - 1];
			--mState;
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(mState & 127);
		}

		static uint8_t pvGetHashByte(size_t hashCode) MOMO_NOEXCEPT
		{
			return (uint8_t)(hashCode >> (sizeof(size_t) * 8 - 8));
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
