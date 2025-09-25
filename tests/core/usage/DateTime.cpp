/*
 * Copyright (c) 2015-2015, Roland Bock
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

#include <chrono>

#define SQLPP_USE_MOCK_DB
#include <sqlpp23/tests/core/all.h>

namespace {
template <typename T>
std::chrono::microseconds time(T t) {
  const auto dp = std::chrono::floor<std::chrono::days>(t);
  return std::chrono::duration_cast<std::chrono::microseconds>(t - dp);
}
}

SQLPP_CREATE_NAME_TAG(now);

int DateTime(int, char*[]) {
  sqlpp::mock_db::connection db = sqlpp::mock_db::make_test_connection();
  sqlpp::mock_db::context_t printer;
  const auto t = test::TabDateTime{};

  for (const auto& row :
       db(select(::sqlpp::value(std::chrono::system_clock::now()).as(now)))) {
    std::print("{}\n", row.now);
  }
  for (const auto& row : db(select(all_of(t)).from(t))) {
    std::print("{}\n",  row.dateN);
    std::print("{}\n",  row.timestampN);
    const auto tp = row.timestampN.value();
    std::print("{}\n",  std::chrono::system_clock::to_time_t(tp));
  }
  std::print("{}\n", to_sql_string(printer,
                             ::sqlpp::value(std::chrono::system_clock::now())));

  db(insert_into(t).set(t.dateN = std::chrono::floor<std::chrono::days>(
                            std::chrono::system_clock::now())));
  db(insert_into(t).set(t.timestampN = std::chrono::system_clock::now()));
  db(insert_into(t).set(t.timeN = time(
                            std::chrono::system_clock::now())));

  db(update(t)
         .set(t.dateN = std::chrono::floor<std::chrono::days>(
                  std::chrono::system_clock::now()))
         .where(t.dateN < std::chrono::system_clock::now()));
  db(update(t)
         .set(t.timestampN = std::chrono::system_clock::now(),
              t.timeN =
                  time(std::chrono::system_clock::now()))
         .where(t.dateN < std::chrono::system_clock::now()));

  db(delete_from(t).where(
      t.dateN ==
      std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())));
  db(delete_from(t).where(t.dateN == std::chrono::system_clock::now()));
  db(delete_from(t).where(
      t.timestampN ==
      std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())));
  db(delete_from(t).where(t.timestampN == std::chrono::system_clock::now()));
  db(delete_from(t).where(
      t.timeN ==
      time(std::chrono::system_clock::now())));

  return 0;
}
