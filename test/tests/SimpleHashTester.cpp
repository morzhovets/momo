/**********************************************************\

  tests/SimpleHashTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "../../momo/HashSet.h"
#include "../../momo/HashMap.h"
#include "../../momo/HashMultiMap.h"
#include "../../momo/HashBuckets/BucketLim4.h"

#include <string>
#include <iostream>
#include <type_traits>
#include <random>

class SimpleHashTester
{
private:
	template<size_t size, size_t alignment>
	class VarItem
	{
	public:
		template<typename HashBucket>
		class HashTraits : public momo::HashTraits<VarItem, HashBucket>
		{
		public:
			size_t GetHashCode(const VarItem& key) const
			{
				return std::hash<unsigned char>()(key.GetValue());
			}

			bool IsEqual(const VarItem& key1, const VarItem& key2) const
			{
				return key1.GetValue() == key2.GetValue();
			}
		};

	public:
		explicit VarItem(unsigned char value) MOMO_NOEXCEPT
		{
			*(unsigned char*)&mStorage = value;
		}

		unsigned char GetValue() const MOMO_NOEXCEPT
		{
			return *(unsigned char*)&mStorage;
		}

	private:
		typename std::aligned_storage<size, alignment>::type mStorage;
	};

public:
	static void TestAll()
	{
		TestVarAll();
		TestStrAll();
	}

	static void TestVarAll()
	{
		TestVarHash<momo::HashBucketOneI1>("momo::HashBucketOneI1");

		TestVarHash<momo::HashBucketLimP1<1, 1, 32>>("momo::HashBucketLimP1<1, 1, 32>");
		TestVarHash<momo::HashBucketLimP1<2, 1, 1>>("momo::HashBucketLimP1<2, 1, 1>");
		TestVarHash<momo::HashBucketLimP1<3, 1, 2>>("momo::HashBucketLimP1<3, 1, 2>");
		TestVarHash<momo::HashBucketLimP1<4, 1, 4>>("momo::HashBucketLimP1<4, 1, 4>");
		TestVarHash<momo::HashBucketLimP1<5, 1, 64>>("momo::HashBucketLimP1<5, 1, 64>");
		TestVarHash<momo::HashBucketLimP1<7, 1, 3>>("momo::HashBucketLimP1<7, 1, 3>");
		TestVarHash<momo::HashBucketLimP1<10, 1, 127>>("momo::HashBucketLimP1<10, 1, 127>");
		TestVarHash<momo::HashBucketLimP1<15, 1, 1>>("momo::HashBucketLimP1<15, 1, 1>");

		TestVarHash<momo::HashBucketLimP<1, 32>>("momo::HashBucketLimP<1, 32>");
		TestVarHash<momo::HashBucketLimP<2, 1>>("momo::HashBucketLimP<2, 1>");
		TestVarHash<momo::HashBucketLimP<3, 2>>("momo::HashBucketLimP<3, 2>");
		TestVarHash<momo::HashBucketLimP<4, 4>>("momo::HashBucketLimP<4, 4>");
		TestVarHash<momo::HashBucketLimP<5, 64>>("momo::HashBucketLimP<5, 64>");
		TestVarHash<momo::HashBucketLimP<7, 3>>("momo::HashBucketLimP<7, 3>");
		TestVarHash<momo::HashBucketLimP<10, 127>>("momo::HashBucketLimP<10, 127>");
		TestVarHash<momo::HashBucketLimP<15, 1>>("momo::HashBucketLimP<15, 1>");

		TestVarHash<momo::HashBucketUnlimP<1, 32>>("momo::HashBucketUnlimP<1, 32>");
		TestVarHash<momo::HashBucketUnlimP<2, 1>>("momo::HashBucketUnlimP<2, 1>");
		TestVarHash<momo::HashBucketUnlimP<3, 2>>("momo::HashBucketUnlimP<3, 2>");
		TestVarHash<momo::HashBucketUnlimP<4, 4>>("momo::HashBucketUnlimP<4, 4>");
		TestVarHash<momo::HashBucketUnlimP<5, 64>>("momo::HashBucketUnlimP<5, 64>");
		TestVarHash<momo::HashBucketUnlimP<7, 3>>("momo::HashBucketUnlimP<7, 3>");
		TestVarHash<momo::HashBucketUnlimP<10, 127>>("momo::HashBucketUnlimP<10, 127>");
		TestVarHash<momo::HashBucketUnlimP<15, 1>>("momo::HashBucketUnlimP<15, 1>");

		TestVarHash<momo::HashBucketLim4<1, 32>>("momo::HashBucketLim4<1, 32>");
		TestVarHash<momo::HashBucketLim4<2, 1>>("momo::HashBucketLim4<2, 1>");
		TestVarHash<momo::HashBucketLim4<3, 127>>("momo::HashBucketLim4<3, 127>");
		TestVarHash<momo::HashBucketLim4<7, 2>>("momo::HashBucketLim4<7, 2>");
	}

	template<typename HashBucket>
	static void TestVarHash(const char* bucketName)
	{
		TestVarHashSet<HashBucket, VarItem<1, 1>>(bucketName, "VarItem<1, 1>");
		TestVarHashSet<HashBucket, VarItem<2, 1>>(bucketName, "VarItem<2, 1>");
	}

	template<typename HashBucket, typename VarItem>
	static void TestVarHashSet(const char* bucketName, const char* varItemName)
	{
		std::cout << bucketName << ": " << varItemName << ": " << std::flush;

		static const size_t count = 256;
		static unsigned char array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = (unsigned char)i;

		typedef momo::HashSet<VarItem, typename VarItem::template HashTraits<HashBucket>> HashSet;
		HashSet set;

		std::shuffle(array, array + count, std::mt19937());
		for (unsigned char c : array)
			assert(set.Insert(VarItem(c)).inserted);
		assert(set.GetCount() == count);

		std::shuffle(array, array + count, std::mt19937());
		for (unsigned char c : array)
			assert(set.Remove(VarItem(c)));
		assert(set.IsEmpty());

		std::cout << "ok" << std::endl;
	}

	static void TestStrAll()
	{
		TestStrHash<momo::HashBucketOneI1>("momo::HashBucketOneI1");
		TestStrHash<momo::HashBucketLimP1<>>("momo::HashBucketLimP1<>");
		TestStrHash<momo::HashBucketLimP<>>("momo::HashBucketLimP<>");
		TestStrHash<momo::HashBucketUnlimP<>>("momo::HashBucketUnlimP<>");
		TestStrHash<momo::HashBucketLim4<>>("momo::HashBucketLim4<>");
	}

	template<typename HashBucket>
	static void TestStrHash(const char* bucketName)
	{
		std::cout << bucketName << ": HashSet: " << std::flush;
		TestStrHashSet<HashBucket>();
		std::cout << "ok" << std::endl;

		std::cout << bucketName << ": HashMap: " << std::flush;
		TestStrHashMap<HashBucket>();
		std::cout << "ok" << std::endl;

		std::cout << bucketName << ": HashMultiMap: " << std::flush;
		TestStrHashMultiMap<HashBucket>();
		std::cout << "ok" << std::endl;
	}

	template<typename HashBucket>
	static void TestStrHashSet()
	{
		typedef momo::HashSet<std::string,
			momo::HashTraits<std::string, HashBucket>> HashSet;
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

	template<typename HashBucket>
	static void TestStrHashMap()
	{
		typedef momo::HashMap<std::string, std::string,
			momo::HashTraits<std::string, HashBucket>> HashMap;
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

	template<typename HashBucket>
	static void TestStrHashMultiMap()
	{
		typedef momo::HashMultiMap<std::string, std::string,
			momo::HashTraits<std::string, HashBucket>> HashMultiMap;
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
