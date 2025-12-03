/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/pch.h

\**********************************************************/

#pragma once

#include "TestSettings.h"

#include <version>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <utility>
#include <new>
#include <iterator>
#include <type_traits>
#include <array>
#include <tuple>
#include <initializer_list>
#include <optional>
#include <concepts>
#include <compare>
#include <bit>
#include <ranges>
#include <atomic>
#include <string_view>

#ifndef MOMO_DISABLE_TYPE_INFO
# include <typeinfo>
#endif

#ifdef MOMO_USE_MEM_MANAGER_WIN
# include <Windows.h>
#endif

#ifdef MOMO_USE_SSE2
# include <emmintrin.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <cerrno>
#include <cmath>
#include <limits>
#include <climits>
#include <cfloat>
#include <random>
#include <span>
