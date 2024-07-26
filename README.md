[![MSVC](https://github.com/morzhovets/momo/actions/workflows/msvc.yml/badge.svg?branch=branch_cpp11)](https://github.com/morzhovets/momo/actions/workflows/msvc.yml?query=branch%3Abranch_cpp11)
[![GCC](https://github.com/morzhovets/momo/actions/workflows/gcc.yml/badge.svg?branch=branch_cpp11)](https://github.com/morzhovets/momo/actions/workflows/gcc.yml?query=branch%3Abranch_cpp11)
[![Clang](https://github.com/morzhovets/momo/actions/workflows/clang.yml/badge.svg?branch=branch_cpp11)](https://github.com/morzhovets/momo/actions/workflows/clang.yml?query=branch%3Abranch_cpp11)
[![AppleClang](https://github.com/morzhovets/momo/actions/workflows/apple_clang.yml/badge.svg?branch=branch_cpp11)](https://github.com/morzhovets/momo/actions/workflows/apple_clang.yml?query=branch%3Abranch_cpp11)
[![IntelLLVM](https://github.com/morzhovets/momo/actions/workflows/intel.yml/badge.svg?branch=branch_cpp11)](https://github.com/morzhovets/momo/actions/workflows/intel.yml?query=branch%3Abranch_cpp11)

#### momo (Memory Optimization is the Main Objective)

This project contains an implementation of the C++ containers, similar to the
[standard](https://en.cppreference.com/w/cpp/container) `set/map` and `unordered_set/map`,
but much more efficient in memory usage.
As for the operation speed, these containers are also better than the standard ones in most cases
([benchmark of unordered containers](https://morzhovets.github.io/hash_gcc_ubuntu16),
[benchmark of ordered containers](https://morzhovets.github.io/tree_gcc_ubuntu16),
[benchmark sources](https://github.com/morzhovets/hash-table-shootout)).

Classes are designed in close conformity with the standard C++20 including exception safety guarantees.
**The compiler only needs to support C++11**.

#### Deviations from the standard

- Container items must be movable (preferably without exceptions) or copyable, similar to items of `std::vector`.

- All iterators and references to items will become invalid after each addition or removal of the item and should not be used.

- In `map` and `unordered_map` type `reference` is a pair of references
(same as in [std::flat_map](https://en.cppreference.com/w/cpp/container/flat_map)),
so `for (auto& p : map)` is illegal, but `for (auto p : map)` or `for (const auto& p : map)`
or `for (auto&& p : map)` is allowed.

#### The main ideas

- The implementation of `set/map` is based on a B-tree.

- `unordered_set/map` are hash tables with buckets in the form of small arrays.

- Memory pools are used to speed up the memory operations.

#### Usage

This library is **header-only** and has zero dependencies.
So you can just copy the folder [include/momo](https://github.com/morzhovets/momo/tree/branch_cpp11/include/momo)
into your source code.

Classes `set/map` and `unordered_set/map` are located in subfolder
[stdish](https://github.com/morzhovets/momo/tree/branch_cpp11/include/momo/stdish), namespace `momo::stdish`.

Some documentation is [here](https://morzhovets.github.io/momo).

#### Other classes

- `stdish::unordered_set_open` and `stdish::unordered_map_open` are based on open addressing hash tables.

- `stdish::vector_intcap` is vector with internal capacity.
This vector doesn't need dynamic memory while its size is not greater than user-defined constant.

- `stdish::unordered_multimap` is similar to `std::unordered_multimap`, but each of duplicate keys is stored only once.

- `stdish::multiset` and `stdish::multimap` are similar to `std::multiset` and `std::multimap`.

- `stdish::unsynchronized_pool_allocator` is allocator with a pool of memory for containers like `std::list` or `std::map`.
Each copy of the container keeps its own memory pool.
Memory is released not only after destruction of the object, but also in case of removal sufficient number of items.

- Folder `momo` also contains many of the analogous classes with non-standard interface, but more flexible,
namely `HashSet`, `HashMap`, `HashMultiMap`, `TreeSet`, `TreeMap`, `Array`, `SegmentedArray`, `MemPool`.

#### DataTable

`momo::DataTable` is similar to [`Boost.MultiIndex`](https://www.boost.org/doc/libs/1_74_0/libs/multi_index/doc/index.html),
but its API looks like [`ADO.NET DataTable`](https://docs.microsoft.com/en-us/dotnet/api/system.data.datatable).
Some examples are [here](https://github.com/morzhovets/momo/blob/branch_cpp11/test/sources/DataSamples.cpp).

#### Supported compilers

- MS Visual C++ (19.0+, Visual Studio 2015+)

- GCC (4.9+)

- Clang (3.6+)

- Apple Clang (8.1+, Xcode 8.3+)

- Intel C++
