/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOneI.h

  namespace momo:
    struct HashBucketOneI

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TStater>
	class BucketOneI
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TStater Stater;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

	public:
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
		BucketOneI() MOMO_NOEXCEPT
		{
			pvSetState(HashBucketOneState::empty);
		}

		BucketOneI(const BucketOneI&) = delete;

		~BucketOneI() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsFull());
		}

		BucketOneI& operator=(const BucketOneI&) = delete;

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
			return pvGetState() == HashBucketOneState::full;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return pvGetState() != HashBucketOneState::empty;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			pvSetState(HashBucketOneState::empty);
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t /*hashCode*/)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			pvSetState(HashBucketOneState::full);
			return &mItemBuffer;
		}

		void AcceptRemove(Params& /*params*/, size_t /*index*/) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(IsFull());
			pvSetState(HashBucketOneState::removed);
		}

	private:
		HashBucketOneState pvGetState() const MOMO_NOEXCEPT
		{
			return Stater::GetState(&mItemBuffer);
		}

		void pvSetState(HashBucketOneState state) MOMO_NOEXCEPT
		{
			Stater::SetState(&mItemBuffer, state);
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

template<typename TStater>
struct HashBucketOneI : public internal::HashBucketBase<1>
{
	typedef TStater Stater;

	template<typename ItemTraits>
	using Bucket = internal::BucketOneI<ItemTraits, Stater>;
};

} // namespace momo
