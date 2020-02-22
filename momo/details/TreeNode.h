/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/TreeNode.h

  namespace momo:
    class TreeNode

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

		template<size_t capacity, bool hasIndexes>
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
			uint8_t indexes[capacity];
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

			~Params() noexcept
			{
			}

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() noexcept
			{
				return mInternalMemPool.GetMemManager().GetBaseMemManager();
			}

			MemPool& GetInternalMemPool() noexcept
			{
				return mInternalMemPool;
			}

			MemPool& GetLeafMemPool(size_t leafMemPoolIndex) noexcept
			{
				return mLeafMemPools[leafMemPoolIndex];
			}

			void MergeFrom(Params& params) noexcept
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
		Node(const Node&) = delete;

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
				::new(static_cast<void*>(node)) Node(leafMemPoolIndex, count);
			}
			else
			{
				void* ptr = params.GetInternalMemPool().Allocate();
				node = BitCaster::PtrToPtr<Node>(ptr, internalOffset);
				::new(static_cast<void*>(node)) Node(leafMemPoolCount, count);
			}
			return node;
		}

		void Destroy(Params& params) noexcept
		{
			if (IsLeaf())
			{
				size_t memPoolIndex = size_t{mMemPoolIndex};
				this->~Node();
				params.GetLeafMemPool(memPoolIndex).Deallocate(this);
			}
			else
			{
				this->~Node();
				params.GetInternalMemPool().Deallocate(
					BitCaster::PtrToPtr<void>(this, -static_cast<ptrdiff_t>(internalOffset)));
			}
		}

		bool IsLeaf() const noexcept
		{
			return size_t{mMemPoolIndex} < leafMemPoolCount;
		}

		size_t GetCapacity() const noexcept
		{
			size_t capacity = maxCapacity;
			if (IsLeaf())
				capacity -= capacityStep * size_t{mMemPoolIndex};
			return capacity;
		}

		size_t GetCount() const noexcept
		{
			return size_t{mCounter.count};
		}

		Node* GetParent() noexcept
		{
			return mParent;
		}

		void SetParent(Node* parent) noexcept
		{
			mParent = parent;
		}

		Node* GetChild(size_t index) noexcept
		{
			MOMO_ASSERT(index <= GetCount());
			return pvGetChildren()[index];
		}

		void SetChild(size_t index, Node* child) noexcept
		{
			MOMO_ASSERT(index <= GetCount());
			pvGetChildren()[index] = child;
		}

		size_t GetChildIndex(const Node* child) const noexcept
		{
			size_t count = GetCount();
			Node* const* children = &mParent - maxCapacity - 1;	//?
			size_t index = UIntMath<>::Dist(children,
				std::find(children, children + count + 1, child));
			MOMO_ASSERT(index <= count);
			return index;
		}

		Item* GetItemPtr(size_t index) noexcept
		{
			return pvGetItemPtr(index, IsContinuous());
		}

		void AcceptBackItem(Params& params, size_t index) noexcept
		{
			size_t count = GetCount();
			MOMO_ASSERT(count < GetCapacity());
			MOMO_ASSERT(index <= count);
			pvAcceptBackItem(params, index, count, IsContinuous());
			if (!IsLeaf())
			{
				Node** children = pvGetChildren();
				std::copy_backward(children + index + 1, children + count + 1,
					children + count + 2);
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
				std::copy(children + index + 1, children + count + 1, children + index);
			}
			--mCounter.count;
		}

	private:
		explicit Node(size_t memPoolIndex, size_t count) noexcept
			: mParent(nullptr),
			mMemPoolIndex(static_cast<uint8_t>(memPoolIndex))
		{
			mCounter.count = static_cast<uint8_t>(count);
			pvInitIndexes(IsContinuous());
		}

		~Node() noexcept
		{
		}

		Node** pvGetChildren() noexcept
		{
			MOMO_ASSERT(!IsLeaf());
			return &mParent - maxCapacity - 1;
		}

		void pvInitIndexes(std::true_type /*isContinuous*/) noexcept
		{
		}

		void pvInitIndexes(std::false_type /*isContinuous*/) noexcept
		{
			for (size_t i = 0; i < maxCapacity; ++i)
				mCounter.indexes[i] = static_cast<uint8_t>(i);
		}

		Item* pvGetItemPtr(size_t index, std::true_type /*isContinuous*/) noexcept
		{
			return &mFirstItem + index;
		}

		Item* pvGetItemPtr(size_t index, std::false_type /*isContinuous*/) noexcept
		{
			return &mFirstItem + mCounter.indexes[index];
		}

		void pvAcceptBackItem(Params& params, size_t index, size_t count,
			std::true_type /*isContinuous*/) noexcept
		{
			ItemTraits::ShiftNothrow(params.GetMemManager(),
				std::reverse_iterator<Item*>(GetItemPtr(count + 1)), count - index);
		}

		void pvAcceptBackItem(Params& /*params*/, size_t index, size_t count,
			std::false_type /*isContinuous*/) noexcept
		{
			uint8_t realIndex = mCounter.indexes[count];
			std::copy_backward(mCounter.indexes + index, mCounter.indexes + count,
				mCounter.indexes + count + 1);
			mCounter.indexes[index] = realIndex;
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
			uint8_t realIndex = mCounter.indexes[index];
			std::copy(mCounter.indexes + index + 1, mCounter.indexes + count,
				mCounter.indexes + index);
			mCounter.indexes[count - 1] = realIndex;
		}

	private:
		Node* mParent;
		uint8_t mMemPoolIndex;
		Counter<maxCapacity, !isContinuous> mCounter;
		ItemBuffer mFirstItem;
	};
}

template<size_t tMaxCapacity, size_t tCapacityStep,
	typename TMemPoolParams = MemPoolParams<(tMaxCapacity < 64) ? 32 : 1>,	//?
	bool tIsContinuous = true>
class TreeNode
{
public:
	static const size_t maxCapacity = tMaxCapacity;
	static const size_t capacityStep = tCapacityStep;
	static const bool isContinuous = tIsContinuous;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits>
	using Node = internal::Node<ItemTraits, maxCapacity, capacityStep, MemPoolParams,
		isContinuous && ItemTraits::isNothrowShiftable>;
};

} // namespace momo
