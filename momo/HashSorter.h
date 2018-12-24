/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashSorter.h

  namespace momo::experimental:
    class HashSorter

\**********************************************************/

#pragma once

#include "Array.h"

namespace momo
{

namespace experimental
{

class HashSorter
{
public:
	typedef Array<size_t> HashArray;

private:
	static const size_t radixSize = 8;

	typedef size_t HashFuncResult;

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
	class HashFuncIterExt
	{
	public:
		HashFuncIterExt(Iterator begin, HashIterator hashBegin) noexcept
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

public:
	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static HashArray SortRet(Iterator begin, size_t count,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc(),
		typename HashArray::MemManager&& memManager = typename HashArray::MemManager())
	{
		HashArray hashArray(count, std::move(memManager));
		SortExt(hashArray.GetBegin(), begin, count, hashFunc, equalFunc);
		return hashArray;
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static void Sort(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
	{
		auto swapFunc = [] (Iterator iter1, Iterator iter2) { std::iter_swap(iter1, iter2); };
		pvSort(begin, count, HashFuncIter<HashFunc>(hashFunc), equalFunc,
			swapFunc, 8 * sizeof(HashFuncResult) - radixSize);
	}

	template<typename HashIterator, typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static void SortExt(HashIterator hashBegin, Iterator begin, size_t count,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		for (size_t i = 0; i < count; ++i)
			hashBegin[i] = hashFunc(begin[i]);
		auto swapFunc = [begin, hashBegin] (Iterator iter1, Iterator iter2)
		{
			std::iter_swap(iter1, iter2);
			std::swap(hashBegin[iter1 - begin], hashBegin[iter2 - begin]);
		};
		pvSort(begin, count, HashFuncIterExt<Iterator, HashIterator>(begin, hashBegin), equalFunc,
			swapFunc, 8 * sizeof(HashFuncResult) - radixSize);
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static bool IsSorted(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count, HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

	template<typename HashIterator, typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static bool IsSortedExt(HashIterator hashBegin, Iterator begin, size_t count,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count,
			HashFuncIterExt<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, bool> Find(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvFind(begin, count, item, hashFunc(item),
			HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

	template<typename HashIterator, typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, bool> FindExt(HashIterator hashBegin, Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvFind(begin, count, item, hashFunc(item),
			HashFuncIterExt<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, Iterator> EqualRange(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvEqualRange(begin, count, item, hashFunc(item),
			HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

	template<typename HashIterator, typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, Iterator> EqualRangeExt(HashIterator hashBegin, Iterator begin,
		size_t count, const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvEqualRange(begin, count, item, hashFunc(item),
			HashFuncIterExt<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

private:
	template<typename Iterator, typename HashFuncIter, typename EqualFunc, typename SwapFunc>
	static void pvSort(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
		const EqualFunc& equalFunc, SwapFunc swapFunc, size_t shift)
	{
		switch (count)
		{
		case 0:
		case 1:
			break;
		case 2:
			if (hashFuncIter(begin) > hashFuncIter(begin + 1))
				swapFunc(begin, begin + 1);
			break;
		default:
			{
				if (count <= (size_t)1 << (radixSize / 2 + 1))
					pvSelectionSort(begin, count, hashFuncIter, equalFunc, swapFunc);
				else
					pvRadixSort(begin, count, hashFuncIter, equalFunc, swapFunc, shift);
			}
		}
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc, typename SwapFunc>
	static void pvSelectionSort(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
		const EqualFunc& equalFunc, SwapFunc swapFunc)
	{
		MOMO_ASSERT(count > 0);
		HashFuncResult hashes[1 << (radixSize / 2 + 1)];	//?
		for (size_t i = 0; i < count; ++i)
			hashes[i] = hashFuncIter(begin + i);
		for (size_t i = 0; i < count - 1; ++i)
		{
			size_t minIndex = std::min_element(hashes + i + 1, hashes + count) - hashes;
			if (hashes[minIndex] < hashes[i])
			{
				swapFunc(begin + i, begin + minIndex);
				std::swap(hashes[i], hashes[minIndex]);
			}
		}
		size_t prevIndex = 0;
		for (size_t i = 1; i < count; ++i)
		{
			if (hashes[i] != hashes[prevIndex])
			{
				pvGroupIf(begin + prevIndex, i - prevIndex, equalFunc, swapFunc);
				prevIndex = i;
			}
		}
		pvGroupIf(begin + prevIndex, count - prevIndex, equalFunc, swapFunc);
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc, typename SwapFunc>
	static void pvRadixSort(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
		const EqualFunc& equalFunc, SwapFunc swapFunc, size_t shift)
	{
		static const size_t radixCount = 1 << radixSize;
		size_t endIndices[radixCount] = {};
		HashFuncResult hash0 = hashFuncIter(begin);
		++endIndices[pvGetRadix(hash0, shift)];
		bool singleHash = true;
		for (size_t i = 1; i < count; ++i)
		{
			HashFuncResult hash = hashFuncIter(begin + i);
			++endIndices[pvGetRadix(hash, shift)];
			singleHash &= (hash == hash0);
		}
		if (singleHash)
			return pvGroup(begin, count, equalFunc, swapFunc);
		for (size_t r = 1; r < radixCount; ++r)
			endIndices[r] += endIndices[r - 1];
		pvRadixSort(begin, hashFuncIter, swapFunc, shift, endIndices);
		size_t nextShift = (shift > radixSize) ? shift - radixSize : 0;
		size_t beginIndex = 0;
		for (size_t e : endIndices)
		{
			if (shift > 0)
				pvSort(begin + beginIndex, e - beginIndex, hashFuncIter, equalFunc, swapFunc, nextShift);
			else
				pvGroupIf(begin + beginIndex, e - beginIndex, equalFunc, swapFunc);
			beginIndex = e;
		}
	}

	template<typename Iterator, typename HashFuncIter, typename SwapFunc>
	static void pvRadixSort(Iterator begin, const HashFuncIter& hashFuncIter, SwapFunc swapFunc,
		size_t shift, const size_t* endIndices)
	{
		static const size_t radixCount = 1 << radixSize;
		size_t beginIndices[radixCount];
		beginIndices[0] = 0;
		for (size_t r = 1; r < radixCount; ++r)
			beginIndices[r] = endIndices[r - 1];
		for (size_t r = 0; r < radixCount; ++r)
		{
			size_t& beginIndex = beginIndices[r];
			size_t endIndex = endIndices[r];
			while (beginIndex < endIndex)
			{
				size_t radix = pvGetRadix(hashFuncIter(begin + beginIndex), shift);
				if (radix != r)
					swapFunc(begin + beginIndex, begin + beginIndices[radix]);
				++beginIndices[radix];
			}
		}
	}

	template<typename Iterator, typename EqualFunc, typename SwapFunc>
	static void pvGroupIf(Iterator begin, size_t count, const EqualFunc& equalFunc,
		SwapFunc swapFunc)
	{
		if (count > 2)
			pvGroup(begin, count, equalFunc, swapFunc);
	}

	template<typename Iterator, typename EqualFunc, typename SwapFunc>
	static void pvGroup(Iterator begin, size_t count, const EqualFunc& equalFunc, SwapFunc swapFunc)
	{
		for (size_t i = 1; i < count; ++i)
		{
			if (equalFunc(begin[i - 1], begin[i]))
				continue;
			for (size_t j = i + 1; j < count; ++j)
			{
				if (equalFunc(begin[i - 1], begin[j]))
				{
					swapFunc(begin + i, begin + j);
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
			HashFuncResult hash = hashFuncIter(begin + i);
			if (hash < prevHash)
				return false;
			if (hash != prevHash)
			{
				if (!pvIsGrouped(begin + prevIndex, i - prevIndex, equalFunc))
					return false;
				prevIndex = i;
			}
		}
		return pvIsGrouped(begin + prevIndex, count - prevIndex, equalFunc);
	}

	template<typename Iterator, typename EqualFunc>
	static bool pvIsGrouped(Iterator begin, size_t count, const EqualFunc& equalFunc)
	{
		for (size_t i = 1; i < count; ++i)
		{
			if (equalFunc(begin[i - 1], begin[i]))
				continue;
			for (size_t j = i + 1; j < count; ++j)
			{
				if (equalFunc(begin[i - 1], begin[j]))
					return false;
			}
		}
		return true;
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc>
	static std::pair<Iterator, bool> pvFind(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashFuncResult itemHash,
		const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, hashFuncIter);
		if (!res.second)
			return res;
		if (equalFunc(*res.first, item))
			return res;
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.first + 1),
			res.first + 1 - begin, item, itemHash, hashFuncIter, equalFunc);
		if (revRes.second)
			return { revRes.first.base() - 1, true };
		return pvFindNext(res.first, begin + count - res.first,
			item, itemHash, hashFuncIter, equalFunc);
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc>
	static std::pair<Iterator, Iterator> pvEqualRange(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashFuncResult itemHash,
		const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, hashFuncIter);
		if (!res.second)
			return { res.first, res.first };
		if (equalFunc(*res.first, item))
		{
			Iterator resBegin = pvFindOther(std::reverse_iterator<Iterator>(res.first + 1),
				res.first + 1 - begin, equalFunc).base();
			return { resBegin, pvFindOther(res.first, begin + count - res.first, equalFunc) };
		}
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.first + 1),
			res.first + 1 - begin, item, itemHash, hashFuncIter, equalFunc);
		if (revRes.second)
		{
			Iterator resBegin = pvFindOther(revRes.first,
				revRes.first.base() - begin, equalFunc).base();
			return { resBegin, revRes.first.base() };
		}
		res = pvFindNext(res.first, begin + count - res.first, item, itemHash,
			hashFuncIter, equalFunc);
		if (!res.second)
			return { res.first, res.first };
		return { res.first, pvFindOther(res.first, begin + count - res.first, equalFunc) };
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc>
	static std::pair<Iterator, bool> pvFindNext(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashFuncResult itemHash,
		const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		Iterator iter = begin;
		while (true)
		{
			iter = pvFindOther(iter, begin + count - iter, equalFunc);
			if (iter == begin + count || hashFuncIter(iter) != itemHash)
				break;
			if (equalFunc(*iter, item))
				return { iter, true };
		}
		return { iter, false };
	}

	template<typename Iterator, typename EqualFunc>
	static Iterator pvFindOther(Iterator begin, size_t count, const EqualFunc& equalFunc)
	{
		MOMO_ASSERT(count > 0);
		auto pred = [begin, &equalFunc] (Iterator iter)
			{ return equalFunc(*begin, *iter) ? -1 : 1; };
		return pvExponentialSearch(begin + 1, count - 1, pred).first;
	}

	template<typename Iterator, typename HashFuncIter>
	static std::pair<Iterator, bool> pvFindHash(Iterator begin, size_t count,
		HashFuncResult itemHash, const HashFuncIter& hashFuncIter)
	{
		auto pred = [itemHash, &hashFuncIter] (Iterator iter)
			{ return pvCompare(hashFuncIter(iter), itemHash); };
		size_t leftIndex = 0;
		size_t rightIndex = count;
		size_t middleIndex = pvMultShift(itemHash, count);
		size_t step = (count < 1 << 6) ? 0 : (count < 1 << 12) ? 1 : (count < 1 << 22) ? 2 : 3;
		while (true)
		{
			HashFuncResult middleHash = hashFuncIter(begin + middleIndex);
			if (middleHash < itemHash)
			{
				leftIndex = middleIndex + 1;
				if (step == 0)
					return pvExponentialSearch(begin + leftIndex, rightIndex - leftIndex, pred);
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
					auto revPred = [itemHash, &hashFuncIter] (ReverseIterator iter)
						{ return -pvCompare(hashFuncIter(iter), itemHash); };
					auto res = pvExponentialSearch(ReverseIterator(begin + rightIndex),
						rightIndex - leftIndex, revPred);
					return { res.first.base() - (res.second ? 1 : 0), res.second };
				}
				size_t diff = pvMultShift(middleHash - itemHash, count);
				if (leftIndex + diff > middleIndex)
					break;
				middleIndex -= diff;
			}
			else
			{
				return { begin + middleIndex, true };
			}
			--step;
		}
		return pvBinarySearch(begin + leftIndex, rightIndex - leftIndex, pred);
	}

	template<typename Iterator, typename Predicate>
	static std::pair<Iterator, bool> pvExponentialSearch(Iterator begin, size_t count, Predicate pred)
	{
		size_t leftIndex = 0;
		for (size_t i = 0; i < count; i = i * 2 + 2)
		{
			int cmp = pred(begin + i);
			if (cmp == 1)
				return pvBinarySearch(begin + leftIndex, i - leftIndex, pred);
			else if (cmp == 0)
				return { begin + i, true };
			leftIndex = i + 1;
		}
		return pvBinarySearch(begin + leftIndex, count - leftIndex, pred);
	}

	template<typename Iterator, typename Predicate>
	static std::pair<Iterator, bool> pvBinarySearch(Iterator begin, size_t count, Predicate pred)
	{
		size_t leftIndex = 0;
		size_t rightIndex = count;
		while (leftIndex < rightIndex)
		{
			size_t middleIndex = (leftIndex + rightIndex) / 2;
			int cmp = pred(begin + middleIndex);
			if (cmp == -1)
				leftIndex = middleIndex + 1;
			else if (cmp == 1)
				rightIndex = middleIndex;
			else
				return { begin + middleIndex, true };
		}
		return { begin + leftIndex, false };
	}

	static size_t pvGetRadix(HashFuncResult value, size_t shift) noexcept
	{
		return (size_t)(value >> shift) & (((size_t)1 << radixSize) - 1);
	}

	static int pvCompare(HashFuncResult value1, HashFuncResult value2) noexcept
	{
		return (value1 < value2) ? -1 : (int)(value1 != value2);
	}

	static size_t pvMultShift(HashFuncResult value1, size_t value2) noexcept
	{
		MOMO_STATIC_ASSERT(sizeof(HashFuncResult) >= sizeof(size_t));
		static const size_t halfSize = 4 * sizeof(HashFuncResult);
		static const HashFuncResult halfMask = ((HashFuncResult)1 << halfSize) - 1;
		HashFuncResult res = (value1 >> halfSize) * (value2 >> halfSize)
			+ (((value1 >> halfSize) * (value2 & halfMask)) >> halfSize)
			+ (((value2 >> halfSize) * (value1 & halfMask)) >> halfSize);
		return (size_t)res;
	}
};

} // namespace experimental

} // namespace momo
