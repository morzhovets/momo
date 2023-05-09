/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxMultiSetTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(clear)
#include "libcxx/multiset/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains)
#include "libcxx/multiset/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count)
#include "libcxx/multiset/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count_transparent)
#include "libcxx/multiset/count_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace)
#include "libcxx/multiset/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace_hint)
#include "libcxx/multiset/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "libcxx/multiset/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range)
#include "libcxx/multiset/equal_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_transparent)
#include "libcxx/multiset/equal_range_transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter)
#include "libcxx/multiset/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_iter_iter)
#include "libcxx/multiset/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_key)
#include "libcxx/multiset/erase_key.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(extract_iterator)
//#include "libcxx/multiset/extract_iterator.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extract_key)
#include "libcxx/multiset/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find)
#include "libcxx/multiset/find.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(incomplete_type)
//#include "libcxx/multiset/incomplete_type.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_cv)
#include "libcxx/multiset/insert_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_initializer_list)
#include "libcxx/multiset/insert_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_cv)
#include "libcxx/multiset/insert_iter_cv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_iter)
#include "libcxx/multiset/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_rv)
#include "libcxx/multiset/insert_iter_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type)
#include "libcxx/multiset/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type_hint)
#include "libcxx/multiset/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_rv)
#include "libcxx/multiset/insert_rv.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator)
#include "libcxx/multiset/iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(lower_bound)
#include "libcxx/multiset/lower_bound.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(max_size)
//#include "libcxx/multiset/max_size.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(merge)
#include "libcxx/multiset/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "libcxx/multiset/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "libcxx/multiset/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(upper_bound)
#include "libcxx/multiset/upper_bound.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_alloc)
#include "libcxx/multiset/multiset.cons/alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "libcxx/multiset/multiset.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare)
#include "libcxx/multiset/multiset.cons/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_compare_alloc)
#include "libcxx/multiset/multiset.cons/compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "libcxx/multiset/multiset.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "libcxx/multiset/multiset.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_assign)
#include "libcxx/multiset/multiset.cons/copy_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_deduct)
#include "libcxx/multiset/multiset.cons/deduct.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default)
#include "libcxx/multiset/multiset.cons/default.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_default_noexcept)
//#include "libcxx/multiset/multiset.cons/default_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
//#include "libcxx/multiset/multiset.cons/dtor_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "libcxx/multiset/multiset.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare)
#include "libcxx/multiset/multiset.cons/initializer_list_compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_compare_alloc)
#include "libcxx/multiset/multiset.cons/initializer_list_compare_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter)
#include "libcxx/multiset/multiset.cons/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_alloc)
#include "libcxx/multiset/multiset.cons/iter_iter_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_iter_iter_comp)
#include "libcxx/multiset/multiset.cons/iter_iter_comp.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "libcxx/multiset/multiset.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "libcxx/multiset/multiset.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign)
#include "libcxx/multiset/multiset.cons/move_assign.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
//#include "libcxx/multiset/multiset.cons/move_assign_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_noexcept)
//#include "libcxx/multiset/multiset.cons/move_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "libcxx/multiset/multiset.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_member_swap)
#include "libcxx/multiset/multiset.special/member_swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_non_member_swap)
#include "libcxx/multiset/multiset.special/non_member_swap.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(special_swap_noexcept)
//#include "libcxx/multiset/multiset.special/swap_noexcept.pass.cpp"
//LIBCXX_TEST_END
