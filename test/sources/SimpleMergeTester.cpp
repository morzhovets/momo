/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/SimpleMergeTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_MERGE

#include "../../include/momo/MergeMap.h"
#include "../../include/momo/MemManagerDict.h"

#include <iostream>
#include <random>

class SimpleMergeTester
{

public:
	static void TestAll()
	{
		std::mt19937 mt;

		TestTemplMergeTraits<momo::MergeTraitsFunc::hash, 0, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessNothrow, 0, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessThrow, 0, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilterEmpty, false>(mt);

		TestTemplMergeTraits<momo::MergeTraitsFunc::hash, 1, momo::MergeBloomFilter<0>, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessNothrow, 2, momo::MergeBloomFilter<1>, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessThrow, 0, momo::MergeBloomFilter<2>, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilter<>, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilter<>, false>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilter<>, false>(mt);

		TestTemplMergeTraits<momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilterEmpty, true>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilterEmpty, true>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilterEmpty, true>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilter<>, true>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilter<>, true>(mt);
		TestTemplMergeTraits<momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilter<>, true>(mt);
	}

	template<momo::MergeTraitsFunc mergeTraitsFunc, size_t logInitialItemCount,
		typename MergeBloomFilter, bool useValuePtr>
	static void TestTemplMergeTraits(std::mt19937& mt)
	{
		std::cout << "momo::MergeTraits<" << static_cast<int>(mergeTraitsFunc) << ", "
			<< logInitialItemCount << ", ";
		if constexpr (MergeBloomFilter::isAlwaysEmpty)
			std::cout << "empty";
		else
			std::cout << MergeBloomFilter::logMult;
		std::cout << "> (" << (useValuePtr ? "+" : "-") << "useValuePtr): " << std::flush;

		static const size_t count = 1 << 10;
		static size_t array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = i;

		typedef momo::MergeTraits<size_t, mergeTraitsFunc,
			momo::MergeArraySettings<logInitialItemCount>, MergeBloomFilter> MergeTraits;
		typedef momo::MergeMap<size_t, size_t, MergeTraits, momo::MemManagerDict<>,
			momo::MergeMapKeyValueTraits<size_t, size_t, momo::MemManagerDict<>, useValuePtr>> MergeMap;
		MergeMap map;

		std::shuffle(array, array + count, mt);
		for (size_t k : array)
		{
			if (k < count / 4)
				map[k] = k;
			else if (k < count / 2)
				map.Insert(k, k);
			else
				map.Add(typename MergeMap::ConstPosition(), k, k);
		}

		auto map2 = map;
		assert(map2.GetCount() == count);

		std::shuffle(array, array + count, mt);
		for (size_t k : array)
			assert(map2.ContainsKey(k));

		for (auto [k, v] : map2)
			assert(map.ContainsKey(k));

		map2.Clear();
		assert(map2.IsEmpty());

		std::cout << "ok" << std::endl;
	}
};

static int testSimpleMerge = (SimpleMergeTester::TestAll(), 0);

#endif // TEST_SIMPLE_MERGE
