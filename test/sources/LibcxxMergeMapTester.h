/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxMergeMapTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/MergeMap.h"
#include "../../include/momo/stdish/unordered_map.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_merge_map
{

template<typename TKey, typename THasher, typename TEqualComparer>
class LibcxxMergeTraits
	: public momo::HashTraitsStd<TKey, THasher, TEqualComparer>
{
private:
	typedef momo::HashTraitsStd<TKey, THasher, TEqualComparer> HashTraitsStd;

public:
	using typename HashTraitsStd::Key;

	static const bool useHintIterators = false;

	typedef momo::MergeBloomFilterEmpty BloomFilter;

#ifdef LIBCXX_TEST_MERGE_HASH
	static const momo::MergeTraitsFunc func = momo::MergeTraitsFunc::hash;
#else
	static const momo::MergeTraitsFunc func = momo::MergeTraitsFunc::lessThrow;
#endif

public:
	using HashTraitsStd::HashTraitsStd;

	size_t GetSegmentItemCount(size_t segIndex) const noexcept
	{
		return size_t{1} << ((segIndex > 0) ? segIndex - 1 : 0);
	}

#ifndef LIBCXX_TEST_MERGE_HASH
	template<typename KeyArg>
	bool IsLess(const Key& key1, const KeyArg& key2) const
	{
		if constexpr (requires { key1 < key2; })
			return std::less<>()(key1, key2);
		else
			return HashTraitsStd::GetHashCode(key1) < HashTraitsStd::GetHashCode(key2);
	}
#endif
};

class LibcxxMergeMapSettings : public momo::MergeMapSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
};

template<typename MergeMap>
class LibcxxMergeMap : public MergeMap
{
public:
	typedef typename MergeMap::MergeTraits HashTraits;

	typedef momo::internal::HashMapBucketBounds<momo::internal::ArrayBounds<
		const typename momo::internal::MergeMapNestedSetItemTraits<
			typename MergeMap::KeyValueTraits>::Item*>> BucketBounds;

	static const size_t bucketMaxItemCount = SIZE_MAX;

public:
	using MergeMap::MergeMap;

	const HashTraits& GetHashTraits() const noexcept
	{
		return MergeMap::GetMergeTraits();
	}

	size_t GetBucketCount() const noexcept
	{
		return SIZE_MAX;
	}

	void Reserve(size_t /*capacity*/)
	{
	}

#ifndef LIBCXX_TEST_MERGE_HASH
	void MergeFrom(LibcxxMergeMap& srcMap)
	{
		assert(MergeMap::IsEmpty());
		MergeMap::Insert(srcMap.GetBegin(), srcMap.GetEnd());
		srcMap.Clear();
	}
#endif
};

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename THasher = std::hash<TKey>,
		typename TEqualComparer = std::equal_to<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using unordered_map = momo::stdish::unordered_map_adaptor<LibcxxMergeMap<momo::MergeMapCore<
		momo::MergeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		LibcxxMergeTraits<TKey, THasher, TEqualComparer>,
		LibcxxMergeMapSettings>>>;
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_MERGE_MAP
#define LIBCXX_TEST_PREFIX "merge_map" LIBCXX_TEST_PREFIX_TAIL
#include "libcxx20/UnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_MERGE_MAP
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_merge_map

} // namespace
