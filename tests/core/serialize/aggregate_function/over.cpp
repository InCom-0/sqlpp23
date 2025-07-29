/*
 * Copyright (c) 2016, Roland Bock
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

SQLPP_CREATE_NAME_TAG(cheese);

int main(int, char*[]) {
  auto const foo = test::TabFoo{};

  SQLPP_COMPARE(avg(foo.doubleN).over().as(cheese),
                "AVG(tab_foo.double_n) OVER() AS cheese");
  SQLPP_COMPARE(count(foo.doubleN).over().as(cheese),
                "COUNT(tab_foo.double_n) OVER() AS cheese");
  SQLPP_COMPARE(max(foo.doubleN).over().as(cheese),
                "MAX(tab_foo.double_n) OVER() AS cheese");
  SQLPP_COMPARE(min(foo.doubleN).over().as(cheese),
                "MIN(tab_foo.double_n) OVER() AS cheese");
  SQLPP_COMPARE(sum(foo.doubleN).over().as(cheese),
                "SUM(tab_foo.double_n) OVER() AS cheese");

  return 0;
}
