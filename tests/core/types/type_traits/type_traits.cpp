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

#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/core/tables.h>
#include <sqlpp23/tests/core/types_helpers.h>
#include "sqlpp23/core/type_traits/data_type.h"

SQLPP_CREATE_NAME_TAG(something);

void test_is_as_expression() {
  auto v = sqlpp::value(17);
  auto t = sqlpp::value("");
  auto col_int = test::TabFoo{}.id;
  auto col_txt = test::TabFoo{}.textNnD;

  // Constant values are no expression alias
  static_assert(not sqlpp::is_as_expression<decltype(v)>::value, "");
  static_assert(not sqlpp::is_as_expression<decltype(v + v)>::value, "");

  // Columns are no expression alias
  static_assert(not sqlpp::is_as_expression<decltype(col_int)>::value, "");
  static_assert(not sqlpp::is_as_expression<decltype(col_int + v)>::value, "");

  // Normal functions are no expression alias
  static_assert(not sqlpp::is_as_expression<decltype(trim(t))>::value, "");
  static_assert(not sqlpp::is_as_expression<decltype(trim(col_txt))>::value,
                "");

  // But their alias is an expression alias (no surprise here, I guess)
  static_assert(sqlpp::is_as_expression<decltype((v).as(something))>::value,
                "");
  static_assert(sqlpp::is_as_expression<decltype((v + v).as(something))>::value,
                "");

  static_assert(
      sqlpp::is_as_expression<decltype((col_int).as(something))>::value, "");
  static_assert(
      sqlpp::is_as_expression<decltype((col_int + v).as(something))>::value,
      "");

  static_assert(
      sqlpp::is_as_expression<decltype((trim(t)).as(something))>::value, "");
  static_assert(
      sqlpp::is_as_expression<decltype((trim(col_txt)).as(something))>::value,
      "");

  // Data types themselves do not have data types
  static_assert(not sqlpp::has_data_type<sqlpp::boolean>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::integral>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::unsigned_integral>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::floating_point>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::text>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::blob>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::date>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::timestamp>::value);
  static_assert(not sqlpp::has_data_type<sqlpp::time>::value);

  // Data types themselves are data types, though
  static_assert(sqlpp::is_data_type<sqlpp::boolean>::value);
  static_assert(sqlpp::is_data_type<sqlpp::integral>::value);
  static_assert(sqlpp::is_data_type<sqlpp::unsigned_integral>::value);
  static_assert(sqlpp::is_data_type<sqlpp::floating_point>::value);
  static_assert(sqlpp::is_data_type<sqlpp::text>::value);
  static_assert(sqlpp::is_data_type<sqlpp::blob>::value);
  static_assert(sqlpp::is_data_type<sqlpp::date>::value);
  static_assert(sqlpp::is_data_type<sqlpp::timestamp>::value);
  static_assert(sqlpp::is_data_type<sqlpp::time>::value);

  // Data type of nested optionals is still just optional something
  using OptText = decltype(test::TabBar{}.textN);
  static_assert(std::is_same_v<sqlpp::data_type_of_t<OptText>,
                               std::optional<sqlpp::text>>);
  static_assert(std::is_same_v<sqlpp::data_type_of_t<std::optional<OptText>>,
                               std::optional<sqlpp::text>>);
  static_assert(sqlpp::is_text<std::optional<OptText>>::value);
  static_assert(sqlpp::is_text<std::optional<std::optional<OptText>>>::value);
}

int main() {
  void test_is_as_expression();
}
