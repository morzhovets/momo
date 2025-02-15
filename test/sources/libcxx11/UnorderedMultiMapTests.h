/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/libcxx11/UnorderedMultiMapTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(contains)
#include "unord.multimap/contains.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(contains_transparent)
#include "unord.multimap/contains.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(count)
#include "unord.multimap/count.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(count_transparent)
#include "unord.multimap/count.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(db_iterators_7)
#include "unord.multimap/db_iterators_7.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_8)
#include "unord.multimap/db_iterators_8.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "unord.multimap/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq)
#include "unord.multimap/eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_const)
#include "unord.multimap/equal_range_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(equal_range_const_transparent)
#include "unord.multimap/equal_range_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(equal_range_non_const)
#include "unord.multimap/equal_range_non_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(equal_range_non_const_transparent)
#include "unord.multimap/equal_range_non_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

#if defined(TEST_MSVC) || defined(__cpp_generic_lambdas)
LIBCXX_TEST_BEGIN(erase_if_)
#include "unord.multimap/erase_if.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(find_const)
#include "unord.multimap/find_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(find_const_transparent)
#include "unord.multimap/find_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(find_non_const)
#include "unord.multimap/find_non_const.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
LIBCXX_TEST_BEGIN(find_non_const_transparent)
#include "unord.multimap/find_non_const.transparent.pass.cpp"
LIBCXX_TEST_END
#endif

//LIBCXX_TEST_BEGIN(incomplete)
//#include "unord.multimap/incomplete.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "unord.multimap/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "unord.multimap/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "unord.multimap/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_member)
#include "unord.multimap/swap_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "unord.multimap/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_allocator)
#include "unord.multimap/unord.multimap.cnstr/allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_copy)
#include "unord.multimap/unord.multimap.cnstr/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_init)
#include "unord.multimap/unord.multimap.cnstr/assign_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_move)
#include "unord.multimap/unord.multimap.cnstr/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy)
#include "unord.multimap/unord.multimap.cnstr/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy_alloc)
#include "unord.multimap/unord.multimap.cnstr/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_DEFAULT_BUCKET
#ifdef LIBCPP_TEST_DEDUCTION_GUIDES

LIBCXX_TEST_BEGIN(cnstr_deduct)
#include "unord.multimap/unord.multimap.cnstr/deduct.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_deduct_const)
#include "unord.multimap/unord.multimap.cnstr/deduct_const.pass.cpp"
LIBCXX_TEST_END

#endif
#endif

LIBCXX_TEST_BEGIN(cnstr_default)
#include "unord.multimap/unord.multimap.cnstr/default.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_default_noexcept)
//#include "unord.multimap/unord.multimap.cnstr/default_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_dtor_noexcept)
//#include "unord.multimap/unord.multimap.cnstr/dtor_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init)
#include "unord.multimap/unord.multimap.cnstr/init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size)
#include "unord.multimap/unord.multimap.cnstr/init_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash)
#include "unord.multimap/unord.multimap.cnstr/init_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal)
#include "unord.multimap/unord.multimap.cnstr/init_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal_allocator)
#include "unord.multimap/unord.multimap.cnstr/init_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move)
#include "unord.multimap/unord.multimap.cnstr/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_alloc)
#include "unord.multimap/unord.multimap.cnstr/move_alloc.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_move_assign_noexcept)
//#include "unord.multimap/unord.multimap.cnstr/move_assign_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cnstr_move_noexcept)
//#include "unord.multimap/unord.multimap.cnstr/move_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range)
#include "unord.multimap/unord.multimap.cnstr/range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size)
#include "unord.multimap/unord.multimap.cnstr/range_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash)
#include "unord.multimap/unord.multimap.cnstr/range_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash_equal)
#include "unord.multimap/unord.multimap.cnstr/range_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_range_size_hash_equal_allocator)
#include "unord.multimap/unord.multimap.cnstr/range_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size)
#include "unord.multimap/unord.multimap.cnstr/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash)
#include "unord.multimap/unord.multimap.cnstr/size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal)
#include "unord.multimap/unord.multimap.cnstr/size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal_allocator)
#include "unord.multimap/unord.multimap.cnstr/size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_db_swap_1)
#include "unord.multimap/unord.multimap.swap/db_swap_1.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(swap_swap_noexcept)
//#include "unord.multimap/unord.multimap.swap/swap_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_non_member)
#include "unord.multimap/unord.multimap.swap/swap_non_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "unord.multimap/unord.multimap.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "unord.multimap/unord.multimap.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "unord.multimap/unord.multimap.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_const_iter)
#include "unord.multimap/unord.multimap.modifiers/erase_const_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db1)
#include "unord.multimap/unord.multimap.modifiers/erase_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db2)
#include "unord.multimap/unord.multimap.modifiers/erase_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db1)
#include "unord.multimap/unord.multimap.modifiers/erase_iter_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db2)
#include "unord.multimap/unord.multimap.modifiers/erase_iter_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db3)
#include "unord.multimap/unord.multimap.modifiers/erase_iter_iter_db3.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db4)
#include "unord.multimap/unord.multimap.modifiers/erase_iter_iter_db4.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "unord.multimap/unord.multimap.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_range)
#include "unord.multimap/unord.multimap.modifiers/erase_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_const_lvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_const_lvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_rvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_hint_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_init)
#include "unord.multimap/unord.multimap.modifiers/insert_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_range)
#include "unord.multimap/unord.multimap.modifiers/insert_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_rvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_rvalue.pass.cpp"
LIBCXX_TEST_END
