/**********************************************************\

  tests/LibcxxVectorTests.h

\**********************************************************/

#ifndef LIBCXX_TEST_INTCAP_ARRAY
LIBCXX_TEST_BEGIN(asan_throw)
#include "libcxx/vector/asan_throw.pass.cpp"
LIBCXX_TEST_END
#endif

//LIBCXX_TEST_BEGIN(const_value_type)
//#include "libcxx/vector/const_value_type.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_back)
#include "libcxx/vector/db_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_cback)
#include "libcxx/vector/db_cback.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_cfront)
#include "libcxx/vector/db_cfront.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_cindex)
#include "libcxx/vector/db_cindex.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_front)
#include "libcxx/vector/db_front.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_index)
#include "libcxx/vector/db_index.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_2)
#include "libcxx/vector/db_iterators_2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_3)
#include "libcxx/vector/db_iterators_3.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_4)
#include "libcxx/vector/db_iterators_4.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_5)
#include "libcxx/vector/db_iterators_5.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_6)
#include "libcxx/vector/db_iterators_6.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_7)
#include "libcxx/vector/db_iterators_7.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(db_iterators_8)
#include "libcxx/vector/db_iterators_8.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "libcxx/vector/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "libcxx/vector/types.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_SEGMENTED_ARRAY

#ifndef LIBCXX_TEST_INTCAP_ARRAY
LIBCXX_TEST_BEGIN(capacity_capacity)
#include "libcxx/vector/vector.capacity/capacity.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(capacity_reserve)
#include "libcxx/vector/vector.capacity/reserve.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_resize_size)
#include "libcxx/vector/vector.capacity/resize_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_resize_size_value)
#include "libcxx/vector/vector.capacity/resize_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_shrink_to_fit)
#include "libcxx/vector/vector.capacity/shrink_to_fit.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(capacity_swap)
#include "libcxx/vector/vector.capacity/swap.pass.cpp"
LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(cons_assign_copy)
#include "libcxx/vector/vector.cons/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_initializer_list)
#include "libcxx/vector/vector.cons/assign_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_move)
#include "libcxx/vector/vector.cons/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_assign_size_value)
#include "libcxx/vector/vector.cons/assign_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_default)
#include "libcxx/vector/vector.cons/construct_default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_iter_iter)
#include "libcxx/vector/vector.cons/construct_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_iter_iter_alloc)
#include "libcxx/vector/vector.cons/construct_iter_iter_alloc.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_INTCAP_ARRAY
LIBCXX_TEST_BEGIN(cons_construct_size)
#include "libcxx/vector/vector.cons/construct_size.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cons_construct_size_value)
#include "libcxx/vector/vector.cons/construct_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_construct_size_value_alloc)
#include "libcxx/vector/vector.cons/construct_size_value_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy)
#include "libcxx/vector/vector.cons/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_copy_alloc)
#include "libcxx/vector/vector.cons/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_INTCAP_ARRAY
LIBCXX_TEST_BEGIN(cons_default_recursive)
#include "libcxx/vector/vector.cons/default.recursive.pass.cpp"
LIBCXX_TEST_END
#endif

//LIBCXX_TEST_BEGIN(cons_default_noexcept)
//#include "libcxx/vector/vector.cons/default_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_dtor_noexcept)
//#include "libcxx/vector/vector.cons/dtor_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list)
#include "libcxx/vector/vector.cons/initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_initializer_list_alloc)
#include "libcxx/vector/vector.cons/initializer_list_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move)
#include "libcxx/vector/vector.cons/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_move_alloc)
#include "libcxx/vector/vector.cons/move_alloc.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_assign_noexcept)
//#include "libcxx/vector/vector.cons/move_assign_noexcept.pass.cpp"
//LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(cons_move_noexcept)
//#include "libcxx/vector/vector.cons/move_noexcept.pass.cpp"
//LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cons_op_equal_initializer_list)
#include "libcxx/vector/vector.cons/op_equal_initializer_list.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
#ifndef LIBCXX_TEST_INTCAP_ARRAY

LIBCXX_TEST_BEGIN(data_data)
#include "libcxx/vector/vector.data/data.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(data_data_const)
#include "libcxx/vector/vector.data/data_const.pass.cpp"
LIBCXX_TEST_END

#endif
#endif

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "libcxx/vector/vector.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_back)
#include "libcxx/vector/vector.modifiers/emplace_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_extra)
#include "libcxx/vector/vector.modifiers/emplace_extra.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter)
#include "libcxx/vector/vector.modifiers/erase_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db1)
#include "libcxx/vector/vector.modifiers/erase_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_db2)
#include "libcxx/vector/vector.modifiers/erase_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter)
#include "libcxx/vector/vector.modifiers/erase_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db1)
#include "libcxx/vector/vector.modifiers/erase_iter_iter_db1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db2)
#include "libcxx/vector/vector.modifiers/erase_iter_iter_db2.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db3)
#include "libcxx/vector/vector.modifiers/erase_iter_iter_db3.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_iter_iter_db4)
#include "libcxx/vector/vector.modifiers/erase_iter_iter_db4.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_initializer_list)
#include "libcxx/vector/vector.modifiers/insert_iter_initializer_list.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_iter_iter)
#include "libcxx/vector/vector.modifiers/insert_iter_iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_rvalue)
#include "libcxx/vector/vector.modifiers/insert_iter_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_size_value)
#include "libcxx/vector/vector.modifiers/insert_iter_size_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_value)
#include "libcxx/vector/vector.modifiers/insert_iter_value.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_pop_back)
#include "libcxx/vector/vector.modifiers/pop_back.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_push_back)
#include "libcxx/vector/vector.modifiers/push_back.pass.cpp"
LIBCXX_TEST_END

#ifndef LIBCXX_TEST_INTCAP_ARRAY
LIBCXX_TEST_BEGIN(modifiers_push_back_exception_safety)
#include "libcxx/vector/vector.modifiers/push_back_exception_safety.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_push_back_rvalue)
#include "libcxx/vector/vector.modifiers/push_back_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_db_swap_1)
#include "libcxx/vector/vector.special/db_swap_1.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(special_swap)
#include "libcxx/vector/vector.special/swap.pass.cpp"
LIBCXX_TEST_END

//LIBCXX_TEST_BEGIN(special_swap_noexcept)
//#include "libcxx/vector/vector.special/swap_noexcept.pass.cpp"
//LIBCXX_TEST_END
