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
		std::mt19937 mt;

		TestTemplMergeSet<false, 1>(mt);
		TestTemplMergeSet<false, 5>(mt);
		TestTemplMergeSet<true, 1>(mt);
		TestTemplMergeSet<true, 5>(mt);
	}

	template<bool isNothrowComparable, size_t logInitialItemCount>
	static void TestTemplMergeSet(std::mt19937& mt)
	{
		std::cout << "momo::MergeSet<" << isNothrowComparable << ", "
			<< logInitialItemCount << ">: " << std::flush;

		static const size_t count = 1 << 10;
		static size_t array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = i;

		typedef momo::MergeTraits<size_t, isNothrowComparable, logInitialItemCount> MergeTraits;
		momo::MergeSet<size_t, MergeTraits> set;

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

		std::cout << "ok" << std::endl;
	}
};

static int testSimpleMerge = (SimpleMergeTester::TestAll(), 0);

#endif // TEST_SIMPLE_MERGE
