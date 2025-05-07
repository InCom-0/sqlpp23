#pragma once

/*
 * Copyright (c) 2013-2016, Roland Bock
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

#include <sqlpp23/core/basic/value.h>
#include <sqlpp23/core/clause/expression_static_check.h>
#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/query/dynamic.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Expression>
struct where_t {
  where_t(Expression expression) : _expression(std::move(expression)) {}
  where_t(const where_t&) = default;
  where_t(where_t&&) = default;
  where_t& operator=(const where_t&) = default;
  where_t& operator=(where_t&&) = default;
  ~where_t() = default;

 private:
  friend reader_t;
  Expression _expression;
};

template <typename Context, typename Expression>
auto to_sql_string(Context& context, const where_t<Expression>& t)
    -> std::string {
  return dynamic_clause_to_sql_string(context, "WHERE", read.expression(t));
}

class assert_no_unknown_tables_in_where_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one expression in where() requires a table "
                        "which is otherwise "
                        "not known in the statement");
  }
};

class assert_no_unknown_static_tables_in_where_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one expression in where() statically "
                        "requires a table which is "
                        "only known dynamically in the statement");
  }
};

template <typename Expression>
struct is_clause<where_t<Expression>> : public std::true_type {};

template <typename Statement, typename Expression>
struct consistency_check<Statement, where_t<Expression>> {
  using type = detail::expression_static_check_t<
      Statement,
      Expression,
      assert_no_unknown_static_tables_in_where_t>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Statement, typename Expression>
struct prepare_check<Statement, where_t<Expression>> {
  using type = static_combined_check_t<
      static_check_t<
          Statement::template _no_unknown_tables<where_t<Expression>>,
          assert_no_unknown_tables_in_where_t>,
      static_check_t<
          Statement::template _no_unknown_static_tables<where_t<Expression>>,
          assert_no_unknown_static_tables_in_where_t>>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Expression>
struct nodes_of<where_t<Expression>> {
  using type = detail::type_vector<Expression>;
};

// NO WHERE YET
struct no_where_t {
  template <typename Statement, DynamicBoolean Expression>
    requires(not contains_aggregate_function<Expression>::value)
  auto where(this Statement&& self, Expression expression) {

    return new_statement<no_where_t>(std::forward<Statement>(self),
                                     where_t<Expression>{expression});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_where_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_where_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <DynamicBoolean Expression>
  requires(not contains_aggregate_function<Expression>::value)
auto where(Expression expression) {
  return statement_t<no_where_t>().where(std::move(expression));
}
}  // namespace sqlpp
