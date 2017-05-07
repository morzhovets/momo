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

class SpeedMapKey
{
public:
	explicit SpeedMapKey(uint32_t* ptr) MOMO_NOEXCEPT
		: mPtr(ptr)
	{
	}

	uint32_t GetInt() const MOMO_NOEXCEPT
	{
		return *mPtr;
	}

	bool operator==(const SpeedMapKey& key) const MOMO_NOEXCEPT
	{
		return GetInt() == key.GetInt();
	}

	bool operator<(const SpeedMapKey& key) const MOMO_NOEXCEPT
	{
		return GetInt() < key.GetInt();
	}

private:
	uint32_t* mPtr;
};

namespace std
{
	template<>
	struct hash<SpeedMapKey>
	{
		size_t operator()(const SpeedMapKey& key) const MOMO_NOEXCEPT
		{
			return std::hash<uint32_t>()(key.GetInt());
		}
	};
}

template<typename Key>
class SpeedMapKeys;

template<>
class SpeedMapKeys<SpeedMapKey> : public momo::Array<SpeedMapKey>
{
public:
	SpeedMapKeys(size_t count, std::mt19937& random)
	{
		mInts.Reserve(count);
		Reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			mInts.AddBackNogrow(random());
			AddBackNogrow(SpeedMapKey(mInts.GetItems() + i));
		}
		std::shuffle(GetBegin(), GetEnd(), random);
	}

private:
	momo::Array<uint32_t> mInts;
};

template<>
class SpeedMapKeys<uint32_t> : public momo::Array<uint32_t>
{
public:
	SpeedMapKeys(size_t count, std::mt19937& random)
	{
		Reserve(count);
		for (size_t i = 0; i < count; ++i)
			AddBackNogrow(random());
	}
};

template<>
class SpeedMapKeys<std::string> : public momo::Array<std::string>
{
public:
	SpeedMapKeys(size_t count, std::mt19937& random)
	{
		Reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			std::stringstream sstream;
			sstream << random();
			AddBackNogrow(sstream.str());
		}
	}
};

template<typename TKey>
class SpeedMapTester
{
public:
	typedef TKey Key;
	typedef uint32_t Value;
	typedef SpeedMapKeys<Key> Keys;

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
	explicit SpeedMapTester(size_t count, std::ostream& stream = std::cout)
		: mRandom(),
		mKeys(count, mRandom),
		mStream(stream)
	{
	}

	template<typename Allocator>
	void TestStdUnorderedMap(const std::string& mapTitle)
	{
		typedef std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, Allocator> Map;
		pvTestMap<Map>(mapTitle);
	}

	template<typename HashBucket>
	void TestHashMap(const std::string& mapTitle)
	{
		typedef momo::stdish::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>,
			std::allocator<std::pair<const Key, Value>>,
			momo::HashMap<Key, Value,
				momo::HashTraitsStd<Key, std::hash<Key>, std::equal_to<Key>, HashBucket>>> Map;
		pvTestMap<Map>(mapTitle);
	}

	template<typename Allocator>
	void TestStdMap(const std::string& mapTitle)
	{
		typedef std::map<Key, Value, std::less<Key>, Allocator> Map;
		pvTestMap<Map>(mapTitle);
	}

	template<typename TreeNode>
	void TestTreeMap(const std::string& mapTitle)
	{
		typedef momo::stdish::map<Key, Value, std::less<Key>, std::allocator<std::pair<const Key, Value>>,
			momo::TreeMap<Key, Value, momo::TreeTraitsStd<Key, std::less<Key>, TreeNode>>> Map;
		pvTestMap<Map>(mapTitle);
	}

	void TestAll()
	{
		TestStdUnorderedMap<std::allocator<std::pair<const Key, Value>>>("std::unordered_map");
		TestHashMap<momo::HashBucketLimP<>>("HashMapLimP");
		TestHashMap<momo::HashBucketOneI1>("HashMapOneI1");
		//TestHashMap<momo::HashBucketLimP1<>>("HashMapLimP1");
		//TestHashMap<momo::HashBucketUnlimP<>>("HashMapUnlimP");
		//TestHashMap<momo::HashBucketLim4<>>("HashMapLim4");

		TestStdMap<momo::stdish::pool_allocator<std::pair<const Key, Value>>>("std::map + pool_allocator");
		TestTreeMap<momo::TreeNode<32, 4, momo::MemPoolParams<>, true>>("TreeNodeSwp");
		TestTreeMap<momo::TreeNode<32, 4, momo::MemPoolParams<>, false>>("TreeNodePrm");
	}

private:
	template<typename Map>
	void pvTestMap(const std::string& mapTitle)
	{
		Map map;
		{
			Timer timer(mapTitle + " insert: ", mStream);
			for (const Key& key : mKeys)
				map[key] = 0;
		}

		std::shuffle(mKeys.GetBegin(), mKeys.GetEnd(), mRandom);
		{
			Timer timer(mapTitle + " find: ", mStream);
			for (const Key& key : mKeys)
				map[key] = 0;
		}

		std::shuffle(mKeys.GetBegin(), mKeys.GetEnd(), mRandom);
		{
			Timer timer(mapTitle + " erase: ", mStream);
			for (const Key& key : mKeys)
				map.erase(key);
		}
	}

private:
	std::mt19937 mRandom;
	Keys mKeys;
	std::ostream& mStream;
};

void TestSpeedMap()
{
#ifdef NDEBUG
	const size_t count = 1 << 21;
#else
	const size_t count = 1 << 12;
#endif

	SpeedMapTester<SpeedMapKey> speedMapTester(count);
	//SpeedMapTester<uint32_t> speedMapTester(count);
	//SpeedMapTester<std::string> speedMapTester(count);

	for (size_t i = 0; i < 3; ++i)
	{
		std::cout << std::endl;
		speedMapTester.TestAll();
	}
}

static int testSpeedMap = (TestSpeedMap(), 0);

#endif // TEST_SPEED_MAP
