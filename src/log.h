/** \file log.h
 *  \brief Definition of logging macros.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <fmt/core.h>
#include <iostream>
#include <string.h>

#define FMT_HEADER_ONLY
template <typename Str, typename... Args> void pyg_logger(const char * level, const char * file, const int line, const Str & str, Args... args) {
    std::cout << level << ":[" << file << "|" << line << "] " << fmt::format(str, args...) << std::endl;
}
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#ifndef NDEBUG
#define PYG_LOG_V(...) pyg_logger("V", __FILENAME__, __LINE__, __VA_ARGS__)
#define PYG_LOG_D(...) pyg_logger("D", __FILENAME__, __LINE__, __VA_ARGS__)
#define PYG_LOG_E(...) pyg_logger("E", __FILENAME__, __LINE__, __VA_ARGS__)
#define PYG_LOG_I(...) pyg_logger("I", __FILENAME__, __LINE__, __VA_ARGS__)
#else // NDEBUG
#define PYG_LOG_V(...)
#define PYG_LOG_D(...)
#define PYG_LOG_E(...)
#define PYG_LOG_I(...)
#endif // NDEBUG
