/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/SimpleHashSortTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_HASH_SORT

#include "../../include/momo/HashSorter.h"
#include "../../include/momo/Array.h"

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

			auto bounds = momo::HashSorter::GetBounds(array.GetBegin(), array.GetCount(),
				"b", hasher);
			assert(bounds.GetCount() == 0);
		}

		for (const auto& pair : data)
		{
			auto findRes = momo::HashSorter::Find(array.GetBegin(), array.GetCount(),
				pair.first, hasher);
			assert(findRes.found);
			assert(*findRes.iterator == pair.first);

			auto bounds = momo::HashSorter::GetBounds(array.GetBegin(), array.GetCount(),
				pair.first, hasher);
			assert(bounds.GetCount() == pair.second);
			assert(static_cast<size_t>(std::count(bounds.GetBegin(), bounds.GetEnd(), pair.first))
				== pair.second);
		}

		if (prehashed)
		{
			{
				assert(!momo::HashSorter::FindPrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), "a", hasher("a")).found);

				auto bounds = momo::HashSorter::GetBoundsPrehashed(array.GetBegin(),
					array.GetCount(), hashArray.GetBegin(), "b", hasher("b"));
				assert(bounds.GetCount() == 0);
			}

			for (const auto& pair : data)
			{
				auto findRes = momo::HashSorter::FindPrehashed(array.GetBegin(), array.GetCount(),
					hashArray.GetBegin(), pair.first, hasher(pair.first));
				assert(findRes.found);
				assert(*findRes.iterator == pair.first);

				auto bounds = momo::HashSorter::GetBoundsPrehashed(array.GetBegin(),
					array.GetCount(), hashArray.GetBegin(), pair.first, hasher(pair.first));
				assert(bounds.GetCount() == pair.second);
				assert(static_cast<size_t>(std::count(bounds.GetBegin(), bounds.GetEnd(), pair.first))
					== pair.second);
			}
		}
	}
};

static int testSimpleHashSort = (SimpleHashSortTester::TestAll(), 0);

#endif // TEST_SIMPLE_HASH_SORT
