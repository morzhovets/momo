/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleDataTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_DATA

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
private:
	class DataTraits1 : public momo::DataTraits
	{
	public:
		static const size_t selectEqualerMaxCount = 1;
	};

public:
	static void TestAll()
	{
		{
			std::cout << "momo::DataColumnListStatic (-RowNumber): " << std::flush;
			typedef momo::DataColumnListStatic<Struct> DataColumnList;
			DataColumnList columnList;
			columnList.SetMutable(dblStruct);
			columnList.PrepareForVisitors(intStruct, dblStruct, strStruct);
			momo::DataTable<DataColumnList> table(std::move(columnList));
			TestData<false>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnListStatic (+RowNumber): " << std::flush;
			typedef momo::DataColumnListStatic<Struct, momo::MemManagerDefault,
				momo::DataSettings<true>> DataColumnList;
			DataColumnList columnList;
			columnList.SetMutable(intStruct);
			columnList.ResetMutable();
			columnList.SetMutable(dblStruct);
			columnList.PrepareForVisitors(intStruct, dblStruct, strStruct);
			momo::DataTable<DataColumnList, DataTraits1> table(std::move(columnList));
			TestData<false>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnList (struct, -RowNumber): " << std::flush;
			typedef momo::DataColumnList<momo::DataColumnTraits<Struct, 4>> DataColumnList;
			DataColumnList columnList = { dblStruct.Mutable(), intStruct };
			columnList.Add(strStruct);
			momo::DataTable<DataColumnList> table(std::move(columnList));
			TestData<true>(table, intStruct, dblStruct, strStruct);
			std::cout << "ok" << std::endl;
		}

		{
			std::cout << "momo::DataColumnList (string, +RowNumber): " << std::flush;
			typedef momo::DataColumnList<momo::DataColumnTraits<BaseStruct, 12>, momo::MemManagerDefault,
				momo::DataItemTraits<momo::MemManagerDefault>, momo::DataSettings<true>> DataColumnList;
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
			typedef momo::DataColumnList<momo::DataColumnTraits<BaseStruct>> DataColumnList;
			momo::DataTable<DataColumnList> table({ intString, strString, dblString.Mutable() });
			TestData<true>(table, intString, dblString, strString);
			std::cout << "ok" << std::endl;
		}
	}

	template<bool dynamic, typename Table, typename IntCol, typename DblCol, typename StrCol>
	static void TestData(Table& table,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol)
	{
		typedef typename Table::Row Row;
		typedef typename Table::ConstRowReference ConstRowReference;
		typedef typename Table::ConstSelection ConstSelection;
		typedef typename Table::Selection Selection;
		typedef typename Table::ConstIterator ConstIterator;

		static const size_t count = 1024;
		static const size_t count2 = 12;

		const Table& ctable = table;

		auto keyIndex = table.AddUniqueHashIndex(intCol, strCol);
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
			Row row = table.NewRow();

			auto visitor = [i] (auto& item)
			{
				if constexpr (std::is_same_v<decltype(item), int&>)
					item = static_cast<int>(i);
			};
			row.VisitReferences(visitor);

			auto visitor2 = [i] (const auto& item)
			{
				typedef std::decay_t<decltype(item)> Item;
				if constexpr (std::is_same_v<Item, int>)
					assert(item == static_cast<int>(i));
				if constexpr (std::is_same_v<Item, const int*>)
					assert(*item == static_cast<int>(i));
			};
			std::as_const(row).VisitReferences(visitor2);
			std::as_const(row).VisitPointers(visitor2);

			Row row2 = table.NewRow();
			row2 = std::move(row);
			assert(std::as_const(row2)[intCol] == static_cast<int>(i));
			table.AddRow(std::move(row2));
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
			table.UpdateRow(i, table.NewRow(row));
		}

		for (size_t i = 0; i < count; i += 2)
			table.UpdateRow(table[i], strCol, std::string("0"));

		for (size_t i = 1; i < count; i += 2)
		{
			std::string s1 = "1";
			table.UpdateRow(table[i], strCol, s1);
		}

		for (size_t i = 0; i < count; ++i)
			table[i].GetMutable(dblCol) = static_cast<double>(i) / 2.0;

		assert(table.GetUniqueHashIndex(intCol, strCol) == keyIndex);
		assert(table.GetMultiHashIndex(intCol) != momo::DataMultiHashIndex::empty);
		
		assert(table.TryAddRow(table.NewRow(table[0])).uniqueHashIndex == keyIndex);

		assert(table.TryUpdateRow(0, table.NewRow(table[1])).uniqueHashIndex == keyIndex);
		assert(table.TryUpdateRow(0, table.NewRow(table[0])).uniqueHashIndex
			== momo::DataUniqueHashIndex::empty);

		assert(table.TryUpdateRow(table[0], strCol, std::string("1")).uniqueHashIndex == keyIndex);
		assert(table.TryUpdateRow(table[0], strCol,
			static_cast<const std::string&>(std::string("0"))).uniqueHashIndex == momo::DataUniqueHashIndex::empty);

		for (size_t i = 0; i < count2; ++i)
		{
			if (i % 2 == 0)
				table.InsertRow(count, intCol = static_cast<int>(count + i));
			else
				table.TryInsertRow(count, intCol = static_cast<int>(count + i));
		}
		assert(table.GetCount() == count + count2);

		static_assert(count2 % 6 == 0);
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
		assert(table.Select(intCol == 0, strCol == "1").GetCount() == 1);
		assert(ctable.Select(strCol == "1", intCol == 0).GetCount() == 1);

		assert(table.SelectCount(emptyFilter, strCol == "0") == count / 2);
		assert(table.Select(emptyFilter, strCol == "1").GetCount() == count / 2);
		assert(ctable.Select(emptyFilter, strCol == "1").GetCount() == count / 2);

		assert(table.SelectCount(dblCol == 0.0) == 1);
		assert(table.Select(dblCol == 1.0).GetCount() == 1);
		assert(ctable.Select(dblCol == 1.0).GetCount() == 1);

		assert(table.SelectCount(dblCol == 0.0, strCol == "0") == 1);
		assert(table.Select(strCol == "0", dblCol == 1.0).GetCount() == 1);
		assert(ctable.Select(dblCol == 1.0, strCol == "1").GetCount() == 0);

		assert((*table.FindByUniqueHash(keyIndex, table.NewRow(strCol = "1", intCol = 0)))[intCol] == 0);
		assert(ctable.FindByUniqueHash(keyIndex, table.NewRow(intCol = 0, strCol = "1"))->Get(intCol) == 0);

		assert(table.FindByUniqueHash(keyIndex, intCol == 0, strCol == "1")->Get(strCol) == "1");
		assert((*ctable.FindByUniqueHash(keyIndex, strCol == "1", intCol == 0))[strCol] == "1");

		assert(table.FindByMultiHash(momo::DataMultiHashIndex::empty,
			strCol == "1").GetCount() == count / 2);
		assert(ctable.FindByMultiHash(momo::DataMultiHashIndex::empty,
			strCol == "1").GetCount() == count / 2);

		{
			typename Table::RowHashPointer hashPointer;
			typename Table::RowHashPointer::Iterator begin;
			hashPointer = table.FindByUniqueHash(keyIndex, strCol == "0", intCol == 0);
			assert(static_cast<bool>(hashPointer));
			begin = hashPointer.GetBegin();
			assert(begin < hashPointer.GetEnd());
		}

		{
			typename Table::RowHashBounds hashBounds;
			typename Table::RowHashBounds::Iterator begin;
			hashBounds = table.FindByMultiHash(momo::DataMultiHashIndex::empty, strCol == "0");
			begin = hashBounds.GetBegin();
			assert(begin < hashBounds.GetEnd());
			assert(hashBounds[0][strCol] == "0");
		}

		assert(table.MakeMutableReference(ctable[0]).GetRaw() == table[0].GetRaw());

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
			Selection selection = table.Select(strCol == "1");
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

			assert(selection.GetLowerBound(strCol == "") == 0);
			assert(selection.GetUpperBound(strCol == "") == 0);
			assert(selection.GetLowerBound(strCol == "0") == 0);
			assert(selection.GetUpperBound(strCol == "0") == count / 2);
			assert(selection.GetLowerBound(strCol == "1") == count / 2);
			assert(selection.GetUpperBound(strCol == "1") == count);
			assert(selection.GetLowerBound(strCol == "2") == count);
			assert(selection.GetUpperBound(strCol == "2") == count);

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

		if constexpr (dynamic)
		{
			Table tablePrj = ctable.Project(dblCol, intCol);
			assert(tablePrj.GetCount() == count);

			for (const auto& col : tablePrj.GetColumnList())
			{
				assert(strcmp(col.GetName(), dblCol.GetName()) == 0
					|| strcmp(col.GetName(), intCol.GetName()) == 0);
			}

			auto row = table.NewRow(tablePrj[0]);
			assert(row[dblCol] == tablePrj[0][dblCol]);
			assert(row[intCol] == tablePrj[0][intCol]);

			assert(ctable.Project(strFilter, dblCol.Mutable(), intCol).GetCount() == count / 2);
			assert(ctable.ProjectDistinct(strCol.Mutable()).GetCount() == 2);
			assert(ctable.ProjectDistinct(strFilter, strCol).GetCount() == 1);
		}

		table.AssignRows(std::reverse_iterator<ConstIterator>(table.GetEnd()),
			std::reverse_iterator<ConstIterator>(momo::internal::UIntMath<>::Next(table.GetBegin(), count / 2)));
		assert(table.GetCount() == count / 2);

		assert(table.RemoveRows(strFilter) == count / 4);
		assert(table.GetCount() == count / 4);

		table.RemoveRows(momo::internal::UIntMath<>::Next(table.GetBegin(), count / 8), table.GetEnd());
		assert(table.GetCount() == count / 8);

		for (size_t i = 0; i < count / 8; ++i)
		{
			auto rowRef = table[i];
			assert(rowRef[intCol] == 511 - static_cast<int>(i));
			assert(rowRef[dblCol] == 511.5 - static_cast<double>(i));
			assert(rowRef[strCol] == "1");
		}
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA
