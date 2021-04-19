/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/RadixSorter.h

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename Iterator>
	struct RadixSorterCodeGetter;

	template<typename Iterator>
	requires std::integral<std::iter_value_t<Iterator>> &&
		std::unsigned_integral<typename UIntSelector<sizeof(std::iter_value_t<Iterator>)>::UInt>
	struct RadixSorterCodeGetter<Iterator>
	{
		auto operator()(Iterator iter) const noexcept
		{
			typedef typename UIntSelector<sizeof(std::iter_value_t<Iterator>)>::UInt Code;
			return static_cast<Code>(*iter);
		}
	};

	template<typename Iterator>
	requires std::is_pointer_v<std::iter_value_t<Iterator>>
	struct RadixSorterCodeGetter<Iterator>
	{
		uintptr_t operator()(Iterator iter) const noexcept
		{
			return PtrCaster::ToUInt(*iter);
		}
	};

	template<size_t tRadixSize = 8>
	requires (0 < tRadixSize && tRadixSize <= 16)
	class RadixSorter
	{
	public:
		static const size_t radixSize = tRadixSize;

	private:
		static const size_t radixCount = size_t{1} << radixSize;
		static const size_t selectionSortMaxCount = size_t{1} << (radixSize / 2 + 1);

	public:
		template<conceptIterator<std::random_access_iterator_tag> Iterator,
			typename CodeGetter = RadixSorterCodeGetter<Iterator>>
		requires std::unsigned_integral<std::invoke_result_t<const CodeGetter&, Iterator>>
		static void Sort(Iterator begin, size_t count, const CodeGetter& codeGetter = CodeGetter())
		{
			auto swapFunc = [] (Iterator iter1, Iterator iter2) { std::iter_swap(iter1, iter2); };
			auto groupFunc = [] (Iterator, size_t) {};
			Sort(begin, count, codeGetter, swapFunc, groupFunc);
		}

		template<conceptIterator<std::random_access_iterator_tag> Iterator,
			typename CodeGetter, typename SwapFunc, typename GroupFunc>
		requires std::unsigned_integral<std::invoke_result_t<const CodeGetter&, Iterator>> &&
			std::invocable<const SwapFunc&, Iterator, Iterator> &&
			std::invocable<const GroupFunc&, Iterator, size_t>
		static void Sort(Iterator begin, size_t count, const CodeGetter& codeGetter,
			const SwapFunc& swapFunc, const GroupFunc& groupFunc)
		{
			typedef decltype(codeGetter(begin)) Code;
			pvSort<Code>(begin, count, codeGetter, swapFunc, groupFunc,
				8 * sizeof(Code) - radixSize);
		}

	private:
		template<typename Code, typename Iterator, typename CodeGetter,
			typename SwapFunc, typename GroupFunc>
		static void pvSort(Iterator begin, size_t count, const CodeGetter& codeGetter,
			const SwapFunc& swapFunc, const GroupFunc& groupFunc, size_t shift)
		{
			if (count < 2)
				return;
			if (count == 2)
			{
				if (codeGetter(begin) > codeGetter(begin + 1))
					swapFunc(begin, begin + 1);
				return;
			}
			if (count <= selectionSortMaxCount)
				pvSelectionSort<Code>(begin, count, codeGetter, swapFunc, groupFunc);
			else
				pvRadixSort<Code>(begin, count, codeGetter, swapFunc, groupFunc, shift);
		}

		template<typename Code, typename Iterator, typename CodeGetter,
			typename SwapFunc, typename GroupFunc>
		static void pvSelectionSort(Iterator begin, size_t count, const CodeGetter& codeGetter,
			const SwapFunc& swapFunc, const GroupFunc& groupFunc)
		{
			MOMO_ASSERT(count > 0);
			std::array<Code, selectionSortMaxCount> codes;	//?
			for (size_t i = 0; i < count; ++i)
				codes[i] = codeGetter(UIntMath<>::Next(begin, i));
			for (size_t i = 0; i < count - 1; ++i)
			{
				size_t minIndex = UIntMath<>::Dist(codes.data(),
					std::min_element(codes.data() + i + 1, codes.data() + count));
				if (codes[minIndex] < codes[i])
				{
					swapFunc(UIntMath<>::Next(begin, i), UIntMath<>::Next(begin, minIndex));
					std::swap(codes[i], codes[minIndex]);
				}
			}
			size_t prevIndex = 0;
			for (size_t i = 1; i < count; ++i)
			{
				if (codes[i] != codes[prevIndex])
				{
					groupFunc(UIntMath<>::Next(begin, prevIndex), i - prevIndex);
					prevIndex = i;
				}
			}
			groupFunc(UIntMath<>::Next(begin, prevIndex), count - prevIndex);
		}

		template<typename Code, typename Iterator, typename CodeGetter,
			typename SwapFunc, typename GroupFunc>
		static void pvRadixSort(Iterator begin, size_t count, const CodeGetter& codeGetter,
			const SwapFunc& swapFunc, const GroupFunc& groupFunc, size_t shift)
		{
			MOMO_ASSERT(count > 0);
			std::array<size_t, radixCount> endIndexes;
			endIndexes.fill(0);
			Code code0 = codeGetter(begin);
			size_t radix0 = pvGetRadix<Code>(code0, shift);
			++endIndexes[radix0];
			bool singleCode = true;
			bool singleRadix = true;
			for (size_t i = 1; i < count; ++i)
			{
				Code code = codeGetter(UIntMath<>::Next(begin, i));
				size_t radix = pvGetRadix<Code>(code, shift);
				++endIndexes[radix];
				singleCode &= (code == code0);
				singleRadix &= (radix == radix0);
			}
			if (singleCode)
				return groupFunc(begin, count);
			size_t nextShift = (shift > radixSize) ? shift - radixSize : 0;
			if (singleRadix)
			{
				MOMO_ASSERT(shift > 0);
				return pvRadixSort<Code>(begin, count, codeGetter, swapFunc, groupFunc, nextShift);
			}
			for (size_t r = 1; r < radixCount; ++r)
				endIndexes[r] += endIndexes[r - 1];
			pvRadixSort<Code>(begin, codeGetter, swapFunc, shift, endIndexes);
			size_t beginIndex = 0;
			if (shift > 0)
			{
				for (size_t e : endIndexes)
				{
					pvSort<Code>(UIntMath<>::Next(begin, beginIndex), e - beginIndex, codeGetter,
						swapFunc, groupFunc, nextShift);
					beginIndex = e;
				}
			}
			else
			{
				for (size_t e : endIndexes)
				{
					groupFunc(UIntMath<>::Next(begin, beginIndex), e - beginIndex);
					beginIndex = e;
				}
			}
		}

		template<typename Code, typename Iterator, typename CodeGetter, typename SwapFunc>
		static void pvRadixSort(Iterator begin, const CodeGetter& codeGetter, const SwapFunc& swapFunc,
			size_t shift, const std::array<size_t, radixCount>& endIndexes)
		{
			std::array<size_t, radixCount> beginIndexes;
			beginIndexes[0] = 0;
			for (size_t r = 1; r < radixCount; ++r)
				beginIndexes[r] = endIndexes[r - 1];
			for (size_t r = 0; r < radixCount; ++r)
			{
				size_t& beginIndex = beginIndexes[r];
				size_t endIndex = endIndexes[r];
				while (beginIndex < endIndex)
				{
					size_t radix = pvGetRadix<Code>(codeGetter(UIntMath<>::Next(begin, beginIndex)), shift);
					if (radix != r)
					{
						swapFunc(UIntMath<>::Next(begin, beginIndex),
							UIntMath<>::Next(begin, beginIndexes[radix]));
					}
					++beginIndexes[radix];
				}
			}
		}

		template<typename Code>
		static size_t pvGetRadix(Code code, size_t shift) noexcept
		{
			return static_cast<size_t>(code >> shift) & ((size_t{1} << radixSize) - 1);
		}
	};
}

} // namespace momo
