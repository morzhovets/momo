/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxUnorderedMultiMapTests.h

\**********************************************************/

//LIBCXX_TEST_BEGIN(bucket)
//#include "libcxx/unord.multimap/bucket.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(bucket_count)
//#include "libcxx/unord.multimap/bucket_count.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(bucket_size)
//#include "libcxx/unord.multimap/bucket_size.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count)
#include "libcxx/unord.multimap/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_7)
#include "libcxx/unord.multimap/db_iterators_7.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_8)
#include "libcxx/unord.multimap/db_iterators_8.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(db_local_iterators_7)
//#include "libcxx/unord.multimap/db_local_iterators_7.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(db_local_iterators_8)
//#include "libcxx/unord.multimap/db_local_iterators_8.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "libcxx/unord.multimap/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq)
#include "libcxx/unord.multimap/eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_const)
#include "libcxx/unord.multimap/equal_range_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_non_const)
#include "libcxx/unord.multimap/equal_range_non_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_const)
#include "libcxx/unord.multimap/find_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_non_const)
#include "libcxx/unord.multimap/find_non_const.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(incomplete)
//#include "libcxx/unord.multimap/incomplete.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "libcxx/unord.multimap/iterators.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(load_factor)
//#include "libcxx/unord.multimap/load_factor.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(local_iterators)
//#include "libcxx/unord.multimap/local_iterators.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(max_bucket_count)
//#include "libcxx/unord.multimap/max_bucket_count.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(max_load_factor)
//#include "libcxx/unord.multimap/max_load_factor.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "libcxx/unord.multimap/max_size.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(rehash)
//#include "libcxx/unord.multimap/rehash.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(reserve)
//#include "libcxx/unord.multimap/reserve.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "libcxx/unord.multimap/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_member)
#include "libcxx/unord.multimap/swap_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "libcxx/unord.multimap/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_allocator)
#include "libcxx/unord.multimap/unord.multimap.cnstr/allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_copy)
#include "libcxx/unord.multimap/unord.multimap.cnstr/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_init)
#include "libcxx/unord.multimap/unord.multimap.cnstr/assign_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_move)
#include "libcxx/unord.multimap/unord.multimap.cnstr/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy)
#include "libcxx/unord.multimap/unord.multimap.cnstr/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy_alloc)
#include "libcxx/unord.multimap/unord.multimap.cnstr/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_TEST_DEDUCTION_GUIDES

LIBCXX_TEST_BEGIN(cnstr_deduct)
#include "libcxx/unord.multimap/unord.multimap.cnstr/deduct.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_deduct_const)
//#include "libcxx/unord.multimap/unord.multimap.cnstr/deduct_const.pass.cpp"
//LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(cnstr_default)
#include "libcxx/unord.multimap/unord.multimap.cnstr/default.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_default_noexcept)
//#include "libcxx/unord.multimap/unord.multimap.cnstr/default_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_dtor_noexcept)
//#include "libcxx/unord.multimap/unord.multimap.cnstr/dtor_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init)
#include "libcxx/unord.multimap/unord.multimap.cnstr/init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size)
#include "libcxx/unord.multimap/unord.multimap.cnstr/init_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash)
#include "libcxx/unord.multimap/unord.multimap.cnstr/init_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal)
#include "libcxx/unord.multimap/unord.multimap.cnstr/init_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal_allocator)
#include "libcxx/unord.multimap/unord.multimap.cnstr/init_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move)
#include "libcxx/unord.multimap/unord.multimap.cnstr/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_alloc)
#include "libcxx/unord.multimap/unord.multimap.cnstr/move_alloc.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_move_assign_noexcept)
//#include "libcxx/unord.multimap/unord.multimap.cnstr/move_assign_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_move_noexcept)
//#include "libcxx/unord.multimap/unord.multimap.cnstr/move_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range)
#include "libcxx/unord.multimap/unord.multimap.cnstr/range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size)
#include "libcxx/unord.multimap/unord.multimap.cnstr/range_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash)
#include "libcxx/unord.multimap/unord.multimap.cnstr/range_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash_equal)
#include "libcxx/unord.multimap/unord.multimap.cnstr/range_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash_equal_allocator)
#include "libcxx/unord.multimap/unord.multimap.cnstr/range_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size)
#include "libcxx/unord.multimap/unord.multimap.cnstr/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash)
#include "libcxx/unord.multimap/unord.multimap.cnstr/size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal)
#include "libcxx/unord.multimap/unord.multimap.cnstr/size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal_allocator)
#include "libcxx/unord.multimap/unord.multimap.cnstr/size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_db_swap_1)
#include "libcxx/unord.multimap/unord.multimap.swap/db_swap_1.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(swap_swap_noexcept)
//#include "libcxx/unord.multimap/unord.multimap.swap/swap_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_non_member)
#include "libcxx/unord.multimap/unord.multimap.swap/swap_non_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "libcxx/unord.multimap/unord.multimap.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "libcxx/unord.multimap/unord.multimap.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "libcxx/unord.multimap/unord.multimap.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_const_iter)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_const_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db1)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db2)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db1)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_iter_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db2)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_iter_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db3)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_iter_iter_db3.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db4)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_iter_iter_db4.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_range)
#include "libcxx/unord.multimap/unord.multimap.modifiers/erase_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_const_lvalue)
#include "libcxx/unord.multimap/unord.multimap.modifiers/insert_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_const_lvalue)
#include "libcxx/unord.multimap/unord.multimap.modifiers/insert_hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_rvalue)
#include "libcxx/unord.multimap/unord.multimap.modifiers/insert_hint_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_init)
#include "libcxx/unord.multimap/unord.multimap.modifiers/insert_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_range)
#include "libcxx/unord.multimap/unord.multimap.modifiers/insert_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_rvalue)
#include "libcxx/unord.multimap/unord.multimap.modifiers/insert_rvalue.pass.cpp"
LIBCXX_TEST_END
