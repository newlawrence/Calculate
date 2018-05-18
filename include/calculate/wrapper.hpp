/*
    Calculate - Version 2.1.0dev0
    Last modified 2018/05/18
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_WRAPPER_HPP__
#define __CALCULATE_WRAPPER_HPP__

#include <type_traits>
#include <tuple>
#include <utility>

#include "util.hpp"


namespace calculate {

namespace detail {

template<typename Type>
struct NoexceptRemover { using type = Type; };

template<typename Result, typename... Args>
struct NoexceptRemover<Result(Args...) noexcept> {
    using type = Result(Args...);
};

template<typename Result, typename... Args>
struct NoexceptRemover<Result(*)(Args...) noexcept> {
    using type = Result(*)(Args...);
};

template<typename Result, typename... Args>
struct NoexceptRemover<Result(* const)(Args...) noexcept> {
    using type = Result(* const)(Args...);
};

template<typename Type, typename Result, typename... Args>
struct NoexceptRemover<Result(Type::*)(Args...) noexcept> {
    using type = Result(Type::*)(Args...);
};

template<typename Type, typename Result, typename... Args>
struct NoexceptRemover<Result(Type::*)(Args...) const noexcept> {
    using type = Result(Type::*)(Args...) const;
};

template<typename Type>
using WithoutNoexcept = typename NoexceptRemover<Type>::type;


template<typename Type>
struct Traits : Traits<WithoutNoexcept<decltype(&Type::operator())>> {};

template<typename Result, typename... Args>
struct Traits<std::function<Result(Args...)>> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<Result(Args...)> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<Result(*)(Args...)> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<Result(* const)(Args...)> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Type, typename Result, typename... Args>
struct Traits<Result(Type::*)(Args...)> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = false;
};

template<typename Type, typename Result, typename... Args>
struct Traits<Result(Type::*)(Args...) const> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};


template<typename Function>
using Result = typename Traits<Function>::result;

template<typename Function>
using Arguments = typename Traits<Function>::arguments;

template<typename Function>
struct Argc {
    static constexpr std::size_t value =
        std::tuple_size<typename Traits<Function>::arguments>::value;
};

template<typename Function>
struct IsConst {
    static constexpr bool value = Traits<Function>::constant;
};

template<typename Type, typename Target>
struct NotSame {
    static constexpr bool value =
        !std::is_same<std::decay_t<Type>, Target>::value &&
        !std::is_base_of<Target, std::decay_t<Type>>::value;
};


template<typename Type, std::size_t>
using ExtractType = Type;

template<typename, std::size_t argc, typename = std::make_index_sequence<argc>>
struct Repeat {};

template<typename Type, std::size_t argc, std::size_t... indices>
struct Repeat<Type, argc, std::index_sequence<indices...>> {
    using type = std::tuple<ExtractType<Type, indices>...>;
};

template<typename Type, std::size_t argc>
using Repeated = typename Repeat<Type, argc>::type;

}


template<typename Type, typename Source>
struct WrapperConcept {
    virtual ~WrapperConcept() = default;
    virtual std::shared_ptr<WrapperConcept> clone() const noexcept = 0;
    virtual std::size_t argc() const noexcept = 0;
    virtual Type call(const std::vector<std::reference_wrapper<Type>>&) const = 0;
    virtual Type eval(const std::vector<Source>&) const = 0;
};


template<typename Type, typename Source = Type>
class Wrapper {
    friend struct std::hash<Wrapper>;

    using WrapperConcept = calculate::WrapperConcept<Type, Source>;

    template<typename Callable>
    struct Inspect {
        static constexpr bool not_self =
            detail::NotSame<Callable, Wrapper>::value;
        static constexpr bool is_model =
            std::is_base_of<WrapperConcept, Callable>::value;
    };

    template<typename Callable, typename Adapter, std::size_t argcount>
    class WrapperModel final : public WrapperConcept {
        Callable _callable;
        Adapter _adapter;

        template<std::size_t... indices>
        Type _invoke(
            const std::vector<std::reference_wrapper<Type>>& args,
            std::index_sequence<indices...>
        ) const {
            if (args.size() != argcount)
                throw ArgumentsMismatch{argcount, args.size()};
            return _callable(args[indices]...);
        }

        template<std::size_t... indices>
        Type _invoke(
            const std::vector<Source>& args,
            std::index_sequence<indices...>
        ) const {
            if (args.size() != argcount)
                throw ArgumentsMismatch{argcount, args.size()};
            return _callable(_adapter(args[indices])...);
        }

    public:
        WrapperModel(Callable callable, Adapter adapter) :
                _callable{callable},
                _adapter{adapter}
        {}

        std::shared_ptr<WrapperConcept> clone() const noexcept override {
            return std::make_shared<WrapperModel>(*this);
        }

        std::size_t argc() const noexcept override { return argcount; }

        Type call(const std::vector<std::reference_wrapper<Type>>& args) const override {
            return _invoke(args, std::make_index_sequence<argcount>{});
        }

        Type eval(const std::vector<Source>& args) const override {
            return _invoke(args, std::make_index_sequence<argcount>{});
        }
    };

	template<typename Callable, typename Adapter>
	using ModelType = WrapperModel<
        Callable,
        Adapter,
        detail::Argc<Callable>::value
    >;

    std::shared_ptr<WrapperConcept> _callable;

    Wrapper(std::shared_ptr<WrapperConcept>&& callable) : _callable{callable} {}

protected:
    Type _eval(const std::vector<Source>& args) const {
        return _callable->eval(args);
    }

public:
    template<typename Callable, typename Adapter>
    Wrapper(Callable&& callable, Adapter&& adapter) :
            _callable{
                std::make_shared<ModelType<Callable, Adapter>>(
                    std::forward<Callable>(callable),
                    std::forward<Adapter>(adapter)
                )
            }
    {
        static_assert(
            std::is_copy_constructible<Callable>::value,
            "Non copy-constructible callable"
        );
        static_assert(
            std::is_copy_constructible<Adapter>::value,
            "Non copy-constructible adapter"
        );
        static_assert(
            std::is_same<
                detail::Arguments<Callable>,
                detail::Repeated<Type, detail::Argc<Callable>::value>
            >::value,
            "Wrong arguments types"
        );
        static_assert(
            std::is_same<detail::Result<Callable>, Type>::value,
            "Wrong return type"
        );
        static_assert(
            std::is_same<
                detail::Arguments<Adapter>,
                detail::Repeated<Source, 1>
            >::value,
            "Wrong adapter arguments types"
        );
        static_assert(
            std::is_same<detail::Result<Adapter>, Type>::value,
            "Wrong adapter return type"
        );
        static_assert(
            detail::IsConst<Callable>::value,
            "Non constant callable"
        );
        static_assert(
            detail::IsConst<Adapter>::value,
            "Non constant adapter"
        );
    }

    template<
        typename Callable,
        typename = std::enable_if_t<Inspect<Callable>::not_self>,
        typename = std::enable_if_t<!Inspect<Callable>::is_model>
    >
    Wrapper(Callable&& callable=[]() { return Type(); }) :
            Wrapper{
                std::forward<Callable>(callable),
                [](const Source& x) { return Type{x}; }
            }
    {}

    template<
        typename Callable,
        typename = std::enable_if_t<Inspect<Callable>::is_model>
    >
    Wrapper(Callable&& callable) :
            _callable{
                std::make_shared<Callable>(
                    std::forward<Callable>(callable)
                )
            }
    {}

    Wrapper clone() const noexcept { return Wrapper{_callable->clone()}; }

    std::size_t argc() const noexcept { return _callable->argc(); }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return _callable->call(util::to_reference<Type>(std::forward<Args>(args)...));
    }

    bool operator==(const Wrapper& other) const noexcept {
        return _callable == other._callable;
    }

    bool operator!=(const Wrapper& other) const noexcept {
        return !operator==(other);
    }
};

}


namespace std {

template<typename Type, typename Source>
struct hash<calculate::Wrapper<Type, Source>> {
    size_t operator()(const calculate::Wrapper<Type, Source>& wrapper) const {
        return hash<std::shared_ptr<calculate::WrapperConcept<Type, Source>>>{}(
            wrapper._callable
        );
    }
};

}

#endif
