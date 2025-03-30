#pragma once

/*
 * Copyright (c) 2024, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
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

#include <tuple>
#include <utility>

#include <sqlpp23/core/operator/as_expression.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
struct tuple_operand {
  template <typename Context, typename T>
  auto operator()(Context& context, const T& t, size_t index) const
      -> std::string {
    const auto prefix = index ? std::string{separator} : std::string{};
    return prefix + operand_to_sql_string(context, t);
  }

  template <typename Context, typename T>
  auto operator()(Context& context, const dynamic_t<T>& t, size_t index) const
      -> std::string {
    if (t.has_value()) {
      return operator()(context, t.value(), index);
    }
    return operator()(context, std::nullopt, index);
  }

  std::string_view separator;
};

// Used to serialize tuple that should ignore dynamic elements.
struct tuple_operand_no_dynamic {
  template <typename Context, typename T>
  auto operator()(Context& context, const T& t, size_t) const -> std::string {
    const auto prefix = need_prefix ? std::string{separator} : std::string{};
    need_prefix = true;
    return prefix + operand_to_sql_string(context, t);
  }

  template <typename Context, typename T>
  auto operator()(Context& context,
                  const sqlpp::dynamic_t<T>& t,
                  size_t index) const -> std::string {
    if (t.has_value()) {
      return operator()(context, t.value(), index);
    }
    return "";
  }

  std::string_view separator;
  mutable bool need_prefix = false;
};

// Used to serialize select columns.
// In particular, it serializes unselected dynamic columns as "NULL AS <name>".
struct tuple_operand_select_column {
  template <typename Context, typename T>
  auto operator()(Context& context, const T& t, size_t index) const
      -> std::string {
    const auto prefix = index ? std::string{separator} : std::string{};
    return prefix + operand_to_sql_string(context, t);
  }

  template <typename Context, typename T, typename NameTag>
  auto operator()(Context& context,
                  const sqlpp::dynamic_t<as_expression<T, NameTag>>& t,
                  size_t index) const -> std::string {
    if (t.has_value()) {
      return operator()(context, t.value(), index);
    }
    return operator()(
        context, as_expression<std::nullopt_t, NameTag>{std::nullopt}, index);
  }

  template <typename Context, typename T>
  auto operator()(Context& context,
                  const sqlpp::dynamic_t<T>& t,
                  size_t index) const -> std::string {
    if (t.has_value()) {
      return operator()(context, t.value(), index);
    }
    static_assert(has_name_tag<T>::value, "select columns have to have a name");
    return operator()(
        context, as_expression<std::nullopt_t, name_tag_of_t<T>>{std::nullopt},
        index);
  }

  std::string_view separator;
};

// Used to names (ignoring dynamic)
struct tuple_operand_name_no_dynamic {
  template <typename Context, typename T>
  auto operator()(Context& context, const T&, size_t) const -> std::string {
    const auto prefix = need_prefix ? std::string{separator} : std::string{};
    need_prefix = true;
    return prefix + name_to_sql_string(context, name_tag_of_t<T>{});
  }

  template <typename Context, typename T>
  auto operator()(Context& context,
                  const sqlpp::dynamic_t<T>& t,
                  size_t index) const -> std::string {
    if (t.has_value()) {
      return operator()(context, t.value(), index);
    }
    return "";
  }

  std::string_view separator;
  mutable bool need_prefix = false;
};

struct tuple_clause {
  template <typename Context, typename T>
  auto operator()(Context& context, const T& t, size_t index) const
      -> std::string {
    const auto prefix = index ? std::string{separator} : std::string{};
    return prefix + to_sql_string(context, t);
  }

  std::string_view separator;
};

template <typename Context, typename Tuple, typename Strategy, size_t... Is>
auto tuple_to_sql_string_impl(Context& context,
                              const Tuple& t,
                              const Strategy& strategy,
                              const std::index_sequence<Is...>&
                              /*unused*/) -> std::string {
  // See https://en.cppreference.com/w/cpp/language/eval_order
  auto result = std::string{};
  ((result += strategy(context, std::get<Is>(t), Is)), ...);
  return result;
}

template <typename Context, typename Tuple, typename Strategy>
auto tuple_to_sql_string(Context& context,
                         const Tuple& t,
                         const Strategy& strategy) -> std::string {
  return tuple_to_sql_string_impl(
      context, t, strategy,
      std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}

template <typename Context, typename... Expressions>
auto dynamic_tuple_clause_to_sql_string(Context& context,
                                        std::string_view name,
                                        const std::tuple<Expressions...>& data)
    -> std::string {
  const auto expressions =
      tuple_to_sql_string(context, data, tuple_operand_no_dynamic{", "});

  if (expressions.empty()) {
    return "";
  }

  return std::format(" {} {}", name, expressions);
}

}  // namespace sqlpp
