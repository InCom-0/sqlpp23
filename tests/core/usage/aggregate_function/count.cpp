/*
 * Copyright (c) 2025, Roland Bock
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

#define SQLPP_USE_MOCK_DB
#include <sqlpp23/tests/core/all.h>

SQLPP_CREATE_NAME_TAG(count_1);
SQLPP_CREATE_NAME_TAG(count_star);

int main(int, char*[]) {
  try {
    const auto tab = test::TabFoo{};
    auto db = sqlpp::mock_db::make_test_connection();

    // clear the table
    db(truncate(tab));

    // insert
    db(insert_into(tab).set(tab.intN = 7));
    db(insert_into(tab).set(tab.intN = 7));
    db(insert_into(tab).set(tab.intN = 9));

    // select count
    for (const auto& row : db(select(
            count(tab.intN).as(sqlpp::alias::count_),
            sqlpp::count(1).as(count_1),
            count(tab.intN).as(count_star)
            ).from(tab))) {
      std::ignore = row.count_;
      std::ignore = row.count_1;
      std::ignore = row.count_star;
    }

    // select distinct count
    for (const auto& row : db(select(
            count(sqlpp::distinct, tab.intN).as(sqlpp::alias::distinct_count_),
            count(sqlpp::distinct, 1).as(count_1)
            ).from(tab))) {
      std::ignore = row.distinct_count_;
      std::ignore = row.count_1;
    }
  } catch (const std::exception& e) {
    std::println(stderr, "Exception: {}", e.what());
    return 1;
  }
  return 0;
}
