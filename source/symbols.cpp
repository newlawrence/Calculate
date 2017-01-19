#include <cmath>

#include "calculate/symbols.hpp"


namespace calculate_symbols {

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
        if (args == _operands.size()) {
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

}


RECORD_CONSTANT("pi",3.141592653589)
RECORD_CONSTANT("e",2.718281828459)
RECORD_CONSTANT("phi",1.618033988749)
RECORD_CONSTANT("gamma",0.577215664901)

RECORD_OPERATOR("+",2,true,true,[](Value x,Value y){return x+y;})
RECORD_OPERATOR("-",2,true,true,[](Value x,Value y){return x-y;})
RECORD_OPERATOR("*",4,true,false,[](Value x,Value y){return x*y;})
RECORD_OPERATOR("/",4,true,false,[](Value x,Value y){return x/y;})
RECORD_OPERATOR("%",4,true,false,[](Value x,Value y){return std::fmod(x,y);})
RECORD_OPERATOR("^",8,false,false,[](Value x,Value y){return std::pow(x,y);})
RECORD_OPERATOR("**",8,false,false,[](Value x,Value y){return std::pow(x,y);})

RECORD_FUNCTION("fabs",[](Value x){return std::fabs(x);})
RECORD_FUNCTION("abs",[](Value x){return std::abs(x);})
RECORD_FUNCTION("fma",[](Value x,Value y,Value z){return std::fma(x,y,z);})
RECORD_FUNCTION("copysign",[](Value x,Value y){return std::copysign(x,y);})
RECORD_FUNCTION("nextafter",[](Value x,Value y){return std::nextafter(x,y);})

RECORD_FUNCTION("fdim",[](Value x,Value y){return std::fdim(x,y);})
RECORD_FUNCTION("fmax",[](Value x,Value y){return std::fmax(x,y);})
RECORD_FUNCTION("fmin",[](Value x,Value y){return std::fmin(x,y);})

RECORD_FUNCTION("ceil",[](Value x){return std::ceil(x);})
RECORD_FUNCTION("floor",[](Value x){return std::floor(x);})
RECORD_FUNCTION("fmod",[](Value x,Value y){return std::fmod(x,y);})
RECORD_FUNCTION("trunc",[](Value x){return std::trunc(x);})
RECORD_FUNCTION("round",[](Value x){return std::round(x);})
RECORD_FUNCTION("rint",[](Value x){return std::rint(x);})
RECORD_FUNCTION("nearbyint",[](Value x){return std::nearbyint(x);})
RECORD_FUNCTION("remainder",[](Value x,Value y){return std::remainder(x,y);})

RECORD_FUNCTION("pow",[](Value x,Value y){return std::pow(x,y);})
RECORD_FUNCTION("sqrt",[](Value x){return std::sqrt(x);})
RECORD_FUNCTION("cbrt",[](Value x){return std::cbrt(x);})
RECORD_FUNCTION("hypot",[](Value x,Value y){return std::hypot(x,y);})

RECORD_FUNCTION("exp",[](Value x){return std::exp(x);})
RECORD_FUNCTION("expm1",[](Value x){return std::expm1(x);})
RECORD_FUNCTION("exp2",[](Value x){return std::exp2(x);})
RECORD_FUNCTION("log",[](Value x){return std::log(x);})
RECORD_FUNCTION("log10",[](Value x){return std::log10(x);})
RECORD_FUNCTION("log1p",[](Value x){return std::log1p(x);})
RECORD_FUNCTION("log2",[](Value x){return std::log2(x);})
RECORD_FUNCTION("logb",[](Value x){return std::logb(x);})

RECORD_FUNCTION("sin",[](Value x){return std::sin(x);})
RECORD_FUNCTION("cos",[](Value x){return std::cos(x);})
RECORD_FUNCTION("tan",[](Value x){return std::tan(x);})
RECORD_FUNCTION("asin",[](Value x){return std::asin(x);})
RECORD_FUNCTION("acos",[](Value x){return std::acos(x);})
RECORD_FUNCTION("atan",[](Value x){return std::atan(x);})
RECORD_FUNCTION("atan2",[](Value x,Value y){return std::atan2(x,y);})

RECORD_FUNCTION("sinh",[](Value x){return std::sinh(x);})
RECORD_FUNCTION("cosh",[](Value x){return std::cosh(x);})
RECORD_FUNCTION("tanh",[](Value x){return std::tanh(x);})
RECORD_FUNCTION("asinh",[](Value x){return std::asinh(x);})
RECORD_FUNCTION("acosh",[](Value x){return std::acosh(x);})
RECORD_FUNCTION("atanh",[](Value x){return std::atanh(x);})

RECORD_FUNCTION("erf",[](Value x){return std::erf(x);})
RECORD_FUNCTION("erfc",[](Value x){return std::erfc(x);})
RECORD_FUNCTION("tgamma",[](Value x){return std::tgamma(x);})
RECORD_FUNCTION("lgamma",[](Value x){return std::lgamma(x);})
