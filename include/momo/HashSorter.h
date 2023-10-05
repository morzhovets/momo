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
	class IterHashFunc
	{
	public:
		explicit IterHashFunc(FastCopyableFunctor<HashFunc> hashFunc) noexcept
			: mHashFunc(hashFunc)
		{
		}

		template<typename Iterator>
		HashFuncResult operator()(Iterator iter) const
		{
			return mHashFunc(*iter);
		}

	private:
		FastCopyableFunctor<HashFunc> mHashFunc;
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
		internal::conceptConstFunctor<void, Iterator, Iterator> IterSwapper = Swapper<Iterator>>
	static void Sort(Iterator begin, size_t count, HashFunc hashFunc = HashFunc(),
		EqualFunc equalFunc = EqualFunc(), IterSwapper iterSwapper = IterSwapper())
	{
		IterHashFunc<HashFunc> iterHashFunc((FastCopyableFunctor<HashFunc>(hashFunc)));
		pvSort(begin, count, FastCopyableFunctor(iterHashFunc),
			FastCopyableFunctor<EqualFunc>(equalFunc),
			FastCopyableFunctor<IterSwapper>(iterSwapper));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>,
		internal::conceptConstFunctor<void, Iterator, Iterator> IterSwapper = Swapper<Iterator>>
	requires std::is_same_v<HashFuncResult&, std::iter_reference_t<HashIterator>>
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

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHashFunc<Item> HashFunc = HashCoder<Item>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	static bool IsSorted(Iterator begin, size_t count, HashFunc hashFunc = HashFunc(),
		EqualFunc equalFunc = EqualFunc())
	{
		IterHashFunc<HashFunc> iterHashFunc((FastCopyableFunctor<HashFunc>(hashFunc)));
		return pvIsSorted(begin, count, FastCopyableFunctor(iterHashFunc),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	requires std::is_same_v<HashFuncResult, std::iter_value_t<HashIterator>>
	static bool IsSortedPrehashed(Iterator begin, size_t count, HashIterator hashBegin,
		EqualFunc equalFunc = EqualFunc())
	{
		return pvIsSorted(begin, count,
			FastCopyableFunctor(IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin)),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHashFunc<Item> HashFunc = HashCoder<Item>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires internal::conceptEqualFunc<EqualFunc, Item>
	static FindResult<Iterator> Find(Iterator begin, size_t count,
		const ItemArg& itemArg, HashFuncResult itemHash,
		HashFunc hashFunc = HashFunc(), EqualFunc equalFunc = EqualFunc())
	{
		IterHashFunc<HashFunc> iterHashFunc((FastCopyableFunctor<HashFunc>(hashFunc)));
		return pvFind(begin, count, itemArg, itemHash, FastCopyableFunctor(iterHashFunc),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires std::is_same_v<HashFuncResult, std::iter_value_t<HashIterator>> &&
		internal::conceptEqualFunc<EqualFunc, Item>
	static FindResult<Iterator> FindPrehashed(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, HashIterator hashBegin, EqualFunc equalFunc = EqualFunc())
	{
		return pvFind(begin, count, itemArg, itemHash,
			FastCopyableFunctor(IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin)),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptHashFunc<Item> HashFunc = HashCoder<Item>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires internal::conceptEqualFunc<EqualFunc, Item>
	static Bounds<Iterator> GetBounds(Iterator begin, size_t count,
		const ItemArg& itemArg, HashFuncResult itemHash,
		HashFunc hashFunc = HashFunc(), EqualFunc equalFunc = EqualFunc())
	{
		IterHashFunc<HashFunc> iterHashFunc((FastCopyableFunctor<HashFunc>(hashFunc)));
		return pvGetBounds(begin, count, itemArg, itemHash,
			FastCopyableFunctor(iterHashFunc), FastCopyableFunctor<EqualFunc>(equalFunc));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		internal::conceptIterator<std::random_access_iterator_tag> HashIterator,
		conceptObject Item = std::iter_value_t<Iterator>,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	requires std::is_same_v<HashFuncResult, std::iter_value_t<HashIterator>> &&
		internal::conceptEqualFunc<EqualFunc, Item>
	static Bounds<Iterator> GetBoundsPrehashed(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, HashIterator hashBegin, EqualFunc equalFunc = EqualFunc())
	{
		return pvGetBounds(begin, count, itemArg, itemHash,
			FastCopyableFunctor(IterPrehashFunc<Iterator, HashIterator>(begin, hashBegin)),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

private:
	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptConstFunctor<size_t, Iterator> IterHashFunc,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>> EqualFunc,
		internal::conceptConstFunctor<void, Iterator, Iterator> IterSwapper>
	static void pvSort(Iterator begin, size_t count, FastCopyableFunctor<IterHashFunc> iterHashFunc,
		FastCopyableFunctor<EqualFunc> equalFunc, FastCopyableFunctor<IterSwapper> iterSwapper)
	{
		auto itemsGrouper = [equalFunc, iterSwapper] (Iterator begin, size_t count)
		{
			if (count > 2)
				pvGroup(begin, count, equalFunc, iterSwapper);
		};
		internal::RadixSorter<>::Sort(begin, count, iterHashFunc, iterSwapper,
			FastCopyableFunctor(itemsGrouper));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>> EqualFunc,
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

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptConstFunctor<size_t, Iterator> IterHashFunc,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>> EqualFunc>
	static bool pvIsSorted(Iterator begin, size_t count,
		FastCopyableFunctor<IterHashFunc> iterHashFunc, FastCopyableFunctor<EqualFunc> equalFunc)
	{
		size_t prevIndex = 0;
		HashFuncResult prevHash = iterHashFunc(begin);
		for (size_t i = 1; i < count; ++i)
		{
			HashFuncResult hash = iterHashFunc(SMath::Next(begin, i));
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

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>> EqualFunc>
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

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		internal::conceptConstFunctor<size_t, Iterator> IterHashFunc,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>, ItemArg> EqualFunc>
	static FindResult<Iterator> pvFind(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, FastCopyableFunctor<IterHashFunc> iterHashFunc,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, iterHashFunc);
		if (!res.found)
			return res;
		if (equalFunc(*res.iterator, itemArg))
			return res;
		auto revRes = pvFindNext(std::reverse_iterator<Iterator>(res.iterator + 1),
			SMath::Dist(begin, res.iterator + 1), itemArg, itemHash, iterHashFunc, equalFunc);
		if (revRes.found)
			return { revRes.iterator.base() - 1, true };
		return pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator),
			itemArg, itemHash, iterHashFunc, equalFunc);
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		internal::conceptConstFunctor<size_t, Iterator> IterHashFunc,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>, ItemArg> EqualFunc>
	static Bounds<Iterator> pvGetBounds(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, FastCopyableFunctor<IterHashFunc> iterHashFunc,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		auto res = pvFindHash(begin, count, itemHash, iterHashFunc);
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
			SMath::Dist(begin, res.iterator + 1), itemArg, itemHash, iterHashFunc, equalFunc);
		if (revRes.found)
		{
			Iterator resBegin = pvFindOther(revRes.iterator,
				SMath::Dist(begin, revRes.iterator.base()), equalFunc).base();
			return Bounds<Iterator>(resBegin, revRes.iterator.base());
		}
		res = pvFindNext(res.iterator, count - SMath::Dist(begin, res.iterator),
			itemArg, itemHash, iterHashFunc, equalFunc);
		if (!res.found)
			return Bounds<Iterator>(res.iterator, res.iterator);
		return Bounds<Iterator>(res.iterator,
			pvFindOther(res.iterator, count - SMath::Dist(begin, res.iterator), equalFunc));
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator, typename ItemArg,
		internal::conceptConstFunctor<size_t, Iterator> IterHashFunc,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>, ItemArg> EqualFunc>
	static FindResult<Iterator> pvFindNext(Iterator begin, size_t count, const ItemArg& itemArg,
		HashFuncResult itemHash, FastCopyableFunctor<IterHashFunc> iterHashFunc,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		Iterator iter = begin;
		while (true)
		{
			iter = pvFindOther(iter, count - SMath::Dist(begin, iter), equalFunc);
			if (iter == SMath::Next(begin, count) || iterHashFunc(iter) != itemHash)
				break;
			if (equalFunc(*iter, itemArg))
				return { iter, true };
		}
		return { iter, false };
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptEqualFunc<std::iter_value_t<Iterator>> EqualFunc>
	static Iterator pvFindOther(Iterator begin, size_t count,
		FastCopyableFunctor<EqualFunc> equalFunc)
	{
		MOMO_ASSERT(count > 0);
		auto compareFunc = [begin, equalFunc] (Iterator iter)
		{
			return equalFunc(*begin, *iter)
				? std::strong_ordering::less : std::strong_ordering::greater;
		};
		return pvExponentialSearch(begin + 1, count - 1, compareFunc).iterator;
	}

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptConstFunctor<size_t, Iterator> IterHashFunc>
	static FindResult<Iterator> pvFindHash(Iterator begin, size_t count,
		HashFuncResult itemHash, FastCopyableFunctor<IterHashFunc> iterHashFunc)
	{
		auto compareFunc = [itemHash, iterHashFunc] (Iterator iter)
			{ return iterHashFunc(iter) <=> itemHash; };
		size_t leftIndex = 0;
		size_t rightIndex = count;
		size_t middleIndex = pvMultShift(itemHash, count);
		size_t step = pvGetStepCount(count);
		while (true)
		{
			HashFuncResult middleHash = iterHashFunc(SMath::Next(begin, middleIndex));
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
					auto revCompareFunc = [itemHash, iterHashFunc] (ReverseIterator iter)
						{ return itemHash <=> iterHashFunc(iter); };
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

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptConstFunctor<std::strong_ordering, Iterator> CompareFunc>
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

	template<internal::conceptIterator<std::random_access_iterator_tag> Iterator,
		internal::conceptConstFunctor<std::strong_ordering, Iterator> CompareFunc>
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
