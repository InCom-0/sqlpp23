/*
 * Copyright (c) 2013 - 2016, Roland Bock
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

#include <iostream>

#include <sqlpp23/sqlite3/database/connection.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include <sqlpp23/sqlpp23.h>
#include "Tables.h"

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& t) {
  if (not t)
    return os << "NULL";
  return os << t.value();
}

namespace sql = sqlpp::sqlite3;
int DynamicSelect(int, char*[]) {
  auto db = sql::make_test_connection();
  test::createTabSample(db);

  const auto tab = test::TabSample{};

  auto i = insert_into(tab).columns(tab.beta, tab.gamma);
  i.add_values(tab.beta = "rhabarbertorte", tab.gamma = false);
  i.add_values(tab.beta = "cheesecake", tab.gamma = false);
  i.add_values(tab.beta = "kaesekuchen", tab.gamma = true);
  auto last_insert_rowid = db(i);

  std::cerr << "last insert rowid: " << last_insert_rowid << std::endl;

  // Just to demonstrate that you can call basically any function
  std::cerr << "last insert rowid: "
            << db(select(sqlpp::verbatim<sqlpp::integral>("last_insert_rowid()")
                             .as(tab.alpha)))
                   .front()
                   .alpha
            << std::endl;

  std::cerr << "last insert rowid: "
            << db(select(dynamic(true, sqlpp::verbatim<sqlpp::integral>(
                                           "last_insert_rowid()")
                                           .as(tab.alpha))))
                   .front()
                   .alpha
            << std::endl;

  std::cerr << "last insert rowid: "
            << db(select(dynamic(false, sqlpp::verbatim<sqlpp::integral>(
                                            "last_insert_rowid()")
                                            .as(tab.alpha))))
                   .front()
                   .alpha
            << std::endl;

  return 0;
}
