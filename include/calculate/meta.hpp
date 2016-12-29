#ifndef __CALCULATE_META_HPP__
#define __CALCULATE_META_HPP__

#include <type_traits>
#include <tuple>

#include "calculate/definitions.hpp"

#define RECORD_TOKEN(TOKEN)                                                   \
template <>                                                                   \
constexpr const char* StringLiteral<calculate_meta::hashDJB2(TOKEN)>::str() { \
    return TOKEN;                                                             \
}
#define TypeString(TOKEN) StringLiteral<calculate_meta::hashDJB2(TOKEN)>


namespace calculate_meta {

    using namespace calculate_definitions;

    constexpr Hash hashDJB2(const Byte *s, Hash h=5381) {
        return !*s ? h : hashDJB2(s + 1, 33 * h ^ static_cast<Unsigned>(*s));
    }
    
    
    template<typename Type>
    struct FunctionTraits : FunctionTraits<decltype(&Type::operator())> {
    };

    template<typename Result, typename... Args>
    struct FunctionTraits<Result(Args...)> {
        using result_type = Result;
        using argument_types = std::tuple<Args...>;
    };

    template<typename Result, typename... Args>
    struct FunctionTraits<Result(*)(Args...)> {
        using result_type = Result;
        using argument_types = std::tuple<Args...>;
    };

    template<typename Result, typename... Args>
    struct FunctionTraits<std::function<Result(Args...)>> {
        using result_type = Result;
        using argument_types = std::tuple<Args...>;
    };

    template<typename Type, typename Result, typename... Args>
    struct FunctionTraits<Result(Type::*)(Args...)> {
        using result_type = Result;
        using argument_types = std::tuple<Args...>;
    };

    template<typename Type, typename Result, typename... Args>
    struct FunctionTraits<Result(Type::*)(Args...) const> {
        using result_type = Result;
        using argument_types = std::tuple<Args...>;
    };

    template<typename Type>
    using LambdaResult = typename FunctionTraits<Type>::result_type;

    template<typename Type>
    using LambdaParams = typename FunctionTraits<Type>::argument_types;

    template<typename Type>
    constexpr SizeT lambdaArgs() {
        return std::tuple_size<LambdaParams<Type>>::value;
    }


    template<typename Dependent, SizeT Index>
    using DependsOn = Dependent;

    template<
        typename Type,
        SizeT n,
        typename Indices = std::make_index_sequence<n>
    >
    struct Repeat;

    template<typename Type, SizeT n, SizeT... Indices>
    struct Repeat<Type, n, std::index_sequence<Indices...>> {
        using type = std::tuple<DependsOn<Type, Indices>...>;
    };

    template<SizeT n>
    using DoubleTuple = typename Repeat<Value, n>::type;


    template<typename Type, SizeT... Indices>
    Value evalVector(Type &function, const vValue &args,
                       std::index_sequence<Indices...>) {
        return function(args[Indices]...);
    }


    template<typename Functor, SizeT n>
    struct FunctorWrapper {
        Functor functor;
        constexpr SizeT args() { return n; };

        Value operator()(const vValue &args) {
            return evalVector(
                functor, args, std::make_index_sequence<n>()
            );
        }
    };

}

template <typename Functor>
auto wrapFunctor(Functor&& functor) {
    using namespace calculate_meta;

    auto introspective_functor = functor;
    using FunctorType = decltype(introspective_functor);

    static_assert(
        std::is_same<LambdaResult<FunctorType>, Value>::value,
        "Return type of builtin function must be double"
    );
    static_assert(
        lambdaArgs<FunctorType>() > 0,
        "At least one argument required for builtin function"
    );
    static_assert(
        std::is_same<
            LambdaParams<FunctorType>,
            DoubleTuple<lambdaArgs<FunctorType>()>
        >::value,
        "All type parameters of builtin function must be double"
    );

    return FunctorWrapper<Functor, lambdaArgs<FunctorType>()>{
        std::forward<Functor>(functor)
    };
}


template <Hash hash>
struct StringLiteral {
    static constexpr const char* str() {
        return "";
    }
};

#endif
