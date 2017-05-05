/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SpeedMapTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SPEED_MAP

#include "../../momo/stdish/unordered_map.h"
#include "../../momo/stdish/map.h"
#include "../../momo/stdish/pool_allocator.h"
#include "../../momo/details/HashBucketLim4.h"

#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <map>
#include <random>

class SpeedMapTesterKey
{
public:
	SpeedMapTesterKey(SpeedMapTesterKey* ptr, uint32_t value) MOMO_NOEXCEPT
		: mPtr(ptr),
		mValue(value)
	{
	}

	uint32_t GetPtrValue() const MOMO_NOEXCEPT
	{
		return mPtr->mValue;
	}

	bool operator==(const SpeedMapTesterKey& key) const MOMO_NOEXCEPT
	{
		return GetPtrValue() == key.GetPtrValue();
	}

	bool operator<(const SpeedMapTesterKey& key) const MOMO_NOEXCEPT
	{
		return GetPtrValue() < key.GetPtrValue();
	}

	friend void swap(SpeedMapTesterKey& key1, SpeedMapTesterKey& key2) MOMO_NOEXCEPT
	{
		std::swap(key1.mPtr, key2.mPtr);
	}

private:
	SpeedMapTesterKey* mPtr;
	uint32_t mValue;
};

namespace std
{
	template<>
	struct hash<SpeedMapTesterKey>
	{
		size_t operator()(const SpeedMapTesterKey& key) const MOMO_NOEXCEPT
		{
			return std::hash<uint32_t>()(key.GetPtrValue());
		}
	};
}

template<typename TKey>
class SpeedMapTester
{
public:
	typedef TKey Key;

private:
	class Timer
	{
	private:
#if defined(_MSC_VER) && _MSC_VER < 1900
		typedef std::chrono::system_clock Clock;
#else
		typedef std::chrono::steady_clock Clock;
#endif

	public:
		Timer(const std::string& title, std::ostream& stream)
			: mStream(stream)
		{
			mStream << title << std::flush;
			mStartTime = Clock::now();
		}

		~Timer()
		{
			auto diff = Clock::now() - mStartTime;
			auto count = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
			mStream << count << std::endl;
		}

	private:
		std::ostream& mStream;
		std::chrono::time_point<Clock> mStartTime;
	};

public:
	explicit SpeedMapTester(size_t count, std::ostream& stream = std::cout);

	template<typename Allocator>
	void TestStdUnorderedMap(const std::string& mapTitle)
	{
		typedef std::unordered_map<Key, uint32_t, std::hash<Key>, std::equal_to<Key>, Allocator> Map;
		pvTestMap<Map>(mapTitle);
	}

	template<typename HashBucket>
	void TestHashMap(const std::string& mapTitle)
	{
		typedef momo::stdish::unordered_map<Key, uint32_t, std::hash<Key>, std::equal_to<Key>,
			std::allocator<std::pair<const Key, uint32_t>>,
			momo::HashMap<Key, uint32_t,
				momo::HashTraitsStd<Key, std::hash<Key>, std::equal_to<Key>, HashBucket>>> Map;
		pvTestMap<Map>(mapTitle);
	}

	template<typename Allocator>
	void TestStdMap(const std::string& mapTitle)
	{
		typedef std::map<Key, uint32_t, std::less<Key>, Allocator> Map;
		pvTestMap<Map>(mapTitle);
	}

	template<typename TreeNode>
	void TestTreeMap(const std::string& mapTitle)
	{
		typedef momo::stdish::map<Key, uint32_t, std::less<Key>, std::allocator<std::pair<const Key, uint32_t>>,
			momo::TreeMap<Key, uint32_t, momo::TreeTraitsStd<Key, std::less<Key>, TreeNode>>> Map;
		pvTestMap<Map>(mapTitle);
	}

	void TestAll()
	{
		TestStdUnorderedMap<std::allocator<std::pair<const Key, uint32_t>>>("std::unordered_map");
		TestHashMap<momo::HashBucketLimP<>>("HashMapLimP");
		TestHashMap<momo::HashBucketOneI1>("HashMapOneI1");
		//TestHashMap<momo::HashBucketLimP1<>>("HashMapLimP1");
		//TestHashMap<momo::HashBucketUnlimP<>>("HashMapUnlimP");
		//TestHashMap<momo::HashBucketLim4<>>("HashMapLim4");

		TestStdMap<momo::stdish::pool_allocator<std::pair<const Key, uint32_t>>>("std::map + pool_allocator");
		TestTreeMap<momo::TreeNode<32, 4, momo::MemPoolParams<>, true>>("TreeNodeSwp");
		TestTreeMap<momo::TreeNode<32, 4, momo::MemPoolParams<>, false>>("TreeNodePrm");
	}

private:
	template<typename Map>
	void pvTestMap(const std::string& mapTitle)
	{
		Map map;
		std::mt19937 mt;
		{
			Timer timer(mapTitle + " insert: ", mStream);
			for (const Key& key : mKeys)
				map[key] = 0;
		}

		std::shuffle(mKeys.GetBegin(), mKeys.GetEnd(), mt);
		{
			Timer timer(mapTitle + " find: ", mStream);
			for (const Key& key : mKeys)
				map[key] = 0;
		}

		std::shuffle(mKeys.GetBegin(), mKeys.GetEnd(), mt);
		{
			Timer timer(mapTitle + " erase: ", mStream);
			for (const Key& key : mKeys)
				map.erase(key);
		}
	}

private:
	momo::Array<Key> mKeys;
	std::ostream& mStream;
};

template<>
SpeedMapTester<SpeedMapTesterKey>::SpeedMapTester(size_t count, std::ostream& stream)
	: mStream(stream)
{
	std::mt19937 mt;
	mKeys.Reserve(count);
	for (size_t i = 0; i < count; ++i)
		mKeys.AddBack(SpeedMapTesterKey(mKeys.GetItems() + i, mt()));
	std::shuffle(mKeys.GetBegin(), mKeys.GetEnd(), mt);
}

template<>
SpeedMapTester<uint32_t>::SpeedMapTester(size_t count, std::ostream& stream)
	: mStream(stream)
{
	std::mt19937 mt;
	mKeys.Reserve(count);
	for (size_t i = 0; i < count; ++i)
		mKeys.AddBack(mt());
}

template<>
SpeedMapTester<std::string>::SpeedMapTester(size_t count, std::ostream& stream)
	: mStream(stream)
{
	std::mt19937 mt;
	mKeys.Reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		std::stringstream sstream;
		sstream << mt();
		mKeys.AddBack(sstream.str());
	}
}

void TestSpeedMap()
{
#ifdef NDEBUG
	const size_t count = 1 << 21;
#else
	const size_t count = 1 << 12;
#endif

	SpeedMapTester<SpeedMapTesterKey> speedMapTester(count, std::cout);
	//SpeedMapTester<uint32_t> speedMapTester(count, std::cout);
	//SpeedMapTester<std::string> speedMapTester(count, std::cout);

	for (size_t i = 0; i < 3; ++i)
	{
		std::cout << std::endl;
		speedMapTester.TestAll();
	}
}

static int testSpeedMap = (TestSpeedMap(), 0);

#endif // TEST_SPEED_MAP
