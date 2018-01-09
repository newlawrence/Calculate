#ifndef __CALCULATE_UTIL_HPP__
#define __CALCULATE_UTIL_HPP__

#include <functional>
#include <regex>
#include <string>


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

}

}

#endif
