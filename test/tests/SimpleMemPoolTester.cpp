/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleMemPoolTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_MEM_POOL

#include "LeakCheckMemManager.h"

#include "../../momo/MemPool.h"
#include "../../momo/Array.h"

#include <iostream>
#include <random>

class SimpleMemPoolTester
{
public:
	static void TestTemplAll()
	{
		std::mt19937 mt;

		TestTemplMemPool<1, 1, 1, 0>(mt);
	}

	template<size_t blockSize, size_t blockAlignment, size_t blockCount, size_t cachedFreeBlockCount>
	static void TestTemplMemPool(std::mt19937& mt)
	{
		std::cout << "momo::MemPoolParamsStatic<" << blockSize << ", " << blockAlignment << ", "
			<< blockCount << ", " << cachedFreeBlockCount << ">: " << std::flush;

		TestMemPoolParams(mt,
			momo::MemPoolParamsStatic<blockSize, blockAlignment, blockCount, cachedFreeBlockCount>());

		TestMemPoolParams(mt,
			momo::MemPoolParams<blockCount, cachedFreeBlockCount>(blockSize, blockAlignment));

		std::cout << "ok" << std::endl;
	}

	template<typename MemPoolParams>
	static void TestMemPoolParams(std::mt19937& mt, const MemPoolParams& params)
	{
		momo::MemPool<MemPoolParams, LeakCheckMemManager> memPool(params);

		static const size_t blockCount = 1024;
		static const size_t testCount = 64;
		momo::Array<void*> blocks = momo::Array<void*>::CreateCap(blockCount);

		for (size_t k = 0; k < testCount; ++k)
		{
			while (blocks.GetCount() < blockCount)
			{
				blocks.AddBack(memPool.Allocate());
				std::memset(blocks.GetBackItem(), 0, memPool.GetBlockSize());
			}
			assert(memPool.GetAllocateCount() == blockCount);

			std::shuffle(blocks.GetBegin(), blocks.GetEnd(), mt);

			size_t lim = k * blockCount / testCount;
			while (blocks.GetCount() > lim)
			{
				memPool.Deallocate(blocks.GetBackItem());
				blocks.RemoveBack();
			}
			assert(memPool.GetAllocateCount() == lim);
		}

		if (memPool.CanDeallocateAll())
		{
			memPool.DeallocateAll();
		}
		else
		{
			for (void* block : blocks)
				memPool.Deallocate(block);
		}
		assert(memPool.GetAllocateCount() == 0);
	}
};

static int testSimpleMemPool = (SimpleMemPoolTester::TestTemplAll(), 0);

#endif // TEST_SIMPLE_MEM_POOL
