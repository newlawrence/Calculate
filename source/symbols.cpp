#include <cmath>

#include "calculate/symbols.hpp"


namespace calculate_symbols {

    mSymbolGen Constant::_symbols;
    mSymbolGen Operator::_symbols;
    mSymbolGen Function::_symbols;


    void Evaluable::print(Stream &stream, String ind) const noexcept {
        if (ind.size() == 0)
            stream << "[" << token << "]\n";
        else if (ind.size() == 3)
            stream << " \\_[" << token << "]\n";
        else
            stream << ind.substr(0, ind.size() - 3) << " \\_[" << token << "]\n";
    }


    Constant::Recorder::Recorder(const String &t, fSymbolGen g) noexcept {
        _symbols[t] = g;
    }

    void Constant::print(Stream &stream, String ind) const noexcept {
        Evaluable::print(stream, ind);
    }

    bool Constant::hasToken(String t) {
        return _symbols.find(t) != _symbols.end();
    }

    vString Constant::queryTokens() {
        vString tokens;

        for (const auto& pair : _symbols)
            tokens.push_back(pair.first);
        return tokens;
    }


    Operator::Recorder::Recorder(const String &t, fSymbolGen g) noexcept {
        _symbols[t] = g;
    }

    void Operator::addBranches(pEvaluable l, pEvaluable r) noexcept {
        _left_operand = l;
        _right_operand = r;
    }

    void Operator::print(Stream &stream, String ind) const noexcept {
        Evaluable::print(stream, ind);
        _left_operand->print(stream, ind + " | ");
        _right_operand->print(stream, ind + "   ");
    }

    bool Operator::hasToken(String t) {
        return _symbols.find(t) != _symbols.end();
    }

    vString Operator::queryTokens() {
        vString tokens;

        for (const auto& pair : _symbols)
            tokens.push_back(pair.first);
        return tokens;
    }


    Function::Recorder::Recorder(const String &t, fSymbolGen g) noexcept {
        _symbols[t] = g;
    }

    void Function::addBranches(const vEvaluable &x) noexcept {
        if (x.size() == _operands.size())
            _operands = x;
    }

    void Function::print(Stream &stream, String ind) const noexcept {
        Evaluable::print(stream, ind);
        for (auto i = 0u; i < args - 1; i++)
            _operands[i]->print(stream, ind + " | ");
        _operands[args - 1]->print(stream, ind + "   ");
    }

    bool Function::hasToken(String t) {
        return _symbols.find(t) != _symbols.end();
    }

    vString Function::queryTokens() {
        vString tokens;

        for (const auto& pair : _symbols)
            tokens.push_back(pair.first);
        return tokens;
    }

}


RECORD_CONSTANT(pi, 3.141592653589)
RECORD_CONSTANT(e, 2.718281828459)
RECORD_CONSTANT(phi, 1.618033988749)
RECORD_CONSTANT(gamma, 0.577215664901)

RECORD_OPERATOR(plus, "+", 200, true, a + b)
RECORD_OPERATOR(minus, "-", 200, true, a - b)
RECORD_OPERATOR(asterisk, "*", 400, true, a * b)
RECORD_OPERATOR(slash, "/", 400, true, a / b)
RECORD_OPERATOR(percent, "%", 400, true, std::fmod(a,b))
RECORD_OPERATOR(caret, "^", 800, false, std::pow(a,b))
RECORD_OPERATOR(double_asterisk, "**", 800, false, std::pow(a,b))

RECORD_FUNCTION(fabs, 1, std::fabs(x[0]))
RECORD_FUNCTION(abs, 1, std::abs(x[0]))
RECORD_FUNCTION(fma, 3, std::fma(x[0], x[1], x[2]))
RECORD_FUNCTION(copysign, 2, std::copysign(x[0], x[1]))
RECORD_FUNCTION(nextafter, 2, std::nextafter(x[0], x[1]))

RECORD_FUNCTION(fdim, 2, std::fdim(x[0], x[1]))
RECORD_FUNCTION(fmax, 2, std::fmax(x[0], x[1]))
RECORD_FUNCTION(fmin, 2, std::fmin(x[0], x[1]))

RECORD_FUNCTION(ceil, 1, std::ceil(x[0]))
RECORD_FUNCTION(floor, 1, std::floor(x[0]))
RECORD_FUNCTION(fmod, 2, std::fmod(x[0], x[1]))
RECORD_FUNCTION(trunc, 1, std::trunc(x[0]))
RECORD_FUNCTION(round, 1, std::round(x[0]))
RECORD_FUNCTION(rint, 1, std::rint(x[0]))
RECORD_FUNCTION(nearbyint, 1, std::nearbyint(x[0]))
RECORD_FUNCTION(remainder, 2, std::remainder(x[0], x[1]))

RECORD_FUNCTION(pow, 2, std::pow(x[0], x[1]))
RECORD_FUNCTION(sqrt, 1, std::sqrt(x[0]))
RECORD_FUNCTION(cbrt, 1, std::cbrt(x[0]))
RECORD_FUNCTION(hypot, 2, std::hypot(x[0], x[1]))

RECORD_FUNCTION(exp, 1, std::exp(x[0]))
RECORD_FUNCTION(expm1, 1, std::expm1(x[0]))
RECORD_FUNCTION(exp2, 1, std::exp2(x[0]))
RECORD_FUNCTION(log, 1, std::log(x[0]))
RECORD_FUNCTION(log10, 1, std::log10(x[0]))
RECORD_FUNCTION(log1p, 1, std::log1p(x[0]))
RECORD_FUNCTION(log2, 1, std::log2(x[0]))
RECORD_FUNCTION(logb, 1, std::logb(x[0]))

RECORD_FUNCTION(sin, 1, std::sin(x[0]))
RECORD_FUNCTION(cos, 1, std::cos(x[0]))
RECORD_FUNCTION(tan, 1, std::tan(x[0]))
RECORD_FUNCTION(asin, 1, std::asin(x[0]))
RECORD_FUNCTION(acos, 1, std::acos(x[0]))
RECORD_FUNCTION(atan, 1, std::atan(x[0]))
RECORD_FUNCTION(atan2, 2, std::atan2(x[0], x[1]))

RECORD_FUNCTION(sinh, 1, std::sinh(x[0]))
RECORD_FUNCTION(cosh, 1, std::cosh(x[0]))
RECORD_FUNCTION(tanh, 1, std::tanh(x[0]))
RECORD_FUNCTION(asinh, 1, std::asinh(x[0]))
RECORD_FUNCTION(acosh, 1, std::acosh(x[0]))
RECORD_FUNCTION(atanh, 1, std::atanh(x[0]))

RECORD_FUNCTION(erf, 1, std::erf(x[0]))
RECORD_FUNCTION(erfc, 1, std::erfc(x[0]))
RECORD_FUNCTION(tgamma, 1, std::tgamma(x[0]))
RECORD_FUNCTION(lgamma, 1, std::lgamma(x[0]))
