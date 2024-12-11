/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketOpen2N2.h

  namespace momo:
    class HashBucketOpen2N2

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<size_t maxCount>
	concept conceptBucketOpen2N2MaxCount = (0 < maxCount && maxCount < 4);

	template<typename TItemTraits, size_t tMaxCount, bool tUseHashCodePartGetter>
	requires conceptBucketOpen2N2MaxCount<tMaxCount>
	class BucketOpen2N2 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const bool useHashCodePartGetter = tUseHashCodePartGetter;

	public:
		static const size_t maxCount = tMaxCount;

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef std::reverse_iterator<Item*> Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		template<size_t count, bool useHashCodePartGetter>
		struct HashData;

		template<size_t count>
		struct HashData<count, false>
		{
			union
			{
				uint16_t shortHashes[count];
				uint8_t hashProbes[count];
			};
		};

		template<size_t count>
		struct HashData<count, true>
		{
			uint8_t shortHashes[count];
			uint8_t hashProbes[count];
		};

		typedef typename UIntSelector<useHashCodePartGetter ? 1 : 2>::UInt ShortHash;

		static const size_t hashCodeShift = sizeof(size_t) * 8 - sizeof(ShortHash) * 8 + 1;
		static const ShortHash emptyShortHash = ShortHash{1} << (sizeof(ShortHash) * 8 - 1);
		static const uint8_t emptyHashProbe = 255;

		static const size_t logBucketCountStep = 8;
		static const size_t logBucketCountAddend = 6;

	public:
		explicit BucketOpen2N2() noexcept
		{
			pvSetEmpty();
		}

		BucketOpen2N2(const BucketOpen2N2&) = delete;

		~BucketOpen2N2() noexcept = default;

		BucketOpen2N2& operator=(const BucketOpen2N2&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return Bounds(Iterator(mItems.GetPointer() + maxCount), pvGetCount());
		}

		template<bool first, conceptObjectPredicate<Item> ItemPredicate>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/,
			FastCopyableFunctor<ItemPredicate> itemPred, size_t hashCode)
		{
			return pvFind(itemPred, hashCode);
		}

		bool IsFull() const noexcept
		{
			return mHashData.shortHashes[0] < emptyShortHash;
		}

		bool WasFull() const noexcept
		{
			return true;
		}

		size_t GetMaxProbe(size_t /*logBucketCount*/) const noexcept
		{
			return pvGetMaxProbe();
		}

		void UpdateMaxProbe(size_t probe) noexcept
		{
			if (probe == 0 || probe <= pvGetMaxProbe())
				return;
			if (probe <= size_t{255})
			{
				mState[0] = static_cast<uint8_t>(probe);
				return;
			}
			pvUpdateMaxProbe(probe);
		}

		void Clear(Params& /*params*/) noexcept
		{
			pvSetEmpty();
		}

		template<conceptObjectCreator<Item> ItemCreator>
		Iterator AddCrt(Params& /*params*/, FastMovableFunctor<ItemCreator> itemCreator,
			size_t hashCode, size_t logBucketCount, size_t probe)
			noexcept(std::is_nothrow_invocable_v<ItemCreator&&, Item*>)
		{
			return pvAdd(std::move(itemCreator), hashCode, logBucketCount, probe);
		}

		template<conceptObjectReplacer<Item> ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter,
			FastMovableFunctor<ItemReplacer> itemReplacer)
		{
			return pvRemove(iter, std::move(itemReplacer));
		}

		template<conceptConstFunctor<size_t> HashCodeFullGetter>
		size_t GetHashCodePart(FastCopyableFunctor<HashCodeFullGetter> hashCodeFullGetter,
			Iterator iter, size_t bucketIndex, size_t logBucketCount, size_t newLogBucketCount)
		{
			if (!useHashCodePartGetter)
				return hashCodeFullGetter();
			size_t index = UIntMath<>::Dist(mItems.GetPointer(), std::to_address(iter));
			uint8_t hashProbe = mHashData.hashProbes[index];
			bool useFullGetter = (hashProbe == emptyHashProbe ||
				(logBucketCount + logBucketCountAddend) / logBucketCountStep
				!= (newLogBucketCount + logBucketCountAddend) / logBucketCountStep);
			if (useFullGetter)
				return hashCodeFullGetter();
			size_t probeShift = pvGetProbeShift(logBucketCount);
			MOMO_ASSERT(probeShift > 0);
			size_t probe = size_t{hashProbe} & ((size_t{1} << probeShift) - 1);
			size_t probe2 = (probe % 2 == 0) ? (probe / 2) * (probe + 1) : probe * ((probe + 1) / 2);
			size_t bucketCount = size_t{1} << logBucketCount;
			return ((bucketIndex - probe2) & (bucketCount - 1))
				| ((size_t{hashProbe} >> probeShift) << logBucketCount)
				| (size_t{mHashData.shortHashes[index]} << hashCodeShift);
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t probe) noexcept
		{
			return (bucketIndex + probe) & (bucketCount - 1);	// quadratic probing
		}

	private:
		void pvSetEmpty() noexcept
		{
			std::fill_n(mHashData.shortHashes, maxCount, ShortHash{emptyShortHash});
			mState[0] = uint8_t{0};
			mState[1] = uint8_t{0};
		}

		template<conceptObjectPredicate<Item> ItemPredicate>
		MOMO_FORCEINLINE Iterator pvFind(FastCopyableFunctor<ItemPredicate> itemPred,
			size_t hashCode)
		{
			ShortHash shortHash = pvCalcShortHash(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashData.shortHashes[i] == shortHash)
				{
					Item* items = mItems.GetPointer();
					if (itemPred(std::as_const(items[i]))) [[likely]]
						return Iterator(items + i + 1);
				}
			}
			return Iterator();
		}

		template<conceptObjectCreator<Item> ItemCreator>
		Iterator pvAdd(FastMovableFunctor<ItemCreator> itemCreator,
			size_t hashCode, size_t logBucketCount, size_t probe)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* newItem = mItems.GetPointer() + maxCount - 1 - count;
			std::move(itemCreator)(newItem);
			mHashData.shortHashes[maxCount - 1 - count] = pvCalcShortHash(hashCode);
			if constexpr (useHashCodePartGetter)
			{
				uint8_t& hashProbe = mHashData.hashProbes[maxCount - 1 - count];
				size_t probeShift = pvGetProbeShift(logBucketCount);
				if (probe < (size_t{1} << probeShift))
					hashProbe = static_cast<uint8_t>(((hashCode >> logBucketCount) << probeShift) | probe);
				else
					hashProbe = emptyHashProbe;
			}
			++mState[1];
			return Iterator(newItem + 1);
		}

		template<conceptObjectReplacer<Item> ItemReplacer>
		Iterator pvRemove(Iterator iter, FastMovableFunctor<ItemReplacer> itemReplacer)
		{
			size_t count = pvGetCount();
			Item* items = mItems.GetPointer();
			size_t index = UIntMath<>::Dist(items, std::to_address(iter));
			MOMO_ASSERT(index >= maxCount - count);
			std::move(itemReplacer)(items[maxCount - count], items[index]);
			mHashData.shortHashes[index] = mHashData.shortHashes[maxCount - count];
			mHashData.shortHashes[maxCount - count] = emptyShortHash;
			if constexpr (useHashCodePartGetter)
				mHashData.hashProbes[index] = mHashData.hashProbes[maxCount - count];
			--mState[1];
			return iter;
		}

		static ShortHash pvCalcShortHash(size_t hashCode) noexcept
		{
			return static_cast<ShortHash>(hashCode >> hashCodeShift);
		}

		static size_t pvGetProbeShift(size_t logBucketCount) noexcept
		{
			return (logBucketCount + logBucketCountAddend + 1) % logBucketCountStep;
		}

		size_t pvGetCount() const noexcept
		{
			return size_t{mState[1]} & 3;
		}

		size_t pvGetMaxProbe() const noexcept
		{
			return size_t{mState[0]} << (mState[1] >> 2);
		}

		MOMO_NOINLINE void pvUpdateMaxProbe(size_t probe) noexcept
		{
			size_t maxProbe0 = probe - 1;
			size_t maxProbe1 = 0;
			while (maxProbe0 >= size_t{255})
			{
				maxProbe0 >>= 1;
				++maxProbe1;
			}
			mState[0] = static_cast<uint8_t>(maxProbe0) + 1;
			mState[1] &= uint8_t{3};
			mState[1] |= static_cast<uint8_t>(maxProbe1 << 2);
		}

	private:
		uint8_t mState[2];
		HashData<maxCount, useHashCodePartGetter> mHashData;
		ObjectBuffer<Item, ItemTraits::alignment, maxCount> mItems;
	};
}

template<size_t tMaxCount = 3>
requires internal::conceptBucketOpen2N2MaxCount<tMaxCount>
class HashBucketOpen2N2 : public internal::HashBucketBase
{
public:
	static const size_t maxCount = tMaxCount;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOpen2N2<ItemTraits, maxCount, useHashCodePartGetter>;

public:
	static size_t CalcCapacity(size_t bucketCount, size_t /*bucketMaxItemCount*/) noexcept
	{
		return static_cast<size_t>(static_cast<double>(bucketCount * maxCount) / 12.0 * 11.0);
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/,
		size_t /*bucketMaxItemCount*/) noexcept
	{
		return 1;
	}
};

} // namespace momo
