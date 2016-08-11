/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataColumn.h

  namespace momo::experimental:
    struct DataColumnTraits
    class DataColumnList
    class DataColumnListStatic

\**********************************************************/

#pragma once

#include "ObjectManager.h"

#include <bitset>

namespace momo
{

namespace experimental
{

template<typename TStruct>
struct DataColumnTraits
{
	typedef TStruct Struct;

	template<typename Type>
	using Column = Type Struct::*;

	template<typename Type>
	static size_t GetCode(const Column<Type>& column) noexcept
	{
		//? typeid(Type).hash_code()
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
	}

	template<typename Type>
	static constexpr size_t GetSize(/*const Column<Type>& column*/) noexcept
	{
		return sizeof(Type);
	}

	template<typename Type>
	static constexpr size_t GetAlignment(/*const Column<Type>& column*/) noexcept
	{
		return MOMO_ALIGNMENT_OF(Type);
	}

	template<typename Type>
	static void Create(/*const Column<Type>& column,*/ Type* pitem)
	{
		new(pitem) Type();
	}

	template<typename Type>
	static void Destroy(/*const Column<Type>& column,*/ Type* pitem) noexcept
	{
		pitem->~Type();
	}

	template<typename TypeArg, typename Type>
	static void Assign(TypeArg&& itemArg, Type& item)
	{
		item = std::forward<TypeArg>(itemArg);
	}
};

template<typename TColumnTraits,
	size_t tLogMaxColumnCount = 7>
class DataColumnList
{
public:
	typedef TColumnTraits ColumnTraits;

	static const size_t logMaxColumnCount = tLogMaxColumnCount;
	MOMO_STATIC_ASSERT(logMaxColumnCount <= 15);

	template<typename Type>
	using Column = typename ColumnTraits::template Column<Type>;

	typedef char Raw;

private:
	static const size_t logVertexCount = logMaxColumnCount + 1;	//?
	static const size_t vertexCount = 1 << logVertexCount;

	template<size_t edgeCount>
	class Graph
	{
	private:
		struct Edge
		{
			size_t vertex;
			size_t value;
			Edge* nextEdge;
		};

	public:
		Graph() noexcept
		{
			mEdgeNumber = 0;
			std::fill(mEdges.begin(), mEdges.end(), nullptr);
		}

		Graph(const Graph&) = delete;

		~Graph() noexcept
		{
		}

		Graph& operator=(const Graph&) = delete;

		void AddEdge(size_t vertex1, size_t vertex2, size_t value) noexcept
		{
			MOMO_ASSERT(mEdgeNumber < vertexCount * 2);
			Edge* edge = &mEdgeStorage[mEdgeNumber];
			++mEdgeNumber;
			edge->vertex = vertex2;
			edge->value = value;
			edge->nextEdge = mEdges[vertex1];
			mEdges[vertex1] = edge;
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
		size_t mEdgeNumber;
		std::array<Edge, edgeCount> mEdgeStorage;
		std::array<Edge*, vertexCount> mEdges;
	};

	typedef std::function<void(Raw*)> CreateFunc;
	typedef std::function<void(Raw*)> DestroyFunc;

public:
	template<typename... Types>
	explicit DataColumnList(const Column<Types>&... columns)
	{
		static const size_t columnCount = sizeof...(columns);
		MOMO_STATIC_ASSERT(0 < columnCount && columnCount < (1 << logMaxColumnCount));
		Graph<2 * columnCount> graph;
		_MakeGraph(graph, 0, 1, columns...);
		std::fill(mAddends.begin(), mAddends.end(), 0);
		for (size_t v = 0; v < vertexCount; ++v)
		{
			if (!graph.HasEdge(v) || mAddends[v] != 0)
				continue;
			mAddends[v] = (size_t)1 << (8 * sizeof(size_t) - 1);
			if (!graph.FillAddends(mAddends, v))
				throw std::runtime_error("Cannot create DataColumnListVar");	//?
		}
		mCreateFunc = [] (Raw* raw) { _Create<void, Types...>(raw, 0); };
		mDestroyFunc = [] (Raw* raw) { _Destroy<void, Types...>(raw, 0); };
	}

	static constexpr bool IsMutable(size_t /*offset*/) noexcept
	{
		return false;
	}

	size_t GetTotalSize() const noexcept
	{
		return mTotalSize;
	}

	void CreateRaw(Raw* raw) const
	{
		mCreateFunc(raw);
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		mDestroyFunc(raw);
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const noexcept
	{
		std::pair<size_t, size_t> vertices = _GetVertices(column);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		size_t offset = addend1 + addend2;
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return offset;
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return *reinterpret_cast<const Type*>(raw + offset);
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return *reinterpret_cast<Type*>(raw + offset);
	}

	template<typename Type, typename TypeArg>
	void Assign(Raw* raw, size_t offset, TypeArg&& itemArg) const
	{
		ColumnTraits::Assign(std::forward<TypeArg>(itemArg), GetByOffset<Type>(raw, offset));
	}

private:
	template<size_t edgeCount, typename Type, typename... Types>
	void _MakeGraph(Graph<edgeCount>& graph, size_t offset, size_t maxAlignment, const Column<Type>& column,
		const Column<Types>&... columns) noexcept
	{
		_CorrectOffset<Type>(offset);
		std::pair<size_t, size_t> vertices = _GetVertices(column);
		graph.AddEdge(vertices.first, vertices.second, offset);
		graph.AddEdge(vertices.second, vertices.first, offset);
		offset += ColumnTraits::template GetSize<Type>();
		maxAlignment = std::minmax(maxAlignment,
			ColumnTraits::template GetAlignment<Type>()).second;
		_MakeGraph(graph, offset, maxAlignment, columns...);
	}

	template<size_t edgeCount>
	void _MakeGraph(Graph<edgeCount>& /*graph*/, size_t offset, size_t maxAlignment) noexcept
	{
		mTotalSize = internal::UIntMath<size_t>::Ceil(offset, maxAlignment);
	}

	template<typename Type>
	static std::pair<size_t, size_t> _GetVertices(const Column<Type>& column) noexcept
	{
		size_t code = ColumnTraits::GetCode(column);
		size_t vertex1 = code & (vertexCount - 1);
		size_t vertex2 = (code >> logVertexCount) & (vertexCount - 1);
		if (vertex1 == vertex2)	//?
			++vertex2;
		return std::make_pair(vertex1, vertex2);
	}

	template<typename Void, typename Type, typename... Types>
	static void _Create(Raw* raw, size_t offset)
	{
		_CorrectOffset<Type>(offset);
		ColumnTraits::Create(reinterpret_cast<Type*>(raw + offset));
		try
		{
			_Create<void, Types...>(raw, offset + ColumnTraits::template GetSize<Type>());
		}
		catch (...)
		{
			ColumnTraits::Destroy(reinterpret_cast<Type*>(raw + offset));
			throw;
		}
	}

	template<typename Void>
	static void _Create(Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Void, typename Type, typename... Types>
	static void _Destroy(Raw* raw, size_t offset) noexcept
	{
		_CorrectOffset<Type>(offset);
		ColumnTraits::Destroy(reinterpret_cast<Type*>(raw + offset));
		_Destroy<void, Types...>(raw, offset + ColumnTraits::template GetSize<Type>());
	}

	template<typename Void>
	static void _Destroy(Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Type>
	static void _CorrectOffset(size_t& offset) noexcept
	{
		static const size_t alignment = ColumnTraits::template GetAlignment<Type>();
		offset = ((offset + alignment - 1) / alignment) * alignment;
	}

private:
	size_t mTotalSize;
	std::array<size_t, vertexCount> mAddends;
	CreateFunc mCreateFunc;
	DestroyFunc mDestroyFunc;
};

template<typename TStruct>
class DataColumnListStatic
{
public:
	typedef TStruct Struct;

	template<typename Type>
	using Column = Type Struct::*;

	typedef Struct Raw;

private:
	static const size_t structSize = sizeof(Struct);

	typedef std::bitset<structSize> MutOffsets;

public:
	DataColumnListStatic() noexcept
	{
	}

	template<typename... Types>
	void SetMutable(const Column<Types>&... columns)
	{
		_SetMutable(columns...);
	}

	bool IsMutable(size_t offset) const noexcept
	{
		return mMutOffsets.test(offset);
	}

	static constexpr size_t GetTotalSize() noexcept
	{
		return structSize;
	}

	void CreateRaw(Raw* raw) const
	{
		new(raw) Raw();
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		(void)raw;
		raw->~Raw();
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const noexcept
	{
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < structSize);
		MOMO_ASSERT(offset % alignof(Type) == 0 || offset % alignof(Struct) == 0);
		return *reinterpret_cast<const Type*>(reinterpret_cast<const char*>(raw) + offset);
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < structSize);
		MOMO_ASSERT(offset % alignof(Type) == 0 || offset % alignof(Struct) == 0);
		return *reinterpret_cast<Type*>(reinterpret_cast<char*>(raw) + offset);
	}

	template<typename Type, typename TypeArg>
	void Assign(Raw* raw, size_t offset, TypeArg&& itemArg) const
	{
		GetByOffset<Type>(raw, offset) = std::forward<TypeArg>(itemArg);
	}

private:
	template<typename Type, typename... Types>
	void _SetMutable(const Column<Type>& column, const Column<Types>&... columns)
	{
		mMutOffsets.set(GetOffset(column));
		_SetMutable(columns...);
	}

	void _SetMutable() noexcept
	{
	}

private:
	MutOffsets mMutOffsets;
};

} // namespace experimental

} // namespace momo
