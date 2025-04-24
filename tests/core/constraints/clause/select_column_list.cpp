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

#include <sqlpp23/tests/core/constraints_helpers.h>

#include <sqlpp23/tests/core/tables.h>

namespace {
SQLPP_CREATE_NAME_TAG(something);

template <typename... Expressions>
concept can_call_select_columns_with_standalone = requires(
    Expressions... expressions) { sqlpp::select_columns(expressions...); };
template <typename... Expressions>
concept can_call_select_columns_with_in_statement = requires(
    Expressions... expressions) {
  sqlpp::statement_t<sqlpp::no_select_column_list_t>{}.columns(expressions...);
};

template <typename... Expressions>
concept can_call_select_columns_with =
    can_call_select_columns_with_standalone<Expressions...> and
    can_call_select_columns_with_in_statement<Expressions...>;

template <typename... Expressions>
concept cannot_call_select_columns_with =
    not(can_call_select_columns_with_standalone<Expressions...> or
        can_call_select_columns_with_in_statement<Expressions...>);
}  // namespace

namespace test {
SQLPP_CREATE_NAME_TAG(max_id);
}

int main() {
  const auto foo = test::TabFoo{};
  const auto bar = test::TabBar{};

  // Confirming the required columns of TabBar.
  static_assert(std::is_same<sqlpp::required_insert_columns_of_t<test::TabBar>,
                             sqlpp::detail::type_set<sqlpp::column_t<
                                 test::TabBar, test::TabBar_::BoolNn>>>::value,
                "");

  // -------------------------
  // select() can be constructed, but is inconsistent since not columns are
  // selected.
  // -------------------------
  {
    auto s = sqlpp::select();
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::assert_columns_selected_t>::value,
                  "");
  }

  // -------------------------
  // select().columns(...)
  // -------------------------

  // select_columns(<non arguments>) is inconsistent and cannot be constructed.
  SQLPP_CHECK_STATIC_ASSERT(sqlpp::select_columns(),
                            "at least one selected column required");

  // select_columns(<arguments with no value>) cannot be called.
  static_assert(can_call_select_columns_with<decltype(bar.boolNn)>,
                "OK, argument a column");
  static_assert(
      can_call_select_columns_with<decltype(dynamic(true, bar.boolNn))>,
      "OK, argument a column");
  static_assert(cannot_call_select_columns_with<decltype(bar.id == 7)>,
                "not a value: comparison");
  static_assert(cannot_call_select_columns_with<decltype(bar.intN = 7),
                                                decltype(bar.boolNn)>,
                "not value: assignment");

  // select_columns(<at least one unnamed column>) is inconsistent and cannot be
  // constructed.
  static_assert(cannot_call_select_columns_with<decltype(sqlpp::value(7))>,
                "each selected column must have a name");
  static_assert(
      cannot_call_select_columns_with<decltype(bar.id), decltype(max(foo.id))>,
      "each selected column must have a name");
  static_assert(cannot_call_select_columns_with<decltype(all_of(bar)),
                                                decltype(max(foo.id))>,
                "each selected column must have a name");

  static_assert(
      cannot_call_select_columns_with<decltype(dynamic(true, sqlpp::value(7)))>,
      "each selected column must have a name");
  static_assert(
      cannot_call_select_columns_with<decltype(bar.id),
                                      decltype(dynamic(true, max(foo.id)))>,
      "each selected column must have a name");
  static_assert(cannot_call_select_columns_with<decltype(dynamic(true, bar.id)),
                                                decltype(max(foo.id))>,
                "each selected column must have a name");
  static_assert(
      cannot_call_select_columns_with<decltype(all_of(bar)),
                                      decltype(dynamic(true, max(foo.id)))>,
      "each selected column must have a name");
  // Note: There is no `dynamic(condition, all_of(table))`

  // select_columns(<selecting table columns without `from`>) can be constructed
  // but is inconsistent.
  {
    auto s = sqlpp::select_columns(bar.id);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_tables_in_selected_columns_t>::value,
        "");
  }

  {
    auto s = sqlpp::select_columns(dynamic(true, bar.id));
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_tables_in_selected_columns_t>::value,
        "");
  }

  // ----------------------------
  // ------- Aggregates ---------
  // ----------------------------
  // select_columns(<mix of aggregate and non-aggregate columns>) can be
  // constructed but is inconsistent.
  {
    auto s = select(foo.id, max(foo.id).as(test::max_id)).from(foo);
    using S = decltype(s);
    static_assert(
        std::is_same<sqlpp::statement_consistency_check_t<S>,
                     sqlpp::assert_select_columns_all_aggregates_t>::value,
        "");
  }

  {
    auto s = select(foo.id, (max(foo.id) + 7).as(test::max_id)).from(foo);
    using S = decltype(s);
    static_assert(
        std::is_same<sqlpp::statement_consistency_check_t<S>,
                     sqlpp::assert_select_columns_all_aggregates_t>::value,
        "");
  }

  {
    auto s = select(foo.id, foo.intN).from(foo).group_by(foo.intN);
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_select_columns_with_group_by_are_aggregates_t>::value,
        "");
  }

  {
    auto s =
        select(foo.id, dynamic(true, foo.intN)).from(foo).group_by(foo.intN);
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_select_columns_with_group_by_are_aggregates_t>::value,
        "");
  }

  {
    auto s = select(foo.id, dynamic(true, (foo.intN + 7).as(test::max_id)))
                 .from(foo)
                 .group_by(foo.intN);
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_select_columns_with_group_by_are_aggregates_t>::value,
        "");
  }

  // Dynamic group by column
  {
    auto s = select(foo.id, dynamic(true, foo.intN.as(test::max_id)))
                 .from(foo)
                 .group_by(foo.id, dynamic(true, foo.intN));
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
  }
  {
    auto s = select(foo.id, foo.intN)
                 .from(foo)
                 .group_by(foo.id, dynamic(true, foo.intN));
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::
                assert_select_columns_with_group_by_match_static_aggregates_t>::
            value,
        "");
  }
  {
    auto s = select(foo.id, (foo.intN + 7).as(test::max_id))
                 .from(foo)
                 .group_by(foo.id, dynamic(true, foo.intN));
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::
                assert_select_columns_with_group_by_match_static_aggregates_t>::
            value,
        "");
  }

  // Non-column group by
  {
    const auto c = foo.id + foo.intN;
    auto s = select(c.as(something))
                 .from(foo)
                 .group_by(c);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
  }
  {
    const auto c = foo.id + foo.intN;
    auto s = select((foo.doubleN + c).as(something))
                 .from(foo)
                 .group_by(foo.doubleN, c);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
  }
  {
    const auto c = foo.id + foo.intN;
    auto s = select((foo.id + c).as(something))
                 .from(foo)
                 .group_by(foo.doubleN, c);
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_select_columns_with_group_by_are_aggregates_t>::value,
        "");
  }
  // ----------------------------
  // ------- Join  --------------
  // ----------------------------
  {
    auto s = select(foo.id).from(bar.cross_join(foo));
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
  }

  {
    // Fail: Statically required table, but provided dynamically only
    auto s = select(foo.id).from(bar.cross_join(dynamic(true, foo)));
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
  }
  {
    // Fail: Statically required table, but provided dynamically only
    auto s = select(foo.id).from(dynamic(true, foo));
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
  }
  {
    // Fail: This is a sub select that statically requires `foo` but provides it
    // dynamically only.
    auto s = select(foo.id, bar.intN).from(dynamic(true, foo));
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
  }
  {
    // Fail: foo is statically required in a selected expression, but provided
    // dynamically only.
    auto s = select((foo.id + bar.intN).as(something)).from(dynamic(true, foo));
    using S = decltype(s);
    static_assert(
        std::is_same<
            sqlpp::statement_consistency_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_static_tables_in_selected_columns_t>::
            value,
        "");
  }
  {
    // Fail: `bar` is required, but not provided. This can be used as a
    // sub-select (consistency check), but not as a table (prepare check), and
    // it could not be prepared or executed either.
    auto s = select(bar.id).from(foo);
    using S = decltype(s);
    static_assert(std::is_same<sqlpp::statement_consistency_check_t<S>,
                               sqlpp::consistent_t>::value,
                  "");
    static_assert(
        std::is_same<
            sqlpp::statement_prepare_check_t<S>,
            sqlpp::assert_no_unknown_tables_in_selected_columns_t>::value,
        "");
  }
}
