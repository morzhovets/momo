/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketMulti.h

  namespace momo:
    struct HashBucketMulti

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount>
	class BucketMulti
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t maxCount = tMaxCount;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

		class Params
		{
		public:
			explicit Params(MemManager& memManager) MOMO_NOEXCEPT
				: mMemManager(memManager)
			{
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() MOMO_NOEXCEPT
			{
				return mMemManager;
			}

		private:
			MemManager& mMemManager;
		};

	public:
		BucketMulti() MOMO_NOEXCEPT
		{
			mCount = (unsigned char)0;
			mWasFull = false;
		}

		BucketMulti(const BucketMulti&) = delete;

		~BucketMulti() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mCount == (unsigned char)0);
		}

		BucketMulti& operator=(const BucketMulti&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(&mItems[0], (size_t)mCount);
		}

		bool TestIndex(size_t /*index*/, size_t /*hashCode*/) const MOMO_NOEXCEPT
		{
			return true;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return (size_t)mCount == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return mWasFull;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(params.GetMemManager(), &mItems[0], (size_t)mCount);
			mCount = (unsigned char)0;
			mWasFull = false;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t /*hashCode*/)
		{
			MOMO_ASSERT(!IsFull());
			Item* pitem = &mItems[mCount];
			itemCreator(pitem);
			++mCount;
			mWasFull |= IsFull();
			return pitem;
		}

		void AcceptRemove(Params& /*params*/, size_t /*index*/) MOMO_NOEXCEPT
		{
			--mCount;
		}

	private:
		unsigned char mCount;
		bool mWasFull;
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount>
struct HashBucketMulti : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	template<typename ItemTraits>
	using Bucket = internal::BucketMulti<ItemTraits, maxCount>;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 4) * 3;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}
};

} // namespace momo
