/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleTreeTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_TREE

#include "../../momo/TreeSet.h"
#include "../../momo/TreeMap.h"
#include "../../momo/stdish/pool_allocator.h"

#include <string>
#include <iostream>
#include <random>
#include <map>

class SimpleTreeTester
{
private:
	template<bool isNothrowMoveConstructible, bool isNothrowSwappable,
		bool isNothrowCopyAssignable = false>
	class TemplItem
	{
	public:
		explicit TemplItem(uint8_t value = uint8_t{0}) noexcept
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

		~TemplItem() noexcept = default;

		//TemplItem& operator=(TemplItem&& item) = delete;

		TemplItem& operator=(const TemplItem& item) noexcept(isNothrowCopyAssignable)
		{
			mValue = item.mValue;
			return *this;
		}

		friend void swap(TemplItem& item1, TemplItem& item2) noexcept(isNothrowSwappable)
		{
			std::swap(item1.mValue, item2.mValue);
		}

		bool operator==(const TemplItem& item) const noexcept
		{
			return mValue == item.mValue;
		}

		bool operator<(const TemplItem& item) const noexcept
		{
			return mValue < item.mValue;
		}

	private:
		uint8_t mValue;
	};

public:
	static void TestStrAll()
	{
		std::cout << "momo::TreeSet: " << std::flush;
		TestStrTreeSet();
		std::cout << "ok" << std::endl;

		std::cout << "momo::TreeMap (-useValuePtr): " << std::flush;
		TestStrTreeMap<false>();
		std::cout << "ok" << std::endl;

		std::cout << "momo::TreeMap (+useValuePtr): " << std::flush;
		TestStrTreeMap<true>();
		std::cout << "ok" << std::endl;
	}

	static void TestStrTreeSet()
	{
		typedef momo::TreeSet<std::string> TreeSet;

		std::string s1 = "s1";
		std::string s2 = "s2";

		TreeSet set = { s2, "s3" };
		set.InsertVar(s1, "s1");

		assert(set.GetCount() == 3);
		assert(set.ContainsKey(s1));
		assert(set.ContainsKey("s2"));

		set.ResetKey(set.GetLowerBound("s1"), s1);
		set.ResetKey(set.GetUpperBound(s1), "s2");

		const auto es = set.Extract(set.Find("s1"));
		assert(es.GetItem() == s1);

		assert(set.GetCount() == 2);
		assert(*set.GetBegin() == s2);
		assert(*std::prev(set.GetEnd()) == "s3");

		TreeSet set2;
		set2 = set;
		assert(std::equal(set.GetBegin(), set.GetEnd(), set2.GetBegin()));
	}

	template<bool useValuePtr>
	static void TestStrTreeMap()
	{
		typedef momo::TreeMap<std::string, std::string, momo::TreeTraits<std::string>,
			momo::MemManagerDefault, momo::TreeMapKeyValueTraits<std::string, std::string,
			momo::MemManagerDefault, useValuePtr>> TreeMap;

		auto pred = [] (auto ref1, auto ref2)
			{ return ref1.key == ref2.key && ref1.value == ref2.value; };

		std::string s1 = "s1";
		std::string s2 = "s2";
		std::string s3 = "s3";
		std::string s4 = "s4";

		TreeMap map = { { "s1", "s1" }, { "s2", s2 } };
		map[s3] = "s3";
		map.InsertCrt(s4, [] (std::string* ptr) { std::construct_at(ptr, "s4"); });

		map.Insert("s1", "s2");
		map.Insert("s2", s3);
		map.Insert(s3, "s4");
		map.Insert(s4, s1);

		map.ResetKey(map.GetLowerBound("s1"), s1);
		map.ResetKey(map.GetUpperBound(s1), "s2");

		{
			const auto& ms1 = map[s1];
			map[s4] = ms1;
			assert(s1.compare(map[s4]) == 0);

			auto ms2 = map[s2];
			map[s4] = *&ms2;
			assert(s2 == *&map[s4]);

			map[s4] = map[s3];
			assert(*&map[s4] == s3);

			(*&map[s4]).clear();
			*&map[s4] += s4;
		}

		assert(map.GetCount() == 4);
		for (auto ref : map)
			assert(ref.key == ref.value);

		TreeMap map2;
		map2 = map;

		auto ep = map.Extract(map.Find("s1"));
		assert(std::as_const(ep).GetKey() == s1 && std::as_const(ep).GetValue() == s1);
		assert(map.GetCount() == 3);

		auto pairRemover = [] (std::string& key, std::string& value)
		{
			assert(key == "s1" && value == "s1");
			std::destroy_at(&key);
			std::destroy_at(&value);
		};
		ep.Remove(pairRemover);
		assert(ep.IsEmpty());
		ep.Clear();

		map.Insert(map2.GetBegin(), std::next(map2.GetBegin()));
		assert(std::equal(map.GetBegin(), map.GetEnd(), map2.GetBegin(), pred));

		map.Clear();
		assert(map.IsEmpty());

		map.Add(map.GetLowerBound("s1"), s1, s1);
		map.Add(map.GetUpperBound("s2"), s2, "s2");
		map.Add(map.GetLowerBound(s3), "s3", s3);
		map.Add(map.GetUpperBound(s4), "s4", "s4");
		assert(std::equal(map.GetBegin(), map.GetEnd(), map2.GetBegin(), pred));
	}

	static void TestTemplAll()
	{
		std::mt19937 mt;

		TestTemplTreeNode<  1, 1, 127, 3, 3>(mt);
		TestTemplTreeNode<  2, 1,  66, 3, 2>(mt);
		TestTemplTreeNode<  3, 1,  32, 2, 3>(mt);
		TestTemplTreeNode<  4, 1,  15, 2, 2>(mt);
		TestTemplTreeNode<  5, 1,   1, 1, 1>(mt);
		TestTemplTreeNode< 10, 1,   3, 1, 0>(mt);
		TestTemplTreeNode<101, 1,   2, 0, 1>(mt);
		TestTemplTreeNode<255, 1,   1, 0, 0>(mt);

		TestTemplTreeNode<  4, 2, 127, 1, 3>(mt);
		TestTemplTreeNode<  5, 2,  66, 3, 1>(mt);
		TestTemplTreeNode<  6, 2,  32, 1, 2>(mt);
		TestTemplTreeNode<  7, 2,  15, 2, 1>(mt);
		TestTemplTreeNode< 14, 3,   1, 0, 3>(mt);
		TestTemplTreeNode< 77, 3,   3, 3, 0>(mt);
		TestTemplTreeNode<121, 3,   2, 0, 2>(mt);
		TestTemplTreeNode<255, 3,   1, 2, 0>(mt);

		TestTemplTreeNode< 37,   7, 127, 3, 3>(mt);
		TestTemplTreeNode< 42,  15,  66, 3, 3>(mt);
		TestTemplTreeNode< 65,  23,  32, 3, 3>(mt);
		TestTemplTreeNode< 77,  30,  15, 3, 3>(mt);
		TestTemplTreeNode< 88,  31,   1, 3, 3>(mt);
		TestTemplTreeNode<104,  33,   3, 3, 3>(mt);
		TestTemplTreeNode<204, 100,   2, 3, 3>(mt);
		TestTemplTreeNode<255,   0,   1, 3, 3>(mt);
	}

	template<size_t maxCapacity, size_t capacityStep, size_t memPoolBlockCount,
		size_t keyTraits, size_t valueTraits>
	static void TestTemplTreeNode(std::mt19937& mt)
	{
		std::cout << "momo::TreeNode<" << maxCapacity << ", " << capacityStep << ", "
			<< memPoolBlockCount << ">: " << std::flush;

		typedef momo::TreeNode<maxCapacity, capacityStep,
			momo::MemPoolParams<memPoolBlockCount>> TreeNode;

		static const size_t count = 256;
		static uint8_t array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = static_cast<uint8_t>(i);

		if (maxCapacity > 1)
		{
			typedef momo::TreeSet<uint8_t, momo::TreeTraits<uint8_t, false, TreeNode>> TreeSet;

			for (size_t i = 0; i <= count; ++i)
			{
				TreeSet set1, set2;
				set1.Insert(array, array + i);
				set2.Insert(array + i, array + count);
				if (i > 0)
					set1.Remove(array[i - 1]);
				if (mt() % 2 == 0)
					std::swap(set1, set2);
				set1.MergeFrom(set2);
				if (i > 0)
					set1.Insert(array[i - 1]);
				assert(set1.GetCount() == count);
				assert(set2.IsEmpty());
				assert(std::equal(set1.GetBegin(), set1.GetEnd(), array));
				set2.Insert(set1.GetBegin(), set1.GetEnd());
				assert(std::equal(set2.GetBegin(), set2.GetEnd(), array));
			}
		}

		{
			typedef TemplItem<(keyTraits < 2), (keyTraits % 2 == 0)> Key;
			typedef TemplItem<(valueTraits < 2), (valueTraits % 2 == 0)> Value;

			typedef momo::TreeTraits<Key, false, TreeNode, true> TreeTraits;
			typedef momo::TreeMap<Key, Value, TreeTraits> TreeMap;

			std::map<Key, Value, std::less<Key>,
				momo::stdish::unsynchronized_pool_allocator<std::pair<const Key, Value>>> smap;
			TreeMap map;

			auto isEqual = [] (auto ref, const std::pair<const Key, Value>& sref)
				{ return ref.key == sref.first && ref.value == sref.second; };

			for (int t = 0; t < 3; ++t)
			{
				std::shuffle(array, array + count, mt);
				for (uint8_t c : array)
				{
					smap.insert({ Key(c), Value(c) });
					assert(map.Insert(Key(c), Value(c)).inserted);
					assert(map.GetCount() == smap.size());
					assert(std::equal(map.GetBegin(), map.GetEnd(), smap.begin(), isEqual));
				}

				if (t < 2)
				{
					std::shuffle(array, array + count, mt);
					for (uint8_t c : array)
					{
						smap.erase(Key(c));
						auto iter = map.Find(Key(c));
						assert(iter != map.GetEnd());
						if (t == 0)
							map.Extract(iter);
						else
							map.Remove(iter);
						assert(map.GetCount() == smap.size());
						assert(std::equal(map.GetBegin(), map.GetEnd(), smap.begin(), isEqual));
					}
				}
				else
				{
					while (!map.IsEmpty())
					{
						size_t mod = map.GetCount() + 1;
						size_t index1 = size_t{mt()} % mod;
						size_t index2 = size_t{mt()} % mod;
						if (index1 > index2)
							std::swap(index1, index2);

						auto siter = smap.erase(momo::internal::UIntMath<>::Next(smap.begin(), index1),
							momo::internal::UIntMath<>::Next(smap.begin(), index2));
						auto iter = map.Remove(momo::internal::UIntMath<>::Next(map.GetBegin(), index1),
							momo::internal::UIntMath<>::Next(map.GetBegin(), index2));
						assert((iter == map.GetEnd() && siter == smap.end()) || isEqual(*iter, *siter));
						assert(map.GetCount() == smap.size());
						assert(std::equal(map.GetBegin(), map.GetEnd(), smap.begin(), isEqual));
					}
				}
			}
		}

		std::cout << "ok" << std::endl;
	}
};

static int testSimpleTree = (SimpleTreeTester::TestStrAll(), SimpleTreeTester::TestTemplAll(), 0);

#endif // TEST_SIMPLE_TREE
