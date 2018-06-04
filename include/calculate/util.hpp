/*
    Calculate - Version 2.1.1rc1
    Last modified 2018/06/04
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_UTIL_HPP__
#define __CALCULATE_UTIL_HPP__

#include <functional>
#include <memory>
#include <regex>
#include <tuple>
#include <type_traits>
#include <vector>

#include "exception.hpp"


namespace calculate {

namespace util {

template<typename T, typename U>
constexpr bool is_same = std::is_same<T, U>::value;

template<typename T, typename U>
constexpr bool is_base_of = std::is_base_of<T, U>::value;

template<typename T>
constexpr bool is_integral = std::is_integral<T>::value;

template<typename T>
constexpr bool is_copy_constructible = std::is_copy_constructible<T>::value;

template<typename T>
constexpr std::size_t tuple_size = std::tuple_size<T>::value;


namespace detail {

using std::begin;
using std::end;

template<typename Type>
constexpr decltype(
    begin(std::declval<Type&>()) != end(std::declval<Type&>()),
    ++std::declval<decltype(begin(std::declval<Type&>()))&>(),
    *begin(std::declval<Type&>()),
    bool{}
) is_iterable(int) { return true; }

template<typename>
constexpr bool is_iterable(...) { return false; }


template<typename Type, std::size_t>
using ExtractType = Type;

template<typename, std::size_t argc, typename = std::make_index_sequence<argc>>
struct MakeTuple {};

template<typename Type, std::size_t argc, std::size_t... indices>
struct MakeTuple<Type, argc, std::index_sequence<indices...>> {
    using type = std::tuple<ExtractType<Type, indices>...>;
};


template<typename Function, typename... Args>
constexpr bool is_noexcept =
    noexcept(std::declval<Function>()(std::declval<Args>()...));

template<typename Type, typename = void>
struct Traits : Traits<decltype(&Type::operator())> {};

template<typename Result, typename... Args>
struct Traits<std::function<Result(Args...)>, void> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(*)(Args...) noexcept,
    std::enable_if_t<is_noexcept<Result(*)(Args...) noexcept, Args...>>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(*)(Args...),
    std::enable_if_t<!is_noexcept<Result(*)(Args...), Args...>>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) noexcept,
    std::enable_if_t<is_noexcept<Type, Args...>>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = false;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...),
    std::enable_if_t<!is_noexcept<Type, Args...>>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = false;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) const noexcept,
    std::enable_if_t<is_noexcept<Type, Args...>>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) const,
    std::enable_if_t<!is_noexcept<Type, Args...>>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

}

template<typename Type>
constexpr bool is_iterable = detail::is_iterable<Type>(0);

template<typename Type>
constexpr bool is_noexcept = detail::is_noexcept<Type>;


template<typename Type, std::size_t argc>
using Tuple = typename detail::MakeTuple<Type, argc>::type;

template<typename Function>
using Result = typename detail::Traits<Function>::result;

template<typename Function>
constexpr bool is_const = detail::Traits<Function>::constant;

template<typename Function>
using Arguments = typename detail::Traits<Function>::arguments;

template<typename Function>
constexpr std::size_t argc =
    util::tuple_size<typename detail::Traits<Function>::arguments>;

template<typename Type, typename Target>
constexpr bool not_same =
    !is_same<std::decay_t<Type>, Target> &&
    !is_base_of<Target, std::decay_t<Type>>;


template<typename Type>
const std::vector<Type>& to_vector(const std::vector<Type>& args) { return args; }

template<typename Type, typename... Args>
std::vector<Type> to_vector(Args&&... args) { return {std::forward<Args>(args)...}; }

template<typename Type, typename Args>
std::enable_if_t<is_iterable<Args>, std::vector<Type>>
to_vector(Args&& args) { return {std::begin(args), std::end(args)}; }


template<class Type>
void hash_combine(std::size_t& seed, const Type& object) {
    std::hash<Type> hasher;
    seed ^= hasher(object) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}

}

#endif
