#ifndef __CALCULATE_DEFINITIONS_HPP__
#define __CALCULATE_DEFINITIONS_HPP__

typedef char Byte;
typedef int Integer;
typedef unsigned Unsigned;
typedef unsigned long Hash;
typedef double Value;

#ifdef __cplusplus

#include <stdexcept>
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

#define nan std::numeric_limits<Value>::quiet_NaN()

typedef bool Bool;
typedef std::size_t SizeT;

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

#endif

#endif
