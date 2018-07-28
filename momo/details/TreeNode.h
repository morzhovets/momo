/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

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
	template<typename TItemTraits, size_t tMaxCapacity, size_t tCapacityStep,
		typename TMemPoolParams, bool tIsContinuous>
	class Node
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t maxCapacity = tMaxCapacity;
		MOMO_STATIC_ASSERT(0 < maxCapacity && maxCapacity < 256);

		static const size_t capacityStep = tCapacityStep;
		MOMO_STATIC_ASSERT(capacityStep > 0);

		static const bool isContinuous = tIsContinuous;
		MOMO_STATIC_ASSERT(!isContinuous || ItemTraits::isNothrowShiftable);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

	private:
		typedef BoolConstant<isContinuous> IsContinuous;

		template<size_t capacity, bool hasIndices>
		struct Counter;

		template<size_t capacity>
		struct Counter<capacity, false>
		{
			uint8_t count;
		};

		template<size_t capacity>
		struct Counter<capacity, true>
		{
			uint8_t count;
			uint8_t indices[capacity];
		};

		typedef ObjectBuffer<Item, ItemTraits::alignment> ItemBuffer;

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		static const size_t leafMemPoolCount = maxCapacity / (2 * capacityStep) + 1;

		static const size_t internalOffset = MOMO_MAX_ALIGNMENT
			* ((sizeof(void*) * (maxCapacity + 1) - 1) / MOMO_MAX_ALIGNMENT + 1);

	public:
		class Params
		{
		private:
			typedef NestedArrayIntCap<leafMemPoolCount, MemPool, MemManagerDummy> LeafMemPools;

			static const size_t internalNodeSize =
				sizeof(Node) + sizeof(Item) * (maxCapacity - 1) + internalOffset;

		public:
			explicit Params(MemManager& memManager)
				: mInternalMemPool(MemPoolParams(internalNodeSize), MemManagerPtr(memManager))
			{
				for (size_t i = 0; i < leafMemPoolCount; ++i)
				{
					size_t capacity = maxCapacity - i * capacityStep;
					size_t leafNodeSize = sizeof(Node) + sizeof(Item) * (capacity - 1);
					mLeafMemPools.AddBackNogrow(MemPool(MemPoolParams(leafNodeSize),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() MOMO_NOEXCEPT
			{
				return mInternalMemPool.GetMemManager().GetBaseMemManager();
			}

			MemPool& GetInternalMemPool() MOMO_NOEXCEPT
			{
				return mInternalMemPool;
			}

			MemPool& GetLeafMemPool(size_t leafMemPoolIndex) MOMO_NOEXCEPT
			{
				return mLeafMemPools[leafMemPoolIndex];
			}

			void MergeFrom(Params& params) MOMO_NOEXCEPT
			{
				mInternalMemPool.MergeFrom(params.mInternalMemPool);
				for (size_t i = 0; i < leafMemPoolCount; ++i)
					mLeafMemPools[i].MergeFrom(params.mLeafMemPools[i]);
			}

		private:
			MemPool mInternalMemPool;
			LeafMemPools mLeafMemPools;
		};

	public:
		Node() = delete;

		Node(const Node&) = delete;

		~Node() = delete;

		Node& operator=(const Node&) = delete;

		static Node* Create(Params& params, bool isLeaf, size_t count)
		{
			MOMO_ASSERT(count <= maxCapacity);
			Node* node;
			if (isLeaf)
			{
				size_t leafMemPoolIndex = (maxCapacity - count) / capacityStep;
				if (leafMemPoolIndex >= leafMemPoolCount)
					leafMemPoolIndex = leafMemPoolCount - 1;
				node = params.GetLeafMemPool(leafMemPoolIndex).template Allocate<Node>();
				node->mMemPoolIndex = (uint8_t)leafMemPoolIndex;
			}
			else
			{
				char* ptr = params.GetInternalMemPool().template Allocate<char>();
				node = reinterpret_cast<Node*>(ptr + internalOffset);
				node->mMemPoolIndex = (uint8_t)leafMemPoolCount;
			}
			node->mParent = nullptr;
			node->mCounter.count = (uint8_t)count;
			node->pvInitIndices(IsContinuous());
			return node;
		}

		void Destroy(Params& params) MOMO_NOEXCEPT
		{
			if (IsLeaf())
				params.GetLeafMemPool((size_t)mMemPoolIndex).Deallocate(this);
			else
				params.GetInternalMemPool().Deallocate(reinterpret_cast<char*>(this) - internalOffset);
		}

		bool IsLeaf() const MOMO_NOEXCEPT
		{
			return (size_t)mMemPoolIndex < leafMemPoolCount;
		}

		size_t GetCapacity() const MOMO_NOEXCEPT
		{
			size_t capacity = maxCapacity;
			if (IsLeaf())
				capacity -= capacityStep * (size_t)mMemPoolIndex;
			return capacity;
		}

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return (size_t)mCounter.count;
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
			MOMO_ASSERT(index <= GetCount());
			return pvGetChildren()[index];
		}

		void SetChild(size_t index, Node* child) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(index <= GetCount());
			pvGetChildren()[index] = child;
		}

		size_t GetChildIndex(const Node* child) const MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			Node* const* children = &mParent - maxCapacity - 1;	//?
			size_t index = std::find(children, children + count + 1, child) - children;
			MOMO_ASSERT(index <= count);
			return index;
		}

		Item* GetItemPtr(size_t index) MOMO_NOEXCEPT
		{
			return pvGetItemPtr(index, IsContinuous());
		}

		void AcceptBackItem(Params& params, size_t index) MOMO_NOEXCEPT
		{
			size_t count = GetCount();
			MOMO_ASSERT(count < GetCapacity());
			MOMO_ASSERT(index <= count);
			pvAcceptBackItem(params, index, count, IsContinuous());
			if (!IsLeaf())
			{
				Node** children = pvGetChildren();
				memmove(children + index + 2, children + index + 1, (count - index) * sizeof(void*));
			}
			++mCounter.count;
		}

		template<typename ItemRemover>
		void Remove(Params& params, size_t index, ItemRemover&& itemRemover)
		{
			size_t count = GetCount();
			MOMO_ASSERT(index < count);
			pvRemove(params, index, count, std::forward<ItemRemover>(itemRemover), IsContinuous());
			if (!IsLeaf())
			{
				Node** children = pvGetChildren();
				memmove(children + index, children + index + 1, (count - index) * sizeof(void*));
			}
			--mCounter.count;
		}

	private:
		Node** pvGetChildren() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsLeaf());
			return &mParent - maxCapacity - 1;
		}

		void pvInitIndices(std::true_type /*isContinuous*/) MOMO_NOEXCEPT
		{
		}

		void pvInitIndices(std::false_type /*isContinuous*/) MOMO_NOEXCEPT
		{
			for (size_t i = 0; i < maxCapacity; ++i)
				mCounter.indices[i] = (uint8_t)i;
		}

		Item* pvGetItemPtr(size_t index, std::true_type /*isContinuous*/) MOMO_NOEXCEPT
		{
			return &mFirstItem + index;
		}

		Item* pvGetItemPtr(size_t index, std::false_type /*isContinuous*/) MOMO_NOEXCEPT
		{
			return &mFirstItem + mCounter.indices[index];
		}

		void pvAcceptBackItem(Params& params, size_t index, size_t count,
			std::true_type /*isContinuous*/) MOMO_NOEXCEPT
		{
			ItemTraits::ShiftNothrow(params.GetMemManager(),
				std::reverse_iterator<Item*>(GetItemPtr(count + 1)), count - index);
		}

		void pvAcceptBackItem(Params& /*params*/, size_t index, size_t count,
			std::false_type /*isContinuous*/) MOMO_NOEXCEPT
		{
			uint8_t realIndex = mCounter.indices[count];
			memmove(mCounter.indices + index + 1, mCounter.indices + index, count - index);
			mCounter.indices[index] = realIndex;
		}

		template<typename ItemRemover>
		void pvRemove(Params& params, size_t index, size_t count, ItemRemover&& itemRemover,
			std::true_type /*isContinuous*/)
		{
			ItemTraits::ShiftNothrow(params.GetMemManager(), GetItemPtr(index), count - index - 1);
			try
			{
				std::forward<ItemRemover>(itemRemover)(*GetItemPtr(count - 1));
			}
			catch (...)
			{
				ItemTraits::ShiftNothrow(params.GetMemManager(),
					std::reverse_iterator<Item*>(GetItemPtr(count)), count - index - 1);
				throw;
			}
		}

		template<typename ItemRemover>
		void pvRemove(Params& /*params*/, size_t index, size_t count, ItemRemover&& itemRemover,
			std::false_type /*isContinuous*/)
		{
			std::forward<ItemRemover>(itemRemover)(*GetItemPtr(index));
			uint8_t realIndex = mCounter.indices[index];
			memmove(mCounter.indices + index, mCounter.indices + index + 1, count - index - 1);
			mCounter.indices[count - 1] = realIndex;
		}

	private:
		Node* mParent;
		uint8_t mMemPoolIndex;
		Counter<maxCapacity, !isContinuous> mCounter;
		ItemBuffer mFirstItem;
	};
}

template<size_t tMaxCapacity, size_t tCapacityStep,
	typename TMemPoolParams = MemPoolParams<(tMaxCapacity < 64) ? 32 : 1>,
	bool tIsContinuous = true>
struct TreeNode
{
	static const size_t maxCapacity = tMaxCapacity;
	static const size_t capacityStep = tCapacityStep;
	static const bool isContinuous = tIsContinuous;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits>
	using Node = internal::Node<ItemTraits, maxCapacity, capacityStep, MemPoolParams,
		isContinuous && ItemTraits::isNothrowShiftable>;
};

} // namespace momo
