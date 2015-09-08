/**********************************************************\

  momo/HashBuckets/BucketOneI1.h

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
	template<typename TItemTraits, typename TMemManager>
	class BucketOneI1
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
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
			Params(MemManager& /*memManager*/) MOMO_NOEXCEPT
			{
			}

		private:
			MOMO_DISABLE_COPY_CONSTRUCTOR(Params);
			MOMO_DISABLE_COPY_OPERATOR(Params);
		};

	public:
		BucketOneI1() MOMO_NOEXCEPT
			: mState(stateEmpty)
		{
		}

		~BucketOneI1() MOMO_NOEXCEPT
		{
			assert(mState == stateEmpty);
		}

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
		void AddBackEmpl(Params& /*params*/, const ItemCreator& itemCreator)
		{
			assert(!IsFull());
			itemCreator(&mItemBuffer);
			mState = stateFull;
		}

		void RemoveBack(Params& /*params*/) MOMO_NOEXCEPT
		{
			assert(IsFull());
			ItemTraits::Destroy(&mItemBuffer, 1);
			mState = stateRemoved;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketOneI1);
		MOMO_DISABLE_COPY_OPERATOR(BucketOneI1);

	private:
		internal::ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		unsigned char mState;
	};
}

struct HashBucketOneI1 : public internal::HashBucketBase<1>
{
	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketOneI1<ItemTraits, MemManager> Bucket;
	};
};

} // namespace momo
