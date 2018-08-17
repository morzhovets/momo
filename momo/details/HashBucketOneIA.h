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
	template<typename TItemTraits, size_t tStateSize>
	class BucketOneIA : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t stateSize = tStateSize;

	public:
		static const size_t maxCount = 1;

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		typedef typename UIntSelector<stateSize, uint8_t>::UInt HashState;

	public:
		explicit BucketOneIA() MOMO_NOEXCEPT
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
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			if (mHashState != pvGetHashState(hashCode))
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
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
			MOMO_NOEXCEPT_IF(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			MOMO_ASSERT(!IsFull());
			std::forward<ItemCreator>(itemCreator)(&mItemBuffer);
			mHashState = pvGetHashState(hashCode);
			return &mItemBuffer;
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			MOMO_ASSERT(IsFull());
			std::forward<ItemReplacer>(itemReplacer)(*&mItemBuffer, *&mItemBuffer);
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
		template<size_t hashStateSize = sizeof(HashState),
			typename std::enable_if<(hashStateSize < sizeof(size_t)), int>::type = 0>
		static HashState pvGetHashState(size_t hashCode) MOMO_NOEXCEPT
		{
			static const size_t hashCodeShift = (sizeof(size_t) - hashStateSize) * 8;
			return (HashState)(hashCode >> hashCodeShift) | 1;
		}

		template<size_t hashStateSize = sizeof(HashState),
			typename std::enable_if<(hashStateSize >= sizeof(size_t)), int>::type = 0>
		static HashState pvGetHashState(size_t hashCode) MOMO_NOEXCEPT
		{
			return ((HashState)hashCode << 1) | 1;
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
