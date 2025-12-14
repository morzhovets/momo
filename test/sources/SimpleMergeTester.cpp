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
private:
	template<typename Key, momo::MergeTraitsFunc tMergeTraitsFunc,
		size_t tLogInitialItemCount, typename TMergeBloomFilter>
	class MergeTraits
		: public momo::MergeTraits<Key, tMergeTraitsFunc, tLogInitialItemCount, TMergeBloomFilter>
	{
	public:
		static const size_t logInitialItemCount = tLogInitialItemCount;

	public:
		size_t GetSegmentItemCount(size_t segIndex) const noexcept
		{
			if (logInitialItemCount < 3)
				return size_t{1} << (logInitialItemCount + segIndex * 2);
			else
				return size_t{1} << (logInitialItemCount + ((segIndex > 0) ? segIndex - 1 : 0));
		}
	};

public:
	static void TestAll()
	{
		std::mt19937 mt;

		TestTemplMergeTraits<uint16_t, momo::MergeTraitsFunc::hash, 0, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<uint16_t, momo::MergeTraitsFunc::lessNothrow, 0, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<uint16_t, momo::MergeTraitsFunc::lessThrow, 0, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<uint16_t, momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<uint16_t, momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilterEmpty, false>(mt);
		TestTemplMergeTraits<uint16_t, momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilterEmpty, false>(mt);

		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::hash, 1, momo::MergeBloomFilter<0>, false>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessNothrow, 2, momo::MergeBloomFilter<1>, false>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessThrow, 1, momo::MergeBloomFilter<2>, false>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilter<>, false>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilter<>, false>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilter<>, false>(mt);

		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilterEmpty, true>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilterEmpty, true>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilterEmpty, true>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::hash, 3, momo::MergeBloomFilter<>, true>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessNothrow, 3, momo::MergeBloomFilter<>, true>(mt);
		TestTemplMergeTraits<size_t, momo::MergeTraitsFunc::lessThrow, 3, momo::MergeBloomFilter<>, true>(mt);
	}

	template<typename Key, momo::MergeTraitsFunc mergeTraitsFunc, size_t logInitialItemCount,
		typename MergeBloomFilter, bool useValuePtr>
	static void TestTemplMergeTraits(std::mt19937& mt)
	{
		std::cout << "momo::MergeTraits<" << sizeof(Key) << ", " 
			<< static_cast<int>(mergeTraitsFunc) << ", " << logInitialItemCount << ", ";
		if constexpr (MergeBloomFilter::isAlwaysEmpty)
			std::cout << "empty";
		else
			std::cout << MergeBloomFilter::logMult;
		std::cout << ">" << (useValuePtr ? " (+useValuePtr)" : "") << ": " << std::flush;

		static const size_t count = 1 << 10;
		static Key array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = Key(i);

		typedef momo::MergeMapCore<
			momo::MergeMapKeyValueTraits<Key, Key, momo::MemManagerDict<>, useValuePtr>,
				MergeTraits<Key, mergeTraitsFunc, logInitialItemCount, MergeBloomFilter>> MergeMap;
		MergeMap map;

		std::shuffle(array, array + count, mt);

		for (Key k : array)
		{
			if (size_t{k} < count / 4)
				map[k] = k;
			else if (size_t{k} < count / 2)
				map.Insert(k, k);
			else
				map.Add(typename MergeMap::ConstPosition(), k, k);
		}

		auto map2 = map;

		assert(map2.GetCount() == count);
		for (auto [k, v] : map2)
			assert(map.ContainsKey(k));

		std::shuffle(array, array + count, mt);

		for (Key k : array)
			assert(map2.ContainsKey(k));

		if constexpr (mergeTraitsFunc == momo::MergeTraitsFunc::hash)
		{
			for (Key k : array)
			{
				if (k % 2 == 0)
					continue;

				assert(map2.Remove(k));

				bool cont = false;
				for (Key t : array)
				{
					assert(map2.ContainsKey(t) == (cont || t % 2 == 0));
					if (t == k)
						cont = true;
				}
			}

			for (auto [k, v] : map2)
				assert(v % 2 == 0);

			std::shuffle(array, array + count, mt);

			for (Key k : array)
				map2[k] = k;

			assert(map2.GetCount() == count);
			for (auto [k, v] : map)
				assert(map2.ContainsKey(k));

			std::shuffle(array, array + count, mt);

			map2.Remove([] (auto key, auto /*value*/) { return key % 2 == 0; });

			for (auto [k, v] : map2)
				assert(v % 2 == 1);

			for (Key k : array)
			{
				auto pos = map2.Find(k);
				assert(!!pos == (k % 2 == 1));
				if (!!pos)
					map2.Extract(pos);
			}

			assert(map2.IsEmpty());
		}

		map.Clear();
		assert(map.IsEmpty());

		std::cout << "ok" << std::endl;
	}
};

static int testSimpleMerge = (SimpleMergeTester::TestAll(), 0);

#endif // TEST_SIMPLE_MERGE
