#pragma once

/*
 * Copyright (c) 2013, Roland Bock
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

#include <sqlpp23/core/aggregate_function/enable_over.h>
#include <sqlpp23/core/clause/select_flags.h>
#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/static_assert.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp::alias {
SQLPP_CREATE_NAME_TAG(max_);
SQLPP_CREATE_NAME_TAG(distinct_max_);
}

namespace sqlpp {
template <typename Flag, typename Expr>
struct max_t : public enable_as<max_t<Flag, Expr>>,
               public enable_comparison<max_t<Flag, Expr>>,
               public enable_over<max_t<Flag, Expr>> {
  constexpr max_t(Expr expr) : _expr(std::move(expr)) {}

  max_t(const max_t&) = default;
  max_t(max_t&&) = default;
  max_t& operator=(const max_t&) = default;
  max_t& operator=(max_t&&) = default;
  ~max_t() = default;

  Expr _expr;
};

template <typename Flag, typename Expr>
struct is_aggregate_function<max_t<Flag, Expr>> : public std::true_type {};

template <typename Flag, typename Expr>
struct nodes_of<max_t<Flag, Expr>> {
  using type = sqlpp::detail::type_vector<Expr>;
};

template <typename Flag, typename Expr>
struct data_type_of<max_t<Flag, Expr>> {
  using type = sqlpp::force_optional_t<data_type_of_t<Expr>>;
};

template <typename Context, typename Flag, typename Expr>
auto to_sql_string(Context& context, const max_t<Flag, Expr>& t)
    -> std::string {
  return "MAX(" + to_sql_string(context, Flag()) +
         to_sql_string(context, t._expr) + ")";
}

template <typename T>
using check_max_arg = std::enable_if_t<values_are_comparable<T, T>::value>;

template <typename T, typename = check_max_arg<T>>
auto max(T t) -> max_t<no_flag_t, T> {
  SQLPP_STATIC_ASSERT(not contains_aggregate_function<T>::value,
                      "max() must not be used on an aggregate function");
  return {std::move(t)};
}

template <typename T, typename = check_max_arg<T>>
auto max(const distinct_t& /*unused*/, T t) -> max_t<distinct_t, T> {
  SQLPP_STATIC_ASSERT(not contains_aggregate_function<T>::value,
                      "max() must not be used on an aggregate function");
  return {std::move(t)};
}
}  // namespace sqlpp
