/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/details/TreeNode.h

  namespace momo:
    class TreeNode

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_DETAILS_TREE_NODE
#define MOMO_INCLUDE_GUARD_DETAILS_TREE_NODE

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

		static const size_t capacityStep = (tCapacityStep > 0) ? tCapacityStep : tMaxCapacity;

		static const bool isContinuous = tIsContinuous;
		MOMO_STATIC_ASSERT(!isContinuous || ItemTraits::isNothrowShiftable);

	public:
		static const size_t maxCapacity = tMaxCapacity;
		MOMO_STATIC_ASSERT(0 < maxCapacity && maxCapacity < 256);

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

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		static const size_t internalOffset = UIntMath<>::Ceil(sizeof(Node*) * (maxCapacity + 1),
			UIntConst::maxAlignment);

		static const size_t leafMemPoolCount = maxCapacity / (2 * capacityStep) + 1;

	public:
		class Params
		{
		private:
			static const size_t itemOffset = UIntMath<>::Ceil(sizeof(Node), ItemTraits::alignment);
			static const size_t internalNodeSize =
				internalOffset + itemOffset + sizeof(Item) * maxCapacity;

			typedef MemPoolParamsStatic<internalNodeSize, UIntConst::maxAlignment,
				MemPoolParams::blockCount, MemPoolParams::cachedFreeBlockCount> InternalMemPoolParams;

		public:
			typedef MemPool<InternalMemPoolParams, MemManagerPtr, NestedMemPoolSettings> InternalMemPool;
			typedef MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> LeafMemPool;

		private:
			typedef NestedArrayIntCap<leafMemPoolCount, LeafMemPool, MemManagerDummy> LeafMemPools;

		public:
			explicit Params(MemManager& memManager)
				: mInternalMemPool(MemManagerPtr(memManager))
			{
				for (size_t i = 0; i < leafMemPoolCount; ++i)
				{
					size_t capacity = maxCapacity - i * capacityStep;
					size_t leafNodeSize = itemOffset + sizeof(Item) * capacity;
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
			if (isLeaf)
			{
				size_t leafMemPoolIndex = pvGetLeafMemPoolIndex(params, count);
				void* nodeBuffer = params.GetLeafMemPool(leafMemPoolIndex).Allocate();
				return ::new(nodeBuffer) Node(leafMemPoolIndex, count);
			}
			else
			{
				void* internalBuffer = params.GetInternalMemPool().Allocate();
				void* nodeBuffer = PtrCaster::Shift<void>(internalBuffer, internalOffset);
				Node* node = ::new(nodeBuffer) Node(leafMemPoolCount, count);
				std::uninitialized_fill_n(node->pvGetChildren(), maxCapacity + 1, nullptr);
				return node;
			}
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
				params.GetInternalMemPool().Deallocate(pvGetInternalBuffer<void>(this));
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
			Node* const* children = pvGetChildren();
			size_t index = UIntMath<>::Dist(children,
				std::find(children, children + count + 1, child));
			MOMO_ASSERT(index <= count);
			return index;
		}

		Item* GetItemPtr(size_t index) noexcept
		{
			static const size_t itemOffset = UIntMath<>::Ceil(sizeof(Node), ItemTraits::alignment);
			Item* items = PtrCaster::Shift<Item>(this, itemOffset);
			return pvGetItemPtr(items, index, IsContinuous());
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

		Node* const* pvGetChildren() const noexcept
		{
			MOMO_ASSERT(!IsLeaf());
			return pvGetInternalBuffer<Node* const>(this);
		}

		Node** pvGetChildren() noexcept
		{
			MOMO_ASSERT(!IsLeaf());
			return pvGetInternalBuffer<Node*>(this);
		}

		void pvInitIndexes(std::true_type /*isContinuous*/) noexcept
		{
		}

		void pvInitIndexes(std::false_type /*isContinuous*/) noexcept
		{
			for (size_t i = 0; i < maxCapacity; ++i)
				mCounter.indexes[i] = static_cast<uint8_t>(i);
		}

		Item* pvGetItemPtr(Item* items, size_t index, std::true_type /*isContinuous*/) noexcept
		{
			return items + index;
		}

		Item* pvGetItemPtr(Item* items, size_t index, std::false_type /*isContinuous*/) noexcept
		{
			return items + mCounter.indexes[index];
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

		template<typename Result, typename Node>
		static Result* pvGetInternalBuffer(Node* node) noexcept
		{
			return PtrCaster::Shift<Result>(node, -static_cast<ptrdiff_t>(internalOffset));
		}

	private:
		//Node*[maxCapacity + 1] // for internal nodes
		Node* mParent;
		uint8_t mMemPoolIndex;
		Counter<maxCapacity, !isContinuous> mCounter;
		//Item[]
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

public:
	static size_t GetSplitItemIndex(size_t itemCount, size_t newItemIndex) noexcept
	{
		size_t splitItemIndex = itemCount / 2;
		if (itemCount % 2 == 0 && splitItemIndex > newItemIndex)
			--splitItemIndex;
		return splitItemIndex;
	}
};

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_DETAILS_TREE_NODE
