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

	template<typename HashFunc>
	class HashFuncIter
	{
	public:
		explicit HashFuncIter(const HashFunc& hashFunc) MOMO_NOEXCEPT
			: mHashFunc(hashFunc)
		{
		}

		template<typename Iterator>
		size_t operator()(Iterator iter) const
		{
			return mHashFunc(*iter);
		}

	private:
		const HashFunc& mHashFunc;
	};

	template<typename Iterator, typename HashArray>
	class HashFuncIterA
	{
	public:
		HashFuncIterA(Iterator begin, const HashArray& hashArray) MOMO_NOEXCEPT
			: mBegin(begin),
			mHashArray(hashArray)
		{
		}

		size_t operator()(Iterator iter) const
		{
			return mHashArray[iter - mBegin];
		}

		size_t operator()(std::reverse_iterator<Iterator> iter) const
		{
			return mHashArray[iter.base() - 1 - mBegin];
		}

	private:
		Iterator mBegin;
		const HashArray& mHashArray;
	};

public:
	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static HashArray SortRA(Iterator begin, size_t count,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc(),
		typename HashArray::MemManager&& memManager = typename HashArray::MemManager())
	{
		HashArray hashArray(count, std::move(memManager));
		SortA(hashArray, begin, count, hashFunc, equalFunc);
		return hashArray;
	}

	template<typename HashArray, typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static void SortA(HashArray& hashArray, Iterator begin, size_t count,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		for (size_t i = 0; i < count; ++i)
			hashArray[i] = hashFunc(begin[i]);
		auto swapFunc = [begin, &hashArray] (Iterator iter1, Iterator iter2)
		{
			std::iter_swap(iter1, iter2);
			std::swap(hashArray[iter1 - begin], hashArray[iter2 - begin]);
		};
		_Sort(begin, count, HashFuncIterA<Iterator, HashArray>(begin, hashArray), equalFunc,
			swapFunc, 8 * sizeof(size_t) - radixSize);
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static void Sort(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
	{
		auto swapFunc = [] (Iterator iter1, Iterator iter2) { std::iter_swap(iter1, iter2); };
		_Sort(begin, count, HashFuncIter<HashFunc>(hashFunc), equalFunc,
			swapFunc, 8 * sizeof(size_t) - radixSize);
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static bool IsSorted(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
	{
		size_t prevIndex = 0;
		size_t prevHash = hashFunc(*begin);
		for (size_t i = 1; i < count; ++i)
		{
			size_t hash = hashFunc(begin[i]);
			if (hash < prevHash)
				return false;
			if (hash != prevHash)
			{
				if (!_IsGrouped(begin + prevIndex, i - prevIndex, equalFunc))
					return false;
				prevIndex = i;
			}
		}
		return _IsGrouped(begin + prevIndex, count - prevIndex, equalFunc);
	}

	template<typename HashArray, typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, bool> FindA(const HashArray& hashArray, Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return _Find(begin, count, item, hashFunc(item),
			HashFuncIterA<Iterator, HashArray>(begin, hashArray), equalFunc);
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, bool> Find(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return _Find(begin, count, item, hashFunc(item),
			HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

	template<typename HashArray, typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, Iterator> EqualRangeA(const HashArray& hashArray, Iterator begin,
		size_t count, const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return _EqualRange(begin, count, item, hashFunc(item),
			HashFuncIterA<Iterator, HashArray>(begin, hashArray), equalFunc);
	}

	template<typename Iterator,
		typename HashFunc = std::hash<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static std::pair<Iterator, Iterator> EqualRange(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return _EqualRange(begin, count, item, hashFunc(item),
			HashFuncIter<HashFunc>(hashFunc), equalFunc);
	}

private:
	template<typename Iterator, typename HashFuncIter, typename EqualFunc, typename SwapFunc>
	static void _Sort(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
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
					_SelectionSort(begin, count, hashFuncIter, equalFunc, swapFunc);
				else
					_RadixSort(begin, count, hashFuncIter, equalFunc, swapFunc, shift);
			}
		}
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc, typename SwapFunc>
	static void _SelectionSort(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
		const EqualFunc& equalFunc, SwapFunc swapFunc)
	{
		MOMO_ASSERT(count > 0);
		size_t hashes[1 << (radixSize / 2 + 1)];	//?
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
				_GroupIf(begin + prevIndex, i - prevIndex, equalFunc, swapFunc);
				prevIndex = i;
			}
		}
		_GroupIf(begin + prevIndex, count - prevIndex, equalFunc, swapFunc);
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc, typename SwapFunc>
	static void _RadixSort(Iterator begin, size_t count, const HashFuncIter& hashFuncIter,
		const EqualFunc& equalFunc, SwapFunc swapFunc, size_t shift)
	{
		static const size_t radixCount = 1 << radixSize;
		size_t endIndices[radixCount] = {};
		size_t hash0 = hashFuncIter(begin);
		++endIndices[_GetRadix(hash0, shift)];
		bool singleHash = true;
		for (size_t i = 1; i < count; ++i)
		{
			size_t hash = hashFuncIter(begin + i);
			++endIndices[_GetRadix(hash, shift)];
			singleHash &= (hash == hash0);
		}
		if (singleHash)
			return _Group(begin, count, equalFunc, swapFunc);
		for (size_t r = 1; r < radixCount; ++r)
			endIndices[r] += endIndices[r - 1];
		_RadixSort(begin, hashFuncIter, swapFunc, shift, endIndices);
		size_t nextShift = (shift > radixSize) ? shift - radixSize : 0;
		size_t beginIndex = 0;
		for (size_t e : endIndices)
		{
			if (shift > 0)
				_Sort(begin + beginIndex, e - beginIndex, hashFuncIter, equalFunc, swapFunc, nextShift);
			else
				_GroupIf(begin + beginIndex, e - beginIndex, equalFunc, swapFunc);
			beginIndex = e;
		}
	}

	template<typename Iterator, typename HashFuncIter, typename SwapFunc>
	static void _RadixSort(Iterator begin, const HashFuncIter& hashFuncIter, SwapFunc swapFunc,
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
				size_t radix = _GetRadix(hashFuncIter(begin + beginIndex), shift);
				if (radix != r)
					swapFunc(begin + beginIndex, begin + beginIndices[radix]);
				++beginIndices[radix];
			}
		}
	}

	template<typename Iterator, typename EqualFunc, typename SwapFunc>
	static void _GroupIf(Iterator begin, size_t count, const EqualFunc& equalFunc,
		SwapFunc swapFunc)
	{
		if (count > 2)
			_Group(begin, count, equalFunc, swapFunc);
	}

	template<typename Iterator, typename EqualFunc, typename SwapFunc>
	static void _Group(Iterator begin, size_t count, const EqualFunc& equalFunc, SwapFunc swapFunc)
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

	template<typename Iterator, typename EqualFunc>
	static bool _IsGrouped(Iterator begin, size_t count, const EqualFunc& equalFunc)
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
	static std::pair<Iterator, bool> _Find(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, size_t itemHash,
		const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		auto res = _FindHash(begin, count, itemHash, hashFuncIter);
		if (!res.second)
			return res;
		if (equalFunc(*res.first, item))
			return res;
		auto revRes = _FindNext(std::reverse_iterator<Iterator>(res.first + 1),
			res.first + 1 - begin, item, itemHash, hashFuncIter, equalFunc);
		if (revRes.second)
			return { revRes.first.base() - 1, true };
		return _FindNext(res.first, begin + count - res.first,
			item, itemHash, hashFuncIter, equalFunc);
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc>
	static std::pair<Iterator, Iterator> _EqualRange(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, size_t itemHash,
		const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		auto res = _FindHash(begin, count, itemHash, hashFuncIter);
		if (!res.second)
			return { res.first, res.first };
		if (equalFunc(*res.first, item))
		{
			Iterator resBegin = _FindOther(std::reverse_iterator<Iterator>(res.first + 1),
				res.first + 1 - begin, equalFunc).base();
			return { resBegin, _FindOther(res.first, begin + count - res.first, equalFunc) };
		}
		auto revRes = _FindNext(std::reverse_iterator<Iterator>(res.first + 1),
			res.first + 1 - begin, item, itemHash, hashFuncIter, equalFunc);
		if (revRes.second)
		{
			Iterator resBegin = _FindOther(revRes.first,
				revRes.first.base() - begin, equalFunc).base();
			return { resBegin, revRes.first.base() };
		}
		res = _FindNext(res.first, begin + count - res.first, item, itemHash,
			hashFuncIter, equalFunc);
		if (!res.second)
			return { res.first, res.first };
		return { res.first, _FindOther(res.first, begin + count - res.first, equalFunc) };
	}

	template<typename Iterator, typename HashFuncIter, typename EqualFunc>
	static std::pair<Iterator, bool> _FindNext(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, size_t itemHash,
		const HashFuncIter& hashFuncIter, const EqualFunc& equalFunc)
	{
		Iterator iter = begin;
		while (true)
		{
			iter = _FindOther(iter, begin + count - iter, equalFunc);
			if (iter == begin + count || hashFuncIter(iter) != itemHash)
				break;
			if (equalFunc(*iter, item))
				return { iter, true };
		}
		return { iter, false };
	}

	template<typename Iterator, typename EqualFunc>
	static Iterator _FindOther(Iterator begin, size_t count, const EqualFunc& equalFunc)
	{
		MOMO_ASSERT(count > 0);
		auto pred = [begin, &equalFunc] (Iterator iter)
			{ return equalFunc(*begin, *iter) ? -1 : 1; };
		return _ExponentialSearch(begin + 1, count - 1, pred).first;
	}

	template<typename Iterator, typename HashFuncIter>
	static std::pair<Iterator, bool> _FindHash(Iterator begin, size_t count, size_t itemHash,
		const HashFuncIter& hashFuncIter)
	{
		auto pred = [itemHash, &hashFuncIter] (Iterator iter)
			{ return _Compare(hashFuncIter(iter), itemHash); };
		size_t leftIndex = 0;
		size_t rightIndex = count;
		size_t middleIndex = _MultShift(itemHash, count);
		size_t step = (count < 1 << 6) ? 0 : (count < 1 << 12) ? 1 : (count < 1 << 22) ? 2 : 3;
		while (true)
		{
			size_t middleHash = hashFuncIter(begin + middleIndex);
			if (middleHash < itemHash)
			{
				leftIndex = middleIndex + 1;
				if (step == 0)
					return _ExponentialSearch(begin + leftIndex, rightIndex - leftIndex, pred);
				middleIndex += _MultShift(itemHash - middleHash, count);
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
						{ return -_Compare(hashFuncIter(iter), itemHash); };
					auto res = _ExponentialSearch(ReverseIterator(begin + rightIndex),
						rightIndex - leftIndex, revPred);
					return { res.first.base() - (res.second ? 1 : 0), res.second };
				}
				size_t diff = _MultShift(middleHash - itemHash, count);
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
		return _BinarySearch(begin + leftIndex, rightIndex - leftIndex, pred);
	}

	template<typename Iterator, typename Predicate>
	static std::pair<Iterator, bool> _ExponentialSearch(Iterator begin, size_t count, Predicate pred)
	{
		size_t leftIndex = 0;
		for (size_t i = 0; i < count; i = i * 2 + 2)
		{
			int cmp = pred(begin + i);
			if (cmp == 1)
				return _BinarySearch(begin + leftIndex, i - leftIndex, pred);
			else if (cmp == 0)
				return { begin + i, true };
			leftIndex = i + 1;
		}
		return _BinarySearch(begin + leftIndex, count - leftIndex, pred);
	}

	template<typename Iterator, typename Predicate>
	static std::pair<Iterator, bool> _BinarySearch(Iterator begin, size_t count, Predicate pred)
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

	static size_t _GetRadix(size_t value, size_t shift) MOMO_NOEXCEPT
	{
		return (value >> shift) & ((1 << radixSize) - 1);
	}

	static int _Compare(size_t value1, size_t value2) MOMO_NOEXCEPT
	{
		return (value1 < value2) ? -1 : (int)(value1 != value2);
	}

	static size_t _MultShift(size_t value1, size_t value2) MOMO_NOEXCEPT
	{
		static const size_t halfSize = 4 * sizeof(size_t);
		return (value1 >> halfSize) * (value2 >> halfSize)
			+ (((value1 >> halfSize) * (value2 & ((1 << halfSize) - 1))) >> halfSize)
			+ (((value2 >> halfSize) * (value1 & ((1 << halfSize) - 1))) >> halfSize);
	}
};

} // namespace experimental

} // namespace momo
