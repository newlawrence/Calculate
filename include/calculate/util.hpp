#ifndef __CALCULATE_UTIL_HPP__
#define __CALCULATE_UTIL_HPP__

#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>


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


template<typename Kind, typename Type, typename Lexer>
class SymbolMap final : std::unordered_map<std::string, Kind> {
public:
    using Base = std::unordered_map<std::string, Kind>;

    using Base::key_type;
    using Base::mapped_type;
    using Base::value_type;
    using Base::iterator;
    using Base::const_iterator;

private:
    std::shared_ptr<Lexer> _lexer;

public:
    SymbolMap(const std::shared_ptr<Lexer>& lexer) : _lexer{lexer} {}

    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;

    using Base::empty;
    using Base::size;
    using Base::find;
    using Base::count;

    using Base::erase;
    using Base::clear;
    using Base::swap;
    using Base::reserve;
};

}

}

#endif
