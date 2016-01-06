momo (Memory Optimization is the Main Objective)
------------------------------------------------

This project contains an implementation of the C++ containers, similar to the standard `set/map`
and `unordered_set/map`, but much more efficient in memory usage.
As for the operation speed, these containers are also better than the standard ones in most cases.

Classes are designed in close conformity with the standard C++ 11 **including exception safety guarantees**.

Deviations from the standard
----------------------------

- Container items must be movable (preferably without exceptions) or copyable, similar to items of `std::vector`.

- After each addition or removal of the item all iterators and references to items become invalid and should
not be used.

- In `map` and `unordered_map` type `reference` is not the same as `value_type&`, so `for (auto& p : map)`
is illegal, but `for (auto p : map)` or `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.

The main ideas
--------------

- The implementation of `set/map` is based on a b-tree.

- `unordered_set/map` are hash tables with buckets in the form of small arrays.

- Memory pools are used to speed up the memory operations.

Usage
-----

Just copy the folder `momo` (with License) in your source code.

Classes `set/map` and `unordered_set/map` located in subfolder `stdish`, namespace `momo::stdish`.

Supported compilers
-------------------

- MS Visual Studio (2013+)

- GCC (4.9+, possibly 4.8+), MinGW

- Clang (3.3+, possibly 3.1+)
