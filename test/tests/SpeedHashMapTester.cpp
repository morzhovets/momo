/**********************************************************\

  tests/SpeedHashMapTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SPEED_HASH_MAP

#include "../../momo/HashMap.h"
#include "../../momo/HashBuckets/BucketLim4.h"

#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <unordered_map>
#include <random>

template<typename TKey>
class SpeedHashMapTester
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
	explicit SpeedHashMapTester(size_t count, std::ostream& stream = std::cout);

	void TestUnorderedMap(bool reserve = false)
	{
		std::unordered_map<Key, size_t, std::hash<Key>> map;
		if (reserve)
			map.reserve(mKeys.GetCount());
		_TestMap(map, "std::unordered_map");
	}

	template<typename HashBucket>
	void TestHashMap(const std::string& mapTitle, bool reserve = false)
	{
		momo::HashMap<Key, size_t, momo::HashTraits<Key, HashBucket>> map;
		if (reserve)
			map.Reserve(mKeys.GetCount());
		_TestMap(map, mapTitle);
	}

	void TestAll()
	{
		TestUnorderedMap();
		TestHashMap<momo::HashBucketLimP1<>>("HashMapLimP1");
		TestHashMap<momo::HashBucketLimP<>>("HashMapLimP");
		TestHashMap<momo::HashBucketUnlimP<>>("HashMapUnlimP");
		TestHashMap<momo::HashBucketOneI1>("HashMapOneI1");
		//TestHashMap<momo::HashBucketLim4<>>("HashMapLim4");
	}

private:
	template<typename Map>
	void _TestMap(Map& map, const std::string& mapTitle)
	{
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
	}

private:
	momo::Array<Key> mKeys;
	std::ostream& mStream;
};

template<>
SpeedHashMapTester<uint32_t>::SpeedHashMapTester(size_t count, std::ostream& stream)
	: mStream(stream)
{
	std::mt19937 mt;
	mKeys.Reserve(count);
	for (size_t i = 0; i < count; ++i)
		mKeys.AddBack(mt());
}

template<>
SpeedHashMapTester<std::string>::SpeedHashMapTester(size_t count, std::ostream& stream)
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

void TestSpeedHashMap()
{
#ifdef NDEBUG
	const size_t count = 1 << 22;
#else
	const size_t count = 1 << 12;
#endif

	SpeedHashMapTester<uint32_t> speedHashMapTester(count, std::cout);
	//SpeedHashMapTester<std::string> speedHashMapTester(count, std::cout);
	for (size_t i = 0; i < 3; ++i)
	{
		std::cout << std::endl;
		speedHashMapTester.TestAll();
	}
}

static int testSpeedHashMap = (TestSpeedHashMap(), 0);

#endif // TEST_SPEED_HASH_MAP
