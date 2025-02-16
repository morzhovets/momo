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
	template<typename Hasher>
	class IterHasher
	{
	public:
		explicit IterHasher(const Hasher& hasher) noexcept
			: mHasher(hasher)
		{
		}

		template<typename Iterator>
		HashCode operator()(Iterator iter) const
		{
			return mHasher(*iter);
		}

	private:
		const Hasher& mHasher;
	};

	template<typename Iterator, typename HashIterator>
	class IterPrehasher
	{
	public:
		explicit IterPrehasher(Iterator begin, HashIterator hashBegin) noexcept
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
		typename Hasher = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>,
		typename IterSwapper = Swapper<Iterator>>
	static void Sort(Iterator begin, size_t count, const Hasher& hasher = Hasher(),
		const EqualComparer& equalComp = EqualComparer(), const IterSwapper& iterSwapper = IterSwapper())
	{
		pvSort(begin, count, IterHasher<Hasher>(hasher), equalComp, iterSwapper);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>,
		typename IterSwapper = Swapper<Iterator>>
	static void SortPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const EqualComparer& equalComp = EqualComparer(), const IterSwapper& iterSwapper = IterSwapper())
	{
		auto iterHashSwapper = [begin, hashBegin, &iterSwapper] (Iterator iter1, Iterator iter2)
		{
			iterSwapper(iter1, iter2);
			std::swap(hashBegin[iter1 - begin], hashBegin[iter2 - begin]);
		};
		pvSort(begin, count, IterPrehasher<Iterator, HashIterator>(begin, hashBegin),
			equalComp, iterHashSwapper);
	}

	template<typename Iterator,
		typename Hasher = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static bool IsSorted(Iterator begin, size_t count, const Hasher& hasher = Hasher(),
		const EqualComparer& equalComp = EqualComparer())
	{
		return pvIsSorted(begin, count, IterHasher<Hasher>(hasher), equalComp);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static bool IsSortedPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const EqualComparer& equalComp = EqualComparer())
	{
		return pvIsSorted(begin, count,
			IterPrehasher<Iterator, HashIterator>(begin, hashBegin), equalComp);
	}

	template<typename Iterator,
		typename Hasher = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static FindResult<Iterator> Find(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const Hasher& hasher = Hasher(), const EqualComparer& equalComp = EqualComparer())
	{
		return pvFind(begin, count, item, hasher(item),
			IterHasher<Hasher>(hasher), equalComp);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static FindResult<Iterator> FindPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHashCode,
		const EqualComparer& equalComp = EqualComparer())
	{
		return pvFind(begin, count, item, itemHashCode,
			IterPrehasher<Iterator, HashIterator>(begin, hashBegin), equalComp);
	}

	template<typename Iterator,
		typename Hasher = HashCoder<typename std::iterator_traits<Iterator>::value_type>,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static Bounds<Iterator> GetBounds(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item,
		const Hasher& hasher = Hasher(), const EqualComparer& equalComp = EqualComparer())
	{
		return pvGetBounds(begin, count, item, hasher(item),
			IterHasher<Hasher>(hasher), equalComp);
	}

	template<typename Iterator, typename HashIterator,
		typename EqualComparer = std::equal_to<typename std::iterator_traits<Iterator>::value_type>>
	static Bounds<Iterator> GetBoundsPrehashed(Iterator begin, size_t count,
		HashIterator hashBegin, const typename std::iterator_traits<Iterator>::value_type& item,
		HashCode itemHashCode, const EqualComparer& equalComp = EqualComparer())
	{
		return pvGetBounds(begin, count, item, itemHashCode,
			IterPrehasher<Iterator, HashIterator>(begin, hashBegin), equalComp);
	}

private:
	template<typename Iterator, typename IterHasher, typename EqualComparer, typename IterSwapper>
	static void pvSort(Iterator begin, size_t count, const IterHasher& iterHasher,
		const EqualComparer& equalComp, const IterSwapper& iterSwapper)
	{
		auto itemsGrouper = [&equalComp, &iterSwapper] (Iterator begin, size_t count)
		{
			if (count > 2)
				pvGroup(begin, count, equalComp, iterSwapper);
		};
		internal::RadixSorter<>::Sort(begin, count, iterHasher, iterSwapper, itemsGrouper);
	}

	template<typename Iterator, typename EqualComparer, typename IterSwapper>
	static void pvGroup(Iterator begin, size_t count, const EqualComparer& equalComp,
		const IterSwapper& iterSwapper)
	{
		for (size_t i = 1; i < count; ++i)
		{
			if (equalComp(*SMath::Next(begin, i - 1), *SMath::Next(begin, i)))
				continue;
			for (size_t j = i + 1; j < count; ++j)
			{
				if (equalComp(*SMath::Next(begin, i - 1), *SMath::Next(begin, j)))
				{
					iterSwapper(SMath::Next(begin, i), SMath::Next(begin, j));
					++i;
				}
			}
		}
	}

	template<typename Iterator, typename IterHasher, typename EqualComparer>
	static bool pvIsSorted(Iterator begin, size_t count, const IterHasher& iterHasher,
		const EqualComparer& equalComp)
	{
		size_t prevIndex = 0;
		HashCode prevHashCode = iterHasher(begin);
		for (size_t i = 1; i < count; ++i)
		{
			HashCode hashCode = iterHasher(SMath::Next(begin, i));
			if (hashCode < prevHashCode)
				return false;
			if (hashCode != prevHashCode)
			{
				if (!pvIsGrouped(SMath::Next(begin, prevIndex), i - prevIndex, equalComp))
					return false;
				prevIndex = i;
				prevHashCode = hashCode;
			}
		}
		return pvIsGrouped(SMath::Next(begin, prevIndex), count - prevIndex, equalComp);
	}

	template<typename Iterator, typename EqualComparer>
	static bool pvIsGrouped(Iterator begin, size_t count, const EqualComparer& equalComp)
	{
		for (size_t i = 1; i < count; ++i)
		{
			if (equalComp(*SMath::Next(begin, i - 1), *SMath::Next(begin, i)))
				continue;
			for (size_t j = i + 1; j < count; ++j)
			{
				if (equalComp(*SMath::Next(begin, i - 1), *SMath::Next(begin, j)))
					return false;
			}
		}
		return true;
	}

	template<typename Iterator, typename IterHasher, typename EqualComparer>
	static FindResult<Iterator> pvFind(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHashCode,
		const IterHasher& iterHasher, const EqualComparer& equalComp)
	{
		auto res = pvFindHashCode(begin, count, itemHashCode, iterHasher);
		if (!res.found)
			return res;
		if (equalComp(*res.iterator, item))
			return res;
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), item, itemHashCode, iterHasher, equalComp);
		if (revRes.found)
			return { revRes.iterator.base() - 1, true };
		return pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator),
			item, itemHashCode, iterHasher, equalComp);
	}

	template<typename Iterator, typename IterHasher, typename EqualComparer>
	static Bounds<Iterator> pvGetBounds(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHashCode,
		const IterHasher& iterHasher, const EqualComparer& equalComp)
	{
		auto res = pvFindHashCode(begin, count, itemHashCode, iterHasher);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		if (equalComp(*res.iterator, item))
		{
			Iterator resBegin = pvFindOther(std::reverse_iterator<Iterator>(res.iterator + 1),
				SMath::Dist(begin, res.iterator + 1), equalComp).base();
			return Bounds<Iterator>(resBegin,
				pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalComp));
		}
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), item, itemHashCode, iterHasher, equalComp);
		if (revRes.found)
		{
			Iterator resBegin = pvFindOther(revRes.iterator,
				SMath::Dist(begin, revRes.iterator.base()), equalComp).base();
			return Bounds<Iterator>(resBegin, revRes.iterator.base());
		}
		res = pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator), item, itemHashCode,
			iterHasher, equalComp);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		return Bounds<Iterator>(res.iterator,
			pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalComp));
	}

	template<typename Iterator, typename IterHasher, typename EqualComparer>
	static FindResult<Iterator> pvFindNext(Iterator begin, size_t count,
		const typename std::iterator_traits<Iterator>::value_type& item, HashCode itemHashCode,
		const IterHasher& iterHasher, const EqualComparer& equalComp)
	{
		Iterator iter = begin;
		while (true)
		{
			iter = pvFindOther(iter, count - SMath::Dist(begin, iter), equalComp);
			if (iter == SMath::Next(begin, count) || iterHasher(iter) != itemHashCode)
				break;
			if (equalComp(*iter, item))
				return { iter, true };
		}
		return { iter, false };
	}

	template<typename Iterator, typename EqualComparer>
	static Iterator pvFindOther(Iterator begin, size_t count, const EqualComparer& equalComp)
	{
		MOMO_ASSERT(count > 0);
		auto iterThreeComp = [begin, &equalComp] (Iterator iter)
			{ return equalComp(*begin, *iter) ? -1 : 1; };
		return pvExponentialSearch(begin + 1, count - 1, iterThreeComp).iterator;
	}

	template<typename Iterator, typename IterHasher>
	static FindResult<Iterator> pvFindHashCode(Iterator begin, size_t count,
		HashCode itemHashCode, const IterHasher& iterHasher)
	{
		auto iterThreeComp = [itemHashCode, &iterHasher] (Iterator iter)
			{ return pvCompare(iterHasher(iter), itemHashCode); };
		size_t leftIndex = 0;
		size_t rightIndex = count;
		size_t middleIndex = pvMultShift(itemHashCode, count);
		size_t step = pvGetStepCount(count);
		while (true)
		{
			HashCode middleHashCode = iterHasher(SMath::Next(begin, middleIndex));
			if (middleHashCode < itemHashCode)
			{
				leftIndex = middleIndex + 1;
				if (step == 0)
				{
					return pvExponentialSearch(SMath::Next(begin, leftIndex),
						rightIndex - leftIndex, iterThreeComp);
				}
				middleIndex += pvMultShift(itemHashCode - middleHashCode, count);
				if (middleIndex >= rightIndex)
					break;
			}
			else if (middleHashCode > itemHashCode)
			{
				rightIndex = middleIndex;
				if (step == 0)
				{
					typedef std::reverse_iterator<Iterator> ReverseIterator;
					auto revIterThreeComp = [itemHashCode, &iterHasher] (ReverseIterator iter)
						{ return -pvCompare(iterHasher(iter), itemHashCode); };
					auto res = pvExponentialSearch(ReverseIterator(SMath::Next(begin, rightIndex)),
						rightIndex - leftIndex, revIterThreeComp);
					return { res.iterator.base() - (res.found ? 1 : 0), res.found };
				}
				size_t diff = pvMultShift(middleHashCode - itemHashCode, count);
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
		return pvBinarySearch(SMath::Next(begin, leftIndex), rightIndex - leftIndex, iterThreeComp);
	}

	template<typename Iterator, typename IterThreeComparer>
	static FindResult<Iterator> pvExponentialSearch(Iterator begin, size_t count,
		const IterThreeComparer& iterThreeComp)
	{
		size_t leftIndex = 0;
		for (size_t i = 0; i < count; i = i * 2 + 2)
		{
			int cmp = iterThreeComp(SMath::Next(begin, i));
			if (cmp > 0)
				return pvBinarySearch(SMath::Next(begin, leftIndex), i - leftIndex, iterThreeComp);
			else if (cmp == 0)
				return { SMath::Next(begin, i), true };
			leftIndex = i + 1;
		}
		return pvBinarySearch(SMath::Next(begin, leftIndex), count - leftIndex, iterThreeComp);
	}

	template<typename Iterator, typename IterThreeComparer>
	static FindResult<Iterator> pvBinarySearch(Iterator begin, size_t count,
		const IterThreeComparer& iterThreeComp)
	{
		size_t leftIndex = 0;
		size_t rightIndex = count;
		while (leftIndex < rightIndex)
		{
			size_t middleIndex = (leftIndex + rightIndex) / 2;
			int cmp = iterThreeComp(SMath::Next(begin, middleIndex));
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
