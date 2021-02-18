/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleMergeTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_MERGE

#include "../../momo/MergeSet.h"

#include <iostream>
#include <random>

class SimpleMergeTester
{

public:
	static void TestAll()
	{
		std::cout << "momo::MergeSet<1>: " << std::flush;
		TestMergeSet<1>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::MergeSet<5>: " << std::flush;
		TestMergeSet<5>();
		std::cout << "ok" << std::endl;
	}

	template<size_t logInitialItemCount>
	static void TestMergeSet()
	{
		std::mt19937 mt;

		static const size_t count = 1 << 10;
		static size_t array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = i;

		momo::MergeSet<size_t, momo::MergeTraits<size_t, logInitialItemCount>> set;

		std::shuffle(array, array + count, mt);
		for (size_t k : array)
			set.Insert(k);

		auto set2 = set;
		assert(set2.GetCount() == count);

		std::shuffle(array, array + count, mt);
		for (size_t k : array)
			assert(set2.ContainsKey(k));

		for (size_t k : set2)
			assert(set.ContainsKey(k));

		set2.Clear();
		assert(set2.IsEmpty());
	}
};

static int testSimpleMerge = (SimpleMergeTester::TestAll(), 0);

#endif // TEST_SIMPLE_MERGE
