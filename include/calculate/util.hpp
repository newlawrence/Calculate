#ifndef __CALCULATE_UTIL_HPP__
#define __CALCULATE_UTIL_HPP__

#include <functional>
#include <memory>
#include <regex>
#include <unordered_map>

#include "exception.hpp"


namespace calculate {

namespace util {

template<class Type>
void hash_combine(std::size_t& seed, const Type& object) {
    std::hash<Type> hasher;
    seed ^= hasher(object) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline std::string replace(
    std::string where,
    const std::string& what,
    const std::string& by
) {
    std::size_t index = 0;
    while (true) {
        index = where.find(what, index);
        if (index == std::string::npos)
            break;
        where.replace(index, what.size(), by);
        index += what.size();
    }
    return where;
}


template<typename Kind, typename Parser>
class SymbolContainer final : std::unordered_map<std::string, Kind> {
    friend Parser;
    using Base = std::unordered_map<std::string, Kind>;
    using Lexer = typename Parser::Lexer;
    using Type = typename Parser::Type;

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
    std::shared_ptr<Lexer> _lexer;

    SymbolContainer(const std::shared_ptr<Lexer>& lexer) :
        _lexer{lexer}
    {}

    SymbolContainer(const SymbolContainer&) = default;
    SymbolContainer(SymbolContainer&&) = default;
    ~SymbolContainer() = default;
    SymbolContainer& operator=(const SymbolContainer&) = default;
    SymbolContainer& operator=(SymbolContainer&&) = default;

    void _validate(const std::string& key, Constant*) {
        if (!std::regex_match(key, _lexer->name_regex))
            throw UnsuitableName{key};
    }

    void _validate(const std::string& key, Function*) {
        if (!std::regex_match(key, _lexer->name_regex))
            throw UnsuitableName{key};
    }

    void _validate(const std::string& key, Operator*) {
        if (!std::regex_match(key, _lexer->symbol_regex))
            throw UnsuitableName{key};
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
        _validate(key, static_cast<mapped_type*>(nullptr));
        return Base::operator[](key);
    }

    mapped_type& operator[](key_type&& key) {
        _validate(key, static_cast<mapped_type*>(nullptr));
        return Base::operator[](key);
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(const std::string& key, Args&&... args) {
        _validate(key, static_cast<mapped_type*>(nullptr));
        return Base::emplace(key, std::forward<Args>(args)...);
    }

};

}

}

#endif
