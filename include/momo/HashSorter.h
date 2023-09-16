/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashSorter.h

  namespace momo:
    class HashSorter

\**********************************************************/

#pragma once

#include "KeyUtility.h"
#include "IteratorUtility.h"
#include "ObjectManager.h"
#include "RadixSorter.h"

namespace momo
{

class HashSorter
{
public:
	typedef size_t HashFuncResult;

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator>
	struct Swapper
	{
		void operator()(Iterator iter1, Iterator iter2) const
		{
			std::iter_swap(iter1, iter2);
		}
	};

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator>
	struct FindResult
	{
		Iterator iterator;
		bool found;
	};

	template<internal::conceptIterator<std::random_access_iterator_tag> TIterator>
	class Bounds : public internal::ArrayBoundsBase<TIterator>
	{
	private:
		typedef internal::ArrayBoundsBase<TIterator> BoundsBase;

	public:
		using typename BoundsBase::Iterator;

	public:
		explicit Bounds(Iterator begin, Iterator end) noexcept
			: BoundsBase(begin, internal::UIntMath<>::Dist(begin, end))
		{
		}
	};

private:
	template<typename HashFunc>
	class HashFuncIter
	{
	public:
		explicit HashFuncIter(const HashFunc& hashFunc) noexcept
			: mHashFunc(hashFunc)
		{
		}

		template<typename Iterator>
		HashFuncResult operator()(Iterator iter) const
		{
			return mHashFunc(*iter);
		}

	private:
		const HashFunc& mHashFunc;
	};

	template<typename Iterator, typename HashIterator>
	class HashFuncIterPrehashed
	{
	public:
		explicit HashFuncIterPrehashed(Iterator begin, HashIterator hashBegin) noexcept
			: mBegin(begin),
			mHashBegin(hashBegin)
		{
		}

		HashFuncResult operator()(Iterator iter) const
		{
			return mHashBegin[iter - mBegin];
		}

		HashFuncResult operator()(std::reverse_iterator<Iterator> iter) const
		{
			return mHashBegin[iter.base() - 1 - mBegin];
		}

	private:
		Iterator mBegin;
		HashIterator mHashBegin;
	};

	typedef internal::UIntMath<> SMath;

public:
	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHashFunc<Item> HashFunc = HashCoder<Item>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>,
		typename SwapFunc = Swapper<Iterator>>
	requires std::invocable<const SwapFunc&, Iterator, Iterator>
	static void Sort(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc(), const SwapFunc& swapFunc = SwapFunc())
	{
		pvSort(begin, count, HashFuncIter<HashFunc>(hashFunc), equalFunc, swapFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>,
		typename SwapFunc = Swapper<Iterator>>
	requires std::is_same_v<HashFuncResult&, std::iter_reference_t<HashIterator>> &&
		std::invocable<const SwapFunc&, Iterator, Iterator>
	static void SortPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const EqualFunc& equalFunc = EqualFunc(), const SwapFunc& swapFunc = SwapFunc())
	{
		auto newSwapFunc = [begin, hashBegin, &swapFunc] (Iterator iter1, Iterator iter2)
		{
			swapFunc(iter1, iter2);
			std::swap(hashBegin[iter1 - begin], hashBegin[iter2 - begin]);
		};
		pvSort(begin, count, HashFuncIterPrehashed<Iterator, HashIterator>(begin, hashBegin),
			equalFunc, newSwapFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHashFunc<Item> HashFunc = HashCoder<Item>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	static bool IsSorted(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count, HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	requires std::is_same_v<HashFuncResult, std::iter_value_t<HashIterator>>
	static bool IsSortedPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const EqualFunc& equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count,
			HashFuncIterPrehashed<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHashFunc<Item> HashFunc = HashCoder<Item>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires internal::conceptEqualFunc<EqualFunc, Item>
	static FindResult<Iterator> Find(Iterator begin, size_t count,
		const ItemArg& itemArg, HashFuncResult itemHash,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvFind(begin, count, itemArg, itemHash, HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires std::is_same_v<HashFuncResult, std::iter_value_t<HashIterator>> &&
		internal::conceptEqualFunc<EqualFunc, Item>
	static FindResult<Iterator> FindPrehashed(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, HashIterator hashBegin, const EqualFunc& equalFunc = EqualFunc())
	{
		return pvFind(begin, count, itemArg, itemHash,
			HashFuncIterPrehashed<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHashFunc<Item> HashFunc = HashCoder<Item>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires internal::conceptEqualFunc<EqualFunc, Item>
	static Bounds<Iterator> GetBounds(Iterator begin, size_t count,
		const ItemArg& itemArg, HashFuncResult itemHash,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvGetBounds(begin, count, itemArg, itemHash, HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires std::is_same_v<HashFuncResult, std::iter_value_t<HashIterator>> &&
		internal::conceptEqualFunc<EqualFunc, Item>
	static Bounds<Iterator> GetBoundsPrehashed(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, HashIterator hashBegin, const EqualFunc& equalFunc = EqualFunc())
	{
		return pvGetBounds(begin, count, itemArg, itemHash,
			HashFuncIterPrehashed<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

private:
	template<typename Iterator, typename HashFuncIter, typename EqualFunc, typename SwapFunc>
	static void pvSort(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
		const EqualFunc& equalFunc, const SwapFunc& swapFunc)
	{
		auto groupFunc = [&equalFunc, &swapFunc] (Iterator begin, size_t count)
		{
			if (count > 2)
				pvGroup(begin, count, equalFunc, swapFunc);
		};
		internal::RadixSorter<>::Sort(begin, count, hashFuncIter, swapFunc, groupFunc);
	}

	template<typename Iterator, typename EqualFunc, typename SwapFunc>
	static void pvGroup(Iterator begin, size_t count, const EqualFunc& equalFunc,
		const SwapFunc& swapFunc)
	{
		for (size_t i = 1; i < count; ++i)
		{
			if (equalFunc(*SMath::Next(begin, i - 1), *SMath::Next(begin, i)))
				continue;
			for (size_t j = i + 1; j < count; ++j)
			{
				if (equalFunc(*SMath::Next(begin, i - 1), *SMath::Next(begin, j)))
				{
					swapFunc(SMath::Next(begin, i), SMath::Next(begin, j));
					++i;
				}
			}
		}
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc>
	static bool pvIsSorted(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
		const EqualFunc& equalFunc)
	{
		size_t prevIndex = 0;
		HashFuncResult prevHash = hashFuncIter(begin);
		for (size_t i = 1; i < count; ++i)
		{
			HashFuncResult hash = hashFuncIter(SMath::Next(begin, i));
			if (hash < prevHash)
				return false;
			if (hash != prevHash)
			{
				if (!pvIsGrouped(SMath::Next(begin, prevIndex), i - prevIndex, equalFunc))
					return false;
				prevIndex = i;
				prevHash = hash;
			}
		}
		return pvIsGrouped(SMath::Next(begin, prevIndex), count - prevIndex, equalFunc);
	}

	template<typename Iterator, typename EqualFunc>
	static bool pvIsGrouped(Iterator begin, size_t count, const EqualFunc& equalFunc)
	{
		for (size_t i = 1; i < count; ++i)
		{
			if (equalFunc(*SMath::Next(begin, i - 1), *SMath::Next(begin, i)))
				continue;
			for (size_t j = i + 1; j < count; ++j)
			{
				if (equalFunc(*SMath::Next(begin, i - 1), *SMath::Next(begin, j)))
					return false;
			}
		}
		return true;
	}

	template<typename Iterator, typename ItemArg, typename HashFuncIter, typename EqualFunc>
	static FindResult<Iterator> pvFind(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, hashFuncIter);
		if (!res.found)
			return res;
		if (equalFunc(*res.iterator, itemArg))
			return res;
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), itemArg, itemHash, hashFuncIter, equalFunc);
		if (revRes.found)
			return { revRes.iterator.base() - 1, true };
		return pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator),
			itemArg, itemHash, hashFuncIter, equalFunc);
	}

	template<typename Iterator, typename ItemArg, typename HashFuncIter, typename EqualFunc>
	static Bounds<Iterator> pvGetBounds(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, hashFuncIter);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		if (equalFunc(*res.iterator, itemArg))
		{
			Iterator resBegin = pvFindOther(std::reverse_iterator<Iterator>(res.iterator + 1),
				SMath::Dist(begin, res.iterator + 1), equalFunc).base();
			return Bounds<Iterator>(resBegin,
				pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalFunc));
		}
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), itemArg, itemHash, hashFuncIter, equalFunc);
		if (revRes.found)
		{
			Iterator resBegin = pvFindOther(revRes.iterator,
				SMath::Dist(begin, revRes.iterator.base()), equalFunc).base();
			return Bounds<Iterator>(resBegin, revRes.iterator.base());
		}
		res = pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator), itemArg, itemHash,
			hashFuncIter, equalFunc);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		return Bounds<Iterator>(res.iterator,
			pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalFunc));
	}

	template<typename Iterator, typename ItemArg, typename HashFuncIter, typename EqualFunc>
	static FindResult<Iterator> pvFindNext(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		Iterator iter = begin;
		while (true)
		{
			iter = pvFindOther(iter, count - SMath::Dist(begin, iter), equalFunc);
			if (iter == SMath::Next(begin, count) || hashFuncIter(iter) != itemHash)
				break;
			if (equalFunc(*iter, itemArg))
				return { iter, true };
		}
		return { iter, false };
	}

	template<typename Iterator, typename EqualFunc>
	static Iterator pvFindOther(Iterator begin, size_t count, const EqualFunc& equalFunc)
	{
		MOMO_ASSERT(count > 0);
		auto compareFunc = [begin, &equalFunc] (Iterator iter)
		{
			return equalFunc(*begin, *iter)
				? std::strong_ordering::less : std::strong_ordering::greater;
		};
		return pvExponentialSearch(begin + 1, count - 1, compareFunc).iterator;
	}

	template<typename Iterator, typename HashFuncIter>
	static FindResult<Iterator> pvFindHash(Iterator begin, size_t count,
		HashFuncResult itemHash, const HashFuncIter& hashFuncIter)
	{
		auto compareFunc = [itemHash, &hashFuncIter] (Iterator iter)
			{ return hashFuncIter(iter) <=> itemHash; };
		size_t leftIndex = 0;
		size_t rightIndex = count;
		size_t middleIndex = pvMultShift(itemHash, count);
		size_t step = pvGetStepCount(count);
		while (true)
		{
			HashFuncResult middleHash = hashFuncIter(SMath::Next(begin, middleIndex));
			if (middleHash < itemHash)
			{
				leftIndex = middleIndex + 1;
				if (step == 0)
				{
					return pvExponentialSearch(SMath::Next(begin, leftIndex),
						rightIndex - leftIndex, compareFunc);
				}
				middleIndex += pvMultShift(itemHash - middleHash, count);
				if (middleIndex >= rightIndex)
					break;
			}
			else if (middleHash > itemHash)
			{
				rightIndex = middleIndex;
				if (step == 0)
				{
					typedef std::reverse_iterator<Iterator> ReverseIterator;
					auto revCompareFunc = [itemHash, &hashFuncIter] (ReverseIterator iter)
						{ return itemHash <=> hashFuncIter(iter); };
					auto res = pvExponentialSearch(ReverseIterator(SMath::Next(begin, rightIndex)),
						rightIndex - leftIndex, revCompareFunc);
					return { res.iterator.base() - (res.found ? 1 : 0), res.found };
				}
				size_t diff = pvMultShift(middleHash - itemHash, count);
				if (leftIndex + diff > middleIndex)
					break;
				middleIndex -= diff;
			}
			else
			{
				return { SMath::Next(begin, middleIndex), true };
			}
			--step;
		}
		return pvBinarySearch(SMath::Next(begin, leftIndex), rightIndex - leftIndex, compareFunc);
	}

	template<typename Iterator, typename CompareFunc>
	static FindResult<Iterator> pvExponentialSearch(Iterator begin, size_t count,
		CompareFunc compareFunc)
	{
		size_t leftIndex = 0;
		for (size_t i = 0; i < count; i = i * 2 + 2)
		{
			std::strong_ordering cmp = compareFunc(SMath::Next(begin, i));
			if (cmp > 0)
				return pvBinarySearch(SMath::Next(begin, leftIndex), i - leftIndex, compareFunc);
			else if (cmp == 0)
				return { SMath::Next(begin, i), true };
			leftIndex = i + 1;
		}
		return pvBinarySearch(SMath::Next(begin, leftIndex), count - leftIndex, compareFunc);
	}

	template<typename Iterator, typename CompareFunc>
	static FindResult<Iterator> pvBinarySearch(Iterator begin, size_t count,
		CompareFunc compareFunc)
	{
		size_t leftIndex = 0;
		size_t rightIndex = count;
		while (leftIndex < rightIndex)
		{
			size_t middleIndex = (leftIndex + rightIndex) / 2;
			std::strong_ordering cmp = compareFunc(SMath::Next(begin, middleIndex));
			if (cmp < 0)
				leftIndex = middleIndex + 1;
			else if (cmp > 0)
				rightIndex = middleIndex;
			else
				return { SMath::Next(begin, middleIndex), true };
		}
		return { SMath::Next(begin, leftIndex), false };
	}

	static size_t pvGetStepCount(size_t count) noexcept
	{
		return (count < 1 << 6) ? 0 : (count < 1 << 12) ? 1 : (count < 1 << 22) ? 2 : 3;
	}

	static size_t pvMultShift(HashFuncResult value1, size_t value2) noexcept
	{
		static_assert(sizeof(HashFuncResult) >= sizeof(size_t));
		static const size_t halfSize = 4 * sizeof(HashFuncResult);
		static const HashFuncResult halfMask = (HashFuncResult{1} << halfSize) - 1;
		HashFuncResult res = (value1 >> halfSize) * (value2 >> halfSize)
			+ (((value1 >> halfSize) * (value2 & halfMask)) >> halfSize)
			+ (((value2 >> halfSize) * (value1 & halfMask)) >> halfSize);
		return static_cast<size_t>(res);
	}
};

} // namespace momo
