/*
 * Copyright (c) 2013-2015, Roland Bock
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

import sqlpp23.core;
import sqlpp23.mock_db;
import sqlpp23.test.core.tables;

#include <sqlpp23/core/name/create_name_tag.h>
#include <sqlpp23/tests/core/make_test_connection.h>
#include <sqlpp23/tests/core/result_helpers.h>

int Prepared(int, char*[]) {
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();
  // test::TabFoo f;
  const auto t = test::TabBar{};

  // empty parameter lists
  {
    using P = sqlpp::make_parameter_list_t<decltype(t.id)>;
    static_assert(P::size::value == 0, "type requirement");
  }

  // single parameter
  {
    using P = sqlpp::make_parameter_list_t<decltype(parameter(t.id))>;
    static_assert(P::size::value == 1, "type requirement");
    auto p = P{};
    p.id = 7;
    std::ignore = p;  // silence warnings about `p` being unused.
  }

  // single parameter
  {
    using P = sqlpp::make_parameter_list_t<decltype(parameter(t.textN))>;
    static_assert(P::size::value == 1, "type requirement");
    auto p = P{};
    p.textN = "cheesecake";
    std::ignore = p;  // silence warnings about `p` being unused.
  }

  // single parameter in expression
  {
    using P = sqlpp::make_parameter_list_t<decltype(t.id == parameter(t.id))>;
    static_assert(P::size::value == 1, "type requirement");
    auto p = P{};
    p.id = 7;
    std::ignore = p;  // silence warnings about `p` being unused.
  }

  // single parameter in larger expression
  {
    using P = sqlpp::make_parameter_list_t<decltype((t.textN.like("%") and
                                                     t.id == parameter(t.id)) or
                                                    t.boolNn != false)>;
    static_assert(P::size::value == 1, "type requirement");
    auto p = P{};
    p.id = 7;
    std::ignore = p;  // silence warnings about `p` being unused.
  }

  // three parameters in expression
  {
    using P =
        sqlpp::parameters_of_t<decltype((t.textN.like(parameter(t.textN)) and
                                         t.id == parameter(t.id)) or
                                        t.boolNn != parameter(t.boolNn))>;
    // parameters
    static_assert(
        std::is_same<
            P, sqlpp::detail::type_vector<
                   decltype(parameter(t.textN)), decltype(parameter(t.id)),
                   decltype(parameter(t.boolNn))>>::value,
        "type requirement");
  }

  // OK, fine, now create a named parameter list from an expression
  {
    using Exp = decltype((t.textN.like(parameter(t.textN)) and
                          t.id == parameter(t.id)) or
                         t.boolNn != parameter(t.boolNn));
    using P = sqlpp::make_parameter_list_t<Exp>;
    P npl;
    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.id)>>,
            decltype(npl.id)>::value,
        "type requirement");
    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.textN)>>,
            decltype(npl.textN)>::value,
        "type requirement");
    static_assert(std::is_same<sqlpp::parameter_value_t<
                                   sqlpp::data_type_of_t<decltype(t.boolNn)>>,
                               decltype(npl.boolNn)>::value,
                  "type requirement");
  }

  // Wonderful, now take a look at the parameter list of a select
  {
    auto s = select(all_of(t)).from(t).where(
        (t.textN.like(parameter(t.textN)) and t.id == parameter(t.id)) or
        t.boolNn != parameter(t.boolNn));
    auto p = db.prepare(s);
    p.parameters.id = 7;
    using S = decltype(s);
    using P = sqlpp::make_parameter_list_t<S>;
    P npl;

    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.id)>>,
            decltype(npl.id)>::value,
        "type requirement");
    static_assert(
        std::is_same<
            sqlpp::parameter_value_t<sqlpp::data_type_of_t<decltype(t.textN)>>,
            decltype(npl.textN)>::value,
        "type requirement");
    static_assert(std::is_same<sqlpp::parameter_value_t<
                                   sqlpp::data_type_of_t<decltype(t.boolNn)>>,
                               decltype(npl.boolNn)>::value,
                  "type requirement");
    npl.id = 7;
    auto x = npl;
    x = npl;
    std::cerr << x.id << std::endl;
    x = decltype(npl)();
    std::cerr << x.id << std::endl;
  }

  // Can we prepare a query without parameters?
  {
    auto ps = db.prepare(select(all_of(t)).from(t).where((t.textN.like("%"))));
    for (const auto& row : db(ps)) {
      std::cerr << row.id << std::endl;
    }
  }

  return 0;
}
