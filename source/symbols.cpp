#include <cmath>

#include "calculate/symbols.hpp"


namespace calculate_symbols {

    mSymbolGen Constant::_symbols;
    mSymbolGen Operator::_symbols;
    mSymbolGen Function::_symbols;


    void Evaluable::addBranches(const vEvaluable &x) noexcept {
        if (x.size() == args)
            _operands = x;
    }
 
    Value Evaluable::evaluate() const noexcept {
        vValue x(args);
        for (auto i = 0u; i < args; i++)
            x[i] = _operands[i]->evaluate();
        return _function(x);
    }

    void Evaluable::print(Stream &stream, String ind) const noexcept {
        if (ind.size() == 0)
            stream << "[" << token << "]\n";
        else if (ind.size() == 3)
            stream << " \\_[" << token << "]\n";
        else {
            stream << ind.substr(0, ind.size() - 3);
            stream << " \\_[" << token << "]\n";
        }
        if (args > 0) {
            for (auto i = 0u; i < args - 1; i++)
                _operands[i]->print(stream, ind + " | ");
            _operands[args - 1]->print(stream, ind + "   ");
        }
    }

}


RECORD_CONSTANT("pi", 3.141592653589)
RECORD_CONSTANT("e", 2.718281828459)
RECORD_CONSTANT("phi", 1.618033988749)
RECORD_CONSTANT("gamma", 0.577215664901)

RECORD_OPERATOR("+", 200, true, [](vArg x){return x[0] + x[1];})
RECORD_OPERATOR("-", 200, true, [](vArg x){return x[0] - x[1];})
RECORD_OPERATOR("*", 400, true, [](vArg x){return x[0] * x[1];})
RECORD_OPERATOR("/", 400, true, [](vArg x){return x[0] / x[1];})
RECORD_OPERATOR("%", 400, true, [](vArg x){return std::fmod(x[0], x[1]);})
RECORD_OPERATOR("^", 800, false, [](vArg x){return std::pow(x[0], x[1]);})
RECORD_OPERATOR("**", 800, false, [](vArg x){return std::pow(x[0], x[1]);})

RECORD_FUNCTION("fabs", 1, [](vArg x){return std::fabs(x[0]);})
RECORD_FUNCTION("abs", 1, [](vArg x){return std::abs(x[0]);})
RECORD_FUNCTION("fma", 3, [](vArg x){return std::fma(x[0], x[1], x[2]);})
RECORD_FUNCTION("copysign", 2, [](vArg x){return std::copysign(x[0], x[1]);})
RECORD_FUNCTION("nextafter", 2, [](vArg x){return std::nextafter(x[0], x[1]);})

RECORD_FUNCTION("fdim", 2, [](vArg x){return std::fdim(x[0], x[1]);})
RECORD_FUNCTION("fmax", 2, [](vArg x){return std::fmax(x[0], x[1]);})
RECORD_FUNCTION("fmin", 2, [](vArg x){return std::fmin(x[0], x[1]);})

RECORD_FUNCTION("ceil", 1, [](vArg x){return std::ceil(x[0]);})
RECORD_FUNCTION("floor", 1, [](vArg x){return std::floor(x[0]);})
RECORD_FUNCTION("fmod", 2, [](vArg x){return std::fmod(x[0], x[1]);})
RECORD_FUNCTION("trunc", 1, [](vArg x){return std::trunc(x[0]);})
RECORD_FUNCTION("round", 1, [](vArg x){return std::round(x[0]);})
RECORD_FUNCTION("rint", 1, [](vArg x){return std::rint(x[0]);})
RECORD_FUNCTION("nearbyint", 1, [](vArg x){return std::nearbyint(x[0]);})
RECORD_FUNCTION("remainder", 2, [](vArg x){return std::remainder(x[0], x[1]);})

RECORD_FUNCTION("pow", 2, [](vArg x){return std::pow(x[0], x[1]);})
RECORD_FUNCTION("sqrt", 1, [](vArg x){return std::sqrt(x[0]);})
RECORD_FUNCTION("cbrt", 1, [](vArg x){return std::cbrt(x[0]);})
RECORD_FUNCTION("hypot", 2, [](vArg x){return std::hypot(x[0], x[1]);})

RECORD_FUNCTION("exp", 1, [](vArg x){return std::exp(x[0]);})
RECORD_FUNCTION("expm1", 1, [](vArg x){return std::expm1(x[0]);})
RECORD_FUNCTION("exp2", 1, [](vArg x){return std::exp2(x[0]);})
RECORD_FUNCTION("log", 1, [](vArg x){return std::log(x[0]);})
RECORD_FUNCTION("log10", 1, [](vArg x){return std::log10(x[0]);})
RECORD_FUNCTION("log1p", 1, [](vArg x){return std::log1p(x[0]);})
RECORD_FUNCTION("log2", 1, [](vArg x){return std::log2(x[0]);})
RECORD_FUNCTION("logb", 1, [](vArg x){return std::logb(x[0]);})

RECORD_FUNCTION("sin", 1, [](vArg x){return std::sin(x[0]);})
RECORD_FUNCTION("cos", 1, [](vArg x){return std::cos(x[0]);})
RECORD_FUNCTION("tan", 1, [](vArg x){return std::tan(x[0]);})
RECORD_FUNCTION("asin", 1, [](vArg x){return std::asin(x[0]);})
RECORD_FUNCTION("acos", 1, [](vArg x){return std::acos(x[0]);})
RECORD_FUNCTION("atan", 1, [](vArg x){return std::atan(x[0]);})
RECORD_FUNCTION("atan2", 2, [](vArg x){return std::atan2(x[0], x[1]);})

RECORD_FUNCTION("sinh", 1, [](vArg x){return std::sinh(x[0]);})
RECORD_FUNCTION("cosh", 1, [](vArg x){return std::cosh(x[0]);})
RECORD_FUNCTION("tanh", 1, [](vArg x){return std::tanh(x[0]);})
RECORD_FUNCTION("asinh", 1, [](vArg x){return std::asinh(x[0]);})
RECORD_FUNCTION("acosh", 1, [](vArg x){return std::acosh(x[0]);})
RECORD_FUNCTION("atanh", 1, [](vArg x){return std::atanh(x[0]);})

RECORD_FUNCTION("erf", 1, [](vArg x){return std::erf(x[0]);})
RECORD_FUNCTION("erfc", 1, [](vArg x){return std::erfc(x[0]);})
RECORD_FUNCTION("tgamma", 1, [](vArg x){return std::tgamma(x[0]);})
RECORD_FUNCTION("lgamma", 1, [](vArg x){return std::lgamma(x[0]);})
