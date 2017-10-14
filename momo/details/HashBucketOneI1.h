/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOneI1.h

  namespace momo:
    struct HashBucketOneI1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits>
	class BucketOneI1
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef BucketBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	public:
		BucketOneI1() MOMO_NOEXCEPT
			: mState(HashBucketOneState::empty)
		{
		}

		BucketOneI1(const BucketOneI1&) = delete;

		~BucketOneI1() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsFull());
		}

		BucketOneI1& operator=(const BucketOneI1&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return IsFull() ? Bounds(&mItemBuffer, 1) : Bounds();
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t /*hashCode*/,
			size_t /*logBucketCount*/)
		{
			return (IsFull() && pred(*&mItemBuffer)) ? &mItemBuffer : nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mState == HashBucketOneState::full;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return mState != HashBucketOneState::empty;
		}

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			return ((size_t)1 << logBucketCount) - 1;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			mState = HashBucketOneState::empty;
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			mState = HashBucketOneState::full;
			return &mItemBuffer;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			MOMO_ASSERT(IsFull());
			itemReplacer(*&mItemBuffer, *&mItemBuffer);
			mState = HashBucketOneState::removed;
			return nullptr;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator /*iter*/,
			size_t /*bucketIndex*/, size_t /*logBucketCount*/, size_t /*newLogBucketCount*/)
		{
			return hashCodeFullGetter();
		}

	private:
		HashBucketOneState mState;
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

struct HashBucketOneI1 : public internal::HashBucketBase<1>
{
	template<typename ItemTraits>
	using Bucket = internal::BucketOneI1<ItemTraits>;
};

} // namespace momo
