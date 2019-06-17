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
	template<typename Object,
		typename = void>
	struct RadixSorterCodeGetter;

	template<typename Object>
	struct RadixSorterCodeGetter<Object, EnableIf<std::is_integral<Object>::value>>
	{
		typedef typename UIntSelector<sizeof(Object)>::UInt Code;

		Code operator()(Object object) const noexcept
		{
			return static_cast<Code>(object);
		}
	};

	template<typename Object>
	struct RadixSorterCodeGetter<Object, EnableIf<std::is_pointer<Object>::value>>
	{
		uintptr_t operator()(Object object) const noexcept
		{
			return BitCaster::ToUInt(object);
		}
	};

	template<size_t tRadixSize = 8>
	class RadixSorter
	{
	public:
		static const size_t radixSize = tRadixSize;
		MOMO_STATIC_ASSERT(0 < radixSize && radixSize <= 16);

	private:
		static const size_t radixCount = (size_t)1 << radixSize;
		static const size_t selectionSortMaxCount = (size_t)1 << (radixSize / 2 + 1);

	public:
		template<typename Iterator,
			typename CodeGetter = RadixSorterCodeGetter<typename std::iterator_traits<Iterator>::value_type>>
		static void Sort(Iterator begin, size_t count, const CodeGetter& codeGetter = CodeGetter())
		{
			typedef decltype(codeGetter(*begin)) Code;
			pvSort<Code>(begin, count, codeGetter, 8 * sizeof(Code) - radixSize);
		}

	private:
		template<typename Code, typename Iterator, typename CodeGetter>
		static void pvSort(Iterator begin, size_t count, const CodeGetter& codeGetter, size_t shift)
		{
			if (count < 2)
				return;
			if (count == 2)
			{
				if (codeGetter(begin[0]) > codeGetter(begin[1]))
					std::iter_swap(begin, begin + 1);
				return;
			}
			if (count <= selectionSortMaxCount)
				pvSelectionSort<Code>(begin, count, codeGetter);
			else
				pvRadixSort<Code>(begin, count, codeGetter, shift);
		}

		template<typename Code, typename Iterator, typename CodeGetter>
		static void pvSelectionSort(Iterator begin, size_t count, const CodeGetter& codeGetter)
		{
			MOMO_ASSERT(count > 0);
			std::array<Code, selectionSortMaxCount> codes;	//?
			for (size_t i = 0; i < count; ++i)
				codes[i] = codeGetter(begin[i]);
			for (size_t i = 0; i < count - 1; ++i)
			{
				size_t minIndex = std::min_element(codes.data() + i + 1,
					codes.data() + count) - codes.data();
				if (codes[minIndex] < codes[i])
				{
					std::iter_swap(begin + i, begin + minIndex);
					std::swap(codes[i], codes[minIndex]);
				}
			}
		}

		template<typename Code, typename Iterator, typename CodeGetter>
		static void pvRadixSort(Iterator begin, size_t count, const CodeGetter& codeGetter,
			size_t shift)
		{
			MOMO_ASSERT(count > 0);
			std::array<size_t, radixCount> endIndexes;
			endIndexes.fill(0);
			Code code0 = codeGetter(begin[0]);
			size_t radix0 = pvGetRadix<Code>(code0, shift);
			++endIndexes[radix0];
			bool singleCode = true;
			bool singleRadix = true;
			for (size_t i = 1; i < count; ++i)
			{
				Code code = codeGetter(begin[i]);
				size_t radix = pvGetRadix<Code>(code, shift);
				++endIndexes[radix];
				singleCode &= (code == code0);
				singleRadix &= (radix == radix0);
			}
			if (singleCode)
				return;
			size_t nextShift = (shift > radixSize) ? shift - radixSize : 0;
			if (singleRadix)
			{
				MOMO_ASSERT(shift > 0);
				pvRadixSort<Code>(begin, count, codeGetter, nextShift);
				return;
			}
			for (size_t r = 1; r < radixCount; ++r)
				endIndexes[r] += endIndexes[r - 1];
			pvRadixSort<Code>(begin, codeGetter, shift, endIndexes);
			if (shift > 0)
			{
				size_t beginIndex = 0;
				for (size_t e : endIndexes)
				{
					pvSort<Code>(begin + beginIndex, e - beginIndex, codeGetter, nextShift);
					beginIndex = e;
				}
			}
		}

		template<typename Code, typename Iterator, typename CodeGetter>
		static void pvRadixSort(Iterator begin, const CodeGetter& codeGetter, size_t shift,
			const std::array<size_t, radixCount>& endIndexes)
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
					size_t radix = pvGetRadix<Code>(codeGetter(begin[beginIndex]), shift);
					if (radix != r)
						std::iter_swap(begin + beginIndex, begin + beginIndexes[radix]);
					++beginIndexes[radix];
				}
			}
		}

		template<typename Code>
		static size_t pvGetRadix(Code code, size_t shift) noexcept
		{
			return (size_t)(code >> shift) & (((size_t)1 << radixSize) - 1);
		}
	};
}

} // namespace momo
