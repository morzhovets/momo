/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx/MultiMapTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(empty)
#include "multimap/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(get_allocator)
#include "multimap/get_allocator.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(incomplete_type)
#include "multimap/incomplete_type.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(iterator)
#include "multimap/iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "multimap/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_types)
#include "multimap/iterator_types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "multimap/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "multimap/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(scary)
#include "multimap/scary.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "multimap/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "multimap/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_alloc)
#include "multimap/multimap.cons/alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "multimap/multimap.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare)
#include "multimap/multimap.cons/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare_alloc)
#include "multimap/multimap.cons/compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "multimap/multimap.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "multimap/multimap.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_assign)
#include "multimap/multimap.cons/copy_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_deduct)
#include "multimap/multimap.cons/deduct.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(cons_deduct_const)
#include "multimap/multimap.cons/deduct_const.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_default)
#include "multimap/multimap.cons/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default_noexcept)
#include "multimap/multimap.cons/default_noexcept.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(cons_default_recursive)
#include "multimap/multimap.cons/default_recursive.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
#include "multimap/multimap.cons/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "multimap/multimap.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare)
#include "multimap/multimap.cons/initializer_list_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare_alloc)
#include "multimap/multimap.cons/initializer_list_compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter)
#include "multimap/multimap.cons/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp)
#include "multimap/multimap.cons/iter_iter_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp_alloc)
#include "multimap/multimap.cons/iter_iter_comp_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "multimap/multimap.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "multimap/multimap.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign)
#include "multimap/multimap.cons/move_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
#include "multimap/multimap.cons/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_noexcept)
#include "multimap/multimap.cons/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "multimap/multimap.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "multimap/multimap.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "multimap/multimap.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "multimap/multimap.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter)
#include "multimap/multimap.modifiers/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "multimap/multimap.modifiers/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "multimap/multimap.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_iterator)
#include "multimap/multimap.modifiers/extract_iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_key)
#include "multimap/multimap.modifiers/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_allocator_requirements)
#include "multimap/multimap.modifiers/insert_allocator_requirements.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_cv)
#include "multimap/multimap.modifiers/insert_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_initializer_list)
#include "multimap/multimap.modifiers/insert_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_cv)
#include "multimap/multimap.modifiers/insert_iter_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_iter)
#include "multimap/multimap.modifiers/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_rv)
#include "multimap/multimap.modifiers/insert_iter_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type)
#include "multimap/multimap.modifiers/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type_hint)
#include "multimap/multimap.modifiers/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_rv)
#include "multimap/multimap.modifiers/insert_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_merge)
#include "multimap/multimap.modifiers/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_compare_three_way)
#include "multimap/multimap.nonmember/compare.three_way.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_op_compare)
#include "multimap/multimap.nonmember/op_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(observers_key_comp)
#include "multimap/multimap.observers/key_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(observers_value_comp)
#include "multimap/multimap.observers/value_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_contains)
#include "multimap/multimap.ops/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_contains_transparent)
#include "multimap/multimap.ops/contains_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count)
#include "multimap/multimap.ops/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count_transparent)
#include "multimap/multimap.ops/count_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_count0)
#include "multimap/multimap.ops/count0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range)
#include "multimap/multimap.ops/equal_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range_transparent)
#include "multimap/multimap.ops/equal_range_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_equal_range0)
#include "multimap/multimap.ops/equal_range0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_find)
#include "multimap/multimap.ops/find.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_find0)
#include "multimap/multimap.ops/find0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_lower_bound)
#include "multimap/multimap.ops/lower_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_lower_bound0)
#include "multimap/multimap.ops/lower_bound0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_upper_bound)
#include "multimap/multimap.ops/upper_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_upper_bound0)
#include "multimap/multimap.ops/upper_bound0.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_member_swap)
#include "multimap/multimap.special/member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_non_member_swap)
#include "multimap/multimap.special/non_member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap_noexcept)
#include "multimap/multimap.special/swap_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(value_compare_invoke)
#include "multimap/multimap.value_compare/invoke.pass.cpp"
LIBCXX_TEST_END
