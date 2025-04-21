#pragma once

/*
Copyright (c) 2018, Roland Bock
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <utility>

#include <sqlpp23/core/noop.h>
#include <sqlpp23/core/operator/any.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename L, typename Operator, typename R>
struct comparison_expression
    : public enable_as {
  constexpr comparison_expression(L l, R r)
      : _l(std::move(l)), _r(std::move(r)) {}
  comparison_expression(const comparison_expression&) = default;
  comparison_expression(comparison_expression&&) = default;
  comparison_expression& operator=(const comparison_expression&) = default;
  comparison_expression& operator=(comparison_expression&&) = default;
  ~comparison_expression() = default;

  L _l;
  R _r;
};

template <typename L, typename Operator, typename R>
struct data_type_of<comparison_expression<L, Operator, R>>
    : std::conditional<
          sqlpp::is_optional<data_type_of_t<L>>::value or
              sqlpp::is_optional<data_type_of_t<remove_any_t<R>>>::value,
          std::optional<boolean>,
          boolean> {};

struct op_is_null;
struct op_is_not_null;
struct op_is_distinct_from;
struct op_is_not_distinct_from;

template <typename L>
struct data_type_of<comparison_expression<L, op_is_null, std::nullopt_t>> {
  using type = boolean;
};

template <typename L>
struct data_type_of<comparison_expression<L, op_is_not_null, std::nullopt_t>> {
  using type = boolean;
};

template <typename L, typename R>
struct data_type_of<comparison_expression<L, op_is_distinct_from, R>> {
  using type = boolean;
};

template <typename L, typename R>
struct data_type_of<comparison_expression<L, op_is_not_distinct_from, R>> {
  using type = boolean;
};

template <typename L, typename Operator, typename R>
struct nodes_of<comparison_expression<L, Operator, R>> {
  using type = detail::type_vector<L, R>;
};

template <typename L, typename Operator, typename R>
struct requires_parentheses<comparison_expression<L, Operator, R>>
    : public std::true_type {};

template <typename Context, typename L, typename Operator, typename R>
auto to_sql_string(Context& context,
                   const comparison_expression<L, Operator, R>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = operand_to_sql_string(context, t._l) + Operator::symbol;
  return ret_val + operand_to_sql_string(context, t._r);
}

struct less {
  static constexpr auto symbol = " < ";
};

// We are using remove_any_t in the basic comparison operators to allow
// comparison with ANY-expressions. Note: any_t does not have a specialization
// for data_type_of to disallow it from being used in other contexts.
template <typename L, typename R>
  requires(values_are_comparable<L, remove_any_t<R>>::value)
constexpr auto operator<(L l, R r) -> comparison_expression<L, less, R> {
  return {std::move(l), std::move(r)};
}

struct less_equal {
  static constexpr auto symbol = " <= ";
};

template <typename L, typename R>
  requires(values_are_comparable<L, remove_any_t<R>>::value)
constexpr auto operator<=(L l, R r) -> comparison_expression<L, less_equal, R> {
  return {std::move(l), std::move(r)};
}

struct equal_to {
  static constexpr auto symbol = " = ";
};

template <typename L, typename R>
  requires(values_are_comparable<L, remove_any_t<R>>::value)
constexpr auto operator==(L l, R r) -> comparison_expression<L, equal_to, R> {
  return {std::move(l), std::move(r)};
}

struct not_equal_to {
  static constexpr auto symbol = " <> ";
};

template <typename L, typename R>
  requires(values_are_comparable<L, remove_any_t<R>>::value)
constexpr auto operator!=(L l, R r)
    -> comparison_expression<L, not_equal_to, R> {
  return {std::move(l), std::move(r)};
}

struct greater_equal {
  static constexpr auto symbol = " >= ";
};

template <typename L, typename R>
  requires(values_are_comparable<L, remove_any_t<R>>::value)
constexpr auto operator>=(L l, R r)
    -> comparison_expression<L, greater_equal, R> {
  return {std::move(l), std::move(r)};
}

struct greater {
  static constexpr auto symbol = " > ";
};

template <typename L, typename R>
  requires(values_are_comparable<L, remove_any_t<R>>::value)
constexpr auto operator>(L l, R r) -> comparison_expression<L, greater, R> {
  return {std::move(l), std::move(r)};
}

struct op_is_null {
  static constexpr auto symbol = " IS ";
};

template <typename L>
constexpr auto is_null(L l)
    -> comparison_expression<L, op_is_null, std::nullopt_t> {
  return {l, std::nullopt};
}

struct op_is_not_null {
  static constexpr auto symbol = " IS NOT ";
};

template <typename L>
constexpr auto is_not_null(L l)
    -> comparison_expression<L, op_is_not_null, std::nullopt_t> {
  return {l, std::nullopt};
}

struct op_is_distinct_from {
  static constexpr auto symbol = " IS DISTINCT FROM ";  // sql standard
  // mysql has NULL-safe equal `<=>` which is_null equivalent to `IS NOT
  // DISTINCT FROM` sqlite3 has `IS NOT`
};

template <typename L, typename R>
  requires(values_are_comparable<L, R>::value)
constexpr auto is_distinct_from(L l, R r)
    -> comparison_expression<L, op_is_distinct_from, R> {
  return {l, r};
}

struct op_is_not_distinct_from {
  static constexpr auto symbol = " IS NOT DISTINCT FROM ";  // sql standard
  // mysql has NULL-safe equal `<=>`
  // sqlite3 has `IS`
};

template <typename L, typename R>
  requires(values_are_comparable<L, R>::value)
constexpr auto is_not_distinct_from(L l, R r)
    -> comparison_expression<L, op_is_not_distinct_from, R> {
  return {l, r};
}

struct op_like {
  static constexpr auto symbol = " LIKE ";
};

template <typename L, typename R>
  requires(is_text<L>::value and is_text<R>::value)
constexpr auto like(L l, R r) -> comparison_expression<L, op_like, R> {
  return {std::move(l), std::move(r)};
}

}  // namespace sqlpp
