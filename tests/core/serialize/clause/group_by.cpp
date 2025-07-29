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

#include <sqlpp23/tests/core/all.h>

int main(int, char*[]) {
  const auto val = sqlpp::value(17);

  const auto foo = test::TabFoo{};

  // Plain columns.
  SQLPP_COMPARE(group_by(foo.id), " GROUP BY tab_foo.id");
  SQLPP_COMPARE(group_by(foo.textNnD), " GROUP BY tab_foo.text_nn_d");
  SQLPP_COMPARE(group_by(foo.boolN), " GROUP BY tab_foo.bool_n");

  // Multiple plain columns.
  SQLPP_COMPARE(group_by(foo.id, foo.textNnD, foo.boolN),
                " GROUP BY tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");

  // Single dynamic column
  SQLPP_COMPARE(group_by(dynamic(true, foo.id)), " GROUP BY tab_foo.id");
  SQLPP_COMPARE(group_by(dynamic(false, foo.id)), "");

  // Multiple dynamic columns (including all dynamic)
  SQLPP_COMPARE(group_by(dynamic(true, foo.id), foo.textNnD, foo.boolN),
                " GROUP BY tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");
  SQLPP_COMPARE(group_by(foo.id, dynamic(true, foo.textNnD), foo.boolN),
                " GROUP BY tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");
  SQLPP_COMPARE(group_by(foo.id, foo.textNnD, dynamic(true, foo.boolN)),
                " GROUP BY tab_foo.id, tab_foo.text_nn_d, tab_foo.bool_n");

  SQLPP_COMPARE(group_by(dynamic(false, foo.id), foo.textNnD, foo.boolN),
                " GROUP BY tab_foo.text_nn_d, tab_foo.bool_n");
  SQLPP_COMPARE(group_by(foo.id, dynamic(false, foo.textNnD), foo.boolN),
                " GROUP BY tab_foo.id, tab_foo.bool_n");
  SQLPP_COMPARE(group_by(foo.id, foo.textNnD, dynamic(false, foo.boolN)),
                " GROUP BY tab_foo.id, tab_foo.text_nn_d");

  SQLPP_COMPARE(
      group_by(foo.id, dynamic(false, foo.textNnD), dynamic(false, foo.boolN)),
      " GROUP BY tab_foo.id");
  SQLPP_COMPARE(
      group_by(dynamic(false, foo.id), foo.textNnD, dynamic(false, foo.boolN)),
      " GROUP BY tab_foo.text_nn_d");
  SQLPP_COMPARE(
      group_by(dynamic(false, foo.id), dynamic(false, foo.textNnD), foo.boolN),
      " GROUP BY tab_foo.bool_n");

  SQLPP_COMPARE(group_by(dynamic(false, foo.id), dynamic(false, foo.textNnD),
                         dynamic(false, foo.boolN)),
                "");

  // Single declared column
  SQLPP_COMPARE(group_by(val), " GROUP BY 17");
  // Note that the parentheses are superflous but also don't hurt.
  SQLPP_COMPARE(group_by(foo.id + 17),
                " GROUP BY (tab_foo.id + 17)");

  // Mixed declared column
  SQLPP_COMPARE(group_by(foo.id, val),
                " GROUP BY tab_foo.id, 17");
  SQLPP_COMPARE(group_by(val, foo.id),
                " GROUP BY 17, tab_foo.id");

  // Mixed dynamic declared column
  SQLPP_COMPARE(group_by(foo.id, dynamic(true, val)),
                " GROUP BY tab_foo.id, 17");
  SQLPP_COMPARE(group_by(dynamic(true, val), foo.id),
                " GROUP BY 17, tab_foo.id");

  SQLPP_COMPARE(group_by(foo.id, dynamic(false, val)),
                " GROUP BY tab_foo.id");
  SQLPP_COMPARE(group_by(dynamic(false, val), foo.id),
                " GROUP BY tab_foo.id");

  return 0;
}
