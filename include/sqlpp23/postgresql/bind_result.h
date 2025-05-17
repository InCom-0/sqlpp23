#pragma once

/**
 * Copyright © 2014-2020, Matthijs Möhlmann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
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

#include <optional>
#include <span>
#include <string_view>

#include <sqlpp23/core/chrono.h>
#include <sqlpp23/core/detail/parse_date_time.h>
#include <sqlpp23/core/query/result_row.h>
#include <sqlpp23/postgresql/database/connection_config.h>
#include <sqlpp23/postgresql/result.h>

namespace sqlpp::postgresql {
namespace detail {
struct statement_handle_t;

inline unsigned char unhex(unsigned char c) {
  switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return c - '0';
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      return c + 10 - 'a';
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      return c + 10 - 'A';
  }
  throw sqlpp::exception{std::string{"Unexpected hex char: "} +
                         static_cast<char>(c)};
}

inline size_t hex_assign(std::vector<uint8_t>& value,
                         const uint8_t* blob,
                         size_t len) {
  const auto result_size = len / 2 - 1;  // unhex - leading chars
  if (value.size() < result_size) {
    value.resize(result_size);  // unhex - leading chars
  }
  size_t val_index = 0;
  size_t blob_index = 2;
  while (blob_index < len) {
    value[val_index] =
        static_cast<unsigned char>(unhex(blob[blob_index]) << 4) +
        unhex(blob[blob_index + 1]);
    ++val_index;
    blob_index += 2;
  }
  return result_size;
}
}  // namespace detail

class bind_result_t {
  Result _result;
  const connection_config* _config;
  int _row_index = -1;
  int _row_count = 0;
  int _field_count = 0;
  // Need to buffer blobs (or switch to PQexecParams with binary results)
  std::vector<std::vector<uint8_t>> _var_buffers;

  bool next_impl() {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "accessing next row of handle at {}",
                           std::hash<void*>{}(_result.native_handle()));
    }

    // Next row
    ++_row_index;
    if (_row_index < _row_count) {
      return true;
    }
    return false;
  }

 public:
  bind_result_t() = default;

  bind_result_t(Result result, const connection_config* config)
      : _result{std::move(result)},
        _config{config},
        _row_count{_result.records_size()},
        _field_count{_result.field_count()},
        _var_buffers{static_cast<size_t>(_field_count),
                     std::vector<uint8_t>{}} {
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                         "constructing bind result, using handle at {}",
                         std::hash<void*>{}(_result.native_handle()));
    }
  }

  bind_result_t(const bind_result_t&) = delete;
  bind_result_t(bind_result_t&&) = default;
  bind_result_t& operator=(const bind_result_t&) = delete;
  bind_result_t& operator=(bind_result_t&&) = default;
  ~bind_result_t() = default;

  bool operator==(const bind_result_t& rhs) const {
    return (this->_result == rhs._result);
  }

  template <typename ResultRow>
  void next(ResultRow& result_row) {
    if (!_result.native_handle()) {
      sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      return;
    }

    if (this->next_impl()) {
      if (not result_row) {
        sqlpp::detail::result_row_bridge{}.validate(result_row);
      }
      sqlpp::detail::result_row_bridge{}.read_fields(result_row, *this);
    } else {
      if (result_row) {
        sqlpp::detail::result_row_bridge{}.invalidate(result_row);
      }
    }
  }

  void read_field(size_t _index, bool& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading boolean result at index {}", index);
    }

    value = _result.get_bool_value(_row_index, index);
  }

  void read_field(size_t _index, double& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading floating_point result at index {}", index);
    }

    value = _result.get_double_value(_row_index, index);
  }

  void read_field(size_t _index, int64_t& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading integral result at index: {}", index);
    }

    value = _result.get_int64_value(_row_index, index);
  }

  void read_field(size_t _index, uint64_t& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(
          log_category::result,
          "PostgreSQL debug: reading unsigned integral result at index {}",
          index);
    }

    value = _result.get_uint64_value(_row_index, index);
  }

  void read_field(size_t _index, std::string_view& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading text result at index {}", index);
    }

    value = std::string_view(
        _result.get_char_ptr_value(_row_index, index),
        static_cast<size_t>(_result.length(_row_index, index)));
  }

  // PostgreSQL will return one of those (using the default ISO client):
  //
  // 2010-10-11 01:02:03 - ISO timestamp without timezone
  // 2011-11-12 01:02:03.123456 - ISO timesapt with sub-second (microsecond)
  // precision 1997-12-17 07:37:16-08 - ISO timestamp with timezone 1992-10-10
  // 01:02:03-06:30 - for some timezones with non-hour offset 1900-01-01 - date
  // only we do not support time-only values !
  void read_field(size_t _index, ::sqlpp::chrono::day_point& value) {
    const auto index = static_cast<int>(_index);

    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading date result at index {}", index);
    }

    const auto date_string =
        _result.get_char_ptr_value(_row_index, index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result, "date string: {}",
                           date_string);
    }
    if (::sqlpp::detail::parse_date(value, date_string) == false) {
      if constexpr (debug_enabled) {
        _config->debug.log(log_category::result, "got invalid date '{}'",
                             date_string);
      }
    }
  }

  // always returns UTC time for timestamp with time zone
  void read_field(size_t _index, ::sqlpp::chrono::microsecond_point& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading date_time result at index {}", index);
    }

    const auto date_string =
        _result.get_char_ptr_value(_row_index, index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result, "got date_time string: {}",
                           date_string);
    }
    if (::sqlpp::detail::parse_timestamp(value, date_string) == false) {
      if constexpr (debug_enabled) {
        _config->debug.log(log_category::result, "got invalid date_time '{}'",
                             date_string);
      }
    }
  }

  // always returns UTC time for time with time zone
  void read_field(size_t _index, ::std::chrono::microseconds& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading time result at index {}", index);
    }

    const auto time_string =
        _result.get_char_ptr_value(_row_index, index);

    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result, "got time string: {}",
                           time_string);
    }

    if (::sqlpp::detail::parse_time_of_day(value, time_string) == false) {
      if constexpr (debug_enabled) {
        _config->debug.log(log_category::result, "got invalid time '{}'",
                             time_string);
      }
    }
  }

  void read_field(size_t _index, std::span<const uint8_t>& value) {
    const auto index = static_cast<int>(_index);
    if constexpr (debug_enabled) {
      _config->debug.log(log_category::result,
                           "reading blob result at index {}", index);
    }

    // Need to decode the hex data.
    // Using PQexecParams would allow to use binary data.
    // That's certainly faster, but some effort to determine the correct column
    // types.
    const auto size = detail::hex_assign(
        _var_buffers[_index],
        _result.get_blob_value(_row_index, index),
        static_cast<size_t>(_result.length(_row_index, index)));

    value = std::span<const uint8_t>(_var_buffers[_index].data(), size);
  }

  template <typename T>
  auto read_field(size_t _index, std::optional<T>& value) -> void {
    const auto index = static_cast<int>(_index);
    if (_result.is_null(_row_index, index)) {
      value.reset();
    } else {
      if (not value.has_value()) {
        value = T{};
      }
      read_field(_index, *value);
    }
  }

  int size() const { return _result.records_size(); }
};
}  // namespace sqlpp::postgresql
