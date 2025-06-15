#pragma once

/*
Copyright (c) 2017 - 2018, Roland Bock
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// logical
#include <sqlpp23/core/operator/logical_expression.h>

// comparison
#include <sqlpp23/core/operator/any.h>
#include <sqlpp23/core/operator/between_expression.h>
#include <sqlpp23/core/operator/comparison_expression.h>
#include <sqlpp23/core/operator/in_expression.h>

// arithmetic
#include <sqlpp23/core/operator/arithmetic_expression.h>

// binary
#include <sqlpp23/core/operator/bit_expression.h>

// assignment
#include <sqlpp23/core/operator/assign_expression.h>

// misc
#include <sqlpp23/core/operator/as_expression.h>
#include <sqlpp23/core/operator/cast_as.h>
#include <sqlpp23/core/operator/exists_expression.h>
#include <sqlpp23/core/operator/sort_order_expression.h>
