/*
    Calculate - Version 2.1.1rc3
    Last modified 2018/06/09
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_UTIL_HPP__
#define __CALCULATE_UTIL_HPP__

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

#include "exception.hpp"


namespace calculate {

namespace util {

template<typename T, typename U>
constexpr bool is_same_v = std::is_same<T, U>::value;

template<typename T, typename U>
constexpr bool is_base_of_v = std::is_base_of<T, U>::value;

template<typename T>
constexpr bool is_integral_v = std::is_integral<T>::value;

template<typename T>
constexpr bool is_copy_constructible_v = std::is_copy_constructible<T>::value;

template<typename T>
constexpr std::size_t tuple_size_v = std::tuple_size<T>::value;


namespace detail {

using std::begin;
using std::end;

template<typename T>
constexpr decltype(
    begin(std::declval<T&>()) != end(std::declval<T&>()),
    ++std::declval<decltype(begin(std::declval<T&>()))&>(),
    *begin(std::declval<T&>()),
    bool{}
) is_iterable(int) { return true; }

template<typename>
constexpr bool is_iterable(...) { return false; }


template<typename T, std::size_t>
using extract_type = T;

template<typename, std::size_t argc, typename = std::make_index_sequence<argc>>
struct make_tuple {};

template<typename T, std::size_t argc, std::size_t... indices>
struct make_tuple<T, argc, std::index_sequence<indices...>> {
    using type = std::tuple<extract_type<T, indices>...>;
};


template<typename T, typename... Args>
constexpr bool is_noexcept_v = noexcept(std::declval<T>()(std::declval<Args>()...));

template<bool c, typename R, typename... Args>
struct TraitsHandler {
    static constexpr bool is_const_v = c;
    using result_t = R;
    using args_tuple_t = std::tuple<std::decay_t<Args>...>;
};

template<typename T, typename = void>
struct Traits : Traits<decltype(&std::decay_t<T>::operator())> {};

template<typename R, typename... Args>
struct Traits<std::function<R(Args...)>> : TraitsHandler<true, R, Args...> {};

template<typename R, typename... Args>
struct Traits<
    R(*)(Args...) noexcept,
    std::enable_if_t<is_noexcept_v<R(Args...) noexcept, Args...>>
> : TraitsHandler<true, R, Args...> {};

template<typename R, typename... Args>
struct Traits<
    R(*)(Args...),
    std::enable_if_t<!is_noexcept_v<R(Args...), Args...>>
> : TraitsHandler<true, R, Args...> {};

template<typename R, typename... Args>
struct Traits<
    R(&)(Args...) noexcept,
    std::enable_if_t<is_noexcept_v<R(Args...) noexcept, Args...>>
> : TraitsHandler<true, R, Args...> {};

template<typename R, typename... Args>
struct Traits<
    R(&)(Args...),
    std::enable_if_t<!is_noexcept_v<R(Args...), Args...>>
> : TraitsHandler<true, R, Args...> {};

template<typename T, typename R, typename... Args>
struct Traits<
    R(T::*)(Args...) noexcept,
    std::enable_if_t<is_noexcept_v<T, Args...>>
> : TraitsHandler<false, R, Args...> {};

template<typename T, typename R, typename... Args>
struct Traits<
    R(T::*)(Args...),
    std::enable_if_t<!is_noexcept_v<T, Args...>>
> : TraitsHandler<false, R, Args...> {};

template<typename T, typename R, typename... Args>
struct Traits<
    R(T::*)(Args...) const noexcept,
    std::enable_if_t<is_noexcept_v<T, Args...>>
> : TraitsHandler<true, R, Args...> {};

template<typename T, typename R, typename... Args>
struct Traits<
    R(T::*)(Args...) const,
    std::enable_if_t<!is_noexcept_v<T, Args...>>
> : TraitsHandler<true, R, Args...> {};

}

template<typename T>
constexpr bool is_iterable_v = detail::is_iterable<T>(0);

template<typename T>
constexpr bool is_noexcept_v = detail::is_noexcept_v<T>;


template<typename T, std::size_t argc>
using make_tuple_t = typename detail::make_tuple<T, argc>::type;

template<typename T>
using result_t = typename detail::Traits<T>::result_t;

template<typename T>
constexpr bool is_const_v = detail::Traits<T>::is_const_v;

template<typename T>
using args_tuple_t = typename detail::Traits<T>::args_tuple_t;

template<typename T>
constexpr std::size_t argc_v =
    tuple_size_v<typename detail::Traits<T>::args_tuple_t>;

template<typename T, typename U>
constexpr bool not_same_v =
    !is_same_v<std::decay_t<T>, U> &&
    !is_base_of_v<U, std::decay_t<T>>;


template<typename T, typename... Args>
std::vector<T> to_vector(Args&&... args) { return {std::forward<Args>(args)...}; }

template<typename T, typename Args>
std::enable_if_t<is_iterable_v<Args>, std::vector<T>>
to_vector(Args&& args) { return {std::begin(args), std::end(args)}; }


template<class T>
void hash_combine(std::size_t& seed, const T& object) {
    std::hash<T> hasher;
    seed ^= hasher(object) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}

}

#endif
