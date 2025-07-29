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
  const auto foo = test::TabFoo{};

  // Plain assignments.
  SQLPP_COMPARE(insert_set(foo.intN = 7), " (int_n) VALUES(7)");
  SQLPP_COMPARE(insert_set(foo.intN = 7, foo.textNnD = "cheesecake"),
                " (int_n, text_nn_d) VALUES(7, 'cheesecake')");

  // Dynamic assignments.
  SQLPP_COMPARE(insert_set(sqlpp::dynamic(true, foo.intN = 7),
                           sqlpp::dynamic(false, foo.textNnD = "cheesecake")),
                " (int_n) VALUES(7)");
  SQLPP_COMPARE(insert_set(sqlpp::dynamic(false, foo.intN = 7),
                           sqlpp::dynamic(true, foo.textNnD = "cheesecake")),
                " (text_nn_d) VALUES('cheesecake')");

  return 0;
}
