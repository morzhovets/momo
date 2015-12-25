/**********************************************************\

  momo/details/TreeNodePrm.h

  namespace momo:
    struct TreeNodePrm

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tCapacity>
	class NodePrm
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t capacity = tCapacity;
		MOMO_STATIC_ASSERT(0 < capacity && capacity < 256);

	private:
		typedef ObjectBuffer<Item, ItemTraits::alignment> ItemBuffer;

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParamsVarSize<>, MemManagerPtr> MemPool;

	public:
		class Params
		{
		private:
			typedef typename MemPool::Params MemPoolParams;

			static const size_t leafNodeSize = sizeof(NodePrm);
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
		NodePrm() = delete;

		NodePrm(const NodePrm&) = delete;

		~NodePrm() = delete;

		NodePrm& operator=(const NodePrm&) = delete;

		static NodePrm& Create(Params& params, bool isLeaf)
		{
			NodePrm& node = *(NodePrm*)params.GetMemPool(isLeaf).Allocate();
			node.mParent = nullptr;
			node.mIsLeaf = isLeaf;
			node.mCount = (unsigned char)0;
			for (size_t i = 0; i < capacity; ++i)
				node.mIndices[i] = (unsigned char)i;
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

		NodePrm* GetParent() MOMO_NOEXCEPT
		{
			return mParent;
		}

		void SetParent(NodePrm* parent) MOMO_NOEXCEPT
		{
			mParent = parent;
		}

		NodePrm* GetChild(size_t index) MOMO_NOEXCEPT
		{
			assert(index <= GetCount());
			return _GetChildren()[index];
		}

		void SetChild(size_t index, NodePrm* child) MOMO_NOEXCEPT
		{
			assert(index <= GetCount());
			_GetChildren()[index] = child;
		}

		size_t GetChildIndex(const NodePrm* child) const MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			NodePrm** children = _GetChildren();
			size_t index = std::find(children, children + count + 1, child) - children;
			assert(index <= count);
			return index;
		}

		Item* GetItemPtr(size_t index) MOMO_NOEXCEPT
		{
			return &mItems[(size_t)mIndices[index]];
		}

		void AcceptBackItem(size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			assert(count < capacity);
			assert(index <= count);
			unsigned char realIndex = mIndices[count];
			memmove(mIndices + index + 1, mIndices + index, count - index);
			mIndices[index] = realIndex;
			if (!mIsLeaf)
			{
				NodePrm** children = _GetChildren();
				memmove(children + index + 2, children + index + 1, (count - index) * sizeof(void*));
			}
			++mCount;
		}

		void Remove(size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			assert(index < count);
			ItemTraits::Destroy(*GetItemPtr(index));
			unsigned char realIndex = mIndices[index];
			memmove(mIndices + index, mIndices + index + 1, count - index - 1);
			mIndices[count - 1] = realIndex;
			if (!mIsLeaf)
			{
				NodePrm** children = _GetChildren();
				memmove(children + index, children + index + 1, (count - index) * sizeof(void*));
			}
			--mCount;
		}

	private:
		NodePrm** _GetChildren() const MOMO_NOEXCEPT
		{
			assert(!mIsLeaf);
			return (NodePrm**)(this + 1);
		}

	private:
		NodePrm* mParent;
		bool mIsLeaf;
		unsigned char mCount;
		unsigned char mIndices[capacity];
		ItemBuffer mItems[capacity];
	};
}

template<size_t tCapacity>
struct TreeNodePrm
{
	static const size_t capacity = tCapacity;

	template<typename ItemTraits, typename MemManager>
	using Node = internal::NodePrm<ItemTraits, MemManager, capacity>;
};

} // namespace momo
