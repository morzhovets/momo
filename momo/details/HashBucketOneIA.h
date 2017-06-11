/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOneIA.h

  namespace momo:
    struct HashBucketOneIA

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<size_t size>
	struct BucketOneIAHashStateSelector
	{
		typedef uint8_t HashState;
	};

	template<>
	struct BucketOneIAHashStateSelector<2>
	{
		typedef uint16_t HashState;
	};

	template<>
	struct BucketOneIAHashStateSelector<4>
	{
		typedef uint32_t HashState;
	};

	template<>
	struct BucketOneIAHashStateSelector<8>
	{
		typedef uint64_t HashState;
	};

	template<typename TItemTraits>
	class BucketOneIA
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const size_t hashStateSize = (ItemTraits::alignment < sizeof(size_t))
			? ItemTraits::alignment : sizeof(size_t);
		static const size_t hashCodeShift = (sizeof(size_t) - hashStateSize) * 8;

		typedef typename BucketOneIAHashStateSelector<hashStateSize>::HashState HashState;

	public:
		BucketOneIA() MOMO_NOEXCEPT
			: mHashState(0)
		{
		}

		BucketOneIA(const BucketOneIA&) = delete;

		~BucketOneIA() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsFull());
		}

		BucketOneIA& operator=(const BucketOneIA&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return IsFull() ? Bounds(&mItemBuffer, 1) : Bounds();
		}

		template<typename Predicate>
		const Item* Find(Params& /*params*/, const Predicate& pred, size_t hashCode) const
		{
			if (mHashState != (HashState)((hashCode >> hashCodeShift) | 1))
				return nullptr;
			return pred(*&mItemBuffer) ? &mItemBuffer : nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return (mHashState & 1) == (HashState)1;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return mHashState != (HashState)0;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			mHashState = (HashState)0;
		}

		template<typename ItemCreator>
		Item* AddCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			mHashState = (HashState)((hashCode >> hashCodeShift) | 1);
			return &mItemBuffer;
		}

		void AcceptRemove(Params& /*params*/, size_t index) MOMO_NOEXCEPT
		{
			(void)index;
			MOMO_ASSERT(index == 0);
			MOMO_ASSERT(IsFull());
			mHashState = (HashState)2;
		}

	private:
		HashState mHashState;
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

struct HashBucketOneIA : public internal::HashBucketBase<1>
{
	template<typename ItemTraits>
	using Bucket = internal::BucketOneIA<ItemTraits>;
};

} // namespace momo
