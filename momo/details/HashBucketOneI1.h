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

		bool TestIndex(size_t /*index*/, size_t /*hashCode*/) const MOMO_NOEXCEPT
		{
			return true;
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
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t /*hashCode*/)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			mState = HashBucketOneState::full;
			return &mItemBuffer;
		}

		void AcceptRemove(Params& /*params*/, size_t /*index*/) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(IsFull());
			mState = HashBucketOneState::removed;
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
