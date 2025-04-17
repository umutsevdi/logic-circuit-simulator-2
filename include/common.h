#pragma once
/*******************************************************************************
 * \file
 * File: include/common.h
 * Created: 02/04/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/lc-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <functional>
#include <iostream>

#ifdef _WIN32
#define SEP '\\'
#else
#define SEP '/'
#endif

#define VERSION 1

int __expect(std::function<bool(void)> expr, const char* msg) noexcept;

int __expect_with_message(std::function<bool(void)> expr, const char* function,
    const char* file, int line, const char* str_expr) noexcept;

#define LOG_PRE(STATUS)                                                        \
    std::cout << STATUS " " << __FILE_NAME__ << " : " << __LINE__ << "\t"      \
              << __FUNCTION__ << "(...)\t"
#define CLASS << *this << " "
#ifndef NDEBUG
#define L_INFO(...) LOG_PRE("INFO ") __VA_ARGS__ << std::endl
#define L_WARN(...) LOG_PRE("WARN ") __VA_ARGS__ << std::endl
#define L_ERROR(...) LOG_PRE("ERROR") __VA_ARGS__ << std::endl
#define S_ERROR(msg, ...) (L_ERROR(msg)), __VA_ARGS__
#define ERROR(msg) (L_ERROR(#msg)), msg
#define lcs_assert(expr)                                                       \
    {                                                                          \
        if (__expect_with_message([&]() mutable -> bool { return expr; },      \
                __FUNCTION__, __FILE_NAME__, __LINE__, #expr))                 \
            exit(1);                                                           \
    }
#else
#define L_INFO(...)
#define L_WARN(...) LOG_PRE("WARN ") __VA_ARGS__ << std::endl
#define L_ERROR(...) LOG_PRE("ERROR") __VA_ARGS__ << std::endl
#define ERROR(msg, ...) (L_ERROR(msg)), __VA_ARGS__
#define S_ERROR(msg) (L_ERROR(#msg)), msg
#define lcs_assert(expr)                                                       \
    {                                                                          \
        __expect_with_message([&]() mutable -> bool { return expr; },          \
            __FUNCTION__, __FILE_NAME__, __LINE__, #expr);                     \
    }
#endif

namespace lcs {
/**
 * A custom container class that stores a pointer to an object defined within
 * a scene. Can not be stored, copied or assigned.
 *
 * Intended use case:
 * get_node<lcs::Gate>(id)->signal();
 * get_node<lcs::Input>(id)->toggle();
 */
template <typename T> class NRef {
public:
    NRef(T* _v)
        : v(_v) { };
    NRef(NRef&&)                 = default;
    NRef(const NRef&)            = delete;
    NRef& operator=(NRef&&)      = delete;
    NRef& operator=(const NRef&) = delete;
    ~NRef() { }

    bool operator==(void* t) const { return v == t; };
    bool operator!=(void* t) const { return v != t; };
    T* operator->() { return v; }
    T* raw() const { return v; }

    friend std::ostream& operator<<(std::ostream& os, const NRef<T>& g)
    {
        if (g.v != nullptr) { os << (*g.v); }
        return os;
    }

private:
    T* v;
};

} // namespace lcs
