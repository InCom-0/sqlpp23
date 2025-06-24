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

#include <sqlpp23/core/operator/enable_as.h>
#include <sqlpp23/core/operator/enable_comparison.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
template <typename T>
struct value_t : public enable_as, public enable_comparison {
  value_t(T t) : _value(std::move(t)) {}
  value_t(const value_t&) = default;
  value_t(value_t&&) = default;
  value_t& operator=(const value_t&) = default;
  value_t& operator=(value_t&&) = default;
  ~value_t() = default;

  T _value;
};

template <typename T>
struct data_type_of<value_t<T>> {
  using type = data_type_of_t<T>;
};

template <typename T>
struct nodes_of<value_t<T>> {
  // Required in case of value(select(...)).
  using type = detail::type_vector<T>;
};

template <typename T>
struct requires_parentheses<value_t<T>> : public requires_parentheses<T> {};

template <typename Context, typename T>
auto to_sql_string(Context& context, const value_t<T>& t) -> std::string {
  return to_sql_string(context, t._value);
}

template <typename T>
  requires(has_data_type_v<T> and
           not has_enabled_as<remove_optional_t<T>>::value and
           not has_name_tag_v<remove_optional_t<T>> and
           not is_statement_v<remove_optional_t<T>>)
auto value(T t) -> value_t<T> {
  return {std::move(t)};
}

template <typename T>
  requires(has_data_type_v<T> and is_statement_v<T> and
           not is_optional<T>::value)
auto value(T t) -> value_t<T> {
  return {std::move(t)};
}

}  // namespace sqlpp
