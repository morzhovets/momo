/**********************************************************\

  tests/SimpleHashTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "../../momo/HashSet.h"
#include "../../momo/HashMap.h"
#include "../../momo/HashMultiMap.h"
#include "../../momo/HashBuckets/BucketFewP.h"
#include "../../momo/HashBuckets/BucketLim4.h"

#include <string>
#include <iostream>

class SimpleHashTester
{
public:
	static void TestAll()
	{
		TestHashBucket<momo::HashBucketFewP<>>("momo::HashBucketFewP");
		TestHashBucket<momo::HashBucketLim4<>>("momo::HashBucketLim4");
		TestHashBucket<momo::HashBucketLimP<>>("momo::HashBucketLimP");
		TestHashBucket<momo::HashBucketLimP1<>>("momo::HashBucketLimP1");
		TestHashBucket<momo::HashBucketOneI1>("momo::HashBucketOneI1");
		TestHashBucket<momo::HashBucketUnlimP<>>("momo::HashBucketUnlimP");
	}

	template<typename HashBucket>
	static void TestHashBucket(const char* bucketName)
	{
		typedef momo::HashTraits<std::string, HashBucket> HashTraits;

		std::cout << bucketName << ": HashSet<std::string>: " << std::flush;
		typedef momo::HashSet<std::string, HashTraits> HashSet;
		TestHashSet<HashSet>();
		std::cout << "ok" << std::endl;

		std::cout << bucketName << ": HashMap<std::string, std::string>: " << std::flush;
		typedef momo::HashMap<std::string, std::string, HashTraits> HashMap;
		TestHashMap<HashMap>();
		std::cout << "ok" << std::endl;

		std::cout << bucketName << ": HashMultiMap<std::string, std::string>: " << std::flush;
		typedef momo::HashMultiMap<std::string, std::string, HashTraits> HashMultiMap;
		TestHashMultiMap<HashMultiMap>();
		std::cout << "ok" << std::endl;
	}

	template<typename HashSet>
	static void TestHashSet()
	{
		HashSet set;
		std::string s1 = "s1";
		set.Insert(s1);
		set.Insert("s2");
		set.Insert("s3");
		set = set;
		set = std::move(set);
		assert(set.GetCount() == 3);
		assert(set.HasKey("s2"));
		typename HashSet::ConstIterator iter = set.Find("s1");
		assert(*iter == "s1");
		std::string rs;
		set.Remove(iter, rs);
		assert(rs == "s1");
		iter = set.Find("s1");
		assert(iter == set.GetEnd());
		set.Insert(&s1, &s1 + 1);
		set.Reset(set.Find("s1"), s1);
		set.Reset(set.Find(s1), "s1", rs);
		assert(rs == "s1");
		set.Reset(set.Find("s2"), "s2");
		set.Reset(set.Find("s2"), "s2", rs);
		assert(rs == "s2");
		set.Remove("s2");
		set.Reserve(100);
		assert(set.GetCapacity() >= 100);
		set.Shrink();
		for (const std::string& s : set)
			assert(s == "s1" || s == "s3");
		set.Clear();
		assert(set.IsEmpty());
	}

	template<typename HashMap>
	static void TestHashMap()
	{
		HashMap map;
		std::string s1 = "s1";
		std::string s2 = "s2";
		std::string s3 = "s3";
		std::string s4 = "s4";
		std::string s5 = "s5";
		map.Insert("s1", "s1");
		map.Insert("s2", s2);
		map.Insert(s3, "s3");
		map.Insert(s4, s4);
		map[s5] = "s5";
		assert((std::string)map["s5"] == s5);
		map["s6"] = "s6";
		map = map;
		map = std::move(map);
		assert(map.GetCount() == 6);
		assert(map.HasKey(s2));
		typename HashMap::ConstIterator iter1 = map.Find(s1);
		assert(iter1->key == s1 && iter1->value == s1);
		map.Remove(s1);
		typename HashMap::Iterator iter2 = map.Find("s5");
		assert(iter2->key == s5 && iter2->value == s5);
		map.Remove(iter2);
		map.Remove(s3);
		map.Remove("s4");
		map.Reserve(100);
		assert(map.GetCapacity() >= 100);
		map.Shrink();
		std::pair<std::string, std::string> pair("s4", s4);
		map.InsertFS(&pair, &pair + 1);
		map.InsertKV(map.Find(s2), std::next(map.Find(s2)));	//?
		assert(map.GetCount() == 3);
		map.Remove(s4);
		for (auto ref : map)
			assert(ref.value == "s2" || ref.value == "s6");
		for (auto ref : (const HashMap&)map)
			assert(ref.value == "s2" || ref.value == "s6");
		assert(map.GetCount() == 2);
		map.Clear();
		assert(map.IsEmpty());
	}

	template<typename HashMultiMap>
	static void TestHashMultiMap()
	{
		HashMultiMap mmap(typename HashMultiMap::HashTraits(1));
		std::string k1 = "k1";
		std::string v1 = "v1";
		std::string k2 = "k2";
		std::string v2 = "v2";
		std::string k3 = "k3";
		std::string v3 = "v3";
		mmap.Add("k1", "v1");
		mmap.Add(k1, "v2");
		mmap.Add("k2", v1);
		mmap.Add(k2, v2);
		mmap.Add(mmap.InsertKey(k3), "v3");
		mmap.Add(mmap.InsertKey("k3"), v3);
		mmap = mmap;
		mmap = std::move(mmap);
		assert(mmap.GetKeyCount() == 3);
		assert(mmap.GetValueCount() == 6);
		assert(mmap.HasKey(k2));
		mmap.RemoveKey(k1);
		auto keyIter = mmap.Find(k2);
		mmap.Remove(keyIter, 0);
		assert(keyIter->values.GetCount() == 1);
		for (const std::string& v : keyIter->values)
			assert(v == v2);
		for (auto ref : mmap.GetKeyBounds())
			assert(ref.key == k2 || ref.key == k3);
		for (auto ref : ((const HashMultiMap&)mmap).GetKeyBounds())
			assert(ref.key == k2 || ref.key == k3);
		mmap.RemoveValues(keyIter);
		mmap.RemoveKey(keyIter);
		mmap.Shrink();
		std::pair<std::string, std::string> pair("k3", v3);
		mmap.AddFS(&pair, &pair + 1);
		mmap.AddKV(mmap.GetBegin(), mmap.GetBegin());	//?
		for (auto ref : mmap)
			assert(ref.key == "k3");
		for (auto ref : (const HashMultiMap&)mmap)
			assert(ref.value == "v3");
		assert(mmap.GetKeyCount() == 1);
		assert(mmap.GetValueCount() == 3);
		mmap.Clear();
		assert(mmap.GetKeyCount() == 0);
		assert(mmap.GetValueCount() == 0);
	}
};

static int testSimpleHash = (SimpleHashTester::TestAll(), 0);

#endif // TEST_SIMPLE_HASH
