#pragma once

#include <vkh/macros.hpp>

#define EG_MAKE(_NAME, _EXPR, _ERROR)                                          \
  auto _NAME##_res = _EXPR;                                                    \
  if (!_NAME##_res) {                                                          \
    Logger::error(_ERROR ": {}", _NAME##_res.error());                         \
    return std::unexpected(_ERROR);                                            \
  }                                                                            \
  auto &(_NAME) = _NAME##_res.value();
