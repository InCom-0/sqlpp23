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

#include <sqlpp23/core/clause/single_table.h>
#include <sqlpp23/core/clause/update_set_list.h>
#include <sqlpp23/core/clause/where.h>
#include <sqlpp23/core/database/connection.h>
#include <sqlpp23/core/database/prepared_update.h>
#include <sqlpp23/core/query/statement.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/type_traits.h>

namespace sqlpp {
struct update_t {};

template <typename Context>
auto to_sql_string(Context&, const update_t&) -> std::string {
  return "UPDATE ";
}

template <>
struct is_clause<update_t> : public std::true_type {};

struct update_result_methods_t {
 private:
  friend class statement_handler_t;

  // Execute
  template <typename Statement, typename Db>
  auto _run(this Statement&& self, Db& db) {
    return statement_handler_t{}.update(std::forward<Statement>(self), db);
  }

  // Prepare
  template <typename Statement, typename Db>
  auto _prepare(this Statement&& self, Db& db)
      -> prepared_update_t<Db, std::decay_t<Statement>> {
    return prepared_update_t<Db, std::decay_t<Statement>>{
        statement_handler_t{}.prepare_update(std::forward<Statement>(self),
                                             db)};
  }
};

template <>
struct result_methods_of<update_t> {
  using type = update_result_methods_t;
};

template <typename Statement>
struct consistency_check<Statement, update_t> {
  using type = consistent_t;
  constexpr auto operator()() {
    return type{};
  }
};

template <>
struct is_result_clause<update_t> : public std::true_type {};

using blank_update_t = statement_t<update_t,
                                   no_single_table_t,
                                   no_update_set_list_t,
                                   no_where_t>;

template <typename _Table>
constexpr auto update(_Table table)
    -> decltype(blank_update_t().single_table(table)) {
  return {blank_update_t().single_table(table)};
}

}  // namespace sqlpp
