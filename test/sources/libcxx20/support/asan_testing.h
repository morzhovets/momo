//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Modified for https://github.com/morzhovets/momo project.
//
//===----------------------------------------------------------------------===//

#ifndef ASAN_TESTING_H
#define ASAN_TESTING_H

template<typename T>
bool is_contiguous_container_asan_correct(const T&) { return true; }

#endif // ASAN_TESTING_H
