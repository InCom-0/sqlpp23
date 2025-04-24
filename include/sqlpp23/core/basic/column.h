#pragma once

/*
 * Copyright (c) 2013-2015, Roland Bock
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

#include <sqlpp23/core/basic/column_fwd.h>
#include <sqlpp23/core/default_value.h>
#include <sqlpp23/core/detail/type_vector.h>
#include <sqlpp23/core/operator/as_expression.h>
#include <sqlpp23/core/operator/assign_expression.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/type_traits.h>
#include <sqlpp23/core/wrong.h>
#include <type_traits>

namespace sqlpp {
// _Table can be a table_t or a cte_ref_t or a select_ref_t
template <typename _Table, typename ColumnSpec>
struct column_t : public enable_as, public enable_comparison {
  using _spec_t = ColumnSpec;
  using _table = _Table;

  column_t() = default;
  column_t(const column_t&) = default;
  column_t(column_t&&) = default;
  column_t& operator=(const column_t&) = default;
  column_t& operator=(column_t&&) = default;
  ~column_t() = default;

  static auto table() -> _table { return _table{}; }

  template <typename T>
    requires(are_correct_assignment_args<column_t, T> and
             not is_const<column_t>::value)
  auto operator=(T value) const -> assign_expression<column_t, op_assign, T> {
    return assign(*this, std::move(value));
  }

  template<typename T = void>
  requires(has_default<column_t>::value and
           not is_const<column_t>::value)
  auto operator=(default_value_t value) const -> assign_expression<column_t, op_assign, default_value_t> {
    return assign(*this, std::move(value));
  }
};

template <typename _Table, typename ColumnSpec>
struct is_aggregate_neutral<column_t<_Table, ColumnSpec>>
    : public std::false_type {};

template <typename _Table, typename ColumnSpec>
struct has_default<column_t<_Table, ColumnSpec>>
    : public ColumnSpec::has_default {};

template <typename _Table, typename ColumnSpec>
struct is_column<column_t<_Table, ColumnSpec>> : public std::true_type {};

template <typename _Table, typename ColumnSpec>
struct name_tag_of<column_t<_Table, ColumnSpec>>
    : public name_tag_of<ColumnSpec> {};

template <typename _Table, typename ColumnSpec>
struct required_tables_of<column_t<_Table, ColumnSpec>> {
  using type = detail::type_set<_Table>;
};

template <typename _Table, typename ColumnSpec>
struct required_static_tables_of<column_t<_Table, ColumnSpec>>
    : public required_tables_of<column_t<_Table, ColumnSpec>> {};

template <typename _Table, typename ColumnSpec>
struct data_type_of<column_t<_Table, ColumnSpec>> {
  using type = std::remove_const_t<typename ColumnSpec::data_type>;
};

template <typename _Table, typename ColumnSpec>
struct is_const<column_t<_Table, ColumnSpec>>
    : public std::is_const<typename ColumnSpec::data_type> {};

template <typename Context, typename _Table, typename ColumnSpec>
auto to_sql_string(Context& context, const column_t<_Table, ColumnSpec>&)
    -> std::string {
  using T = column_t<_Table, ColumnSpec>;

  return name_to_sql_string(context, name_tag_of_t<_Table>{}) + "." +
         name_to_sql_string(context, name_tag_of_t<T>{});
}
}  // namespace sqlpp
