/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/TreeSet.h

  namespace momo:
    struct TreeSetItemTraits
    struct TreeSetSettings
    class TreeSet

\**********************************************************/

#pragma once

#include "TreeTraits.h"
#include "SetUtility.h"
#include "IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TNode, typename TSettings>
	class TreeSetConstIterator : private IteratorVersion<TSettings::checkVersion>
	{
	public:
		typedef TNode Node;
		typedef TSettings Settings;
		typedef typename Node::Item Item;

		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef TreeSetConstIterator ConstIterator;

	private:
		typedef internal::IteratorVersion<Settings::checkVersion> IteratorVersion;

	public:
		TreeSetConstIterator() MOMO_NOEXCEPT
			: mNode(nullptr),
			mItemIndex(0)
		{
		}

		TreeSetConstIterator(Node& node, size_t itemIndex, const size_t& version,
			bool move) MOMO_NOEXCEPT
			: IteratorVersion(version),
			mNode(&node),
			mItemIndex(itemIndex)
		{
			if (move)
				_Move();
		}

		//operator ConstIterator() const MOMO_NOEXCEPT

		TreeSetConstIterator& operator++()
		{
			MOMO_CHECK(mNode != nullptr);
			MOMO_CHECK(IteratorVersion::Check());
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
			_Move();
			return *this;
		}

		TreeSetConstIterator& operator--()
		{
			MOMO_CHECK(mNode != nullptr);
			MOMO_CHECK(IteratorVersion::Check());
			Node* node = mNode;
			size_t itemIndex = mItemIndex;
			if (!node->IsLeaf())
			{
				node = node->GetChild(itemIndex);
				while (!node->IsLeaf())
					node = node->GetChild(node->GetCount());
				itemIndex = node->GetCount();
			}
			if (itemIndex == 0)
			{
				while (true)
				{
					Node* childNode = node;
					node = node->GetParent();
					MOMO_CHECK(node != nullptr);
					if (childNode != node->GetChild(0))
					{
						itemIndex = node->GetChildIndex(childNode) - 1;
						break;
					}
				}
			}
			else
			{
				--itemIndex;
			}
			mNode = node;
			mItemIndex = itemIndex;
			return *this;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mNode != nullptr);
			MOMO_CHECK(mItemIndex < mNode->GetCount());
			MOMO_CHECK(IteratorVersion::Check());
			return mNode->GetItemPtr(mItemIndex);
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mNode == iter.mNode && mItemIndex == iter.mItemIndex;
		}

		MOMO_MORE_TREE_ITERATOR_OPERATORS(TreeSetConstIterator)

		Node* GetNode() const MOMO_NOEXCEPT
		{
			return mNode;
		}

		size_t GetItemIndex() const MOMO_NOEXCEPT
		{
			return mItemIndex;
		}

		void Check(const size_t& version) const
		{
			(void)version;
			MOMO_CHECK(mNode != nullptr);
			MOMO_CHECK(IteratorVersion::Check(version));
		}

	private:
		void _Move() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mNode->IsLeaf());
			if (mItemIndex < mNode->GetCount())
				return;
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
}

template<typename TKey,
	typename TItem = TKey>
struct TreeSetItemTraits : public internal::SetItemTraits<TKey, TItem>
{
	typedef TKey Key;
	typedef TItem Item;

	typedef internal::ObjectManager<Item> ItemManager;

	static void Destroy(Item& item) MOMO_NOEXCEPT
	{
		ItemManager::Destroy(item);
	}

	static void SwapNothrowAnyway(Item& item1, Item& item2) MOMO_NOEXCEPT
	{
		ItemManager::SwapNothrowAnyway(item1, item2);
	}
};

struct TreeSetSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
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

	struct NodeItemTraits
	{
		typedef typename TreeSet::Item Item;

		static const bool isNothrowAnywaySwappable = ItemTraits::isNothrowAnywaySwappable;

		static const size_t alignment = ItemTraits::alignment;

		static void Destroy(Item& item) MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(item);
		}

		static void SwapNothrowAnyway(Item& item1, Item& item2) MOMO_NOEXCEPT
		{
			ItemTraits::SwapNothrowAnyway(item1, item2);
		}
	};

	typedef typename TreeTraits::TreeNode TreeNode;
	typedef typename TreeNode::template Node<NodeItemTraits, MemManager> Node;

	typedef typename Node::Params NodeParams;

	static const size_t nodeMaxCapacity = TreeNode::maxCapacity;
	MOMO_STATIC_ASSERT(nodeMaxCapacity > 0);

	typedef internal::SetCrew<TreeTraits, MemManager, NodeParams> Crew;

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
			explicit Iterator(Segment* segmentPtr) MOMO_NOEXCEPT
				: mSegmentPtr(segmentPtr),
				mItemIndex(segmentPtr->beginIndex)
			{
			}

			Iterator& operator++() MOMO_NOEXCEPT
			{
				++mItemIndex;
				if (mItemIndex == mSegmentPtr->endIndex)
				{
					++mSegmentPtr;
					mItemIndex = mSegmentPtr->beginIndex;
				}
				return *this;
			}

			Item* operator->() const MOMO_NOEXCEPT
			{
				return mSegmentPtr->node->GetItemPtr(mItemIndex);
			}

			Item& operator*() const MOMO_NOEXCEPT
			{
				return *operator->();
			}

		private:
			Segment* mSegmentPtr;
			size_t mItemIndex;
		};

	public:
		struct SplitResult
		{
			Node* newNode;
			size_t newItemIndex;
			size_t middleIndex;
			Node* newNode1;
			Node* newNode2;
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

		Node* CreateNode(bool isLeaf, size_t count)
		{
			mNewNodes.Reserve(mNewNodes.GetCount() + 1);
			Node* node = &Node::Create(mNodeParams, isLeaf, count);
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

		Node* GrowLeafNode(Node* node, size_t itemIndex)
		{
			MOMO_ASSERT(node->IsLeaf());
			mOldNodes.AddBack(node);
			size_t itemCount = node->GetCount();
			Node* newNode = CreateNode(true, itemCount + 1);
			AddSegment(node, 0, newNode, 0, itemIndex);
			AddSegment(node, itemIndex, newNode, itemIndex + 1, itemCount - itemIndex);
			return newNode;
		}

		SplitResult SplitNode(Node* node, size_t itemIndex)
		{
			SplitResult splitRes = _SplitNode(node, itemIndex);
			if (!node->IsLeaf())
			{
				size_t itemCount = node->GetCount();
				Node* newNode = splitRes.newNode1;
				size_t newIndex = 0;
				for (size_t i = 0; i <= itemCount; ++i)
				{
					if (i != itemIndex)
						newNode->SetChild(newIndex, node->GetChild(i));
					newIndex += (i == itemIndex) ? 2 : 1;
					if (i == splitRes.middleIndex)
					{
						newNode = splitRes.newNode2;
						newIndex = 0;
					}
				}
			}
			return splitRes;
		}

		template<typename ItemCreator>
		void RelocateCreate(const ItemCreator& itemCreator, Item* pitem)
		{
			mSrcSegments.AddBack({ nullptr, 0, 0 });
			mDstSegments.AddBack({ nullptr, 0, 0 });
			ItemTraits::RelocateCreate(Iterator(mSrcSegments.GetItems()),
				Iterator(mDstSegments.GetItems()), mItemCount, itemCreator, pitem);
			mSrcSegments.Clear();
			mDstSegments.Clear();
			mItemCount = 0;
			mNewNodes.Swap(mOldNodes);
		}

	private:
		SplitResult _SplitNode(Node* node, size_t itemIndex)
		{
			bool isLeaf = node->IsLeaf();
			mOldNodes.AddBack(node);
			size_t itemCount = node->GetCount();
			size_t middleIndex = itemCount / 2;
			if (itemCount % 2 == 0 && middleIndex > itemIndex)
				--middleIndex;
			if (itemIndex <= middleIndex)
			{
				Node* newNode1 = CreateNode(isLeaf, middleIndex + 1);
				Node* newNode2 = CreateNode(isLeaf, itemCount - middleIndex - 1);
				AddSegment(node, 0, newNode1, 0, itemIndex);
				AddSegment(node, itemIndex, newNode1, itemIndex + 1, middleIndex - itemIndex);
				AddSegment(node, middleIndex + 1, newNode2, 0, itemCount - middleIndex - 1);
				return { newNode1, itemIndex, middleIndex, newNode1, newNode2 };
			}
			else
			{
				Node* newNode1 = CreateNode(isLeaf, middleIndex);
				Node* newNode2 = CreateNode(isLeaf, itemCount - middleIndex);
				AddSegment(node, 0, newNode1, 0, middleIndex);
				AddSegment(node, middleIndex + 1, newNode2, 0, itemIndex - middleIndex - 1);
				AddSegment(node, itemIndex, newNode2, itemIndex - middleIndex, itemCount - itemIndex);
				return { newNode2, itemIndex - middleIndex - 1, middleIndex, newNode1, newNode2 };
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
			if (mRootNode != nullptr)
				_Destroy(mRootNode);
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
			if (mRootNode != nullptr)
				_Destroy(mRootNode);
			throw;
		}
	}

	~TreeSet() MOMO_NOEXCEPT
	{
		if (mRootNode != nullptr)
			_Destroy(mRootNode);
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
		return _MakeIterator(node, 0, true);
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		return _MakeIterator(mRootNode, mRootNode->GetCount(), false);
	}

	MOMO_FRIEND_SWAP(TreeSet)
	MOMO_FRIENDS_BEGIN_END(const TreeSet&, ConstIterator)

	const TreeTraits& GetTreeTraits() const MOMO_NOEXCEPT
	{
		return mCrew.GetContainerTraits();
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
		if (mRootNode == nullptr)
			return;
		_Destroy(mRootNode);
		mRootNode = nullptr;
		mCount = 0;
		++mCrew.GetVersion();
	}

	ConstIterator LowerBound(const Key& key) const
	{
		return _LowerBound(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type LowerBound(const KeyArg& key) const
	{
		return _LowerBound(key);
	}

	ConstIterator UpperBound(const Key& key) const
	{
		return _UpperBound(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type UpperBound(const KeyArg& key) const
	{
		return _UpperBound(key);
	}

	ConstIterator Find(const Key& key) const
	{
		return _Find(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type Find(const KeyArg& key) const
	{
		return _Find(key);
	}

	bool HasKey(const Key& key) const
	{
		return _IsEqual(_LowerBound(key), key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, bool>::type HasKey(const KeyArg& key) const
	{
		return _IsEqual(_LowerBound(key), key);
	}

	template<typename ItemCreator>
	InsertResult InsertCrt(const Key& key, const ItemCreator& itemCreator)
	{
		return _Insert(key, itemCreator, true);
	}

	template<typename... ItemArgs>
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key, Creator<ItemArgs...>(std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		return _Insert(ItemTraits::GetKey(const_cast<const Item&>(item)),
			Creator<Item>(std::move(item)), false);
	}

	InsertResult Insert(const Item& item)
	{
		return _Insert(ItemTraits::GetKey(item), Creator<const Item&>(item), false);
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
		return _Add(iter, itemCreator, true);
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

	ConstIterator Remove(ConstIterator iter)
	{
		auto assignFunc1 = [] (Item&& /*srcItem*/) { };
		auto assignFunc2 = [] (Item&& srcItem, Item& dstItem)
			{ ItemTraits::Assign(std::move(srcItem), dstItem); };
		return _Remove(iter, assignFunc1, assignFunc2);
	}

	ConstIterator Remove(ConstIterator iter, Item& resItem)
	{
		auto assignFunc1 = [&resItem] (Item&& srcItem)
			{ ItemTraits::Assign(std::move(srcItem), resItem); };
		auto assignFunc2 = [&resItem] (Item&& srcItem, Item& dstItem)
			{ ItemTraits::Assign(std::move(srcItem), dstItem, resItem); };
		return _Remove(iter, assignFunc1, assignFunc2);
	}

	bool Remove(const Key& key)
	{
		ConstIterator iter = LowerBound(key);
		if (!_IsEqual(iter, key))
			return false;
		Remove(iter);
		return true;
	}

	void Reset(ConstIterator iter, Item&& newItem)
	{
		Item& item = _GetItemForReset(iter, const_cast<const Item&>(newItem));
		ItemTraits::Assign(std::move(newItem), item);
	}

	void Reset(ConstIterator iter, const Item& newItem)
	{
		Item& item = _GetItemForReset(iter, newItem);
		ItemTraits::Assign(newItem, item);
	}

	void Reset(ConstIterator iter, Item&& newItem, Item& resItem)
	{
		Item& item = _GetItemForReset(iter, const_cast<const Item&>(newItem));
		ItemTraits::Assign(std::move(newItem), item, resItem);
	}

	void Reset(ConstIterator iter, const Item& newItem, Item& resItem)
	{
		Item& item = _GetItemForReset(iter, newItem);
		ItemTraits::Assign(newItem, item, resItem);
	}

private:
	ConstIterator _MakeIterator(Node* node, size_t itemIndex, bool move) const MOMO_NOEXCEPT
	{
		return ConstIterator(*node, itemIndex, mCrew.GetVersion(), move);
	}

	template<typename KeyArg>
	bool _IsEqual(ConstIterator iter, const KeyArg& key) const
	{
		return iter != GetEnd() && !GetTreeTraits().IsLess(key, ItemTraits::GetKey(*iter));
	}

	bool _ExtraCheck(ConstIterator iter) const MOMO_NOEXCEPT
	{
		try
		{
			const TreeTraits& treeTraits = GetTreeTraits();
			const Key& key = ItemTraits::GetKey(*iter);
			bool res = (iter == GetBegin()
				|| treeTraits.IsLess(ItemTraits::GetKey(*std::prev(iter)), key));
			res = res && (iter == std::prev(GetEnd())
				|| treeTraits.IsLess(key, ItemTraits::GetKey(*std::next(iter))));
			return res;
		}
		catch (...)
		{
			//?
			return false;
		}
	}

	template<typename KeyArg>
	ConstIterator _LowerBound(const KeyArg& key) const
	{
		if (mRootNode == nullptr)
			return ConstIterator();
		ConstIterator iter = GetEnd();
		Node* node = mRootNode;
		while (true)
		{
			size_t index = _LowerBound(node, key);
			if (index < node->GetCount())
				iter = _MakeIterator(node, index, false);
			if (node->IsLeaf())
				break;
			node = node->GetChild(index);
		}
		return iter;
	}

	template<typename KeyArg>
	ConstIterator _UpperBound(const KeyArg& key) const
	{
		ConstIterator iter = _LowerBound(key);
		if (_IsEqual(iter, key))
			++iter;
		return iter;
	}

	template<typename KeyArg>
	ConstIterator _Find(const KeyArg& key) const
	{
		ConstIterator iter = _LowerBound(key);
		return _IsEqual(iter, key) ? iter : GetEnd();
	}

	template<typename KeyArg>
	size_t _LowerBound(Node* node, const KeyArg& key) const
	{
		size_t leftIndex = 0;
		size_t rightIndex = node->GetCount();
		if (TreeTraits::useLinearSearch)
		{
			for (; leftIndex < rightIndex; ++leftIndex)
			{
				const Item& item = *node->GetItemPtr(leftIndex);
				if (!GetTreeTraits().IsLess(ItemTraits::GetKey(item), key))
					break;
			}
		}
		else
		{
			while (leftIndex < rightIndex)
			{
				size_t middleIndex = (leftIndex + rightIndex) / 2;
				const Item& item = *node->GetItemPtr(middleIndex);
				if (GetTreeTraits().IsLess(ItemTraits::GetKey(item), key))
					leftIndex = middleIndex + 1;
				else
					rightIndex = middleIndex;
			}
		}
		return leftIndex;
	}

	void _Destroy(Node* node) MOMO_NOEXCEPT
	{
		size_t count = node->GetCount();
		for (size_t i = 0; i < count; ++i)
			ItemTraits::Destroy(*node->GetItemPtr(i));
		if (!node->IsLeaf())
		{
			for (size_t i = 0; i <= count; ++i)
				_Destroy(node->GetChild(i));
		}
		node->Destroy(mCrew.GetDetailParams());
	}

	Item& _GetItemForReset(ConstIterator iter, const Item& newItem)
	{
		iter.Check(mCrew.GetVersion());
		MOMO_CHECK(iter != GetEnd());
		const TreeTraits& treeTraits = GetTreeTraits();
		const Key& key = ItemTraits::GetKey(*iter);
		const Key& newKey = ItemTraits::GetKey(newItem);
		MOMO_EXTRA_CHECK(!treeTraits.IsLess(key, newKey) && !treeTraits.IsLess(newKey, key));
		return *iter.GetNode()->GetItemPtr(iter.GetItemIndex());
	}

	template<typename ItemCreator>
	InsertResult _Insert(const Key& key, const ItemCreator& itemCreator, bool extraCheck)
	{
		ConstIterator iter = LowerBound(key);
		if (_IsEqual(iter, key))
			return InsertResult(iter, false);
		iter = _Add(iter, itemCreator, extraCheck);
		return InsertResult(iter, true);
	}

	template<typename ItemCreator>
	ConstIterator _Add(ConstIterator iter, const ItemCreator& itemCreator, bool extraCheck)
	{
		if (mRootNode == nullptr)
			return _AddFirst(iter, itemCreator);
		iter.Check(mCrew.GetVersion());
		Node* node = iter.GetNode();
		size_t itemIndex = iter.GetItemIndex();
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
			itemCreator(node->GetItemPtr(itemCount));
			node->AcceptBackItem(itemIndex);
		}
		else
		{
			Relocator relocator(GetMemManager(), mCrew.GetDetailParams());
			if (itemCount < nodeMaxCapacity)
				_AddGrow(relocator, node, itemIndex, itemCreator);
			else
				_AddSplit(relocator, node, itemIndex, itemCreator);
		}
		++mCount;
		++mCrew.GetVersion();
		ConstIterator resIter = _MakeIterator(node, itemIndex, false);
		(void)extraCheck;
		MOMO_EXTRA_CHECK(!extraCheck || _ExtraCheck(resIter));
		return resIter;
	}

	template<typename ItemCreator>
	ConstIterator _AddFirst(ConstIterator iter, const ItemCreator& itemCreator)
	{
		(void)iter;
		MOMO_CHECK(iter == ConstIterator());
		mRootNode = &Node::Create(mCrew.GetDetailParams(), true, 0);
		try
		{
			return _Add(GetEnd(), itemCreator, false);
		}
		catch (...)
		{
			mRootNode->Destroy(mCrew.GetDetailParams());
			mRootNode = nullptr;
			throw;
		}
	}

	template<typename ItemCreator>
	void _AddGrow(Relocator& relocator, Node*& node, size_t itemIndex,
		const ItemCreator& itemCreator)
	{
		Node* newNode = relocator.GrowLeafNode(node, itemIndex);
		relocator.RelocateCreate(itemCreator, newNode->GetItemPtr(itemIndex));
		Node* parentNode = node->GetParent();
		if (parentNode == nullptr)
			mRootNode = newNode;
		else
			parentNode->SetChild(parentNode->GetChildIndex(node), newNode);
		newNode->SetParent(parentNode);
		node = newNode;
	}

	template<typename ItemCreator>
	void _AddSplit(Relocator& relocator, Node*& leafNode, size_t& leafItemIndex,
		const ItemCreator& itemCreator)
	{
		Node* node = leafNode;
		size_t itemIndex = leafItemIndex;
		typename Relocator::SplitResult splitRes = relocator.SplitNode(node, itemIndex);
		leafNode = splitRes.newNode;
		leafItemIndex = splitRes.newItemIndex;
		while (true)
		{
			size_t childMiddleIndex = splitRes.middleIndex;
			Node* childNode = node;
			node = node->GetParent();
			if (node == nullptr)
			{
				node = relocator.CreateNode(false, 0);
				itemIndex = 0;
				relocator.AddSegment(childNode, childMiddleIndex, node, 0, 1);
				break;
			}
			size_t itemCount = node->GetCount();
			itemIndex = node->GetChildIndex(childNode);
			if (itemCount < node->GetCapacity())
			{
				relocator.AddSegment(childNode, childMiddleIndex, node, itemCount, 1);
				break;
			}
			Node* childNewNode1 = splitRes.newNode1;
			Node* childNewNode2 = splitRes.newNode2;
			splitRes = relocator.SplitNode(node, itemIndex);
			relocator.AddSegment(childNode, childMiddleIndex,
				splitRes.newNode, splitRes.newItemIndex, 1);
			splitRes.newNode->SetChild(splitRes.newItemIndex, childNewNode1);
			splitRes.newNode->SetChild(splitRes.newItemIndex + 1, childNewNode2);
		}
		relocator.RelocateCreate(itemCreator, leafNode->GetItemPtr(leafItemIndex));
		if (node->GetParent() == nullptr)
			mRootNode = node;
		node->AcceptBackItem(itemIndex);
		node->SetChild(itemIndex, splitRes.newNode1);
		node->SetChild(itemIndex + 1, splitRes.newNode2);
		_UpdateParents(node);	//?
	}

	void _UpdateParents(Node* node) MOMO_NOEXCEPT
	{
		size_t count = node->GetCount();
		for (size_t i = 0; i <= count; ++i)
		{
			Node* childNode = node->GetChild(i);
			childNode->SetParent(node);
			if (!childNode->IsLeaf())
				_UpdateParents(childNode);
		}
	}

	template<typename AssignFunc1, typename AssignFunc2>
	ConstIterator _Remove(ConstIterator iter, AssignFunc1 assignFunc1, AssignFunc2 assignFunc2)
	{
		iter.Check(mCrew.GetVersion());
		MOMO_CHECK(iter != GetEnd());
		Node* node = iter.GetNode();
		size_t itemIndex = iter.GetItemIndex();
		if (node->IsLeaf())
		{
			assignFunc1(std::move(*node->GetItemPtr(itemIndex)));
			node->Remove(itemIndex);
			_Rebalance(node, node);
		}
		else
		{
			node = _RemoveInternal(node, itemIndex, assignFunc1, assignFunc2);
			itemIndex = 0;
		}
		--mCount;
		++mCrew.GetVersion();
		return _MakeIterator(node, itemIndex, true);
	}

	template<typename AssignFunc1, typename AssignFunc2>
	Node* _RemoveInternal(Node* node, size_t itemIndex,
		AssignFunc1 assignFunc1, AssignFunc2 assignFunc2)
	{
		Node* childNode = node->GetChild(itemIndex);
		while (!childNode->IsLeaf())
			childNode = childNode->GetChild(childNode->GetCount());
		while (childNode != node && childNode->GetCount() == 0)
			childNode = childNode->GetParent();
		Node* resNode;
		if (childNode == node)
		{
			assignFunc1(std::move(*node->GetItemPtr(itemIndex)));
			Node* remNode = node->GetChild(itemIndex + 1);
			_Destroy(node->GetChild(itemIndex));
			node->Remove(itemIndex);
			node->SetChild(itemIndex, remNode);
			resNode = node->GetChild(itemIndex);
		}
		else
		{
			size_t childItemIndex = childNode->GetCount() - 1;
			assignFunc2(std::move(*childNode->GetItemPtr(childItemIndex)),
				*node->GetItemPtr(itemIndex));
			if (childNode->IsLeaf())
			{
				childNode->Remove(childItemIndex);
			}
			else
			{
				Node* remNode = childNode->GetChild(childItemIndex);
				_Destroy(childNode->GetChild(childItemIndex + 1));
				childNode->Remove(childItemIndex);
				childNode->SetChild(childItemIndex, remNode);
			}
			resNode = node->GetChild(itemIndex + 1);
		}
		while (!resNode->IsLeaf())
			resNode = resNode->GetChild(0);
		_Rebalance(childNode, resNode);
		return resNode;
	}

	void _Rebalance(Node* node, Node* savedNode) MOMO_NOEXCEPT
	{
		try
		{
			while (true)
			{
				Node* parentNode = node->GetParent();
				MOMO_ASSERT(parentNode != savedNode);
				if (parentNode == nullptr)
				{
					MOMO_ASSERT(mRootNode == node);
					if (node->GetCount() == 0 && !node->IsLeaf())
					{
						MOMO_ASSERT(node != savedNode);
						mRootNode = node->GetChild(0);
						mRootNode->SetParent(nullptr);
						node->Destroy(mCrew.GetDetailParams());
					}
					break;
				}
				size_t index = parentNode->GetChildIndex(node);
				bool brk = !_Rebalance(parentNode, index, savedNode)
					&& !_Rebalance(parentNode, index + 1, savedNode);
				if (brk)
					break;
				node = parentNode;
			}
		}
		catch (...)
		{
			// no throw!
		}
	}

	bool _Rebalance(Node* parentNode, size_t index, Node* savedNode)
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
		if (itemCount1 + itemCount2 >= node1->GetCapacity())
			return false;
		Relocator relocator(GetMemManager(), mCrew.GetDetailParams());
		relocator.AddSegment(node2, 0, node1, itemCount1 + 1, itemCount2);
		relocator.RelocateCreate(Creator<Item>(std::move(*parentNode->GetItemPtr(index))),
			node1->GetItemPtr(itemCount1));
		parentNode->Remove(index);
		parentNode->SetChild(index, node1);
		Node* lastChildNode = node1->IsLeaf() ? nullptr : node1->GetChild(itemCount1);
		for (size_t i = 0; i <= itemCount2; ++i)
			node1->AcceptBackItem(itemCount1 + i);
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
		node2->Destroy(mCrew.GetDetailParams());
		return true;
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
