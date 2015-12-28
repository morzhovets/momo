/**********************************************************\

  momo/details/TreeNodeSwp.h

  namespace momo:
    struct TreeNodeSwp

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tCapacity>
	class NodeSwp
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

			static const size_t leafNodeSize = sizeof(NodeSwp);
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
		NodeSwp() = delete;

		NodeSwp(const NodeSwp&) = delete;

		~NodeSwp() = delete;

		NodeSwp& operator=(const NodeSwp&) = delete;

		static NodeSwp& Create(Params& params, bool isLeaf)
		{
			NodeSwp& node = *(NodeSwp*)params.GetMemPool(isLeaf).Allocate();
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

		NodeSwp* GetParent() MOMO_NOEXCEPT
		{
			return mParent;
		}

		void SetParent(NodeSwp* parent) MOMO_NOEXCEPT
		{
			mParent = parent;
		}

		NodeSwp* GetChild(size_t index) MOMO_NOEXCEPT
		{
			assert(index <= GetCount());
			return _GetChildren()[index];
		}

		void SetChild(size_t index, NodeSwp* child) MOMO_NOEXCEPT
		{
			assert(index <= GetCount());
			_GetChildren()[index] = child;
		}

		size_t GetChildIndex(const NodeSwp* child) const MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			NodeSwp** children = _GetChildren();
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
				ItemTraits::SwapNothrowAnyway(*GetItemPtr(i), *GetItemPtr(i - 1));
			if (!mIsLeaf)
			{
				NodeSwp** children = _GetChildren();
				memmove(children + index + 2, children + index + 1, (count - index) * sizeof(void*));
			}
			++mCount;
		}

		void Remove(size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			assert(index < count);
			for (size_t i = index + 1; i < count; ++i)
				ItemTraits::SwapNothrowAnyway(*GetItemPtr(i), *GetItemPtr(i - 1));
			ItemTraits::Destroy(*GetItemPtr(count - 1));
			if (!mIsLeaf)
			{
				NodeSwp** children = _GetChildren();
				memmove(children + index, children + index + 1, (count - index) * sizeof(void*));
			}
			--mCount;
		}

	private:
		NodeSwp** _GetChildren() const MOMO_NOEXCEPT
		{
			assert(!mIsLeaf);
			return (NodeSwp**)(this + 1);
		}

	private:
		NodeSwp* mParent;
		bool mIsLeaf;
		unsigned char mCount;
		ItemBuffer mItems[capacity];
	};
}

template<size_t tCapacity>
struct TreeNodeSwp
{
	static const size_t capacity = tCapacity;

	template<typename ItemTraits, typename MemManager>
	using Node = internal::NodeSwp<ItemTraits, MemManager, capacity>;
};

} // namespace momo
