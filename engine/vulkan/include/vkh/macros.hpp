#pragma once

#define VK_MAKE(_NAME, _EXPR, _ERROR)                                          \
  auto _NAME##_res = _EXPR;                                                    \
  if (_NAME##_res.result != vk::Result::eSuccess) {                            \
    Logger::error(_ERROR ": {}", vk::to_string(_NAME##_res.result));           \
    return std::unexpected(_ERROR);                                            \
  }                                                                            \
  auto &(_NAME) = _NAME##_res.value;

#define VMA_MAKE(_NAME, _EXPR, _ERROR)                                         \
  auto _NAME##_res = _EXPR;                                                    \
  if (_NAME##_res.result != vk::Result::eSuccess) {                            \
    Logger::error(_ERROR ": {}", vk::to_string(_NAME##_res.result));           \
    return std::unexpected(_ERROR);                                            \
  }                                                                            \
  auto &(_NAME) = _NAME##_res.value;
