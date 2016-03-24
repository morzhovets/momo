/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/BucketOneI.h

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
	template<typename TItemTraits, typename TMemManager, typename TStater>
	class BucketOneI
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef TStater Stater;
		typedef typename ItemTraits::Item Item;

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
		BucketOneI() MOMO_NOEXCEPT
		{
			_SetState(stateEmpty);
		}

		BucketOneI(const BucketOneI&) = delete;

		~BucketOneI() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsFull());
		}

		BucketOneI& operator=(const BucketOneI&) = delete;

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
			return _GetState() == stateFull;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return _GetState() != stateEmpty;
		}

		void Clear(Params& /*params*/) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(&mItemBuffer, 1);
			_SetState(stateEmpty);
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			_SetState(stateFull);
			return &mItemBuffer;
		}

		void RemoveBack(Params& /*params*/) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(IsFull());
			ItemTraits::Destroy(&mItemBuffer, 1);
			_SetState(stateRemoved);
		}

	private:
		unsigned char _GetState() const MOMO_NOEXCEPT
		{
			return Stater::GetState(&mItemBuffer);
		}

		void _SetState(unsigned char state) MOMO_NOEXCEPT
		{
			Stater::SetState(&mItemBuffer, state);
		}

	private:
		internal::ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

template<typename TStater>
struct HashBucketOneI : public internal::HashBucketBase<1>
{
	typedef TStater Stater;

	template<typename ItemTraits, typename MemManager>
	using Bucket = internal::BucketOneI<ItemTraits, MemManager, Stater>;
};

} // namespace momo
