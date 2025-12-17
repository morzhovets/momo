/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/pch.h

\**********************************************************/

#pragma once

#include "TestSettings.h"

#include <cstddef>
#include <memory>
#include <algorithm>
#include <functional>
#include <utility>
#include <new>
#include <iterator>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <cstdint>
#include <array>
#include <tuple>
#include <initializer_list>
#include <cstdlib>
#include <atomic>

#ifndef MOMO_DISABLE_TYPE_INFO
# include <typeinfo>
#endif

#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
# include <string_view>
#endif

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
# include <compare>
#endif

#ifdef MOMO_HAS_CONTAINERS_RANGES
# include <ranges>
#endif

#ifdef MOMO_USE_MEM_MANAGER_WIN
# include <Windows.h>
#endif

#ifdef MOMO_USE_SSE2
# include <emmintrin.h>
#endif

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <limits>
#include <climits>
#include <cfloat>
#include <random>
