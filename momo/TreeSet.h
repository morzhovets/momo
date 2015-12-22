/**********************************************************\

  momo/TreeSet.h

  namespace momo:
    struct TreeSetItemTraits
    struct TreeSetSettings
    class TreeSet

\**********************************************************/

#pragma once

#include "TreeTraits.h"

namespace momo
{

namespace internal
{
	template<typename TTreeNode, typename TSettings>
	class TreeSetConstIterator
	{
	public:
		typedef TTreeNode TreeNode;
		typedef TSettings Settings;
		typedef typename TreeNode::Item Item;

		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef TreeSetConstIterator ConstIterator;

	public:
		TreeSetConstIterator() MOMO_NOEXCEPT
			: mNode(nullptr),
			mItemIndex(0)
		{
		}

		TreeSetConstIterator(TreeNode& node, size_t itemIndex) MOMO_NOEXCEPT
			: mNode(&node),
			mItemIndex(itemIndex)
		{
		}

		//operator ConstIterator() const MOMO_NOEXCEPT

		TreeSetConstIterator& operator++()
		{
			MOMO_CHECK(mNode != nullptr);
			if (!mNode->IsLeaf())
			{
				MOMO_CHECK(mItemIndex < mNode->GetCount());
				mNode = mNode->GetChild(mItemIndex + 1);
				while (!mNode->IsLeaf())
					mNode = mNode->GetChild(0);
				mItemIndex = 0;
			}
			else if (mItemIndex + 1 == mNode->GetCount())
			{
				TreeNode* node = mNode;
				while (true)
				{
					TreeNode* parentNode = node->GetParent();
					if (parentNode == nullptr)
					{
						mNode = node;
						mItemIndex = node->GetCount();
						break;
					}
					size_t index = parentNode->GetChildIndex(node);
					if (index < parentNode->GetCount())
					{
						mNode = parentNode;
						mItemIndex = index;
						break;
					}
					node = parentNode;
				}
			}
			else
			{
				++mItemIndex;
			}
			return *this;
		}

		TreeSetConstIterator operator++(int)
		{
			TreeSetConstIterator tempIter = *this;
			++*this;
			return tempIter;
		}

		TreeSetConstIterator& operator--()
		{
			MOMO_CHECK(mNode != nullptr);
			if (!mNode->IsLeaf())
			{
				mNode = mNode->GetChild(mItemIndex);
				while (!mNode->IsLeaf())
					mNode = mNode->GetChild(mNode->GetCount());
				mItemIndex = mNode->GetCount() - 1;
			}
			else if (mItemIndex == 0)
			{
				TreeNode* node = mNode;
				while (true)
				{
					TreeNode* parentNode = node->GetParent();
					MOMO_CHECK(parentNode != nullptr);
					size_t index = parentNode->GetChildIndex(node);
					if (index > 0)
					{
						mNode = parentNode;
						mItemIndex = index - 1;
						break;
					}
					node = parentNode;
				}
			}
			else
			{
				--mItemIndex;
			}
			return *this;
		}

		TreeSetConstIterator operator--(int)
		{
			TreeSetConstIterator tempIter = *this;
			--*this;
			return tempIter;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mNode != nullptr);
			MOMO_CHECK(mItemIndex < mNode->GetCount());
			return mNode->GetItemPtr(mItemIndex);
		}

		Reference operator*() const
		{
			return *operator->();
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mNode == iter.mNode && mItemIndex == iter.mItemIndex;
		}

		bool operator!=(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return !(*this == iter);
		}

		TreeNode* GetNode() const MOMO_NOEXCEPT
		{
			return mNode;
		}

		size_t GetItemIndex() const MOMO_NOEXCEPT
		{
			return mItemIndex;
		}

	private:
		TreeNode* mNode;
		size_t mItemIndex;
	};
}

template<typename TKey,
	typename TItem = TKey>
struct TreeSetItemTraits
{
	typedef TKey Key;
	typedef TItem Item;

	typedef internal::ObjectManager<Item> ItemManager;

	static const size_t alignment = ItemManager::alignment;

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

	static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT((std::is_same<Item, Key>::value));
		return item;
	}

	static void Destroy(Item& item) MOMO_NOEXCEPT
	{
		ItemManager::Destroy(item);
	}

	template<typename Iterator, typename ItemCreator>
	static void RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
		const ItemCreator& itemCreator, void* pobject)
	{
		ItemManager::RelocateCreate(srcBegin, dstBegin, count, itemCreator, pobject);
	}

	static void SwapNothrow(Item& item1, Item& item2) MOMO_NOEXCEPT
	{
		internal::ObjectBuffer<Item, alignment> itemBuffer;
		ItemManager::CreateNothrow(std::move(item1), &itemBuffer);
		ItemManager::AssignNothrowAnyway(std::move(item2), item1);
		ItemManager::AssignNothrowAnyway(std::move(*&itemBuffer), item2);
	}
};

struct TreeSetSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	//static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION_VALUE;
};

template<typename TKey,
	typename TTreeTraits = TreeTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = TreeSetItemTraits<TKey>,
	typename TSettings = TreeSetSettings>
class TreeSet
{
public:
	typedef TKey Key;
	typedef TTreeTraits TreeTraits;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Item Item;

private:
	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	static const size_t nodeCapacity = TreeTraits::nodeCapacity;
	MOMO_STATIC_ASSERT(nodeCapacity > 2);

	typedef internal::TreeNode<ItemTraits, MemManager, nodeCapacity> Node;
	typedef typename Node::Params NodeParams;

	class Crew
	{
		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	private:
		struct Data
		{
			size_t version;
			TreeTraits treeTraits;
			NodeParams nodeParams;
			MemManager memManager;
		};

	public:
		Crew(const TreeTraits& treeTraits, MemManager&& memManager)
		{
			mData = (Data*)memManager.Allocate(sizeof(Data));
			mData->version = 0;
			new(&mData->memManager) MemManager(std::move(memManager));
			try
			{
				new(&mData->treeTraits) TreeTraits(treeTraits);
				try
				{
					new(&mData->nodeParams) NodeParams(mData->memManager);
				}
				catch (...)
				{
					mData->treeTraits.~TreeTraits();
					throw;
				}
			}
			catch (...)
			{
				MemManager dataMemManager = std::move(mData->memManager);
				mData->memManager.~MemManager();
				dataMemManager.Deallocate(mData, sizeof(Data));
				throw;
			}
		}

		Crew(Crew&& crew) MOMO_NOEXCEPT
			: mData(nullptr)
		{
			Swap(crew);
		}

		Crew(const Crew&) = delete;

		~Crew() MOMO_NOEXCEPT
		{
			if (!_IsNull())
			{
				mData->nodeParams.~NodeParams();
				mData->treeTraits.~TreeTraits();
				MemManager memManager = std::move(mData->memManager);
				mData->memManager.~MemManager();
				memManager.Deallocate(mData, sizeof(Data));
			}
		}

		Crew& operator=(Crew&& crew) MOMO_NOEXCEPT
		{
			Crew(std::move(crew)).Swap(*this);
			return *this;
		}

		Crew& operator=(const Crew&) = delete;

		void Swap(Crew& crew) MOMO_NOEXCEPT
		{
			std::swap(mData, crew.mData);
		}

		const size_t& GetVersion() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->version;
		}

		size_t& GetVersion() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->version;
		}

		const NodeParams& GetNodeParams() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->nodeParams;
		}

		NodeParams& GetNodeParams() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->nodeParams;
		}

		const TreeTraits& GetTreeTraits() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->treeTraits;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->memManager;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->memManager;
		}

	private:
		bool _IsNull() const MOMO_NOEXCEPT
		{
			return mData == nullptr;
		}

	private:
		Data* mData;
	};

public:
	typedef internal::TreeSetConstIterator<Node, Settings> ConstIterator;

	typedef internal::InsertResult<ConstIterator> InsertResult;

private:
	class Relocator
	{
	private:
		struct Segment
		{
			Node* node;
			size_t beginIndex;
			size_t endIndex;
		};

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		template<typename Item>
		using Array4 = Array<Item, MemManagerPtr, ArrayItemTraits<Item>, ArraySettings<4>>;

		typedef Array4<Node*> Nodes;
		typedef Array4<Segment> Segments;

		class Iterator
		{
		public:
			typedef typename Segments::Iterator SegmentIterator;

		public:
			explicit Iterator(SegmentIterator segmentIter) MOMO_NOEXCEPT
				: mSegmentIter(segmentIter),
				mItemIndex(segmentIter->beginIndex)
			{
			}

			Iterator& operator++()
			{
				++mItemIndex;
				if (mItemIndex == mSegmentIter->endIndex)
				{
					++mSegmentIter;
					mItemIndex = mSegmentIter->beginIndex;
				}
				return *this;
			}

			Item& operator*() const
			{
				return *mSegmentIter->node->GetItemPtr(mItemIndex);
			}

		private:
			SegmentIterator mSegmentIter;
			size_t mItemIndex;
		};

	public:
		Relocator(MemManager& memManager, NodeParams& nodeParams) MOMO_NOEXCEPT
			: mNodeParams(nodeParams),
			mOldNodes(MemManagerPtr(memManager)),
			mNewNodes(MemManagerPtr(memManager)),
			mSrcSegments(MemManagerPtr(memManager)),
			mDstSegments(MemManagerPtr(memManager)),
			mItemCount(0)
		{
		}

		Relocator(const Relocator&) = delete;
		
		~Relocator() MOMO_NOEXCEPT
		{
			for (Node* node : mNewNodes)
				node->Destroy(mNodeParams);
		}

		Relocator& operator=(const Relocator&) = delete;

		Node* CreateNewNode(bool isLeaf)
		{
			mNewNodes.Reserve(mNewNodes.GetCount() + 1);
			Node* node = &Node::Create(mNodeParams, isLeaf);
			mNewNodes.AddBackNogrow(node);
			return node;
		}

		void AddSegment(Node* srcNode, size_t srcBeginIndex,
			Node* dstNode, size_t dstBeginIndex, size_t itemCount)
		{
			if (itemCount > 0)
			{
				mSrcSegments.AddBack({ srcNode, srcBeginIndex, srcBeginIndex + itemCount });
				mDstSegments.AddBack({ dstNode, dstBeginIndex, dstBeginIndex + itemCount });
				mItemCount += itemCount;
			}
		}

		ConstIterator SplitLeafNode(Node* oldNode, size_t middleIndex, size_t itemIndex,
			Node* newNode1, Node* newNode2)
		{
			mOldNodes.AddBack(oldNode);
			size_t itemCount = oldNode->GetCount();
			if (itemIndex <= middleIndex)
			{
				newNode1->SetCount(middleIndex + 1);
				newNode2->SetCount(itemCount - middleIndex - 1);
				AddSegment(oldNode, 0, newNode1, 0, itemIndex);
				AddSegment(oldNode, itemIndex, newNode1, itemIndex + 1, middleIndex - itemIndex);
				AddSegment(oldNode, middleIndex + 1, newNode2, 0, itemCount - middleIndex - 1);
				return ConstIterator(*newNode1, itemIndex);
			}
			else
			{
				newNode1->SetCount(middleIndex);
				newNode2->SetCount(itemCount - middleIndex);
				AddSegment(oldNode, 0, newNode1, 0, middleIndex);
				AddSegment(oldNode, middleIndex + 1, newNode2, 0, itemIndex - middleIndex - 1);
				AddSegment(oldNode, itemIndex, newNode2, itemIndex - middleIndex, itemCount - itemIndex);
				return ConstIterator(*newNode2, itemIndex - middleIndex - 1);
			}
		}

		ConstIterator SplitInternalNode(Node* oldNode, size_t middleIndex, size_t itemIndex,
			Node* newNode1, Node* newNode2)
		{
			ConstIterator iter = SplitLeafNode(oldNode, middleIndex, itemIndex, newNode1, newNode2);
			size_t itemCount = oldNode->GetCount();
			Node* newNode = newNode1;
			size_t newIndex = 0;
			for (size_t i = 0; i <= itemCount; ++i)
			{
				if (i != itemIndex)
					newNode->SetChild(newIndex, oldNode->GetChild(i));
				newIndex += (i == itemIndex) ? 2 : 1;
				if (i == middleIndex)
				{
					newNode = newNode2;
					newIndex = 0;
				}
			}
			return iter;
		}

		template<typename ItemCreator>
		void RelocateCreate(const ItemCreator& itemCreator, Item* pitem)
		{
			mSrcSegments.AddBack({ nullptr, 0, 0 });
			mDstSegments.AddBack({ nullptr, 0, 0 });
			ItemTraits::RelocateCreate(Iterator(mSrcSegments.GetBegin()),
				Iterator(mDstSegments.GetBegin()), mItemCount, itemCreator, pitem);
			mSrcSegments.Clear();
			mDstSegments.Clear();
			mItemCount = 0;
			mNewNodes.Swap(mOldNodes);
		}

	private:
		NodeParams& mNodeParams;
		Nodes mOldNodes;
		Nodes mNewNodes;
		Segments mSrcSegments;
		Segments mDstSegments;
		size_t mItemCount;
	};

public:
	explicit TreeSet(const TreeTraits& treeTraits = TreeTraits(),
		MemManager&& memManager = MemManager())
		: mCrew(treeTraits, std::move(memManager)),
		mCount(0),
		mRootNode(nullptr)
	{
	}

	TreeSet(std::initializer_list<Item> items,
		const TreeTraits& treeTraits = TreeTraits(), MemManager&& memManager = MemManager())
		: TreeSet(treeTraits, std::move(memManager))
	{
		try
		{
			Insert(items);
		}
		catch (...)
		{
			Clear();
			throw;
		}
	}

	TreeSet(TreeSet&& treeSet) MOMO_NOEXCEPT
		: mCrew(std::move(treeSet.mCrew)),
		mCount(treeSet.mCount),
		mRootNode(treeSet.mRootNode)
	{
		treeSet.mCount = 0;
		treeSet.mRootNode = nullptr;
	}

	TreeSet(const TreeSet& treeSet)
		: TreeSet(treeSet.GetTreeTraits(), MemManager(treeSet.GetMemManager()))
	{
		try
		{
			for (const Item& item : treeSet)
				Add(GetEnd(), item);
		}
		catch (...)
		{
			Clear();
			throw;
		}
	}

	~TreeSet() MOMO_NOEXCEPT
	{
		Clear();
	}

	TreeSet& operator=(TreeSet&& treeSet) MOMO_NOEXCEPT
	{
		TreeSet(std::move(treeSet)).Swap(*this);
		return *this;
	}

	TreeSet& operator=(const TreeSet& treeSet)
	{
		if (this != &treeSet)
			TreeSet(treeSet).Swap(*this);
		return *this;
	}

	void Swap(TreeSet& treeSet) MOMO_NOEXCEPT
	{
		mCrew.Swap(treeSet.mCrew);
		std::swap(mCount, treeSet.mCount);
		std::swap(mRootNode, treeSet.mRootNode);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		Node* node = mRootNode;
		while (!node->IsLeaf())
			node = node->GetChild(0);
		return ConstIterator(*node, 0);
	}
	
	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		return ConstIterator(*mRootNode, mRootNode->GetCount());
	}

	MOMO_FRIEND_SWAP(TreeSet)
	MOMO_FRIENDS_BEGIN_END(const TreeSet&, ConstIterator)

	const TreeTraits& GetTreeTraits() const MOMO_NOEXCEPT
	{
		return mCrew.GetTreeTraits();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mCrew.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mCrew.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mCount;
	}

	bool IsEmpty() const MOMO_NOEXCEPT
	{
		return mCount == 0;
	}

	void Clear() MOMO_NOEXCEPT
	{
		if (mRootNode != nullptr)
			_Clear(mRootNode);
		mCount = 0;
	}

	ConstIterator LowerBound(const Key& key) const
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		ConstIterator iter = GetEnd();
		Node* node = mRootNode;
		while (true)
		{
			size_t itemCount = node->GetCount();
			size_t leftIndex = 0;
			size_t rightIndex = itemCount;
			while (leftIndex < rightIndex)
			{
				size_t middleIndex = (leftIndex + rightIndex) / 2;
				const Item& item = *node->GetItemPtr(middleIndex);
				if (GetTreeTraits().Less(ItemTraits::GetKey(item), key))
					leftIndex = middleIndex + 1;
				else
					rightIndex = middleIndex;
			}
			if (leftIndex < itemCount)
				iter = ConstIterator(*node, leftIndex);
			if (node->IsLeaf())
				break;
			node = node->GetChild(leftIndex);
		}
		return iter;
	}

	ConstIterator UpperBound(const Key& key) const
	{
		ConstIterator iter = LowerBound(key);
		if (_IsEqual(iter, key))
			++iter;
		return iter;
	}

	ConstIterator Find(const Key& key) const
	{
		ConstIterator iter = LowerBound(key);
		return _IsEqual(iter, key) ? iter : GetEnd();
	}

	bool HasKey(const Key& key) const
	{
		return _IsEqual(LowerBound(key), key);
	}

	template<typename ItemCreator>
	InsertResult InsertCrt(const Key& key, const ItemCreator& itemCreator)
	{
		ConstIterator iter = LowerBound(key);
		if (_IsEqual(iter, key))
			return InsertResult(iter, false);
		iter = AddCrt(iter, itemCreator);
		return InsertResult(iter, true);
	}

	template<typename... ItemArgs>
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key, Creator<ItemArgs...>(std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		return InsertVar(ItemTraits::GetKey((const Item&)item), std::move(item));
	}

	InsertResult Insert(const Item& item)
	{
		return InsertVar(ItemTraits::GetKey(item), item);
	}

	template<typename Iterator>
	size_t Insert(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Item, *begin);
		size_t count = 0;
		for (Iterator iter = begin; iter != end; ++iter)
			count += Insert(*iter).inserted ? 1 : 0;
		return count;
	}

	size_t Insert(std::initializer_list<Item> items)
	{
		return Insert(items.begin(), items.end());
	}

	template<typename ItemCreator>
	ConstIterator AddCrt(ConstIterator iter, const ItemCreator& itemCreator)
	{
		if (mRootNode == nullptr)
		{
			mRootNode = &Node::Create(mCrew.GetNodeParams(), true);
			iter = GetEnd();
		}
		Node* node = iter.GetNode();
		size_t itemIndex = iter.GetItemIndex();
		if (!node->IsLeaf())
		{
			node = node->GetChild(itemIndex);
			while (!node->IsLeaf())
				node = node->GetChild(node->GetCount());
			itemIndex = node->GetCount();
		}
		ConstIterator resIter;
		size_t itemCount = node->GetCount();
		if (itemCount < nodeCapacity)
		{
			itemCreator(node->GetItemPtr(itemCount));
			node->AcceptBackItem(itemIndex);
			resIter = ConstIterator(*node, itemIndex);
		}
		else
		{
			resIter = _AddRelocate(node, itemIndex, itemCreator);
		}
		++mCount;
		return resIter;
	}

	template<typename... ItemArgs>
	ConstIterator AddVar(ConstIterator iter, ItemArgs&&... itemArgs)
	{
		return AddCrt(iter, Creator<ItemArgs...>(std::forward<ItemArgs>(itemArgs)...));
	}

	ConstIterator Add(ConstIterator iter, Item&& item)
	{
		return AddVar(iter, std::move(item));
	}

	ConstIterator Add(ConstIterator iter, const Item& item)
	{
		return AddVar(iter, item);
	}

	//ConstIterator Remove(ConstIterator iter)
	//ConstIterator Remove(ConstIterator iter, Item& resItem)
	//bool Remove(const Key& key)

	//void Reset(ConstIterator iter, Item&& newItem)
	//void Reset(ConstIterator iter, const Item& newItem)
	//void Reset(ConstIterator iter, Item&& newItem, Item& resItem)
	//void Reset(ConstIterator iter, const Item& newItem, Item& resItem)

private:
	bool _IsEqual(ConstIterator iter, const Key& key) const MOMO_NOEXCEPT
	{
		return iter != GetEnd() && !GetTreeTraits().Less(key, ItemTraits::GetKey(*iter));
	}

	void _Clear(Node* node) MOMO_NOEXCEPT
	{
		size_t count = node->GetCount();
		for (size_t i = 0; i < count; ++i)
			ItemTraits::Destroy(*node->GetItemPtr(i));
		if (!node->IsLeaf())
		{
			for (size_t i = 0; i <= count; ++i)
				_Clear(node->GetChild(i));
		}
		node->Destroy(mCrew.GetNodeParams());
	}

	template<typename ItemCreator>
	ConstIterator _AddRelocate(Node* node, size_t itemIndex, const ItemCreator& itemCreator)
	{
		Relocator relocator(GetMemManager(), mCrew.GetNodeParams());
		Node* newNode1 = relocator.CreateNewNode(true);
		Node* newNode2 = relocator.CreateNewNode(true);
		static const size_t middleIndex = nodeCapacity / 2;
		ConstIterator resIter = relocator.SplitLeafNode(node, middleIndex, itemIndex,
			newNode1, newNode2);
		while (true)
		{
			Node* parentNode = node->GetParent();
			if (parentNode == nullptr)
			{
				parentNode = relocator.CreateNewNode(false);
				itemIndex = 0;
				relocator.AddSegment(node, middleIndex, parentNode, 0, 1);
				node = parentNode;
				break;
			}
			else if (parentNode->GetCount() < nodeCapacity)
			{
				itemIndex = parentNode->GetChildIndex(node);
				relocator.AddSegment(node, middleIndex, parentNode, parentNode->GetCount(), 1);
				node = parentNode;
				break;
			}
			itemIndex = parentNode->GetChildIndex(node);
			Node* parentNewNode1 = relocator.CreateNewNode(false);
			Node* parentNewNode2 = relocator.CreateNewNode(false);
			ConstIterator parentIter = relocator.SplitInternalNode(parentNode, middleIndex,
				itemIndex, parentNewNode1, parentNewNode2);
			Node* parentNewNode = parentIter.GetNode();
			size_t parentItemIndex = parentIter.GetItemIndex();
			relocator.AddSegment(node, middleIndex, parentNewNode, parentItemIndex, 1);
			parentNewNode->SetChild(parentItemIndex, newNode1);
			parentNewNode->SetChild(parentItemIndex + 1, newNode2);
			node = parentNode;
			newNode1 = parentNewNode1;
			newNode2 = parentNewNode2;
		}
		relocator.RelocateCreate(itemCreator,
			resIter.GetNode()->GetItemPtr(resIter.GetItemIndex()));
		if (node->GetCount() == 0)
			mRootNode = node;
		node->AcceptBackItem(itemIndex);
		node->SetChild(itemIndex, newNode1);
		node->SetChild(itemIndex + 1, newNode2);
		_SetParents(node);
		return resIter;
	}

	void _SetParents(Node* node) MOMO_NOEXCEPT
	{
		size_t count = node->GetCount();
		for (size_t i = 0; i <= count; ++i)
		{
			Node* childNode = node->GetChild(i);
			childNode->SetParent(node);
			if (!childNode->IsLeaf())
				_SetParents(childNode);
		}
	}

private:
	Crew mCrew;
	size_t mCount;
	Node* mRootNode;
};

} // namespace momo

namespace std
{
	template<typename N, typename S>
	struct iterator_traits<momo::internal::TreeSetConstIterator<N, S>>
	{
		typedef bidirectional_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::TreeSetConstIterator<N, S>::Pointer pointer;
		typedef typename momo::internal::TreeSetConstIterator<N, S>::Reference reference;
		typedef typename momo::internal::TreeSetConstIterator<N, S>::Item value_type;
	};
} // namespace std
