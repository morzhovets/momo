/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx20/ListTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(compare)
#include "list/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(compare_three_way)
#include "list/compare.three_way.pass.cpp"
LIBCXX_TEST_END

#ifndef TEST_HAS_NO_EXCEPTIONS
LIBCXX_TEST_BEGIN(exception_safety)
#include "list/exception_safety.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(get_allocator)
#include "list/get_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(incomplete_type)
#include "list/incomplete_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "list/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "list/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "list/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "list/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_empty)
#include "list/list.capacity/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_max_size)
#include "list/list.capacity/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_resize_size)
#include "list/list.capacity/resize_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_resize_size_value)
#include "list/list.capacity/resize_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_size)
#include "list/list.capacity/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_copy)
#include "list/list.cons/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "list/list.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_move)
#include "list/list.cons/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "list/list.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "list/list.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_CLASS
LIBCXX_TEST_BEGIN(cons_deduct)
#include "list/list.cons/deduct.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_default)
#include "list/list.cons/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default_noexcept)
#include "list/list.cons/default_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_default_stack_alloc)
#include "list/list.cons/default_stack_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_dtor)
#include "list/list.cons/dtor.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
#include "list/list.cons/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(cons_from_range)
#include "list/list.cons/from_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "list/list.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_alloc)
#include "list/list.cons/initializer_list_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_input_iterator)
#include "list/list.cons/input_iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "list/list.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "list/list.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
#include "list/list.cons/move_assign_noexcept.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_noexcept)
#include "list/list.cons/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_op_equal_initializer_list)
#include "list/list.cons/op_equal_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_size_type)
#include "list/list.cons/size_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_size_value_alloc)
#include "list/list.cons/size_value_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase)
#include "list/list.erasure/erase.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erasure_erase_if)
#include "list/list.erasure/erase_if.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23

LIBCXX_TEST_BEGIN(modifiers_append_range)
#include "list/list.modifiers/append_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_assign_range)
#include "list/list.modifiers/assign_range.pass.cpp"
LIBCXX_TEST_END

#endif // TEST_STD_VER

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "list/list.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_back)
#include "list/list.modifiers/emplace_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_front)
#include "list/list.modifiers/emplace_front.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter)
#include "list/list.modifiers/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "list/list.modifiers/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_pop_back)
#include "list/list.modifiers/pop_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_pop_front)
#include "list/list.modifiers/pop_front.pass.cpp"
LIBCXX_TEST_END

//#if TEST_STD_VER >= 23
//LIBCXX_TEST_BEGIN(modifiers_prepend_range)
//#include "list/list.modifiers/prepend_range.pass.cpp"
//LIBCXX_TEST_END
//#endif

LIBCXX_TEST_BEGIN(modifiers_push_back)
#include "list/list.modifiers/push_back.pass.cpp"
LIBCXX_TEST_END

#ifndef TEST_HAS_NO_EXCEPTIONS
LIBCXX_TEST_BEGIN(modifiers_push_back_exception_safety)
#include "list/list.modifiers/push_back_exception_safety.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_push_back_rvalue)
#include "list/list.modifiers/push_back_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_push_front)
#include "list/list.modifiers/push_front.pass.cpp"
LIBCXX_TEST_END

#ifndef TEST_HAS_NO_EXCEPTIONS
LIBCXX_TEST_BEGIN(modifiers_push_front_exception_safety)
#include "list/list.modifiers/push_front_exception_safety.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_push_front_rvalue)
#include "list/list.modifiers/push_front_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_remove)
#include "list/list.ops/remove.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(ops_remove_if)
#include "list/list.ops/remove_if.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(ops_unique)
//#include "list/list.ops/unique.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(ops_unique_pred)
//#include "list/list.ops/unique_pred.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap)
#include "list/list.special/swap.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap_noexcept)
#include "list/list.special/swap_noexcept.pass.cpp"
LIBCXX_TEST_END
