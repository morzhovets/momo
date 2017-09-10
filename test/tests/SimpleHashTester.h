/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTester.h

\**********************************************************/

#include "../../momo/HashSet.h"
#include "../../momo/HashMap.h"
#include "../../momo/HashMultiMap.h"

#include <string>
#include <iostream>
#include <random>

class SimpleHashTester
{
private:
	template<size_t size, size_t alignment>
	class TemplItem
	{
	public:
		template<typename HashBucket>
		class HashTraits : public momo::HashTraits<TemplItem, HashBucket>
		{
		public:
			size_t GetHashCode(const TemplItem& key) const
			{
				return std::hash<unsigned char>()(key.GetValue());
			}

			bool IsEqual(const TemplItem& key1, const TemplItem& key2) const
			{
				return key1.GetValue() == key2.GetValue();
			}
		};

	public:
		explicit TemplItem(unsigned char value) MOMO_NOEXCEPT
		{
			*pvGetPtr() = value;
		}

		unsigned char GetValue() const MOMO_NOEXCEPT
		{
			return *pvGetPtr();
		}

	private:
		const unsigned char* pvGetPtr() const MOMO_NOEXCEPT
		{
			return reinterpret_cast<const unsigned char*>(&mStorage);
		}

		unsigned char* pvGetPtr() MOMO_NOEXCEPT
		{
			return reinterpret_cast<unsigned char*>(&mStorage);
		}

	private:
		typename std::aligned_storage<size, alignment>::type mStorage;
	};

public:
	template<typename HashBucket, size_t size, size_t alignment>
	static void TestTemplHashSet(const char* bucketName)
	{
		std::cout << bucketName << ": TemplItem<" << size << ", " << alignment << ">: " << std::flush;

		static const size_t count = 256;
		static unsigned char array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = (unsigned char)i;

		std::mt19937 mt;

		typedef TemplItem<size, alignment> Item;
		typedef momo::HashSet<Item, typename Item::template HashTraits<HashBucket>> HashSet;
		HashSet set;

		std::shuffle(array, array + count, mt);
		for (unsigned char c : array)
			assert(set.Insert(Item(c)).inserted);
		assert(set.GetCount() == count);

		std::shuffle(array, array + count, mt);
		for (unsigned char c : array)
			assert(set.Remove(Item(c)));
		assert(set.IsEmpty());

		std::cout << "ok" << std::endl;
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
		std::string s1 = "s1";
		HashSet set = { s1, "s2" };
		set.Insert("s3");
		set = set;
		set = std::move(set);
		assert(set.GetCount() == 3);
		assert(set.HasKey("s2"));
		typename HashSet::ConstIterator iter = set.Find("s1");
		assert(*iter == "s1");
		auto es = set.Extract(iter);
		assert(es.GetItem() == "s1");
		set.Remove(set.Add(set.Find(s1), std::move(es)), es);
		assert(es.GetItem() == s1);
		set.Insert(std::move(es));
		assert(es.IsEmpty());
		set.Remove(set.Find("s1"));
		iter = set.Find("s1");
		assert(iter == set.GetEnd());
		set.Insert(&s1, &s1 + 1);
		set.ResetKey(set.Find("s1"), s1);
		set.ResetKey(set.Find("s2"), "s2");
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
		std::string s1 = "s1";
		std::string s2 = "s2";
		std::string s3 = "s3";
		std::string s4 = "s4";
		std::string s5 = "s5";
		HashMap map = { {"s1", "s1"}, {"s2", s2} };
		map.Insert(s3, "s3");
		map.Insert(s4, s4);
		map[s5] = "s5";
		assert((std::string)map["s5"] == s5);
		map["s6"] = "s6";
		map.ResetKey(map.Find("s1"), s1);
		map.ResetKey(map.Find(s2), "s2");
		map = map;
		map = std::move(map);
		assert(map.GetCount() == 6);
		assert(map.HasKey(s2));
		typename HashMap::ConstIterator iter1 = map.Find(s1);
		assert(iter1->key == s1 && iter1->value == s1);
		map.Remove(s1);
		typename HashMap::Iterator iter2 = map.Find("s5");
		assert(iter2->key == s5 && iter2->value == s5);
		auto ep = map.Extract(iter2);
		assert(ep.GetKey() == s5 && ep.GetValue() == s5);
		map.Remove(map.Add(map.Find(s5), std::move(ep)), ep);
		assert(ep.GetKey() == s5 && ep.GetValue() == s5);
		map.Insert(std::move(ep));
		assert(ep.IsEmpty());
		map.Remove(map.Find("s5"));
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
			momo::HashTraitsVar<std::string, HashBucket>> HashMultiMap;
		std::string k1 = "k1";
		std::string v1 = "v1";
		std::string k2 = "k2";
		std::string v2 = "v2";
		std::string k3 = "k3";
		std::string v3 = "v3";
		HashMultiMap mmap({ {"k1", "v1"}, {k1, "v2"} }, typename HashMultiMap::HashTraits(1));
		mmap.Add("k2", v1);
		mmap.Add(k2, v2);
		mmap.Add(mmap.InsertKey(k3), "v3");
		mmap.Add(mmap.InsertKey("k3"), v3);
		mmap.ResetKey(mmap.Find("k1"), k1);
		mmap.ResetKey(mmap.Find(k2), "k2");
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
