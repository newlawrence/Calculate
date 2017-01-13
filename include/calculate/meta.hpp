#ifndef __CALCULATE_META_HPP__
#define __CALCULATE_META_HPP__

#include <type_traits>
#include <functional>
#include <tuple>

#include "calculate/definitions.hpp"

#define TypeString(TOKEN) decltype(TOKEN##_tstr)


namespace calculate_meta_utility {

    using namespace calculate_definitions;

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
    using FunctionResult = typename FunctionTraits<Type>::result_type;

    template<typename Type>
    using FunctionParams = typename FunctionTraits<Type>::argument_types;

    template<typename Type>
    constexpr SizeT argCount() {
        return std::tuple_size<FunctionParams<Type>>::value;
    }


    template<typename Dependent, SizeT index>
    using DependsOn = Dependent;

    template<
        typename Type,
        SizeT n,
        typename indices = std::make_index_sequence<n>
    >
    struct Repeat;

    template<typename Type, SizeT n, SizeT... indices>
    struct Repeat<Type, n, std::index_sequence<indices...>> {
        using type = std::tuple<DependsOn<Type, indices>...>;
    };

    template<SizeT n>
    using FloatTuple = typename Repeat<Value, n>::type;


    constexpr bool anyArgs(SizeT a) { return a > 0; }

    template<SizeT a>
    using NonEmptyArgs = typename std::enable_if<anyArgs(a), Value>::type;

    template<SizeT a>
    using EmptyArgs = typename std::enable_if<!anyArgs(a), Value>::type;

}


namespace calculate_meta {

    using namespace calculate_meta_utility;

    class FunctionWrapper {

        class FunctionConcept {
        public:
            virtual SizeT args() const noexcept = 0;
            virtual Value evaluate(const vValue &args) const noexcept = 0;
        };

        template<typename Function, SizeT n>
        class FunctionModel : public FunctionConcept {
            Function _function;

            template<SizeT... indices>
            Value _evaluate(
                const vValue &args,
                std::index_sequence<indices...>,
                NonEmptyArgs<sizeof...(indices)>
            ) const noexcept { return _function(args[indices]...); }

            template<SizeT... indices>
            Value _evaluate(
                const vValue &args,
                std::index_sequence<indices...>,
                EmptyArgs<sizeof...(indices)>
            ) const noexcept { return _function(); }

        public:
            FunctionModel(Function function) : _function(function) {}

            virtual SizeT args() const noexcept { return n; };

            virtual Value evaluate(const vValue &args) const noexcept {
                return _evaluate(args, std::make_index_sequence<n>(), 0.);
            }
        };

        std::unique_ptr<FunctionConcept> _function;

        FunctionWrapper() = delete;
        FunctionWrapper(const FunctionWrapper&) = delete;
        FunctionWrapper(FunctionWrapper&&) = delete;
        FunctionWrapper& operator=(const FunctionWrapper&) = delete;
        FunctionWrapper& operator=(FunctionWrapper&&) = delete;

    public:
        template<typename Function>
        FunctionWrapper(Function&& function) {
            using namespace calculate_meta;

            static_assert(
                std::is_same<FunctionResult<Function>, Value>::value,
                "Return type of builtin function must be double"
            );
            static_assert(
                std::is_same<
                    FunctionParams<Function>,
                    FloatTuple<argCount<Function>()>
                >::value,
                "All type parameters of builtin function must be double"
            );

            _function = std::make_unique<
                FunctionModel<Function, argCount<Function>()>
            >(std::forward<Function>(function));
        }

        SizeT args() const noexcept {
            return _function->args();
        }

        Value operator()(const vValue &args) const noexcept {
            return _function->evaluate(args);
        }
    };

}


template <char ... chars>
struct StringLiteral {
    static constexpr const char str[sizeof...(chars) + 1] = {chars..., '\0'};
    constexpr operator const char* (void) const {
        return str;
    }
};
template <char... chars>
constexpr const char StringLiteral<chars...>::str[sizeof...(chars) + 1];

template <typename Type, Type... chars>
constexpr StringLiteral<chars...> operator ""_tstr() { return {}; }

#endif
