#include <cmath>
#include <algorithm>

#include "symbols.h"


namespace symbols {

    mValue Constant::_symbols;
    mSymbolGen Operator::_symbols;
    String Operator::_regex_simple;
    String Operator::_regex_composite;
    mSymbolGen Function::_symbols;


    pSymbol newSymbol(double *v) {
        return pSymbol(new Variable(v));
    }

    pSymbol newSymbol(const String &t) {
        if ([&t] {
                try {std::stod(t); return true;}
                catch (std::logic_error) {return false;}
            }())
            return pSymbol(new Constant(t));
        else if (Constant::_symbols.find(t) != Constant::_symbols.end())
            return pSymbol(
                new Constant(std::to_string(Constant::_symbols[t]))
            );
        else if (t == "(")
            return pSymbol(new Parenthesis<'('>);
        else if (t == ")")
            return pSymbol(new Parenthesis<')'>);
        else if (t == ",")
            return pSymbol(new Separator);
        else if (Operator::_symbols.find(t) != Operator::_symbols.end())
            return Operator::_symbols[t]();
        else if (Function::_symbols.find(t) != Function::_symbols.end())
            return Function::_symbols[t]();
        else
            throw UndefinedSymbolException();
    }


    Constant::Recorder::Recorder(const String &t, double v) noexcept {
        Constant::_symbols[t] = v;
    }


    Operator::Recorder::Recorder(const String &t, fSymbolGen g) noexcept {
        Operator::_symbols[t] = g;
        if (std::all_of(
            t.begin(), t.end(), [&t](char ch) {return ch == t[0];})
            )
            _regex_simple += t;
        else
            _regex_composite += "|" + t;
    }

    void Operator::addBranches(pSymbol l, pSymbol r) noexcept {
        _left_operand = l;
        _right_operand = r;
    }

    String Operator::getSymbolsRegex() noexcept {
        return String("[") + _regex_simple + ",()]+" + _regex_composite;
    }


    Function::Recorder::Recorder(const String &t, fSymbolGen g) noexcept {
        Function::_symbols[t] = g;
    }

    void Function::addBranches(vSymbol &&x) noexcept {
        _operands = std::move(x);
    }


    RECORD_CONSTANT(pi,3.141592653589)
    RECORD_CONSTANT(e,2.718281828459)
    RECORD_CONSTANT(phi,1.618033988749)
    RECORD_CONSTANT(gamma,0.577215664901)

    RECORD_OPERATOR(Sub,-,200,true,a-b)
    RECORD_OPERATOR(Add,+,200,true,a+b)
    RECORD_OPERATOR(Mul,*,400,true,a*b)
    RECORD_OPERATOR(Div,/,400,true,a/b)
    RECORD_OPERATOR(Mod,%,400,true,std::fmod(a,b))
    RECORD_OPERATOR(Pow,^,800,false,std::pow(a,b))
    RECORD_OPERATOR(Pow2,**,800,false,std::pow(a,b))

    RECORD_FUNCTION(fabs,1,std::fabs(x[0]))
    RECORD_FUNCTION(abs,1,std::abs(x[0]))
    RECORD_FUNCTION(fma,3,std::fma(x[0],x[1],x[2]))

    RECORD_FUNCTION(fdim,2,std::fdim(x[0],x[1]))
    RECORD_FUNCTION(fmax,2,std::fmax(x[0],x[1]))
    RECORD_FUNCTION(fmin,2,std::fmin(x[0],x[1]))
    RECORD_FUNCTION(copysign,2,std::copysign(x[0],x[1]))
    RECORD_FUNCTION(nextafter,2,std::nextafter(x[0],x[1]))

    RECORD_FUNCTION(ceil,1,std::ceil(x[0]))
    RECORD_FUNCTION(floor,1,std::floor(x[0]))
    RECORD_FUNCTION(fmod,2,std::fmod(x[0],x[1]))
    RECORD_FUNCTION(trunc,1,std::trunc(x[0]))
    RECORD_FUNCTION(round,1,std::round(x[0]))
    RECORD_FUNCTION(rint,1,std::rint(x[0]))
    RECORD_FUNCTION(nearbyint,1,std::nearbyint(x[0]))
    RECORD_FUNCTION(remainder,2,std::remainder(x[0],x[1]))

    RECORD_FUNCTION(pow,2,std::pow(x[0],x[1]))
    RECORD_FUNCTION(sqrt,1,std::sqrt(x[0]))
    RECORD_FUNCTION(cbrt,1,std::cbrt(x[0]))
    RECORD_FUNCTION(hypot,2,std::hypot(x[0],x[1]))    

    RECORD_FUNCTION(exp,1,std::exp(x[0]))
    RECORD_FUNCTION(expmone,1,std::expm1(x[0]))
    RECORD_FUNCTION(exptwo,1,std::exp2(x[0]))
    RECORD_FUNCTION(ln,1,std::log(x[0]))
    RECORD_FUNCTION(log,1,std::log10(x[0]))
    RECORD_FUNCTION(logonep,1,std::log1p(x[0]))
    RECORD_FUNCTION(logtwo,1,std::log2(x[0]))
    RECORD_FUNCTION(logb,1,std::logb(x[0]))

    RECORD_FUNCTION(sin,1,std::sin(x[0]))
    RECORD_FUNCTION(cos,1,std::cos(x[0]))
    RECORD_FUNCTION(tan,1,std::tan(x[0]))
    RECORD_FUNCTION(asin,1,std::asin(x[0]))
    RECORD_FUNCTION(acos,1,std::acos(x[0]))
    RECORD_FUNCTION(atan,1,std::atan(x[0]))
    RECORD_FUNCTION(atantwo,2,std::atan2(x[0],x[1]))

    RECORD_FUNCTION(sinh,1,std::sinh(x[0]))
    RECORD_FUNCTION(cosh,1,std::cosh(x[0]))
    RECORD_FUNCTION(tanh,1,std::tanh(x[0]))
    RECORD_FUNCTION(asinh,1,std::asinh(x[0]))
    RECORD_FUNCTION(acosh,1,std::acosh(x[0]))
    RECORD_FUNCTION(atanh,1,std::atanh(x[0]))

    RECORD_FUNCTION(erf,1,std::erf(x[0]))
    RECORD_FUNCTION(erfc,1,std::erfc(x[0]))
    RECORD_FUNCTION(tgamma,1,std::tgamma(x[0]))
    RECORD_FUNCTION(lgamma,1,std::lgamma(x[0]))

}
