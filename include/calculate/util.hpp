/*
    Calculate - Version 2.1.0dev0
    Last modified 2018/05/18
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_UTIL_HPP__
#define __CALCULATE_UTIL_HPP__

#include <functional>
#include <memory>
#include <regex>
#include <unordered_map>
#include <vector>

#include "exception.hpp"


namespace calculate {

namespace util {

namespace detail {

    using std::begin;
    using std::end;

    template<typename Type>
    static constexpr decltype(
        begin(std::declval<Type&>()) != end(std::declval<Type&>()),
        ++std::declval<decltype(begin(std::declval<Type&>()))&>(),
        *begin(std::declval<Type&>()),
        bool{}
    ) is_iterable(int) { return true; }

    template<typename Type>
    static constexpr bool is_iterable(...) { return false; }

}

template<typename Type>
struct Check {
    static constexpr bool iterable = detail::is_iterable<Type>(0);
};

template<typename Type, typename Args>
std::enable_if_t<Check<Args>::iterable, std::vector<Type>>
to_vector(Args&& args) { return {std::begin(args), std::end(args)}; }

template<typename Type, typename... Args>
std::vector<Type> to_vector(Args&&... args) {
    return {std::forward<Args>(args)...};
}

template<typename Type, typename Args>
std::enable_if_t<Check<Args>::iterable, std::vector<std::reference_wrapper<Type>>>
to_reference(Args&& args) { return {std::begin(args), std::end(args)}; }

template<typename Type, typename... Args>
std::vector<std::reference_wrapper<Type>> to_reference(Args&&... args) {
    return {std::ref(std::forward<Args>(args))...};
}


template<class Type>
void hash_combine(std::size_t& seed, const Type& object) {
    std::hash<Type> hasher;
    seed ^= hasher(object) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}


template<typename Kind, typename Parser>
class SymbolContainer final : std::unordered_map<std::string, Kind> {
    friend Parser;
    using Base = std::unordered_map<std::string, Kind>;
    using Lexer = typename Parser::Lexer;

    using Constant = typename Parser::Constant;
    using Function = typename Parser::Function;
    using Operator = typename Parser::Operator;

public:
    using typename Base::key_type;
    using typename Base::mapped_type;
    using typename Base::value_type;
    using typename Base::iterator;
    using typename Base::const_iterator;

private:
    Lexer* _lexer;

    SymbolContainer(Lexer* lexer) : _lexer{lexer} {}

    SymbolContainer(const SymbolContainer&) = default;
    SymbolContainer(SymbolContainer&&) = default;
    ~SymbolContainer() = default;

    SymbolContainer& operator=(const SymbolContainer&) = default;
    SymbolContainer& operator=(SymbolContainer&&) = default;

    void _validate(const std::string& key, Constant*) const {
        if (!std::regex_match(key, _lexer->name_regex))
            throw UnsuitableName{key};
    }

    void _validate(const std::string& key, Function*) const {
        if (!std::regex_match(key, _lexer->name_regex))
            throw UnsuitableName{key};
    }

    void _validate(const std::string& key, Operator*) const {
        if (!std::regex_match(key, _lexer->symbol_regex))
            throw UnsuitableName{key};
    }

    void _validate(const std::string& key) const {
        _validate(key, static_cast<mapped_type*>(nullptr));
    }

public:
    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;

    using Base::empty;
    using Base::size;
    using Base::find;
    using Base::count;
    using Base::at;

    using Base::erase;
    using Base::clear;
    using Base::swap;
    using Base::reserve;

    mapped_type& operator[](const key_type& key) {
        return Base::find(key)->second;
    }

    mapped_type& operator[](key_type&& key) {
        return Base::find(std::move(key))->second;
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(const std::string& key, Args&&... args) {
        _validate(key);
        return Base::emplace(key, std::forward<Args>(args)...);
    }

    std::pair<iterator, bool> insert(const value_type& value) {
        _validate(value.first);
        return Base::insert(value);
    }

    template<typename Value>
    std::pair<iterator, bool> insert(Value&& value) {
        _validate(value.first);
        return Base::insert(std::forward<Value>(value));
    }

    template<typename Iterator>
    void insert(Iterator first, Iterator last) {
        for (auto element = first; element != last; ++element)
            _validate(element->first);
        Base::insert(first, last);
    }

    void insert(std::initializer_list<value_type> list) {
        for (const auto& element : list)
            _validate(element.first);
        Base::insert(list);
    }
};

template<typename Parser>
class AliasContainer final : std::unordered_map<std::string, std::string> {
    friend Parser;
    using Base = std::unordered_map<std::string, std::string>;
    using Lexer = typename Parser::Lexer;

public:
    using typename Base::key_type;
    using typename Base::mapped_type;
    using typename Base::value_type;
    using typename Base::iterator;
    using typename Base::const_iterator;

private:
    Lexer* _lexer;

    AliasContainer(Lexer* lexer) : _lexer{lexer} {}

    AliasContainer(const AliasContainer&) = default;
    AliasContainer(AliasContainer&&) = default;
    ~AliasContainer() = default;

    AliasContainer& operator=(const AliasContainer&) = default;
    AliasContainer& operator=(AliasContainer&&) = default;

    void _validate(const std::string& key, const std::string& value) const {
        if (!std::regex_match(key, _lexer->symbol_regex))
            throw UnsuitableName{key};
        if (!std::regex_match(value, _lexer->name_regex))
            throw UnsuitableName{value};
    }

public:
    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;

    using Base::empty;
    using Base::size;
    using Base::find;
    using Base::count;
    using Base::at;

    using Base::erase;
    using Base::clear;
    using Base::swap;
    using Base::reserve;

    mapped_type& operator[](const key_type& key) {
        return Base::find(key)->second;
    }

    mapped_type& operator[](key_type&& key) {
        return Base::find(std::move(key))->second;
    }

    template<typename Value>
    std::pair<iterator, bool> emplace(const std::string& key, Value&& value) {
        _validate(key, value);
        return Base::emplace(key, std::forward<Value>(value));
    }

    std::pair<iterator, bool> insert(const value_type& value) {
        _validate(value.first, value.second);
        return Base::insert(value);
    }

    template<typename Value>
    std::pair<iterator, bool> insert(Value&& value) {
        _validate(value.first, value.second);
        return Base::insert(std::forward<Value>(value));
    }

    template<typename Iterator>
    void insert(Iterator first, Iterator last) {
        for (auto element = first; element != last; ++element)
            _validate(element->first, element->second);
        Base::insert(first, last);
    }

    void insert(std::initializer_list<value_type> list) {
        for (const auto& element : list)
            _validate(element.first, element.second);
        Base::insert(list);
    }
};

}

#endif
