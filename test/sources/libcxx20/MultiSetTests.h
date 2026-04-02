/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx20/MultiSetTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(clear)
#include "multiset/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count)
#include "multiset/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count_transparent)
#include "multiset/count_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace)
#include "multiset/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace_hint)
#include "multiset/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "multiset/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range)
#include "multiset/equal_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_transparent)
#include "multiset/equal_range_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter)
#include "multiset/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_iter)
#include "multiset/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_key)
#include "multiset/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extract_iterator)
#include "multiset/extract_iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extract_key)
#include "multiset/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find)
#include "multiset/find.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(get_allocator)
#include "multiset/get_allocator.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(incomplete_type)
#include "multiset/incomplete_type.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(insert_cv)
#include "multiset/insert_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_emplace_allocator_requirements)
#include "multiset/insert_emplace_allocator_requirements.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_initializer_list)
#include "multiset/insert_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_cv)
#include "multiset/insert_iter_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_iter)
#include "multiset/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_rv)
#include "multiset/insert_iter_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type)
#include "multiset/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type_hint)
#include "multiset/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(insert_range)
#include "multiset/insert_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(insert_rv)
#include "multiset/insert_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator)
#include "multiset/iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "multiset/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(lower_bound)
#include "multiset/lower_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "multiset/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(merge)
#include "multiset/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "multiset/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(scary)
#include "multiset/scary.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "multiset/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "multiset/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(upper_bound)
#include "multiset/upper_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_contains)
#include "multiset/common/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_contains_transparent)
#include "multiset/common/contains_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_iterator_types)
#include "multiset/common/iterator_types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_alloc)
#include "multiset/multiset.cons/alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "multiset/multiset.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare)
#include "multiset/multiset.cons/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare_alloc)
#include "multiset/multiset.cons/compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "multiset/multiset.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "multiset/multiset.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_assign)
#include "multiset/multiset.cons/copy_assign.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_CLASS
LIBCXX_TEST_BEGIN(cons_deduct)
#include "multiset/multiset.cons/deduct.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_default)
#include "multiset/multiset.cons/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default_noexcept)
#include "multiset/multiset.cons/default_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
#include "multiset/multiset.cons/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(cons_from_range)
#include "multiset/multiset.cons/from_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "multiset/multiset.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare)
#include "multiset/multiset.cons/initializer_list_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare_alloc)
#include "multiset/multiset.cons/initializer_list_compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter)
#include "multiset/multiset.cons/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_alloc)
#include "multiset/multiset.cons/iter_iter_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp)
#include "multiset/multiset.cons/iter_iter_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "multiset/multiset.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "multiset/multiset.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign)
#include "multiset/multiset.cons/move_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
#include "multiset/multiset.cons/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_noexcept)
#include "multiset/multiset.cons/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "multiset/multiset.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_compare_three_way)
#include "multiset/multiset.nonmember/compare.three_way.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_op_compare)
#include "multiset/multiset.nonmember/op_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(observers_comp)
#include "multiset/multiset.observers/comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_member_swap)
#include "multiset/multiset.special/member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_non_member_swap)
#include "multiset/multiset.special/non_member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap_noexcept)
#include "multiset/multiset.special/swap_noexcept.pass.cpp"
LIBCXX_TEST_END
