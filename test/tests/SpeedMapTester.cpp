/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SpeedMapTester.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SPEED_MAP

#include "../../momo/stdish/unordered_map.h"
#include "../../momo/stdish/map.h"

#include "../../momo/details/HashBucketLimP4.h"
#include "../../momo/details/HashBucketOpen2N2.h"
#include "../../momo/details/HashBucketOpen8.h"

#ifdef TEST_OLD_HASH_BUCKETS
#include "../../momo/details/HashBucketLim4.h"
#include "../../momo/details/HashBucketLimP.h"
#include "../../momo/details/HashBucketLimP1.h"
#include "../../momo/details/HashBucketUnlimP.h"
#include "../../momo/details/HashBucketOneIA.h"
#include "../../momo/details/HashBucketOneI1.h"
#include "../../momo/details/HashBucketOpenN1.h"
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <random>
#include <cmath>
#include <climits>
#include <unordered_map>
#include <map>

class IntPtr
{
public:
	explicit IntPtr(uint64_t* ptr) noexcept
		: mPtr(ptr)
	{
	}

	uint64_t GetInt() const noexcept
	{
		return *mPtr;
	}

	bool operator==(const IntPtr& key) const noexcept
	{
		return GetInt() == key.GetInt();
	}

	bool operator<(const IntPtr& key) const noexcept
	{
		return GetInt() < key.GetInt();
	}

private:
	uint64_t* mPtr;
};

namespace std
{
	template<>
	struct hash<IntPtr>
	{
		size_t operator()(const IntPtr& key) const noexcept
		{
			return std::hash<uint64_t>()(key.GetInt());
		}
	};
}

template<typename Key>
class SpeedMapKeys;

template<>
class SpeedMapKeys<uint64_t> : public momo::Array<uint64_t>
{
public:
	SpeedMapKeys(size_t count, std::mt19937_64& random)
	{
		Reserve(count);
		uint64_t* begin = GetItems();
		uint64_t* end = begin + count;
		while (true)
		{
			for (size_t i = GetCount(); i < count; ++i)
				AddBackNogrow(random());
			std::sort(begin, end);
			size_t newCount = momo::internal::UIntMath<>::Dist(begin, std::unique(begin, end));
			if (newCount == count)
				break;
			SetCount(newCount);
		}
		std::shuffle(begin, end, random);
	}

	static const char* GetKeyTitle() noexcept
	{
		return "uint64_t";
	}
};

template<>
class SpeedMapKeys<IntPtr> : public momo::Array<IntPtr>
{
public:
	SpeedMapKeys(size_t count, std::mt19937_64& random)
		: mData(count, random)
	{
		Reserve(count);
		for (size_t i = 0; i < count; ++i)
			AddBackNogrow(IntPtr(mData.GetItems() + i));
		std::shuffle(GetBegin(), GetEnd(), random);
	}

	static const char* GetKeyTitle() noexcept
	{
		return "IntPtr";
	}

private:
	SpeedMapKeys<uint64_t> mData;
};

template<>
class SpeedMapKeys<std::string> : public momo::Array<std::string>
{
public:
	SpeedMapKeys(size_t count, std::mt19937_64& random)
	{
		Reserve(count);
		std::string* begin = GetItems();
		std::string* end = begin + count;
		while (true)
		{
			for (size_t i = GetCount(); i < count; ++i)
			{
				std::stringstream sstream;
				sstream << random() % 100000000;
				AddBackNogrow(sstream.str());
			}
			std::sort(begin, end);
			size_t newCount = momo::internal::UIntMath<>::Dist(begin, std::unique(begin, end));
			if (newCount == count)
				break;
			SetCount(newCount);
		}
		std::shuffle(begin, end, random);
	}

	static const char* GetKeyTitle() noexcept
	{
		return "std::string";
	}
};

template<typename TKey>
class SpeedMapTester
{
public:
	typedef TKey Key;
	typedef uint64_t Value;

private:
	typedef SpeedMapKeys<Key> Keys;

	typedef std::chrono::steady_clock Clock;
	typedef std::chrono::time_point<Clock> TimePoint;
	typedef int64_t TickCount;

	template<typename Time = TickCount>
	struct TestResult
	{
		Time insertTime;
		Time findExistingTime;
		Time findRandomTime;
		Time eraseTime;
	};

public:
	explicit SpeedMapTester(size_t maxKeyCount, size_t runCount, std::ostream& resStream,
		std::ostream& procStream = std::cout)
		: mRandom(),
		mKeys(maxKeyCount, mRandom),
		mKeys2(maxKeyCount, mRandom),
		mRunCount(runCount),
		mResStream(resStream),
		mProcStream(procStream)
	{
		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		mResStream << std::ctime(&now) << " " << Keys::GetKeyTitle() << " "
			<< sizeof(void*) * 8 << "bit" << std::endl;
		mResStream << "title;count;"
			<< "insert (norm);find existing (norm);find random (norm);erase (norm);"
			<< "insert (real);find existing (real);find random (real);erase (real)"
			<< std::endl;

		mProcStream << "key title: " << Keys::GetKeyTitle() << "\n" << std::endl;
	}

	template<typename HashBucket>
	void TestHashBucket(const std::string& mapTitle, float maxLoadFactor = 0.0, bool reserve = false)
	{
		typedef std::allocator<std::pair<const Key, Value>> Allocator;
		typedef momo::stdish::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, Allocator,
			momo::HashMap<Key, Value, momo::HashTraitsStd<Key, std::hash<Key>, std::equal_to<Key>, HashBucket>,
			momo::MemManagerStd<Allocator>>> HashMap;
		TestHashMap<HashMap>(mapTitle, maxLoadFactor, reserve);
	}

	template<typename HashMap>
	void TestHashMap(const std::string& mapTitle, float maxLoadFactor = 0.0, bool reserve = false)
	{
		float trueMaxLoadFactor = maxLoadFactor;
		if (maxLoadFactor == 0)
			trueMaxLoadFactor = HashMap().max_load_factor();

		TestResult<double> maxNormRes = { 0.0, 0.0, 0.0, 0.0 };
		TestResult<double> avgNormRes = { 0.0, 0.0, 0.0, 0.0 };

		for (size_t i = 9; i <= 16; ++i)
		{
			size_t keyCount = mKeys.GetCount() / 16 * i;
			TestResult<double> normRes = pvTestHashMap<HashMap>(mapTitle, keyCount,
				trueMaxLoadFactor, reserve);

			maxNormRes.insertTime = std::minmax(maxNormRes.insertTime, normRes.insertTime).second;
			maxNormRes.findExistingTime = std::minmax(maxNormRes.findExistingTime, normRes.findExistingTime).second;
			maxNormRes.findRandomTime = std::minmax(maxNormRes.findRandomTime, normRes.findRandomTime).second;
			maxNormRes.eraseTime = std::minmax(maxNormRes.eraseTime, normRes.eraseTime).second;

			avgNormRes.insertTime += normRes.insertTime / 8.0;
			avgNormRes.findExistingTime += normRes.findExistingTime / 8.0;
			avgNormRes.findRandomTime += normRes.findRandomTime / 8.0;
			avgNormRes.eraseTime += normRes.eraseTime / 8.0;
		}

		mResStream << ";max;" << maxNormRes.insertTime << ";" << maxNormRes.findExistingTime << ";"
			<< maxNormRes.findRandomTime << ";" << maxNormRes.eraseTime << ";;;;" << std::endl;
		mResStream << ";avg;" << avgNormRes.insertTime << ";" << avgNormRes.findExistingTime << ";"
			<< avgNormRes.findRandomTime << ";" << avgNormRes.eraseTime << ";;;;" << std::endl;
	}

	template<typename TreeNode>
	void TestTreeNode(const std::string& mapTitle)
	{
		typedef std::allocator<std::pair<const Key, Value>> Allocator;
		typedef momo::stdish::map<Key, Value, std::less<Key>, Allocator,
			momo::TreeMap<Key, Value, momo::TreeTraitsStd<Key, std::less<Key>, false, TreeNode>,
			momo::MemManagerStd<Allocator>>> TreeMap;
		TestTreeMap<TreeMap>(mapTitle);
	}

	template<typename TreeMap>
	void TestTreeMap(const std::string& mapTitle)
	{
		pvTestTreeMap<TreeMap>(mapTitle, mKeys.GetCount() / 7);
		pvTestTreeMap<TreeMap>(mapTitle, mKeys.GetCount() / 5);
	}

	void TestAll()
	{
		TestHashMap<std::unordered_map<Key, Value>>("std::unordered_map");
		TestHashBucket<momo::HashBucketLimP4<>>("momo::HashBucketLimP4<>");
		TestHashBucket<momo::HashBucketOpen2N2<>>("momo::HashBucketOpen2N2<>");
		TestHashBucket<momo::HashBucketOpen8>("momo::HashBucketOpen8");
#ifdef TEST_OLD_HASH_BUCKETS
		TestHashBucket<momo::HashBucketLim4<>>("momo::HashBucketLim4<>");
		TestHashBucket<momo::HashBucketLimP<>>("momo::HashBucketLimP<>");
		TestHashBucket<momo::HashBucketLimP1<>>("momo::HashBucketLimP1<>");
		TestHashBucket<momo::HashBucketUnlimP<>>("momo::HashBucketUnlimP<>");
		TestHashBucket<momo::HashBucketOneIA<>>("momo::HashBucketOneIA<>");
		TestHashBucket<momo::HashBucketOneI1>("momo::HashBucketOneI1");
		TestHashBucket<momo::HashBucketOpenN1<>>("momo::HashBucketOpenN1<>");
#endif

		TestTreeMap<std::map<Key, Value>>("std::map");
		TestTreeNode<momo::TreeNode<32, 4, momo::MemPoolParams<>, true>>("momo::TreeNode<32, 4, <>, true>");
		TestTreeNode<momo::TreeNode<32, 4, momo::MemPoolParams<>, false>>("momo::TreeNode<32, 4, <>, false>");
	}

private:
	template<typename HashMap>
	TestResult<double> pvTestHashMap(const std::string& mapTitle, size_t keyCount, float maxLoadFactor, bool reserve)
	{
		auto afterCreate = [maxLoadFactor, reserve, keyCount] (HashMap& map)
		{
			map.max_load_factor(maxLoadFactor);
			if (reserve)
				map.reserve(keyCount);
		};

		std::stringstream sstream;
		sstream << mapTitle << " mlf=" << maxLoadFactor;
		if (reserve)
			sstream << " rsrv";
		std::string trueMapTitle = sstream.str();

		TestResult<> res = pvTestMap<HashMap>(trueMapTitle, keyCount, afterCreate);

		double norm = static_cast<double>(keyCount) / 1e3;
		TestResult<double> normRes = pvMakeNormResult(res, norm);

		pvOutputResult(trueMapTitle, keyCount, res, normRes);

		return normRes;
	}

	template<typename TreeMap>
	TestResult<double> pvTestTreeMap(const std::string& mapTitle, size_t keyCount)
	{
		auto afterCreate = [] (TreeMap&) { };

		TestResult<> res = pvTestMap<TreeMap>(mapTitle, keyCount, afterCreate);

		double norm = static_cast<double>(keyCount) * log2(static_cast<double>(keyCount)) / 1e3;
		TestResult<double> normRes = pvMakeNormResult(res, norm);

		pvOutputResult(mapTitle, keyCount, res, normRes);

		return normRes;
	}

	template<typename Map, typename AfterCreate>
	TestResult<> pvTestMap(const std::string& mapTitle, size_t keyCount, AfterCreate afterCreate)
	{
		mProcStream << "key count: " << keyCount << std::endl;
		TestResult<> res = { LLONG_MAX, LLONG_MAX, LLONG_MAX, LLONG_MAX };

		for (size_t t = 1; t <= mRunCount; ++t)
		{
			TimePoint start;

			Map map;
			afterCreate(map);

			std::shuffle(mKeys.GetBegin(), mKeys.GetBegin() + keyCount, mRandom);
			start = pvStart(t, mapTitle + " insert: ");
			for (size_t i = 0; i < keyCount; ++i)
				map.emplace(mKeys[i], 0);
			TickCount insertTime = pvFinish(start);
			res.insertTime = std::minmax(res.insertTime, insertTime).first;

			std::shuffle(mKeys.GetBegin(), mKeys.GetBegin() + keyCount, mRandom);
			start = pvStart(t, mapTitle + " find existing: ");
			for (size_t i = 0; i < keyCount; ++i)
			{
				if (map.find(mKeys[i]) == map.end())
					mProcStream << "";
			}
			TickCount findExistingTime = pvFinish(start);
			res.findExistingTime = std::minmax(res.findExistingTime, findExistingTime).first;

			start = pvStart(t, mapTitle + " find random: ");
			for (size_t i = 0; i < keyCount; ++i)
			{
				if (map.find(mKeys2[i]) != map.end())
					mProcStream << "";
			}
			TickCount findRandomTime = pvFinish(start);
			res.findRandomTime = std::minmax(res.findRandomTime, findRandomTime).first;

			std::shuffle(mKeys.GetBegin(), mKeys.GetBegin() + keyCount, mRandom);
			start = pvStart(t, mapTitle + " erase: ");
			for (size_t i = 0; i < keyCount; ++i)
				map.erase(mKeys[i]);
			TickCount eraseTime = pvFinish(start);
			res.eraseTime = std::minmax(res.eraseTime, eraseTime).first;
		}

		return res;
	}

	TimePoint pvStart(size_t number, const std::string& output)
	{
		mProcStream << number << ") " << output << std::flush;
		return Clock::now();
	}

	TickCount pvFinish(TimePoint start)
	{
		auto diff = Clock::now() - start;
		TickCount count = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
		mProcStream << count / 1000 << " ms" << std::endl;
		return count;
	}

	TestResult<double> pvMakeNormResult(TestResult<> res, double norm)
	{
		return { res.insertTime / norm, res.findExistingTime / norm,
			res.findRandomTime / norm, res.eraseTime / norm };
	}

	void pvOutputResult(const std::string& mapTitle, size_t keyCount, TestResult<> testRes,
		TestResult<double> normTestRes)
	{
		mResStream << mapTitle << ";" << keyCount << ";"
			<< normTestRes.insertTime << ";" << normTestRes.findExistingTime << ";"
			<< normTestRes.findRandomTime << ";" << normTestRes.eraseTime << ";"
			<< testRes.insertTime << ";" << testRes.findExistingTime << ";"
			<< testRes.findRandomTime << ";" << testRes.eraseTime << std::endl;

		mProcStream << "insert time: " << normTestRes.insertTime << std::endl;
		mProcStream << "find existing time: " << normTestRes.findExistingTime << std::endl;
		mProcStream << "find random time: " << normTestRes.findRandomTime << std::endl;
		mProcStream << "erase time: " << normTestRes.eraseTime << std::endl;
		mProcStream << std::endl;
	}

private:
	std::mt19937_64 mRandom;
	Keys mKeys;
	Keys mKeys2;
	size_t mRunCount;
	std::ostream& mResStream;
	std::ostream& mProcStream;
};

void TestSpeedMap()
{
	std::cout << "TestSpeedMap started" << std::endl;

#ifdef NDEBUG
	const size_t maxKeyCount = 1 << 21;
	std::ofstream resStream("bench.csv", std::ios_base::app);
#else
	const size_t maxKeyCount = 1 << 12;
	std::stringstream resStream;
#endif

	SpeedMapTester<uint64_t>(maxKeyCount, 3, resStream).TestAll();
	SpeedMapTester<IntPtr>(maxKeyCount, 3, resStream).TestAll();
}

static int testSpeedMap = (TestSpeedMap(), 0);

#endif // TEST_SPEED_MAP
