#pragma once

/**
 * Copyright © 2014-2019, Matthijs Möhlmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <sqlpp23/core/clause/where.h>
#include <sqlpp23/core/concepts.h>
#include <sqlpp23/core/detail/type_set.h>
#include <sqlpp23/core/tuple_to_sql_string.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/postgresql/database/serializer_context.h>
#include <sqlpp23/postgresql/reader.h>

namespace sqlpp {
namespace postgresql {
SQLPP_WRAPPED_STATIC_ASSERT(
    assert_no_unknown_tables_in_on_conflict_do_update_t,
    "at least one expression in on_conflict().do_update().where() requires a "
    "table which is otherwise not known in the statement");

SQLPP_WRAPPED_STATIC_ASSERT(
    assert_no_unknown_static_tables_in_on_conflict_do_update_t,
    "at least one expression in on_conflict().do_update().where() statically "
    "requires a table which is only known dynamically in the statement");

// ON CONFLICT ... DO UPDATE ... WHERE ...
template <typename OnConflictUpdate, typename Expression>
struct on_conflict_do_update_where_t {
  on_conflict_do_update_where_t(OnConflictUpdate on_conflict_update,
                                Expression expression)
      : _on_conflict_update(on_conflict_update), _expression(expression) {}
  on_conflict_do_update_where_t(const on_conflict_do_update_where_t&) = default;
  on_conflict_do_update_where_t(on_conflict_do_update_where_t&&) = default;
  on_conflict_do_update_where_t& operator=(
      const on_conflict_do_update_where_t&) = default;
  on_conflict_do_update_where_t& operator=(on_conflict_do_update_where_t&&) =
      default;
  ~on_conflict_do_update_where_t() = default;

 private:
  friend ::sqlpp::reader_t;
  friend ::sqlpp::postgresql::reader_t;
  OnConflictUpdate _on_conflict_update;
  Expression _expression;
};

template <typename OnConflictUpdate, typename Expression>
auto to_sql_string(
    postgresql::context_t& context,
    const on_conflict_do_update_where_t<OnConflictUpdate, Expression>& t)
    -> std::string {
  // Note: Temporary required to enforce parameter ordering.
  auto ret_val = to_sql_string(context, read.on_conflict_update(t)) + " WHERE ";
  return ret_val + to_sql_string(context, read.expression(t));
}

}  // namespace postgresql

template <typename OnConflictUpdate, typename Expression>
struct nodes_of<
    postgresql::on_conflict_do_update_where_t<OnConflictUpdate, Expression>> {
  using type = detail::type_vector<OnConflictUpdate, Expression>;
};

template <typename OnConflictUpdate, typename Expression>
struct is_clause<
    postgresql::on_conflict_do_update_where_t<OnConflictUpdate, Expression>>
    : public std::true_type {};

template <typename Statement, typename OnConflictUpdate, typename Expression>
struct consistency_check<
    Statement,
    postgresql::on_conflict_do_update_where_t<OnConflictUpdate, Expression>> {
  using type = consistent_t;
};

template <typename Statement, typename OnConflictUpdate, typename Expression>
struct prepare_check<
    Statement,
    postgresql::on_conflict_do_update_where_t<OnConflictUpdate, Expression>> {
  using type = static_combined_check_t<
      static_check_t<
          Statement::template _no_unknown_tables<
              postgresql::on_conflict_do_update_where_t<OnConflictUpdate,
                                                        Expression>>,
          postgresql::assert_no_unknown_tables_in_on_conflict_do_update_t>,
      static_check_t<
          Statement::template _no_unknown_static_tables<
              postgresql::on_conflict_do_update_where_t<OnConflictUpdate,
                                                        Expression>>,
          postgresql::
              assert_no_unknown_static_tables_in_on_conflict_do_update_t>>;
};

namespace postgresql {
// ON CONFLICT ... DO UPDATE ...
template <typename OnConflict, typename... Assignments>
struct on_conflict_do_update_t {
  on_conflict_do_update_t(OnConflict on_conflict,
                          std::tuple<Assignments...> assignments)
      : _on_conflict(std::move(on_conflict)),
        _assignments(std::move(assignments)) {}
  on_conflict_do_update_t(const on_conflict_do_update_t&) = default;
  on_conflict_do_update_t(on_conflict_do_update_t&&) = default;
  on_conflict_do_update_t& operator=(const on_conflict_do_update_t&) = default;
  on_conflict_do_update_t& operator=(on_conflict_do_update_t&&) = default;
  ~on_conflict_do_update_t() = default;

  template <typename Statement, DynamicBoolean Expression>
  auto where(this Statement&& statement, Expression expression) {
    SQLPP_STATIC_ASSERT(not contains_aggregate_function<Expression>::value,
                        "where() must not contain aggregate functions");

    auto new_clause =
        on_conflict_do_update_where_t<on_conflict_do_update_t, Expression>{
            statement, std::move(expression)};
    return new_statement<on_conflict_do_update_t>(
        std::forward<Statement>(statement), std::move(new_clause));
  }

 private:
  friend ::sqlpp::reader_t;
  friend ::sqlpp::postgresql::reader_t;
  OnConflict _on_conflict;
  std::tuple<Assignments...> _assignments;
};

template <typename OnConflict, typename... Assignments>
auto to_sql_string(postgresql::context_t& context,
                   const on_conflict_do_update_t<OnConflict, Assignments...>& t)
    -> std::string {
  return to_sql_string(context, read.on_conflict(t)) + " DO UPDATE SET " +
         tuple_to_sql_string(context, read.assignments(t),
                             tuple_operand_no_dynamic{", "});
}

}  // namespace postgresql

template <typename OnConflict, typename... Assignments>
struct is_clause<
    postgresql::on_conflict_do_update_t<OnConflict, Assignments...>>
    : public std::true_type {};

template <typename OnConflict, typename... Assignments>
struct nodes_of<
    postgresql::on_conflict_do_update_t<OnConflict, Assignments...>> {
  using type = detail::type_vector<OnConflict, Assignments...>;
};

template <typename Statement, typename OnConflict, typename... Assignments>
struct consistency_check<
    Statement,
    postgresql::on_conflict_do_update_t<OnConflict, Assignments...>> {
  using type = consistent_t;
};

template <typename Statement, typename OnConflict, typename... Assignments>
struct prepare_check<
    Statement,
    postgresql::on_conflict_do_update_t<OnConflict, Assignments...>> {
  using type = static_combined_check_t<
      static_check_t<
          Statement::template _no_unknown_tables<
              postgresql::on_conflict_do_update_t<OnConflict, Assignments...>>,
          postgresql::assert_no_unknown_tables_in_on_conflict_do_update_t>,
      static_check_t<
          Statement::template _no_unknown_static_tables<
              postgresql::on_conflict_do_update_t<OnConflict, Assignments...>>,
          postgresql::
              assert_no_unknown_static_tables_in_on_conflict_do_update_t>>;
};

}  // namespace sqlpp
