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

class TemplTreeTester
{
public:
	static void TestAll()
	{
		TestTreeNode2<1, 1>();
		TestTreeNode2<2, 1>();
		TestTreeNode2<3, 1>();
		TestTreeNode2<4, 1>();
		TestTreeNode2<5, 1>();
		TestTreeNode2<10, 1>();
		TestTreeNode2<101, 1>();
		TestTreeNode2<255, 1>();

		TestTreeNode2<4, 2>();
		TestTreeNode2<5, 2>();
		TestTreeNode2<6, 2>();
		TestTreeNode2<7, 2>();
		TestTreeNode2<12, 2>();
		TestTreeNode2<55, 2>();
		TestTreeNode2<111, 2>();
		TestTreeNode2<255, 2>();

		TestTreeNode2<6, 3>();
		TestTreeNode2<7, 3>();
		TestTreeNode2<8, 3>();
		TestTreeNode2<9, 3>();
		TestTreeNode2<14, 3>();
		TestTreeNode2<77, 3>();
		TestTreeNode2<121, 3>();
		TestTreeNode2<255, 3>();

		TestTreeNode2<37, 7>();
		TestTreeNode2<42, 15>();
		TestTreeNode2<65, 23>();
		TestTreeNode2<77, 30>();
		TestTreeNode2<88, 31>();
		TestTreeNode2<104, 33>();
		TestTreeNode2<204, 100>();
		TestTreeNode2<255, 127>();
	}

	template<size_t maxCapacity, size_t capacityStep>
	static void TestTreeNode2()
	{
		static const bool useSwap = (maxCapacity + capacityStep) % 2 == 0;
		TestTreeNode3<maxCapacity, capacityStep, useSwap>();
	}

	template<size_t maxCapacity, size_t capacityStep, bool useSwap>
	static void TestTreeNode3()
	{
		std::cout << "momo::TreeNode<" << maxCapacity << ", " << capacityStep << ", "
			<< (useSwap ? "true" : "false") << ">: " << std::flush;

		static const size_t count = 256;
		static unsigned char array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = (unsigned char)i;

		momo::TreeSet<unsigned char, momo::TreeTraits<unsigned char,
			momo::TreeNode<maxCapacity, capacityStep, useSwap>>> set;

		std::shuffle(array, array + count, std::mt19937());
		for (unsigned char c : array)
			assert(set.Insert(c).inserted);
		assert(set.GetCount() == count);

		std::shuffle(array, array + count, std::mt19937());
		for (unsigned char c : array)
			assert(set.Remove(c));
		assert(set.IsEmpty());

		set.Insert(128);

		std::cout << "ok" << std::endl;
	}
};

static int testTemplTree = (TemplTreeTester::TestAll(), 0);

#endif // TEST_TEMPL_TREE
