/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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
	template<size_t maxCount>
	concept conceptBucketOpenN1MaxCount = (0 < maxCount && maxCount < 8);

	template<typename TItemTraits, size_t tMaxCount, bool tReverse>
	requires conceptBucketOpenN1MaxCount<tMaxCount>
	class BucketOpenN1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const bool reverse = tReverse;

	public:
		static const size_t maxCount = tMaxCount;

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef std::conditional_t<reverse, std::reverse_iterator<Item*>, Item*> Iterator;
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

		~BucketOpenN1() noexcept = default;

		BucketOpenN1& operator=(const BucketOpenN1&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return Bounds(pvMakeIterator(ptGetItemPtr(0)), pvGetCount());
		}

		template<bool first, typename Predicate>
		requires std::predicate<Predicate, const Item&>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/, Predicate pred, size_t hashCode)
		{
			return pvFind(pred, hashCode);
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
				return (size_t{1} << logBucketCount) - 1;
			return pvGetMaxProbe(maxProbeExp);
		}

		void UpdateMaxProbe(size_t probe) noexcept
		{
			if (probe == 0)
				return;
			uint8_t maxProbeExp = pvGetMaxProbeExp();
			if (maxProbeExp == infProbeExp || probe <= pvGetMaxProbe(maxProbeExp))
				return;
			pvUpdateMaxProbe(probe);
		}

		void Clear(Params& /*params*/) noexcept
		{
			pvSetEmpty();
		}

		template<conceptCreator<Item> ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
			noexcept(std::is_nothrow_invocable_v<ItemCreator&&, Item*>)
		{
			return pvAdd(std::move(itemCreator), hashCode);
		}

		template<conceptReplacer<Item> ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer itemReplacer)
		{
			return pvRemove(iter, std::move(itemReplacer));
		}

	protected:
		const void* ptGetData() const noexcept
		{
			return mData;
		}

		Item* ptGetItemPtr(size_t index) noexcept
		{
			return &mItems + (reverse ? maxCount - 1 - index : index);
		}

		static uint8_t ptCalcShortHash(size_t hashCode) noexcept
		{
			uint32_t hashCode24 = static_cast<uint32_t>(hashCode >> (sizeof(size_t) * 8 - 24));
			return static_cast<uint8_t>((hashCode24 * uint32_t{emptyShortHash}) >> 24);
		}

	private:
		void pvSetEmpty() noexcept
		{
			std::fill_n(pvGetShortHashes(), maxCount, uint8_t{emptyShortHash});
			pvGetMaxProbeExp() = uint8_t{0};
		}

		template<typename Predicate>
		MOMO_FORCEINLINE Iterator pvFind(Predicate pred, size_t hashCode)
		{
			uint8_t shortHash = ptCalcShortHash(hashCode);
			const uint8_t* thisShortHashes = pvGetShortHashes();
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (thisShortHashes[i] == shortHash)
				{
					if (pred(std::as_const((&mItems)[i]))) [[likely]]
						return pvMakeIterator(&mItems + i);
				}
			}
			return Iterator();
		}

		template<typename ItemCreator>
		Iterator pvAdd(ItemCreator itemCreator, size_t hashCode)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* newItem = ptGetItemPtr(count);
			std::move(itemCreator)(newItem);
			pvGetShortHash(count) = ptCalcShortHash(hashCode);
			if (count + 1 < maxCount)
				++pvGetState();
			return pvMakeIterator(newItem);
		}

		template<typename ItemReplacer>
		Iterator pvRemove(Iterator iter, ItemReplacer itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = internal::UIntMath<>::Dist(pvMakeIterator(ptGetItemPtr(0)), iter);
			MOMO_ASSERT(index < count);
			std::move(itemReplacer)(*ptGetItemPtr(count - 1), *ptGetItemPtr(index));
			pvGetShortHash(index) = pvGetShortHash(count - 1);
			pvGetShortHash(count - 1) = emptyShortHash;
			if (count < maxCount)
				--pvGetState();
			else
				pvGetState() = emptyShortHash + static_cast<uint8_t>(maxCount) - 1;
			return iter;
		}

		uint8_t pvGetState() const noexcept
		{
			return mData[reverse ? 0 : maxCount - 1];
		}

		uint8_t& pvGetState() noexcept
		{
			return mData[reverse ? 0 : maxCount - 1];
		}

		uint8_t pvGetMaxProbeExp() const noexcept
		{
			return mData[maxCount];
		}

		uint8_t& pvGetMaxProbeExp() noexcept
		{
			return mData[maxCount];
		}

		uint8_t& pvGetShortHash(size_t index) noexcept
		{
			return mData[reverse ? maxCount - 1 - index : index];
		}

		uint8_t* pvGetShortHashes() noexcept
		{
			return mData;
		}

		static Iterator pvMakeIterator(Item* itemPtr) noexcept
		{
			return Iterator(itemPtr + (reverse ? 1 : 0));
		}

		size_t pvGetCount() const noexcept
		{
			uint8_t state = pvGetState();
			return (state >= emptyShortHash) ? size_t{state} - size_t{emptyShortHash} : maxCount;
		}

		static size_t pvGetMaxProbe(uint8_t maxProbeExp) noexcept
		{
			return (size_t{maxProbeExp} & 7) << (maxProbeExp >> 3);
		}

		void pvUpdateMaxProbe(size_t probe) noexcept
		{
			size_t maxProbe0 = probe - 1;
			size_t maxProbe1 = 0;
			while (maxProbe0 >= size_t{7})
			{
				maxProbe0 >>= 1;
				++maxProbe1;
			}
			pvGetMaxProbeExp() = (maxProbe1 <= size_t{31})
				? static_cast<uint8_t>(maxProbe0 + 1) | static_cast<uint8_t>(maxProbe1 << 3)
				: infProbeExp;
		}

	private:
		uint8_t mData[maxCount + 1];
		ObjectBuffer<Item, ItemTraits::alignment, maxCount> mItems;
	};
}

template<size_t tMaxCount = 3,
	bool tReverse = true>
requires internal::conceptBucketOpenN1MaxCount<tMaxCount>
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
		return static_cast<size_t>(static_cast<double>(bucketCount * maxCount) / 6.0 * 5.0);
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/,
		size_t /*bucketMaxItemCount*/) noexcept
	{
		return 1;
	}
};

} // namespace momo
