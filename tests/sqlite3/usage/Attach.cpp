/*
 * Copyright (c) 2015 - 2016, Roland Bock
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

#include <cassert>

#include <sqlpp23/sqlite3/database/connection.h>
#include <sqlpp23/sqlpp23.h>
#include <sqlpp23/tests/sqlite3/make_test_connection.h>
#include "Tables.h"

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

namespace sql = sqlpp::sqlite3;

int Attach(int, char*[]) {
  // Opening a connection to an in-memory database and creating a table in it
  auto config = sql::make_test_config();
  auto db = sql::make_test_connection();
  test::createTabSample(db);

  // Attaching another in-memory database and creating the same table in it
  auto other = db.attach(*config, "other");
  db.execute(R"(CREATE TABLE other.tab_sample (
  id INTEGER PRIMARY KEY,
  alpha bigint(20) DEFAULT NULL,
  beta varchar(255) DEFAULT NULL,
  gamma boolean
))");

  auto left = test::TabSample{};
  auto right =
      schema_qualified_table(other, test::TabSample{})
          .as(sqlpp::alias::right);  // this is a table in the attached database

  // inserting in one tab_sample
  db(insert_into(left).default_values());

  // selecting from the other tab_sample
  assert(db(select(all_of(right)).from(right)).empty());

  return 0;
}
