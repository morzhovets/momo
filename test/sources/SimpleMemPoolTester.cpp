/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/SimpleMemPoolTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_MEM_POOL

#include "../../include/momo/MemPool.h"
#include "../../include/momo/Array.h"
#include "../../include/momo/MemManagerDict.h"

#include <iostream>
#include <random>

class SimpleMemPoolTester
{
public:
	static void TestTemplAll()
	{
		std::mt19937 mt;
		TestTemplMemPool0(mt);
	}

	static void TestTemplMemPool0(std::mt19937& mt)
	{
		TestTemplMemPool1<1>(mt);
		TestTemplMemPool1<2>(mt);
		TestTemplMemPool1<3>(mt);
		TestTemplMemPool1<4>(mt);
		TestTemplMemPool1<6>(mt);
		TestTemplMemPool1<11>(mt);
		TestTemplMemPool1<17>(mt);
		TestTemplMemPool1<23>(mt);
		TestTemplMemPool1<45>(mt);
		TestTemplMemPool1<67>(mt);
		TestTemplMemPool1<89>(mt);
		TestTemplMemPool1<999>(mt);
	}

	template<size_t blockSize>
	static void TestTemplMemPool1(std::mt19937& mt)
	{
		TestTemplMemPool2<blockSize, 1>(mt);
		TestTemplMemPool2<blockSize, 2>(mt);
		TestTemplMemPool2<blockSize, 4>(mt);
		TestTemplMemPool2<blockSize, 8>(mt);
		TestTemplMemPool2<blockSize, 16>(mt);
		TestTemplMemPool2<blockSize, 256>(mt);
		TestTemplMemPool2<blockSize, 3>(mt);
		TestTemplMemPool2<blockSize, 6>(mt);
		TestTemplMemPool2<blockSize, 100>(mt);
	}

	template<size_t blockSize, size_t blockAlignment>
	static void TestTemplMemPool2(std::mt19937& mt)
	{
		TestTemplMemPool3<blockSize, blockAlignment, 1>(mt);
		TestTemplMemPool3<blockSize, blockAlignment, 2>(mt);
		TestTemplMemPool3<blockSize, blockAlignment, 3>(mt);
		TestTemplMemPool3<blockSize, blockAlignment, 127>(mt);
		TestTemplMemPool3<blockSize, blockAlignment,
			momo::MemPoolConst::defaultBlockCount>(mt);
	}

	template<size_t blockSize, size_t blockAlignment, size_t blockCount>
	static void TestTemplMemPool3(std::mt19937& mt)
	{
		static const size_t blockSize3 = blockSize % 3;
		static const size_t cachedFreeBlockCount = (blockSize3 < 2) ? blockSize3
			: momo::MemPoolConst::defaultCachedFreeBlockCount;
		TestTemplMemPool4<blockSize, blockAlignment, blockCount, cachedFreeBlockCount>(mt);
	}

	template<size_t blockSize, size_t blockAlignment, size_t blockCount, size_t cachedFreeBlockCount>
	static void TestTemplMemPool4(std::mt19937& mt)
	{
		std::cout << "momo::MemPoolParamsStatic<" << blockSize << ", " << blockAlignment << ", "
			<< blockCount << ", " << cachedFreeBlockCount << ">: " << std::flush;

		if (mt() % 2 == 0)
		{
			TestMemPoolParams(mt,
				momo::MemPoolParamsStatic<blockSize, blockAlignment, blockCount, cachedFreeBlockCount>());
		}
		else
		{
			TestMemPoolParams(mt,
				momo::MemPoolParams<blockCount, cachedFreeBlockCount>(blockSize, blockAlignment));
		}

		std::cout << "ok" << std::endl;
	}

	template<typename MemPoolParams>
	static void TestMemPoolParams(std::mt19937& mt, const MemPoolParams& params)
	{
		momo::MemPool<MemPoolParams, momo::MemManagerDict<>> memPool(params);

		static const size_t blockCount = 1024;
		static const size_t testCount = 8;
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
				size_t bufferSize = 0;
				assert(memPool.GetMemManager().FindBlock(blocks.GetBackItem(), &bufferSize));
				assert(bufferSize >= params.GetBlockCount() * params.GetBlockSize());

				memPool.Deallocate(blocks.GetBackItem());
				blocks.RemoveBack();
			}
			assert(memPool.GetAllocateCount() == lim);
		}

		if (memPool.CanDeallocateAll())
		{
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

				auto pred = [&blocks, lim] (void* block)
				{
					auto begin = blocks.GetBegin();
					auto mid = begin + lim;
					auto end = blocks.GetEnd();
					if (mid - begin < end - mid)
						return std::find(begin, mid, block) == mid;
					else
						return std::find(mid, end, block) != end;
				};
				memPool.DeallocateSet(pred);
				blocks.SetCount(lim);
				assert(memPool.GetAllocateCount() == lim);
			}
		}

		assert(memPool.GetMemManager().FindBlock(blocks.GetItems()) == nullptr);

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
