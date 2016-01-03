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
#include "../../momo/details/BucketLim4.h"

#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <unordered_map>
#include <map>
#include <random>

template<typename TKey>
class SpeedMapTester
{
public:
	typedef TKey Key;

private:
	class Timer
	{
	public:
		Timer(const std::string& title, std::ostream& stream)
			: mStream(stream)
		{
			mStream << title << std::flush;
			mStartTime = clock();
		}

		~Timer()
		{
			clock_t time = clock() - mStartTime;
			mStream << (int64_t)time * 1000 / CLOCKS_PER_SEC << std::endl;
		}

	private:
		std::ostream& mStream;
		clock_t mStartTime;
	};

public:
	explicit SpeedMapTester(size_t count, std::ostream& stream = std::cout);

	template<typename Allocator>
	void TestStdUnorderedMap(const std::string& mapTitle)
	{
		typedef std::unordered_map<Key, size_t, std::hash<Key>, std::equal_to<Key>, Allocator> Map;
		_TestMap<Map>(mapTitle);
	}

	template<typename HashBucket>
	void TestHashMap(const std::string& mapTitle)
	{
		typedef momo::stdish::unordered_map<Key, size_t, std::hash<Key>, std::equal_to<Key>,
			std::allocator<std::pair<const Key, size_t>>,
			momo::HashMap<Key, size_t,
				momo::HashTraitsStd<Key, std::hash<Key>, std::equal_to<Key>, HashBucket>>> Map;
		_TestMap<Map>(mapTitle);
	}

	template<typename Allocator>
	void TestStdMap(const std::string& mapTitle)
	{
		typedef std::map<Key, size_t, std::less<Key>, Allocator> Map;
		_TestMap<Map>(mapTitle);
	}

	template<typename TreeNode>
	void TestTreeMap(const std::string& mapTitle)
	{
		typedef momo::stdish::map<Key, size_t, std::less<Key>, std::allocator<std::pair<const Key, size_t>>,
			momo::TreeMap<Key, size_t, momo::TreeTraitsStd<Key, std::less<Key>, TreeNode>>> Map;
		_TestMap<Map>(mapTitle);
	}

	void TestAll()
	{
		TestStdUnorderedMap<std::allocator<std::pair<const Key, size_t>>>("std::unordered_map");
		TestHashMap<momo::HashBucketLimP1<>>("HashMapLimP1");
		TestHashMap<momo::HashBucketLimP<>>("HashMapLimP");
		TestHashMap<momo::HashBucketUnlimP<>>("HashMapUnlimP");
		TestHashMap<momo::HashBucketOneI1>("HashMapOneI1");
		//TestHashMap<momo::HashBucketLim4<>>("HashMapLim4");

		TestStdMap<momo::stdish::pool_allocator<std::pair<const Key, size_t>>>("std::map + pool_allocator");
		TestTreeMap<momo::TreeNode<32, 4, false>>("TreeNodePrm");
		TestTreeMap<momo::TreeNode<32, 4, true>>("TreeNodeSwp");
	}

private:
	template<typename Map>
	void _TestMap(const std::string& mapTitle)
	{
		Map map;
		{
			Timer timer(mapTitle + " insert: ", mStream);
			for (const Key& key : mKeys)
				map[key] = 0;
		}
		{
			Timer timer(mapTitle + " find: ", mStream);
			for (const Key& key : mKeys)
				map[key] = 0;
		}
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

	SpeedMapTester<uint32_t> speedMapTester(count, std::cout);
	//SpeedMapTester<std::string> speedMapTester(count, std::cout);
	for (size_t i = 0; i < 3; ++i)
	{
		std::cout << std::endl;
		speedMapTester.TestAll();
	}
}

static int testSpeedMap = (TestSpeedMap(), 0);

#endif // TEST_SPEED_MAP
