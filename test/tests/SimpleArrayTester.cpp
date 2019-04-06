/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleArrayTester.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_ARRAY

#undef NDEBUG

#include "../../momo/Array.h"
#include "../../momo/SegmentedArray.h"

#include <string>
#include <iostream>

class SimpleArrayTester
{
public:
	static void TestAll()
	{
		std::cout << "momo::Array (1): " << std::flush;
		typedef momo::Array<std::string> Array1;
		TestStrArray<Array1>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::Array (2): " << std::flush;
		typedef momo::Array<std::string, momo::MemManagerC,
			momo::ArrayItemTraits<std::string, momo::MemManagerC>,
			momo::ArraySettings<std::is_nothrow_move_constructible<std::string>::value ? 2 : 0>> Array2;
		TestStrArray<Array2>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::SegmentedArray: " << std::flush;
		typedef momo::SegmentedArray<std::string> SegmentedArray;
		TestStrArray<SegmentedArray>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::SegmentedArraySqrt: " << std::flush;
		typedef momo::SegmentedArraySqrt<std::string> SegmentedArraySqrt;
		TestStrArray<SegmentedArraySqrt>();
		std::cout << "ok" << std::endl;
	}

	template<typename Array>
	static void TestStrArray()
	{
		Array ar;
		ar = Array(2, "s");
		ar = Array(1);
		std::string s1 = "s1";
		ar.AddBack(s1);
		ar.AddBack("s2");
		ar.AddBackCrt([] (void* ptr) { ::new(ptr) std::string("s3"); });
		ar.Reserve(10);
		assert(ar.GetCapacity() >= 10);
		assert(ar.Contains("s3"));
		assert(!ar.Contains("s"));
		ar.AddBackNogrow(s1);
		ar.AddBackNogrow("s2");
		ar.AddBackNogrowCrt([] (void* ptr) { ::new(ptr) std::string("s3"); });
		ar.Insert(0, s1);
		ar.Insert(1, "s2");
		ar.Insert(3, ar[0]);
		ar.Insert(3, std::move(ar[3]));
		ar.Insert(3, 2, ar[0]);
		ar.SetCount(14);
		ar.SetCount(20, "s");
		ar.Remove(3, 7);
		ar.RemoveBack(5);
		ar = ar;
		ar = std::move(ar);
		ar = Array(std::make_move_iterator(ar.GetBegin()) + 3,
			std::make_move_iterator(ar.GetEnd()));
		assert(ar.GetCount() == 5);
		ar.SetCount(4);
		ar.Shrink();
		//ar.Insert(1, 2, std::make_move_iterator(ar.GetBegin()));
		ar.Insert(1, std::move(ar[0]));
		ar.Insert(2, std::move(ar[2]));
		assert(ar[0] == "");
		assert(ar[1] == "s1");
		assert(ar[2] == "s2");
		assert(ar[3] == "");
		assert(ar[4] == "s3");
		assert(ar.GetBackItem() == "");
		ar.Clear();
		assert(ar.IsEmpty());
		ar.AddBack(std::move(s1));
		for (std::string& s : ar)
			assert(s == "s1");
	}
};

static int testSimpleArray = (SimpleArrayTester::TestAll(), 0);

#endif // TEST_SIMPLE_ARRAY
