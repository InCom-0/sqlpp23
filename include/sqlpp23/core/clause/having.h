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
#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/logic.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/reader.h>
#include <sqlpp23/core/to_sql_string.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename Expression>
struct having_t {
  having_t(Expression expression) : _expression(std::move(expression)) {}

  having_t(const having_t&) = default;
  having_t(having_t&&) = default;
  having_t& operator=(const having_t&) = default;
  having_t& operator=(having_t&&) = default;
  ~having_t() = default;

 private:
  friend reader_t;
  Expression _expression;
};

template <typename Context, typename Expression>
auto to_sql_string(Context& context, const having_t<Expression>& t)
    -> std::string {
  return dynamic_clause_to_sql_string(context, "HAVING", read.expression(t));
}

class assert_no_unknown_tables_in_having_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one having-expression requires a table "
                        "which is otherwise not known in the statement");
  }
};

class assert_no_unknown_static_tables_in_having_t
    : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one having-expression statically requires a "
                        "table which is only "
                        "known dynamically in the statement");
  }
};

class assert_having_all_aggregates_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(
        wrong<T...>,
        "having expression not built out of aggregate expressions");
  }
};

class assert_having_all_static_aggregates_t : public wrapped_static_assert {
 public:
  template <typename... T>
  static void verify(T&&...) {
    static_assert(wrong<T...>,
                        "at least one static having expression is provided "
                        "dynamically only in group_by");
  }
};

template <typename Expression>
struct is_clause<having_t<Expression>> : public std::true_type {};

template <typename Expression>
struct nodes_of<having_t<Expression>> {
  using type = detail::type_vector<Expression>;
};

template <typename Statement, typename Expression>
struct consistency_check<Statement, having_t<Expression>> {
  using type = static_combined_check_t<
      static_check_t<
          is_aggregate_expression<typename Statement::_all_provided_aggregates,
                                  Expression>::value,
          assert_having_all_aggregates_t>,
      static_check_t<static_part_is_aggregate_expression<
                         typename Statement::_all_provided_static_aggregates,
                         Expression>::value,
                     assert_having_all_static_aggregates_t>,
      detail::expression_static_check_t<
          Statement,
          Expression,
          assert_no_unknown_static_tables_in_having_t>>;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename Statement, typename Expression>
struct prepare_check<Statement, having_t<Expression>> {
  using type = static_combined_check_t<
      static_check_t<
          Statement::template _no_unknown_tables<having_t<Expression>>,
          assert_no_unknown_tables_in_having_t>,
      static_check_t<
          Statement::template _no_unknown_static_tables<having_t<Expression>>,
          assert_no_unknown_static_tables_in_having_t>>;
  constexpr auto operator()() {
    return type{};
  }
};

// NO HAVING YET
struct no_having_t {
  template <typename Statement, DynamicBoolean Expression>
  auto having(this Statement&& self, Expression expression) {
    return new_statement<no_having_t>(std::forward<Statement>(self),
                                      having_t<Expression>{expression});
  }
};

template <typename Context>
auto to_sql_string(Context&, const no_having_t&) -> std::string {
  return "";
}

template <typename Statement>
struct consistency_check<Statement, no_having_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <typename T>
auto having(T&& t)
    -> decltype(statement_t<no_having_t>().having(std::forward<T>(t))) {
  return statement_t<no_having_t>().having(std::forward<T>(t));
}

}  // namespace sqlpp
