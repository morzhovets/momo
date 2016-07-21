/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataColumn.h

  namespace momo::experimental:
    class DataColumnList
    struct DataColumnTraits
    class DataColumnListVar

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace experimental
{

template<typename TStruct>
class DataColumnList
{
public:
	typedef TStruct Struct;

	template<typename Type>
	using Column = Type Struct::*;

	typedef Struct Raw;

public:
	size_t GetTotalSize() const noexcept
	{
		return sizeof(Struct);
	}

	void CreateRaw(Raw* raw) const
	{
		new(raw) Raw();
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		raw->~Raw();
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const noexcept
	{
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < GetTotalSize());
		MOMO_ASSERT(offset % alignof(Type) == 0);
		return *reinterpret_cast<Type*>(reinterpret_cast<char*>(raw) + offset);
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < GetTotalSize());
		MOMO_ASSERT(offset % alignof(Type) == 0);
		return *reinterpret_cast<const Type*>(reinterpret_cast<const char*>(raw) + offset);
	}

	template<typename Type>
	Type& GetByColumn(Raw* raw, const Column<Type>& column) const noexcept
	{
		return raw->*column;
	}

	template<typename Type>
	const Type& GetByColumn(const Raw* raw, const Column<Type>& column) const noexcept
	{
		return raw->*column;
	}
};

template<typename TStruct>
struct DataColumnTraits
{
	typedef TStruct Struct;

	template<typename Type>
	using Column = Type Struct::*;

	template<typename Type>
	static size_t GetCode(const Column<Type>& column) noexcept
	{
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
	}

	template<typename Type>
	static void Create(Type* pitem)	//? column
	{
		new(pitem) Type();
	}

	template<typename Type>
	static void Destroy(Type* pitem) noexcept
	{
		pitem->~Type();
	}
};

template<typename TColumnTraits>
class DataColumnListVar
{
public:
	typedef TColumnTraits ColumnTraits;

	template<typename Type>
	using Column = typename ColumnTraits::template Column<Type>;

	typedef char Raw;

private:
	typedef std::function<void(Raw*)> CreateFunc;
	typedef std::function<void(Raw*)> DestroyFunc;

public:
	template<typename... Types>
	explicit DataColumnListVar(const Column<Types>&... columns)
	{
		static const size_t count = sizeof...(columns);
		MOMO_STATIC_ASSERT(0 < count && count < 32);
		std::fill_n(mOffsets, 256, SIZE_MAX);
		if (!_FillOffsets(0, 1, columns...))
			throw std::runtime_error("Cannot create DataColumnListVar");
		mCreateFunc = [] (Raw* raw) { _Create<void, Types...>(raw, 0); };
		mDestroyFunc = [] (Raw* raw) { _Destroy<void, Types...>(raw, 0); };
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
		size_t code = _GetCode(column);
		size_t offset = mOffsets[code];
		MOMO_ASSERT(offset != SIZE_MAX);
		MOMO_ASSERT(offset % alignof(Type) == 0);
		return offset;
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < GetTotalSize());
		MOMO_ASSERT(offset % alignof(Type) == 0);
		return *reinterpret_cast<Type*>(raw + offset);
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < GetTotalSize());
		MOMO_ASSERT(offset % alignof(Type) == 0);
		return *reinterpret_cast<const Type*>(raw + offset);
	}

	template<typename Type>
	Type& GetByColumn(Raw* raw, const Column<Type>& column) const
	{
		return GetByOffset<Type>(raw, GetOffset(column));
	}

	template<typename Type>
	const Type& GetByColumn(const Raw* raw, const Column<Type>& column) const
	{
		return GetByOffset<Type>(raw, GetOffset(column));
	}

private:
	template<typename Type, typename... Types>
	bool _FillOffsets(size_t offset, size_t maxAlignment, const Column<Type>& column,
		const Column<Types>&... columns) noexcept
	{
		size_t code = _GetCode(column);
		if (mOffsets[code] != SIZE_MAX)
			return false;
		_CorrectOffset<Type>(offset);
		mOffsets[code] = offset;
		offset += sizeof(Type);
		maxAlignment = std::minmax(maxAlignment, alignof(Type)).second;
		if (_FillOffsets(offset, maxAlignment, columns...))
			return true;
		mOffsets[code] = SIZE_MAX;
		return false;
	}

	bool _FillOffsets(size_t offset, size_t maxAlignment) noexcept
	{
		mTotalSize = internal::UIntMath<size_t>::Ceil(offset, maxAlignment);
		return true;
	}

	template<typename Type>
	static size_t _GetCode(const Column<Type>& column) noexcept
	{
		size_t code = ColumnTraits::GetCode(column);
		code ^= code >> 16;
		code ^= code >> 8;
		return code & 255;
	}

	template<typename Void, typename Type, typename... Types>
	static void _Create(Raw* raw, size_t offset)
	{
		_CorrectOffset<Type>(offset);
		ColumnTraits::Create(reinterpret_cast<Type*>(raw + offset));
		try
		{
			_Create<void, Types...>(raw, offset + sizeof(Type));
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
		_Destroy<void, Types...>(raw, offset + sizeof(Type));
	}

	template<typename Void>
	static void _Destroy(Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Type>
	static void _CorrectOffset(size_t& offset) noexcept
	{
		if (offset > 0)
			offset = internal::UIntMath<size_t>::Ceil(offset, alignof(Type));
	}

private:
	size_t mTotalSize;
	size_t mOffsets[256];
	CreateFunc mCreateFunc;
	DestroyFunc mDestroyFunc;
};

} // namespace experimental

} // namespace momo
