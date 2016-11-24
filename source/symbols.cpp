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

RECORD_FUNCTION(fabs, std::fabs(x[0]))
RECORD_FUNCTION(abs, std::abs(x[0]))
RECORD_FUNCTION(fma, std::fma(x[0], x[1], x[2]))
RECORD_FUNCTION(copysign, std::copysign(x[0], x[1]))
RECORD_FUNCTION(nextafter, std::nextafter(x[0], x[1]))

RECORD_FUNCTION(fdim, std::fdim(x[0], x[1]))
RECORD_FUNCTION(fmax, std::fmax(x[0], x[1]))
RECORD_FUNCTION(fmin, std::fmin(x[0], x[1]))

RECORD_FUNCTION(ceil, std::ceil(x[0]))
RECORD_FUNCTION(floor, std::floor(x[0]))
RECORD_FUNCTION(fmod, std::fmod(x[0], x[1]))
RECORD_FUNCTION(trunc, std::trunc(x[0]))
RECORD_FUNCTION(round, std::round(x[0]))
RECORD_FUNCTION(rint, std::rint(x[0]))
RECORD_FUNCTION(nearbyint, std::nearbyint(x[0]))
RECORD_FUNCTION(remainder, std::remainder(x[0], x[1]))

RECORD_FUNCTION(pow, std::pow(x[0], x[1]))
RECORD_FUNCTION(sqrt, std::sqrt(x[0]))
RECORD_FUNCTION(cbrt, std::cbrt(x[0]))
RECORD_FUNCTION(hypot, std::hypot(x[0], x[1]))

RECORD_FUNCTION(exp, std::exp(x[0]))
RECORD_FUNCTION(expm1, std::expm1(x[0]))
RECORD_FUNCTION(exp2, std::exp2(x[0]))
RECORD_FUNCTION(log, std::log(x[0]))
RECORD_FUNCTION(log10, std::log10(x[0]))
RECORD_FUNCTION(log1p, std::log1p(x[0]))
RECORD_FUNCTION(log2, std::log2(x[0]))
RECORD_FUNCTION(logb, std::logb(x[0]))

RECORD_FUNCTION(sin, std::sin(x[0]))
RECORD_FUNCTION(cos, std::cos(x[0]))
RECORD_FUNCTION(tan, std::tan(x[0]))
RECORD_FUNCTION(asin, std::asin(x[0]))
RECORD_FUNCTION(acos, std::acos(x[0]))
RECORD_FUNCTION(atan, std::atan(x[0]))
RECORD_FUNCTION(atan2, std::atan2(x[0], x[1]))

RECORD_FUNCTION(sinh, std::sinh(x[0]))
RECORD_FUNCTION(cosh, std::cosh(x[0]))
RECORD_FUNCTION(tanh, std::tanh(x[0]))
RECORD_FUNCTION(asinh, std::asinh(x[0]))
RECORD_FUNCTION(acosh, std::acosh(x[0]))
RECORD_FUNCTION(atanh, std::atanh(x[0]))

RECORD_FUNCTION(erf, std::erf(x[0]))
RECORD_FUNCTION(erfc, std::erfc(x[0]))
RECORD_FUNCTION(tgamma, std::tgamma(x[0]))
RECORD_FUNCTION(lgamma, std::lgamma(x[0]))
