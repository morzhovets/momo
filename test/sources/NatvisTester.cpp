/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/NatvisTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_NATVIS

#include "../../include/momo/Array.h"
#include "../../include/momo/SegmentedArray.h"
#include "../../include/momo/HashSet.h"
#include "../../include/momo/HashMap.h"
#include "../../include/momo/HashMultiMap.h"
#include "../../include/momo/TreeSet.h"
#include "../../include/momo/TreeMap.h"
#include "../../include/momo/DataTable.h"

#ifdef TEST_MSVC
#pragma warning (disable: 4307)	// integral constant overflow
#endif

static int testNatvis = []
{
	momo::Array<int> ar;
	momo::ArrayIntCap<64, int> aric;
	momo::SegmentedArray<int> sar;
	momo::HashSet<int> hset;
	momo::HashSetOpen<int> hseto;
	momo::HashSet<int, momo::HashTraits<int, momo::HashBucketOpen2N2<>>> hseto2;
	momo::HashMap<int, int> hmap;
	momo::HashMultiMap<int, int> hmmap;
	momo::TreeSet<int> tset;
	momo::TreeSet<int, momo::TreeTraits<int, false,
		momo::TreeNode<32, 4, momo::MemPoolParams<8>, false>>> tsetc;
	momo::TreeMap<int, int> tmap;
	momo::TreeMap<int, int, momo::TreeTraits<int>, momo::MemManagerDefault,
		momo::TreeMapKeyValueTraits<int, int, momo::MemManagerDefault, true>> tmapp;

	MOMO_DATA_COLUMN_STRING(int, intCol);
	momo::DataTable<> dt(intCol);

	for (int i = 0; i < 50; ++i)
	{
		int key = 1055 + i * 1000;
		ar.AddBack(key);
		aric.AddBack(key);
		sar.AddBack(key);
		hset.Insert(key);
		hseto.Insert(key);
		hseto2.Insert(key);
		hmap.Insert(key, i);
		for (int j = 0; j <= i; ++j)
			hmmap.Add(key, j);
		tset.Insert(key);
		tsetc.Insert(key);
		tmap.Insert(key, i);
		tmapp.Insert(key, i);
		dt.AddRow(intCol = key);
	}

	momo::DataTable<>::Selection sel = dt.Select();

	return 0;
}();

#endif // TEST_NATVIS
