/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleTreeTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_TREE

#undef NDEBUG

#include "../../momo/TreeSet.h"
#include "../../momo/TreeMap.h"

#include <string>
#include <iostream>

class SimpleTreeTester
{
public:
	static void TestAll()
	{
		std::cout << "momo::TreeSet: " << std::flush;
		TestStrTreeSet();
		std::cout << "ok" << std::endl;

		std::cout << "momo::TreeMap: " << std::flush;
		TestStrTreeMap();
		std::cout << "ok" << std::endl;
	}

	static void TestStrTreeSet()
	{
		typedef momo::TreeSet<std::string> TreeSet;
		std::string s1 = "s1";
		TreeSet set = { s1, "s2" };
		set.Insert("s3");
		set = set;
		set = std::move(set);
		assert(set.GetCount() == 3);
		assert(set.HasKey("s2"));
		TreeSet::ConstIterator iter = set.Find("s1");
		assert(*iter == "s1");
		auto rs = set.Extract(iter);
		assert(rs.GetItem() == "s1");
		set.Add(set.LowerBound(s1), std::move(rs));
		set.Remove(set.Find(s1), rs);
		assert(rs.GetItem() == s1);
		set.Insert(std::move(rs));
		set.Remove(set.Find("s1"));
		iter = set.Find("s1");
		assert(iter == set.GetEnd());
		set.Insert(&s1, &s1 + 1);
		set.ResetKey(set.LowerBound("s1"), s1);
		set.ResetKey(set.UpperBound(s1), "s2");
		set.Remove("s2");
		for (const std::string& s : set)
			assert(s == "s1" || s == "s3");
		set.Clear();
		assert(set.IsEmpty());
	}

	static void TestStrTreeMap()
	{
		typedef momo::TreeMap<std::string, std::string> TreeMap;
		std::string s1 = "s1";
		std::string s2 = "s2";
		std::string s3 = "s3";
		std::string s4 = "s4";
		std::string s5 = "s5";
		TreeMap map = { {"s1", "s1"}, {"s2", s2} };
		map.Insert(s3, "s3");
		map.Insert(s4, s4);
		map[s5] = "s5";
		assert((std::string)map["s5"] == s5);
		map["s6"] = "s6";
		map.ResetKey(map.LowerBound("s1"), s1);
		map.ResetKey(map.UpperBound(s1), "s2");
		map = map;
		map = std::move(map);
		assert(map.GetCount() == 6);
		assert(map.HasKey(s2));
		TreeMap::ConstIterator iter1 = map.Find(s1);
		assert(iter1->key == s1 && iter1->value == s1);
		map.Remove(s1);
		TreeMap::Iterator iter2 = map.Find("s5");
		assert(iter2->key == s5 && iter2->value == s5);
		auto rp = map.Extract(iter2);
		assert(rp.GetKey() == s5 && rp.GetValue() == s5);
		map.Remove(s3);
		map.Remove("s4");
		std::pair<std::string, std::string> pair("s4", s4);
		map.InsertFS(&pair, &pair + 1);
		map.InsertKV(map.Find(s2), std::next(map.Find(s2)));	//?
		assert(map.GetCount() == 3);
		map.Remove(s4);
		for (auto ref : map)
			assert(ref.value == "s2" || ref.value == "s6");
		for (auto ref : (const TreeMap&)map)
			assert(ref.value == "s2" || ref.value == "s6");
		assert(map.GetCount() == 2);
		map.Clear();
		assert(map.IsEmpty());
	}
};

static int testSimpleTree = (SimpleTreeTester::TestAll(), 0);

#endif // TEST_SIMPLE_TREE
