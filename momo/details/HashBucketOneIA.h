/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOneIA.h

  namespace momo:
    struct HashBucketOneIA

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<size_t size>
	struct BucketOneIACodeStateSelector
	{
		typedef unsigned char CodeState;
	};

	template<>
	struct BucketOneIACodeStateSelector<2>
	{
		typedef uint16_t CodeState;
	};

	template<>
	struct BucketOneIACodeStateSelector<4>
	{
		typedef uint32_t CodeState;
	};

	template<>
	struct BucketOneIACodeStateSelector<8>
	{
		typedef uint64_t CodeState;
	};

	template<typename TItemTraits>
	class BucketOneIA
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const size_t codeStateSize = (ItemTraits::alignment < sizeof(size_t))
			? ItemTraits::alignment : sizeof(size_t);
		static const size_t hashCodeShift = (sizeof(size_t) - codeStateSize) * 8;

		typedef typename BucketOneIACodeStateSelector<codeStateSize>::CodeState CodeState;

	public:
		BucketOneIA() MOMO_NOEXCEPT
			: mCodeState(0)
		{
		}

		BucketOneIA(const BucketOneIA&) = delete;

		~BucketOneIA() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsFull());
		}

		BucketOneIA& operator=(const BucketOneIA&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return IsFull() ? Bounds(&mItemBuffer, 1) : Bounds();
		}

		bool TestIndex(size_t index, size_t hashCode) const MOMO_NOEXCEPT
		{
			(void)index;
			MOMO_ASSERT(index == 0);
			MOMO_ASSERT(IsFull());
			return hashCode >> (hashCodeShift + 1) == (size_t)(mCodeState >> 1);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return (mCodeState & 1) == (CodeState)1;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return mCodeState != (CodeState)0;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			mCodeState = (CodeState)0;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			mCodeState = (CodeState)((hashCode >> hashCodeShift) | 1);
			return &mItemBuffer;
		}

		void AcceptRemove(Params& /*params*/, size_t index) MOMO_NOEXCEPT
		{
			(void)index;
			MOMO_ASSERT(index == 0);
			MOMO_ASSERT(IsFull());
			mCodeState = (CodeState)2;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		CodeState mCodeState;
	};
}

struct HashBucketOneIA : public internal::HashBucketBase<1>
{
	template<typename ItemTraits>
	using Bucket = internal::BucketOneIA<ItemTraits>;
};

} // namespace momo
