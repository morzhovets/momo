/**********************************************************\

  momo/details/TreeNode.h

  namespace momo:
    struct TreeNode

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tCapacity, bool tUseSwap>
	class Node
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t capacity = tCapacity;
		MOMO_STATIC_ASSERT(0 < capacity && capacity < 256);

		static const bool useSwap = tUseSwap;
		MOMO_STATIC_ASSERT(!useSwap || ItemTraits::isNothrowAnywaySwappable);

	private:
		typedef BoolConstant<useSwap> UseSwap;

		template<size_t capacity, bool hasIndices>
		struct Counter;

		template<size_t capacity>
		struct Counter<capacity, false>
		{
			unsigned char count;
		};

		template<size_t capacity>
		struct Counter<capacity, true>
		{
			unsigned char count;
			unsigned char indices[capacity];
		};

		typedef ObjectBuffer<Item, ItemTraits::alignment> ItemBuffer;

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParamsVarSize<>, MemManagerPtr> MemPool;

	public:
		class Params
		{
		private:
			typedef typename MemPool::Params MemPoolParams;

			static const size_t leafNodeSize = sizeof(Node);
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
		Node() = delete;

		Node(const Node&) = delete;

		~Node() = delete;

		Node& operator=(const Node&) = delete;

		static Node& Create(Params& params, bool isLeaf)
		{
			Node& node = *(Node*)params.GetMemPool(isLeaf).Allocate();
			node.mParent = nullptr;
			node.mIsLeaf = isLeaf;
			node.mCounter.count = (unsigned char)0;
			node._InitIndices(UseSwap());
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
			return (size_t)mCounter.count;
		}

		void SetCount(size_t count) MOMO_NOEXCEPT
		{
			assert(count <= capacity);
			mCounter.count = (unsigned char)count;
		}

		Node* GetParent() MOMO_NOEXCEPT
		{
			return mParent;
		}

		void SetParent(Node* parent) MOMO_NOEXCEPT
		{
			mParent = parent;
		}

		Node* GetChild(size_t index) MOMO_NOEXCEPT
		{
			assert(index <= GetCount());
			return _GetChildren()[index];
		}

		void SetChild(size_t index, Node* child) MOMO_NOEXCEPT
		{
			assert(index <= GetCount());
			_GetChildren()[index] = child;
		}

		size_t GetChildIndex(const Node* child) const MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			Node** children = _GetChildren();
			size_t index = std::find(children, children + count + 1, child) - children;
			assert(index <= count);
			return index;
		}

		Item* GetItemPtr(size_t index) MOMO_NOEXCEPT
		{
			return _GetItemPtr(index, UseSwap());
		}

		void AcceptBackItem(size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			assert(count < capacity);
			assert(index <= count);
			_AcceptBackItem(index, count, UseSwap());
			if (!mIsLeaf)
			{
				Node** children = _GetChildren();
				memmove(children + index + 2, children + index + 1, (count - index) * sizeof(void*));
			}
			++mCounter.count;
		}

		void Remove(size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			assert(index < count);
			_Remove(index, count, UseSwap());
			if (!mIsLeaf)
			{
				Node** children = _GetChildren();
				memmove(children + index, children + index + 1, (count - index) * sizeof(void*));
			}
			--mCounter.count;
		}

	private:
		void _InitIndices(std::true_type /*useSwap*/) MOMO_NOEXCEPT
		{
		}

		void _InitIndices(std::false_type /*useSwap*/) MOMO_NOEXCEPT
		{
			for (size_t i = 0; i < capacity; ++i)
				mCounter.indices[i] = (unsigned char)i;
		}

		Item* _GetItemPtr(size_t index, std::true_type /*useSwap*/) MOMO_NOEXCEPT
		{
			return &mItems[index];
		}

		Item* _GetItemPtr(size_t index, std::false_type /*useSwap*/) MOMO_NOEXCEPT
		{
			return &mItems[mCounter.indices[index]];
		}

		void _AcceptBackItem(size_t index, size_t count, std::true_type /*useSwap*/) MOMO_NOEXCEPT
		{
			for (size_t i = count; i > index; --i)
				ItemTraits::SwapNothrowAnyway(*GetItemPtr(i), *GetItemPtr(i - 1));
		}

		void _AcceptBackItem(size_t index, size_t count, std::false_type /*useSwap*/) MOMO_NOEXCEPT
		{
			unsigned char realIndex = mCounter.indices[count];
			memmove(mCounter.indices + index + 1, mCounter.indices + index, count - index);
			mCounter.indices[index] = realIndex;
		}

		void _Remove(size_t index, size_t count, std::true_type /*useSwap*/) MOMO_NOEXCEPT
		{
			for (size_t i = index + 1; i < count; ++i)
				ItemTraits::SwapNothrowAnyway(*GetItemPtr(i), *GetItemPtr(i - 1));
			ItemTraits::Destroy(*GetItemPtr(count - 1));
		}

		void _Remove(size_t index, size_t count, std::false_type /*useSwap*/) MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(*GetItemPtr(index));
			unsigned char realIndex = mCounter.indices[index];
			memmove(mCounter.indices + index, mCounter.indices + index + 1, count - index - 1);
			mCounter.indices[count - 1] = realIndex;
		}

		Node** _GetChildren() const MOMO_NOEXCEPT
		{
			assert(!mIsLeaf);
			return (Node**)(this + 1);
		}

	private:
		Node* mParent;
		bool mIsLeaf;
		Counter<capacity, !useSwap> mCounter;
		ItemBuffer mItems[capacity];
	};
}

template<size_t tCapacity, bool tUseSwap>
struct TreeNode
{
	static const size_t capacity = tCapacity;
	static const size_t useSwap = tUseSwap;

	template<typename ItemTraits, typename MemManager>
	using Node = internal::Node<ItemTraits, MemManager,
		capacity, useSwap && ItemTraits::isNothrowAnywaySwappable>;
};

} // namespace momo
