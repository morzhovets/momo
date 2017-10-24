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

		typedef Item* Iterator;
		typedef BucketBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

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

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t /*hashCode*/)
		{
			return (IsFull() && pred(*&mItemBuffer)) ? &mItemBuffer : nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return pvGetState() == HashBucketOneState::full;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return pvGetState() != HashBucketOneState::empty;
		}

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			return ((size_t)1 << logBucketCount) - 1;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			pvSetState(HashBucketOneState::empty);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			pvSetState(HashBucketOneState::full);
			return &mItemBuffer;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			MOMO_ASSERT(IsFull());
			itemReplacer(*&mItemBuffer, *&mItemBuffer);
			pvSetState(HashBucketOneState::removed);
			return nullptr;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator /*iter*/,
			size_t /*bucketIndex*/, size_t /*logBucketCount*/, size_t /*newLogBucketCount*/)
		{
			return hashCodeFullGetter();
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

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOneI<ItemTraits, Stater>;
};

} // namespace momo
