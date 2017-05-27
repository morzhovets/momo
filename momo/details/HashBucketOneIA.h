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
	template<typename TItemTraits>
	class BucketOneIA
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

	public:
		class Params
		{
		public:
			explicit Params(MemManager& memManager) MOMO_NOEXCEPT
				: mMemManager(memManager)
			{
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() MOMO_NOEXCEPT
			{
				return mMemManager;
			}

		private:
			MemManager& mMemManager;
		};

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
			return hashCode >> (sizeof(size_t) * 8 - 7) == (size_t)mCodeState >> 1;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return (mCodeState & 1) == (unsigned char)1;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return mCodeState != (unsigned char)0;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (IsFull())
				ItemTraits::Destroy(params.GetMemManager(), &mItemBuffer, 1);
			mCodeState = (unsigned char)0;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode)
		{
			MOMO_ASSERT(!IsFull());
			itemCreator(&mItemBuffer);
			mCodeState = (unsigned char)((hashCode >> (sizeof(size_t) * 8 - 8)) | 1);
			return &mItemBuffer;
		}

		void AcceptRemove(Params& /*params*/, size_t index) MOMO_NOEXCEPT
		{
			(void)index;
			MOMO_ASSERT(index == 0);
			MOMO_ASSERT(IsFull());
			mCodeState = (unsigned char)2;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		unsigned char mCodeState;
	};
}

struct HashBucketOneIA : public internal::HashBucketBase<1>
{
	template<typename ItemTraits>
	using Bucket = internal::BucketOneIA<ItemTraits>;
};

} // namespace momo
