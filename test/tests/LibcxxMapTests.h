/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxMapTests.h

\**********************************************************/

//LIBCXX_TEST_BEGIN(incomplete_type)
//#include "libcxx/map/incomplete_type.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "libcxx/map/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_at)
#include "libcxx/map/map.access/at.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_empty)
#include "libcxx/map/map.access/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_index_key)
#include "libcxx/map/map.access/index_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_index_rv_key)
#include "libcxx/map/map.access/index_rv_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_iterator)
#include "libcxx/map/map.access/iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_max_size)
#include "libcxx/map/map.access/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_size)
#include "libcxx/map/map.access/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_alloc)
#include "libcxx/map/map.cons/alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "libcxx/map/map.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare)
#include "libcxx/map/map.cons/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare_alloc)
#include "libcxx/map/map.cons/compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "libcxx/map/map.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "libcxx/map/map.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_assign)
#include "libcxx/map/map.cons/copy_assign.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_TEST_DEDUCTION_GUIDES

LIBCXX_TEST_BEGIN(cons_deduct)
#include "libcxx/map/map.cons/deduct.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_deduct_const)
//#include "libcxx/map/map.cons/deduct_const.pass.cpp"
//LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(cons_default)
#include "libcxx/map/map.cons/default.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_default_noexcept)
//#include "libcxx/map/map.cons/default_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_default_recursive)
//#include "libcxx/map/map.cons/default_recursive.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
//#include "libcxx/map/map.cons/dtor_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "libcxx/map/map.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare)
#include "libcxx/map/map.cons/initializer_list_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare_alloc)
#include "libcxx/map/map.cons/initializer_list_compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter)
#include "libcxx/map/map.cons/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp)
#include "libcxx/map/map.cons/iter_iter_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp_alloc)
#include "libcxx/map/map.cons/iter_iter_comp_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "libcxx/map/map.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "libcxx/map/map.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign)
#include "libcxx/map/map.cons/move_assign.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
//#include "libcxx/map/map.cons/move_assign_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_noexcept)
//#include "libcxx/map/map.cons/move_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "libcxx/map/map.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "libcxx/map/map.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "libcxx/map/map.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_VS2015_RELEASE
LIBCXX_TEST_BEGIN(modifiers_erase_iter)
#include "libcxx/map/map.modifiers/erase_iter.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "libcxx/map/map.modifiers/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCPP_VS2015_RELEASE
LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "libcxx/map/map.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END
#endif

//LIBCXX_TEST_BEGIN(modifiers_extract_iterator)
//#include "libcxx/map/map.modifiers/extract_iterator.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_key)
#include "libcxx/map/map.modifiers/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_cv)
#include "libcxx/map/map.modifiers/insert_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_initializer_list)
#include "libcxx/map/map.modifiers/insert_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_cv)
#include "libcxx/map/map.modifiers/insert_iter_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_iter)
#include "libcxx/map/map.modifiers/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_rv)
#include "libcxx/map/map.modifiers/insert_iter_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type)
#include "libcxx/map/map.modifiers/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type_hint)
#include "libcxx/map/map.modifiers/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_or_assign)
#include "libcxx/map/map.modifiers/insert_or_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_rv)
#include "libcxx/map/map.modifiers/insert_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_merge)
#include "libcxx/map/map.modifiers/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_try_emplace)
#include "libcxx/map/map.modifiers/try.emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count)
#include "libcxx/map/map.ops/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count_transparent)
#include "libcxx/map/map.ops/count_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range)
#include "libcxx/map/map.ops/equal_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range_transparent)
#include "libcxx/map/map.ops/equal_range_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_find)
#include "libcxx/map/map.ops/find.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_lower_bound)
#include "libcxx/map/map.ops/lower_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_upper_bound)
#include "libcxx/map/map.ops/upper_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_member_swap)
#include "libcxx/map/map.special/member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_non_member_swap)
#include "libcxx/map/map.special/non_member_swap.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(special_swap_noexcept)
//#include "libcxx/set/set.special/swap_noexcept.pass.cpp"
//LIBCXX_TEST_END
