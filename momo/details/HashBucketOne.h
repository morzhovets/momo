/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketOne.h

  namespace momo:
    class HashBucketOne

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tStateSize>
	class BucketOne : public BucketBase
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
		typedef typename UIntSelector<
			(stateSize < ItemTraits::alignment) ? ItemTraits::alignment : stateSize,
			uint8_t>::UInt HashState;

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
			return IsFull() ? Bounds(&mItemBuffer, 1) : Bounds();
		}

		template<bool first, typename Predicate>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			if (mHashState == pvGetHashState(hashCode))
			{
				if (pred(*&mItemBuffer)) [[likely]]
					return &mItemBuffer;
			}
			return nullptr;
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
		requires std::invocable<ItemCreator&&, Item*>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
			noexcept(std::is_nothrow_invocable_v<ItemCreator&&, Item*>)
		{
			MOMO_ASSERT(!IsFull());
			std::forward<ItemCreator>(itemCreator)(&mItemBuffer);
			mHashState = pvGetHashState(hashCode);
			return &mItemBuffer;
		}

		template<typename ItemReplacer>
		requires std::invocable<ItemReplacer&&, Item&, Item&>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			(void)iter;
			MOMO_ASSERT(iter == &mItemBuffer);
			MOMO_ASSERT(IsFull());
			std::forward<ItemReplacer>(itemReplacer)(*&mItemBuffer, *&mItemBuffer);
			mHashState = HashState{2};
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
			return static_cast<size_t>(mHashState >> 1);
		}

	private:
		static HashState pvGetHashState(size_t hashCode) noexcept
		{
			if constexpr (sizeof(HashState) < sizeof(size_t))
			{
				static const size_t hashCodeShift = (sizeof(size_t) - sizeof(HashState)) * 8;
				return static_cast<HashState>(hashCode >> hashCodeShift) | 1;
			}
			else
			{
				return (static_cast<HashState>(hashCode) << 1) | 1;
			}
		}

	private:
		HashState mHashState;
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};
}

template<size_t tStateSize = 1>
class HashBucketOne : public internal::HashBucketBase
{
public:
	static const size_t stateSize = tStateSize;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOne<ItemTraits, stateSize>;
};

} // namespace momo
