/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxUnorderedSetTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(bucket)
#include "libcxx/unord.set/bucket.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(bucket_count)
#include "libcxx/unord.set/bucket_count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(bucket_size)
#include "libcxx/unord.set/bucket_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(clear)
#include "libcxx/unord.set/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains)
#include "libcxx/unord.set/contains.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(contains_transparent)
#include "libcxx/unord.set/contains.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(count)
#include "libcxx/unord.set/count.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(count_transparent)
#include "libcxx/unord.set/count.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(db_iterators_7)
#include "libcxx/unord.set/db_iterators_7.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_8)
#include "libcxx/unord.set/db_iterators_8.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_local_iterators_7)
#include "libcxx/unord.set/db_local_iterators_7.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_local_iterators_8)
#include "libcxx/unord.set/db_local_iterators_8.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace)
#include "libcxx/unord.set/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace_hint)
#include "libcxx/unord.set/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "libcxx/unord.set/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq)
#include "libcxx/unord.set/eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_const)
#include "libcxx/unord.set/equal_range_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(equal_range_const_transparent)
#include "libcxx/unord.set/equal_range_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(equal_range_non_const)
#include "libcxx/unord.set/equal_range_non_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(equal_range_non_const_transparent)
#include "libcxx/unord.set/equal_range_non_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(erase_const_iter)
#include "libcxx/unord.set/erase_const_iter.pass.cpp"
LIBCXX_TEST_END

#if defined(TEST_MSVC) || defined(__cpp_generic_lambdas)
LIBCXX_TEST_BEGIN(erase_if_)
#include "libcxx/unord.set/erase_if.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(erase_iter_db1)
#include "libcxx/unord.set/erase_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_db2)
#include "libcxx/unord.set/erase_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_iter_db1)
#include "libcxx/unord.set/erase_iter_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_iter_db2)
#include "libcxx/unord.set/erase_iter_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_iter_db3)
#include "libcxx/unord.set/erase_iter_iter_db3.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_iter_db4)
#include "libcxx/unord.set/erase_iter_iter_db4.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_key)
#include "libcxx/unord.set/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_range)
#include "libcxx/unord.set/erase_range.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(extract_iterator)
//#include "libcxx/unord.set/extract_iterator.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extract_key)
#include "libcxx/unord.set/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_const)
#include "libcxx/unord.set/find_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(find_const_transparent)
#include "libcxx/unord.set/find_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(find_non_const)
#include "libcxx/unord.set/find_non_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(find_non_const_transparent)
#include "libcxx/unord.set/find_non_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

//LIBCXX_TEST_BEGIN(incomplete)
//#include "libcxx/unord.set/incomplete.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_const_lvalue)
#include "libcxx/unord.set/insert_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_hint_const_lvalue)
#include "libcxx/unord.set/insert_hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_hint_rvalue)
#include "libcxx/unord.set/insert_hint_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_init)
#include "libcxx/unord.set/insert_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type)
#include "libcxx/unord.set/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type_hint)
#include "libcxx/unord.set/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_range)
#include "libcxx/unord.set/insert_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_rvalue)
#include "libcxx/unord.set/insert_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "libcxx/unord.set/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(load_factor)
#include "libcxx/unord.set/load_factor.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(local_iterators)
#include "libcxx/unord.set/local_iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_bucket_count)
#include "libcxx/unord.set/max_bucket_count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_load_factor)
#include "libcxx/unord.set/max_load_factor.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "libcxx/unord.set/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(merge)
#include "libcxx/unord.set/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(rehash)
#include "libcxx/unord.set/rehash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(reserve)
#include "libcxx/unord.set/reserve.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "libcxx/unord.set/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_member)
#include "libcxx/unord.set/swap_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "libcxx/unord.set/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_allocator)
#include "libcxx/unord.set/unord.set.cnstr/allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_copy)
#include "libcxx/unord.set/unord.set.cnstr/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_init)
#include "libcxx/unord.set/unord.set.cnstr/assign_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_move)
#include "libcxx/unord.set/unord.set.cnstr/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy)
#include "libcxx/unord.set/unord.set.cnstr/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy_alloc)
#include "libcxx/unord.set/unord.set.cnstr/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_DEFAULT_BUCKET
#ifdef LIBCPP_TEST_DEDUCTION_GUIDES
LIBCXX_TEST_BEGIN(cnstr_deduct)
#include "libcxx/unord.set/unord.set.cnstr/deduct.pass.cpp"
LIBCXX_TEST_END
#endif
#endif

LIBCXX_TEST_BEGIN(cnstr_default)
#include "libcxx/unord.set/unord.set.cnstr/default.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_default_noexcept)
//#include "libcxx/unord.set/unord.set.cnstr/default_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_dtor_noexcept)
//#include "libcxx/unord.set/unord.set.cnstr/dtor_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init)
#include "libcxx/unord.set/unord.set.cnstr/init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size)
#include "libcxx/unord.set/unord.set.cnstr/init_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash)
#include "libcxx/unord.set/unord.set.cnstr/init_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal)
#include "libcxx/unord.set/unord.set.cnstr/init_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal_allocator)
#include "libcxx/unord.set/unord.set.cnstr/init_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move)
#include "libcxx/unord.set/unord.set.cnstr/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_alloc)
#include "libcxx/unord.set/unord.set.cnstr/move_alloc.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_move_assign_noexcept)
//#include "libcxx/unord.set/unord.set.cnstr/move_assign_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_move_noexcept)
//#include "libcxx/unord.set/unord.set.cnstr/move_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range)
#include "libcxx/unord.set/unord.set.cnstr/range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size)
#include "libcxx/unord.set/unord.set.cnstr/range_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash)
#include "libcxx/unord.set/unord.set.cnstr/range_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash_equal)
#include "libcxx/unord.set/unord.set.cnstr/range_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash_equal_allocator)
#include "libcxx/unord.set/unord.set.cnstr/range_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size)
#include "libcxx/unord.set/unord.set.cnstr/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash)
#include "libcxx/unord.set/unord.set.cnstr/size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal)
#include "libcxx/unord.set/unord.set.cnstr/size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal_allocator)
#include "libcxx/unord.set/unord.set.cnstr/size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_db_swap_1)
#include "libcxx/unord.set/unord.set.swap/db_swap_1.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(swap_swap_noexcept)
//#include "libcxx/unord.set/unord.set.swap/swap_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_non_member)
#include "libcxx/unord.set/unord.set.swap/swap_non_member.pass.cpp"
LIBCXX_TEST_END
