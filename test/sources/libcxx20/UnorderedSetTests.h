/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx20/UnorderedSetTests.h

\**********************************************************/

#if !defined(LIBCXX_TEST_MERGE_SET)

#if !defined(LIBCXX_TEST_HASH_LIST_SET)
LIBCXX_TEST_BEGIN(bucket)
#include "unord.set/bucket.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(bucket_count)
#include "unord.set/bucket_count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(bucket_size)
#include "unord.set/bucket_size.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_MERGE_SET

LIBCXX_TEST_BEGIN(clear)
#include "unord.set/clear.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains)
#include "unord.set/contains.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(contains_transparent)
#include "unord.set/contains.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count)
#include "unord.set/count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(count_transparent)
#include "unord.set/count.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace)
#include "unord.set/emplace.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(emplace_hint)
#include "unord.set/emplace_hint.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(empty)
#include "unord.set/empty.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq_different_hash)
#include "unord.set/eq.different_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(eq)
#include "unord.set/eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_transparent)
#include "unord.set/equal_range.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_const)
#include "unord.set/equal_range_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(equal_range_non_const)
#include "unord.set/equal_range_non_const.pass.cpp"
LIBCXX_TEST_END

#if !defined(LIBCXX_TEST_MERGE_SET) || defined(LIBCXX_TEST_MERGE_HASH)

LIBCXX_TEST_BEGIN(erase_const_iter)
#include "unord.set/erase_const_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_if_)
#include "unord.set/erase_if.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_key)
#include "unord.set/erase_key.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(erase_range)
#include "unord.set/erase_range.pass.cpp"
LIBCXX_TEST_END

#if !defined(LIBCXX_TEST_HASH_LIST_SET)

LIBCXX_TEST_BEGIN(extract_iterator)
#include "unord.set/extract_iterator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(extract_key)
#include "unord.set/extract_key.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_HASH_LIST_SET

#endif // !defined(LIBCXX_TEST_MERGE_SET) || defined(LIBCXX_TEST_MERGE_HASH)

LIBCXX_TEST_BEGIN(find_transparent)
#include "unord.set/find.transparent.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_const)
#include "unord.set/find_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(find_non_const)
#include "unord.set/find_non_const.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(get_allocator)
#include "unord.set/get_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(hash_function)
#include "unord.set/hash_function.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(incomplete)
#include "unord.set/incomplete.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(insert_and_emplace_allocator_requirements)
#include "unord.set/insert_and_emplace_allocator_requirements.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_const_lvalue)
#include "unord.set/insert_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_hint_const_lvalue)
#include "unord.set/insert_hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_hint_rvalue)
#include "unord.set/insert_hint_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_init)
#include "unord.set/insert_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_iter_iter)
#include "unord.set/insert_iter_iter.pass.cpp"
LIBCXX_TEST_END

#if (!defined(LIBCXX_TEST_MERGE_SET) || defined(LIBCXX_TEST_MERGE_HASH)) && !defined(LIBCXX_TEST_HASH_LIST_SET)

LIBCXX_TEST_BEGIN(insert_node_type)
#include "unord.set/insert_node_type.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(insert_node_type_hint)
#include "unord.set/insert_node_type_hint.pass.cpp"
LIBCXX_TEST_END

#endif // (!defined(LIBCXX_TEST_MERGE_SET) || defined(LIBCXX_TEST_MERGE_HASH)) && !defined(LIBCXX_TEST_HASH_LIST_SET)

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(insert_range)
#include "unord.set/insert_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(insert_rvalue)
#include "unord.set/insert_rvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterator_concept_conformance)
#include "unord.set/iterator_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(iterators)
#include "unord.set/iterators.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(key_eq)
#include "unord.set/key_eq.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(load_factor)
#include "unord.set/load_factor.pass.cpp"
LIBCXX_TEST_END

#if !defined(LIBCXX_TEST_MERGE_SET) && !defined(LIBCXX_TEST_HASH_LIST_SET)
LIBCXX_TEST_BEGIN(local_iterators)
#include "unord.set/local_iterators.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(max_bucket_count)
#include "unord.set/max_bucket_count.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_load_factor)
#include "unord.set/max_load_factor.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(max_size)
#include "unord.set/max_size.pass.cpp"
LIBCXX_TEST_END

#if !defined(LIBCXX_TEST_MERGE_SET) || defined(LIBCXX_TEST_MERGE_HASH)
LIBCXX_TEST_BEGIN(merge)
#include "unord.set/merge.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(range_concept_conformance)
#include "unord.set/range_concept_conformance.compile.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(rehash)
#include "unord.set/rehash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(reserve)
#include "unord.set/reserve.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(size)
#include "unord.set/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_member)
#include "unord.set/swap_member.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(types)
#include "unord.set/types.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(common_iterator_difference_type)
#include "unord.set/common/iterator_difference_type.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(extra_insert_range_ext)
#include "unord.set/extra/insert_range_ext.pass.cpp"
LIBCXX_TEST_END
#endif

#ifndef TEST_HAS_NO_EXCEPTIONS
#ifdef LIBCXX_TEST_FAILURE

#if !defined(LIBCXX_TEST_MERGE_SET)

LIBCXX_TEST_BEGIN(spec_assert_bucket)
#include "unord.set/spec/assert.bucket.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_assert_bucket_size)
#include "unord.set/spec/assert.bucket_size.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_MERGE_SET

LIBCXX_TEST_BEGIN(spec_assert_max_load_factor)
#include "unord.set/spec/assert.max_load_factor.pass.cpp"
LIBCXX_TEST_END

#if !defined(LIBCXX_TEST_MERGE_SET)

LIBCXX_TEST_BEGIN(spec_debug_erase_iter)
#include "unord.set/spec/debug.erase.iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_erase_iter_iter)
#include "unord.set/spec/debug.erase.iter_iter.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_MERGE_SET

LIBCXX_TEST_BEGIN(spec_debug_insert_hint_const_lvalue)
#include "unord.set/spec/debug.insert.hint_const_lvalue.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_dereference)
#include "unord.set/spec/debug.iterator.dereference.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_iterator_increment)
#include "unord.set/spec/debug.iterator.increment.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
LIBCXX_TEST_BEGIN(spec_debug_local_iterator_dereference)
#include "unord.set/spec/debug.local_iterator.dereference.pass.cpp"
LIBCXX_TEST_END
#endif

#if !defined(LIBCXX_TEST_MERGE_SET)

LIBCXX_TEST_BEGIN(spec_debug_local_iterator_increment)
#include "unord.set/spec/debug.local_iterator.increment.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(spec_debug_swap)
#include "unord.set/spec/debug.swap.pass.cpp"
LIBCXX_TEST_END

#endif // LIBCXX_TEST_MERGE_SET

#endif // LIBCXX_TEST_FAILURE
#endif // TEST_HAS_NO_EXCEPTIONS

LIBCXX_TEST_BEGIN(cnstr_allocator)
#include "unord.set/unord.set.cnstr/allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_copy)
#include "unord.set/unord.set.cnstr/assign_copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_init)
#include "unord.set/unord.set.cnstr/assign_init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_assign_move)
#include "unord.set/unord.set.cnstr/assign_move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy)
#include "unord.set/unord.set.cnstr/copy.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_copy_alloc)
#include "unord.set/unord.set.cnstr/copy_alloc.pass.cpp"
LIBCXX_TEST_END

#ifdef LIBCXX_TEST_CLASS
LIBCXX_TEST_BEGIN(cnstr_deduct)
#include "unord.set/unord.set.cnstr/deduct.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cnstr_default)
#include "unord.set/unord.set.cnstr/default.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_default_noexcept)
#include "unord.set/unord.set.cnstr/default_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_dtor_noexcept)
#include "unord.set/unord.set.cnstr/dtor_noexcept.pass.cpp"
LIBCXX_TEST_END

#if TEST_STD_VER >= 23
LIBCXX_TEST_BEGIN(cnstr_from_range)
#include "unord.set/unord.set.cnstr/from_range.pass.cpp"
LIBCXX_TEST_END
#endif

LIBCXX_TEST_BEGIN(cnstr_init)
#include "unord.set/unord.set.cnstr/init.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size)
#include "unord.set/unord.set.cnstr/init_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_allocator)
#include "unord.set/unord.set.cnstr/init_size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash)
#include "unord.set/unord.set.cnstr/init_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_allocator)
#include "unord.set/unord.set.cnstr/init_size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal)
#include "unord.set/unord.set.cnstr/init_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_init_size_hash_equal_allocator)
#include "unord.set/unord.set.cnstr/init_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter)
#include "unord.set/unord.set.cnstr/iter_iter.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size)
#include "unord.set/unord.set.cnstr/iter_iter_size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_allocator)
#include "unord.set/unord.set.cnstr/iter_iter_size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash)
#include "unord.set/unord.set.cnstr/iter_iter_size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_allocator)
#include "unord.set/unord.set.cnstr/iter_iter_size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_equal)
#include "unord.set/unord.set.cnstr/iter_iter_size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_iter_iter_size_hash_equal_allocator)
#include "unord.set/unord.set.cnstr/iter_iter_size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move)
#include "unord.set/unord.set.cnstr/move.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_alloc)
#include "unord.set/unord.set.cnstr/move_alloc.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_assign_noexcept)
#include "unord.set/unord.set.cnstr/move_assign_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_move_noexcept)
#include "unord.set/unord.set.cnstr/move_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size)
#include "unord.set/unord.set.cnstr/size.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_allocator)
#include "unord.set/unord.set.cnstr/size_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash)
#include "unord.set/unord.set.cnstr/size_hash.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_allocator)
#include "unord.set/unord.set.cnstr/size_hash_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal)
#include "unord.set/unord.set.cnstr/size_hash_equal.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(cnstr_size_hash_equal_allocator)
#include "unord.set/unord.set.cnstr/size_hash_equal_allocator.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_noexcept)
#include "unord.set/unord.set.swap/swap_noexcept.pass.cpp"
LIBCXX_TEST_END

LIBCXX_TEST_BEGIN(swap_swap_non_member)
#include "unord.set/unord.set.swap/swap_non_member.pass.cpp"
LIBCXX_TEST_END
