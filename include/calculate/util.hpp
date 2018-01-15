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
class SymbolContainer final : std::unordered_map<std::string, Kind> {
public:
    using Base = std::unordered_map<std::string, Kind>;

    using typename Base::key_type;
    using typename Base::mapped_type;
    using typename Base::value_type;
    using typename Base::iterator;
    using typename Base::const_iterator;

private:
    std::shared_ptr<Lexer> _lexer;

public:
    template<
         typename LexerType,
         std::enable_if_t<std::is_base_of<Lexer, LexerType>::value>* = nullptr
    >
    SymbolContainer(
        const LexerType& lexer,
        std::initializer_list<value_type> list={}
    ) : SymbolContainer{std::make_shared<LexerType>(lexer), list} {}

    SymbolContainer(
        const std::shared_ptr<Lexer>& lexer,
        std::initializer_list<value_type> list={}
    ) : Base{list}, _lexer{lexer} {}

    template<
         typename LexerType,
         std::enable_if_t<std::is_base_of<Lexer, LexerType>::value>* = nullptr
    >
    SymbolContainer(
        const LexerType& lexer,
        iterator begin,
        iterator end
    ) : SymbolContainer{std::make_shared<LexerType>(lexer), begin, end} {}

    SymbolContainer(
        const std::shared_ptr<Lexer>& lexer,
        iterator begin,
        iterator end
    ) : Base{begin, end}, _lexer{lexer} {}

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
