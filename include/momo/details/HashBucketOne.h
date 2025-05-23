/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/details/HashBucketOne.h

  namespace momo:
    class HashBucketOne

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_DETAILS_HASH_BUCKET_ONE
#define MOMO_INCLUDE_GUARD_DETAILS_HASH_BUCKET_ONE

#include "BucketUtility.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMinStateSize>
	class BucketOne : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t minStateSize = tMinStateSize;
		MOMO_STATIC_ASSERT(0 < minStateSize && minStateSize <= sizeof(size_t));

	public:
		static const size_t maxCount = 1;

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const size_t availableStateSize = (ItemTraits::alignment <= sizeof(size_t))
			? ItemTraits::alignment : sizeof(size_t);
		typedef typename UIntSelector<(minStateSize < availableStateSize)
			? availableStateSize : minStateSize>::UInt HashState;

	public:
		explicit BucketOne() noexcept
			: mHashState(0)
		{
		}

		BucketOne(const BucketOne&) = delete;

		~BucketOne() noexcept
		{
			MOMO_ASSERT(!IsFull());
		}

		BucketOne& operator=(const BucketOne&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return IsFull() ? Bounds(pvGetItemPtr(), 1) : Bounds();
		}

		template<bool first, typename ItemPredicate>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/,
			const ItemPredicate& itemPred, size_t hashCode)
		{
			if (mHashState != pvGetHashState(hashCode))
				return nullptr;
			Item* itemPtr = pvGetItemPtr();
			return itemPred(*itemPtr) ? itemPtr : nullptr;
		}

		bool IsFull() const noexcept
		{
			return (mHashState & 1) == HashState{1};
		}

		bool WasFull() const noexcept
		{
			return mHashState != HashState{0};
		}

		void Clear(Params& /*params*/) noexcept
		{
			mHashState = HashState{0};
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
			noexcept(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			MOMO_ASSERT(!IsFull());
			std::forward<ItemCreator>(itemCreator)(pvGetItemPtr<false>());
			mHashState = pvGetHashState(hashCode);
			return pvGetItemPtr();
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(IsFull());
			Item* itemPtr = pvGetItemPtr();
			MOMO_ASSERT(iter == itemPtr);
			std::forward<ItemReplacer>(itemReplacer)(*itemPtr, *itemPtr);
			mHashState = HashState{2};
			return nullptr;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator iter,
			size_t /*bucketIndex*/, size_t /*logBucketCount*/, size_t /*newLogBucketCount*/)
		{
			(void)iter;
			MOMO_ASSERT(iter == pvGetItemPtr());
			if (sizeof(HashState) < sizeof(size_t))
				return hashCodeFullGetter();
			return static_cast<size_t>(mHashState >> 1);
		}

	private:
		template<size_t stateSize = sizeof(HashState)>
		static EnableIf<(stateSize < sizeof(size_t)),
		HashState> pvGetHashState(size_t hashCode) noexcept
		{
			static const size_t hashCodeShift = (sizeof(size_t) - stateSize) * 8;
			return static_cast<HashState>(hashCode >> hashCodeShift) | 1;
		}

		template<size_t stateSize = sizeof(HashState)>
		static EnableIf<(stateSize >= sizeof(size_t)),
		HashState> pvGetHashState(size_t hashCode) noexcept
		{
			return (static_cast<HashState>(hashCode) << 1) | 1;
		}

		template<bool isWithinLifetime = true>
		Item* pvGetItemPtr() noexcept
		{
			return mItemBuffer.template GetPtr<isWithinLifetime>();
		}

	private:
		HashState mHashState;
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

template<size_t tMinStateSize = 1>
class HashBucketOne : public internal::HashBucketBase
{
public:
	static const size_t minStateSize = tMinStateSize;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOne<ItemTraits, minStateSize>;
};

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_DETAILS_HASH_BUCKET_ONE
