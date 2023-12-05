/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxSetTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(clear)
#include "libcxx/set/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains)
#include "libcxx/set/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains_transparent)
#include "libcxx/set/contains_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count)
#include "libcxx/set/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count_transparent)
#include "libcxx/set/count_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace)
#include "libcxx/set/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace_hint)
#include "libcxx/set/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "libcxx/set/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range)
#include "libcxx/set/equal_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_transparent)
#include "libcxx/set/equal_range_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter)
#include "libcxx/set/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_iter)
#include "libcxx/set/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_key)
#include "libcxx/set/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extract_iterator)
#include "libcxx/set/extract_iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extract_key)
#include "libcxx/set/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find)
#include "libcxx/set/find.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(get_allocator)
#include "libcxx/set/get_allocator.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(incomplete_type)
#include "libcxx/set/incomplete_type.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(insert_cv)
#include "libcxx/set/insert_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_and_emplace_allocator_requirements)
#include "libcxx/set/insert_and_emplace_allocator_requirements.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_initializer_list)
#include "libcxx/set/insert_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_cv)
#include "libcxx/set/insert_iter_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_iter)
#include "libcxx/set/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_rv)
#include "libcxx/set/insert_iter_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type)
#include "libcxx/set/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type_hint)
#include "libcxx/set/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_rv)
#include "libcxx/set/insert_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator)
#include "libcxx/set/iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "libcxx/set/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_types)
#include "libcxx/set/iterator_types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(lower_bound)
#include "libcxx/set/lower_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "libcxx/set/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(merge)
#include "libcxx/set/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "libcxx/set/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "libcxx/set/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "libcxx/set/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(upper_bound)
#include "libcxx/set/upper_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_alloc)
#include "libcxx/set/set.cons/alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "libcxx/set/set.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare)
#include "libcxx/set/set.cons/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare_alloc)
#include "libcxx/set/set.cons/compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "libcxx/set/set.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "libcxx/set/set.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_assign)
#include "libcxx/set/set.cons/copy_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_deduct)
#include "libcxx/set/set.cons/deduct.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default)
#include "libcxx/set/set.cons/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default_noexcept)
#include "libcxx/set/set.cons/default_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
#include "libcxx/set/set.cons/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "libcxx/set/set.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare)
#include "libcxx/set/set.cons/initializer_list_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare_alloc)
#include "libcxx/set/set.cons/initializer_list_compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter)
#include "libcxx/set/set.cons/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_alloc)
#include "libcxx/set/set.cons/iter_iter_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp)
#include "libcxx/set/set.cons/iter_iter_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "libcxx/set/set.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "libcxx/set/set.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign)
#include "libcxx/set/set.cons/move_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
#include "libcxx/set/set.cons/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_noexcept)
#include "libcxx/set/set.cons/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "libcxx/set/set.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_compare_three_way)
#include "libcxx/set/set.nonmember/compare.three_way.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(nonmember_op_compare)
#include "libcxx/set/set.nonmember/op_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(observers_comp)
#include "libcxx/set/set.observers/comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_member_swap)
#include "libcxx/set/set.special/member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_non_member_swap)
#include "libcxx/set/set.special/non_member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap_noexcept)
#include "libcxx/set/set.special/swap_noexcept.pass.cpp"
LIBCXX_TEST_END
