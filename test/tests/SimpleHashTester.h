/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

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
			static const bool isFastNothrowHashable = std::is_same<HashBucket, momo::HashBucketOpen8>::value;	//?

		public:
			size_t GetHashCode(const TemplItem& /*key*/) const
			{
				return 42; //std::hash<unsigned char>()(key.GetValue());
			}

			bool IsEqual(const TemplItem& key1, const TemplItem& key2) const
			{
				return key1.GetValue() == key2.GetValue();
			}
		};

	public:
		explicit TemplItem(unsigned char value) noexcept
		{
			*pvGetPtr() = value;
		}

		unsigned char GetValue() const noexcept
		{
			return *pvGetPtr();
		}

	private:
		const unsigned char* pvGetPtr() const noexcept
		{
			return reinterpret_cast<const unsigned char*>(&mStorage);
		}

		unsigned char* pvGetPtr() noexcept
		{
			return reinterpret_cast<unsigned char*>(&mStorage);
		}

	private:
		typename std::aligned_storage<size, alignment>::type mStorage;
	};

public:
	template<typename HashBucket>
	static void TestStrHash(const char* bucketName)
	{
		typedef momo::HashTraits<std::string, HashBucket> HashTraits;

		std::cout << bucketName << ": HashSet: " << std::flush;
		TestStrHashSet<HashTraits>();
		std::cout << "ok" << std::endl;

		std::cout << bucketName << ": HashMap: " << std::flush;
		TestStrHashMap<HashTraits>();
		std::cout << "ok" << std::endl;

		std::cout << bucketName << ": HashMultiMap: " << std::flush;
		TestStrHashMultiMap<HashTraits>();
		std::cout << "ok" << std::endl;
	}

	template<typename HashTraits>
	static void TestStrHashSet()
	{
		typedef momo::HashSet<std::string, HashTraits> HashSet;

		std::string s1 = "s1";
		std::string s2 = "s2";
		std::string s3 = "s3";

		HashSet set;
		set.InsertVar(s1, "s1");
		set.Add(set.Find("s2"), s2);
		set.Add(set.Find(s3), "s3");

		set.ResetKey(set.Find(s2), "s2");
		set.ResetKey(set.Find("s3"), s3);

		assert(set.GetCount() == 3);
		assert(set.ContainsKey(s2));
		assert(set.ContainsKey("s3"));

		auto es = set.Extract(set.Find("s1"));
		assert(es.GetItem() == s1);

		assert(set.GetCount() == 2);
		set.Add(set.Find(s1), std::move(es));
		assert(set.GetCount() == 3);

		HashSet set2;
		set2 = set;
		assert(set.GetCount() == set2.GetCount());

		set.Clear(false);
		assert(set.IsEmpty());
		assert(set.GetCapacity() >= 3);
		set.Shrink();
		assert(set.GetCapacity() == 0);
	}

	template<typename HashTraits>
	static void TestStrHashMap()
	{
		typedef momo::HashMap<std::string, std::string, HashTraits> HashMap;

		std::string s1 = "s1";
		std::string s2 = "s2";
		std::string s3 = "s3";
		std::string s4 = "s4";
		std::string s5 = "s5";

		HashMap map = { { "s1", s1 } };
		map.Add(map.Find(s2), "s2", "s2");
		map.Add(map.Find(s3), "s3", s3);
		map.Add(map.Find("s4"), s4, "s4");
		map.Add(map.Find("s5"), s5, s5);

		map.Insert("s1", "s2");
		map.Insert("s2", s3);
		map.Insert(s3, "s4");
		map.Insert(s4, s1);

		map.ResetKey(map.Find(s5), "s5");

		auto ep = map.Extract(map.Find(s1));
		assert(ep.GetKey() == s1 && ep.GetValue() == s1);
		assert(map.GetCount() == 4);

		map.Add(map.Find("s1"), std::move(ep));
		assert(map.GetCount() == 5);

		for (auto ref : map)
			assert(ref.key == ref.value);

		HashMap map2;
		map2 = map;
		assert(map.GetCount() == map2.GetCount());

		map.Clear(false);
		assert(map.IsEmpty());
		assert(map.GetCapacity() >= 3);
		map.Shrink();
		assert(map.GetCapacity() == 0);
	}

	template<typename HashTraits>
	static void TestStrHashMultiMap()
	{
		typedef momo::HashMultiMap<std::string, std::string, HashTraits> HashMultiMap;
		std::string k1 = "k1";
		std::string v1 = "v1";
		std::string k2 = "k2";
		std::string v2 = "v2";
		std::string k3 = "k3";
		std::string v3 = "v3";
		HashMultiMap mmap = { {"k1", "v1"}, {k1, "v2"} };
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
		assert(mmap.ContainsKey(k2));
		mmap.RemoveKey(k1);
		auto keyIter = mmap.Find(k2);
		mmap.Remove(keyIter, 0);
		assert(keyIter->GetCount() == 1);
		for (const std::string& v : *keyIter)
			assert(v == v2);
		for (auto ref : mmap.GetKeyBounds())
			assert(ref.key == k2 || ref.key == k3);
		for (auto ref : (static_cast<const HashMultiMap&>(mmap)).GetKeyBounds())
			assert(ref.key == k2 || ref.key == k3);
		mmap.RemoveValues(keyIter);
		mmap.RemoveKey(keyIter);
		mmap.Shrink();
		std::pair<std::string, std::string> pair("k3", v3);
		mmap.Add(&pair, &pair + 1);
		mmap.Add(mmap.GetBegin(), mmap.GetBegin());	//?
		for (auto ref : mmap)
			assert(ref.key == "k3");
		for (auto ref : static_cast<const HashMultiMap&>(mmap))
			assert(ref.value == "v3");
		assert(mmap.GetKeyCount() == 1);
		assert(mmap.GetValueCount() == 3);
		mmap.Clear();
		assert(mmap.GetKeyCount() == 0);
		assert(mmap.GetValueCount() == 0);
	}

	template<typename HashBucket, size_t size, size_t alignment>
	static void TestTemplHashSet(const char* bucketName)
	{
		std::cout << bucketName << ": TemplItem<" << size << ", " << alignment << ">: " << std::flush;

		static const size_t count = 256;
		static unsigned char array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = static_cast<unsigned char>(i);

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
		{
			auto pos = set.Find(Item(c));
			assert(!!pos);
			set.Remove(set.Insert(set.Extract(pos)).iterator);
		}
		assert(set.IsEmpty());

		std::cout << "ok" << std::endl;
	}
};
