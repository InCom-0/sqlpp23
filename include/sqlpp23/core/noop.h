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

#include <type_traits>

#include <sqlpp23/core/database/prepared_execute.h>
#include <sqlpp23/core/query/statement_handler.h>
#include <sqlpp23/core/to_sql_string.h>

namespace sqlpp {
struct noop {};

struct no_result_methods_t {
 private:
  friend class statement_handler_t;

  // Execute
  template <typename Statement, typename Db>
  auto _run(this Statement&& self, Db& db) {
    return statement_handler_t{}.execute(std::forward<Statement>(self), db);
  }

  // Prepare
  template <typename Statement, typename Db>
  auto _prepare(this Statement&& self, Db& db)
      -> prepared_execute_t<Db, std::decay_t<Statement>> {
    return prepared_execute_t<Db, std::decay_t<Statement>>{
        statement_handler_t{}.prepare_execute(std::forward<Statement>(self),
                                              db)};
  }
};

template <>
struct result_methods_of<noop> {
  using type = no_result_methods_t;
};

template <typename Context>
auto to_sql_string(Context&, const noop&) -> std::string {
  return {};
}

template <typename T>
struct is_noop : std::is_same<T, noop> {};
}  // namespace sqlpp
