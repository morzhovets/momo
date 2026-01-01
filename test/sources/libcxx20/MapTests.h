/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx20/MapTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(compare)
#include "map/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(get_allocator)
#include "map/get_allocator.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(incomplete_type)
#include "map/incomplete_type.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "map/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "map/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "map/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_iterator_types)
#include "map/common/iterator_types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extra_emplace_ext)
#include "map/extra/emplace_ext.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_at)
#include "map/map.access/at.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_empty)
#include "map/map.access/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_index_key)
#include "map/map.access/index_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_index_rv_key)
#include "map/map.access/index_rv_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_iterator)
#include "map/map.access/iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_max_size)
#include "map/map.access/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(access_size)
#include "map/map.access/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_alloc)
#include "map/map.cons/alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "map/map.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare)
#include "map/map.cons/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare_alloc)
#include "map/map.cons/compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "map/map.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "map/map.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_assign)
#include "map/map.cons/copy_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_deduct)
#include "map/map.cons/deduct.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_deduct_const)
#include "map/map.cons/deduct_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default)
#include "map/map.cons/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default_noexcept)
#include "map/map.cons/default_noexcept.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(cons_default_recursive)
#include "map/map.cons/default_recursive.compile.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
#include "map/map.cons/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(cons_from_range)
#include "map/map.cons/from_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "map/map.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare)
#include "map/map.cons/initializer_list_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare_alloc)
#include "map/map.cons/initializer_list_compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter)
#include "map/map.cons/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp)
#include "map/map.cons/iter_iter_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp_alloc)
#include "map/map.cons/iter_iter_comp_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "map/map.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "map/map.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign)
#include "map/map.cons/move_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
#include "map/map.cons/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_noexcept)
#include "map/map.cons/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "map/map.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "map/map.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "map/map.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "map/map.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter)
#include "map/map.modifiers/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "map/map.modifiers/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "map/map.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_iterator)
#include "map/map.modifiers/extract_iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_key)
#include "map/map.modifiers/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_and_emplace_allocator_requirements)
#include "map/map.modifiers/insert_and_emplace_allocator_requirements.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_cv)
#include "map/map.modifiers/insert_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_initializer_list)
#include "map/map.modifiers/insert_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_cv)
#include "map/map.modifiers/insert_iter_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_iter)
#include "map/map.modifiers/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_rv)
#include "map/map.modifiers/insert_iter_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type)
#include "map/map.modifiers/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type_hint)
#include "map/map.modifiers/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_or_assign)
#include "map/map.modifiers/insert_or_assign.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(modifiers_insert_range)
#include "map/map.modifiers/insert_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_insert_rv)
#include "map/map.modifiers/insert_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_merge)
#include "map/map.modifiers/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_try_emplace)
#include "map/map.modifiers/try.emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_compare_three_way)
#include "map/map.nonmember/compare.three_way.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_op_compare)
#include "map/map.nonmember/op_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(observers_key_comp)
#include "map/map.observers/key_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(observers_value_comp)
#include "map/map.observers/value_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_contains)
#include "map/map.ops/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_contains_transparent)
#include "map/map.ops/contains_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count)
#include "map/map.ops/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count_transparent)
#include "map/map.ops/count_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count0)
#include "map/map.ops/count0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range)
#include "map/map.ops/equal_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range_transparent)
#include "map/map.ops/equal_range_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range0)
#include "map/map.ops/equal_range0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_find)
#include "map/map.ops/find.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_find0)
#include "map/map.ops/find0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_lower_bound)
#include "map/map.ops/lower_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_lower_bound0)
#include "map/map.ops/lower_bound0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_upper_bound)
#include "map/map.ops/upper_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_upper_bound0)
#include "map/map.ops/upper_bound0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_member_swap)
#include "map/map.special/member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_non_member_swap)
#include "map/map.special/non_member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap_noexcept)
#include "map/map.special/swap_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(value_compare_invoke)
#include "map/map.value_compare/invoke.pass.cpp"
LIBCXX_TEST_END
