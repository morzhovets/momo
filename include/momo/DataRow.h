/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataRow.h

\**********************************************************/

#pragma once

#include "IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TRefVisitor>
	class DataPtrVisitor
	{
	public:
		typedef TRefVisitor RefVisitor;

	public:
		explicit DataPtrVisitor(FastCopyableFunctor<RefVisitor> refVisitor) noexcept
			: mRefVisitor(refVisitor)
		{
		}

		template<typename QItem, typename ColumnInfo>
		void operator()(QItem* item, const ColumnInfo& columnInfo) const
		{
			if constexpr (!std::is_void_v<QItem>)
			{
				if constexpr (conceptConstFunctor<RefVisitor, void, QItem&, const ColumnInfo&>)
					mRefVisitor(*item, columnInfo);
				else if constexpr (conceptConstFunctor<RefVisitor, void, QItem&>)
					mRefVisitor(*item);
				else
					pvVisitError();
			}
			else
			{
				pvVisitError();
			}
		}

	private:
		[[noreturn]] static void pvVisitError()
		{
			MOMO_THROW(std::logic_error("Visit unknown type"));
		}

	private:
		FastCopyableFunctor<RefVisitor> mRefVisitor;
	};

	template<typename TColumnList, typename TRawMemPool>
	class DataRow : public Swappable
	{
	protected:
		typedef TRawMemPool RawMemPool;

	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

	public:
		DataRow() = delete;

		DataRow(DataRow&& row) noexcept
			: mColumnList(row.mColumnList),
			mRaw(std::exchange(row.mRaw, nullptr)),
			mRawMemPool(std::exchange(row.mRawMemPool, nullptr))
		{
		}

		DataRow(const DataRow&) = delete;

		~DataRow() noexcept
		{
			if (mRaw == nullptr)
				return;
			mColumnList->DestroyRaw(nullptr, mRaw);
			mRawMemPool->DeallocateLazy(mRaw);
		}

		DataRow& operator=(DataRow&& row) noexcept
		{
			DataRow(std::move(row)).Swap(*this);
			return *this;
		}

		DataRow& operator=(const DataRow&) = delete;

		void Swap(DataRow& row) noexcept
		{
			std::swap(mColumnList, row.mColumnList);
			std::swap(mRaw, row.mRaw);
			std::swap(mRawMemPool, row.mRawMemPool);
		}

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		template<typename Item>
		const Item& GetByOffset(size_t offset) const
		{
			return ColumnList::template GetByOffset<const Item>(mRaw, offset);
		}

		template<typename Item>
		Item& GetByOffset(size_t offset)
		{
			return ColumnList::template GetByOffset<Item>(mRaw, offset);
		}

		template<typename Item>
		MOMO_FORCEINLINE const Item& Get(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		MOMO_FORCEINLINE Item& Get(const Column<Item>& column)
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		MOMO_FORCEINLINE const Item& operator[](const Column<Item>& column) const
		{
			return Get(column);
		}

		template<typename Item>
		MOMO_FORCEINLINE Item& operator[](const Column<Item>& column)
		{
			return Get(column);
		}

		template<typename Item>
		MOMO_FORCEINLINE const Item& operator->*(const Column<Item>& column) const
		{
			return Get(column);
		}

		template<typename Item>
		MOMO_FORCEINLINE Item& operator->*(const Column<Item>& column)
		{
			return Get(column);
		}

		template<typename PtrVisitor>	// ptrVisitor(const auto* item [, const ColumnInfo& columnInfo])
		void VisitPointers(PtrVisitor ptrVisitor) const
		{
			mColumnList->VisitPointers(mRaw, FastCopyableFunctor<PtrVisitor>(ptrVisitor));
		}

		template<typename PtrVisitor>	// ptrVisitor(auto* item [, const ColumnInfo& columnInfo])
		void VisitPointers(PtrVisitor ptrVisitor)
		{
			mColumnList->VisitPointers(mRaw, FastCopyableFunctor<PtrVisitor>(ptrVisitor));
		}

		template<typename RefVisitor>	// refVisitor(const auto& item [, const ColumnInfo& columnInfo])
		void VisitReferences(RefVisitor refVisitor) const
		{
			VisitPointers(DataPtrVisitor<RefVisitor>(FastCopyableFunctor<RefVisitor>(refVisitor)));
		}

		template<typename RefVisitor>	// refVisitor(auto& item [, const ColumnInfo& columnInfo])
		void VisitReferences(RefVisitor refVisitor)
		{
			VisitPointers(DataPtrVisitor<RefVisitor>(FastCopyableFunctor<RefVisitor>(refVisitor)));
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		Raw* GetRaw() noexcept
		{
			return mRaw;
		}

		const Raw* operator->() const noexcept
		{
			return GetRaw();
		}

		Raw* operator->() noexcept
		{
			return GetRaw();
		}

	protected:
		explicit DataRow(const ColumnList* columnList, Raw* raw, RawMemPool* rawMemPool) noexcept
			: mColumnList(columnList),
			mRaw(raw),
			mRawMemPool(rawMemPool)
		{
		}

		Raw* ptGetRaw() const noexcept
		{
			return mRaw;
		}

		Raw* ptExtractRaw() noexcept
		{
			Raw* raw = mRaw;
			mRaw = nullptr;
			return raw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
		RawMemPool* mRawMemPool;
	};

	template<typename TColumnList>
	class DataConstRowReference : private VersionKeeper<typename TColumnList::Settings>
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataConstRowReference ConstReference;

	protected:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		DataConstRowReference() = delete;

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		template<typename Item>
		const Item& GetByOffset(size_t offset) const
		{
			VersionKeeper::Check();
			return ColumnList::template GetByOffset<const Item>(mRaw, offset);
		}

		template<typename Item>
		MOMO_FORCEINLINE const Item& Get(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		MOMO_FORCEINLINE const Item& operator[](const Column<Item>& column) const
		{
			return Get(column);
		}

		template<typename Item>
		MOMO_FORCEINLINE const Item& operator->*(const Column<Item>& column) const
		{
			return Get(column);
		}

		size_t GetNumber() const
			requires (Settings::keepRowNumber)
		{
			return mColumnList->GetNumber(GetRaw());
		}

		template<typename PtrVisitor>	// ptrVisitor(const auto* item [, const ColumnInfo& columnInfo])
		void VisitPointers(PtrVisitor ptrVisitor) const
		{
			mColumnList->VisitPointers(GetRaw(), FastCopyableFunctor<PtrVisitor>(ptrVisitor));
		}

		template<typename RefVisitor>	// refVisitor(const auto& item [, const ColumnInfo& columnInfo])
		void VisitReferences(RefVisitor refVisitor) const
		{
			VisitPointers(DataPtrVisitor<RefVisitor>(FastCopyableFunctor<RefVisitor>(refVisitor)));
		}

		const Raw* GetRaw() const
		{
			VersionKeeper::Check();
			return mRaw;
		}

		const Raw* operator->() const
		{
			return GetRaw();
		}

	protected:
		explicit DataConstRowReference(const ColumnList* columnList, Raw* raw,
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mColumnList(columnList),
			mRaw(raw)
		{
		}

		Raw* ptGetRaw() const noexcept
		{
			return mRaw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
	};

	template<typename TColumnList>
	class DataRowReference : public DataConstRowReference<TColumnList>
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataConstRowReference<ColumnList> ConstReference;

	public:
		using ConstReference::ConstReference;

		template<typename Item>
		Item& GetMutableByOffset(size_t offset) const
		{
			ConstReference::GetRaw();	// check
			MOMO_CHECK(ConstReference::GetColumnList().IsMutable(offset));
			return ColumnList::template GetByOffset<Item>(ptGetRaw(), offset);
		}

		template<typename Item>
		Item& GetMutable(const Column<Item>& column) const
		{
			return GetMutableByOffset<Item>(ConstReference::GetColumnList().GetOffset(column));
		}

	protected:
		Raw* ptGetRaw() const noexcept
		{
			return ConstReference::ptGetRaw();
		}
	};

	template<typename TRowBounds>
	class DataRowPointer : public TRowBounds
	{
	protected:
		typedef TRowBounds RowBounds;

	public:
		typedef typename RowBounds::Iterator::Reference Reference;

		typedef DataRowPointer<typename RowBounds::ConstBounds> ConstPointer;

		typedef typename RowBounds::Settings Settings;

	public:
		using RowBounds::RowBounds;

		explicit DataRowPointer(RowBounds rowBounds) noexcept
			: RowBounds(rowBounds)
		{
		}

		operator ConstPointer() const noexcept
		{
			return ConstPointer(static_cast<const RowBounds&>(*this));
		}

		decltype(auto) operator->() const
		{
			MOMO_CHECK(RowBounds::GetCount() == 1);
			return RowBounds::GetBegin();
		}

		Reference operator*() const
		{
			MOMO_CHECK(RowBounds::GetCount() == 1);
			return *RowBounds::GetBegin();
		}

		bool operator!() const noexcept
		{
			return RowBounds::GetCount() == 0;
		}

		explicit operator bool() const noexcept
		{
			return !!*this;
		}
	};
}

} // namespace momo
