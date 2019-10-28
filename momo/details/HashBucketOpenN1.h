/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

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
	template<typename TItemTraits, bool tUseSSE2, size_t tMaxCount, bool tReverse>
	class BucketOpenN1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const bool useSSE2 = tUseSSE2;
		static const bool reverse = tReverse;

//#ifndef MOMO_USE_SSE2
//		MOMO_STATIC_ASSERT(!useSSE2);
//#endif

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
		static const uint8_t infProbe = 255;

		typedef typename std::conditional<useSSE2, int64_t, int8_t>::type Data8;

	public:
		explicit BucketOpenN1() MOMO_NOEXCEPT
		{
			pvSetEmpty();
		}

		BucketOpenN1(const BucketOpenN1&) = delete;

		~BucketOpenN1() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetCount() == 0);
		}

		BucketOpenN1& operator=(const BucketOpenN1&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(pvMakeIterator(pvGetItemPtr(0)), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			uint8_t shortHash = pvCalcShortHash(hashCode);
#ifdef MOMO_USE_SSE2
			if (useSSE2)
			{
				//MOMO_PREFETCH_RANGE(this, sizeof(*this));
				__m128i shortHashes = _mm_set1_epi8(shortHash);
				__m128i thisShortHashes = _mm_set_epi64x((int64_t)0, (int64_t)mData8);
				int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(shortHashes, thisShortHashes));
				mask &= (1 << maxCount) - 1;
				while (mask != 0)
				{
					size_t index = (size_t)pvCountTrailingZeros((uint8_t)mask);
					if (pred(*&mItems[index]))
						return pvMakeIterator(&mItems[index]);
					mask &= mask - 1;
				}
				return Iterator(nullptr);
			}
#endif
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mData.shortHashes[i] == shortHash && pred(*&mItems[i]))
					return pvMakeIterator(&mItems[i]);
			}
			return Iterator(nullptr);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return pvGetState() < emptyShortHash;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return true;
		}

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			if (mData.maxProbe == infProbe)
				return ((size_t)1 << logBucketCount) - 1;
			return (size_t)(mData.maxProbe & 7) << (mData.maxProbe >> 3);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			Item* begin = &mItems[0] + (reverse ? maxCount - count : 0);
			ItemTraits::Destroy(params.GetMemManager(), begin, count);
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t logBucketCount, size_t probe)
			MOMO_NOEXCEPT_IF(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = pvGetItemPtr(count);
			std::forward<ItemCreator>(itemCreator)(pitem);
			pvGetShortHash(count) = pvCalcShortHash(hashCode);
			if (probe > 0)
				pvSetMaxProbe(hashCode, logBucketCount, probe);
			if (count + 1 < maxCount)
				++pvGetState();
			return pvMakeIterator(pitem);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = iter - pvMakeIterator(pvGetItemPtr(0));
			MOMO_ASSERT(index < count);
			std::forward<ItemReplacer>(itemReplacer)(*pvGetItemPtr(count - 1),
				*pvGetItemPtr(index));
			pvGetShortHash(index) = pvGetShortHash(count - 1);
			pvGetShortHash(count - 1) = emptyShortHash;
			if (count < maxCount)
				--pvGetState();
			else
				pvGetState() = emptyShortHash + (uint8_t)maxCount - 1;
			return iter;
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t /*probe*/) MOMO_NOEXCEPT
		{
			return (bucketIndex + 1) & (bucketCount - 1);
		}

	private:
		uint8_t pvGetState() const MOMO_NOEXCEPT
		{
			return mData.shortHashes[reverse ? 0 : maxCount - 1];
		}

		uint8_t& pvGetState() MOMO_NOEXCEPT
		{
			return pvGetShortHash(maxCount - 1);
		}

		uint8_t& pvGetShortHash(size_t index) MOMO_NOEXCEPT
		{
			return mData.shortHashes[reverse ? maxCount - 1 - index : index];
		}

		Item* pvGetItemPtr(size_t index) MOMO_NOEXCEPT
		{
			return &mItems[reverse ? maxCount - 1 - index : index];
		}

		static Iterator pvMakeIterator(Item* pitem) MOMO_NOEXCEPT
		{
			return Iterator(pitem + (reverse ? 1 : 0));
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			uint8_t state = pvGetState();
			return (state >= emptyShortHash) ? (size_t)(state - emptyShortHash) : maxCount;
		}

		static uint8_t pvCalcShortHash(size_t hashCode) MOMO_NOEXCEPT
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyShortHash) >> 24);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			mData8 = (Data8)(-1);
			mData.maxProbe = (uint8_t)0;
			std::fill_n(mData.shortHashes, maxCount, (uint8_t)emptyShortHash);
		}

		void pvSetMaxProbe(size_t hashCode, size_t logBucketCount, size_t probe) MOMO_NOEXCEPT
		{
			size_t bucketCount = (size_t)1 << logBucketCount;
			size_t startBucketIndex = BucketBase::GetStartBucketIndex(hashCode, bucketCount);
			size_t thisBucketIndex = (startBucketIndex + probe) & (bucketCount - 1);
			BucketOpenN1* startBucket = this - (ptrdiff_t)thisBucketIndex
				+ (ptrdiff_t)startBucketIndex;
			if (probe <= startBucket->GetMaxProbe(logBucketCount))
				return;
			size_t maxProbe0 = probe - 1;
			size_t maxProbe1 = 0;
			while (maxProbe0 >= (size_t)7)
			{
				maxProbe0 >>= 1;
				++maxProbe1;
			}
			startBucket->mData.maxProbe = (maxProbe1 <= (size_t)31)
				? (uint8_t)(maxProbe0 + 1) | (uint8_t)(maxProbe1 << 3) : infProbe;
		}

		static uint8_t pvCountTrailingZeros(uint8_t mask) MOMO_NOEXCEPT
		{
			MOMO_ASSERT((uint8_t)0 < mask && mask < (uint8_t)128);
			static const uint8_t tab[127] =
			{
				   0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			};
			return tab[mask - 1];
		}

	private:
		union
		{
			struct
			{
				uint8_t shortHashes[maxCount];
				uint8_t maxProbe;
			} mData;
			Data8 mData8;
		};
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

#ifdef MOMO_USE_SSE2
template<bool tUseSSE2 = true,
	size_t tMaxCount = tUseSSE2 ? 7 : 3,
	bool tReverse = !tUseSSE2>
#else
template<bool tUseSSE2 = false,
	size_t tMaxCount = 3,
	bool tReverse = true>
#endif
class HashBucketOpenN1 : public internal::HashBucketBase
{
public:
	static const bool useSSE2 = tUseSSE2;
	static const size_t maxCount = tMaxCount;
	static const bool reverse = tReverse;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOpenN1<ItemTraits, useSSE2, maxCount, reverse>;

public:
	static size_t CalcCapacity(size_t bucketCount, size_t /*bucketMaxItemCount*/) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 7) * 6;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/,
		size_t /*bucketMaxItemCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}
};

} // namespace momo
