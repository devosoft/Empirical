/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/LocalStorage.hpp
 * @brief Recoverable browser storage. Missing keys are distinct from failed reads.
 *
 * Use example:
 *
 *   auto saved = emp::web::LocalStorage::Set("display-mode", "compact");
 *   if (!saved) { ReportError(saved.error()); }
 *   auto loaded = emp::web::LocalStorage::Get("display-mode");
 *   if (loaded && loaded->has_value()) { ApplyMode(**loaded); }
 *
 * Set(), Get(), and Remove() return std::expected; check the result before
 * reading it, and use error() for a failure message. Get() contains an optional
 * string, so a missing key is a successful read with no value. Storage failures
 * are reported without C++ exceptions. Native builds return an unavailable error.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_LOCAL_STORAGE_HPP_GUARD
#define INCLUDE_EMP_WEB_LOCAL_STORAGE_HPP_GUARD

#include <cstdlib>
#include <expected>
#include <optional>
#include <string>

#include "init.hpp"

namespace emp::web {
  class LocalStorage {
    static std::expected<std::optional<std::string>, std::string>
    Access(int operation, const std::string & key, const std::string & value = "") {
#ifdef __EMSCRIPTEN__
      // Browser storage is a throwing API. Catch at the JS boundary; no C++ exceptions are used.
      auto * buffer = reinterpret_cast<char *>(MAIN_THREAD_EM_ASM_PTR({
        let result;
        try {
          const key = UTF8ToString($1);
          if ($0 === 0) {
            const value = localStorage.getItem(key);
            result = value === null ? 'm' : 'v' + value;
          } else {
            if ($0 === 1) localStorage.setItem(key, UTF8ToString($2));
            else localStorage.removeItem(key);
            result = 'm';
          }
        } catch (error) { result = 'e' + String(error); }
        const size = lengthBytesUTF8(result) + 1;
        const buffer = _malloc(size);
        if (buffer) stringToUTF8(result, buffer, size);
        return buffer;
      }, operation, key.c_str(), value.c_str()));
      if (!buffer) { return std::unexpected("Unable to allocate storage result."); }
      const char status = buffer[0];
      std::string result(buffer + 1);
      std::free(buffer);
      if (status == 'e') { return std::unexpected(std::move(result)); }
      if (status == 'v') { return result; }
      return std::nullopt;
#else
      (void) operation; (void) key; (void) value;
      return std::unexpected("Browser storage is unavailable in native builds.");
#endif
    }
  public:
    static std::expected<std::optional<std::string>, std::string> Get(const std::string & key) {
      return Access(0, key);
    }
    static std::expected<void, std::string> Set(const std::string & key, const std::string & value) {
      auto result = Access(1, key, value);
      if (!result) { return std::unexpected(result.error()); }
      return {};
    }
    static std::expected<void, std::string> Remove(const std::string & key) {
      auto result = Access(2, key);
      if (!result) { return std::unexpected(result.error()); }
      return {};
    }
  };
}

#endif  // INCLUDE_EMP_WEB_LOCAL_STORAGE_HPP_GUARD
