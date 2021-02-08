/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataColumn.h

  macros:
    MOMO_DATA_COLUMN_STRUCT
    MOMO_DATA_COLUMN_STRING_TAG
    MOMO_DATA_COLUMN_STRING

  namespace momo:
    class DataSettings
    struct DataStructDefault
    class DataColumnTraits
    class DataItemTraits
    class DataColumnList
    class DataColumnListStatic

\**********************************************************/

#pragma once

#include "Array.h"
#include "HashSet.h"

#include <typeinfo>

#define MOMO_DATA_COLUMN_STRUCT(Struct, name) \
	constexpr momo::internal::DataColumn<decltype(std::declval<Struct&>().name), Struct> \
	name(uint64_t{offsetof(Struct, name)}, #name)

#define MOMO_DATA_COLUMN_STRING_TAG(Tag, Type, name) \
	static_assert(!std::is_class<Tag>::value || std::is_empty<Tag>::value); \
	constexpr momo::internal::DataColumn<Type, Tag> \
	name(momo::internal::StrHasher::GetHashCode64(#name), #name)

#define MOMO_DATA_COLUMN_STRING(Type, name) \
	MOMO_DATA_COLUMN_STRING_TAG(momo::DataStructDefault<>, Type, name)

namespace momo
{

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

	template<typename TColumn, typename TItemArg>
	class DataOperator
	{
	public:
		typedef TColumn Column;
		typedef TItemArg ItemArg;

	public:
		explicit DataOperator(const Column& column, ItemArg&& itemArg) noexcept
			: mColumn(column),
			mItemArg(std::forward<ItemArg>(itemArg))
		{
		}

#ifdef _MSC_VER	//?
		DataOperator(DataOperator&& oper) noexcept
			: mColumn(oper.mColumn),
			mItemArg(std::forward<ItemArg>(oper.mItemArg))
		{
			//MOMO_ASSERT(false);
		}
#endif

		DataOperator(const DataOperator&) = delete;

		~DataOperator() = default;

		DataOperator& operator=(const DataOperator&) = delete;

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

	template<typename TColumn, typename TItemArg>
	class DataEqualer : public DataOperator<TColumn, TItemArg>
	{
	private:
		typedef DataOperator<TColumn, TItemArg> Operator;

	public:
		using Operator::Operator;
	};

	template<typename TColumn, typename TItemArg>
	class DataAssigner : public DataOperator<TColumn, TItemArg>
	{
	private:
		typedef DataOperator<TColumn, TItemArg> Operator;

	public:
		using Operator::Operator;
	};

	template<typename Item>
	struct DataMutable
	{
	};

	template<typename TItem, typename TStruct,
		typename TCode = uint64_t>
	class DataColumn : public DataColumn<DataMutable<TItem>, TStruct, TCode>
	{
	public:
		typedef TItem Item;
		typedef TStruct Struct;
		typedef TCode Code;

		typedef DataColumn BaseColumn;

		typedef DataColumn<DataMutable<Item>, Struct, Code> MutableColumn;

		typedef DataEqualer<DataColumn, const Item&> Equaler;

		template<typename ItemArg>
		using Assigner = DataAssigner<DataColumn, ItemArg>;

	public:
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

		Equaler operator==(const Item& item) const noexcept
		{
			return Equaler(*this, item);
		}

		template<typename ItemArg>
		Assigner<ItemArg> operator=(ItemArg&& itemArg) const noexcept
		{
			return Assigner<ItemArg>(*this, std::forward<ItemArg>(itemArg));
		}

	private:
		Code mCode;
		const char* mName;
	};

	template<typename TItem, typename TStruct, typename TCode>
	class DataColumn<DataMutable<TItem>, TStruct, TCode>
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

	template<typename Struct,
		typename = void>
	struct DataVisitableItemsGetter
	{
		typedef std::tuple<> VisitableItems;
	};

	template<typename Struct>
	struct DataVisitableItemsGetter<Struct, std::void_t<typename Struct::VisitableItems>>
	{
		typedef typename Struct::VisitableItems VisitableItems;
	};

	template<typename TStruct,
		typename TCode = uint64_t>
	class DataColumnInfo
	{
	public:
		typedef TStruct Struct;
		typedef TCode Code;

		template<typename Item>
		using Column = DataColumn<Item, Struct, Code>;

		typedef typename DataVisitableItemsGetter<Struct>::VisitableItems VisitableItems;

	public:
		template<typename Item>
		DataColumnInfo(const Column<Item>& column) noexcept
			: mCode(column.GetCode()),
			mName(column.GetName()),
			mTypeInfo(typeid(Item))
		{
		}

		Code GetCode() const noexcept
		{
			return mCode;
		}

		const char* GetName() const noexcept
		{
			return mName;
		}

		const std::type_info& GetTypeInfo() const noexcept
		{
			return mTypeInfo;
		}

		template<typename PtrVisitor>
		void Visit(const void* item, const PtrVisitor& ptrVisitor) const
		{
			pvVisitRec<0>(item, ptrVisitor);
		}

		template<typename PtrVisitor>
		void Visit(void* item, const PtrVisitor& ptrVisitor) const
		{
			pvVisitRec<0>(item, ptrVisitor);
		}

	private:
		template<size_t index, typename Void, typename PtrVisitor>
		void pvVisitRec(Void* item, const PtrVisitor& ptrVisitor) const
		{
			if constexpr (index < std::tuple_size_v<VisitableItems>)
			{
				typedef typename std::tuple_element<index, VisitableItems>::type Item;
				typedef typename std::conditional<std::is_const<Void>::value,
					const Item*, Item*>::type ItemPtr;
				if (typeid(Item) == mTypeInfo)
					pvVisit(static_cast<ItemPtr>(item), ptrVisitor);
				else
					pvVisitRec<index + 1>(item, ptrVisitor);
			}
			else
			{
				return pvVisit(item, ptrVisitor);
			}
		}

		template<typename Item, typename PtrVisitor>
		std::enable_if_t<IsInvocable<const PtrVisitor&, void, Item*, DataColumnInfo>::value> pvVisit(
			Item* item, const PtrVisitor& ptrVisitor) const
		{
			ptrVisitor(item, *this);
		}

		template<typename Item, typename PtrVisitor>
		std::enable_if_t<IsInvocable<const PtrVisitor&, void, Item*>::value &&
			!IsInvocable<const PtrVisitor&, void, Item*, DataColumnInfo>::value>
		pvVisit(Item* item, const PtrVisitor& ptrVisitor) const
		{
			ptrVisitor(item);
		}

	private:
		Code mCode;
		const char* mName;
		const std::type_info& mTypeInfo;
	};
}

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

template<typename... TVisitableItems>
struct DataStructDefault
{
	typedef std::tuple<TVisitableItems...> VisitableItems;
};

template<typename TStruct = DataStructDefault<>,
	size_t tLogVertexCount = 8>
class DataColumnTraits
{
public:
	typedef TStruct Struct;

	static const size_t logVertexCount = tLogVertexCount;
	static_assert(4 <= logVertexCount && logVertexCount < 16);

	static const size_t maxColumnCount = size_t{1} << (logVertexCount - 1);
	static const size_t maxCodeParam = 255;

	typedef internal::DataColumnInfo<Struct> ColumnInfo;

	template<typename Item>
	using Column = typename ColumnInfo::template Column<Item>;

	template<typename Item>
	using QualifiedColumn = Column<Item>;

	typedef typename ColumnInfo::Code ColumnCode;

	typedef HashTraitsOpen<ColumnCode> ColumnCodeHashTraits;

public:
	MOMO_FORCEINLINE static std::pair<size_t, size_t> GetVertices(ColumnCode code,
		size_t codeParam) noexcept
	{
		static const size_t vertexCount1 = (size_t{1} << logVertexCount) - 1;
		size_t shortCode = static_cast<size_t>(code + (code >> 32));
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
		explicit ColumnRecord(ColumnInfo columnInfo, size_t offset) noexcept	//?
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

	typedef std::function<void(MemManager&, const ColumnRecord*, const DataColumnList*,
		const Raw*, Raw*)> CreateFunc;
	typedef std::function<void(MemManager*, const ColumnRecord*, Raw*)> DestroyFunc;

	struct FuncRecord
	{
		size_t columnIndex;
		CreateFunc createFunc;
		DestroyFunc destroyFunc;
	};

	typedef internal::NestedArrayIntCap<0, FuncRecord, MemManagerPtr> FuncRecords;

	typedef internal::NestedArrayIntCap<0, uint8_t, MemManagerPtr> MutableOffsets;

public:
	typedef typename Columns::ConstIterator ConstIterator;
	typedef ConstIterator Iterator;

public:
	explicit DataColumnList(MemManager memManager = MemManager())
		: mCodeParam(0),
		mTotalSize(Settings::keepRowNumber ? sizeof(size_t) : 0),
		mAlignment(Settings::keepRowNumber ? internal::ObjectAlignmenter<size_t>::alignment : 1),
		mColumnCodeSet(ColumnCodeHashTraits(), std::move(memManager)),
		mColumns(MemManagerPtr(GetMemManager())),
		mFuncRecords(MemManagerPtr(GetMemManager())),
		mMutableOffsets(MemManagerPtr(GetMemManager()))
	{
		std::fill(mAddends.begin(), mAddends.end(), 0);
	}

	template<typename Item, typename... Items>
	explicit DataColumnList(const QualifiedColumn<Item>& column,
		const QualifiedColumn<Items>&... columns)
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

	MOMO_FRIENDS_BEGIN_END(const DataColumnList&, ConstIterator)

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
		return (mMutableOffsets[offset / 8] & static_cast<uint8_t>(1 << (offset % 8))) != 0;
	}

	size_t GetTotalSize() const noexcept
	{
		return mTotalSize;
	}

	size_t GetAlignment() const noexcept
	{
		return mAlignment;
	}

	void CreateRaw(Raw* raw)
	{
		pvCreateRaw(nullptr, nullptr, raw);
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		pvDestroyRaw(nullptr, raw);
	}

	void DestroyRaw(Raw* raw) noexcept
	{
		pvDestroyRaw(&GetMemManager(), raw);
	}

	void ImportRaw(const DataColumnList& srcColumnList, const Raw* srcRaw, Raw* raw)
	{
		pvCreateRaw((&srcColumnList != this) ? &srcColumnList : nullptr, srcRaw, raw);
	}

	template<bool extraCheck = true>
	MOMO_FORCEINLINE size_t GetOffset(ColumnInfo columnInfo) const
	{
		ColumnCode code = columnInfo.GetCode();
		MOMO_EXTRA_CHECK(!extraCheck || mColumnCodeSet.ContainsKey(code));
		size_t offset = pvGetOffset(code);
		MOMO_ASSERT(offset < mTotalSize);
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < mTotalSize);
		//MOMO_ASSERT(offset % ItemTraits::template GetAlignment<Item>() == 0);
		return *internal::PtrCaster::Shift<Item>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		ItemTraits::Assign(std::forward<ItemArg>(itemArg), GetByOffset<Item>(raw, offset));
	}

	size_t GetNumber(const Raw* raw) const noexcept
	{
		static_assert(Settings::keepRowNumber);
		return *internal::PtrCaster::Shift<const size_t>(raw, 0);
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		static_assert(Settings::keepRowNumber);
		*internal::PtrCaster::Shift<size_t>(raw, 0) = number;
	}

	bool Contains(ColumnInfo columnInfo, size_t* resOffset = nullptr) const noexcept
	{
		ColumnCode code = columnInfo.GetCode();
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(code, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		if (addend1 == 0 || addend2 == 0)
			return false;
		if (!mColumnCodeSet.ContainsKey(code))
			return false;
		if (resOffset != nullptr)
			*resOffset = addend1 + addend2;
		return true;
	}

	template<typename PtrVisitor>
	void VisitPointers(const Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<const void>(raw, ptrVisitor);
	}

	template<typename PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<void>(raw, ptrVisitor);
	}

private:
	template<typename... Items>
	void pvAdd(const bool* columnMutables, const Column<Items>&... columns)
	{
		static const size_t columnCount = sizeof...(columns);
		size_t initColumnCount = GetCount();
		if (columnCount + initColumnCount > maxColumnCount)
			throw std::logic_error("Too many columns");
		std::array<ColumnCode, columnCount> columnCodes = {{ ColumnInfo(columns).GetCode()... }};
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
		funcRec.createFunc = [] (MemManager& memManager, const ColumnRecord* columns,
			const DataColumnList* srcColumnList, const Raw* srcRaw, Raw* raw)
		{
			if (srcRaw == nullptr)
			{
				pvCreate<std::nullptr_t, std::nullptr_t, Items...>(memManager, columns,
					nullptr, nullptr, raw);
			}
			else if (srcColumnList == nullptr)
			{
				pvCreate<std::nullptr_t, const Raw*, Items...>(memManager, columns,
					nullptr, srcRaw, raw);
			}
			else
			{
				pvCreate<const DataColumnList*, const Raw*, Items...>(memManager, columns,
					srcColumnList, srcRaw, raw);
			}
		};
		funcRec.destroyFunc = [] (MemManager* memManager, const ColumnRecord* columns, Raw* raw)
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
			for (ColumnCode code : columnCodes)
				mColumnCodeSet.Remove(code);	// no throw
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
		static_assert(internal::ObjectAlignmenter<Item>::Check(alignment, size));
		offset = internal::UIntMath<>::Ceil(offset, alignment);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(*columnCodes, codeParam);
		graph.AddEdges(vertices.first, vertices.second, offset);
		offset += size;
		maxAlignment = std::minmax(maxAlignment, size_t{alignment}).second;
		pvAddEdges<Void, Items...>(graph, offset, maxAlignment, codeParam, columnCodes + 1);
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
			mMutableOffsets[offset / 8] |= static_cast<uint8_t>(1 << (offset % 8));
		pvAddColumns(columnCodes + 1, columnMutables + 1, columns...);
	}

	void pvAddColumns(const ColumnCode* /*columnCodes*/, const bool* /*columnMutables*/) noexcept
	{
	}

	MOMO_FORCEINLINE size_t pvGetOffset(ColumnCode code) const noexcept
	{
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(code, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		return addend1 + addend2;
	}

	template<typename DataColumnListPtr, typename RawPtr, typename Item, typename... Items>
	static void pvCreate(MemManager& memManager, const ColumnRecord* columns,
		DataColumnListPtr srcColumnList, RawPtr srcRaw, Raw* raw)
	{
		size_t offset = columns->GetOffset();
		Item* item = internal::PtrCaster::Shift<Item>(raw, offset);
		const Item* srcItem = nullptr;
		if (!std::is_same<RawPtr, std::nullptr_t>::value)
		{
			size_t srcOffset = offset;
			if (std::is_same<DataColumnListPtr, std::nullptr_t>::value ||
				static_cast<const DataColumnList*>(srcColumnList)->Contains(*columns, &srcOffset))
			{
				srcItem = internal::PtrCaster::Shift<const Item>(
					static_cast<const Raw*>(srcRaw), srcOffset);
			}
		}
		if (srcItem == nullptr)
			ItemTraits::Create(memManager, item);
		else
			ItemTraits::Copy(memManager, *srcItem, item);
		try
		{
			pvCreate<DataColumnListPtr, RawPtr, Items...>(memManager, columns + 1,
				srcColumnList, srcRaw, raw);
		}
		catch (...)
		{
			ItemTraits::Destroy(&memManager, *item);
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
		ItemTraits::Destroy(memManager, *internal::PtrCaster::Shift<Item>(raw, offset));
		pvDestroy<void, Items...>(memManager, columns + 1, raw);
	}

	template<typename Void>
	static void pvDestroy(MemManager* /*memManager*/, const ColumnRecord* /*columns*/,
		Raw* /*raw*/) noexcept
	{
	}

	void pvCreateRaw(const DataColumnList* srcColumnList, const Raw* srcRaw, Raw* raw)
	{
		MemManager& memManager = GetMemManager();
		size_t funcIndex = 0;
		try
		{
			size_t funcCount = mFuncRecords.GetCount();
			for (; funcIndex < funcCount; ++funcIndex)
			{
				const FuncRecord& funcRec = mFuncRecords[funcIndex];
				funcRec.createFunc(memManager, &mColumns[funcRec.columnIndex],
					srcColumnList, srcRaw, raw);
			}
		}
		catch (...)
		{
			for (size_t i = 0; i < funcIndex; ++i)
			{
				const FuncRecord& funcRec = mFuncRecords[i];
				funcRec.destroyFunc(&memManager, &mColumns[funcRec.columnIndex], raw);
			}
			throw;
		}
	}

	void pvDestroyRaw(MemManager* memManager, Raw* raw) const noexcept
	{
		for (const FuncRecord& funcRec : mFuncRecords)
			funcRec.destroyFunc(memManager, &mColumns[funcRec.columnIndex], raw);
	}

	template<typename Void, typename PtrVisitor>
	void pvVisitPointers(Void* raw, const PtrVisitor& ptrVisitor) const
	{
		for (const ColumnRecord& columnRec : mColumns)
		{
			Void* item = internal::PtrCaster::Shift<Void>(raw, columnRec.GetOffset());
			columnRec.Visit(item, ptrVisitor);	//?
		}
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
	typename TMemManager = MemManagerDefault,
	typename TSettings = DataSettings<>>
class DataColumnListStatic
{
public:
	typedef TStruct Struct;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	typedef internal::DataColumnInfo<Struct> ColumnInfo;

	template<typename Item>
	using Column = typename ColumnInfo::template Column<Item>;

	template<typename Item>
	using QualifiedColumn = Column<Item>;	//?

	typedef Struct Raw;

	static_assert(std::is_class<Struct>::value);

private:
	typedef internal::ObjectManager<Raw, MemManager> RawManager;

	typedef internal::NestedArrayIntCap<0, ColumnInfo, MemManager> Columns;

	typedef std::array<uint8_t, (sizeof(Struct) + 7) / 8> MutableOffsets;

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
		return (mMutableOffsets[offset / 8] & static_cast<uint8_t>(1 << (offset % 8))) != 0;
	}

	size_t GetTotalSize() const noexcept
	{
		size_t totalSize = sizeof(Struct);
		if (Settings::keepRowNumber)
		{
			totalSize = internal::UIntMath<>::Ceil(totalSize,
				internal::ObjectAlignmenter<size_t>::alignment);
			totalSize += sizeof(size_t);
		}
		return totalSize;
	}

	size_t GetAlignment() const noexcept
	{
		size_t alignment = RawManager::alignment;
		if (Settings::keepRowNumber)
		{
			alignment = std::minmax(alignment,
				size_t{internal::ObjectAlignmenter<size_t>::alignment}).second;
		}
		return alignment;
	}

	void CreateRaw(Raw* raw)
	{
		(typename RawManager::template Creator<>(GetMemManager()))(raw);
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		RawManager::Destroyer::Destroy(nullptr, *raw);
	}

	void DestroyRaw(Raw* raw) noexcept
	{
		RawManager::Destroy(GetMemManager(), *raw);
	}

	void ImportRaw(const DataColumnListStatic& /*srcColumnList*/, const Raw* srcRaw, Raw* raw)
	{
		RawManager::Copy(GetMemManager(), *srcRaw, raw);
	}

	template<bool extraCheck = true>
	MOMO_FORCEINLINE size_t GetOffset(ColumnInfo columnInfo) const noexcept
	{
		size_t offset = pvGetOffset(columnInfo);
		MOMO_ASSERT(offset < sizeof(Struct));
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < sizeof(Struct));
		//MOMO_ASSERT(offset % internal::ObjectAlignmenter<Item>::alignment == 0);
		return *internal::PtrCaster::Shift<Item>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		GetByOffset<Item>(raw, offset) = std::forward<ItemArg>(itemArg);
	}

	size_t GetNumber(const Raw* raw) const noexcept
	{
		return *internal::PtrCaster::Shift<const size_t>(raw, pvGetNumberOffset());
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		*internal::PtrCaster::Shift<size_t>(raw, pvGetNumberOffset()) = number;
	}

	bool Contains(ColumnInfo columnInfo, size_t* resOffset = nullptr) const noexcept
	{
		if (resOffset != nullptr)
			*resOffset = pvGetOffset(columnInfo);
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
		pvVisitPointers<const void>(raw, ptrVisitor);
	}

	template<typename PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<void>(raw, ptrVisitor);
	}

private:
	template<typename Item, typename... Items>
	void pvSetMutable(const Column<Item>& column, const Column<Items>&... columns)
	{
		size_t offset = GetOffset(column);
		mMutableOffsets[offset / 8] |= static_cast<uint8_t>(1 << (offset % 8));
		pvSetMutable(columns...);
	}

	void pvSetMutable() noexcept
	{
	}

	MOMO_FORCEINLINE size_t pvGetOffset(ColumnInfo columnInfo) const noexcept
	{
		return static_cast<size_t>(columnInfo.GetCode());	//?
	}

	static size_t pvGetNumberOffset() noexcept
	{
		static_assert(Settings::keepRowNumber);
		return internal::UIntMath<>::Ceil(sizeof(Struct),
			internal::ObjectAlignmenter<size_t>::alignment);
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

	template<typename Void, typename PtrVisitor>
	void pvVisitPointers(Void* raw, const PtrVisitor& ptrVisitor) const
	{
		if (mColumns.IsEmpty())
			throw std::logic_error("Not prepared for visitors");
		for (const ColumnInfo& columnInfo : mColumns)
		{
			Void* item = internal::PtrCaster::Shift<Void>(raw, pvGetOffset(columnInfo));
			columnInfo.Visit(item, ptrVisitor);
		}
	}

private:
	Columns mColumns;
	MutableOffsets mMutableOffsets;
};

} // namespace momo
