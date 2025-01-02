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
	typedef size_t HashCode;

	template<internal::conceptRandomIterator17 Iterator>
	struct Swapper
	{
		void operator()(Iterator iter1, Iterator iter2) const
		{
			std::iter_swap(iter1, iter2);
		}
	};

	template<internal::conceptRandomIterator17 Iterator>
	struct FindResult
	{
		Iterator iterator;
		bool found;
	};

	template<internal::conceptRandomIterator17 TIterator>
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
	template<internal::conceptRandomIterator17 Iterator,
		internal::conceptHasher<std::iter_value_t<Iterator>> Hasher>
	class IterHasher
	{
	public:
		explicit IterHasher(FastCopyableFunctor<Hasher> hasher) noexcept
			: mHasher(hasher)
		{
		}

		auto operator()(Iterator iter) const
		{
			return mHasher(*iter);
		}

		auto operator()(std::reverse_iterator<Iterator> iter) const
		{
			return mHasher(*iter);
		}

	private:
		FastCopyableFunctor<Hasher> mHasher;
	};

	template<internal::conceptRandomIterator17 Iterator,
		internal::conceptRandomIterator17 HashIterator>
	class IterPrehashFunc
	{
	public:
		explicit IterPrehashFunc(Iterator begin, HashIterator hashBegin) noexcept
			: mBegin(begin),
			mHashBegin(hashBegin)
		{
		}

		auto operator()(Iterator iter) const
		{
			return mHashBegin[iter - mBegin];
		}

		auto operator()(std::reverse_iterator<Iterator> iter) const
		{
			return mHashBegin[iter.base() - 1 - mBegin];
		}

	private:
		Iterator mBegin;
		HashIterator mHashBegin;
	};

	typedef internal::UIntMath<> SMath;

public:
	template<internal::conceptRandomIterator17 Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHasher<Item> Hasher = HashCoder<Item>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>,
		internal::conceptConstFunctor<void, Iterator, Iterator> IterSwapper = Swapper<Iterator>>
	static void Sort(Iterator begin, size_t count, Hasher hasher = Hasher(),
		EqualFunc equalFunc = EqualFunc(), IterSwapper iterSwapper = IterSwapper())
	{
		IterHasher<Iterator, Hasher> iterHasher((FastCopyableFunctor<Hasher>(hasher)));
		pvSort(begin, count, FastCopyableFunctor(iterHasher),
			FastCopyableFunctor<EqualFunc>(equalFunc),
			FastCopyableFunctor<IterSwapper>(iterSwapper));
	}

	template<internal::conceptRandomIterator17 Iterator,
		internal::conceptRandomIterator17 HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>,
		internal::conceptConstFunctor<void, Iterator, Iterator> IterSwapper = Swapper<Iterator>>
	requires std::is_same_v<HashCode&, std::iter_reference_t<HashIterator>>
	static void SortPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		EqualFunc equalFunc = EqualFunc(), IterSwapper iterSwapper = IterSwapper())
	{
		auto iterHashSwapper = [begin, hashBegin, iterSwapper] (Iterator iter1, Iterator iter2)
		{
			iterSwapper(iter1, iter2);
			std::swap(hashBegin[iter1 - begin], hashBegin[iter2 - begin]);
		};
		pvSort(begin, count,
			FastCopyableFunctor(IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin)),
			FastCopyableFunctor<EqualFunc>(equalFunc), FastCopyableFunctor(iterHashSwapper));
	}

	template<internal::conceptRandomIterator17 Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHasher<Item> Hasher = HashCoder<Item>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	static bool IsSorted(Iterator begin, size_t count, Hasher hasher = Hasher(),
		EqualFunc equalFunc = EqualFunc())
	{
		IterHasher<Iterator, Hasher> iterHasher((FastCopyableFunctor<Hasher>(hasher)));
		return pvIsSorted(begin, count, FastCopyableFunctor(iterHasher),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptRandomIterator17 Iterator,
		internal::conceptRandomIterator17 HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	requires std::is_same_v<HashCode, std::iter_value_t<HashIterator>>
	static bool IsSortedPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		EqualFunc equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count,
			FastCopyableFunctor(IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin)),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptRandomIterator17 Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHasher<Item> Hasher = HashCoder<Item>,
		internal::conceptEqualFunc<Item, Item, ItemArg> EqualFunc = std::equal_to<>>
	static FindResult<Iterator> Find(Iterator begin, size_t count,
		const ItemArg& itemArg, HashCode argHashCode,
		Hasher hasher = Hasher(), EqualFunc equalFunc = EqualFunc())
	{
		IterHasher<Iterator, Hasher> iterHasher((FastCopyableFunctor<Hasher>(hasher)));
		return pvFind(begin, count, itemArg, argHashCode, FastCopyableFunctor(iterHasher),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptRandomIterator17 Iterator, typename ItemArg,
		internal::conceptRandomIterator17 HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item, Item, ItemArg> EqualFunc = std::equal_to<>>
	requires std::is_same_v<HashCode, std::iter_value_t<HashIterator>>
	static FindResult<Iterator> FindPrehashed(Iterator begin, size_t count, const ItemArg& itemArg,
		HashCode argHashCode, HashIterator hashBegin, EqualFunc equalFunc = EqualFunc())
	{
		return pvFind(begin, count, itemArg, argHashCode,
			FastCopyableFunctor(IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin)),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptRandomIterator17 Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHasher<Item> Hasher = HashCoder<Item>,
		internal::conceptEqualFunc<Item, Item, ItemArg> EqualFunc = std::equal_to<>>
	static Bounds<Iterator> GetBounds(Iterator begin, size_t count,
		const ItemArg& itemArg, HashCode argHashCode,
		Hasher hasher = Hasher(), EqualFunc equalFunc = EqualFunc())
	{
		IterHasher<Iterator, Hasher> iterHasher((FastCopyableFunctor<Hasher>(hasher)));
		return pvGetBounds(begin, count, itemArg, argHashCode,
			FastCopyableFunctor(iterHasher), FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptRandomIterator17 Iterator, typename ItemArg,
		internal::conceptRandomIterator17 HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item, Item, ItemArg> EqualFunc = std::equal_to<>>
	requires std::is_same_v<HashCode, std::iter_value_t<HashIterator>>
	static Bounds<Iterator> GetBoundsPrehashed(Iterator begin, size_t count, const ItemArg& itemArg,
		HashCode argHashCode, HashIterator hashBegin, EqualFunc equalFunc = EqualFunc())
	{
		return pvGetBounds(begin, count, itemArg, argHashCode,
			FastCopyableFunctor(IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin)),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

private:
	template<internal::conceptRandomIterator17 Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptConstFunctor<size_t, Iterator> IterHasher,
		internal::conceptEqualFunc<Item> EqualFunc,
		internal::conceptConstFunctor<void, Iterator, Iterator> IterSwapper>
	static void pvSort(Iterator begin, size_t count, FastCopyableFunctor<IterHasher> iterHasher,
		FastCopyableFunctor<EqualFunc> equalFunc, FastCopyableFunctor<IterSwapper> iterSwapper)
	{
		auto itemsGrouper = [equalFunc, iterSwapper] (Iterator begin, size_t count)
		{
			if (count > 2)
				pvGroup(begin, count, equalFunc, iterSwapper);
		};
		internal::RadixSorter<>::Sort(begin, count, iterHasher, iterSwapper,
			FastCopyableFunctor(itemsGrouper));
	}

	template<internal::conceptRandomIterator17 Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc,
		internal::conceptConstFunctor<void, Iterator, Iterator> IterSwapper>
	static void pvGroup(Iterator begin, size_t count, FastCopyableFunctor<EqualFunc> equalFunc,
		FastCopyableFunctor<IterSwapper> iterSwapper)
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

	template<internal::conceptRandomIterator17 Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptConstFunctor<size_t, Iterator> IterHasher,
		internal::conceptEqualFunc<Item> EqualFunc>
	static bool pvIsSorted(Iterator begin, size_t count,
		FastCopyableFunctor<IterHasher> iterHasher, FastCopyableFunctor<EqualFunc> equalFunc)
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
				if (!pvIsGrouped(SMath::Next(begin, prevIndex), i - prevIndex, equalFunc))
					return false;
				prevIndex = i;
				prevHashCode = hashCode;
			}
		}
		return pvIsGrouped(SMath::Next(begin, prevIndex), count - prevIndex, equalFunc);
	}

	template<internal::conceptRandomIterator17 Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc>
	static bool pvIsGrouped(Iterator begin, size_t count, FastCopyableFunctor<EqualFunc> equalFunc)
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

	template<internal::conceptRandomIterator17 Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptConstFunctor<size_t, Iterator> IterHasher,
		internal::conceptEqualFunc<Item, Item, ItemArg> EqualFunc>
	static FindResult<Iterator> pvFind(Iterator begin, size_t count, const ItemArg& itemArg,
		HashCode argHashCode, FastCopyableFunctor<IterHasher> iterHasher,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		auto res = pvFindHashCode(begin, count, argHashCode, iterHasher);
		if (!res.found)
			return res;
		if (equalFunc(*res.iterator, itemArg))
			return res;
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), itemArg, argHashCode, iterHasher, equalFunc);
		if (revRes.found)
			return { revRes.iterator.base() - 1, true };
		return pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator),
			itemArg, argHashCode, iterHasher, equalFunc);
	}

	template<internal::conceptRandomIterator17 Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptConstFunctor<size_t, Iterator> IterHasher,
		internal::conceptEqualFunc<Item, Item, ItemArg> EqualFunc>
	static Bounds<Iterator> pvGetBounds(Iterator begin, size_t count, const ItemArg& itemArg,
		HashCode argHashCode, FastCopyableFunctor<IterHasher> iterHasher,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		auto res = pvFindHashCode(begin, count, argHashCode, iterHasher);
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
			SMath::Dist(begin, res.iterator + 1), itemArg, argHashCode, iterHasher, equalFunc);
		if (revRes.found)
		{
			Iterator resBegin = pvFindOther(revRes.iterator,
				SMath::Dist(begin, revRes.iterator.base()), equalFunc).base();
			return Bounds<Iterator>(resBegin, revRes.iterator.base());
		}
		res = pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator),
			itemArg, argHashCode, iterHasher, equalFunc);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		return Bounds<Iterator>(res.iterator,
			pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalFunc));
	}

	template<internal::conceptRandomIterator17 Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptConstFunctor<size_t, Iterator> IterHasher,
		internal::conceptEqualFunc<Item, Item, ItemArg> EqualFunc>
	static FindResult<Iterator> pvFindNext(Iterator begin, size_t count, const ItemArg& itemArg,
		HashCode argHashCode, FastCopyableFunctor<IterHasher> iterHasher,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		Iterator iter = begin;
		while (true)
		{
			iter = pvFindOther(iter, count - SMath::Dist(begin, iter), equalFunc);
			if (iter == SMath::Next(begin, count) || iterHasher(iter) != argHashCode)
				break;
			if (equalFunc(*iter, itemArg))
				return { iter, true };
		}
		return { iter, false };
	}

	template<internal::conceptRandomIterator17 Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc>
	static Iterator pvFindOther(Iterator begin, size_t count,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		MOMO_ASSERT(count > 0);
		auto iterThreeComp = [begin, equalFunc] (Iterator iter)
		{
			return equalFunc(*begin, *iter)
				? std::strong_ordering::less : std::strong_ordering::greater;
		};
		return pvExponentialSearch(begin + 1, count - 1, FastCopyableFunctor(iterThreeComp)).iterator;
	}

	template<internal::conceptRandomIterator17 Iterator,
		internal::conceptConstFunctor<size_t, Iterator> IterHasher>
	static FindResult<Iterator> pvFindHashCode(Iterator begin, size_t count,
		HashCode argHashCode, FastCopyableFunctor<IterHasher> iterHasher)
	{
		auto iterThreeComp = [argHashCode, iterHasher] (Iterator iter)
			{ return iterHasher(iter) <=> argHashCode; };
		size_t leftIndex = 0;
		size_t rightIndex = count;
		size_t middleIndex = pvMultShift(argHashCode, count);
		size_t step = pvGetStepCount(count);
		while (true)
		{
			HashCode middleHashCode = iterHasher(SMath::Next(begin, middleIndex));
			if (middleHashCode < argHashCode)
			{
				leftIndex = middleIndex + 1;
				if (step == 0)
				{
					return pvExponentialSearch(SMath::Next(begin, leftIndex),
						rightIndex - leftIndex, FastCopyableFunctor(iterThreeComp));
				}
				middleIndex += pvMultShift(argHashCode - middleHashCode, count);
				if (middleIndex >= rightIndex)
					break;
			}
			else if (middleHashCode > argHashCode)
			{
				rightIndex = middleIndex;
				if (step == 0)
				{
					typedef std::reverse_iterator<Iterator> ReverseIterator;
					auto revIterThreeComp = [argHashCode, iterHasher] (ReverseIterator iter)
						{ return argHashCode <=> iterHasher(iter); };
					auto res = pvExponentialSearch(ReverseIterator(SMath::Next(begin, rightIndex)),
						rightIndex - leftIndex, FastCopyableFunctor(revIterThreeComp));
					return { res.iterator.base() - (res.found ? 1 : 0), res.found };
				}
				size_t diff = pvMultShift(middleHashCode - argHashCode, count);
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
		return pvBinarySearch(SMath::Next(begin, leftIndex), rightIndex - leftIndex,
			FastCopyableFunctor(iterThreeComp));
	}

	template<internal::conceptRandomIterator17 Iterator,
		internal::conceptConstFunctor<std::strong_ordering, Iterator> IterThreeComparer>
	static FindResult<Iterator> pvExponentialSearch(Iterator begin, size_t count,
		FastCopyableFunctor<IterThreeComparer> iterThreeComp)
	{
		size_t leftIndex = 0;
		for (size_t i = 0; i < count; i = i * 2 + 2)
		{
			std::strong_ordering cmp = iterThreeComp(SMath::Next(begin, i));
			if (cmp > 0)
				return pvBinarySearch(SMath::Next(begin, leftIndex), i - leftIndex, iterThreeComp);
			else if (cmp == 0)
				return { SMath::Next(begin, i), true };
			leftIndex = i + 1;
		}
		return pvBinarySearch(SMath::Next(begin, leftIndex), count - leftIndex, iterThreeComp);
	}

	template<internal::conceptRandomIterator17 Iterator,
		internal::conceptConstFunctor<std::strong_ordering, Iterator> IterThreeComparer>
	static FindResult<Iterator> pvBinarySearch(Iterator begin, size_t count,
		FastCopyableFunctor<IterThreeComparer> iterThreeComp)
	{
		size_t leftIndex = 0;
		size_t rightIndex = count;
		while (leftIndex < rightIndex)
		{
			size_t middleIndex = (leftIndex + rightIndex) / 2;
			std::strong_ordering cmp = iterThreeComp(SMath::Next(begin, middleIndex));
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

	static size_t pvMultShift(HashCode value1, size_t value2) noexcept
	{
		static_assert(sizeof(HashCode) >= sizeof(size_t));
		static const size_t halfSize = 4 * sizeof(HashCode);
		static const HashCode halfMask = (HashCode{1} << halfSize) - 1;
		HashCode res = (value1 >> halfSize) * (value2 >> halfSize)
			+ (((value1 >> halfSize) * (value2 & halfMask)) >> halfSize)
			+ (((value2 >> halfSize) * (value1 & halfMask)) >> halfSize);
		return static_cast<size_t>(res);
	}
};

} // namespace momo
