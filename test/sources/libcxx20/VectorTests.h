/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx20/VectorTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(access)
#include "vector/access.pass.cpp"
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

#ifndef TEST_HAS_NO_EXCEPTIONS

LIBCXX_TEST_BEGIN(spec_asan_throw)
#include "vector/spec/asan_throw.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_FAILURE

LIBCXX_TEST_BEGIN(spec_assert_back_empty)
#include "vector/spec/assert.back.empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_cback_empty)
#include "vector/spec/assert.cback.empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_cfront_empty)
#include "vector/spec/assert.cfront.empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_cindex_oob)
#include "vector/spec/assert.cindex.oob.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_front_empty)
#include "vector/spec/assert.front.empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_index_oob)
#include "vector/spec/assert.index.oob.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_pop_back_empty)
#include "vector/spec/assert.pop_back.empty.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_ARRAY

LIBCXX_TEST_BEGIN(spec_debug_iterator_add)
#include "vector/spec/debug.iterator.add.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_compare)
#include "vector/spec/debug.iterator.compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_decrement)
#include "vector/spec/debug.iterator.decrement.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_dereference)
#include "vector/spec/debug.iterator.dereference.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_increment)
#include "vector/spec/debug.iterator.increment.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_index)
#include "vector/spec/debug.iterator.index.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_subtract)
#include "vector/spec/debug.iterator.subtract.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_ARRAY

#endif // LIBCXX_TEST_FAILURE

#endif // TEST_HAS_NO_EXCEPTIONS

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

#ifndef TEST_HAS_NO_EXCEPTIONS
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
LIBCXX_TEST_BEGIN(capacity_reserve_exceptions)
#include "vector/vector.capacity/reserve_exceptions.pass.cpp"
LIBCXX_TEST_END
#endif
#endif

LIBCXX_TEST_BEGIN(capacity_resize_size)
#include "vector/vector.capacity/resize_size.pass.cpp"
LIBCXX_TEST_END

#ifndef TEST_HAS_NO_EXCEPTIONS
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
LIBCXX_TEST_BEGIN(capacity_resize_size_exceptions)
#include "vector/vector.capacity/resize_size_exceptions.pass.cpp"
LIBCXX_TEST_END
#endif
#endif

LIBCXX_TEST_BEGIN(capacity_resize_size_value)
#include "vector/vector.capacity/resize_size_value.pass.cpp"
LIBCXX_TEST_END

#ifndef TEST_HAS_NO_EXCEPTIONS
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
LIBCXX_TEST_BEGIN(capacity_resize_size_value_exceptions)
#include "vector/vector.capacity/resize_size_value_exceptions.pass.cpp"
LIBCXX_TEST_END
#endif
#endif

LIBCXX_TEST_BEGIN(capacity_shrink_to_fit)
#include "vector/vector.capacity/shrink_to_fit.pass.cpp"
LIBCXX_TEST_END

#ifndef TEST_HAS_NO_EXCEPTIONS
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
LIBCXX_TEST_BEGIN(capacity_shrink_to_fit_exceptions)
#include "vector/vector.capacity/shrink_to_fit_exceptions.pass.cpp"
LIBCXX_TEST_END
#endif
#endif

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

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(cons_construct_from_range)
#include "vector/vector.cons/construct_from_range.pass.cpp"
LIBCXX_TEST_END
#endif

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

#ifdef LIBCXX_TEST_CLASS
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

#ifndef TEST_HAS_NO_EXCEPTIONS
LIBCXX_TEST_BEGIN(cons_exceptions)
#include "vector/vector.cons/exceptions.pass.cpp"
LIBCXX_TEST_END
#endif

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

#endif // LIBCXX_TEST_SEGMENTED_ARRAY

LIBCXX_TEST_BEGIN(erasure_erase)
#include "vector/vector.erasure/erase.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "vector/vector.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(modifiers_append_range)
#include "vector/vector.modifiers/append_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_assert_push_back_invalidation)
#include "vector/vector.modifiers/assert.push_back.invalidation.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(modifiers_assign_range)
#include "vector/vector.modifiers/assign_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "vector/vector.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_destroy_elements)
#include "vector/vector.modifiers/destroy_elements.pass.cpp"
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

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "vector/vector.modifiers/erase_iter_iter.pass.cpp"
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

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(modifiers_insert_range)
#include "vector/vector.modifiers/insert_range.pass.cpp"
LIBCXX_TEST_END
#endif

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

LIBCXX_TEST_BEGIN(special_swap)
#include "vector/vector.special/swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap_noexcept)
#include "vector/vector.special/swap_noexcept.compile.pass.cpp"
LIBCXX_TEST_END
