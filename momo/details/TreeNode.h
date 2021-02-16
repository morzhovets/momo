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
		static_assert(0 < maxCapacity && maxCapacity < 256);

		static const size_t capacityStep = tCapacityStep;
		static_assert(capacityStep > 0);

		static const bool isContinuous = tIsContinuous;
		static_assert(!isContinuous || ItemTraits::isNothrowShiftable);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

	private:
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

		static const size_t internalOffset = UIntMath<>::Ceil(sizeof(Node*) * (maxCapacity + 1),
			UIntConst::maxAlignment);

		static const size_t internalNodeSize = sizeof(Node*) + 2 + (isContinuous ? 0 : maxCapacity)
			+ sizeof(Item) * maxCapacity + internalOffset;
		typedef MemPoolParamsStatic<internalNodeSize, UIntConst::maxAlignment,
			MemPoolParams::blockCount, MemPoolParams::cachedFreeBlockCount> InternalMemPoolParams;

		typedef MemPool<InternalMemPoolParams, MemManagerPtr, NestedMemPoolSettings> InternalMemPool;
		typedef MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> LeafMemPool;

		static const size_t leafMemPoolCount = maxCapacity / (2 * capacityStep) + 1;

	public:
		class Params
		{
		private:
			typedef NestedArrayIntCap<leafMemPoolCount, LeafMemPool, MemManagerDummy> LeafMemPools;

		public:
			explicit Params(MemManager& memManager)
				: mInternalMemPool(MemManagerPtr(memManager))
			{
				for (size_t i = 0; i < leafMemPoolCount; ++i)
				{
					size_t capacity = maxCapacity - i * capacityStep;
					size_t leafNodeSize = sizeof(Node) + sizeof(Item) * (capacity - 1);
					mLeafMemPools.AddBackNogrow(LeafMemPool(MemPoolParams(leafNodeSize),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() = default;

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() noexcept
			{
				return mInternalMemPool.GetMemManager().GetBaseMemManager();
			}

			InternalMemPool& GetInternalMemPool() noexcept
			{
				return mInternalMemPool;
			}

			LeafMemPool& GetLeafMemPool(size_t leafMemPoolIndex) noexcept
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
			InternalMemPool mInternalMemPool;
			LeafMemPools mLeafMemPools;
		};

	public:
		Node() = delete;

		Node(const Node&) = delete;

		Node& operator=(const Node&) = delete;

		static Node* Create(Params& params, bool isLeaf, size_t count)
		{
			MOMO_ASSERT(count <= maxCapacity);
			Node* node;
			if (isLeaf)
			{
				size_t leafMemPoolIndex = pvGetLeafMemPoolIndex(params, count);
				node = params.GetLeafMemPool(leafMemPoolIndex).template Allocate<Node>();
				::new(static_cast<void*>(node)) Node(leafMemPoolIndex, count);
			}
			else
			{
				void* ptr = params.GetInternalMemPool().Allocate();
				node = PtrCaster::Shift<Node>(ptr, internalOffset);
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
					PtrCaster::Shift<void>(this, -static_cast<ptrdiff_t>(internalOffset)));
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
			if constexpr (isContinuous)
				return &mFirstItem + index;
			else
				return &mFirstItem + mCounter.indexes[index];
		}

		void AcceptBackItem(Params& params, size_t index) noexcept
		{
			size_t count = GetCount();
			MOMO_ASSERT(count < GetCapacity());
			MOMO_ASSERT(index <= count);
			if constexpr (isContinuous)
			{
				ItemTraits::ShiftNothrow(params.GetMemManager(),
					std::reverse_iterator<Item*>(GetItemPtr(count + 1)), count - index);
			}
			else
			{
				uint8_t realIndex = mCounter.indexes[count];
				std::copy_backward(mCounter.indexes + index, mCounter.indexes + count,
					mCounter.indexes + count + 1);
				mCounter.indexes[index] = realIndex;
			}
			if (!IsLeaf())
			{
				Node** children = pvGetChildren();
				std::copy_backward(children + index + 1, children + count + 1,
					children + count + 2);
			}
			++mCounter.count;
		}

		template<typename ItemRemover>
		requires std::invocable<ItemRemover&&, Item&>
		void Remove(Params& params, size_t index, ItemRemover&& itemRemover)
		{
			size_t count = GetCount();
			MOMO_ASSERT(index < count);
			if constexpr (isContinuous)
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
			else
			{
				std::forward<ItemRemover>(itemRemover)(*GetItemPtr(index));
				uint8_t realIndex = mCounter.indexes[index];
				std::copy(mCounter.indexes + index + 1, mCounter.indexes + count,
					mCounter.indexes + index);
				mCounter.indexes[count - 1] = realIndex;
			}
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
			if constexpr (!isContinuous)
			{
				for (size_t i = 0; i < maxCapacity; ++i)
					mCounter.indexes[i] = static_cast<uint8_t>(i);
			}
		}

		~Node() = default;

		static size_t pvGetLeafMemPoolIndex(Params& params, size_t count) noexcept
		{
			if (params.GetInternalMemPool().GetAllocateCount() <= 1 && MemPoolParams::blockCount > 1)
				return 0;
			size_t leafMemPoolIndex = (maxCapacity - count) / capacityStep;
			if (leafMemPoolIndex >= leafMemPoolCount)
				leafMemPoolIndex = leafMemPoolCount - 1;
			return leafMemPoolIndex;
		}

		Node** pvGetChildren() noexcept
		{
			MOMO_ASSERT(!IsLeaf());
			return &mParent - maxCapacity - 1;
		}

	private:
		Node* mParent;
		uint8_t mMemPoolIndex;
		Counter<maxCapacity, !isContinuous> mCounter;
		ItemBuffer mFirstItem;
	};
}

template<size_t tMaxCapacity = 32,
	size_t tCapacityStep = (tMaxCapacity >= 16) ? tMaxCapacity / 8 : 2,
	typename TMemPoolParams = MemPoolParams<(tMaxCapacity < 64) ? 8 : 1>,
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
