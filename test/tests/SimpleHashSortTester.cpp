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

		std::cout << "momo::HashSorter: " << std::flush;
		TestHashSort(mt, false);
		std::cout << "ok" << std::endl;

		std::cout << "momo::HashSorter (prehashed): " << std::flush;
		TestHashSort(mt, true);
		std::cout << "ok" << std::endl;
	}

	static void TestHashSort(std::mt19937& mt, bool prehashed)
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
				hashArray.AddBack(std::hash<std::string>()(s));

			momo::HashSorter::SortPrehashed(array.GetBegin(), array.GetCount(),
				hashArray.GetBegin());
		}
		else
		{
			momo::HashSorter::Sort(array.GetBegin(), array.GetCount());
		}

		assert(momo::HashSorter::IsSorted(array.GetBegin(), array.GetCount()));

		if (prehashed)
		{
			assert(momo::HashSorter::IsSortedPrehashed(array.GetBegin(), array.GetCount(),
				hashArray.GetBegin()));
		}

		{
			assert(!momo::HashSorter::Find(array.GetBegin(), array.GetCount(), "a").second);

			auto per = momo::HashSorter::GetEqualRange(array.GetBegin(), array.GetCount(), "b");
			assert(per.first == per.second);
		}

		for (const auto& pair : data)
		{
			auto pf = momo::HashSorter::Find(array.GetBegin(), array.GetCount(), pair.first);
			assert(pf.second);
			assert(*pf.first == pair.first);

			auto per = momo::HashSorter::GetEqualRange(array.GetBegin(), array.GetCount(), pair.first);
			assert(momo::internal::UIntMath<>::Dist(per.first, per.second) == pair.second);
			assert(static_cast<size_t>(std::count(per.first, per.second, pair.first)) == pair.second);
		}

		if (prehashed)
		{
			{
				assert(!momo::HashSorter::FindPrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), "a", std::hash<std::string>()("a")).second);

				auto per = momo::HashSorter::GetEqualRangePrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), "b", std::hash<std::string>()("b"));
				assert(per.first == per.second);
			}

			for (const auto& pair : data)
			{
				auto pf = momo::HashSorter::FindPrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), pair.first, std::hash<std::string>()(pair.first));
				assert(pf.second);
				assert(*pf.first == pair.first);

				auto per = momo::HashSorter::GetEqualRangePrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), pair.first, std::hash<std::string>()(pair.first));
				assert(momo::internal::UIntMath<>::Dist(per.first, per.second) == pair.second);
				assert(static_cast<size_t>(std::count(per.first, per.second, pair.first)) == pair.second);
			}
		}
	}
};

static int testSimpleHashSort = (SimpleHashSortTester::TestAll(), 0);

#endif // TEST_SIMPLE_HASH_SORT
