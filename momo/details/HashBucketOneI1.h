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
	class BucketOneI1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		static const size_t maxCount = 1;

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	public:
		explicit BucketOneI1() MOMO_NOEXCEPT
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
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t /*hashCode*/)
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

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			mState = HashBucketOneState::empty;
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
			MOMO_NOEXCEPT_IF(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			MOMO_ASSERT(!IsFull());
			std::forward<ItemCreator>(itemCreator)(&mItemBuffer);
			mState = HashBucketOneState::full;
			return &mItemBuffer;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			MOMO_ASSERT(IsFull());
			std::forward<ItemReplacer>(itemReplacer)(*&mItemBuffer, *&mItemBuffer);
			mState = HashBucketOneState::removed;
			return nullptr;
		}

	private:
		HashBucketOneState mState;
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

struct HashBucketOneI1 : public internal::HashBucketBase<1>
{
	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOneI1<ItemTraits>;
};

} // namespace momo
