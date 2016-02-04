/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/TemplHashTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_TEMPL_HASH

#undef NDEBUG

#include "../../momo/HashSet.h"
#include "../../momo/details/BucketLim4.h"

#include <iostream>
#include <type_traits>
#include <random>

class TemplHashTester
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
		TestHashSet<momo::HashBucketOneI1, 1, 1>("momo::HashBucketOneI1");
		TestHashSet<momo::HashBucketOneI1, 4, 2>("momo::HashBucketOneI1");

		TestHashSet<momo::HashBucketLimP1< 1, 16, 1>,  1, 1>("momo::HashBucketLimP1< 1, 16, 1>");
		TestHashSet<momo::HashBucketLimP1< 1, 99, 2>,  2, 1>("momo::HashBucketLimP1< 1, 99, 2>");
		TestHashSet<momo::HashBucketLimP1< 2, 32, 4>, 16, 8>("momo::HashBucketLimP1< 2, 32, 4>");
		TestHashSet<momo::HashBucketLimP1< 3,  1, 8>,  4, 4>("momo::HashBucketLimP1< 3,  1, 8>");
		TestHashSet<momo::HashBucketLimP1< 4,  2, 1>,  4, 4>("momo::HashBucketLimP1< 4,  2, 1>");
		TestHashSet<momo::HashBucketLimP1< 7, 11, 2>,  6, 2>("momo::HashBucketLimP1< 7, 11, 2>");
		TestHashSet<momo::HashBucketLimP1<15,  1, 4>, 11, 1>("momo::HashBucketLimP1<15,  1, 4>");

		TestHashSet<momo::HashBucketLimP< 1, 16, false>, 11, 1>("momo::HashBucketLimP< 1, 16, false>");
		TestHashSet<momo::HashBucketLimP< 1, 64, false>,  1, 1>("momo::HashBucketLimP< 1, 64, false>");
		TestHashSet<momo::HashBucketLimP< 2, 99, false>,  3, 1>("momo::HashBucketLimP< 2, 99, false>");
		TestHashSet<momo::HashBucketLimP< 2, 88, false>,  2, 2>("momo::HashBucketLimP< 2, 88, false>");
		TestHashSet<momo::HashBucketLimP< 2,  1, false>,  8, 4>("momo::HashBucketLimP< 2,  1, false>");
		TestHashSet<momo::HashBucketLimP< 3,  2, false>,  6, 1>("momo::HashBucketLimP< 3,  2, false>");
		TestHashSet<momo::HashBucketLimP< 3,  3, false>,  8, 2>("momo::HashBucketLimP< 3,  3, false>");
		TestHashSet<momo::HashBucketLimP< 4,  4, false>,  4, 4>("momo::HashBucketLimP< 4,  4, false>");
		TestHashSet<momo::HashBucketLimP< 5,  5, false>, 10, 2>("momo::HashBucketLimP< 5,  5, false>");
		TestHashSet<momo::HashBucketLimP< 6,  8, false>,  4, 4>("momo::HashBucketLimP< 6,  8, false>");
		TestHashSet<momo::HashBucketLimP< 7, 11, false>, 16, 4>("momo::HashBucketLimP< 7, 11, false>");
		TestHashSet<momo::HashBucketLimP< 8, 33, false>,  8, 8>("momo::HashBucketLimP< 8, 33, false>");
		TestHashSet<momo::HashBucketLimP<10, 32, false>, 32, 8>("momo::HashBucketLimP<10, 32, false>");
		TestHashSet<momo::HashBucketLimP<12, 55, false>, 11, 1>("momo::HashBucketLimP<12, 55, false>");
		TestHashSet<momo::HashBucketLimP<14,  1, false>, 16, 8>("momo::HashBucketLimP<14,  1, false>");
		TestHashSet<momo::HashBucketLimP<15, 32, false>, 16, 8>("momo::HashBucketLimP<15, 32, false>");

		TestHashSet<momo::HashBucketLimP< 1, 16, true>, 11, 1>("momo::HashBucketLimP< 1, 16, true>");
		TestHashSet<momo::HashBucketLimP< 1, 64, true>,  1, 1>("momo::HashBucketLimP< 1, 64, true>");
		TestHashSet<momo::HashBucketLimP< 2, 99, true>,  3, 1>("momo::HashBucketLimP< 2, 99, true>");
		TestHashSet<momo::HashBucketLimP< 2, 88, true>,  2, 2>("momo::HashBucketLimP< 2, 88, true>");
		TestHashSet<momo::HashBucketLimP< 2,  1, true>,  8, 4>("momo::HashBucketLimP< 2,  1, true>");
		TestHashSet<momo::HashBucketLimP< 3,  2, true>,  6, 1>("momo::HashBucketLimP< 3,  2, true>");
		TestHashSet<momo::HashBucketLimP< 3,  3, true>,  8, 2>("momo::HashBucketLimP< 3,  3, true>");
		TestHashSet<momo::HashBucketLimP< 4,  4, true>,  4, 4>("momo::HashBucketLimP< 4,  4, true>");
		TestHashSet<momo::HashBucketLimP< 5,  5, true>, 10, 2>("momo::HashBucketLimP< 5,  5, true>");
		TestHashSet<momo::HashBucketLimP< 6,  8, true>,  4, 4>("momo::HashBucketLimP< 6,  8, true>");
		TestHashSet<momo::HashBucketLimP< 7, 11, true>, 16, 4>("momo::HashBucketLimP< 7, 11, true>");
		TestHashSet<momo::HashBucketLimP< 8, 33, true>,  8, 8>("momo::HashBucketLimP< 8, 33, true>");
		TestHashSet<momo::HashBucketLimP<10, 32, true>, 32, 8>("momo::HashBucketLimP<10, 32, true>");
		TestHashSet<momo::HashBucketLimP<12, 55, true>, 11, 1>("momo::HashBucketLimP<12, 55, true>");
		TestHashSet<momo::HashBucketLimP<14,  1, true>, 16, 8>("momo::HashBucketLimP<14,  1, true>");
		TestHashSet<momo::HashBucketLimP<15, 32, true>, 16, 8>("momo::HashBucketLimP<15, 32, true>");

		TestHashSet<momo::HashBucketUnlimP< 1, 32>,  1, 1>("momo::HashBucketUnlimP< 1, 32>");
		TestHashSet<momo::HashBucketUnlimP< 1,  1>,  2, 1>("momo::HashBucketUnlimP< 1,  1>");
		TestHashSet<momo::HashBucketUnlimP< 2, 99>, 16, 8>("momo::HashBucketUnlimP< 2, 99>");
		TestHashSet<momo::HashBucketUnlimP< 3, 16>,  4, 4>("momo::HashBucketUnlimP< 3, 16>");
		TestHashSet<momo::HashBucketUnlimP< 7, 11>,  6, 2>("momo::HashBucketUnlimP< 7, 11>");
		TestHashSet<momo::HashBucketUnlimP<15,  1>, 11, 1>("momo::HashBucketUnlimP<15,  1>");

		TestHashSet<momo::HashBucketLim4<1, 32>,  1, 1>("momo::HashBucketLim4<1, 32>");
		TestHashSet<momo::HashBucketLim4<2, 99>, 16, 8>("momo::HashBucketLim4<2, 99>");
		TestHashSet<momo::HashBucketLim4<4,  1>, 12, 4>("momo::HashBucketLim4<4,  1>");
	}

	template<typename HashBucket, size_t size, size_t alignment>
	static void TestHashSet(const char* bucketName)
	{
		std::cout << bucketName << ": TemplItem<" << size << ", " << alignment << ">: " << std::flush;

		static const size_t count = 256;
		static unsigned char array[count];
		for (size_t i = 0; i < count; ++i)
			array[i] = (unsigned char)i;

		typedef TemplItem<size, alignment> Item;
		typedef momo::HashSet<Item, typename Item::template HashTraits<HashBucket>> HashSet;
		HashSet set;

		std::shuffle(array, array + count, std::mt19937());
		for (unsigned char c : array)
			assert(set.Insert(Item(c)).inserted);
		assert(set.GetCount() == count);

		std::shuffle(array, array + count, std::mt19937());
		for (unsigned char c : array)
			assert(set.Remove(Item(c)));
		assert(set.IsEmpty());

		std::cout << "ok" << std::endl;
	}
};

static int testTemplHash = (TemplHashTester::TestAll(), 0);

#endif // TEST_TEMPL_HASH
