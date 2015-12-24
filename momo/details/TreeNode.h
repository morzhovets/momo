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

		typedef momo::MemPool<MemPoolParamsVarSize<>, MemManagerPtr> MemPool;

	public:
		class Params
		{
		private:
			typedef typename MemPool::Params MemPoolParams;

			static const size_t leafNodeSize = sizeof(TreeNode);
			static const size_t internalNodeSize = leafNodeSize + sizeof(void*) * (capacity + 1);

		public:
			explicit Params(MemManager& memManager)
				: mInternalMemPool(MemPoolParams(internalNodeSize), MemManagerPtr(memManager)),
				mLeafMemPool(MemPoolParams(leafNodeSize), MemManagerPtr(memManager))
			{
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemPool& GetMemPool(bool isLeaf) MOMO_NOEXCEPT
			{
				return isLeaf ? mLeafMemPool : mInternalMemPool;
			}

		private:
			MemPool mInternalMemPool;
			MemPool mLeafMemPool;
		};

	public:
		TreeNode() = delete;

		TreeNode(const TreeNode&) = delete;

		~TreeNode() = delete;

		TreeNode& operator=(const TreeNode&) = delete;

		static TreeNode& Create(Params& params, bool isLeaf)
		{
			TreeNode& node = *(TreeNode*)params.GetMemPool(isLeaf).Allocate();
			node.mParent = nullptr;
			node.mIsLeaf = isLeaf;
			node.mCount = (unsigned char)0;
			return node;
		}

		void Destroy(Params& params) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			for (size_t i = 0; i < count; ++i)
				ItemTraits::Destroy(*GetItemPtr(i));
			params.GetMemPool(mIsLeaf).Deallocate(this);
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
			assert(index <= GetCount());
			return _GetChildren()[index];
		}

		void SetChild(size_t index, TreeNode* child) MOMO_NOEXCEPT
		{
			assert(index <= GetCount());
			_GetChildren()[index] = child;
		}

		size_t GetChildIndex(const TreeNode* child) const MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			TreeNode** children = _GetChildren();
			size_t index = std::find(children, children + count + 1, child) - children;
			assert(index <= count);
			return index;
		}

		Item* GetItemPtr(size_t index) MOMO_NOEXCEPT
		{
			return &mItems[index];
		}

		void AcceptBackItem(size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			assert(count < capacity);
			assert(index <= count);
			for (size_t i = count; i > index; --i)
				ItemTraits::SwapNothrow(*GetItemPtr(i), *GetItemPtr(i - 1));
			if (!mIsLeaf)
			{
				TreeNode** children = _GetChildren();
				memmove(children + index + 2, children + index + 1, (count - index) * sizeof(void*));
			}
			++mCount;
		}

		void Remove(size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			assert(index < count);
			for (size_t i = index + 1; i < count; ++i)
				ItemTraits::SwapNothrow(*GetItemPtr(i), *GetItemPtr(i - 1));
			ItemTraits::Destroy(*GetItemPtr(count - 1));
			if (!mIsLeaf)
			{
				TreeNode** children = _GetChildren();
				memmove(children + index, children + index + 1, (count - index) * sizeof(void*));
			}
			--mCount;
		}

	private:
		TreeNode** _GetChildren() const MOMO_NOEXCEPT
		{
			assert(!mIsLeaf);
			return (TreeNode**)(this + 1);
		}

	private:
		TreeNode* mParent;
		bool mIsLeaf;
		unsigned char mCount;
		ItemBuffer mItems[capacity];
	};
}

} // namespace momo
