/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/DataColumn.h

  macros:
    MOMO_DATA_COLUMN_STRUCT
    MOMO_DATA_COLUMN_STRING_TAG
    MOMO_DATA_COLUMN_STRING

  namespace momo:
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

#ifndef MOMO_INCLUDE_GUARD_DATA_COLUMN
#define MOMO_INCLUDE_GUARD_DATA_COLUMN

#include "Array.h"
#include "HashSet.h"

#ifndef MOMO_DISABLE_TYPE_INFO
# include <typeinfo>
#endif

#define MOMO_DATA_COLUMN_STRUCT(Struct, name) \
	constexpr momo::DataColumn<decltype(std::declval<Struct>().name), Struct, momo::DataColumnCodeOffset> \
		name{momo::DataColumnCodeOffset(offsetof(Struct, name)), #Struct "." #name}

#define MOMO_DATA_COLUMN_STRING_TAG(Tag, Type, name) \
	constexpr momo::DataColumn<Type, Tag, uint64_t> name{#name}

#define MOMO_DATA_COLUMN_STRING(Type, name) \
	MOMO_DATA_COLUMN_STRING_TAG(momo::DataStructDefault<>, Type, name)

namespace momo
{

enum class DataColumnCodeOffset : size_t
{
};

namespace internal
{
	class StrHasher
	{
	private:
		static const uint64_t fnvBasis64 = 14695981039346656037ull;
		static const uint64_t fnvPrime64 = 1099511628211ull;

	public:
		// Fowler-Noll-Vo hash function (1a)
		static constexpr uint64_t GetHashCode64(const char* str) noexcept
		{
			return (*str == '\0') ? fnvBasis64
				: (GetHashCode64(str + 1) ^ uint64_t{static_cast<unsigned char>(*str)}) * fnvPrime64;
		}
	};

	template<typename Item>
	struct DataMutable
	{
	};

	template<typename Struct,
		bool isStructWithMembers = std::is_class<Struct>::value && !std::is_empty<Struct>::value>
	struct DataColumnCodeSelector
	{
		typedef uint64_t Code;
	};

	template<typename Struct>
	struct DataColumnCodeSelector<Struct, true>
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

	template<typename TStruct, typename TCode>
	class DataColumnInfoBase
	{
	public:
		typedef TStruct Struct;
		typedef TCode Code;

	private:
		template<typename Struct,
			typename = void>
		struct VisitableItemsSelector
		{
			typedef std::tuple<> VisitableItems;
		};

#ifndef MOMO_DISABLE_TYPE_INFO
		template<typename Struct>
		struct VisitableItemsSelector<Struct, Void<typename Struct::VisitableItems>>
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

		template<typename ColumnInfo, typename QVoid, typename PtrVisitor>
		void ptVisit(QVoid* item, const PtrVisitor& ptrVisitor) const
		{
#ifndef MOMO_DISABLE_TYPE_INFO
			pvVisitRec<ColumnInfo, 0>(item, ptrVisitor);
#else
			pvVisit<ColumnInfo>(item, ptrVisitor);
#endif
		}

	private:
#ifndef MOMO_DISABLE_TYPE_INFO
		template<typename ColumnInfo, size_t index, typename QVoid, typename PtrVisitor>
		EnableIf<(index < std::tuple_size<VisitableItems>::value)>
		pvVisitRec(QVoid* item, const PtrVisitor& ptrVisitor) const
		{
			typedef typename std::tuple_element<index, VisitableItems>::type Item;
			if (typeid(Item) == mTypeInfo)
				pvVisit<ColumnInfo>(PtrCaster::FromBytePtr<Item, true, true>(item), ptrVisitor);
			else
				pvVisitRec<ColumnInfo, index + 1>(item, ptrVisitor);
		}

		template<typename ColumnInfo, size_t index, typename QVoid, typename PtrVisitor>
		EnableIf<(index == std::tuple_size<VisitableItems>::value)>
		pvVisitRec(QVoid* item, const PtrVisitor& ptrVisitor) const
		{
			pvVisit<ColumnInfo>(item, ptrVisitor);
		}
#endif

		template<typename ColumnInfo, typename QItem, typename PtrVisitor>
		EnableIf<IsInvocable<const PtrVisitor&, void, QItem*, const ColumnInfo&>::value>
		pvVisit(QItem* item, const PtrVisitor& ptrVisitor) const
		{
			ptrVisitor(item, *static_cast<const ColumnInfo*>(this));
		}

		template<typename ColumnInfo, typename QItem, typename PtrVisitor>
		EnableIf<IsInvocable<const PtrVisitor&, void, QItem*>::value &&
			!IsInvocable<const PtrVisitor&, void, QItem*, const ColumnInfo&>::value>
		pvVisit(QItem* item, const PtrVisitor& ptrVisitor) const
		{
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

#ifndef MOMO_HAS_GUARANTEED_COPY_ELISION
	DataEquality(DataEquality&&) = default;
#endif

	DataEquality(const DataEquality&) = delete;

	~DataEquality() = default;

	DataEquality& operator=(const DataEquality&) = delete;

	template<size_t index>
	const typename std::tuple_element<index, Tuple>::type& Get() const noexcept
	{
		return std::get<index>(mTuple);
	}

	template<typename RightColumn>
	DataEquality<Columns..., RightColumn> And(const RightColumn& rightColumn,
		const typename internal::DataColumnItemSelector<RightColumn>::Item& rightItem) && noexcept
	{
		return pvAnd(rightColumn, rightItem,
			typename internal::SequenceMaker<sizeof...(Columns)>::Sequence());
	}

private:
	template<typename RightColumn,
		typename RightItem = typename internal::DataColumnItemSelector<RightColumn>::Item,
		size_t... sequence>
	DataEquality<Columns..., RightColumn> pvAnd(const RightColumn& rightColumn,
		const RightItem& rightItem, internal::Sequence<sequence...>) const noexcept
	{
		return DataEquality<Columns..., RightColumn>(
			std::pair<const Columns&, const typename internal::DataColumnItemSelector<Columns>::Item&>(
				Get<sequence>().GetColumn(), Get<sequence>().GetItem())...,
			std::pair<const RightColumn&, const RightItem&>(rightColumn, rightItem));
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

#ifndef MOMO_HAS_GUARANTEED_COPY_ELISION
	DataEquality(DataEquality&&) = default;
#endif

	DataEquality(const DataEquality&) = delete;

	~DataEquality() = default;

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
	const DataEquality& Get() const noexcept
	{
		MOMO_STATIC_ASSERT(index == 0);
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

#ifdef MOMO_HAS_DEDUCTION_GUIDES
template<typename Column>
DataEquality(Column, typename internal::DataColumnItemSelector<Column>::Item)
	-> DataEquality<Column>;
#endif

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

#ifndef MOMO_HAS_GUARANTEED_COPY_ELISION
	DataAssignment(DataAssignment&&) = default;
#endif

	DataAssignment(const DataAssignment&) = delete;

	~DataAssignment() = default;

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

#ifdef MOMO_HAS_DEDUCTION_GUIDES
template<typename Column, typename ItemArg>
DataAssignment(Column, ItemArg&)
	-> DataAssignment<Column, ItemArg&>;
#endif

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
		: mCode(internal::StrHasher::GetHashCode64(name)),
		mName(name)
	{
		MOMO_STATIC_ASSERT(std::is_same<Code, uint64_t>::value);
	}

	constexpr explicit DataColumn(Code code, const char* name = "") noexcept
		: mCode(code),
		mName(name)
	{
	}

	//DataColumn(const DataColumn&) = delete;

	//~DataColumn() = default;

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
	constexpr explicit DataColumn() noexcept
	{
	}

	DataColumn(const DataColumn&) = default;

	~DataColumn() = default;

	DataColumn& operator=(const DataColumn&) = default;
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

	template<typename PtrVisitor>
	void Visit(const void* item, const PtrVisitor& ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfo>(item, ptrVisitor);
	}

	template<typename PtrVisitor>
	void Visit(void* item, const PtrVisitor& ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfo>(item, ptrVisitor);
	}

	template<typename Item>
	static DataEquality<Column<Item>> MakeEquality(
		const Column<Item>& column, const internal::Identity<Item>& item) noexcept
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

template<typename TStruct>
class DataColumnInfoNative : public internal::DataColumnInfoBase<TStruct, DataColumnCodeOffset>
{
private:
	typedef internal::DataColumnInfoBase<TStruct, DataColumnCodeOffset> ColumnInfoBase;

public:
	typedef TStruct Struct;
	//using typename ColumnInfoBase::Struct;	// vs2017

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

	template<typename PtrVisitor>
	void Visit(const void* item, const PtrVisitor& ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfoNative>(item, ptrVisitor);
	}

	template<typename PtrVisitor>
	void Visit(void* item, const PtrVisitor& ptrVisitor) const
	{
		ColumnInfoBase::template ptVisit<DataColumnInfoNative>(item, ptrVisitor);
	}

	template<typename Item>
	static DataEquality<Column<Item>> MakeEquality(
		const Column<Item>& column, const internal::Identity<Item>& item) noexcept
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

	typedef ArraySettings<> TableRawsSettings;
	typedef ArraySettings<4, true, true> SelectionRawsSettings;
};

template<typename TStruct = DataStructDefault<>,
	size_t tLogVertexCount = 8>
class DataColumnTraits
{
public:
	typedef TStruct Struct;

	static const size_t logVertexCount = tLogVertexCount;
	MOMO_STATIC_ASSERT(4 <= logVertexCount && logVertexCount < 16);

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
		if (sizeof(ColumnCode) > 4)
			shortCode += static_cast<size_t>(static_cast<uint64_t>(columnCode) >> 32);
		shortCode += shortCode >> 16;
		if (logVertexCount < 8)
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

template<typename TMemManager>
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

	template<typename Item>
	static void Destroy(MemManager* memManager, Item& item) noexcept
	{
		ItemManager<Item>::Destroyer::Destroy(memManager, item);
	}

	template<typename Item>
	static void Copy(MemManager& memManager, const Item& srcItem, Item* dstItem)
	{
		ItemManager<Item>::Copy(memManager, srcItem, dstItem);
	}

	template<typename ItemArg, typename Item>
	static void Assign(ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}
};

template<typename TColumnTraits = DataColumnTraits<>,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = DataItemTraits<TMemManager>,
	typename TSettings = DataSettings<>>
class DataColumnList
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

		~Graph() = default;

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

	typedef internal::NestedArrayIntCap<0, ColumnRecord, MemManagerPtr> Columns;

	typedef void (*RawItemsCreator)(MemManager&, const ColumnRecord*, const DataColumnList*,
		const Raw*, Raw*);
	typedef void (*RawItemsDestroyer)(MemManager*, const ColumnRecord*, Raw*) /*noexcept*/;

	struct FuncRecord
	{
		size_t columnIndex;
		RawItemsCreator rawItemsCreator;
		RawItemsDestroyer rawItemsDestroyer;
	};

	typedef internal::NestedArrayIntCap<0, FuncRecord, MemManagerPtr> FuncRecords;

	typedef internal::NestedArrayIntCap<0, uint8_t, MemManagerPtr> MutableOffsets;

public:
	typedef typename Columns::ConstIterator ConstIterator;
	typedef ConstIterator Iterator;

public:
	DataColumnList()
		: DataColumnList(MemManager())
	{
	}

	explicit DataColumnList(MemManager memManager)
		: mCodeParam(0),
		mTotalSize(Settings::keepRowNumber ? sizeof(size_t) : 0),
		mAlignment(1),
		mColumnCodeSet(ColumnCodeHashTraits(), std::move(memManager)),
		mColumns(MemManagerPtr(GetMemManager())),
		mFuncRecords(MemManagerPtr(GetMemManager())),
		mMutableOffsets(MemManagerPtr(GetMemManager()))
	{
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
		mColumns(std::move(columnList.mColumns)),
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
		mColumns(columnList.mColumns, MemManagerPtr(GetMemManager())),
		mFuncRecords(columnList.mFuncRecords, MemManagerPtr(GetMemManager())),
		mMutableOffsets(columnList.mMutableOffsets, MemManagerPtr(GetMemManager()))
	{
	}

	~DataColumnList() = default;

	DataColumnList& operator=(const DataColumnList&) = delete;

	ConstIterator GetBegin() const noexcept
	{
		return mColumns.GetBegin();
	}

	ConstIterator GetEnd() const noexcept
	{
		return mColumns.GetEnd();
	}

	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(DataColumnList, ConstIterator)

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
		return mColumns.GetCount();
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
	static auto GetBaseColumn(const QualifiedColumn<Item>& column) noexcept
		-> decltype(ColumnTraits::GetBaseColumn(column))
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

	void DestroyRaw(MemManager* memManager, Raw* raw) const noexcept
	{
		for (const FuncRecord& funcRec : mFuncRecords)
			funcRec.rawItemsDestroyer(memManager, &mColumns[funcRec.columnIndex], raw);
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
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return internal::MemCopyer::FromBuffer<size_t>(raw);
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return internal::MemCopyer::ToBuffer(number, raw);
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

	template<typename PtrVisitor>
	void VisitPointers(const Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers(raw, ptrVisitor);
	}

	template<typename PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
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
			throw std::logic_error("Too many columns");
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
				throw std::runtime_error("Cannot add columns");
		}
		FuncRecord funcRec;
		funcRec.columnIndex = initColumnCount;
		funcRec.rawItemsCreator = [] (MemManager& memManager, const ColumnRecord* columns,
			const DataColumnList* srcColumnList, const Raw* srcRaw, Raw* raw)
		{
			if (srcRaw == nullptr)
				pvCreate<Items...>(memManager, columns, nullptr, nullptr, raw);
			else if (srcColumnList == nullptr)
				pvCreate<Items...>(memManager, columns, nullptr, srcRaw, raw);
			else
				pvCreate<Items...>(memManager, columns, srcColumnList, srcRaw, raw);
		};
		funcRec.rawItemsDestroyer = [] (MemManager* memManager, const ColumnRecord* columns, Raw* raw) noexcept
			{ pvDestroy<void, Items...>(memManager, columns, raw); };
		mColumns.Reserve(initColumnCount + columnCount);
		mFuncRecords.Reserve(mFuncRecords.GetCount() + 1);
		mMutableOffsets.SetCount((offset + 7) / 8, uint8_t{0});
		try
		{
			mColumnCodeSet.Insert(columnCodes.begin(), columnCodes.end());
		}
		catch (...)
		{
			for (ColumnCode columnCode : columnCodes)
				mColumnCodeSet.Remove(columnCode);	// no throw
			//mMutableOffsets.SetCount((mTotalSize + 7) / 8);
			throw;
		}
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
		for (const ColumnRecord& columnRec : mColumns)
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
		pvAddEdges<void, Items...>(graph, offset, maxAlignment, codeParam, columnCodes);
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

	template<typename Void, typename Item, typename... Items>
	static void pvAddEdges(Graph& graph, size_t& offset, size_t& maxAlignment, size_t codeParam,
		const ColumnCode* columnCodes)
	{
		static const size_t size = ItemTraits::template GetSize<Item>();
		static const size_t alignment = ItemTraits::template GetAlignment<Item>();
		MOMO_STATIC_ASSERT(internal::ObjectAlignmenter<Item>::Check(alignment, size));
		offset = internal::UIntMath<>::Ceil(offset, alignment);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(*columnCodes, codeParam);
		graph.AddEdges(vertices.first, vertices.second, offset);
		offset += size;
		maxAlignment = std::minmax(maxAlignment, size_t{alignment}).second;
		pvAddEdges<void, Items...>(graph, offset, maxAlignment, codeParam, columnCodes + 1);
	}

	template<typename Void>
	static void pvAddEdges(Graph& /*graph*/, size_t& /*offset*/, size_t& /*maxAlignment*/,
		size_t /*codeParam*/, const ColumnCode* /*columnCodes*/)
	{
	}

	template<typename Item, typename... Items>
	void pvAddColumns(const ColumnCode* columnCodes, const bool* columnMutables,
		const Column<Item>& column, const Column<Items>&... columns) noexcept
	{
		size_t offset = pvGetOffset(*columnCodes);
		mColumns.AddBackNogrow(ColumnRecord(column, offset));
		if (*columnMutables)
			internal::UIntMath<uint8_t>::SetBit(mMutableOffsets.GetItems(), offset);
		pvAddColumns(columnCodes + 1, columnMutables + 1, columns...);
	}

	void pvAddColumns(const ColumnCode* /*columnCodes*/, const bool* /*columnMutables*/) noexcept
	{
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
	static void pvCreate(MemManager& memManager, const ColumnRecord* columns,
		DataColumnListPtr srcColumnList, RawPtr srcRaw, Raw* raw)
	{
		size_t offset = columns->GetOffset();
		Item* item = pvGetItemPtr<Item>(raw, offset);
		const Item* srcItem = nullptr;
		if (!std::is_same<RawPtr, std::nullptr_t>::value)
		{
			size_t srcOffset = offset;
			const DataColumnList* srcColumnListPtr = static_cast<const DataColumnList*>(srcColumnList);
			if (srcColumnListPtr == nullptr //std::is_same<DataColumnListPtr, std::nullptr_t>::value	// gcc 11
				|| srcColumnListPtr->Contains(*columns, &srcOffset))
			{
				srcItem = pvGetItemPtr<Item, true>(static_cast<const Raw*>(srcRaw), srcOffset);
			}
		}
		if (srcItem == nullptr)
			ItemTraits::Create(memManager, item);
		else
			ItemTraits::Copy(memManager, *srcItem, item);
		try
		{
			pvCreate<Items...>(memManager, columns + 1, srcColumnList, srcRaw, raw);
		}
		catch (...)
		{
			ItemTraits::Destroy(&memManager, *pvGetItemPtr<Item, true>(raw, offset));	//?
			throw;
		}
	}

	template<typename DataColumnListPtr, typename RawPtr>
	static void pvCreate(MemManager& /*memManager*/, const ColumnRecord* /*columns*/,
		DataColumnListPtr /*srcColumnList*/, RawPtr /*srcRaw*/, Raw* /*raw*/) noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvDestroy(MemManager* memManager, const ColumnRecord* columns, Raw* raw) noexcept
	{
		size_t offset = columns->GetOffset();
		ItemTraits::Destroy(memManager, *pvGetItemPtr<Item, true>(raw, offset));
		pvDestroy<void, Items...>(memManager, columns + 1, raw);
	}

	template<typename Void>
	static void pvDestroy(MemManager* /*memManager*/, const ColumnRecord* /*columns*/,
		Raw* /*raw*/) noexcept
	{
	}

	void pvCreateRaw(MemManager& memManager, const DataColumnList* srcColumnList,
		const Raw* srcRaw, Raw* raw) const
	{
		size_t funcIndex = 0;
		try
		{
			size_t funcCount = mFuncRecords.GetCount();
			for (; funcIndex < funcCount; ++funcIndex)
			{
				const FuncRecord& funcRec = mFuncRecords[funcIndex];
				funcRec.rawItemsCreator(memManager, &mColumns[funcRec.columnIndex],
					srcColumnList, srcRaw, raw);
			}
		}
		catch (...)
		{
			for (size_t i = 0; i < funcIndex; ++i)
			{
				const FuncRecord& funcRec = mFuncRecords[i];
				funcRec.rawItemsDestroyer(&memManager, &mColumns[funcRec.columnIndex], raw);
			}
			throw;
		}
	}

	template<typename QRaw, typename PtrVisitor>
	void pvVisitPointers(QRaw* raw, const PtrVisitor& ptrVisitor) const
	{
		for (const ColumnRecord& columnRec : mColumns)
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
		typename QByte = internal::ConstLike<internal::Byte, QRaw>>
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
	Columns mColumns;
	FuncRecords mFuncRecords;
	MutableOffsets mMutableOffsets;
};

template<typename TStruct,
	typename TColumnInfo = DataColumnInfo<TStruct>,
	typename TMemManager = MemManagerDefault,
	typename TSettings = DataSettings<>>
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

	MOMO_STATIC_ASSERT(std::is_class<Struct>::value && !std::is_empty<Struct>::value);

private:
	typedef internal::ObjectManager<Raw, MemManager> RawManager;

	typedef internal::NestedArrayIntCap<0, ColumnInfo, MemManager> Columns;

	typedef std::array<uint8_t, (sizeof(Struct) + 7) / 8> MutableOffsets;

	typedef typename ColumnInfo::Code ColumnCode;
	MOMO_STATIC_ASSERT(std::is_same<DataColumnCodeOffset, ColumnCode>::value);

public:
	explicit DataColumnListStatic(MemManager memManager = MemManager())
		: mColumns(std::move(memManager))
	{
		ResetMutable();
	}

	DataColumnListStatic(DataColumnListStatic&& columnList) noexcept
		: mColumns(std::move(columnList.mColumns)),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	DataColumnListStatic(const DataColumnListStatic& columnList)
		: mColumns(columnList.mColumns),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	~DataColumnListStatic() = default;

	DataColumnListStatic& operator=(const DataColumnListStatic&) = delete;

	const MemManager& GetMemManager() const noexcept
	{
		return mColumns.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mColumns.GetMemManager();
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		pvSetMutable(columns...);
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
		return sizeof(Struct) + (Settings::keepRowNumber ? sizeof(size_t) : 0);
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

	void DestroyRaw(MemManager* memManager, Raw* raw) const noexcept
	{
		RawManager::Destroyer::Destroy(memManager, *raw);
	}

	void ImportRaw(MemManager& memManager, const DataColumnListStatic& /*srcColumnList*/,
		const Raw* srcRaw, Raw* raw) const
	{
		RawManager::Copy(memManager, *srcRaw, raw);
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
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return internal::MemCopyer::FromBuffer<size_t>(pvGetBytePtr(raw, sizeof(Struct)));
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		internal::MemCopyer::ToBuffer(number, pvGetBytePtr(raw, sizeof(Struct)));
	}

	bool Contains(const ColumnInfo& columnInfo, size_t* resOffset = nullptr) const noexcept
	{
		if (resOffset != nullptr)
			*resOffset = pvGetOffset(columnInfo.GetCode());
		return true;
	}

	template<typename Item, typename... Items>
	void PrepareForVisitors(const Column<Item>& column, const Column<Items>&... columns)	//?
	{
		static const size_t columnCount = 1 + sizeof...(columns);
		mColumns.Reserve(columnCount);
		mColumns.Clear(false);
		pvAddColumns(column, columns...);
	}

	template<typename PtrVisitor>
	void VisitPointers(const Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers(raw, ptrVisitor);
	}

	template<typename PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers(raw, ptrVisitor);
	}

private:
	template<typename Item, typename... Items>
	void pvSetMutable(const Column<Item>& column, const Column<Items>&... columns)
	{
		internal::UIntMath<uint8_t>::SetBit(mMutableOffsets.data(), GetOffset(column));
		pvSetMutable(columns...);
	}

	void pvSetMutable() noexcept
	{
	}

	MOMO_FORCEINLINE size_t pvGetOffset(ColumnCode columnCode) const noexcept
	{
		size_t offset = static_cast<size_t>(columnCode);	//?
		MOMO_ASSERT(offset < sizeof(Struct));
		return offset;
	}

	template<typename Item, typename... Items>
	void pvAddColumns(const Column<Item>& column, const Column<Items>&... columns) noexcept
	{
		mColumns.AddBackNogrow(ColumnInfo(column));
		pvAddColumns(columns...);
	}

	void pvAddColumns() noexcept
	{
	}

	template<typename QRaw, typename PtrVisitor>
	void pvVisitPointers(QRaw* raw, const PtrVisitor& ptrVisitor) const
	{
		if (mColumns.IsEmpty())
			throw std::logic_error("Not prepared for visitors");
		for (const ColumnInfo& columnInfo : mColumns)
			columnInfo.Visit(pvGetBytePtr(raw, pvGetOffset(columnInfo.GetCode())), ptrVisitor);
	}

	template<typename QRaw>
	static internal::ConstLike<internal::Byte, QRaw>* pvGetBytePtr(QRaw* raw, size_t offset) noexcept
	{
		return internal::PtrCaster::ToBytePtr(raw) + offset;
	}

private:
	Columns mColumns;
	MutableOffsets mMutableOffsets;
};

template<typename TStruct>
using DataColumnListNative = DataColumnListStatic<TStruct, DataColumnInfoNative<TStruct>>;

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_DATA_COLUMN
