#pragma once

/*
 * Copyright (c) 2024, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdio>
#include <print>

#ifdef BUILD_WITH_MODULES
import sqlpp23.core;
#else
#include <sqlpp23/sqlpp23.h>
#endif


template<typename T>
struct std::formatter<std::optional<T>, char>
{
  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext& ctx) {
    auto it = ctx.begin();
    if (it == ctx.end())
      return it;

    if (it != ctx.end() && *it != '}')
      throw std::format_error("parser not implemented for std::optional<T>");

    return it;
  }

  template <class FmtContext>
  FmtContext::iterator format(std::optional<T> t, FmtContext& ctx) const {
    if (not t) {
      return std::ranges::copy("NULL", ctx.out()).out;
    }

    return std::ranges::copy(std::format("{}", t.value()), ctx.out()).out;
  }
};

inline std::string_view isolation_level_to_string(
    const sqlpp::isolation_level& level) {
  switch (level) {
    case sqlpp::isolation_level::serializable: {
      return "SERIALIZABLE";
    }
    case sqlpp::isolation_level::repeatable_read: {
      return "REPEATABLE READ";
    }
    case sqlpp::isolation_level::read_committed: {
      return "READ COMMITTED";
    }
    case sqlpp::isolation_level::read_uncommitted: {
      return "READ UNCOMMITTED";
    }
    case sqlpp::isolation_level::undefined: {
      return "BEGIN";
    }
  }
}

template<>
struct std::formatter<sqlpp::isolation_level, char>
{
  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext& ctx) {
    auto it = ctx.begin();
    if (it == ctx.end())
      return it;

    if (it != ctx.end() && *it != '}')
      throw std::format_error(
          "parser not implemented for sqlpp::isolation_level");

    return it;
  }

  template <class FmtContext>
  FmtContext::iterator format(sqlpp::isolation_level t, FmtContext& ctx) const {
    return std::ranges::copy(isolation_level_to_string(t), ctx.out()).out;
  }
};


template <typename L, typename R>
auto require_equal(int line, const L& l, const R& r) -> void {
  if (l != r) {
    std::println("{}: {} != {}", line, l, r);
    throw std::runtime_error("Unexpected result");
  }
}
