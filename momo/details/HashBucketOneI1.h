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
	public:
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		static const unsigned char stateEmpty = 0;
		static const unsigned char stateFull = 1;
		static const unsigned char stateRemoved = 2;

	public:
		class Params
		{
		public:
			explicit Params(MemManager& /*memManager*/) MOMO_NOEXCEPT
			{
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;
		};

	public:
		BucketOneI1() MOMO_NOEXCEPT
			: mState(stateEmpty)
		{
		}

		BucketOneI1(const BucketOneI1&) = delete;

		~BucketOneI1() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsFull());
		}

		BucketOneI1& operator=(const BucketOneI1&) = delete;

		ConstBounds GetBounds(const Params& /*params*/) const MOMO_NOEXCEPT
		{
			return IsFull() ? ConstBounds(&mItemBuffer, 1) : ConstBounds();
		}

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return IsFull() ? Bounds(&mItemBuffer, 1) : Bounds();
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mState == stateFull;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return mState != stateEmpty;
		}

		void Clear(Params& /*params*/) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(&mItemBuffer, 1);
			mState = stateEmpty;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			mState = stateFull;
			return &mItemBuffer;
		}

		void DecCount(Params& /*params*/) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(IsFull());
			mState = stateRemoved;
		}

	private:
		internal::ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		unsigned char mState;
	};
}

struct HashBucketOneI1 : public internal::HashBucketBase<1>
{
	template<typename ItemTraits>
	using Bucket = internal::BucketOneI1<ItemTraits>;
};

} // namespace momo
