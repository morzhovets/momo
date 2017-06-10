/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/TemplTreeTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_TEMPL_TREE

#undef NDEBUG

#include "../../momo/TreeSet.h"

#include <iostream>
#include <random>
#include <set>

class TemplTreeTester
{
public:
	static void TestAll()
	{
		TestTreeNode3<  1, 1, 127>();
		TestTreeNode3<  2, 1,  66>();
		TestTreeNode3<  3, 1,  32>();
		TestTreeNode3<  4, 1,  15>();
		TestTreeNode3<  5, 1,   1>();
		TestTreeNode3< 10, 1,   3>();
		TestTreeNode3<101, 1,   2>();
		TestTreeNode3<255, 1,   1>();

		TestTreeNode3<  4, 2, 127>();
		TestTreeNode3<  5, 2,  66>();
		TestTreeNode3<  6, 2,  32>();
		TestTreeNode3<  7, 2,  15>();
		TestTreeNode3< 12, 2,   1>();
		TestTreeNode3< 55, 2,   3>();
		TestTreeNode3<111, 2,   2>();
		TestTreeNode3<255, 2,   1>();

		TestTreeNode3<  6, 3, 127>();
		TestTreeNode3<  7, 3,  66>();
		TestTreeNode3<  8, 3,  32>();
		TestTreeNode3<  9, 3,  15>();
		TestTreeNode3< 14, 3,   1>();
		TestTreeNode3< 77, 3,   3>();
		TestTreeNode3<121, 3,   2>();
		TestTreeNode3<255, 3,   1>();

		TestTreeNode3< 37,   7, 127>();
		TestTreeNode3< 42,  15,  66>();
		TestTreeNode3< 65,  23,  32>();
		TestTreeNode3< 77,  30,  15>();
		TestTreeNode3< 88,  31,   1>();
		TestTreeNode3<104,  33,   3>();
		TestTreeNode3<204, 100,   2>();
		TestTreeNode3<255, 127,   1>();
	}

	template<size_t maxCapacity, size_t capacityStep, size_t memPoolBlockCount>
	static void TestTreeNode3()
	{
		static const bool useSwap = (maxCapacity + capacityStep) % 2 == 0;
		TestTreeNode4<maxCapacity, capacityStep, memPoolBlockCount, useSwap>();
	}

	template<size_t maxCapacity, size_t capacityStep, size_t memPoolBlockCount, bool useSwap>
	static void TestTreeNode4()
	{
		std::cout << "momo::TreeNode<" << maxCapacity << ", " << capacityStep << ", "
			<< memPoolBlockCount << ", " << (useSwap ? "true" : "false") << ">: " << std::flush;

		typedef momo::TreeNode<maxCapacity, capacityStep,
			momo::MemPoolParams<memPoolBlockCount>, useSwap> TreeNode;

		static const size_t count = 256;
		static unsigned char array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = (unsigned char)i;

		std::mt19937 mt;

		{
			std::set<unsigned char> sset;
			momo::TreeSet<unsigned char, momo::TreeTraits<unsigned char, TreeNode>> mset;

			std::shuffle(array, array + count, mt);
			for (unsigned char c : array)
			{
				sset.insert(c);
				assert(mset.Insert(c).inserted);
				assert(mset.GetCount() == sset.size());
				assert(std::equal(mset.GetBegin(), mset.GetEnd(), sset.begin()));
			}

			std::shuffle(array, array + count, mt);
			for (unsigned char c : array)
			{
				sset.erase(c);
				assert(mset.Remove(c));
				assert(mset.GetCount() == sset.size());
				assert(std::equal(mset.GetBegin(), mset.GetEnd(), sset.begin()));
			}

			assert(mset.IsEmpty());
			mset.Insert(128);
			assert(mset.GetCount() == 1);
		}

		std::cout << "ok" << std::endl;
	}
};

static int testTemplTree = (TemplTreeTester::TestAll(), 0);

#endif // TEST_TEMPL_TREE
