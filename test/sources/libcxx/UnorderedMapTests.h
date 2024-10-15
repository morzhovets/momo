/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx/UnorderedMapTests.h

\**********************************************************/

LIBCXX_TEST_BEGIN(bucket)
#include "unord.map/bucket.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(bucket_count)
#include "unord.map/bucket_count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(bucket_size)
#include "unord.map/bucket_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(compare)
#include "unord.map/compare.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains)
#include "unord.map/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains_transparent)
#include "unord.map/contains.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count)
#include "unord.map/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count_transparent)
#include "unord.map/count.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "unord.map/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq_different_hash)
#include "unord.map/eq.different_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq)
#include "unord.map/eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_transparent)
#include "unord.map/equal_range.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_const)
#include "unord.map/equal_range_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_non_const)
#include "unord.map/equal_range_non_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_if_)
#include "unord.map/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_transparent)
#include "unord.map/find.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_const)
#include "unord.map/find_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_non_const)
#include "unord.map/find_non_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(get_allocator)
#include "unord.map/get_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(hash_function)
#include "unord.map/hash_function.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(incomplete_type)
#include "unord.map/incomplete_type.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "unord.map/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "unord.map/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(key_eq)
#include "unord.map/key_eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(load_factor)
#include "unord.map/load_factor.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(local_iterators)
#include "unord.map/local_iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_bucket_count)
#include "unord.map/max_bucket_count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_load_factor)
#include "unord.map/max_load_factor.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "unord.map/max_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "unord.map/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(rehash)
#include "unord.map/rehash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(reserve)
#include "unord.map/reserve.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "unord.map/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_member)
#include "unord.map/swap_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "unord.map/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_iterator_difference_type)
#include "unord.map/common/iterator_difference_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extra_emplace_ext)
#include "unord.map/extra/emplace_ext.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_FAILURE

LIBCXX_TEST_BEGIN(spec_assert_bucket)
#include "unord.map/spec/assert.bucket.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_bucket_size)
#include "unord.map/spec/assert.bucket_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_max_load_factor)
#include "unord.map/spec/assert.max_load_factor.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_at_abort)
#include "unord.map/spec/at.abort.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_at_const_abort)
#include "unord.map/spec/at.const.abort.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_erase_iter)
#include "unord.map/spec/debug.erase.iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_erase_iter_iter)
#include "unord.map/spec/debug.erase.iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_insert_hint_const_lvalue)
#include "unord.map/spec/debug.insert.hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_insert_hint_rvalue)
#include "unord.map/spec/debug.insert.hint_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_dereference)
#include "unord.map/spec/debug.iterator.dereference.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_increment)
#include "unord.map/spec/debug.iterator.increment.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(spec_debug_local_iterator_dereference)
#include "unord.map/spec/debug.local_iterator.dereference.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(spec_debug_local_iterator_increment)
#include "unord.map/spec/debug.local_iterator.increment.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_swap)
#include "unord.map/spec/debug.swap.pass.cpp"
LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(cnstr_allocator)
#include "unord.map/unord.map.cnstr/allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_copy)
#include "unord.map/unord.map.cnstr/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_init)
#include "unord.map/unord.map.cnstr/assign_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_move)
#include "unord.map/unord.map.cnstr/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy)
#include "unord.map/unord.map.cnstr/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy_alloc)
#include "unord.map/unord.map.cnstr/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_DEFAULT_BUCKET

LIBCXX_TEST_BEGIN(cnstr_deduct)
#include "unord.map/unord.map.cnstr/deduct.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_deduct_const)
#include "unord.map/unord.map.cnstr/deduct_const.pass.cpp"
LIBCXX_TEST_END

#endif

LIBCXX_TEST_BEGIN(cnstr_default)
#include "unord.map/unord.map.cnstr/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_default_noexcept)
#include "unord.map/unord.map.cnstr/default_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_dtor_noexcept)
#include "unord.map/unord.map.cnstr/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init)
#include "unord.map/unord.map.cnstr/init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size)
#include "unord.map/unord.map.cnstr/init_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_allocator)
#include "unord.map/unord.map.cnstr/init_size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash)
#include "unord.map/unord.map.cnstr/init_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_allocator)
#include "unord.map/unord.map.cnstr/init_size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal)
#include "unord.map/unord.map.cnstr/init_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal_allocator)
#include "unord.map/unord.map.cnstr/init_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter)
#include "unord.map/unord.map.cnstr/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size)
#include "unord.map/unord.map.cnstr/iter_iter_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_allocator)
#include "unord.map/unord.map.cnstr/iter_iter_size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash)
#include "unord.map/unord.map.cnstr/iter_iter_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_allocator)
#include "unord.map/unord.map.cnstr/iter_iter_size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_equal)
#include "unord.map/unord.map.cnstr/iter_iter_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_equal_allocator)
#include "unord.map/unord.map.cnstr/iter_iter_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move)
#include "unord.map/unord.map.cnstr/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_alloc)
#include "unord.map/unord.map.cnstr/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_assign_noexcept)
#include "unord.map/unord.map.cnstr/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_noexcept)
#include "unord.map/unord.map.cnstr/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size)
#include "unord.map/unord.map.cnstr/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_allocator)
#include "unord.map/unord.map.cnstr/size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash)
#include "unord.map/unord.map.cnstr/size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_allocator)
#include "unord.map/unord.map.cnstr/size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal)
#include "unord.map/unord.map.cnstr/size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal_allocator)
#include "unord.map/unord.map.cnstr/size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(elem_at)
#include "unord.map/unord.map.elem/at.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(elem_index)
#include "unord.map/unord.map.elem/index.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_clear)
#include "unord.map/unord.map.modifiers/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace)
#include "unord.map/unord.map.modifiers/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_emplace_hint)
#include "unord.map/unord.map.modifiers/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_const_iter)
#include "unord.map/unord.map.modifiers/erase_const_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_key)
#include "unord.map/unord.map.modifiers/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_erase_range)
#include "unord.map/unord.map.modifiers/erase_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_iterator)
#include "unord.map/unord.map.modifiers/extract_iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_extract_key)
#include "unord.map/unord.map.modifiers/extract_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_and_emplace_allocator_requirements)
#include "unord.map/unord.map.modifiers/insert_and_emplace_allocator_requirements.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_const_lvalue)
#include "unord.map/unord.map.modifiers/insert_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_const_lvalue)
#include "unord.map/unord.map.modifiers/insert_hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_hint_rvalue)
#include "unord.map/unord.map.modifiers/insert_hint_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_init)
#include "unord.map/unord.map.modifiers/insert_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type)
#include "unord.map/unord.map.modifiers/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_node_type_hint)
#include "unord.map/unord.map.modifiers/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_or_assign)
#include "unord.map/unord.map.modifiers/insert_or_assign.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_range)
#include "unord.map/unord.map.modifiers/insert_range.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_insert_rvalue)
#include "unord.map/unord.map.modifiers/insert_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_merge)
#include "unord.map/unord.map.modifiers/merge.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(modifiers_try_emplace)
#include "unord.map/unord.map.modifiers/try.emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_noexcept)
#include "unord.map/unord.map.swap/swap_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_non_member)
#include "unord.map/unord.map.swap/swap_non_member.pass.cpp"
LIBCXX_TEST_END
