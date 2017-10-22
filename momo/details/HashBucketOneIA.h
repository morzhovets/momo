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

	template<typename TItemTraits, size_t tStateSize>
	class BucketOneIA
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t stateSize = tStateSize;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef BucketBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		typedef typename BucketOneIAHashStateSelector<stateSize>::HashState HashState;

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
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode,
			size_t logBucketCount)
		{
			if (mHashState != pvGetHashState(hashCode, logBucketCount))
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

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			return ((size_t)1 << logBucketCount) - 1;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			mHashState = (HashState)0;
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode,
			size_t logBucketCount, size_t /*probe*/)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			mHashState = pvGetHashState(hashCode, logBucketCount);
			return &mItemBuffer;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			MOMO_ASSERT(IsFull());
			itemReplacer(*&mItemBuffer, *&mItemBuffer);
			mHashState = (HashState)2;
			return nullptr;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator iter,
			size_t /*bucketIndex*/, size_t /*logBucketCount*/, size_t /*newLogBucketCount*/)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			if (sizeof(HashState) < sizeof(size_t))
				return hashCodeFullGetter();
			return (size_t)(mHashState >> 1);
		}

	private:
		static HashState pvGetHashState(size_t hashCode, size_t logBucketCount) MOMO_NOEXCEPT
		{
			size_t shift = (sizeof(HashState) < sizeof(size_t)) ? logBucketCount : 0;
			return ((HashState)(hashCode >> shift) << 1) | 1;
		}

	private:
		HashState mHashState;
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

template<size_t tStateSize = 0>	// 0 for stateSize = ItemTraits::alignment
struct HashBucketOneIA : public internal::HashBucketBase<1>
{
	static const size_t stateSize = tStateSize;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOneIA<ItemTraits,
		(stateSize == 0) ? ItemTraits::alignment : stateSize>;
};

} // namespace momo
