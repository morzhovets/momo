/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketOpenN1.h

  namespace momo:
    class HashBucketOpenN1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, bool tReverse,
		size_t tDataAlignment = 1>
	class BucketOpenN1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const bool reverse = tReverse;
		static const size_t dataAlignment = tDataAlignment;

	public:
		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 8);

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef typename std::conditional<reverse,
			std::reverse_iterator<Item*>, Item*>::type Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const uint8_t emptyShortHash = 248;
		static const uint8_t infProbeExp = 255;

	public:
		explicit BucketOpenN1() noexcept
		{
			pvSetEmpty();
		}

		BucketOpenN1(const BucketOpenN1&) = delete;

		~BucketOpenN1() noexcept
		{
			MOMO_ASSERT(pvGetCount() == 0);
		}

		BucketOpenN1& operator=(const BucketOpenN1&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return Bounds(pvMakeIterator(ptGetItemPtr(0)), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			uint8_t shortHash = ptCalcShortHash(hashCode);
			const uint8_t* thisShortHashes = ptGetShortHashes();
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (thisShortHashes[i] == shortHash && pred(*&mItems[i]))
					return pvMakeIterator(&mItems[i]);
			}
			return Iterator();
		}

		bool IsFull() const noexcept
		{
			return pvGetState() < emptyShortHash;
		}

		bool WasFull() const noexcept
		{
			return true;
		}

		size_t GetMaxProbe(size_t logBucketCount) const noexcept
		{
			uint8_t maxProbeExp = pvGetMaxProbeExp();
			if (maxProbeExp == infProbeExp)
				return ((size_t)1 << logBucketCount) - 1;
			return pvGetMaxProbe(maxProbeExp);
		}

		void UpdateMaxProbe(size_t probe) noexcept
		{
			uint8_t& maxProbeExp = pvGetMaxProbeExp();
			if (maxProbeExp == infProbeExp || probe <= pvGetMaxProbe(maxProbeExp))
				return;
			size_t maxProbe0 = probe - 1;
			size_t maxProbe1 = 0;
			while (maxProbe0 >= (size_t)7)
			{
				maxProbe0 >>= 1;
				++maxProbe1;
			}
			maxProbeExp = (maxProbe1 <= (size_t)31)
				? (uint8_t)(maxProbe0 + 1) | (uint8_t)(maxProbe1 << 3) : infProbeExp;
		}

		void Clear(Params& params) noexcept
		{
			size_t count = pvGetCount();
			Item* begin = &mItems[0] + (reverse ? maxCount - count : 0);
			ItemTraits::Destroy(params.GetMemManager(), begin, count);
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
			noexcept(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = ptGetItemPtr(count);
			std::forward<ItemCreator>(itemCreator)(pitem);
			pvGetShortHash(count) = ptCalcShortHash(hashCode);
			if (count + 1 < maxCount)
				++pvGetState();
			return pvMakeIterator(pitem);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = iter - pvMakeIterator(ptGetItemPtr(0));
			MOMO_ASSERT(index < count);
			std::forward<ItemReplacer>(itemReplacer)(*ptGetItemPtr(count - 1),
				*ptGetItemPtr(index));
			pvGetShortHash(index) = pvGetShortHash(count - 1);
			pvGetShortHash(count - 1) = emptyShortHash;
			if (count < maxCount)
				--pvGetState();
			else
				pvGetState() = emptyShortHash + (uint8_t)maxCount - 1;
			return iter;
		}

	protected:
		uint8_t* ptGetShortHashes() noexcept
		{
			return pvGetBytePtr(0);
		}

		Item* ptGetItemPtr(size_t index) noexcept
		{
			return &mItems[reverse ? maxCount - 1 - index : index];
		}

		static uint8_t ptCalcShortHash(size_t hashCode) noexcept
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyShortHash) >> 24);
		}

	private:
		uint8_t pvGetState() const noexcept
		{
			return *pvGetBytePtr(reverse ? 0 : maxCount - 1);
		}

		uint8_t& pvGetState() noexcept
		{
			return *pvGetBytePtr(reverse ? 0 : maxCount - 1);
		}

		uint8_t pvGetMaxProbeExp() const noexcept
		{
			return *pvGetBytePtr(maxCount);
		}

		uint8_t& pvGetMaxProbeExp() noexcept
		{
			return *pvGetBytePtr(maxCount);
		}

		uint8_t& pvGetShortHash(size_t index) noexcept
		{
			return *pvGetBytePtr(reverse ? maxCount - 1 - index : index);
		}

		const uint8_t* pvGetBytePtr(size_t index) const noexcept
		{
			return BitCaster::PtrToPtr<const uint8_t>(mData, index);
		}

		uint8_t* pvGetBytePtr(size_t index) noexcept
		{
			return BitCaster::PtrToPtr<uint8_t>(mData, index);
		}

		static Iterator pvMakeIterator(Item* pitem) noexcept
		{
			return Iterator(pitem + (reverse ? 1 : 0));
		}

		size_t pvGetCount() const noexcept
		{
			uint8_t state = pvGetState();
			return (state >= emptyShortHash) ? (size_t)(state - emptyShortHash) : maxCount;
		}

		static size_t pvGetMaxProbe(uint8_t maxProbeExp) noexcept
		{
			return (size_t)(maxProbeExp & 7) << (maxProbeExp >> 3);
		}

		void pvSetEmpty() noexcept
		{
			std::fill(std::begin(mData), std::end(mData), (char)0);
			std::fill_n(ptGetShortHashes(), maxCount, (uint8_t)emptyShortHash);
		}

	private:
		alignas(dataAlignment) char mData[maxCount + 1];
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 3,
	bool tReverse = true>
class HashBucketOpenN1 : public internal::HashBucketBase
{
public:
	static const size_t maxCount = tMaxCount;
	static const bool reverse = tReverse;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOpenN1<ItemTraits, maxCount, reverse>;

public:
	static size_t CalcCapacity(size_t bucketCount, size_t /*bucketMaxItemCount*/) noexcept
	{
		return (bucketCount * maxCount / 6) * 5;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/,
		size_t /*bucketMaxItemCount*/) noexcept
	{
		return 1;
	}
};

} // namespace momo
