#pragma once

#include <format>
#include <iostream>
#include <source_location>
namespace Color {
enum Code {
  FG_RED = 31,
  FG_GREEN = 32,
  FG_BLUE = 34,
  FG_DEFAULT = 39,
  BG_RED = 41,
  BG_GREEN = 42,
  BG_BLUE = 44,
  BG_DEFAULT = 49
};
class Modifier {

public:
  Code code;

  Modifier(Code pCode) : code(pCode) {}
  friend std::ostream &operator<<(std::ostream &os, const Modifier &mod) {
    return os << "\033[" << mod.code << "m";
  }
};

} // namespace Color

#ifndef NDEBUG
#define LOG(...)                                                               \
  std::cout << Color::Modifier(Color::FG_BLUE) << "LOG "                       \
            << std::source_location::current().file_name() << ":"              \
            << std::source_location::current().line() << " | "                 \
            << Color::Modifier(Color::FG_DEFAULT) << std::format(__VA_ARGS__)  \
            << "\n";
#elif
#define LOG(expr)
#endif

#define LOG_ERR(...)                                                           \
  std::cerr << Color::Modifier(Color::FG_RED) << "ERR "                        \
            << std::source_location::current().file_name() << ":"              \
            << std::source_location::current().line() << " | "                 \
            << Color::Modifier(Color::FG_DEFAULT) << std::format(__VA_ARGS__)  \
            << "\n";
