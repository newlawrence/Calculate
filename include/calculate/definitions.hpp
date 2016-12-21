#ifndef __CALCULATE_DEFINITIONS_HPP__
#define __CALCULATE_DEFINITIONS_HPP__

typedef char Byte;
typedef unsigned Unsigned;
typedef unsigned long Hash;
typedef double Value;

#ifdef __cplusplus

#include <memory>
#include <limits>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <regex>
#include <unordered_map>

#define TypeString(TOKEN) _TypeString<decltype(TOKEN##_tstr)>
#define nan std::numeric_limits<Value>::quiet_NaN()

typedef bool Bool;

namespace calculate_definitions {

    using Stream = std::ostringstream;
    using String = std::string;
    using vString = std::vector<String>;

    using pValue = std::unique_ptr<Value[]>;
    using vValue = std::vector<Value>;
    using mValue = std::unordered_map<String, Value>;
    using fValue = std::function<Value(const vValue&)>;

    using Regex = std::regex;
    using Match = std::smatch;

}

template <char... chars>
using _TypeStringType = std::integer_sequence<char, chars...>;

template <typename Type, Type... chars>
constexpr _TypeStringType<chars...> operator""_tstr() { return {}; }

template <typename Type>
struct _TypeString;

template <char... chars>
struct _TypeString<_TypeStringType<chars...>> {
    static const char* const str() {
        static constexpr char _str[sizeof...(chars) + 1] = { chars..., '\0' };
        return _str;
    }
};

#endif

#endif
