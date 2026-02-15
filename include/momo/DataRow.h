/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/DataRow.h

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_DATA_ROW
#define MOMO_INCLUDE_GUARD_DATA_ROW

#include "IteratorUtility.h"

#include <atomic>

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
		explicit DataPtrVisitor(const RefVisitor& refVisitor) noexcept
			: mRefVisitor(refVisitor)
		{
		}

		template<typename ColumnInfo>
		void operator()(const void* /*item*/, const ColumnInfo& /*columnInfo*/) const	//?
		{
			MOMO_THROW(std::logic_error("Visit unknown type"));
		}

		template<typename QItem, typename ColumnInfo>
		EnableIf<IsInvocable<const RefVisitor&, void, QItem&, const ColumnInfo&>::value>
		operator()(QItem* item, const ColumnInfo& columnInfo) const
		{
			mRefVisitor(*item, columnInfo);
		}

		template<typename QItem, typename ColumnInfo>
		EnableIf<IsInvocable<const RefVisitor&, void, QItem&>::value &&
			!IsInvocable<const RefVisitor&, void, QItem&, const ColumnInfo&>::value>
		operator()(QItem* item, const ColumnInfo& /*columnInfo*/) const
		{
			mRefVisitor(*item);
		}

	private:
		const RefVisitor& mRefVisitor;
	};

	template<typename TColumnList>
	class DataRow
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

	protected:
		typedef std::atomic<void*> FreeRaws;

	public:
		DataRow() = delete;

		DataRow(DataRow&& row) noexcept
			: mColumnList(row.mColumnList),
			mRaw(row.mRaw),
			mFreeRaws(row.mFreeRaws)
		{
			row.mRaw = nullptr;
			row.mFreeRaws = nullptr;
		}

		DataRow(const DataRow&) = delete;

		~DataRow() noexcept
		{
			if (mRaw == nullptr)
				return;
			mColumnList->DestroyRaw(nullptr, mRaw);
			void* raw = mRaw;
			while (true)
			{
				void* headRaw = *mFreeRaws;
				MemCopyer::ToBuffer(headRaw, raw);
				if (mFreeRaws->compare_exchange_weak(headRaw, raw))
					break;
			}
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
			std::swap(mFreeRaws, row.mFreeRaws);
		}

		MOMO_FRIEND_SWAP(DataRow)

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

		template<typename PtrVisitor>	// ptrVisitor(const auto* item [, const ColumnInfo& columnInfo])
		void VisitPointers(const PtrVisitor& ptrVisitor) const
		{
			mColumnList->VisitPointers(mRaw, ptrVisitor);
		}

		template<typename PtrVisitor>	// ptrVisitor(auto* item [, const ColumnInfo& columnInfo])
		void VisitPointers(const PtrVisitor& ptrVisitor)
		{
			mColumnList->VisitPointers(mRaw, ptrVisitor);
		}

		template<typename RefVisitor>	// refVisitor(const auto& item [, const ColumnInfo& columnInfo])
		void VisitReferences(const RefVisitor& refVisitor) const
		{
			VisitPointers(DataPtrVisitor<RefVisitor>(refVisitor));
		}

		template<typename RefVisitor>	// refVisitor(auto& item [, const ColumnInfo& columnInfo])
		void VisitReferences(const RefVisitor& refVisitor)
		{
			VisitPointers(DataPtrVisitor<RefVisitor>(refVisitor));
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
		explicit DataRow(const ColumnList* columnList, Raw* raw, FreeRaws* freeRaws) noexcept
			: mColumnList(columnList),
			mRaw(raw),
			mFreeRaws(freeRaws)
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
		FreeRaws* mFreeRaws;
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

		size_t GetNumber() const
		{
			return mColumnList->GetNumber(GetRaw());
		}

		template<typename PtrVisitor>	// ptrVisitor(const auto* item [, const ColumnInfo& columnInfo])
		void VisitPointers(const PtrVisitor& ptrVisitor) const
		{
			mColumnList->VisitPointers(GetRaw(), ptrVisitor);
		}

		template<typename RefVisitor>	// refVisitor(const auto& item [, const ColumnInfo& columnInfo])
		void VisitReferences(const RefVisitor& refVisitor) const
		{
			VisitPointers(DataPtrVisitor<RefVisitor>(refVisitor));
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

		explicit DataRowPointer() noexcept	// vs2015, gcc 5
		{
		}

		explicit DataRowPointer(RowBounds rowBounds) noexcept
			: RowBounds(rowBounds)
		{
		}

		operator ConstPointer() const noexcept
		{
			return ConstPointer(static_cast<const RowBounds&>(*this));
		}

		typename RowBounds::Iterator operator->() const
		{
			MOMO_CHECK(RowBounds::GetCount() == 1);
			return RowBounds::GetBegin();
		}

		Reference operator*() const
		{
			MOMO_CHECK(RowBounds::GetCount() == 1);
			return *RowBounds::GetBegin();
		}

		explicit operator bool() const noexcept
		{
			return RowBounds::GetCount() > 0;
		}
	};
}

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_DATA_ROW
