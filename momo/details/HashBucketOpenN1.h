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
		typename TData = std::array<char, tMaxCount + 1>>
	class BucketOpenN1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TData Data;

		static const bool reverse = tReverse;

	public:
		static const size_t maxCount = tMaxCount;
		static_assert(0 < maxCount && maxCount < 8);
		static_assert(maxCount + 1 <= sizeof(Data));

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef typename std::conditional<reverse,
			std::reverse_iterator<Item*>, Item*>::type Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		typedef unsigned char Byte;

		static const Byte emptyShortHash = 248;
		static const Byte infProbeExp = 255;

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

		template<bool first, typename Predicate>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			Byte shortHash = ptCalcShortHash(hashCode);
			const Byte* thisShortHashes = pvGetShortHashes();
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
			Byte maxProbeExp = pvGetMaxProbeExp();
			if (maxProbeExp == infProbeExp)
				return (size_t{1} << logBucketCount) - 1;
			return pvGetMaxProbe(maxProbeExp);
		}

		void UpdateMaxProbe(size_t probe) noexcept
		{
			if (probe == 0)
				return;
			Byte maxProbeExp = pvGetMaxProbeExp();
			if (maxProbeExp == infProbeExp || probe <= pvGetMaxProbe(maxProbeExp))
				return;
			pvUpdateMaxProbe(probe);
		}

		void Clear(Params& /*params*/) noexcept
		{
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/, size_t /*probe*/)
			noexcept(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* newItem = ptGetItemPtr(count);
			std::forward<ItemCreator>(itemCreator)(newItem);
			pvGetShortHash(count) = ptCalcShortHash(hashCode);
			if (count + 1 < maxCount)
				++pvGetState();
			return pvMakeIterator(newItem);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = internal::UIntMath<>::Dist(pvMakeIterator(ptGetItemPtr(0)), iter);
			MOMO_ASSERT(index < count);
			std::forward<ItemReplacer>(itemReplacer)(*ptGetItemPtr(count - 1),
				*ptGetItemPtr(index));
			pvGetShortHash(index) = pvGetShortHash(count - 1);
			pvGetShortHash(count - 1) = emptyShortHash;
			if (count < maxCount)
				--pvGetState();
			else
				pvGetState() = emptyShortHash + static_cast<Byte>(maxCount) - 1;
			return iter;
		}

	protected:
		const Data& ptGetData() const noexcept
		{
			return mData;
		}

		Item* ptGetItemPtr(size_t index) noexcept
		{
			return &mItems[reverse ? maxCount - 1 - index : index];
		}

		static Byte ptCalcShortHash(size_t hashCode) noexcept
		{
			uint32_t hashCode24 = static_cast<uint32_t>(hashCode >> (sizeof(size_t) * 8 - 24));
			return static_cast<Byte>((hashCode24 * uint32_t{emptyShortHash}) >> 24);
		}

	private:
		Byte pvGetState() const noexcept
		{
			return *pvGetBytePtr(reverse ? 0 : maxCount - 1);
		}

		Byte& pvGetState() noexcept
		{
			return *pvGetBytePtr(reverse ? 0 : maxCount - 1);
		}

		Byte pvGetMaxProbeExp() const noexcept
		{
			return *pvGetBytePtr(maxCount);
		}

		Byte& pvGetMaxProbeExp() noexcept
		{
			return *pvGetBytePtr(maxCount);
		}

		Byte& pvGetShortHash(size_t index) noexcept
		{
			return *pvGetBytePtr(reverse ? maxCount - 1 - index : index);
		}

		Byte* pvGetShortHashes() noexcept
		{
			return pvGetBytePtr(0);
		}

		const Byte* pvGetBytePtr(size_t index) const noexcept
		{
			return PtrCaster::Shift<const Byte>(&mData, index);
		}

		Byte* pvGetBytePtr(size_t index) noexcept
		{
			return PtrCaster::Shift<Byte>(&mData, index);
		}

		static Iterator pvMakeIterator(Item* itemPtr) noexcept
		{
			return Iterator(itemPtr + (reverse ? 1 : 0));
		}

		size_t pvGetCount() const noexcept
		{
			Byte state = pvGetState();
			return (state >= emptyShortHash) ? size_t{state} - size_t{emptyShortHash} : maxCount;
		}

		void pvSetEmpty() noexcept
		{
			mData = Data();
			std::fill_n(pvGetShortHashes(), maxCount, Byte{emptyShortHash});
			pvGetMaxProbeExp() = Byte{0};
		}

		static size_t pvGetMaxProbe(Byte maxProbeExp) noexcept
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
				? static_cast<Byte>(maxProbe0 + 1) | static_cast<Byte>(maxProbe1 << 3)
				: infProbeExp;
		}

	private:
		Data mData;
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
		return static_cast<size_t>(static_cast<double>(bucketCount * maxCount) / 6.0 * 5.0);
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/,
		size_t /*bucketMaxItemCount*/) noexcept
	{
		return 1;
	}
};

} // namespace momo
