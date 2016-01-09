#### momo (Memory Optimization is the Main Objective)

This project contains an implementation of the C++ containers, similar to the standard `unordered_set/map`, but much more efficient in memory usage.
As for the operation speed, these containers are also better than the standard ones in most cases.

Classes are designed in close conformity with the standard C++ 11 **including exception safety guarantees**.

#### Deviations from the standard

- Container items must be movable (preferably without exceptions) or copyable, similar to items of `std::vector`.

- After each addition or removal of the item all iterators and references to items become invalid and should
not be used.

- In `unordered_map` type `reference` is not the same as `value_type&`, so `for (auto& p : map)`
is illegal, but `for (auto p : map)` or `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.

#### Usage

Just copy the folder `momo` (with License) in your source code.

Classes `unordered_set/map` located in subfolder `stdish`, namespace `momo::stdish`.

#### Other classes

- `stdish::unordered_set_open` and `stdish::unordered_map_open` are based on open addressing hash tables.

- `stdish::vector_intcap` is vector with internal capacity. This vector doesn't need dynamic memory while its size is not greater than user-defined constant.

- `stdish::unordered_multimap` are similar to `std::unordered_multimap`, but each of duplicate keys stored only once.

- Folder `momo` also contains many of the analogous classes with non-standard interface, but more flexible, namely `HashSet`, `HashMap`, `HashMultiMap`, `Array`, `SegmentedArray`, `MemPool`.

#### Supported compilers

- MS Visual Studio (2012+)

- GCC (4.7+), MinGW

- Clang (3.3+, possibly 3.1+)
