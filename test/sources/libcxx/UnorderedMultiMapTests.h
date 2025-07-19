/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx/UnorderedMultiMapTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(contains_transparent)
#include "unord.multimap/contains.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count)
#include "unord.multimap/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count_transparent)
#include "unord.multimap/count.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "unord.multimap/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq_different_hash)
#include "unord.multimap/eq.different_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq)
#include "unord.multimap/eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_transparent)
#include "unord.multimap/equal_range.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_const)
#include "unord.multimap/equal_range_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_non_const)
#include "unord.multimap/equal_range_non_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_if_)
#include "unord.multimap/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_transparent)
#include "unord.multimap/find.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_const)
#include "unord.multimap/find_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_non_const)
#include "unord.multimap/find_non_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(get_allocator)
#include "unord.multimap/get_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(hash_function)
#include "unord.multimap/hash_function.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(incomplete)
#include "unord.multimap/incomplete.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "unord.multimap/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "unord.multimap/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(key_eq)
#include "unord.multimap/key_eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "unord.multimap/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "unord.multimap/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "unord.multimap/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_member)
#include "unord.multimap/swap_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "unord.multimap/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_contains)
#include "unord.multimap/common/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_iterator_difference_type)
#include "unord.multimap/common/iterator_difference_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extra_emplace_ext)
#include "unord.multimap/extra/emplace_ext.pass.cpp"
LIBCXX_TEST_END

#ifndef TEST_HAS_NO_EXCEPTIONS
#ifdef LIBCXX_TEST_FAILURE

LIBCXX_TEST_BEGIN(spec_debug_erase_iter)
#include "unord.multimap/spec/debug.erase.iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_erase_iter_iter)
#include "unord.multimap/spec/debug.erase.iter_iter.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(spec_debug_insert_hint_const_lvalue)
#include "unord.multimap/spec/debug.insert.hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END
#endif

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(spec_debug_insert_hint_rvalue)
#include "unord.multimap/spec/debug.insert.hint_rvalue.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(spec_debug_iterator_dereference)
#include "unord.multimap/spec/debug.iterator.dereference.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_increment)
#include "unord.multimap/spec/debug.iterator.increment.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_swap)
#include "unord.multimap/spec/debug.swap.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_FAILURE
#endif // TEST_HAS_NO_EXCEPTIONS

LIBCXX_TEST_BEGIN(cnstr_allocator)
#include "unord.multimap/unord.multimap.cnstr/allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_copy)
#include "unord.multimap/unord.multimap.cnstr/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_init)
#include "unord.multimap/unord.multimap.cnstr/assign_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_move)
#include "unord.multimap/unord.multimap.cnstr/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy)
#include "unord.multimap/unord.multimap.cnstr/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy_alloc)
#include "unord.multimap/unord.multimap.cnstr/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_CLASS

LIBCXX_TEST_BEGIN(cnstr_deduct)
#include "unord.multimap/unord.multimap.cnstr/deduct.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_deduct_const)
#include "unord.multimap/unord.multimap.cnstr/deduct_const.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_CLASS

LIBCXX_TEST_BEGIN(cnstr_default)
#include "unord.multimap/unord.multimap.cnstr/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_default_noexcept)
#include "unord.multimap/unord.multimap.cnstr/default_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_dtor_noexcept)
#include "unord.multimap/unord.multimap.cnstr/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(cnstr_from_range)
#include "unord.multimap/unord.multimap.cnstr/from_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cnstr_init)
#include "unord.multimap/unord.multimap.cnstr/init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size)
#include "unord.multimap/unord.multimap.cnstr/init_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_allocator)
#include "unord.multimap/unord.multimap.cnstr/init_size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash)
#include "unord.multimap/unord.multimap.cnstr/init_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_allocator)
#include "unord.multimap/unord.multimap.cnstr/init_size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal)
#include "unord.multimap/unord.multimap.cnstr/init_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal_allocator)
#include "unord.multimap/unord.multimap.cnstr/init_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move)
#include "unord.multimap/unord.multimap.cnstr/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter)
#include "unord.multimap/unord.multimap.cnstr/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size)
#include "unord.multimap/unord.multimap.cnstr/iter_iter_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_allocator)
#include "unord.multimap/unord.multimap.cnstr/iter_iter_size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash)
#include "unord.multimap/unord.multimap.cnstr/iter_iter_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_allocator)
#include "unord.multimap/unord.multimap.cnstr/iter_iter_size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_equal)
#include "unord.multimap/unord.multimap.cnstr/iter_iter_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_equal_allocator)
#include "unord.multimap/unord.multimap.cnstr/iter_iter_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_alloc)
#include "unord.multimap/unord.multimap.cnstr/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_assign_noexcept)
#include "unord.multimap/unord.multimap.cnstr/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_noexcept)
#include "unord.multimap/unord.multimap.cnstr/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size)
#include "unord.multimap/unord.multimap.cnstr/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_allocator)
#include "unord.multimap/unord.multimap.cnstr/size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash)
#include "unord.multimap/unord.multimap.cnstr/size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_allocator)
#include "unord.multimap/unord.multimap.cnstr/size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal)
#include "unord.multimap/unord.multimap.cnstr/size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal_allocator)
#include "unord.multimap/unord.multimap.cnstr/size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "unord.multimap/unord.multimap.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "unord.multimap/unord.multimap.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "unord.multimap/unord.multimap.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_const_iter)
#include "unord.multimap/unord.multimap.modifiers/erase_const_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "unord.multimap/unord.multimap.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_range)
#include "unord.multimap/unord.multimap.modifiers/erase_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_allocator_requirements)
#include "unord.multimap/unord.multimap.modifiers/insert_allocator_requirements.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_const_lvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_const_lvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_rvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_hint_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_init)
#include "unord.multimap/unord.multimap.modifiers/insert_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_iter_iter)
#include "unord.multimap/unord.multimap.modifiers/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(modifiers_insert_range)
#include "unord.multimap/unord.multimap.modifiers/insert_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(modifiers_insert_rvalue)
#include "unord.multimap/unord.multimap.modifiers/insert_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_noexcept)
#include "unord.multimap/unord.multimap.swap/swap_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_non_member)
#include "unord.multimap/unord.multimap.swap/swap_non_member.pass.cpp"
LIBCXX_TEST_END
