/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleHashSortTester.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH_SORT

#undef NDEBUG

#include "../../momo/HashSorter.h"
#include "../../momo/Array.h"

#include <string>
#include <iostream>
#include <random>
#include <sstream>

class SimpleHashSortTester
{
public:
	static void TestAll()
	{
		std::mt19937 mt;
		std::hash<std::string> hasher;

		std::cout << "momo::HashSorter: " << std::flush;
		TestHashSort(mt, false, hasher);
		std::cout << "ok" << std::endl;

		std::cout << "momo::HashSorter (prehashed): " << std::flush;
		TestHashSort(mt, true, hasher);
		std::cout << "ok" << std::endl;

		std::cout << "momo::HashSorter (const): " << std::flush;
		TestHashSort(mt, false, [] (const std::string&) { return size_t{0}; });
		std::cout << "ok" << std::endl;
	}

	template<typename Hasher>
	static void TestHashSort(std::mt19937& mt, bool prehashed, const Hasher& hasher)
	{
		static const size_t count = 128;
		momo::Array<std::pair<std::string, size_t>> data;
		for (size_t i = 0; i < count; ++i)
		{
			std::stringstream sstream;	// gcc 4.9
			sstream << mt();
			data.AddBack({ sstream.str(), mt() % 32 + 1 });
		}

		momo::Array<std::string> array;
		for (const auto& pair : data)
		{
			for (size_t i = 0; i < pair.second; ++i)
				array.AddBack(pair.first);
		}
		std::shuffle(array.GetBegin(), array.GetEnd(), mt);

		momo::Array<size_t> hashArray;

		if (prehashed)
		{
			for (const std::string& s : array)
				hashArray.AddBack(hasher(s));

			momo::HashSorter::SortPrehashed(array.GetBegin(), array.GetCount(),
				hashArray.GetBegin());
		}
		else
		{
			momo::HashSorter::Sort(array.GetBegin(), array.GetCount(), hasher);
		}

		assert(momo::HashSorter::IsSorted(array.GetBegin(), array.GetCount(), hasher));

		if (prehashed)
		{
			assert(momo::HashSorter::IsSortedPrehashed(array.GetBegin(), array.GetCount(),
				hashArray.GetBegin()));
			assert(std::is_sorted(hashArray.GetBegin(), hashArray.GetEnd()));
		}

		{
			assert(!momo::HashSorter::Find(array.GetBegin(), array.GetCount(), "a", hasher).found);

			auto per = momo::HashSorter::GetEqualRange(array.GetBegin(), array.GetCount(), "b", hasher);
			assert(per.first == per.second);
		}

		for (const auto& pair : data)
		{
			auto pf = momo::HashSorter::Find(array.GetBegin(), array.GetCount(), pair.first, hasher);
			assert(pf.found);
			assert(*pf.iterator == pair.first);

			auto per = momo::HashSorter::GetEqualRange(array.GetBegin(), array.GetCount(),
				pair.first, hasher);
			assert(momo::internal::UIntMath<>::Dist(per.first, per.second) == pair.second);
			assert(static_cast<size_t>(std::count(per.first, per.second, pair.first)) == pair.second);
		}

		if (prehashed)
		{
			{
				assert(!momo::HashSorter::FindPrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), "a", hasher("a")).found);

				auto per = momo::HashSorter::GetEqualRangePrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), "b", hasher("b"));
				assert(per.first == per.second);
			}

			for (const auto& pair : data)
			{
				auto pf = momo::HashSorter::FindPrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), pair.first, hasher(pair.first));
				assert(pf.found);
				assert(*pf.iterator == pair.first);

				auto per = momo::HashSorter::GetEqualRangePrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), pair.first, hasher(pair.first));
				assert(momo::internal::UIntMath<>::Dist(per.first, per.second) == pair.second);
				assert(static_cast<size_t>(std::count(per.first, per.second, pair.first)) == pair.second);
			}
		}
	}
};

static int testSimpleHashSort = (SimpleHashSortTester::TestAll(), 0);

#endif // TEST_SIMPLE_HASH_SORT
