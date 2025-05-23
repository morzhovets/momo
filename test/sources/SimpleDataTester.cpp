/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/SimpleDataTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_DATA

#include "../../include/momo/DataTable.h"
#include "../../include/momo/MemManagerDict.h"

#include <string>
#include <iostream>
#include <sstream>

namespace
{
	typedef momo::DataStructDefault<int, double, std::string> BaseStruct;

	struct Struct
	{
		typedef std::tuple<int, double, std::string> VisitableItems;

		int intStruct;
		double dblStruct;
		std::string strStruct;
	};

	MOMO_DATA_COLUMN_STRUCT(Struct, intStruct);
	MOMO_DATA_COLUMN_STRUCT(Struct, dblStruct);
	MOMO_DATA_COLUMN_STRUCT(Struct, strStruct);

#ifdef TEST_MSVC
# pragma warning (push)
# pragma warning (disable: 4307)	// integral constant overflow
#endif

	MOMO_DATA_COLUMN_STRING_TAG(BaseStruct, int, intString);
	MOMO_DATA_COLUMN_STRING_TAG(BaseStruct, double, dblString);
	MOMO_DATA_COLUMN_STRING_TAG(BaseStruct, std::string, strString);

#ifdef TEST_MSVC
# pragma warning (pop)
#endif
}

class SimpleDataTester
{
private:
	class DataTraits1 : public momo::DataTraits
	{
	public:
		static const size_t selectEqualityMaxCount = 1;
	};

public:
	static void TestAll()
	{
		{
			std::cout << "momo::DataColumnListNative (-RowNumber): " << std::flush;
			typedef momo::DataColumnListStatic<Struct, momo::DataColumnInfoNative<Struct>,
				momo::MemManagerDict<>> DataColumnList;
			DataColumnList columnList;
			columnList.SetMutable(&Struct::dblStruct);
			columnList.PrepareForVisitors(&Struct::intStruct, &Struct::dblStruct, &Struct::strStruct);
			momo::DataTable<DataColumnList> table(std::move(columnList));
			TestData<false, true>(table, &Struct::intStruct, &Struct::dblStruct, &Struct::strStruct);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnListStatic (-RowNumber): " << std::flush;
			typedef momo::DataColumnListStatic<Struct, momo::DataColumnInfo<Struct>,
				momo::MemManagerDict<>> DataColumnList;
			DataColumnList columnList;
			columnList.SetMutable(dblStruct);
			columnList.PrepareForVisitors(intStruct, dblStruct, strStruct);
			momo::DataTable<DataColumnList> table(std::move(columnList));
			TestData<false>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnListStatic (+RowNumber): " << std::flush;
			typedef momo::DataColumnListStatic<Struct, momo::DataColumnInfo<Struct>,
				momo::MemManagerDict<>, momo::DataSettings<true>> DataColumnList;
			DataColumnList columnList;
			columnList.SetMutable(intStruct);
			columnList.ResetMutable();
			columnList.SetMutable(dblStruct);
			columnList.PrepareForVisitors(intStruct, dblStruct, strStruct);
			momo::DataTable<DataColumnList, DataTraits1> table(std::move(columnList));
			TestData<false>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}

#if !(defined(TEST_GCC) && __GNUC__ < 6)	// std::hash<enum class>
		{
			std::cout << "momo::DataColumnList (struct, -RowNumber): " << std::flush;
			typedef momo::DataColumnList<momo::DataColumnTraits<Struct, 4>,
				momo::MemManagerDict<>> DataColumnList;
			DataColumnList columnList = { dblStruct.Mutable(), intStruct };
			columnList.Add(strStruct);
			momo::DataTable<DataColumnList> table(std::move(columnList));
			TestData<true>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}
#endif

		{
			std::cout << "momo::DataColumnList (string, +RowNumber): " << std::flush;
			typedef momo::DataColumnList<momo::DataColumnTraits<BaseStruct, 12>, momo::MemManagerDict<>,
				momo::DataItemTraits<momo::MemManagerDict<>>, momo::DataSettings<true>> DataColumnList;
			DataColumnList columnList;
			columnList.Add(dblString.Mutable());
			columnList.Add(intString);
			columnList.Add(strString);
			momo::DataTable<DataColumnList, DataTraits1> table(std::move(columnList));
			TestData<true>(table, intString, dblString, strString);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnList (string, -RowNumber): " << std::flush;
			typedef momo::DataColumnList<momo::DataColumnTraits<BaseStruct>,
				momo::MemManagerDict<>> DataColumnList;
			momo::DataTable<DataColumnList> table({ intString, strString, dblString.Mutable() });
			TestData<true>(table, intString, dblString, strString);
			std::cout << "ok" << std::endl;
		}
	}

	template<bool dynamic, bool native = false,
		typename Table, typename IntCol, typename DblCol, typename StrCol>
	static void TestData(Table& table,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol)
	{
		typedef typename Table::Row Row;
		typedef typename Table::ConstRowReference ConstRowReference;
		typedef typename Table::ConstSelection ConstSelection;
		typedef typename Table::Selection Selection;
		typedef typename Table::ConstIterator ConstIterator;
		typedef typename Table::ColumnList::ColumnInfo ColumnInfo;

		static const size_t count = 1024;
		static const size_t count2 = 12;

		const Table& ctable = table;

		auto keyIndex = table.AddUniqueHashIndex(intCol, strCol);
		table.AddMultiHashIndex(strCol);

		table.Reserve(count);

		for (size_t i = 0; i < count / 2; ++i)
		{
			bool added = false;
#if defined(__cpp_if_constexpr)
			if constexpr (!native)
			{
				if (i % 2 == 1)
					added = !!table.TryAddRow(intCol = static_cast<int>(i));
			}
#endif
			if (!added)
			{
#if defined(MOMO_HAS_DEDUCTION_GUIDES)
				table.AddRow(momo::DataAssignment(intCol, static_cast<int>(i)));
#else
				table.AddRow(ColumnInfo::MakeAssignment(intCol, static_cast<int>(i)));
#endif
			}
		}

		for (size_t i = count / 2; i < count; ++i)
		{
			Row row = table.NewRow();

			auto voidVisitor = [i, &intCol] (void* item, ColumnInfo columnInfo)
			{
				if (columnInfo.GetCode() == ColumnInfo::GetCode(intCol))
					*static_cast<int*>(item) = static_cast<int>(i);
			};
			row.VisitPointers(voidVisitor);

#if !defined(MOMO_DISABLE_TYPE_INFO) && defined(__cpp_generic_lambdas) && defined(__cpp_if_constexpr)
			auto refVisitor = [i] (auto& item)
			{
				typedef std::decay_t<decltype(item)> Item;
				if constexpr (std::is_same_v<Item, int>)
					assert(item == static_cast<int>(i));
			};
			row.VisitReferences(refVisitor);
			std::as_const(row).VisitReferences(refVisitor);

			auto ptrVisitor = [i] (auto* item)
			{
				typedef decltype(item) Item;
				if constexpr (std::is_same_v<Item, int*> || std::is_same_v<Item, const int*>)
					assert(*item == static_cast<int>(i));
			};
			row.VisitPointers(ptrVisitor);
			std::as_const(row).VisitPointers(ptrVisitor);
#endif

			Row row2 = table.NewRow();
			row2 = std::move(row);
			assert(static_cast<const Row&>(row2)[intCol] == static_cast<int>(i));
			table.Add(std::move(row2));
		}

		table.AddMultiHashIndex(intCol);

		{
			typedef typename Table::template ConstItemBounds<std::string> StringBounds;
			StringBounds strings;
			typename StringBounds::Iterator begin;
			strings = ctable.GetColumnItems(strCol);
			begin = strings.GetBegin();
			assert(strings.GetCount() == count);
			assert(begin < strings.GetEnd());
			assert(strings.GetEnd() - begin == static_cast<ptrdiff_t>(count));
			assert(strings[0].empty());
		}

		for (auto row : table)
			assert(row[strCol].empty());
		for (auto row : ctable)
			assert(row[strCol].empty());

		for (size_t i = 0; i < count; ++i)
		{
			Row row = table.NewRow(table[i]);
			row[intCol] = static_cast<int>(i) / 2;
			row[strCol] = (i % 2 == 0) ? "1" : "2";
			table.Update(i, table.NewRow(row));
		}

		for (size_t i = 0; i < count; i += 2)
			table.Update(table[i], strCol, "0");

		for (size_t i = 1; i < count; i += 2)
		{
			std::string s1 = "1";
			table.Update(table[i], strCol, s1);
		}

		for (size_t i = 0; i < count; ++i)
			table[i].GetMutable(dblCol) = static_cast<double>(i) / 2.0;

		assert(table.GetUniqueHashIndex(intCol, strCol) == keyIndex);
		assert(table.GetMultiHashIndex(intCol) != momo::DataMultiHashIndex::empty);
		
		assert(table.TryAdd(table.NewRow(table[0])).uniqueHashIndex == keyIndex);

		assert(table.TryUpdate(0, table.NewRow(table[1])).uniqueHashIndex == keyIndex);
		assert(table.TryUpdate(0, table.NewRow(table[0])));

		assert(table.TryUpdate(table[0], strCol, "1").uniqueHashIndex == keyIndex);
		assert(table.TryUpdate(table[0], strCol, static_cast<const std::string&>(std::string("0"))));

		for (size_t i = 0; i < count2; ++i)
		{
			int v = static_cast<int>(count + i);
			bool added = false;
#if defined(__cpp_if_constexpr)
			if constexpr (!native)
			{
				if (i % 2 == 1)
					added = !!table.TryInsertRow(count, intCol = v);
			}
#endif
			if (!added)
			{
#if defined(MOMO_HAS_DEDUCTION_GUIDES)
				table.InsertRow(count, momo::DataAssignment(intCol, v));
#else
				table.InsertRow(count, ColumnInfo::MakeAssignment(intCol, v));
#endif
			}
		}
		assert(table.GetCount() == count + count2);

		MOMO_STATIC_ASSERT(count2 % 6 == 0);
		for (size_t i = 0; i < count2 / 6; ++i)
		{
			table.Remove(count, false);
			table.Remove(count, true);
			table.Remove(table[count]);
			table.Extract(count, false);
			table.Extract(count, true);
			table.Extract(table[count]);
		}
		assert(table.GetCount() == count);

		assert(table.SelectEmpty().IsEmpty());
		assert(ctable.SelectEmpty().IsEmpty());

		auto emptyFilter = [] (ConstRowReference) { return true; };
		auto strFilter = [&strCol] (ConstRowReference rowRef) { return rowRef[strCol] == "0"; };

		assert(table.SelectCount() == count);
		assert(table.Select().GetCount() == count);
		assert(ctable.Select().GetCount() == count);

		assert(table.SelectCount(momo::DataEquality<>().And(dblCol, 0.0)) == 1);
		assert(table.Select(momo::DataEquality<>().And(dblCol, 1.0)).GetCount() == 1);
		assert(ctable.Select(momo::DataEquality<>().And(dblCol, 1.0)).GetCount() == 1);

		assert(table.SelectCount(
			momo::DataEquality<>().And(strCol, "0").And(intCol, 1).And(dblCol, 1.0)) == 1);
		assert(table.Select(
			momo::DataEquality<>().And(intCol, 0).And(dblCol, 0.5).And(strCol, "1")).GetCount() == 1);
		assert(ctable.Select(
			momo::DataEquality<>().And(strCol, "1").And(dblCol, 0.5).And(intCol, 0)).GetCount() == 1);

		assert(table.SelectCount(emptyFilter, momo::DataEquality<>().And(strCol, "0")) == count / 2);
		assert(table.Select(emptyFilter, momo::DataEquality<>().And(strCol, "1")).GetCount() == count / 2);
		assert(ctable.Select(emptyFilter, momo::DataEquality<>().And(strCol, "1")).GetCount() == count / 2);

		assert(table.SelectCount(momo::DataEquality<>().And(dblCol, 0.0).And(strCol, "0"),
			emptyFilter) == 1);
		assert(table.Select(momo::DataEquality<>().And(strCol, "0").And(dblCol, 1.0),
			emptyFilter).GetCount() == 1);
		assert(ctable.Select(momo::DataEquality<>().And(dblCol, 1.0).And(strCol, "1"),
			emptyFilter).GetCount() == 0);

		assert(table.FindByUniqueHash(keyIndex,
			ColumnInfo::MakeEquality(intCol, 0), ColumnInfo::MakeEquality(strCol, "1"))->Get(strCol) == "1");
		assert((*ctable.FindByUniqueHash(keyIndex,
			ColumnInfo::MakeEquality(strCol, "1"), ColumnInfo::MakeEquality(intCol, 0)))[strCol] == "1");

		assert(table.FindByUniqueHash(
			momo::DataEquality<>().And(intCol, 0).And(strCol, "1"), keyIndex)->Get(strCol) == "1");
		assert((*ctable.FindByUniqueHash(
			momo::DataEquality<>().And(strCol, "1").And(intCol, 0), keyIndex))[strCol] == "1");

		assert(table.FindByMultiHash(momo::DataMultiHashIndex::empty,
			ColumnInfo::MakeEquality(strCol, "1")).GetCount() == count / 2);
		assert(ctable.FindByMultiHash(momo::DataMultiHashIndex::empty,
			ColumnInfo::MakeEquality(strCol, "1")).GetCount() == count / 2);

		assert(table.FindByMultiHash(momo::DataEquality<>().And(strCol, "1")).GetCount() == count / 2);
		assert(ctable.FindByMultiHash(momo::DataEquality<>().And(strCol, "1")).GetCount() == count / 2);

		pvTestDataNative<native>(table, intCol, dblCol, strCol);

		{
			typename Table::RowHashPointer hashPointer;
			typename Table::RowHashPointer::Iterator begin;
			hashPointer = table.FindByUniqueHash(
				momo::DataEquality<>().And(strCol, "0").And(intCol, 0), keyIndex);
			assert(static_cast<bool>(hashPointer));
			begin = hashPointer.GetBegin();
			assert(begin < hashPointer.GetEnd());
		}

		{
			typename Table::RowHashBounds hashBounds;
			typename Table::RowHashBounds::Iterator begin;
			hashBounds = table.FindByMultiHash(momo::DataEquality<>().And(strCol, "0"));
			begin = hashBounds.GetBegin();
			assert(begin < hashBounds.GetEnd());
			assert(hashBounds[0][strCol] == "0");
		}

		assert(table.MakeMutableReference(ctable[0]).GetRaw() == table[0].GetRaw());

#if !defined(MOMO_DISABLE_TYPE_INFO) && defined(__cpp_generic_lambdas)
		{
			std::stringstream sstream;
			auto visitor = [&sstream] (const auto& item, auto columnInfo)
			{
				if (columnInfo.GetTypeInfo() != typeid(double))
					sstream << item;
			};
			ctable[0].VisitReferences(visitor);
			assert(sstream.str() == "00");
		}
#endif

		{
			Table table2(table);
			assert(table2.GetCount() == count);
			assert(table2.ContainsColumn(intCol));
			assert(table2.ContainsColumn(dblCol));
			assert(table2.ContainsColumn(strCol));

			assert(table2.GetUniqueHashIndex(intCol, strCol) != momo::DataUniqueHashIndex::empty);
			assert(table2.GetMultiHashIndex(intCol) != momo::DataMultiHashIndex::empty);
			assert(table2.GetMultiHashIndex(strCol) != momo::DataMultiHashIndex::empty);

			table2.Clear();
			assert(table2.IsEmpty());

			table2.RemoveUniqueHashIndexes();
			table2.RemoveMultiHashIndexes();

			table2 = ctable;
			assert(table2.GetCount() == count);
			table2 = Table(table.Select());
			assert(table2.GetCount() == count);
			table2 = Table(ctable.Select());
			assert(table2.GetCount() == count);
		}

		{
			Selection selection = table.Select(momo::DataEquality<>().And(strCol, "1"));
			assert(selection.GetCount() == count / 2);
			for (auto row : selection)
				assert(row[strCol] == "1");

			ConstSelection cselection = selection;
			assert(cselection.GetCount() == count / 2);
			cselection = selection;
			assert(cselection.GetCount() == count / 2);

			Selection selection2(table.Select(), strFilter);
			selection = selection2;
			assert(selection.GetCount() == count / 2);
			for (const std::string& s : selection.GetColumnItems(strCol))
				assert(s == "0");

			selection.Clear();
			assert(selection.IsEmpty());
		}
		{
			Selection selection = table.Select().Sort(strCol);

			assert(selection.GetLowerBound(ColumnInfo::MakeEquality(strCol, "")) == 0);
			assert(selection.GetUpperBound(ColumnInfo::MakeEquality(strCol, "")) == 0);
			assert(selection.GetLowerBound(ColumnInfo::MakeEquality(strCol, "0")) == 0);
			assert(selection.GetUpperBound(ColumnInfo::MakeEquality(strCol, "0")) == count / 2);
			assert(selection.GetLowerBound(ColumnInfo::MakeEquality(strCol, "1")) == count / 2);
			assert(selection.GetUpperBound(ColumnInfo::MakeEquality(strCol, "1")) == count);
			assert(selection.GetLowerBound(ColumnInfo::MakeEquality(strCol, "2")) == count);
			assert(selection.GetUpperBound(ColumnInfo::MakeEquality(strCol, "2")) == count);

			selection.Reserve(selection.GetCount() * 2);
			selection.Assign(selection.GetBegin(), selection.GetEnd());
			selection.Add(selection[count - 1]);
			selection.Add(selection.GetBegin() + count / 2, selection.GetEnd());
			selection.Insert(count, selection[count]);
			selection.Insert(count / 2, selection.GetBegin(), selection.GetBegin() + count / 2);
			selection.Set(0, selection[1]);

			for (size_t i = 0; i < selection.GetCount(); ++i)
				assert(selection[i][strCol] == ((i < count) ? "0" : "1"));

			selection.Remove(0, count / 2);
			selection.Remove(count, selection.GetCount() - count);

			selection.Reverse();
			for (size_t i = 0; i < selection.GetCount(); ++i)
				assert(selection[i][strCol] == ((i < count / 2) ? "1" : "0"));

			auto rowComp = [&intCol] (ConstRowReference rowRef1, ConstRowReference rowRef2)
				{ return rowRef1[intCol] > rowRef2[intCol]; };
			selection.Sort(rowComp);

			auto rowPred = [&intCol] (ConstRowReference rowRef) { return rowRef[intCol] < 1; };
			assert(selection.BinarySearch(rowPred) == count - 2);

			selection.Group(strCol);
			for (size_t i = 1; i < count; ++i)
				assert(i == count / 2 || selection[i][strCol] == selection[i - 1][strCol]);

			assert(selection.Remove(strFilter) == count / 2);
			assert(selection.GetCount() == count / 2);
		}

		pvTestDataDynamic<dynamic>(table, intCol, dblCol, strCol);

		table.Assign(std::reverse_iterator<ConstIterator>(table.GetEnd()),
			std::reverse_iterator<ConstIterator>(momo::internal::UIntMath<>::Next(table.GetBegin(), count / 2)));
		assert(table.GetCount() == count / 2);

		assert(table.Remove(strFilter) == count / 4);
		assert(table.GetCount() == count / 4);

		table.Remove(momo::internal::UIntMath<>::Next(table.GetBegin(), count / 8), table.GetEnd());
		assert(table.GetCount() == count / 8);

		for (size_t i = 0; i < count / 8; ++i)
		{
			auto rowRef = table[i];
			assert(rowRef[intCol] == 511 - static_cast<int>(i));
			assert(rowRef[dblCol] == 511.5 - static_cast<double>(i));
			assert(rowRef[strCol] == "1");
		}
	}

private:
	template<bool native, typename Table, typename IntCol, typename DblCol, typename StrCol>
	static typename std::enable_if<native, void>::type pvTestDataNative(Table& /*table*/,
		const IntCol& /*intCol*/, const DblCol& /*dblCol*/, const StrCol& /*strCol*/)
	{
	}

	template<bool native, typename Table, typename IntCol, typename DblCol, typename StrCol>
	static typename std::enable_if<!native, void>::type pvTestDataNative(Table& table,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol)
	{
		const Table& ctable = table;

		assert(table.SelectCount(strCol == "0" && intCol == 1 && dblCol == 1.0) == 1);
		assert(table.Select(intCol == 0 && dblCol == 0.5 && strCol == "1").GetCount() == 1);
		assert(ctable.Select(strCol == "1" && dblCol == 0.5 && intCol == 0).GetCount() == 1);

		auto keyIndex = ctable.GetUniqueHashIndex(intCol, strCol);

		assert((*table.FindByUniqueHash(keyIndex, table.NewRow(strCol = "1", intCol = 0)))[intCol] == 0);
		assert(ctable.FindByUniqueHash(keyIndex, table.NewRow(intCol = 0, strCol = "1"))->Get(intCol) == 0);
	}

	template<bool dynamic, typename Table, typename IntCol, typename DblCol, typename StrCol>
	static typename std::enable_if<dynamic, void>::type pvTestDataDynamic(Table& table,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol)
	{
		typedef typename Table::ColumnList ColumnList;
		typedef typename Table::ConstRowReference ConstRowReference;

		const Table& ctable = table;

		auto strFilter = [&strCol] (ConstRowReference rowRef) { return rowRef[strCol] == "0"; };
		size_t count = ctable.GetCount();

		Table tablePrj = ctable.Project({ dblCol, intCol }, dblCol, intCol);
		assert(tablePrj.GetCount() == count);
		for (const auto& col : tablePrj.GetColumnList())
			assert(strcmp(col.GetName(), dblCol.GetName()) == 0 || strcmp(col.GetName(), intCol.GetName()) == 0);

		assert(ctable.Project({ dblCol.Mutable(), intCol }, strFilter, dblCol, intCol).GetCount() == count / 2);
		assert(ctable.ProjectDistinct({ strCol.Mutable() }, strCol).GetCount() == 2);
		assert(ctable.ProjectDistinct(ColumnList(ctable.GetColumnList()), strFilter, strCol).GetCount() == 1);
	}

	template<bool dynamic, typename Table, typename IntCol, typename DblCol, typename StrCol>
	static typename std::enable_if<!dynamic, void>::type pvTestDataDynamic(Table& table,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol)
	{
		typedef typename Table::ColumnList ColumnList;
		typedef typename Table::ConstRowReference ConstRowReference;

		const Table& ctable = table;

		auto strFilter = [&strCol] (ConstRowReference rowRef) { return rowRef[strCol] == "0"; };
		size_t count = ctable.GetCount();

		assert(ctable.Project(ColumnList(), dblCol, intCol).GetCount() == count);
		assert(ctable.Project(ColumnList(), strFilter, dblCol, intCol).GetCount() == count / 2);
		assert(ctable.ProjectDistinct(ColumnList(), strCol).GetCount() == 2);
		assert(ctable.ProjectDistinct(ColumnList(), strFilter, strCol).GetCount() == 1);

		{
#if defined(__cpp_designated_initializers)
			auto row = table.NewRow({ .intStruct = 1, .dblStruct = 3.5, .strStruct = "2" });
#else
			auto row = table.NewRow({ /*.intStruct =*/ 1, /*.dblStruct =*/ 3.5, /*.strStruct =*/ "2" });
#endif

			assert(row->intStruct == 1);
			assert(row->dblStruct == 3.5);
			assert(row->strStruct == "2");
		}
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA
