/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/HashSorter.h

  namespace momo:
    class HashSorter

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_HASH_SORTER
#define MOMO_INCLUDE_GUARD_HASH_SORTER

#include "RadixSorter.h"
#include "HashTraits.h"

namespace momo
{

class HashSorter
{
public:
	typedef size_t HashCode;

	template<typename Iterator>
	struct Swapper
	{
		void operator()(Iterator iter1, Iterator iter2) const
		{
			std::iter_swap(iter1, iter2);
		}
	};

	template<typename Iterator>
	struct FindResult
	{
		Iterator iterator;
		bool found;
	};

	template<typename TIterator>
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
	class IterHashFunc
	{
	public:
		explicit IterHashFunc(const HashFunc& hashFunc) noexcept
			: mHashFunc(hashFunc)
		{
		}

		template<typename Iterator>
		HashCode operator()(Iterator iter) const
		{
			return mHashFunc(*iter);
		}

	private:
		const HashFunc& mHashFunc;
	};

	template<typename Iterator, typename HashIterator>
	class IterPrehashFunc
	{
	public:
		explicit IterPrehashFunc(Iterator begin, HashIterator hashBegin) noexcept
			: mBegin(begin),
			mHashBegin(hashBegin)
		{
		}

		HashCode operator()(Iterator iter) const
		{
			return mHashBegin[iter - mBegin];
		}

		HashCode operator()(std::reverse_iterator<Iterator> iter) const
		{
			return mHashBegin[iter.base() - 1 - mBegin];
		}

	private:
		Iterator mBegin;
		HashIterator mHashBegin;
	};

	typedef internal::UIntMath<> SMath;

public:
	template<typename Iterator,
		typename HashFunc = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>,
		typename IterSwapper = Swapper<Iterator>>
	static void Sort(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc(), const IterSwapper& iterSwapper = IterSwapper())
	{
		pvSort(begin, count, IterHashFunc<HashFunc>(hashFunc), equalFunc, iterSwapper);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>,
		typename IterSwapper = Swapper<Iterator>>
	static void SortPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const EqualFunc& equalFunc = EqualFunc(), const IterSwapper& iterSwapper = IterSwapper())
	{
		auto iterHashSwapper = [begin, hashBegin, &iterSwapper] (Iterator iter1, Iterator iter2)
		{
			iterSwapper(iter1, iter2);
			std::swap(hashBegin[iter1 - begin], hashBegin[iter2 - begin]);
		};
		pvSort(begin, count, IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin),
			equalFunc, iterHashSwapper);
	}

	template<typename Iterator,
		typename HashFunc = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static bool IsSorted(Iterator begin, size_t count, const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count, IterHashFunc<HashFunc>(hashFunc), equalFunc);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static bool IsSortedPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const EqualFunc& equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count,
			IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

	template<typename Iterator,
		typename HashFunc = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static FindResult<Iterator> Find(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvFind(begin, count, item, hashFunc(item),
			IterHashFunc<HashFunc>(hashFunc), equalFunc);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static FindResult<Iterator> FindPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHash,
		const EqualFunc& equalFunc = EqualFunc())
	{
		return pvFind(begin, count, item, itemHash,
			IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

	template<typename Iterator,
		typename HashFunc = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static Bounds<Iterator> GetBounds(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const HashFunc& hashFunc = HashFunc(), const EqualFunc& equalFunc = EqualFunc())
	{
		return pvGetBounds(begin, count, item, hashFunc(item),
			IterHashFunc<HashFunc>(hashFunc), equalFunc);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualFunc = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static Bounds<Iterator> GetBoundsPrehashed(Iterator begin, size_t count,
		HashIterator hashBegin, const typename std::iterator_traits<Iterator>::value_type& item,
		HashCode itemHash, const EqualFunc& equalFunc = EqualFunc())
	{
		return pvGetBounds(begin, count, item, itemHash,
			IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin), equalFunc);
	}

private:
	template<typename Iterator, typename IterHashFunc, typename EqualFunc, typename IterSwapper>
	static void pvSort(Iterator begin, size_t count, const IterHashFunc& iterHashFunc,
		const EqualFunc& equalFunc, const IterSwapper& iterSwapper)
	{
		auto groupFunc = [&equalFunc, &iterSwapper] (Iterator begin, size_t count)
		{
			if (count > 2)
				pvGroup(begin, count, equalFunc, iterSwapper);
		};
		internal::RadixSorter<>::Sort(begin, count, iterHashFunc, iterSwapper, groupFunc);
	}

	template<typename Iterator, typename EqualFunc, typename IterSwapper>
	static void pvGroup(Iterator begin, size_t count, const EqualFunc& equalFunc,
		const IterSwapper& iterSwapper)
	{
		for (size_t i = 1; i < count; ++i)
		{
			if (equalFunc(*SMath::Next(begin, i - 1), *SMath::Next(begin, i)))
				continue;
			for (size_t j = i + 1; j < count; ++j)
			{
				if (equalFunc(*SMath::Next(begin, i - 1), *SMath::Next(begin, j)))
				{
					iterSwapper(SMath::Next(begin, i), SMath::Next(begin, j));
					++i;
				}
			}
		}
	}

	template<typename Iterator, typename IterHashFunc, typename EqualFunc>
	static bool pvIsSorted(Iterator begin, size_t count, const IterHashFunc& iterHashFunc,
		const EqualFunc& equalFunc)
	{
		size_t prevIndex = 0;
		HashCode prevHash = iterHashFunc(begin);
		for (size_t i = 1; i < count; ++i)
		{
			HashCode hash = iterHashFunc(SMath::Next(begin, i));
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

	template<typename Iterator, typename IterHashFunc, typename EqualFunc>
	static FindResult<Iterator> pvFind(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHash,
		const IterHashFunc& iterHashFunc, const EqualFunc& equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, iterHashFunc);
		if (!res.found)
			return res;
		if (equalFunc(*res.iterator, item))
			return res;
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), item, itemHash, iterHashFunc, equalFunc);
		if (revRes.found)
			return { revRes.iterator.base() - 1, true };
		return pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator),
			item, itemHash, iterHashFunc, equalFunc);
	}

	template<typename Iterator, typename IterHashFunc, typename EqualFunc>
	static Bounds<Iterator> pvGetBounds(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHash,
		const IterHashFunc& iterHashFunc, const EqualFunc& equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, iterHashFunc);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		if (equalFunc(*res.iterator, item))
		{
			Iterator resBegin = pvFindOther(std::reverse_iterator<Iterator>(res.iterator + 1),
				SMath::Dist(begin, res.iterator + 1), equalFunc).base();
			return Bounds<Iterator>(resBegin,
				pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalFunc));
		}
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), item, itemHash, iterHashFunc, equalFunc);
		if (revRes.found)
		{
			Iterator resBegin = pvFindOther(revRes.iterator,
				SMath::Dist(begin, revRes.iterator.base()), equalFunc).base();
			return Bounds<Iterator>(resBegin, revRes.iterator.base());
		}
		res = pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator), item, itemHash,
			iterHashFunc, equalFunc);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		return Bounds<Iterator>(res.iterator,
			pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalFunc));
	}

	template<typename Iterator, typename IterHashFunc, typename EqualFunc>
	static FindResult<Iterator> pvFindNext(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHash,
		const IterHashFunc& iterHashFunc, const EqualFunc& equalFunc)
	{
		Iterator iter = begin;
		while (true)
		{
			iter = pvFindOther(iter, count - SMath::Dist(begin, iter), equalFunc);
			if (iter == SMath::Next(begin, count) || iterHashFunc(iter) != itemHash)
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
		auto iterComparer = [begin, &equalFunc] (Iterator iter)
			{ return equalFunc(*begin, *iter) ? -1 : 1; };
		return pvExponentialSearch(begin + 1, count - 1, iterComparer).iterator;
	}

	template<typename Iterator, typename IterHashFunc>
	static FindResult<Iterator> pvFindHash(Iterator begin, size_t count,
		HashCode itemHash, const IterHashFunc& iterHashFunc)
	{
		auto iterComparer = [itemHash, &iterHashFunc] (Iterator iter)
			{ return pvCompare(iterHashFunc(iter), itemHash); };
		size_t leftIndex = 0;
		size_t rightIndex = count;
		size_t middleIndex = pvMultShift(itemHash, count);
		size_t step = pvGetStepCount(count);
		while (true)
		{
			HashCode middleHash = iterHashFunc(SMath::Next(begin, middleIndex));
			if (middleHash < itemHash)
			{
				leftIndex = middleIndex + 1;
				if (step == 0)
				{
					return pvExponentialSearch(SMath::Next(begin, leftIndex),
						rightIndex - leftIndex, iterComparer);
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
					auto revCompareFunc = [itemHash, &iterHashFunc] (ReverseIterator iter)
						{ return -pvCompare(iterHashFunc(iter), itemHash); };
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
		return pvBinarySearch(SMath::Next(begin, leftIndex), rightIndex - leftIndex, iterComparer);
	}

	template<typename Iterator, typename IterComparer>
	static FindResult<Iterator> pvExponentialSearch(Iterator begin, size_t count,
		const IterComparer& iterComparer)
	{
		size_t leftIndex = 0;
		for (size_t i = 0; i < count; i = i * 2 + 2)
		{
			int cmp = iterComparer(SMath::Next(begin, i));
			if (cmp > 0)
				return pvBinarySearch(SMath::Next(begin, leftIndex), i - leftIndex, iterComparer);
			else if (cmp == 0)
				return { SMath::Next(begin, i), true };
			leftIndex = i + 1;
		}
		return pvBinarySearch(SMath::Next(begin, leftIndex), count - leftIndex, iterComparer);
	}

	template<typename Iterator, typename IterComparer>
	static FindResult<Iterator> pvBinarySearch(Iterator begin, size_t count,
		const IterComparer& iterComparer)
	{
		size_t leftIndex = 0;
		size_t rightIndex = count;
		while (leftIndex < rightIndex)
		{
			size_t middleIndex = (leftIndex + rightIndex) / 2;
			int cmp = iterComparer(SMath::Next(begin, middleIndex));
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

	static int pvCompare(HashCode value1, HashCode value2) noexcept
	{
		return (value1 < value2) ? -1 : int{value1 != value2};
	}

	static size_t pvMultShift(HashCode value1, size_t value2) noexcept
	{
		MOMO_STATIC_ASSERT(sizeof(HashCode) >= sizeof(size_t));
		static const size_t halfSize = 4 * sizeof(HashCode);
		static const HashCode halfMask = (HashCode{1} << halfSize) - 1;
		HashCode res = (value1 >> halfSize) * (value2 >> halfSize)
			+ (((value1 >> halfSize) * (value2 & halfMask)) >> halfSize)
			+ (((value2 >> halfSize) * (value1 & halfMask)) >> halfSize);
		return static_cast<size_t>(res);
	}
};

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_HASH_SORTER
