#### momo (Memory Optimization is the Main Objective)

This project contains an implementation of the C++ containers, similar to the standard `set/map`
and `unordered_set/map`, but much more efficient in memory usage. As for the operation speed, these containers are also better than the standard ones in most cases.

Classes are designed in close conformity with the standard C++17 **including exception safety guarantees**.

#### Deviations from the standard

- Container items must be movable (preferably without exceptions) or copyable, similar to items of `std::vector`.

- All iterators and references to items will become invalid after each addition or removal of the item and should not be used.

- In `map` and `unordered_map` type `reference` is not the same as `value_type&`, so `for (auto& p : map)`
is illegal, but `for (auto p : map)` or `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.

#### The main ideas

- The implementation of `set/map` is based on a B-tree.

- `unordered_set/map` are hash tables with buckets in the form of small arrays.

- Memory pools are used to speed up the memory operations.

#### Usage

Just copy the folder `momo` in your source code. This folder contains only header files.

Classes `set/map` and `unordered_set/map` are located in subfolder `stdish`, namespace `momo::stdish`.

#### Other classes

- `stdish::unordered_set_open` and `stdish::unordered_map_open` are based on open addressing hash tables.

- `stdish::vector_intcap` is vector with internal capacity. This vector doesn't need dynamic memory while its size is not greater than user-defined constant.

- `stdish::pool_allocator` is allocator with a pool of memory for containers like `std::list`. Each copy of the container keeps its own memory pool. Memory is released not only after destruction of the object, but also in case of removal sufficient number of items.

- `stdish::unordered_multimap` is similar to `std::unordered_multimap`, but each of duplicate keys is stored only once.

- Folder `momo` also contains many of the analogous classes with non-standard interface, but more flexible, namely `HashSet`, `HashMap`, `HashMultiMap`, `TreeSet`, `TreeMap`, `Array`, `SegmentedArray`, `MemPool`.

#### Supported compilers

- MS Visual C++ (VS 2013+, internal version 12.0+)

- GCC (4.9+) with -std=c++11

- Clang (3.3+) with -std=c++11
