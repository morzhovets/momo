/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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
	template<size_t maxCapacity>
	concept conceptNodeCapacity = (0 < maxCapacity && maxCapacity < 256);

	template<typename TItemTraits, size_t tMaxCapacity, size_t tCapacityStep,
		conceptMemPoolParamsBlockSizeAlignment TMemPoolParams, bool tIsContinuous>
	requires conceptNodeCapacity<tMaxCapacity> &&
		(!tIsContinuous || TItemTraits::isNothrowShiftable)
	class Node
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t capacityStep = (tCapacityStep > 0) ? tCapacityStep : tMaxCapacity;

		static const bool isContinuous = tIsContinuous;

	public:
		static const size_t maxCapacity = tMaxCapacity;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

	private:
		template<size_t indexCount = isContinuous ? 0 : maxCapacity>
		struct Counter
		{
			uint8_t count;
			uint8_t indexes[indexCount];
		};

		template<size_t indexCount>	// gcc
		requires (indexCount == 0)
		struct Counter<indexCount>
		{
			uint8_t count;
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

			~Params() noexcept = default;

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
				std::byte* internalBuffer = params.GetInternalMemPool().template Allocate<std::byte>();
				void* nodeBuffer = internalBuffer + internalOffset;
				Node* node = ::new(nodeBuffer) Node(leafMemPoolCount, count);
				std::uninitialized_default_construct_n(
					pvGetChildren<false>(node), maxCapacity + 1);
				return node;
			}
		}

		void Destroy(Params& params) noexcept
		{
			if (IsLeaf())
				params.GetLeafMemPool(size_t{mMemPoolIndex}).Deallocate(this);
			else
				params.GetInternalMemPool().Deallocate(pvGetInternalBuffer(this));
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
			return pvGetChildren(this)[index];
		}

		void SetChild(size_t index, Node* child) noexcept
		{
			MOMO_ASSERT(index <= GetCount());
			pvGetChildren(this)[index] = child;
		}

		size_t GetChildIndex(const Node* child) const noexcept
		{
			size_t count = GetCount();
			Node* const* children = pvGetChildren(this);
			size_t index = UIntMath<>::Dist(children,
				std::find(children, children + count + 1, child));
			MOMO_ASSERT(index <= count);
			return index;
		}

		Item* GetItemPtr(size_t index) noexcept
		{
			static const size_t itemOffset = UIntMath<>::Ceil(sizeof(Node), ItemTraits::alignment);
			Item* items = PtrCaster::FromBytePtr<Item>(PtrCaster::ToBytePtr(this) + itemOffset);
			if constexpr (isContinuous)
				return items + index;
			else
				return items + mCounter.indexes[index];
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
				Node** children = pvGetChildren(this);
				std::copy_backward(children + index + 1, children + count + 1,
					children + count + 2);
			}
			++mCounter.count;
		}

		template<conceptObjectRemover<Item> ItemRemover>
		void Remove(Params& params, size_t index, FastMovableFunctor<ItemRemover> itemRemover)
		{
			size_t count = GetCount();
			MOMO_ASSERT(index < count);
			if constexpr (isContinuous)
			{
				ItemTraits::ShiftNothrow(params.GetMemManager(), GetItemPtr(index), count - index - 1);
				std::reverse_iterator<Item*> revIter(GetItemPtr(count));
				Finalizer shiftFin(&ItemTraits::template ShiftNothrow<decltype(revIter)>,
					params.GetMemManager(), revIter, count - index - 1);
				std::move(itemRemover)(*revIter);
				shiftFin.Detach();
			}
			else
			{
				std::move(itemRemover)(*GetItemPtr(index));
				uint8_t realIndex = mCounter.indexes[index];
				std::copy(mCounter.indexes + index + 1, mCounter.indexes + count,
					mCounter.indexes + index);
				mCounter.indexes[count - 1] = realIndex;
			}
			if (!IsLeaf())
			{
				Node** children = pvGetChildren(this);
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

		~Node() noexcept = default;

		static size_t pvGetLeafMemPoolIndex(Params& params, size_t count) noexcept
		{
			if (params.GetInternalMemPool().GetAllocateCount() <= 1 && MemPoolParams::blockCount > 1)
				return 0;
			size_t leafMemPoolIndex = (maxCapacity - count) / capacityStep;
			if (leafMemPoolIndex >= leafMemPoolCount)
				leafMemPoolIndex = leafMemPoolCount - 1;
			return leafMemPoolIndex;
		}

		template<bool isWithinLifetime = true,
			typename QNode>
		static ConstLike<Node*, QNode>* pvGetChildren(QNode* node) noexcept
		{
			MOMO_ASSERT(!node->IsLeaf());
			return PtrCaster::FromBytePtr<Node*, isWithinLifetime>(pvGetInternalBuffer(node));
		}

		template<typename QNode>
		static ConstLike<std::byte, QNode>* pvGetInternalBuffer(QNode* node) noexcept
		{
			return PtrCaster::ToBytePtr(node) - internalOffset;
		}

	private:
		//Node*[maxCapacity + 1] // for internal nodes
		Node* mParent;
		uint8_t mMemPoolIndex;
		Counter<> mCounter;
		//Item[]
	};
}

template<size_t tMaxCapacity = 32,
	size_t tCapacityStep = (tMaxCapacity >= 16) ? tMaxCapacity / 8 : 2,
	internal::conceptMemPoolParamsBlockSizeAlignment TMemPoolParams
		= MemPoolParams<(tMaxCapacity < 64) ? 8 : 1>,
	bool tIsContinuous = true>
requires internal::conceptNodeCapacity<tMaxCapacity>
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
