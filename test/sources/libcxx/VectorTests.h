/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx/VectorTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(access)
#include "vector/access.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(asan_throw)
#include "vector/asan_throw.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(compare)
#include "vector/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(compare_three_way)
#include "vector/compare.three_way.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
LIBCXX_TEST_BEGIN(contiguous)
#include "vector/contiguous.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(db_back)
#include "vector/db_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_cback)
#include "vector/db_cback.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_cfront)
#include "vector/db_cfront.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_cindex)
#include "vector/db_cindex.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_front)
#include "vector/db_front.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_index)
#include "vector/db_index.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_ARRAY

LIBCXX_TEST_BEGIN(db_iterators_2)
#include "vector/db_iterators_2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_3)
#include "vector/db_iterators_3.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_4)
#include "vector/db_iterators_4.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_5)
#include "vector/db_iterators_5.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_6)
#include "vector/db_iterators_6.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_7)
#include "vector/db_iterators_7.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_8)
#include "vector/db_iterators_8.pass.cpp"
LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(get_allocator)
#include "vector/get_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "vector/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "vector/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "vector/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(reverse_iterators)
#include "vector/reverse_iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "vector/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_capacity)
#include "vector/vector.capacity/capacity.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_empty)
#include "vector/vector.capacity/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_max_size)
#include "vector/vector.capacity/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_reserve)
#include "vector/vector.capacity/reserve.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_resize_size)
#include "vector/vector.capacity/resize_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_resize_size_value)
#include "vector/vector.capacity/resize_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_shrink_to_fit)
#include "vector/vector.capacity/shrink_to_fit.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_size)
#include "vector/vector.capacity/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_swap)
#include "vector/vector.capacity/swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_copy)
#include "vector/vector.cons/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "vector/vector.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_iter_iter)
#include "vector/vector.cons/assign_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_move)
#include "vector/vector.cons/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_size_value)
#include "vector/vector.cons/assign_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_default)
#include "vector/vector.cons/construct_default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_iter_iter)
#include "vector/vector.cons/construct_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_iter_iter_alloc)
#include "vector/vector.cons/construct_iter_iter_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_size)
#include "vector/vector.cons/construct_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_size_value)
#include "vector/vector.cons/construct_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_size_value_alloc)
#include "vector/vector.cons/construct_size_value_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "vector/vector.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "vector/vector.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_ARRAY
LIBCXX_TEST_BEGIN(cons_deduct)
#include "vector/vector.cons/deduct.pass.cpp"
LIBCXX_TEST_END
#endif

#ifndef LIBCXX_TEST_INTCAP_ARRAY
LIBCXX_TEST_BEGIN(cons_default_recursive)
#include "vector/vector.cons/default.recursive.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_default_noexcept)
#include "vector/vector.cons/default_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
#include "vector/vector.cons/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_exceptions)
#include "vector/vector.cons/exceptions.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "vector/vector.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_alloc)
#include "vector/vector.cons/initializer_list_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "vector/vector.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "vector/vector.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
#include "vector/vector.cons/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_noexcept)
#include "vector/vector.cons/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_op_equal_initializer_list)
#include "vector/vector.cons/op_equal_initializer_list.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_SEGMENTED_ARRAY

LIBCXX_TEST_BEGIN(data_data)
#include "vector/vector.data/data.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(data_data_const)
#include "vector/vector.data/data_const.pass.cpp"
LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(erasure_erase)
#include "vector/vector.erasure/erase.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "vector/vector.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "vector/vector.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "vector/vector.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_back)
#include "vector/vector.modifiers/emplace_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_extra)
#include "vector/vector.modifiers/emplace_extra.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter)
#include "vector/vector.modifiers/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db1)
#include "vector/vector.modifiers/erase_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db2)
#include "vector/vector.modifiers/erase_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "vector/vector.modifiers/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db1)
#include "vector/vector.modifiers/erase_iter_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db2)
#include "vector/vector.modifiers/erase_iter_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db3)
#include "vector/vector.modifiers/erase_iter_iter_db3.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db4)
#include "vector/vector.modifiers/erase_iter_iter_db4.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_initializer_list)
#include "vector/vector.modifiers/insert_iter_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_iter_iter)
#include "vector/vector.modifiers/insert_iter_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_lvalue)
#include "vector/vector.modifiers/insert_iter_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_rvalue)
#include "vector/vector.modifiers/insert_iter_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_size_value)
#include "vector/vector.modifiers/insert_iter_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_pop_back)
#include "vector/vector.modifiers/pop_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_push_back)
#include "vector/vector.modifiers/push_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_push_back_exception_safety)
#include "vector/vector.modifiers/push_back_exception_safety.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_push_back_rvalue)
#include "vector/vector.modifiers/push_back_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_db_swap_1)
#include "vector/vector.special/db_swap_1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap)
#include "vector/vector.special/swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap_noexcept)
#include "vector/vector.special/swap_noexcept.compile.pass.cpp"
LIBCXX_TEST_END
