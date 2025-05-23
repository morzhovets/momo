/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/TreeSet.h

  namespace momo:
    class TreeSetItemTraits
    class TreeSetSettings
    class TreeSet
    class TreeMultiSet

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_TREE_SET
#define MOMO_INCLUDE_GUARD_TREE_SET

#include "TreeTraits.h"
#include "SetUtility.h"
#include "IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TNode, typename TSettings>
	class TreeSetConstIterator : private VersionKeeper<TSettings>
	{
	protected:
		typedef TNode Node;
		typedef TSettings Settings;
		typedef typename Node::Item Item;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef TreeSetConstIterator ConstIterator;

	private:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		explicit TreeSetConstIterator() noexcept
			: mNode(nullptr),
			mItemIndex(0)
		{
		}

		//operator ConstIterator() const noexcept

		TreeSetConstIterator& operator++()
		{
			VersionKeeper::Check();
			MOMO_CHECK(mNode != nullptr);
			if (mNode->IsLeaf())
			{
				++mItemIndex;
			}
			else
			{
				MOMO_CHECK(mItemIndex < mNode->GetCount());
				mNode = mNode->GetChild(mItemIndex + 1);
				while (!mNode->IsLeaf())
					mNode = mNode->GetChild(0);
				mItemIndex = 0;
			}
			pvMoveIf();
			return *this;
		}

		TreeSetConstIterator& operator--()
		{
			VersionKeeper::Check();
			MOMO_CHECK(mNode != nullptr);
			Node* node = mNode;
			size_t itemIndex = mItemIndex;
			if (!node->IsLeaf())
			{
				node = node->GetChild(itemIndex);
				while (!node->IsLeaf())
					node = node->GetChild(node->GetCount());
				itemIndex = node->GetCount();
			}
			while (itemIndex == 0)
			{
				Node* childNode = node;
				node = node->GetParent();
				MOMO_CHECK(node != nullptr);
				itemIndex = node->GetChildIndex(childNode);
			}
			mNode = node;
			mItemIndex = itemIndex - 1;
			return *this;
		}

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(mNode != nullptr);
			MOMO_CHECK(mItemIndex < mNode->GetCount());
			return mNode->GetItemPtr(mItemIndex);
		}

		friend bool operator==(TreeSetConstIterator iter1, TreeSetConstIterator iter2) noexcept
		{
			return iter1.mNode == iter2.mNode && iter1.mItemIndex == iter2.mItemIndex;
		}

		MOMO_MORE_TREE_ITERATOR_OPERATORS(TreeSetConstIterator)

	protected:
		explicit TreeSetConstIterator(Node& node, size_t itemIndex, const size_t* version,
			bool move) noexcept
			: VersionKeeper(version),
			mNode(&node),
			mItemIndex(itemIndex)
		{
			if (move)
				pvMoveIf();
		}

		Node* ptGetNode() const noexcept
		{
			return mNode;
		}

		size_t ptGetItemIndex() const noexcept
		{
			return mItemIndex;
		}

		void ptCheck(const size_t* version, bool allowEmpty) const
		{
			VersionKeeper::Check(version, allowEmpty);
			MOMO_CHECK(allowEmpty || mNode != nullptr);
		}

	private:
		void pvMoveIf() noexcept
		{
			if (mItemIndex == mNode->GetCount())
				pvMove();
		}

		void pvMove() noexcept
		{
			MOMO_ASSERT(mNode->IsLeaf());
			while (true)
			{
				Node* parentNode = mNode->GetParent();
				if (parentNode == nullptr)
				{
					mItemIndex = mNode->GetCount();
					break;
				}
				Node* childNode = mNode;
				mNode = parentNode;
				if (childNode != mNode->GetChild(mNode->GetCount()))
				{
					mItemIndex = mNode->GetChildIndex(childNode);
					break;
				}
			}
		}

	private:
		Node* mNode;
		size_t mItemIndex;
	};

	template<typename TSegment>
	class TreeSetRelocatorIterator
	{
	public:
		typedef TSegment Segment;

		typedef decltype(std::declval<Segment&>().node->GetItemPtr(size_t{})) Pointer;
		typedef decltype(*Pointer()) Reference;

	public:
		explicit TreeSetRelocatorIterator(Segment* segmentPtr) noexcept
			: mSegmentPtr(segmentPtr),
			mItemIndex(segmentPtr->beginIndex)
		{
		}

		TreeSetRelocatorIterator& operator++() noexcept
		{
			++mItemIndex;
			if (mItemIndex == mSegmentPtr->endIndex)
			{
				++mSegmentPtr;
				mItemIndex = mSegmentPtr->beginIndex;
			}
			return *this;
		}

		Pointer operator->() const noexcept
		{
			return mSegmentPtr->node->GetItemPtr(mItemIndex);
		}

		Reference operator*() const noexcept
		{
			return *operator->();
		}

	private:
		Segment* mSegmentPtr;
		size_t mItemIndex;
	};

	template<typename TTreeSetItemTraits>
	class TreeSetNodeItemTraits
	{
	protected:
		typedef TTreeSetItemTraits TreeSetItemTraits;

	public:
		typedef typename TreeSetItemTraits::Item Item;
		typedef typename TreeSetItemTraits::MemManager MemManager;

		static const bool isNothrowShiftable = TreeSetItemTraits::isNothrowShiftable;

		static const size_t alignment = TreeSetItemTraits::alignment;

	public:
		template<typename Iterator>
		static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
		{
			TreeSetItemTraits::ShiftNothrow(memManager, begin, shift);
		}
	};
}

template<typename TKey, typename TMemManager>
class TreeSetItemTraits : public internal::SetItemTraits<TKey, TMemManager>
{
private:
	typedef internal::SetItemTraits<TKey, TMemManager> SetItemTraits;

public:
	using typename SetItemTraits::Item;
	using typename SetItemTraits::MemManager;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	static const bool isNothrowShiftable = ItemManager::isNothrowShiftable;

public:
	template<typename Iterator, typename ItemCreator>
	static void RelocateCreate(MemManager& memManager, Iterator srcBegin, Iterator dstBegin,
		size_t count, ItemCreator&& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcBegin, dstBegin, count,
			std::forward<ItemCreator>(itemCreator), newItem);
	}

	template<typename Iterator>
	static void ShiftNothrow(MemManager& memManager, Iterator begin, size_t shift) noexcept
	{
		ItemManager::ShiftNothrow(memManager, begin, shift);
	}
};

class TreeSetSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
};

/*!
	All `TreeSet` functions and constructors have strong exception safety,
	but not the following cases:
	1. Functions `Insert` receiving many items have basic exception safety.
	2. Function `Remove` receiving predicate has basic exception safety.
	3. Functions `MergeFrom` and `MergeTo` have basic exception safety.
*/

template<typename TKey,
	typename TTreeTraits = TreeTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = TreeSetItemTraits<TKey, TMemManager>,
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

	MOMO_STATIC_ASSERT(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));

private:
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef internal::SetCrew<TreeTraits, MemManager, Settings::checkVersion> Crew;

	typedef internal::TreeSetNodeItemTraits<ItemTraits> NodeItemTraits;

	typedef typename internal::TreeTraitsNodeSelector<TreeTraits, NodeItemTraits>::Node Node;

	typedef typename Node::Params NodeParams;

	static const size_t nodeMaxCapacity = Node::maxCapacity;
	MOMO_STATIC_ASSERT(nodeMaxCapacity > 0);

public:
	typedef internal::TreeSetConstIterator<Node, Settings> ConstIterator;
	typedef ConstIterator Iterator;

	typedef internal::InsertResult<ConstIterator> InsertResult;

	typedef internal::SetExtractedItem<ItemTraits, Settings> ExtractedItem;

private:
	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	template<typename KeyArg>
	struct IsValidKeyArg : public TreeTraits::template IsValidKeyArg<KeyArg>
	{
	};

	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetNode)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetItemIndex)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, Check)
	};

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

		typedef internal::NestedArrayIntCap<4, Node*, MemManagerPtr> Nodes;
		typedef internal::NestedArrayIntCap<4, Segment, MemManagerPtr> Segments;

		typedef internal::TreeSetRelocatorIterator<Segment> Iterator;

	public:
		struct SplitResult
		{
			Node* newNode;
			size_t newItemIndex;
			size_t splitItemIndex;
			Node* newNode1;
			Node* newNode2;
		};

	public:
		explicit Relocator(NodeParams& nodeParams) noexcept
			: mNodeParams(nodeParams),
			mOldNodes(MemManagerPtr(nodeParams.GetMemManager())),
			mNewNodes(MemManagerPtr(nodeParams.GetMemManager())),
			mSrcSegments(MemManagerPtr(nodeParams.GetMemManager())),
			mDstSegments(MemManagerPtr(nodeParams.GetMemManager())),
			mItemCount(0)
		{
		}

		Relocator(const Relocator&) = delete;

		~Relocator() noexcept
		{
			for (Node* node : mNewNodes)
				node->Destroy(mNodeParams);
		}

		Relocator& operator=(const Relocator&) = delete;

		Node* CreateNode(bool isLeaf, size_t count)
		{
			mNewNodes.Reserve(mNewNodes.GetCount() + 1);
			Node* node = Node::Create(mNodeParams, isLeaf, count);
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

		Node* GrowLeafNode(Node* node, size_t newItemIndex)
		{
			MOMO_ASSERT(node->IsLeaf());
			mOldNodes.AddBack(node);
			size_t itemCount = node->GetCount();
			Node* newNode = CreateNode(true, itemCount + 1);
			AddSegment(node, 0, newNode, 0, newItemIndex);
			AddSegment(node, newItemIndex, newNode, newItemIndex + 1, itemCount - newItemIndex);
			return newNode;
		}

		SplitResult SplitNode(const TreeTraits& treeTraits, Node* node, size_t newItemIndex)
		{
			SplitResult splitRes = pvSplitNode(treeTraits, node, newItemIndex);
			if (!node->IsLeaf())
			{
				size_t itemCount = node->GetCount();
				Node* newNode = splitRes.newNode1;
				size_t newChildIndex = 0;
				for (size_t i = 0; i <= itemCount; ++i)
				{
					if (i != newItemIndex)
						newNode->SetChild(newChildIndex, node->GetChild(i));
					newChildIndex += (i == newItemIndex) ? 2 : 1;
					if (i == splitRes.splitItemIndex)
					{
						newNode = splitRes.newNode2;
						newChildIndex = 0;
					}
				}
			}
			return splitRes;
		}

		template<typename ItemCreator>
		void RelocateCreate(ItemCreator&& itemCreator, Item* newItem)
		{
			mSrcSegments.AddBack({ nullptr, 0, 0 });
			mDstSegments.AddBack({ nullptr, 0, 0 });
			ItemTraits::RelocateCreate(mNodeParams.GetMemManager(),
				Iterator(mSrcSegments.GetItems()), Iterator(mDstSegments.GetItems()),
				mItemCount, std::forward<ItemCreator>(itemCreator), newItem);
			mSrcSegments.Clear();
			mDstSegments.Clear();
			mItemCount = 0;
			mNewNodes.Swap(mOldNodes);
		}

	private:
		SplitResult pvSplitNode(const TreeTraits& treeTraits, Node* node, size_t newItemIndex)
		{
			bool isLeaf = node->IsLeaf();
			mOldNodes.AddBack(node);
			size_t itemCount = node->GetCount();
			size_t splitItemIndex = treeTraits.GetSplitItemIndex(itemCount, newItemIndex);
			MOMO_ASSERT(splitItemIndex < itemCount);
			if (newItemIndex <= splitItemIndex)
			{
				Node* newNode1 = CreateNode(isLeaf, splitItemIndex + 1);
				Node* newNode2 = CreateNode(isLeaf, itemCount - splitItemIndex - 1);
				AddSegment(node, 0, newNode1, 0, newItemIndex);
				AddSegment(node, newItemIndex, newNode1, newItemIndex + 1,
					splitItemIndex - newItemIndex);
				AddSegment(node, splitItemIndex + 1, newNode2, 0, itemCount - splitItemIndex - 1);
				return { newNode1, newItemIndex, splitItemIndex, newNode1, newNode2 };
			}
			else
			{
				Node* newNode1 = CreateNode(isLeaf, splitItemIndex);
				Node* newNode2 = CreateNode(isLeaf, itemCount - splitItemIndex);
				AddSegment(node, 0, newNode1, 0, splitItemIndex);
				AddSegment(node, splitItemIndex + 1, newNode2, 0, newItemIndex - splitItemIndex - 1);
				AddSegment(node, newItemIndex, newNode2, newItemIndex - splitItemIndex,
					itemCount - newItemIndex);
				return { newNode2, newItemIndex - splitItemIndex - 1, splitItemIndex,
					newNode1, newNode2 };
			}
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
	TreeSet()
		: TreeSet(TreeTraits())
	{
	}

	explicit TreeSet(const TreeTraits& treeTraits, MemManager memManager = MemManager())
		: mCrew(treeTraits, std::move(memManager)),
		mCount(0),
		mRootNode(nullptr),
		mNodeParams(nullptr)
	{
	}

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(*std::declval<ArgIterator>())>
	explicit TreeSet(ArgIterator begin, ArgSentinel end,
		const TreeTraits& treeTraits = TreeTraits(), MemManager memManager = MemManager())
		: TreeSet(treeTraits, std::move(memManager))
	{
		try
		{
			Insert(std::move(begin), std::move(end));
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	TreeSet(std::initializer_list<Item> items)
		: TreeSet(items, TreeTraits())
	{
	}

	explicit TreeSet(std::initializer_list<Item> items, const TreeTraits& treeTraits,
		MemManager memManager = MemManager())
		: TreeSet(items.begin(), items.end(), treeTraits, std::move(memManager))
	{
	}

	TreeSet(TreeSet&& treeSet) noexcept
		: mCrew(std::move(treeSet.mCrew)),
		mCount(treeSet.mCount),
		mRootNode(treeSet.mRootNode),
		mNodeParams(treeSet.mNodeParams)
	{
		treeSet.mCount = 0;
		treeSet.mRootNode = nullptr;
		treeSet.mNodeParams = nullptr;
	}

	TreeSet(const TreeSet& treeSet)
		: TreeSet(treeSet, MemManager(treeSet.GetMemManager()))
	{
	}

	explicit TreeSet(const TreeSet& treeSet, MemManager memManager)
		: TreeSet(treeSet.GetTreeTraits(), std::move(memManager))
	{
		mCount = treeSet.mCount;
		if (mCount == 0)
			return;
		mNodeParams = pvCreateNodeParams();
		try
		{
			mRootNode = pvCopy(treeSet.mRootNode);
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
		if (!mRootNode->IsLeaf())
			pvUpdateParents(mRootNode);
	}

	~TreeSet() noexcept
	{
		pvDestroy();
	}

	TreeSet& operator=(TreeSet&& treeSet) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(treeSet), *this);
	}

	TreeSet& operator=(const TreeSet& treeSet)
	{
		return internal::ContainerAssigner::Copy(treeSet, *this);
	}

	void Swap(TreeSet& treeSet) noexcept
	{
		mCrew.Swap(treeSet.mCrew);
		std::swap(mCount, treeSet.mCount);
		std::swap(mRootNode, treeSet.mRootNode);
		std::swap(mNodeParams, treeSet.mNodeParams);
	}

	ConstIterator GetBegin() const noexcept
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		Node* node = mRootNode;
		while (!node->IsLeaf())
			node = node->GetChild(0);
		return pvMakeIterator(node, 0, true);
	}

	ConstIterator GetEnd() const noexcept
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		return pvMakeIterator(mRootNode, mRootNode->GetCount(), false);
	}

	MOMO_FRIEND_SWAP(TreeSet)
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(TreeSet, ConstIterator)

	const TreeTraits& GetTreeTraits() const noexcept
	{
		return mCrew.GetContainerTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mCrew.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mCrew.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mCount;
	}

	bool IsEmpty() const noexcept
	{
		return mCount == 0;
	}

	void Clear() noexcept
	{
		pvDestroy();
		mRootNode = nullptr;
		mNodeParams = nullptr;
		mCount = 0;
		mCrew.IncVersion();
	}

	ConstIterator GetLowerBound(const Key& key) const
	{
		return pvGetLowerBound(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstIterator> GetLowerBound(const KeyArg& key) const
	{
		return pvGetLowerBound(key);
	}

	ConstIterator GetUpperBound(const Key& key) const
	{
		return pvGetUpperBound(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstIterator> GetUpperBound(const KeyArg& key) const
	{
		return pvGetUpperBound(key);
	}

	ConstIterator Find(const Key& key) const
	{
		return pvFind(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	ConstIterator> Find(const KeyArg& key) const
	{
		return pvFind(key);
	}

	bool ContainsKey(const Key& key) const
	{
		return !pvIsGreater(pvGetLowerBound(key), key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	bool> ContainsKey(const KeyArg& key) const
	{
		return !pvIsGreater(pvGetLowerBound(key), key);
	}

	size_t GetKeyCount(const Key& key) const
	{
		return TreeTraits::multiKey ? pvGetKeyCount(key) : ContainsKey(key) ? 1 : 0;
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	size_t> GetKeyCount(const KeyArg& key) const
	{
		return pvGetKeyCount(key);
	}

	template<typename ItemCreator, bool extraCheck = true>
	InsertResult InsertCrt(const Key& key, ItemCreator&& itemCreator)
	{
		return pvInsert<extraCheck>(key, std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		const Key& key = ItemTraits::GetKey(static_cast<const Item&>(item));
		return pvInsert<false>(key, Creator<Item&&>(GetMemManager(), std::move(item)));
	}

	InsertResult Insert(const Item& item)
	{
		return pvInsert<false>(ItemTraits::GetKey(item),
			Creator<const Item&>(GetMemManager(), item));
	}

	InsertResult Insert(ExtractedItem&& extItem)
	{
		MemManager& memManager = GetMemManager();
		auto itemCreator = [&memManager, &extItem] (Item* newItem)
		{
			auto itemRemover = [&memManager, newItem] (Item& item)
				{ ItemTraits::Relocate(&memManager, item, newItem); };
			extItem.Remove(itemRemover);
		};
		return pvInsert<false>(ItemTraits::GetKey(extItem.GetItem()), itemCreator);
	}

	template<typename ArgIterator, typename ArgSentinel>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		MOMO_STATIC_ASSERT(internal::IsSetArgIterator<ArgIterator, Item>::value);
		if (begin == end)
			return 0;
		const TreeTraits& treeTraits = GetTreeTraits();
		MemManager& memManager = GetMemManager();
		size_t initCount = GetCount();
		ArgIterator iter = std::move(begin);
		auto&& ref0 = *iter;
		typedef decltype(ref0) ItemArg;
		Iterator pos = InsertVar(ItemTraits::GetKey(static_cast<const Item&>(ref0)),
			std::forward<ItemArg>(ref0)).position;
		for (++iter; iter != end; ++iter)
		{
			ItemArg&& ref = *iter;
			const Key& key = ItemTraits::GetKey(static_cast<const Item&>(ref));
			const Key& prevKey = ItemTraits::GetKey(*pos);
			if (treeTraits.IsLess(key, prevKey) || !pvIsGreater(std::next(pos), key))
				pos = InsertVar(key, std::forward<ItemArg>(ref)).position;
			else if (TreeTraits::multiKey || treeTraits.IsLess(prevKey, key))
				pos = pvAdd<false>(std::next(pos), Creator<ItemArg>(memManager, std::forward<ItemArg>(ref)));
		}
		return GetCount() - initCount;
	}

	size_t Insert(std::initializer_list<Item> items)
	{
		return Insert(items.begin(), items.end());
	}

	template<typename ItemCreator, bool extraCheck = true>
	ConstIterator AddCrt(ConstIterator iter, ItemCreator&& itemCreator)
	{
		return pvAdd<extraCheck>(iter, std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	ConstIterator AddVar(ConstIterator iter, ItemArgs&&... itemArgs)
	{
		return AddCrt(iter,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	ConstIterator Add(ConstIterator iter, Item&& item)
	{
		return AddVar(iter, std::move(item));
	}

	ConstIterator Add(ConstIterator iter, const Item& item)
	{
		return AddVar(iter, item);
	}

	ConstIterator Add(ConstIterator iter, ExtractedItem&& extItem)
	{
		MemManager& memManager = GetMemManager();
		auto itemCreator = [&memManager, &extItem] (Item* newItem)
		{
			auto itemRemover = [&memManager, newItem] (Item& item)
				{ ItemTraits::Relocate(&memManager, item, newItem); };
			extItem.Remove(itemRemover);
		};
		return AddCrt(iter, itemCreator);
	}

	ConstIterator Remove(ConstIterator iter)
	{
		auto itemRemover = [this] (Item& item) noexcept
			{ ItemTraits::Destroy(&GetMemManager(), item); };
		auto itemReplacer = [this] (Item& srcItem, Item& dstItem)
			{ ItemTraits::Replace(GetMemManager(), srcItem, dstItem); };
		return pvRemove(iter, itemRemover, itemReplacer);
	}

	ConstIterator Remove(ConstIterator iter, ExtractedItem& extItem)
	{
		ConstIterator resIter;
		auto itemCreator = [this, iter, &resIter] (Item* newItem)
			{ resIter = pvExtract(iter, newItem); };
		extItem.Create(itemCreator);
		return resIter;
	}

	ConstIterator Remove(ConstIterator begin, ConstIterator end)
	{
		if (mRootNode == nullptr)
		{
			MOMO_CHECK(begin == ConstIterator() && end == ConstIterator());
			return ConstIterator();
		}
		ConstIteratorProxy::Check(begin, mCrew.GetVersion(), false);
		ConstIteratorProxy::Check(end, mCrew.GetVersion(), false);
		ConstIterator thisEnd = GetEnd();
		size_t remCount = 0;
		for (ConstIterator iter = begin; iter != end; ++iter)
		{
			MOMO_CHECK(iter != thisEnd);
			++remCount;
		}
		if (remCount == 0)
			return end;
		if (remCount == mCount)
		{
			Clear();
			return GetEnd();
		}
		ConstIterator prevEnd = std::prev(end);
		Node* node1 = ConstIteratorProxy::GetNode(begin);
		size_t itemIndex1 = ConstIteratorProxy::GetItemIndex(begin);
		Node* node2 = ConstIteratorProxy::GetNode(prevEnd);
		size_t itemIndex2 = ConstIteratorProxy::GetItemIndex(prevEnd);
		Node* resNode;
		size_t resItemIndex;
		if (node1 == node2 && node1->IsLeaf())
		{
			auto itemRemover = [this] (Item& item) noexcept
				{ ItemTraits::Destroy(&GetMemManager(), item); };
			for (size_t i = itemIndex2 + 1; i > itemIndex1; --i)
				node1->Remove(*mNodeParams, i - 1, itemRemover);
			resNode = node1;
			resItemIndex = itemIndex1;
			pvRebalance(resNode, resNode, true);
		}
		else
		{
			resNode = pvRemoveRange(node1, itemIndex1, node2, itemIndex2);
			resItemIndex = 0;
		}
		mCount -= remCount;
		mCrew.IncVersion();
		return pvMakeIterator(resNode, resItemIndex, true);
	}

	size_t Remove(const Key& key)
	{
		ConstIterator iter = pvGetLowerBound(key);
		if (pvIsGreater(iter, key))
			return 0;
		if (!TreeTraits::multiKey)
		{
			Remove(iter);
			return 1;
		}
		ConstIterator iter2 = std::next(iter);
		size_t remCount = 1;
		while (!pvIsGreater(iter2, key))
		{
			++iter2;
			++remCount;
		}
		Remove(iter, iter2);
		return remCount;
	}

	template<typename ItemFilter>
	internal::EnableIf<internal::IsInvocable<const ItemFilter&, bool, const Item&>::value,
	size_t> Remove(const ItemFilter& itemFilter)
	{
		size_t initCount = GetCount();
		ConstIterator iter = GetBegin();
		while (iter != GetEnd())
		{
			if (itemFilter(*iter))
				iter = Remove(iter);
			else
				++iter;
		}
		return initCount - GetCount();
	}

	ExtractedItem Extract(ConstIterator iter)
	{
		return ExtractedItem(*this, iter);	// need RVO for exception safety
	}

	template<typename KeyArg, bool extraCheck = true>
	void ResetKey(ConstIterator iter, KeyArg&& keyArg)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), false);
		MOMO_CHECK(iter != GetEnd());
		Node* node = ConstIteratorProxy::GetNode(iter);
		Item& item = *node->GetItemPtr(ConstIteratorProxy::GetItemIndex(iter));
		ItemTraits::AssignKey(GetMemManager(), std::forward<KeyArg>(keyArg), item);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(iter));
	}

	template<typename RSet>
	void MergeFrom(RSet&& srcSet)
	{
		srcSet.MergeTo(*this);
	}

	template<typename Set>
	void MergeTo(Set& dstSet)
	{
		MOMO_STATIC_ASSERT(std::is_same<Key, typename Set::Key>::value);
		MOMO_STATIC_ASSERT(std::is_same<Item, typename Set::Item>::value);
		pvMergeTo(dstSet);
	}

	void MergeTo(TreeSet& dstTreeSet)
	{
		if (this == &dstTreeSet)
			return;
		if (!std::is_empty<TreeTraits>::value)
			return pvMergeTo(dstTreeSet);
		size_t count = GetCount();
		if (count == 0)
			return;
		size_t dstCount = dstTreeSet.GetCount();
		if (MemManagerProxy::IsEqual(GetMemManager(), dstTreeSet.GetMemManager()))
		{
			if (dstCount == 0)
			{
				std::swap(mCount, dstTreeSet.mCount);
				std::swap(mRootNode, dstTreeSet.mRootNode);
				std::swap(mNodeParams, dstTreeSet.mNodeParams);
				mCrew.IncVersion();
				dstTreeSet.mCrew.IncVersion();
				return;
			}
			Node* rootNode = nullptr;
			if (pvIsOrdered(*this, dstTreeSet))
				rootNode = pvMergeFast(*this, dstTreeSet);
			else if (pvIsOrdered(dstTreeSet, *this))
				rootNode = pvMergeFast(dstTreeSet, *this);
			if (rootNode != nullptr)
			{
				dstTreeSet.mCount += mCount;
				mCount = 0;
				dstTreeSet.mRootNode = rootNode;
				mRootNode = nullptr;
				dstTreeSet.mNodeParams->MergeFrom(*mNodeParams);
				mCrew.IncVersion();
				dstTreeSet.mCrew.IncVersion();
				return;
			}
		}
		if (count * internal::UIntMath<>::Log2(count + dstCount) < count + dstCount)	//?
			pvMergeTo(dstTreeSet);
		else
			pvMergeToLinear(dstTreeSet);
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), allowEmpty);
	}

private:
	void pvDestroy() noexcept
	{
		if (mRootNode != nullptr)
			pvDestroy(mRootNode);
		if (mNodeParams != nullptr)
		{
			mNodeParams->~NodeParams();
			MemManagerProxy::Deallocate(GetMemManager(), mNodeParams, sizeof(NodeParams));
		}
	}

	void pvDestroy(Node* node) noexcept
	{
		MemManager& memManager = GetMemManager();
		size_t itemCount = node->GetCount();
		for (size_t i = 0; i < itemCount; ++i)
			ItemTraits::Destroy(&memManager, *node->GetItemPtr(i));
		if (!node->IsLeaf())
		{
			for (size_t i = 0; i <= itemCount; ++i)
				pvDestroy(node->GetChild(i));
		}
		node->Destroy(*mNodeParams);
	}

	Node* pvCopy(Node* srcNode)
	{
		MemManager& memManager = GetMemManager();
		size_t itemCount = srcNode->GetCount();
		bool isLeaf = srcNode->IsLeaf();
		Node* dstNode = Node::Create(*mNodeParams, isLeaf, itemCount);
		size_t itemIndex = 0;
		size_t childIndex = 0;
		try
		{
			for (; itemIndex < itemCount; ++itemIndex)
			{
				Creator<const Item&>(memManager, *srcNode->GetItemPtr(itemIndex))(
					dstNode->GetItemPtr(itemIndex));
			}
			if (!isLeaf)
			{
				for (; childIndex <= itemCount; ++childIndex)
					dstNode->SetChild(childIndex, pvCopy(srcNode->GetChild(childIndex)));
			}
		}
		catch (...)
		{
			for (size_t i = 0; i < itemIndex; ++i)
				ItemTraits::Destroy(&memManager, *dstNode->GetItemPtr(i));
			for (size_t i = 0; i < childIndex; ++i)
				pvDestroy(dstNode->GetChild(i));
			dstNode->Destroy(*mNodeParams);
			throw;
		}
		return dstNode;
	}

	void pvUpdateParents(Node* node) noexcept
	{
		MOMO_ASSERT(!node->IsLeaf());
		size_t itemCount = node->GetCount();
		for (size_t i = 0; i <= itemCount; ++i)
		{
			Node* childNode = node->GetChild(i);
			childNode->SetParent(node);
			if (!childNode->IsLeaf())
				pvUpdateParents(childNode);
		}
	}

	NodeParams* pvCreateNodeParams()
	{
		MemManager& memManager = GetMemManager();
		return MemManagerProxy::template AllocateCreate<NodeParams>(memManager, memManager);
	}

	ConstIterator pvMakeIterator(Node* node, size_t itemIndex, bool move) const noexcept
	{
		return ConstIteratorProxy(*node, itemIndex, mCrew.GetVersion(), move);
	}

	template<typename KeyArg>
	bool pvIsGreater(ConstIterator iter, const KeyArg& key) const
	{
		return iter == GetEnd() || GetTreeTraits().IsLess(key, ItemTraits::GetKey(*iter));
	}

	bool pvIsOrdered(ConstIterator iter1, ConstIterator iter2) const
	{
		const Key& key1 = ItemTraits::GetKey(*iter1);
		const Key& key2 = ItemTraits::GetKey(*iter2);
		const TreeTraits& treeTraits = GetTreeTraits();
		return TreeTraits::multiKey ? !treeTraits.IsLess(key2, key1)
			: treeTraits.IsLess(key1, key2);
	}

	bool pvIsOrdered(const TreeSet& treeSet1, const TreeSet& treeSet2) const
	{
		MOMO_ASSERT(!treeSet1.IsEmpty() && !treeSet2.IsEmpty());
		return pvIsOrdered(std::prev(treeSet1.GetEnd()), treeSet2.GetBegin());
	}

	bool pvExtraCheck(ConstIterator iter) const noexcept
	{
		try
		{
			if (iter != GetBegin() && !pvIsOrdered(std::prev(iter), iter))
				return false;
			if (iter != std::prev(GetEnd()) && !pvIsOrdered(iter, std::next(iter)))
				return false;
			return true;
		}
		catch (...)
		{
			//?
			return false;
		}
	}

	template<typename KeyArg>
	ConstIterator pvGetLowerBound(const KeyArg& key) const
	{
		const TreeTraits& treeTraits = GetTreeTraits();
		auto itemPred = [&treeTraits, &key] (const Item& item)
			{ return !treeTraits.IsLess(ItemTraits::GetKey(item), key); };
		return pvFindFirst(itemPred);
	}

	template<typename KeyArg>
	ConstIterator pvGetUpperBound(const KeyArg& key) const
	{
		const TreeTraits& treeTraits = GetTreeTraits();
		auto itemPred = [&treeTraits, &key] (const Item& item)
			{ return treeTraits.IsLess(key, ItemTraits::GetKey(item)); };
		return pvFindFirst(itemPred);
	}

	template<typename ItemPredicate>
	ConstIterator pvFindFirst(const ItemPredicate& itemPred) const
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		ConstIterator iter = GetEnd();
		Node* node = mRootNode;
		while (true)
		{
			size_t index = pvFindFirst(node, itemPred);
			if (index < node->GetCount())
				iter = pvMakeIterator(node, index, false);
			if (node->IsLeaf())
				break;
			node = node->GetChild(index);
		}
		return iter;
	}

	template<typename ItemPredicate>
	size_t pvFindFirst(Node* node, const ItemPredicate& itemPred) const
	{
		if (TreeTraits::useLinearSearch)
		{
			size_t itemCount = node->GetCount();
			if (itemCount == 0 || !itemPred(*node->GetItemPtr(itemCount - 1)))
				return itemCount;
			size_t index = 0;
			while (!itemPred(*node->GetItemPtr(index)))
				++index;
			return index;
		}
		else
		{
			size_t leftIndex = 0;
			size_t rightIndex = node->GetCount();
			while (leftIndex < rightIndex)
			{
				size_t middleIndex = (leftIndex + rightIndex) / 2;
				if (itemPred(*node->GetItemPtr(middleIndex)))
					rightIndex = middleIndex;
				else
					leftIndex = middleIndex + 1;
			}
			return leftIndex;
		}
	}

	template<typename KeyArg>
	ConstIterator pvFind(const KeyArg& key) const
	{
		ConstIterator iter = pvGetLowerBound(key);	//?
		return !pvIsGreater(iter, key) ? iter : GetEnd();
	}

	template<typename KeyArg>
	size_t pvGetKeyCount(const KeyArg& key) const
	{
		size_t count = 0;
		for (ConstIterator iter = pvGetLowerBound(key); !pvIsGreater(iter, key); ++iter)
			++count;
		return count;
	}

	template<bool extraCheck, typename ItemCreator>
	InsertResult pvInsert(const Key& key, ItemCreator&& itemCreator)
	{
		ConstIterator iter = pvGetUpperBound(key);
		if (!TreeTraits::multiKey && iter != GetBegin())
		{
			ConstIterator prevIter = std::prev(iter);
			if (!GetTreeTraits().IsLess(ItemTraits::GetKey(*prevIter), key))
				return { prevIter, false };
		}
		iter = pvAdd<extraCheck>(iter, std::forward<ItemCreator>(itemCreator));
		return { iter, true };
	}

	template<bool extraCheck, typename ItemCreator>
	ConstIterator pvAdd(ConstIterator iter, ItemCreator&& itemCreator)
	{
		if (mRootNode == nullptr)
			return pvAddFirst(iter, std::forward<ItemCreator>(itemCreator));
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), false);
		Node* node = ConstIteratorProxy::GetNode(iter);
		size_t itemIndex = ConstIteratorProxy::GetItemIndex(iter);
		if (!node->IsLeaf())
		{
			node = node->GetChild(itemIndex);
			while (!node->IsLeaf())
				node = node->GetChild(node->GetCount());
			itemIndex = node->GetCount();
		}
		size_t itemCount = node->GetCount();
		if (itemCount < node->GetCapacity())
		{
			std::forward<ItemCreator>(itemCreator)(node->GetItemPtr(itemCount));
			node->AcceptBackItem(*mNodeParams, itemIndex);
		}
		else
		{
			Relocator relocator(*mNodeParams);
			if (itemCount < nodeMaxCapacity)
				pvAddGrow(relocator, node, itemIndex, std::forward<ItemCreator>(itemCreator));
			else
				pvAddSplit(relocator, node, itemIndex, std::forward<ItemCreator>(itemCreator));
		}
		++mCount;
		mCrew.IncVersion();
		ConstIterator resIter = pvMakeIterator(node, itemIndex, false);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(resIter));
		return resIter;
	}

	template<typename ItemCreator>
	ConstIterator pvAddFirst(ConstIterator iter, ItemCreator&& itemCreator)
	{
		(void)iter;
		MOMO_CHECK(iter == ConstIterator());
		if (mNodeParams == nullptr)
			mNodeParams = pvCreateNodeParams();
		mRootNode = Node::Create(*mNodeParams, true, 0);
		try
		{
			return pvAdd<false>(GetEnd(), std::forward<ItemCreator>(itemCreator));
		}
		catch (...)
		{
			mRootNode->Destroy(*mNodeParams);
			mRootNode = nullptr;
			throw;
		}
	}

	template<typename ItemCreator>
	void pvAddGrow(Relocator& relocator, Node*& node, size_t itemIndex, ItemCreator&& itemCreator)
	{
		Node* newNode = relocator.GrowLeafNode(node, itemIndex);
		relocator.RelocateCreate(std::forward<ItemCreator>(itemCreator),
			newNode->GetItemPtr(itemIndex));
		Node* parentNode = node->GetParent();
		if (parentNode == nullptr)
			mRootNode = newNode;
		else
			parentNode->SetChild(parentNode->GetChildIndex(node), newNode);
		newNode->SetParent(parentNode);
		node = newNode;
	}

	template<typename ItemCreator>
	void pvAddSplit(Relocator& relocator, Node*& leafNode, size_t& leafItemIndex,
		ItemCreator&& itemCreator)
	{
		const TreeTraits& treeTraits = GetTreeTraits();
		Node* node = leafNode;
		size_t itemIndex = leafItemIndex;
		typename Relocator::SplitResult splitRes = relocator.SplitNode(treeTraits, node, itemIndex);
		leafNode = splitRes.newNode;
		leafItemIndex = splitRes.newItemIndex;
		while (true)
		{
			size_t childSplitItemIndex = splitRes.splitItemIndex;
			Node* childNode = node;
			node = node->GetParent();
			if (node == nullptr)
			{
				node = relocator.CreateNode(false, 0);
				itemIndex = 0;
				relocator.AddSegment(childNode, childSplitItemIndex, node, 0, 1);
				break;
			}
			size_t itemCount = node->GetCount();
			itemIndex = node->GetChildIndex(childNode);
			if (itemCount < node->GetCapacity())
			{
				relocator.AddSegment(childNode, childSplitItemIndex, node, itemCount, 1);
				break;
			}
			Node* childNewNode1 = splitRes.newNode1;
			Node* childNewNode2 = splitRes.newNode2;
			splitRes = relocator.SplitNode(treeTraits, node, itemIndex);
			relocator.AddSegment(childNode, childSplitItemIndex,
				splitRes.newNode, splitRes.newItemIndex, 1);
			splitRes.newNode->SetChild(splitRes.newItemIndex, childNewNode1);
			splitRes.newNode->SetChild(splitRes.newItemIndex + 1, childNewNode2);
		}
		relocator.RelocateCreate(std::forward<ItemCreator>(itemCreator),
			leafNode->GetItemPtr(leafItemIndex));
		if (node->GetParent() == nullptr)
			mRootNode = node;
		node->AcceptBackItem(*mNodeParams, itemIndex);
		node->SetChild(itemIndex, splitRes.newNode1);
		node->SetChild(itemIndex + 1, splitRes.newNode2);
		pvUpdateParents(node);	//?
	}

	template<typename ItemRemover, typename ItemReplacer>
	ConstIterator pvRemove(ConstIterator iter, ItemRemover&& itemRemover,
		ItemReplacer&& itemReplacer)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), false);
		MOMO_CHECK(iter != GetEnd());
		Node* node = ConstIteratorProxy::GetNode(iter);
		size_t itemIndex = ConstIteratorProxy::GetItemIndex(iter);
		if (node->IsLeaf())
		{
			node->Remove(*mNodeParams, itemIndex, std::forward<ItemRemover>(itemRemover));
			pvRebalance(node, node, true);
		}
		else
		{
			node = pvRemoveInternal(node, itemIndex, std::forward<ItemRemover>(itemRemover),
				std::forward<ItemReplacer>(itemReplacer));
			itemIndex = 0;
		}
		--mCount;
		mCrew.IncVersion();
		return pvMakeIterator(node, itemIndex, true);
	}

	ConstIterator pvExtract(ConstIterator iter, Item* extItem)
	{
		auto itemRemover = [this, extItem] (Item& item)
			{ ItemTraits::Relocate(&GetMemManager(), item, extItem); };
		auto itemReplacer = [this, extItem] (Item& srcItem, Item& dstItem)
			{ ItemTraits::ReplaceRelocate(GetMemManager(), srcItem, dstItem, extItem); };
		return pvRemove(iter, itemRemover, itemReplacer);
	}

	template<typename ItemRemover, typename ItemReplacer>
	Node* pvRemoveInternal(Node* node, size_t itemIndex, ItemRemover&& itemRemover,
		ItemReplacer&& itemReplacer)
	{
		Node* childNode = node->GetChild(itemIndex);
		while (!childNode->IsLeaf())
			childNode = childNode->GetChild(childNode->GetCount());
		while (childNode != node && childNode->GetCount() == 0)
			childNode = childNode->GetParent();
		Node* resNode;
		if (childNode == node)
		{
			Node* rightNode = node->GetChild(itemIndex + 1);
			pvDestroyInternal(node, itemIndex, false, std::forward<ItemRemover>(itemRemover));
			resNode = rightNode;
		}
		else
		{
			size_t childItemIndex = childNode->GetCount() - 1;
			auto itemReplaceRemover = [node, itemIndex, &itemReplacer] (Item& item)
				{ std::forward<ItemReplacer>(itemReplacer)(item, *node->GetItemPtr(itemIndex)); };
			if (childNode->IsLeaf())
				childNode->Remove(*mNodeParams, childItemIndex, itemReplaceRemover);
			else
				pvDestroyInternal(childNode, childItemIndex, true, itemReplaceRemover);
			resNode = node->GetChild(itemIndex + 1);
		}
		while (!resNode->IsLeaf())
			resNode = resNode->GetChild(0);
		pvRebalance(childNode, resNode, true);
		return resNode;
	}

	Node* pvRemoveRange(Node* node1, size_t itemIndex1, Node* node2, size_t itemIndex2)
	{
		auto itemRemover = [this] (Item& item) noexcept
			{ ItemTraits::Destroy(&GetMemManager(), item); };
		size_t comIndex1 = itemIndex1;
		size_t comIndex2 = itemIndex2;
		Node* comNode = pvGetCommonParent(node1, node2, comIndex1, comIndex2);
		MOMO_ASSERT(!comNode->IsLeaf());
		if (!node1->IsLeaf())
		{
			node1 = node1->GetChild(itemIndex1);
			while (!node1->IsLeaf())
				node1 = node1->GetChild(node1->GetCount());
			itemIndex1 = node1->GetCount();
		}
		while (itemIndex1 == 0)
		{
			pvToParent(node1, itemIndex1);
			if (node1 == comNode)
				break;
		}
		Node* rebNode1 = node1;
		if (node1 != comNode)
		{
			--itemIndex1;
			auto itemReplaceRemover = [this, comNode, comIndex1] (Item& item)
				{ ItemTraits::Replace(GetMemManager(), item, *comNode->GetItemPtr(comIndex1)); };
			if (node1->IsLeaf())
			{
				node1->Remove(*mNodeParams, itemIndex1, itemReplaceRemover);
				for (size_t i = node1->GetCount(); i > itemIndex1; --i)
					node1->Remove(*mNodeParams, i - 1, itemRemover);
				pvToParent(node1, itemIndex1);
			}
			else
			{
				pvDestroyInternal(node1, itemIndex1, true, itemReplaceRemover);
			}
			while (node1 != comNode)
			{
				for (size_t i = node1->GetCount(); i > itemIndex1; --i)
					pvDestroyInternal(node1, i - 1, true, itemRemover);
				pvToParent(node1, itemIndex1);
			}
			++comIndex1;
		}
		if (node2 != comNode)
		{
			if (node2->IsLeaf())
			{
				for (size_t i = itemIndex2 + 1; i > 0; --i)
					node2->Remove(*mNodeParams, i - 1, itemRemover);
				pvToParent(node2, itemIndex2);
			}
			else
			{
				pvDestroyInternal(node2, itemIndex2, false, itemRemover);
			}
			while (node2 != comNode)
			{
				for (size_t i = itemIndex2; i > 0; --i)
					pvDestroyInternal(node2, i - 1, false, itemRemover);
				pvToParent(node2, itemIndex2);
			}
		}
		else
		{
			++comIndex2;
		}
		for (size_t i = comIndex2; i > comIndex1; --i)
			pvDestroyInternal(comNode, i - 1, false, itemRemover);
		Node* resNode = comNode->GetChild(comIndex1);
		while (!resNode->IsLeaf())
			resNode = resNode->GetChild(0);
		if (rebNode1 != comNode)
			pvRebalance(rebNode1, resNode, false);
		pvRebalance(resNode, resNode, false);
		return resNode;
	}

	template<typename ItemRemover>
	void pvDestroyInternal(Node* node, size_t itemIndex, bool destroyRight,
		ItemRemover&& itemRemover)
	{
		MOMO_ASSERT(!node->IsLeaf());
		Node* leftNode = node->GetChild(itemIndex);
		Node* rightNode = node->GetChild(itemIndex + 1);
		node->Remove(*mNodeParams, itemIndex, std::forward<ItemRemover>(itemRemover));
		pvDestroy(destroyRight ? rightNode : leftNode);
		node->SetChild(itemIndex, destroyRight ? leftNode : rightNode);
	}

	static Node* pvGetCommonParent(Node* node1, Node* node2,
		size_t& comIndex1, size_t& comIndex2) noexcept
	{
		size_t height1 = pvGetHeight(node1);
		size_t height2 = pvGetHeight(node2);
		for (; height1 < height2; ++height1)
			pvToParent(node1, comIndex1);
		for (; height2 < height1; ++height2)
			pvToParent(node2, comIndex2);
		while (node1 != node2)
		{
			pvToParent(node1, comIndex1);
			pvToParent(node2, comIndex2);
		}
		return node1;
	}

	static size_t pvGetHeight(Node* node) noexcept
	{
		size_t height = 1;
		for (; !node->IsLeaf(); node = node->GetChild(0))
			++height;
		return height;
	}

	static void pvToParent(Node*& node, size_t& index) noexcept
	{
		Node* parentNode = node->GetParent();
		MOMO_ASSERT(parentNode != nullptr);
		index = parentNode->GetChildIndex(node);
		node = parentNode;
	}

	void pvRebalance(Node* node, Node* savedNode, bool fast) noexcept
	{
		while (mRootNode->GetCount() == 0 && !mRootNode->IsLeaf())
		{
			MOMO_ASSERT(mRootNode != savedNode);
			mRootNode = mRootNode->GetChild(0);
			mRootNode->GetParent()->Destroy(*mNodeParams);
			mRootNode->SetParent(nullptr);
		}
		try
		{
			while (true)
			{
				Node* parentNode = node->GetParent();
				MOMO_ASSERT(parentNode != savedNode);
				if (parentNode == nullptr)
					break;
				size_t index = parentNode->GetChildIndex(node);
				bool stop = !pvRebalance(parentNode, index + 1, savedNode)
					&& !pvRebalance(parentNode, index, savedNode) && fast;
				if (stop)
					break;
				node = parentNode;
			}
		}
		catch (...)
		{
			// no throw!
		}
	}

	bool pvRebalance(Node* parentNode, size_t index, Node* savedNode)
	{
		if (index == 0 || index > parentNode->GetCount())
			return false;
		--index;
		Node* node1 = parentNode->GetChild(index);
		Node* node2 = parentNode->GetChild(index + 1);
		if (node2 == savedNode)
			return false;
		size_t itemCount1 = node1->GetCount();
		size_t itemCount2 = node2->GetCount();
		if (itemCount1 + itemCount2 + 1 > node1->GetCapacity())
			return false;
		Relocator relocator(*mNodeParams);
		relocator.AddSegment(node2, 0, node1, itemCount1 + 1, itemCount2);
		auto itemRemover = [this, &relocator, node1, itemCount1] (Item& item)
		{
			MemManager& memManager = GetMemManager();
			auto itemCreator = [&memManager, &item] (Item* newItem)
				{ ItemTraits::Relocate(&memManager, item, newItem); };
			relocator.RelocateCreate(itemCreator, node1->GetItemPtr(itemCount1));
		};
		parentNode->Remove(*mNodeParams, index, itemRemover);
		parentNode->SetChild(index, node1);
		Node* lastChildNode = node1->IsLeaf() ? nullptr : node1->GetChild(itemCount1);
		for (size_t i = 0; i <= itemCount2; ++i)
			node1->AcceptBackItem(*mNodeParams, itemCount1 + i);
		if (!node1->IsLeaf())
		{
			node1->SetChild(itemCount1, lastChildNode);
			for (size_t i = 0; i <= itemCount2; ++i)
			{
				Node* childNode = node2->GetChild(i);
				node1->SetChild(itemCount1 + i + 1, childNode);
				childNode->SetParent(node1);
			}
		}
		node2->Destroy(*mNodeParams);
		return true;
	}

	template<typename Set>
	void pvMergeTo(Set& dstSet)
	{
		ConstIterator iter = GetBegin();
		while (iter != GetEnd())
		{
			auto itemCreator = [this, &iter] (Item* newItem)
				{ iter = pvExtract(iter, newItem); };
			if (!dstSet.InsertCrt(ItemTraits::GetKey(*iter), itemCreator).inserted)
				++iter;
		}
	}

	void pvMergeToLinear(TreeSet& dstTreeSet)
	{
		ConstIterator iter = GetBegin();
		ConstIterator dstIter = dstTreeSet.GetBegin();
		while (iter != GetEnd())
		{
			while (dstIter != dstTreeSet.GetEnd() && pvIsOrdered(dstIter, iter))
				++dstIter;
			if (TreeTraits::multiKey || dstTreeSet.pvIsGreater(dstIter, ItemTraits::GetKey(*iter)))
			{
				auto itemCreator = [this, &iter] (Item* newItem)
					{ iter = pvExtract(iter, newItem); };
				dstIter = std::next(dstTreeSet.pvAdd<false>(dstIter, itemCreator));
			}
			else
			{
				++iter;
				++dstIter;
			}
		}
	}

	static Node* pvMergeFast(TreeSet& treeSet1, TreeSet& treeSet2)
	{
		size_t height1 = pvGetHeight(treeSet1.mRootNode);
		size_t height2 = pvGetHeight(treeSet2.mRootNode);
		TreeSet* treeSetPtr1 = &treeSet1;
		TreeSet* treeSetPtr2 = &treeSet2;
		bool swap = false;
		if (height1 > height2)
		{
			swap = true;
			std::swap(height1, height2);
			std::swap(treeSetPtr1, treeSetPtr2);
		}
		Node* rootNode1 = treeSetPtr1->mRootNode;
		Node* node2 = treeSetPtr2->mRootNode;
		for (size_t i = height1; i < height2; ++i)
			node2 = node2->GetChild(swap ? node2->GetCount() : 0);
		try
		{
			while (true)
			{
				node2 = node2->GetParent();
				if (node2 != nullptr && node2->GetCount() < nodeMaxCapacity)
					break;
				Node* newRootNode1 = Node::Create(*treeSetPtr1->mNodeParams, false, 0);
				newRootNode1->SetChild(0, rootNode1);
				rootNode1->SetParent(newRootNode1);
				rootNode1 = newRootNode1;
				if (node2 == nullptr)
				{
					node2 = rootNode1;
					break;
				}
			}
			Node* node1 = ConstIteratorProxy::GetNode(
				swap ? treeSetPtr1->GetBegin() : std::prev(treeSetPtr1->GetEnd()));
			auto itemRemover = [treeSetPtr1, node2] (Item& item)
			{
				ItemTraits::Relocate(&treeSetPtr1->GetMemManager(), item,
					node2->GetItemPtr(node2->GetCount()));
			};
			if (node1->IsLeaf())
			{
				node1->Remove(*treeSetPtr1->mNodeParams,
					swap ? 0 : node1->GetCount() - 1, itemRemover);
			}
			else
			{
				Node* node10 = node1->GetChild(swap ? 0 : node1->GetCount());
				Node* node11 = node1->GetChild(swap ? 1 : node1->GetCount() - 1);
				node1->Remove(*treeSetPtr1->mNodeParams,
					swap ? 0 : node1->GetCount() - 1, itemRemover);
				treeSetPtr1->pvDestroy(node10);
				node1->SetChild(swap ? 0 : node1->GetCount(), node11);
			}
		}
		catch (...)
		{
			Node* node1 = treeSetPtr1->mRootNode->GetParent();
			treeSetPtr1->mRootNode->SetParent(nullptr);
			while (node1 != nullptr)
			{
				Node* nextNode1 = node1->GetParent();
				node1->Destroy(*treeSetPtr1->mNodeParams);
				node1 = nextNode1;
			}
			throw;
		}
		Node* childNode2 = node2->GetChild(swap ? node2->GetCount() : 0);
		node2->AcceptBackItem(*treeSetPtr2->mNodeParams, swap ? node2->GetCount() : 0);
		if (node2 == rootNode1)
		{
			node2->SetChild(swap ? 1 : 0, childNode2);
			node2->SetChild(swap ? 0 : 1, treeSetPtr2->mRootNode);
			treeSetPtr2->mRootNode->SetParent(node2);
			return rootNode1;
		}
		else
		{
			node2->SetChild(swap ? node2->GetCount() : 0, rootNode1);
			node2->SetChild(swap ? node2->GetCount() - 1 : 1, childNode2);
			rootNode1->SetParent(node2);
			return treeSetPtr2->mRootNode;
		}
	}

private:
	Crew mCrew;
	size_t mCount;
	Node* mRootNode;
	NodeParams* mNodeParams;
};

template<typename TKey>
using TreeMultiSet = TreeSet<TKey, TreeTraits<TKey, true>>;

} // namespace momo

namespace std
{
	template<typename N, typename S>
	struct iterator_traits<momo::internal::TreeSetConstIterator<N, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::TreeSetConstIterator<N, S>,
			bidirectional_iterator_tag>
	{
	};

	template<typename S>
	struct iterator_traits<momo::internal::TreeSetRelocatorIterator<S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::TreeSetRelocatorIterator<S>,
			forward_iterator_tag>
	{
	};
} // namespace std

#endif // MOMO_INCLUDE_GUARD_TREE_SET
