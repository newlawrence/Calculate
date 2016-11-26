#ifndef __CALCULATE_DEFINITIONS_HPP__
#define __CALCULATE_DEFINITIONS_HPP__

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

#define nan std::numeric_limits<Value>::quiet_NaN


namespace calculate_definitions {

    using Stream = std::ostringstream;
    using Byte = char;
    using String = std::string;
    using vString = std::vector<String>;

    using Unsigned = unsigned;
    using Hash = unsigned long;

    using Value = double;
    using pValue = std::unique_ptr<Value[]>;
    using vValue = std::vector<Value>;
    using mValue = std::unordered_map<String, Value>;

    using Regex = std::regex;
    using Match = std::smatch;

}

#endif
