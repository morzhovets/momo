/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataColumn.h

  macros:
    MOMO_DATA_COLUMN_STRUCT
    MOMO_DATA_COLUMN_STRING_TAG
    MOMO_DATA_COLUMN_STRING

  namespace momo:
    concept conceptDataColumnList
    concept conceptDataStructWithMembers
    enum class DataColumnCodeOffset
    class DataEquality
    class DataAssignment
    struct DataStructDefault
    class DataColumn
    class DataColumnInfo
    class DataColumnInfoNative
    class DataSettings
    class DataColumnTraits
    class DataItemTraits
    class DataColumnList
    class DataColumnListStatic
    class DataColumnListNative

\**********************************************************/

#pragma once

#include "Array.h"
#include "HashSet.h"

#ifndef MOMO_DISABLE_TYPE_INFO
# include <typeinfo>
#endif

#define MOMO_DATA_COLUMN_STRUCT(Struct, name) \
	constexpr momo::DataColumn<decltype(std::declval<Struct>().name), Struct, momo::DataColumnCodeOffset> \
		name{momo::DataColumnCodeOffset{offsetof(Struct, name)}, #Struct "." #name}

#define MOMO_DATA_COLUMN_STRING_TAG(Tag, Type, name) \
	constexpr momo::DataColumn<Type, Tag, uint64_t> name{#name}

#define MOMO_DATA_COLUMN_STRING(Type, name) \
	MOMO_DATA_COLUMN_STRING_TAG(momo::DataStructDefault<>, Type, name)

namespace momo
{

template<typename DataColumnList,
	typename TestItem = void*>
concept conceptDataColumnList =
	std::is_nothrow_destructible_v<DataColumnList> &&
	std::is_nothrow_move_constructible_v<DataColumnList> &&
	conceptMemManager<typename DataColumnList::MemManager> &&
	requires (DataColumnList& columnList, typename DataColumnList::Raw* raw,
		typename DataColumnList::MemManager& memManager,
		const typename DataColumnList::template Column<TestItem>& column)
	{
		typename DataColumnList::Settings;
		{ columnList.GetMemManager() } noexcept
			-> std::same_as<typename DataColumnList::MemManager&>;
		{ std::as_const(columnList).GetTotalSize() } noexcept -> std::same_as<size_t>;
		{ std::as_const(columnList).GetAlignment() } noexcept -> std::same_as<size_t>;
		{ std::as_const(columnList).CreateRaw(memManager, raw) } -> std::same_as<void>;
		{ std::as_const(columnList).DestroyRaw(&memManager, raw) } noexcept -> std::same_as<void>;
		{ std::as_const(columnList).GetOffset(column) } -> std::same_as<size_t>;
		{ DataColumnList::template GetByOffset<TestItem>(raw, size_t{}) } noexcept
			-> std::same_as<TestItem&>;
	};

template<typename DataStruct>
concept conceptDataStructWithMembers =
	std::is_class_v<DataStruct> && !std::is_empty_v<DataStruct>;

enum class DataColumnCodeOffset : size_t
{
};

namespace internal
{
	template<typename Item>
	struct DataMutable
	{
	};

	template<typename Struct>
	struct DataColumnCodeSelector
	{
		typedef uint64_t Code;
	};

	template<conceptDataStructWithMembers Struct>
	struct DataColumnCodeSelector<Struct>
	{
		typedef DataColumnCodeOffset Code;
	};

	template<typename TColumn>
	struct DataColumnItemSelector
	{
		typedef typename TColumn::Item Item;
	};

	template<typename TItem, typename TStruct>
	struct DataColumnItemSelector<TItem TStruct::*>
	{
		typedef TItem Item;	//?
	};

	template<typename DataPtrVisitor, typename Item, typename ColumnInfo>
	concept conceptDataPtrVisitor =
		conceptConstFunctor<DataPtrVisitor, void, Item*> ||
		conceptConstFunctor<DataPtrVisitor, void, Item*, const ColumnInfo&>;

	template<typename TStruct, typename TCode>
	class DataColumnInfoBase
	{
	public:
		typedef TStruct Struct;
		typedef TCode Code;

	private:
		template<typename Struct>
		struct VisitableItemsSelector
		{
			typedef std::tuple<> VisitableItems;
		};

#ifndef MOMO_DISABLE_TYPE_INFO
		template<typename Struct>
		requires requires { typename Struct::VisitableItems; }
		struct VisitableItemsSelector<Struct>
		{
			typedef typename Struct::VisitableItems VisitableItems;
		};
#endif

	public:
		typedef typename VisitableItemsSelector<Struct>::VisitableItems VisitableItems;

	public:
		Code GetCode() const noexcept
		{
			return mCode;
		}

#ifndef MOMO_DISABLE_TYPE_INFO
		const std::type_info& GetTypeInfo() const noexcept
		{
			return mTypeInfo;
		}
#endif

	protected:
		template<typename Item>
		explicit DataColumnInfoBase(Code code, Item*) noexcept
#ifndef MOMO_DISABLE_TYPE_INFO
			: mTypeInfo(typeid(Item))
#endif
		{
			mCode = code;
		}

		template<typename ColumnInfo, typename QVoid,
			conceptDataPtrVisitor<QVoid, ColumnInfo> PtrVisitor>
		void ptVisit(QVoid* item, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
		{
			pvVisitRec<ColumnInfo, 0>(item, ptrVisitor);
		}

	private:
		template<typename ColumnInfo, size_t index, typename QVoid,
			conceptDataPtrVisitor<QVoid, ColumnInfo> PtrVisitor>
		void pvVisitRec(QVoid* item, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
		{
#ifndef MOMO_DISABLE_TYPE_INFO
			if constexpr (index < std::tuple_size_v<VisitableItems>)
			{
				typedef std::tuple_element_t<index, VisitableItems> Item;
				if (typeid(Item) == mTypeInfo)
					pvVisit<ColumnInfo>(PtrCaster::FromBytePtr<Item, true, true>(item), ptrVisitor);
				else
					pvVisitRec<ColumnInfo, index + 1>(item, ptrVisitor);
			}
			else
#endif
			{
				return pvVisit<ColumnInfo>(item, ptrVisitor);
			}
		}

		template<typename ColumnInfo, typename QItem,
			conceptDataPtrVisitor<QItem, ColumnInfo> PtrVisitor>
		void pvVisit(QItem* item, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
		{
			if constexpr (conceptConstFunctor<PtrVisitor, void, QItem*, const ColumnInfo&>)
				ptrVisitor(item, *static_cast<const ColumnInfo*>(this));
			else
				ptrVisitor(item);
		}

	private:
		Code mCode;
#ifndef MOMO_DISABLE_TYPE_INFO
		const std::type_info& mTypeInfo;
#endif
	};
}

template<typename... Columns>
class DataEquality
{
private:
	typedef std::tuple<DataEquality<Columns>...> Tuple;

public:
	DataEquality(std::pair<const Columns&,
		const typename internal::DataColumnItemSelector<Columns>::Item&>... pairs) noexcept
		: mTuple(pairs...)
	{
	}

	DataEquality(const DataEquality&) = delete;

	~DataEquality() noexcept = default;

	DataEquality& operator=(const DataEquality&) = delete;

	template<size_t index>
	const std::tuple_element_t<index, Tuple>& Get() const noexcept
	{
		return std::get<index>(mTuple);
	}

	template<typename RightColumn>
	DataEquality<Columns..., RightColumn> And(const RightColumn& rightColumn,
		const typename internal::DataColumnItemSelector<RightColumn>::Item& rightItem) && noexcept
	{
		return pvAnd(rightColumn, rightItem, std::index_sequence_for<Columns...>());
	}

private:
	template<typename RightColumn, size_t... sequence>
	DataEquality<Columns..., RightColumn> pvAnd(const RightColumn& rightColumn,
		const typename internal::DataColumnItemSelector<RightColumn>::Item& rightItem,
		std::index_sequence<sequence...>) const noexcept
	{
		return DataEquality<Columns..., RightColumn>(
			std::pair<const Columns&, const typename internal::DataColumnItemSelector<Columns>::Item&>(
				Get<sequence>().GetColumn(), Get<sequence>().GetItem())...,
			std::pair<const RightColumn&, const typename internal::DataColumnItemSelector<RightColumn>::Item&>(
				rightColumn, rightItem));
	}

private:
	Tuple mTuple;
};

template<typename Column>
class DataEquality<Column>
{
public:
	typedef typename internal::DataColumnItemSelector<Column>::Item Item;

public:
	DataEquality(const Column& column, const Item& item) noexcept
		: mColumn(column),
		mItem(item)
	{
	}

	DataEquality(std::pair<const Column&, const Item&> pair) noexcept
		: DataEquality(pair.first, pair.second)
	{
	}

	DataEquality(const DataEquality&) = delete;

	~DataEquality() noexcept = default;

	DataEquality& operator=(const DataEquality&) = delete;

	const Column& GetColumn() const noexcept
	{
		return mColumn;
	}

	const Item& GetItem() const noexcept
	{
		return mItem;
	}

	template<size_t index>
	requires (index == 0)
	const DataEquality& Get() const noexcept
	{
		return *this;
	}

	template<typename RightColumn>
	DataEquality<Column, RightColumn> And(const RightColumn& rightColumn,
		const typename internal::DataColumnItemSelector<RightColumn>::Item& rightItem) && noexcept
	{
		typedef typename internal::DataColumnItemSelector<RightColumn>::Item RightItem;
		return DataEquality<Column, RightColumn>(
			std::pair<const Column&, const Item&>(mColumn, mItem),
			std::pair<const RightColumn&, const RightItem&>(rightColumn, rightItem));
	}

	template<typename... LeftColumns>
	friend DataEquality<LeftColumns..., Column> operator&&(
		DataEquality<LeftColumns...> leftEquals, DataEquality<Column> equal) noexcept
	{
		return std::move(leftEquals).And(equal.GetColumn(), equal.GetItem());
	}

private:
	const Column& mColumn;
	const Item& mItem;
};

template<typename Column>
DataEquality(Column, typename internal::DataColumnItemSelector<Column>::Item)
	-> DataEquality<Column>;

template<typename TColumn, typename TItemArg>
class DataAssignment
{
public:
	typedef TColumn Column;
	typedef TItemArg ItemArg;

public:
	DataAssignment(const Column& column, ItemArg&& itemArg) noexcept
		: mColumn(column),
		mItemArg(std::forward<ItemArg>(itemArg))
	{
	}

	DataAssignment(const DataAssignment&) = delete;

	~DataAssignment() noexcept = default;

	DataAssignment& operator=(const DataAssignment&) = delete;

	const Column& GetColumn() const noexcept
	{
		return mColumn;
	}

	ItemArg&& GetItemArg() const noexcept
	{
		return std::forward<ItemArg>(mItemArg);
	}

private:
	const Column& mColumn;
	ItemArg&& mItemArg;
};

template<typename Column, typename ItemArg>
DataAssignment(Column, ItemArg&)
	-> DataAssignment<Column, ItemArg&>;

template<typename... TVisitableItems>
struct DataStructDefault
{
	typedef std::tuple<TVisitableItems...> VisitableItems;
};

template<typename TItem,
	typename TStruct = DataStructDefault<>,
	typename TCode = typename internal::DataColumnCodeSelector<TStruct>::Code>
class DataColumn : public DataColumn<internal::DataMutable<TItem>, TStruct, TCode>
{
public:
	typedef TItem Item;
	typedef TStruct Struct;
	typedef TCode Code;

	typedef DataColumn BaseColumn;
	typedef DataColumn<internal::DataMutable<Item>, Struct, Code> MutableColumn;

public:
	constexpr explicit DataColumn(const char* name) noexcept
		requires std::is_same_v<Code, uint64_t>
		: mCode(internal::StrHasher::GetHashCode64(name)),
		mName(name)
	{
	}

	constexpr explicit DataColumn(Code code, const char* name = "") noexcept
		: mCode(code),
		mName(name)
	{
	}

	//DataColumn(const DataColumn&) = delete;

	//~DataColumn() noexcept = default;

	//DataColumn& operator=(const DataColumn&) = delete;

	const BaseColumn& GetBaseColumn() const noexcept
	{
		return *this;
	}

	constexpr Code GetCode() const noexcept
	{
		return mCode;
	}

	constexpr const char* GetName() const noexcept
	{
		return mName;
	}

	constexpr const MutableColumn& Mutable() const noexcept
	{
		return *this;
	}

	constexpr bool IsMutable() const noexcept
	{
		return false;
	}

	DataEquality<DataColumn> operator==(const Item& item) const noexcept
	{
		return DataEquality<DataColumn>(*this, item);
	}

	template<typename ItemArg>
	DataAssignment<DataColumn, ItemArg> operator=(ItemArg&& itemArg) const noexcept
	{
		return DataAssignment<DataColumn, ItemArg>(*this, std::forward<ItemArg>(itemArg));
	}

private:
	Code mCode;
	const char* mName;
};

template<typename TItem, typename TStruct, typename TCode>
class DataColumn<internal::DataMutable<TItem>, TStruct, TCode>
{
public:
	typedef TItem Item;
	typedef TStruct Struct;
	typedef TCode Code;

	typedef DataColumn<Item, Struct, Code> BaseColumn;

public:
	const BaseColumn& GetBaseColumn() const noexcept
	{
		return static_cast<const BaseColumn&>(*this);
	}

	constexpr bool IsMutable() const noexcept
	{
		return true;
	}

protected:
	explicit DataColumn() noexcept = default;

	DataColumn(const DataColumn&) noexcept = default;

	~DataColumn() noexcept = default;

	DataColumn& operator=(const DataColumn&) noexcept = default;
};

template<typename TStruct = DataStructDefault<>,	//?
	typename TCode = typename internal::DataColumnCodeSelector<TStruct>::Code>
class DataColumnInfo : public internal::DataColumnInfoBase<TStruct, TCode>
{
private:
	typedef internal::DataColumnInfoBase<TStruct, TCode> ColumnInfoBase;

public:
	using typename ColumnInfoBase::Struct;
	using typename ColumnInfoBase::Code;

	template<typename Item>
	using Column = DataColumn<Item, Struct, Code>;

public:
	template<typename Item>
	DataColumnInfo(const Column<Item>& column) noexcept
		: ColumnInfoBase(GetCode(column), static_cast<Item*>(nullptr)),
		mName(column.GetName())
	{
	}

	using ColumnInfoBase::GetCode;

	template<typename Item>
	static Code GetCode(const Column<Item>& column) noexcept
	{
		return column.GetCode();
	}

	const char* GetName() const noexcept
	{
		return mName;
	}

	template<internal::conceptDataPtrVisitor<const void, DataColumnInfo> PtrVisitor>
	void Visit(const void* item, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfo>(item, ptrVisitor);
	}

	template<internal::conceptDataPtrVisitor<void, DataColumnInfo> PtrVisitor>
	void Visit(void* item, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfo>(item, ptrVisitor);
	}

	template<typename Item>
	static DataEquality<Column<Item>> MakeEquality(
		const Column<Item>& column, const std::type_identity_t<Item>& item) noexcept
	{
		return DataEquality<Column<Item>>(column, item);
	}

	template<typename Item, typename ItemArg>
	static DataAssignment<Column<Item>, ItemArg> MakeAssignment(
		const Column<Item>& column, ItemArg&& itemArg) noexcept
	{
		return DataAssignment<Column<Item>, ItemArg>(column, std::forward<ItemArg>(itemArg));
	}

private:
	const char* mName;
};

template<conceptDataStructWithMembers TStruct>
class DataColumnInfoNative : public internal::DataColumnInfoBase<TStruct, DataColumnCodeOffset>
{
private:
	typedef internal::DataColumnInfoBase<TStruct, DataColumnCodeOffset> ColumnInfoBase;

public:
	using typename ColumnInfoBase::Struct;
	using typename ColumnInfoBase::Code;

	template<typename Item>
	using Column = Item Struct::*;

public:
	template<typename Item>
	DataColumnInfoNative(const Column<Item>& column) noexcept
		: ColumnInfoBase(GetCode(column), static_cast<Item*>(nullptr))
	{
	}

	using ColumnInfoBase::GetCode;

	template<typename Item>
	static Code GetCode(const Column<Item>& column) noexcept
	{
		static Struct staticStruct;	//?
		return static_cast<Code>(
			internal::PtrCaster::ToBytePtr(std::addressof(staticStruct.*column))
			- internal::PtrCaster::ToBytePtr(std::addressof(staticStruct)));
	}

	template<internal::conceptDataPtrVisitor<const void, DataColumnInfoNative> PtrVisitor>
	void Visit(const void* item, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfoNative>(item, ptrVisitor);
	}

	template<internal::conceptDataPtrVisitor<void, DataColumnInfoNative> PtrVisitor>
	void Visit(void* item, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfoNative>(item, ptrVisitor);
	}

	template<typename Item>
	static DataEquality<Column<Item>> MakeEquality(
		const Column<Item>& column, const std::type_identity_t<Item>& item) noexcept
	{
		return DataEquality<Column<Item>>(column, item);
	}

	template<typename Item, typename ItemArg>
	static DataAssignment<Column<Item>, ItemArg> MakeAssignment(
		const Column<Item>& column, ItemArg&& itemArg) noexcept
	{
		return DataAssignment<Column<Item>, ItemArg>(column, std::forward<ItemArg>(itemArg));
	}
};

template<bool tKeepRowNumber = false>
class DataSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;

	static const bool keepRowNumber = tKeepRowNumber;

	typedef size_t RowNumber;	//?

	typedef ArraySettings<> TableRawsSettings;
	typedef ArraySettings<4, true, true> SelectionRawsSettings;
};

template<typename TStruct = DataStructDefault<>,
	size_t tLogVertexCount = 8>
requires (4 <= tLogVertexCount && tLogVertexCount < 16)
class DataColumnTraits
{
public:
	typedef TStruct Struct;

	static const size_t logVertexCount = tLogVertexCount;

	static const size_t maxColumnCount = size_t{1} << (logVertexCount - 1);
	static const size_t maxCodeParam = 255;

	typedef DataColumnInfo<Struct> ColumnInfo;

	template<typename Item>
	using Column = typename ColumnInfo::template Column<Item>;

	template<typename Item>
	using QualifiedColumn = Column<Item>;

	typedef typename ColumnInfo::Code ColumnCode;

	typedef HashTraitsOpen<ColumnCode> ColumnCodeHashTraits;

public:
	MOMO_FORCEINLINE static std::pair<size_t, size_t> GetVertices(
		ColumnCode columnCode, size_t codeParam) noexcept
	{
		static const size_t vertexCount1 = (size_t{1} << logVertexCount) - 1;
		size_t shortCode = static_cast<size_t>(columnCode);
		if constexpr (sizeof(ColumnCode) > 4)
			shortCode += static_cast<size_t>(static_cast<uint64_t>(columnCode) >> 32);
		shortCode += shortCode >> 16;
		if constexpr (logVertexCount < 8)
			shortCode += shortCode >> 8;
		size_t vertex1 = (shortCode & vertexCount1) ^ (codeParam >> 4);
		size_t vertex2 = ((shortCode >> logVertexCount) & vertexCount1) ^ (codeParam & 15);
		vertex2 ^= (vertex1 == vertex2) ? 1 : 0;
		return { vertex1, vertex2 };
	}

	template<typename Item>
	static const typename QualifiedColumn<Item>::BaseColumn& GetBaseColumn(
		const QualifiedColumn<Item>& column) noexcept
	{
		return column.GetBaseColumn();
	}

	template<typename Item>
	static bool IsMutable(const QualifiedColumn<Item>& column) noexcept
	{
		return column.IsMutable();
	}
};

template<conceptMemManager TMemManager>
class DataItemTraits
{
public:
	typedef TMemManager MemManager;

private:
	template<typename Item>
	using ItemManager = internal::ObjectManager<Item, MemManager>;

public:
	template<typename Item>
	static constexpr size_t GetSize() noexcept
	{
		return sizeof(Item);
	}

	template<typename Item>
	static constexpr size_t GetAlignment() noexcept
	{
		return ItemManager<Item>::alignment;
	}

	template<typename Item>
	static void Create(MemManager& memManager, Item* item)
	{
		(typename ItemManager<Item>::template Creator<>(memManager))(item);
	}

	template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr, typename Item>
	static void Destroy(MemManagerOrNullPtr memManager, Item& item) noexcept
	{
		ItemManager<Item>::Destroy(memManager, item);
	}

	template<typename Item>
	static void Copy(MemManager& memManager, const Item& srcItem, Item* dstItem)
	{
		ItemManager<Item>::Copy(&memManager, srcItem, dstItem);
	}

	template<typename ItemArg, typename Item>
	static void Assign(ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}
};

template<typename TColumnTraits = DataColumnTraits<>,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TItemTraits = DataItemTraits<TMemManager>,
	typename TSettings = DataSettings<>>
class DataColumnList : public internal::Rangeable
{
public:
	typedef TColumnTraits ColumnTraits;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	typedef typename ColumnTraits::ColumnInfo ColumnInfo;

	template<typename Item>
	using Column = typename ColumnTraits::template Column<Item>;

	template<typename Item>
	using QualifiedColumn = typename ColumnTraits::template QualifiedColumn<Item>;

	typedef void Raw;

	class ColumnRecord : public ColumnInfo
	{
	public:
		explicit ColumnRecord(const ColumnInfo& columnInfo, size_t offset) noexcept	//?
			: ColumnInfo(columnInfo),
			mOffset(offset)
		{
		}

		size_t GetOffset() const noexcept
		{
			return mOffset;
		}

	private:
		size_t mOffset;
	};

private:
	static const size_t logVertexCount = ColumnTraits::logVertexCount;
	static const size_t maxColumnCount = ColumnTraits::maxColumnCount;
	static const size_t maxCodeParam = ColumnTraits::maxCodeParam;

	static const size_t vertexCount = size_t{1} << logVertexCount;

	// http://cmph.sourceforge.net/papers/chm92.pdf
	class Graph
	{
	private:
		struct Edge
		{
			size_t vertex;
			size_t value;
			Edge* nextEdge;
		};

		static const size_t maxEdgeCount = 2 * maxColumnCount;

	public:
		explicit Graph() noexcept
			: mEdgeNumber(0)
		{
			std::fill(mEdges.begin(), mEdges.end(), nullptr);
		}

		Graph(const Graph&) = delete;

		~Graph() noexcept = default;

		Graph& operator=(const Graph&) = delete;

		void AddEdges(size_t vertex1, size_t vertex2, size_t value) noexcept
		{
			MOMO_EXTRA_CHECK(vertex1 != vertex2);
			pvAddEdge(vertex1, vertex2, value);
			pvAddEdge(vertex2, vertex1, value);
		}

		bool HasEdge(size_t vertex) const noexcept
		{
			return mEdges[vertex] != nullptr;
		}

		bool FillAddends(size_t* addends, size_t vertex) const noexcept
		{
			size_t addend = addends[vertex];
			for (Edge* edge = mEdges[vertex]; edge != nullptr; edge = edge->nextEdge)
			{
				size_t vertex2 = edge->vertex;
				size_t& addend2 = addends[vertex2];
				if (addend2 == 0)
				{
					addend2 = edge->value - addend;
					if (!FillAddends(addends, vertex2))
						return false;
				}
				else
				{
					if (addend + addend2 != edge->value)
						return false;
				}
			}
			return true;
		}

	private:
		void pvAddEdge(size_t vertex1, size_t vertex2, size_t value) noexcept
		{
			MOMO_ASSERT(mEdgeNumber < vertexCount * 2);
			Edge* edge = &mEdgeStorage[mEdgeNumber];
			++mEdgeNumber;
			edge->vertex = vertex2;
			edge->value = value;
			edge->nextEdge = mEdges[vertex1];
			mEdges[vertex1] = edge;
		}

	private:
		size_t mEdgeNumber;
		std::array<Edge, maxEdgeCount> mEdgeStorage;
		std::array<Edge*, vertexCount> mEdges;
	};

	typedef std::array<size_t, vertexCount> Addends;

	typedef typename ColumnTraits::ColumnCode ColumnCode;

	typedef typename ColumnTraits::ColumnCodeHashTraits ColumnCodeHashTraits;

	typedef HashSet<ColumnCode, ColumnCodeHashTraits, MemManager,
		HashSetItemTraits<ColumnCode, MemManager>, internal::NestedHashSetSettings> ColumnCodeSet;

	typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef internal::NestedArrayIntCap<0, ColumnRecord, MemManagerPtr> ColumnRecords;

	typedef void (*RawItemsCreator)(MemManager&, const ColumnRecord*, const DataColumnList*,
		const Raw*, Raw*);
	typedef void (*RawItemsDestroyer)(MemManager*, const ColumnRecord*, Raw*) noexcept;

	struct FuncRecord
	{
		size_t columnIndex;
		RawItemsCreator rawItemsCreator;
		RawItemsDestroyer rawItemsDestroyer;
	};

	typedef internal::NestedArrayIntCap<0, FuncRecord, MemManagerPtr> FuncRecords;

	typedef internal::NestedArrayIntCap<0, uint8_t, MemManagerPtr> MutableOffsets;

public:
	typedef typename ColumnRecords::ConstIterator ConstIterator;
	typedef ConstIterator Iterator;

public:
	DataColumnList()
		: DataColumnList(MemManager())
	{
	}

	explicit DataColumnList(MemManager memManager)
		: mCodeParam(0),
		mTotalSize(0),
		mAlignment(1),
		mColumnCodeSet(ColumnCodeHashTraits(), std::move(memManager)),
		mColumnRecords(MemManagerPtr(GetMemManager())),
		mFuncRecords(MemManagerPtr(GetMemManager())),
		mMutableOffsets(MemManagerPtr(GetMemManager()))
	{
		if constexpr (Settings::keepRowNumber)
			mTotalSize += sizeof(typename Settings::RowNumber);
		std::fill(mAddends.begin(), mAddends.end(), 0);
	}

	template<typename Item, typename... Items>
	DataColumnList(const QualifiedColumn<Item>& column, const QualifiedColumn<Items>&... columns)
		: DataColumnList()
	{
		Add(column, columns...);
	}

	DataColumnList(DataColumnList&& columnList) noexcept
		: mCodeParam(columnList.mCodeParam),
		mAddends(columnList.mAddends),
		mTotalSize(columnList.mTotalSize),
		mAlignment(columnList.mAlignment),
		mColumnCodeSet(std::move(columnList.mColumnCodeSet)),
		mColumnRecords(std::move(columnList.mColumnRecords)),
		mFuncRecords(std::move(columnList.mFuncRecords)),
		mMutableOffsets(std::move(columnList.mMutableOffsets))
	{
	}

	DataColumnList(const DataColumnList& columnList)
		: mCodeParam(columnList.mCodeParam),
		mAddends(columnList.mAddends),
		mTotalSize(columnList.mTotalSize),
		mAlignment(columnList.mAlignment),
		mColumnCodeSet(columnList.mColumnCodeSet),
		mColumnRecords(columnList.mColumnRecords, MemManagerPtr(GetMemManager())),
		mFuncRecords(columnList.mFuncRecords, MemManagerPtr(GetMemManager())),
		mMutableOffsets(columnList.mMutableOffsets, MemManagerPtr(GetMemManager()))
	{
	}

	~DataColumnList() noexcept = default;

	DataColumnList& operator=(const DataColumnList&) = delete;

	ConstIterator GetBegin() const noexcept
	{
		return mColumnRecords.GetBegin();
	}

	ConstIterator GetEnd() const noexcept
	{
		return mColumnRecords.GetEnd();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mColumnCodeSet.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mColumnCodeSet.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mColumnRecords.GetCount();
	}

	template<typename Item, typename... Items>
	void Add(const QualifiedColumn<Item>& column, const QualifiedColumn<Items>&... columns)
	{
		static const size_t columnCount = 1 + sizeof...(columns);
		std::array<bool, columnCount> columnMutables = {{ ColumnTraits::IsMutable(column),
			ColumnTraits::IsMutable(columns)... }};
		pvAdd(columnMutables.data(), GetBaseColumn(column), GetBaseColumn(columns)...);
	}

	template<typename Item>
	static decltype(auto) GetBaseColumn(const QualifiedColumn<Item>& column) noexcept
	{
		return ColumnTraits::GetBaseColumn(column);
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		return internal::UIntMath<uint8_t>::GetBit(mMutableOffsets.GetItems(), offset);
	}

	size_t GetTotalSize() const noexcept
	{
		return mTotalSize;
	}

	size_t GetAlignment() const noexcept
	{
		return mAlignment;
	}

	void CreateRaw(MemManager& memManager, Raw* raw) const
	{
		pvCreateRaw(memManager, nullptr, nullptr, raw);
	}

	template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
	void DestroyRaw(MemManagerOrNullPtr memManager, Raw* raw) const noexcept
	{
		for (const FuncRecord& funcRec : mFuncRecords)
		{
			funcRec.rawItemsDestroyer(static_cast<MemManager*>(memManager),
				&mColumnRecords[funcRec.columnIndex], raw);
		}
	}

	void ImportRaw(MemManager& memManager, const DataColumnList& srcColumnList,
		const Raw* srcRaw, Raw* raw) const
	{
		pvCreateRaw(memManager, (&srcColumnList != this) ? &srcColumnList : nullptr, srcRaw, raw);
	}

	template<bool extraCheck = true,
		typename Item>
	MOMO_FORCEINLINE size_t GetOffset(const Column<Item>& column) const
	{
		ColumnCode columnCode = ColumnInfo::GetCode(column);
		MOMO_EXTRA_CHECK(!extraCheck || mColumnCodeSet.ContainsKey(columnCode));
		size_t offset = pvGetOffset(columnCode);
		MOMO_ASSERT(offset < mTotalSize);
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < mTotalSize);
		//MOMO_ASSERT(offset % ItemTraits::template GetAlignment<Item>() == 0);
		return *pvGetItemPtr<Item, true>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		ItemTraits::Assign(std::forward<ItemArg>(itemArg), GetByOffset<Item>(raw, offset));
	}

	size_t GetNumber(const Raw* raw) const noexcept
		requires (Settings::keepRowNumber)
	{
		return size_t{internal::MemCopyer::FromBuffer<typename Settings::RowNumber>(raw)};
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
		requires (Settings::keepRowNumber)
	{
		auto rowNumber = static_cast<typename Settings::RowNumber>(number);
		MOMO_ASSERT(number == size_t{rowNumber});
		return internal::MemCopyer::ToBuffer(rowNumber, raw);
	}

	bool Contains(const ColumnInfo& columnInfo, size_t* resOffset = nullptr) const noexcept
	{
		ColumnCode columnCode = columnInfo.GetCode();
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnCode, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		if (addend1 == 0 || addend2 == 0)
			return false;
		if (!mColumnCodeSet.ContainsKey(columnCode))
			return false;
		if (resOffset != nullptr)
			*resOffset = addend1 + addend2;
		return true;
	}

	template<internal::conceptDataPtrVisitor<const void, ColumnInfo> PtrVisitor>
	void VisitPointers(const Raw* raw, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		pvVisitPointers(raw, ptrVisitor);
	}

	template<internal::conceptDataPtrVisitor<void, ColumnInfo> PtrVisitor>
	void VisitPointers(Raw* raw, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		pvVisitPointers(raw, ptrVisitor);
	}

private:
	template<typename... Items>
	void pvAdd(const bool* columnMutables, const Column<Items>&... columns)
	{
		static const size_t columnCount = sizeof...(columns);
		size_t initColumnCount = GetCount();
		if (columnCount + initColumnCount > maxColumnCount)
			MOMO_THROW(std::logic_error("Too many columns"));
		std::array<ColumnCode, columnCount> columnCodes = {{ ColumnInfo::GetCode(columns)... }};
		Addends addends;
		size_t offset;
		size_t maxAlignment;
		size_t codeParam = mCodeParam;
		while (true)
		{
			std::fill(addends.begin(), addends.end(), 0);
			offset = mTotalSize;
			maxAlignment = mAlignment;
			if (pvFillAddends<Items...>(addends, offset, maxAlignment, codeParam, columnCodes.data()))
				break;
			++codeParam;
			if (codeParam > maxCodeParam)
				MOMO_THROW(std::runtime_error("Cannot add columns"));
		}
		FuncRecord funcRec;
		funcRec.columnIndex = initColumnCount;
		funcRec.rawItemsCreator = [] (MemManager& memManager, const ColumnRecord* columnRecords,
			const DataColumnList* srcColumnList, const Raw* srcRaw, Raw* raw)
		{
			if (srcRaw == nullptr)
				pvCreate<Items...>(memManager, columnRecords, nullptr, nullptr, raw);
			else if (srcColumnList == nullptr)
				pvCreate<Items...>(memManager, columnRecords, nullptr, srcRaw, raw);
			else
				pvCreate<Items...>(memManager, columnRecords, srcColumnList, srcRaw, raw);
		};
		funcRec.rawItemsDestroyer = []
			(MemManager* memManager, const ColumnRecord* columnRecords, Raw* raw) noexcept
		{
			if (memManager != nullptr)
				pvDestroy<Items...>(memManager, columnRecords, raw);
			else
				pvDestroy<Items...>(nullptr, columnRecords, raw);
		};
		mColumnRecords.Reserve(initColumnCount + columnCount);
		mFuncRecords.Reserve(mFuncRecords.GetCount() + 1);
		mMutableOffsets.SetCount((offset + 7) / 8, uint8_t{0});
		auto codeInsertReverter = [this, &columnCodes] () noexcept
		{
			for (ColumnCode columnCode : columnCodes)
				mColumnCodeSet.Remove(columnCode);	// no throw
			//mMutableOffsets.SetCount((mTotalSize + 7) / 8);
		};
		for (internal::Finalizer fin = codeInsertReverter; fin; fin.Detach())
			mColumnCodeSet.Insert(columnCodes.begin(), columnCodes.end());
		mCodeParam = codeParam;
		mAddends = addends;
		mTotalSize = offset;
		mAlignment = maxAlignment;
		pvAddColumns(columnCodes.data(), columnMutables, columns...);
		mFuncRecords.AddBackNogrow(std::move(funcRec));
	}

	template<typename... Items>
	bool pvFillAddends(Addends& addends, size_t& offset, size_t& maxAlignment, size_t codeParam,
		const ColumnCode* columnCodes) const
	{
		Graph graph;
		for (const ColumnRecord& columnRec : mColumnRecords)
		{
			std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnRec.GetCode(),
				codeParam);
			graph.AddEdges(vertices.first, vertices.second, columnRec.GetOffset());
		}
		return pvFillAddends<Items...>(addends, graph, offset, maxAlignment, codeParam, columnCodes);
	}

	template<typename... Items>
	static bool pvFillAddends(Addends& addends, Graph& graph, size_t& offset, size_t& maxAlignment,
		size_t codeParam, const ColumnCode* columnCodes)
	{
		const ColumnCode* columnCodePtr = columnCodes;
		(pvAddEdge<Items>(graph, offset, maxAlignment, codeParam, *columnCodePtr++), ...);
		for (size_t v = 0; v < vertexCount; ++v)
		{
			if (!graph.HasEdge(v) || addends[v] != 0)
				continue;
			addends[v] = size_t{1} << (8 * sizeof(size_t) - 1);
			if (!graph.FillAddends(addends.data(), v))
				return false;
		}
		return true;
	}

	template<typename Item>
	static void pvAddEdge(Graph& graph, size_t& offset, size_t& maxAlignment,
		size_t codeParam, ColumnCode columnCode)
	{
		static const size_t size = ItemTraits::template GetSize<Item>();
		static const size_t alignment = ItemTraits::template GetAlignment<Item>();
		static_assert(internal::ObjectAlignmenter<Item>::Check(alignment, size));
		offset = internal::UIntMath<>::Ceil(offset, alignment);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnCode, codeParam);
		graph.AddEdges(vertices.first, vertices.second, offset);
		offset += size;
		maxAlignment = std::minmax(maxAlignment, size_t{alignment}).second;
	}

	template<typename... Items>
	void pvAddColumns(const ColumnCode* columnCodes, const bool* columnMutables,
		const Column<Items>&... columns) noexcept
	{
		const ColumnCode* columnCodePtr = columnCodes;
		const bool* columnMutablePtr = columnMutables;
		(pvAddColumn(*columnCodePtr++, *columnMutablePtr++, columns), ...);
	}

	template<typename Item>
	void pvAddColumn(ColumnCode columnCode, bool columnMutable, const Column<Item>& column) noexcept
	{
		size_t offset = pvGetOffset(columnCode);
		mColumnRecords.AddBackNogrow(ColumnRecord(column, offset));
		if (columnMutable)
			internal::UIntMath<uint8_t>::SetBit(mMutableOffsets.GetItems(), offset);
	}

	MOMO_FORCEINLINE size_t pvGetOffset(ColumnCode columnCode) const noexcept
	{
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnCode, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		return addend1 + addend2;
	}

	template<typename Item, typename... Items, typename DataColumnListPtr, typename RawPtr>
	static void pvCreate(MemManager& memManager, const ColumnRecord* columnRecordPtr,
		[[maybe_unused]] DataColumnListPtr srcColumnList, [[maybe_unused]] RawPtr srcRaw, Raw* raw)
	{
		size_t offset = columnRecordPtr->GetOffset();
		Item* item = pvGetItemPtr<Item>(raw, offset);
		const Item* srcItem = nullptr;
		if constexpr (!std::is_null_pointer_v<RawPtr>)
		{
			if constexpr (std::is_null_pointer_v<DataColumnListPtr>)
			{
				srcItem = pvGetItemPtr<Item, true>(srcRaw, offset);
			}
			else
			{
				size_t srcOffset;
				if (srcColumnList->Contains(*columnRecordPtr, &srcOffset))
					srcItem = pvGetItemPtr<Item, true>(srcRaw, srcOffset);
			}
		}
		if (srcItem == nullptr)
			ItemTraits::Create(memManager, item);
		else
			ItemTraits::Copy(memManager, *srcItem, item);
		if constexpr (sizeof...(Items) > 0)
		{
			//item = pvGetItemPtr<Item, true>(raw, offset);
			internal::Finalizer fin(&ItemTraits::template Destroy<MemManager*, Item>, &memManager, *item);
			pvCreate<Items...>(memManager, columnRecordPtr + 1, srcColumnList, srcRaw, raw);
			fin.Detach();
		}
	}

	template<typename... Items, internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
	static void pvDestroy(MemManagerOrNullPtr memManager, const ColumnRecord* columnRecords, Raw* raw)
	{
		const ColumnRecord* columnRecordPtr = columnRecords;
		(ItemTraits::Destroy(memManager,
			*pvGetItemPtr<Items, true>(raw, (columnRecordPtr++)->GetOffset())), ...);
	}

	void pvCreateRaw(MemManager& memManager, const DataColumnList* srcColumnList,
		const Raw* srcRaw, Raw* raw) const
	{
		size_t funcIndex = 0;
		auto rawItemsDestroyer = [this, &funcIndex, &memManager, raw] () noexcept
		{
			for (size_t i = 0; i < funcIndex; ++i)
			{
				const FuncRecord& funcRec = mFuncRecords[i];
				funcRec.rawItemsDestroyer(&memManager, &mColumnRecords[funcRec.columnIndex], raw);
			}
		};
		for (internal::Finalizer fin = rawItemsDestroyer; fin; fin.Detach())
		{
			size_t funcCount = mFuncRecords.GetCount();
			for (; funcIndex < funcCount; ++funcIndex)
			{
				const FuncRecord& funcRec = mFuncRecords[funcIndex];
				funcRec.rawItemsCreator(memManager, &mColumnRecords[funcRec.columnIndex],
					srcColumnList, srcRaw, raw);
			}
		}
	}

	template<typename QRaw,
		internal::conceptDataPtrVisitor<internal::ConstLike<void, QRaw>, ColumnInfo> PtrVisitor>
	void pvVisitPointers(QRaw* raw, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		for (const ColumnRecord& columnRec : mColumnRecords)
			columnRec.Visit(pvGetBytePtr(raw, columnRec.GetOffset()), ptrVisitor);	//?
	}

	template<typename Item,
		bool isWithinLifetime = false,
		typename QRaw>
	static internal::ConstLike<Item, QRaw>* pvGetItemPtr(QRaw* raw, size_t offset) noexcept
	{
		return internal::PtrCaster::FromBytePtr<Item, isWithinLifetime, true>(
			pvGetBytePtr(raw, offset));
	}

	template<typename QRaw,
		typename QByte = internal::ConstLike<std::byte, QRaw>>
	static QByte* pvGetBytePtr(QRaw* raw, size_t offset) noexcept
	{
		return static_cast<QByte*>(raw) + offset;
	}

private:
	size_t mCodeParam;
	Addends mAddends;
	size_t mTotalSize;
	size_t mAlignment;
	ColumnCodeSet mColumnCodeSet;
	ColumnRecords mColumnRecords;
	FuncRecords mFuncRecords;
	MutableOffsets mMutableOffsets;
};

template<conceptDataStructWithMembers TStruct,
	typename TColumnInfo = DataColumnInfo<TStruct>,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TSettings = DataSettings<>>
requires std::is_same_v<DataColumnCodeOffset, typename TColumnInfo::Code>
class DataColumnListStatic
{
public:
	typedef TStruct Struct;
	typedef TColumnInfo ColumnInfo;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	template<typename Item>
	using Column = typename ColumnInfo::template Column<Item>;

	typedef Struct Raw;

private:
	typedef internal::ObjectManager<Raw, MemManager> RawManager;

	typedef internal::NestedArrayIntCap<0, ColumnInfo, MemManager> VisitableColumns;

	typedef std::array<uint8_t, (sizeof(Struct) + 7) / 8> MutableOffsets;

	typedef typename ColumnInfo::Code ColumnCode;

public:
	explicit DataColumnListStatic(MemManager memManager = MemManager())
		: mVisitableColumns(std::move(memManager))
	{
		ResetMutable();
	}

	DataColumnListStatic(DataColumnListStatic&& columnList) noexcept
		: mVisitableColumns(std::move(columnList.mVisitableColumns)),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	DataColumnListStatic(const DataColumnListStatic& columnList)
		: mVisitableColumns(columnList.mVisitableColumns),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	~DataColumnListStatic() noexcept = default;

	DataColumnListStatic& operator=(const DataColumnListStatic&) = delete;

	const MemManager& GetMemManager() const noexcept
	{
		return mVisitableColumns.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mVisitableColumns.GetMemManager();
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		(pvSetMutable(GetOffset(columns)), ...);
	}

	void ResetMutable() noexcept
	{
		std::fill(mMutableOffsets.begin(), mMutableOffsets.end(), uint8_t{0});
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < sizeof(Struct));
		return internal::UIntMath<uint8_t>::GetBit(mMutableOffsets.data(), offset);
	}

	size_t GetTotalSize() const noexcept
	{
		size_t totalSize = sizeof(Struct);
		if constexpr (Settings::keepRowNumber)
			totalSize += sizeof(typename Settings::RowNumber);
		return totalSize;
	}

	size_t GetAlignment() const noexcept
	{
		return RawManager::alignment;
	}

	void CreateRaw(MemManager& memManager, Raw* raw) const
	{
		(typename RawManager::template Creator<>(memManager))(raw);
	}

	void CreateRaw(MemManager& memManager, Raw&& srcRaw, Raw* raw) const
	{
		(typename RawManager::template Creator<Raw&&>(memManager, std::move(srcRaw)))(raw);
	}

	template<internal::conceptMemManagerOrNullPtr<MemManager> MemManagerOrNullPtr>
	void DestroyRaw(MemManagerOrNullPtr memManager, Raw* raw) const noexcept
	{
		RawManager::Destroy(memManager, *raw);
	}

	void ImportRaw(MemManager& memManager, const DataColumnListStatic& /*srcColumnList*/,
		const Raw* srcRaw, Raw* raw) const
	{
		RawManager::Copy(&memManager, *srcRaw, raw);
	}

	template<bool extraCheck = true,
		typename Item>
	MOMO_FORCEINLINE size_t GetOffset(const Column<Item>& column) const noexcept
	{
		return pvGetOffset(ColumnInfo::GetCode(column));
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < sizeof(Struct));
		//MOMO_ASSERT(offset % internal::ObjectAlignmenter<Item>::alignment == 0);
		return *internal::PtrCaster::FromBytePtr<Item, true, true>(pvGetBytePtr(raw, offset));
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		GetByOffset<Item>(raw, offset) = std::forward<ItemArg>(itemArg);
	}

	size_t GetNumber(const Raw* raw) const noexcept
		requires (Settings::keepRowNumber)
	{
		return size_t{internal::MemCopyer::FromBuffer<typename Settings::RowNumber>(
			pvGetBytePtr(raw, sizeof(Struct)))};
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
		requires (Settings::keepRowNumber)
	{
		auto rowNumber = static_cast<typename Settings::RowNumber>(number);
		MOMO_ASSERT(number == size_t{rowNumber});
		internal::MemCopyer::ToBuffer(rowNumber, pvGetBytePtr(raw, sizeof(Struct)));
	}

	bool Contains(const ColumnInfo& columnInfo, size_t* resOffset = nullptr) const noexcept
	{
		if (resOffset != nullptr)
			*resOffset = pvGetOffset(columnInfo.GetCode());
		return true;
	}

	template<typename... Items>
	void PrepareForVisitors(const Column<Items>&... columns)	//?
	{
		static const size_t columnCount = sizeof...(columns);
		mVisitableColumns.Reserve(columnCount);
		mVisitableColumns.Clear(false);
		(mVisitableColumns.AddBackNogrow(ColumnInfo(columns)), ...);
	}

	template<internal::conceptDataPtrVisitor<const void, ColumnInfo> PtrVisitor>
	void VisitPointers(const Raw* raw, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		pvVisitPointers(raw, ptrVisitor);
	}

	template<internal::conceptDataPtrVisitor<void, ColumnInfo> PtrVisitor>
	void VisitPointers(Raw* raw, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		pvVisitPointers(raw, ptrVisitor);
	}

private:
	void pvSetMutable(size_t offset) noexcept
	{
		internal::UIntMath<uint8_t>::SetBit(mMutableOffsets.data(), offset);
	}

	MOMO_FORCEINLINE size_t pvGetOffset(ColumnCode columnCode) const noexcept
	{
		size_t offset = static_cast<size_t>(columnCode);
		MOMO_ASSERT(offset < sizeof(Struct));
		return offset;
	}

	template<typename QRaw,
		internal::conceptDataPtrVisitor<internal::ConstLike<void, QRaw>, ColumnInfo> PtrVisitor>
	void pvVisitPointers(QRaw* raw, FastCopyableFunctor<PtrVisitor> ptrVisitor) const
	{
		if (mVisitableColumns.IsEmpty())
			MOMO_THROW(std::logic_error("Not prepared for visitors"));
		for (const ColumnInfo& columnInfo : mVisitableColumns)
			columnInfo.Visit(pvGetBytePtr(raw, pvGetOffset(columnInfo.GetCode())), ptrVisitor);
	}

	template<typename QRaw>
	static internal::ConstLike<std::byte, QRaw>* pvGetBytePtr(QRaw* raw, size_t offset) noexcept
	{
		return internal::PtrCaster::ToBytePtr(raw) + offset;
	}

private:
	VisitableColumns mVisitableColumns;
	MutableOffsets mMutableOffsets;
};

template<conceptDataStructWithMembers TStruct>
using DataColumnListNative = DataColumnListStatic<TStruct, DataColumnInfoNative<TStruct>>;

} // namespace momo
