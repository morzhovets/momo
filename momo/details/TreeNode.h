/**********************************************************\

  momo/details/TreeNode.h

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager, size_t tCapacity>
	class TreeNode
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t capacity = tCapacity;

	private:
		typedef ObjectBuffer<Item, ItemTraits::alignment> ItemBuffer;

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams<sizeof(void*) * (capacity + 3) + sizeof(Item) * capacity>,
			MemManagerPtr> MemPool;

	public:
		class Params
		{
		public:
			explicit Params(MemManager& memManager)
				: mMemPool(MemManagerPtr(memManager))
			{
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemPool& GetMemPool() MOMO_NOEXCEPT
			{
				return mMemPool;
			}

		private:
			MemPool mMemPool;
		};

	public:
		TreeNode() = delete;

		TreeNode(const TreeNode&) = delete;

		~TreeNode() = delete;

		TreeNode& operator=(const TreeNode&) = delete;

		static TreeNode& Create(Params& params, bool isLeaf)
		{
			TreeNode& node = *(TreeNode*)params.GetMemPool().Allocate();
			node.mParent = nullptr;
			node.mIsLeaf = isLeaf;
			node.mCount = (unsigned char)0;
			for (size_t i = 0; i <= capacity; ++i)
				node.mChildren[i] = nullptr;
			return node;
		}

		void Destroy(Params& params) MOMO_NOEXCEPT
		{
			params.GetMemPool().Deallocate(this);
		}

		bool IsLeaf() const MOMO_NOEXCEPT
		{
			return mIsLeaf;
		}

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return (size_t)mCount;
		}

		void SetCount(size_t count) MOMO_NOEXCEPT
		{
			assert(count <= capacity);
			mCount = (unsigned char)count;
		}

		TreeNode* GetParent() MOMO_NOEXCEPT
		{
			return mParent;
		}

		void SetParent(TreeNode* parent) MOMO_NOEXCEPT
		{
			mParent = parent;
		}

		TreeNode* GetChild(size_t index) MOMO_NOEXCEPT
		{
			assert(!mIsLeaf);
			assert(index <= mCount);
			return mChildren[index];
		}

		void SetChild(size_t index, TreeNode* child) MOMO_NOEXCEPT
		{
			assert(!mIsLeaf);
			assert(index <= mCount);
			mChildren[index] = child;
		}

		size_t GetChildIndex(const TreeNode* child) const MOMO_NOEXCEPT
		{
			assert(!mIsLeaf);
			size_t index = std::find(mChildren, mChildren + mCount + 1, child) - mChildren;
			assert(index <= mCount);
			return index;
		}

		Item* GetItemPtr(size_t index) MOMO_NOEXCEPT
		{
			return &mItems[index];
		}

		void AcceptBackItem(size_t index) MOMO_NOEXCEPT
		{
			assert(mCount < capacity);
			assert(index <= mCount);
			for (size_t i = mCount; i > index; --i)
				ItemTraits::SwapNothrow(*GetItemPtr(i), *GetItemPtr(i - 1));
			memmove(mChildren + index + 2, mChildren + index + 1, (mCount - index) * sizeof(void*));
			++mCount;
		}

	private:
		bool mIsLeaf;
		unsigned char mCount;
		TreeNode* mParent;
		TreeNode* mChildren[capacity + 1];
		ItemBuffer mItems[capacity];
	};
}

} // namespace momo
