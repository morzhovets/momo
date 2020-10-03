/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleArrayTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_ARRAY

#include "../../momo/Array.h"
#include "../../momo/SegmentedArray.h"

#include <string>
#include <iostream>

class SimpleArrayTester
{
private:
	template<bool isNothrowMoveConstructible>
	class TemplItem
	{
	public:
		explicit TemplItem(size_t value) noexcept
			: mValue(value)
		{
		}

		TemplItem(TemplItem&& item) noexcept(isNothrowMoveConstructible)
			: mValue(item.mValue)
		{
		}

		TemplItem(const TemplItem& item) noexcept(false)
			: mValue(item.mValue)
		{
		}

		~TemplItem() = default;

		TemplItem& operator=(TemplItem&& item) = default;

		TemplItem& operator=(const TemplItem& item) = default;

		bool operator==(const TemplItem& item) const noexcept
		{
			return mValue == item.mValue;
		}

	private:
		size_t mValue;
	};

public:
	static void TestStrAll()
	{
		std::cout << "momo::Array<std::string>: " << std::flush;
		TestStrArray<momo::Array<std::string>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::ArrayIntCap<..., std::string>: " << std::flush;
		TestStrArray<momo::ArrayIntCap<std::is_nothrow_move_constructible<std::string>::value ? 2 : 0,	// gcc 4.9
			std::string>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::SegmentedArray<std::string>: " << std::flush;
		TestStrArray<momo::SegmentedArray<std::string>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::SegmentedArraySqrt<std::string>: " << std::flush;
		TestStrArray<momo::SegmentedArraySqrt<std::string>>();
		std::cout << "ok" << std::endl;
	}

	template<typename Array>
	static void TestStrArray()
	{
		Array ar = Array::CreateCrt(1, [] (void* ptr) { ::new(ptr) std::string(); });

		std::string s1 = "s1";
		ar.AddBack(s1);
		ar.AddBack("s2");
		ar.AddBackCrt([] (void* ptr) { ::new(ptr) std::string("s3"); });

		ar.Reserve(10);
		assert(ar.GetCapacity() >= 10);

		//assert(ar.Contains("s3"));
		//assert(!ar.Contains("s"));

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
		ar.Shrink();

		ar.AddBack(std::move(s1));
		ar.Insert(0, 3, ar[0]);
		assert(ar.GetCount() == 4);

		Array ar2;
		ar2 = ar;
		for (std::string& s : ar2)
			assert(s == "s1");

		ar2.Clear(true);
		assert(ar2.IsEmpty());
	}

	static void TestTemplAll()
	{
		std::cout << "momo::Array<size_t, momo::MemManagerCpp>: " << std::flush;
		TestTemplArray<momo::Array<size_t, momo::MemManagerCpp>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::Array<size_t, momo::MemManagerC>: " << std::flush;
		TestTemplArray<momo::Array<size_t, momo::MemManagerC>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::Array<TemplItem<false>, momo::MemManagerCpp>: " << std::flush;
		TestTemplArray<momo::Array<TemplItem<false>, momo::MemManagerCpp>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::Array<TemplItem<true>, momo::MemManagerC>: " << std::flush;
		TestTemplArray<momo::Array<TemplItem<true>, momo::MemManagerC>>();
		std::cout << "ok" << std::endl;

#ifdef MOMO_USE_MEM_MANAGER_WIN
		std::cout << "momo::Array<size_t, momo::MemManagerWin>: " << std::flush;
		TestTemplArray<momo::Array<size_t, momo::MemManagerWin>>();
		std::cout << "ok" << std::endl;
#endif

		std::cout << "momo::ArrayIntCap<4, size_t>: " << std::flush;
		TestTemplArray<momo::ArrayIntCap<4, size_t>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::SegmentedArray<size_t>: " << std::flush;
		TestTemplArray<momo::SegmentedArray<size_t>>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::SegmentedArraySqrt<size_t>: " << std::flush;
		TestTemplArray<momo::SegmentedArraySqrt<size_t>>();
		std::cout << "ok" << std::endl;
	}

	template<typename Array>
	static void TestTemplArray()
	{
		typedef typename Array::Item Item;

		Array ar;
		static const size_t count = 20000;

		for (size_t i = 0; i < count; ++i)
			ar.AddBack(Item(i));

		ar.Shrink();

		for (size_t i = 0; i < count; ++i)
			assert(ar[i] == Item(i));
	}
};

static int testSimpleArray = (SimpleArrayTester::TestStrAll(), SimpleArrayTester::TestTemplAll(), 0);

#endif // TEST_SIMPLE_ARRAY
