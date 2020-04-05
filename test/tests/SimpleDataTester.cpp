/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleDataTester.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_DATA

#undef NDEBUG

#include "../../momo/DataTable.h"

#include <string>
#include <iostream>
#include <sstream>

namespace
{
	typedef momo::DataStructDefault<int, double, std::string> BaseStruct;

	struct Struct : public BaseStruct
	{
		int intStruct;
		double dblStruct;
		std::string strStruct;
	};

	MOMO_DATA_COLUMN_STRUCT(Struct, intStruct);
	MOMO_DATA_COLUMN_STRUCT(Struct, dblStruct);
	MOMO_DATA_COLUMN_STRUCT(Struct, strStruct);

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning (push)
#pragma warning (disable: 4307)	// integral constant overflow
#endif

	MOMO_DATA_COLUMN_STRING_TAG(BaseStruct, int, intString);
	MOMO_DATA_COLUMN_STRING_TAG(BaseStruct, double, dblString);
	MOMO_DATA_COLUMN_STRING_TAG(BaseStruct, std::string, strString);

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning (pop)
#endif
}

class SimpleDataTester
{
public:
	static void TestAll()
	{
		{
			std::cout << "momo::DataColumnListStatic: " << std::flush;
			typedef momo::DataColumnListStatic<Struct> DataColumnList;
			DataColumnList columnList;
			columnList.SetMutable(intStruct);
			columnList.ResetMutable();
			columnList.SetMutable(dblStruct);
			columnList.PrepareForVisitors(intStruct, dblStruct, strStruct);
			momo::DataTable<DataColumnList> table(std::move(columnList));
			TestData<false>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnList (struct): " << std::flush;
			typedef momo::DataColumnList<momo::DataColumnTraits<Struct>> DataColumnList;
			DataColumnList columnList;
			columnList.Add(strStruct);
			columnList.Add(dblStruct.Mutable(), intStruct);
			momo::DataTable<DataColumnList> table(std::move(columnList));
			TestData<true>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnList (string): " << std::flush;
			typedef momo::DataColumnList<momo::DataColumnTraits<BaseStruct>> DataColumnList;
			momo::DataTable<DataColumnList> table(intString, strString, dblString.Mutable());
			TestData<true>(table, intString, dblString, strString);
			std::cout << "ok" << std::endl;
		}
	}

	template<bool dynamic, typename DataTable, typename IntCol, typename DblCol, typename StrCol>
	static void TestData(DataTable& table,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol)
	{
		typedef typename DataTable::Row DataRow;
		typedef typename DataTable::ConstRowReference ConstRowReference;

		static const size_t count = 1024;
		static const size_t count2 = 12;

		const DataTable& ctable = table;

		table.AddUniqueHashIndex(intCol, strCol);
		table.AddMultiHashIndex(strCol);

		table.Reserve(count);

		for (size_t i = 0; i < count / 2; ++i)
		{
			if (i % 2 == 0)
				table.AddRow(intCol = static_cast<int>(i));
			else
				table.TryAddRow(intCol = static_cast<int>(i));
		}

		for (size_t i = count / 2; i < count; ++i)
		{
			DataRow row = table.NewRow();
#if defined(__cpp_generic_lambdas) && defined(__cpp_if_constexpr)
			auto visitor = [i] (auto& item)
			{
				if constexpr (std::is_same<decltype(item), int&>::value)
					item = static_cast<int>(i);
			};
			row.VisitReferences(visitor);
#else
			row[intCol] = static_cast<int>(i);
#endif
			table.AddRow(std::move(row));
		}

		table.AddMultiHashIndex(intCol);

		for (const std::string& s : ctable.GetColumnItems(strCol))
			assert(s.empty());

		for (size_t i = 0; i < count; ++i)
		{
			DataRow row = table.NewRow(table[i]);
			row[intCol] = static_cast<int>(i) / 2;
			row[strCol] = (i % 2 == 0) ? "1" : "2";
			table.UpdateRow(i, table.NewRow(row));
		}

		for (size_t i = 0; i < count; ++i)
			table.UpdateRow(table[i], strCol, std::string((i % 2 == 0) ? "0" : "1"));

		for (size_t i = 0; i < count; ++i)
			table[i].GetMutable(dblCol) = static_cast<double>(i) / 2.0;

		for (auto row : table) { (void)row; }
		for (auto row : ctable) { (void)row; }
		for (auto row : table.Select()) { (void)row; }
		for (auto row : ctable.Select()) { (void)row; }

		assert(table.GetUniqueHashIndex(intCol, strCol) != momo::DataUniqueHashIndex::empty);
		assert(table.GetMultiHashIndex(intCol) != momo::DataMultiHashIndex::empty);

		for (size_t i = 0; i < count2; ++i)
		{
			if (i % 2 == 0)
				table.InsertRow(count, intCol = static_cast<int>(count + i));
			else
				table.TryInsertRow(count, intCol = static_cast<int>(count + i));
		}
		assert(table.GetCount() == count + count2);

		MOMO_STATIC_ASSERT(count2 % 6 == 0);
		for (size_t i = 0; i < count2 / 6; ++i)
		{
			table.RemoveRow(count, false);
			table.RemoveRow(count, true);
			table.RemoveRow(table[count]);
			table.ExtractRow(count, false);
			table.ExtractRow(count, true);
			table.ExtractRow(table[count]);
		}
		assert(table.GetCount() == count);

		assert(table.SelectEmpty().IsEmpty());
		assert(ctable.SelectEmpty().IsEmpty());

		auto emptyFilter = [] (ConstRowReference) { return true; };
		auto strFilter = [&strCol] (ConstRowReference rowRef) { return rowRef[strCol] == "0"; };

		assert(table.SelectCount() == count);
		assert(table.Select().GetCount() == count);
		assert(ctable.Select().GetCount() == count);

		assert(table.SelectCount(strCol == "0", intCol == 1) == 1);
		assert(table.Select(strCol == "1", intCol == 0).GetCount() == 1);
		assert(ctable.Select(strCol == "1", intCol == 0).GetCount() == 1);

		assert(table.SelectCount(emptyFilter, strCol == "0") == count / 2);
		assert(table.Select(emptyFilter, strCol == "1").GetCount() == count / 2);
		assert(ctable.Select(emptyFilter, strCol == "1").GetCount() == count / 2);

#if defined(__cpp_generic_lambdas)
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

		auto cselection = ctable.Select(strCol == "1");
		cselection.Set(0, cselection[1]);
		for (const std::string& s : cselection.GetColumnItems(strCol))
			assert(s == "1");

		cselection = table.Select().Sort(strCol);

		assert(cselection.GetLowerBound(strCol == "") == 0);
		assert(cselection.GetUpperBound(strCol == "") == 0);
		assert(cselection.GetLowerBound(strCol == "0") == 0);
		assert(cselection.GetUpperBound(strCol == "0") == count / 2);
		assert(cselection.GetLowerBound(strCol == "1") == count / 2);
		assert(cselection.GetUpperBound(strCol == "1") == count);
		assert(cselection.GetLowerBound(strCol == "2") == count);
		assert(cselection.GetUpperBound(strCol == "2") == count);

		cselection.Assign(cselection.GetBegin(), cselection.GetEnd());
		cselection.Add(cselection[count - 1]);
		cselection.Add(cselection.GetBegin() + count / 2, cselection.GetEnd());
		cselection.Insert(count, cselection[count]);
		cselection.Insert(count / 2, cselection.GetBegin(), cselection.GetBegin() + count / 2);

		for (size_t i = 0; i < cselection.GetCount(); ++i)
			assert(cselection[i][strCol] == ((i < count) ? "0" : "1"));

		cselection.Remove(0, count / 2);
		cselection.Remove(count, cselection.GetCount() - count);

		cselection.Reverse();

		cselection.Group(intCol);
		for (size_t i = 0; i < count; i += 2)
			assert(cselection[i][intCol] == cselection[i + 1][intCol]);

		cselection.Filter(strFilter);
		assert(cselection.GetCount() == count / 2);
		cselection.Remove(strFilter);
		assert(cselection.IsEmpty());

		assert(table.SelectCount(dblCol == 0.0) == 1);
		assert(table.Select(dblCol == 1.0).GetCount() == 1);
		assert(ctable.Select(dblCol == 1.0).GetCount() == 1);

		assert((*table.FindByUniqueHash(table.GetUniqueHashIndex(intCol, strCol),
			table.NewRow(strCol = "1", intCol = 0)))[intCol] == 0);
		assert(ctable.FindByUniqueHash(ctable.GetUniqueHashIndex(intCol, strCol),
			table.NewRow(strCol = "1", intCol = 0)).GetCount() == 1);

		assert(static_cast<bool>(table.FindByUniqueHash(momo::DataUniqueHashIndex::empty,
			strCol == "1", intCol == 0)));
		assert((*table.FindByUniqueHash(momo::DataUniqueHashIndex::empty,
			strCol == "1", intCol == 0))[strCol] == "1");
		assert((*ctable.FindByUniqueHash(momo::DataUniqueHashIndex::empty,
			strCol == "1", intCol == 0))[strCol] == "1");

		assert(table.FindByMultiHash(momo::DataMultiHashIndex::empty,
			strCol == "1").GetCount() == count / 2);
		assert(ctable.FindByMultiHash(momo::DataMultiHashIndex::empty,
			strCol == "1").GetCount() == count / 2);

		assert(table.MakeMutableReference(ctable[0]).GetNumber() == 0);

		pvTestDataDynamic<dynamic>(ctable, intCol, dblCol, strCol);

		DataTable tableCopy(table);
		assert(tableCopy.GetCount() == count);
		assert(tableCopy.ContainsColumn(intCol));
		assert(tableCopy.ContainsColumn(dblCol));
		assert(tableCopy.ContainsColumn(strCol));

		assert(tableCopy.GetUniqueHashIndex(intCol, strCol) != momo::DataUniqueHashIndex::empty);
		assert(tableCopy.GetMultiHashIndex(intCol) != momo::DataMultiHashIndex::empty);
		assert(tableCopy.GetMultiHashIndex(strCol) != momo::DataMultiHashIndex::empty);

		tableCopy.RemoveUniqueHashIndexes();
		tableCopy.RemoveMultiHashIndexes();

		tableCopy = ctable;
		assert(tableCopy.GetCount() == count);
		tableCopy = DataTable(table.Select());
		assert(tableCopy.GetCount() == count);
		tableCopy = DataTable(ctable.Select());
		assert(tableCopy.GetCount() == count);

		table.AssignRows(table.GetBegin(), table.GetEnd());
		assert(table.GetCount() == count);
		table.FilterRows(emptyFilter);
		assert(table.GetCount() == count);

		table.RemoveRows(table.GetBegin() + count / 2, table.GetEnd());
		assert(table.GetCount() == count / 2);
		table.RemoveRows(emptyFilter);
		assert(table.IsEmpty());

		table.Clear();
	}

private:
	template<bool dynamic, typename DataTable, typename IntCol, typename DblCol, typename StrCol>
	static void pvTestDataDynamic(const DataTable& ctable,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol,
		typename std::enable_if<dynamic, int>::type = 0)
	{
		typedef typename DataTable::ConstRowReference ConstRowReference;

		auto strFilter = [&strCol] (ConstRowReference rowRef) { return rowRef[strCol] == "0"; };
		size_t count = ctable.GetCount();

		DataTable tablePrj = ctable.Project(dblCol, intCol);
		for (const auto& col : tablePrj.GetColumnList())
			assert(strcmp(col.GetName(), dblCol.GetName()) == 0 || strcmp(col.GetName(), intCol.GetName()) == 0);

		assert(ctable.Project(strFilter, dblCol.Mutable(), intCol).GetCount() == count / 2);
		assert(ctable.ProjectDistinct(strCol.Mutable()).GetCount() == 2);
		assert(ctable.ProjectDistinct(strFilter, strCol).GetCount() == 1);
	}

	template<bool dynamic, typename DataTable, typename IntCol, typename DblCol, typename StrCol>
	static void pvTestDataDynamic(const DataTable& /*ctable*/,
		const IntCol& /*intCol*/, const DblCol& /*dblCol*/, const StrCol& /*strCol*/,
		typename std::enable_if<!dynamic, int>::type = 0)
	{
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA
