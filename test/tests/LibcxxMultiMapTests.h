/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxMultiMapTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(empty)
#include "libcxx/multimap/empty.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(incomplete_type)
//#include "libcxx/multimap/incomplete_type.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator)
#include "libcxx/multimap/iterator.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(max_size)
//#include "libcxx/multimap/max_size.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "libcxx/multimap/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "libcxx/multimap/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_alloc)
#include "libcxx/multimap/multimap.cons/alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "libcxx/multimap/multimap.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare)
#include "libcxx/multimap/multimap.cons/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare_alloc)
#include "libcxx/multimap/multimap.cons/compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "libcxx/multimap/multimap.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "libcxx/multimap/multimap.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_assign)
#include "libcxx/multimap/multimap.cons/copy_assign.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_TEST_DEDUCTION_GUIDES

LIBCXX_TEST_BEGIN(cons_deduct)
#include "libcxx/multimap/multimap.cons/deduct.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_deduct_const)
//#include "libcxx/multimap/multimap.cons/deduct_const.pass.cpp"
//LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(cons_default)
#include "libcxx/multimap/multimap.cons/default.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_default_noexcept)
//#include "libcxx/multimap/multimap.cons/default_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_default_recursive)
//#include "libcxx/multimap/multimap.cons/default_recursive.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
//#include "libcxx/multimap/multimap.cons/dtor_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "libcxx/multimap/multimap.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare)
#include "libcxx/multimap/multimap.cons/initializer_list_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare_alloc)
#include "libcxx/multimap/multimap.cons/initializer_list_compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter)
#include "libcxx/multimap/multimap.cons/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp)
#include "libcxx/multimap/multimap.cons/iter_iter_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp_alloc)
#include "libcxx/multimap/multimap.cons/iter_iter_comp_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "libcxx/multimap/multimap.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "libcxx/multimap/multimap.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign)
#include "libcxx/multimap/multimap.cons/move_assign.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
//#include "libcxx/multimap/multimap.cons/move_assign_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_noexcept)
//#include "libcxx/multimap/multimap.cons/move_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "libcxx/multimap/multimap.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "libcxx/multimap/multimap.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "libcxx/multimap/multimap.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_VS2015_RELEASE
LIBCXX_TEST_BEGIN(modifiers_erase_iter)
#include "libcxx/multimap/multimap.modifiers/erase_iter.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "libcxx/multimap/multimap.modifiers/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "libcxx/multimap/multimap.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(modifiers_extract_iterator)
//#include "libcxx/multimap/multimap.modifiers/extract_iterator.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_key)
#include "libcxx/multimap/multimap.modifiers/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_cv)
#include "libcxx/multimap/multimap.modifiers/insert_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_initializer_list)
#include "libcxx/multimap/multimap.modifiers/insert_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_cv)
#include "libcxx/multimap/multimap.modifiers/insert_iter_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_iter)
#include "libcxx/multimap/multimap.modifiers/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_rv)
#include "libcxx/multimap/multimap.modifiers/insert_iter_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type)
#include "libcxx/multimap/multimap.modifiers/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type_hint)
#include "libcxx/multimap/multimap.modifiers/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_rv)
#include "libcxx/multimap/multimap.modifiers/insert_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_merge)
#include "libcxx/multimap/multimap.modifiers/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count)
#include "libcxx/multimap/multimap.ops/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count_transparent)
#include "libcxx/multimap/multimap.ops/count_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range)
#include "libcxx/multimap/multimap.ops/equal_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range_transparent)
#include "libcxx/multimap/multimap.ops/equal_range_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_find)
#include "libcxx/multimap/multimap.ops/find.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_lower_bound)
#include "libcxx/multimap/multimap.ops/lower_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_upper_bound)
#include "libcxx/multimap/multimap.ops/upper_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_member_swap)
#include "libcxx/multimap/multimap.special/member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_non_member_swap)
#include "libcxx/multimap/multimap.special/non_member_swap.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(special_swap_noexcept)
//#include "libcxx/set/set.special/swap_noexcept.pass.cpp"
//LIBCXX_TEST_END
