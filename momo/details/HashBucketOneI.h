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
	class BucketOneI : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TStater Stater;

	public:
		static const size_t maxCount = 1;

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	public:
		explicit BucketOneI() MOMO_NOEXCEPT
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

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			pvSetState(HashBucketOneState::empty);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
			MOMO_NOEXCEPT_IF(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			MOMO_ASSERT(!IsFull());
			std::forward<ItemCreator>(itemCreator)(&mItemBuffer);
			pvSetState(HashBucketOneState::full);
			return &mItemBuffer;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			MOMO_ASSERT(IsFull());
			std::forward<ItemReplacer>(itemReplacer)(*&mItemBuffer, *&mItemBuffer);
			pvSetState(HashBucketOneState::removed);
			return nullptr;
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
