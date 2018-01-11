#ifndef __CALCULATE_WRAPPER_HPP__
#define __CALCULATE_WRAPPER_HPP__

#include <memory>
#include <type_traits>
#include <tuple>
#include <vector>
#include <utility>

#include "exception.hpp"


namespace calculate {

namespace detail {

template<typename Function, typename... Args>
struct NoExcept {
    static constexpr bool value =
        noexcept(std::declval<Function>()(std::declval<Args>()...));
};

template<typename Type, typename = void>
struct Traits : Traits<decltype(&Type::operator())> {};

template<typename Result, typename... Args>
struct Traits<std::function<Result(Args...)>, void> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(Args...) noexcept,
    std::enable_if_t<NoExcept<Result(*)(Args...) noexcept, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(Args...),
    std::enable_if_t<!NoExcept<Result(*)(Args...), Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(*)(Args...) noexcept,
    std::enable_if_t<NoExcept<Result(*)(Args...) noexcept, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(*)(Args...),
    std::enable_if_t<!NoExcept<Result(*)(Args...), Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(* const)(Args...) noexcept,
    std::enable_if_t<NoExcept<Result(*)(Args...) noexcept, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(* const)(Args...),
    std::enable_if_t<!NoExcept<Result(*)(Args...), Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) noexcept,
    std::enable_if_t<NoExcept<Type, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = false;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...),
    std::enable_if_t<!NoExcept<Type, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = false;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) const noexcept,
    std::enable_if_t<NoExcept<Type, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool constant = true;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) const,
    std::enable_if_t<!NoExcept<Type, Args...>::value>
> {
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

template<typename, std::size_t n, typename = std::make_index_sequence<n>>
struct Repeat {};

template<typename Type, std::size_t n, std::size_t... indices>
struct Repeat<Type, n, std::index_sequence<indices...>> {
    using type = std::tuple<ExtractType<Type, indices>...>;
};

template<typename Type, std::size_t n>
using Repeated = typename Repeat<Type, n>::type;

}


template<typename Type, typename Source>
struct WrapperConcept {
    virtual ~WrapperConcept() = default;
    virtual std::shared_ptr<WrapperConcept> copy() const noexcept = 0;
    virtual std::size_t argc() const noexcept = 0;
    virtual bool is_const() const noexcept = 0;
    virtual Type call(const std::vector<Type>&) const = 0;
    virtual Type call(const std::vector<Type>&) = 0;
    virtual Type eval(const std::vector<Source>&) const = 0;
    virtual Type eval(const std::vector<Source>&) = 0;
};


template<typename Type, typename Source = Type>
class Wrapper {
    friend struct std::hash<Wrapper>;

    using WrapperConcept = calculate::WrapperConcept<Type, Source>;

    template<typename Callable>
    struct Inspect {
        static constexpr bool not_me =
            detail::NotSame<Callable, Wrapper>::value;
        static constexpr bool is_model =
            std::is_base_of<WrapperConcept, Callable>::value;
    };

    template<typename Callable, typename Adapter, std::size_t n, bool constant>
    class WrapperModel final : public WrapperConcept {
        template<bool c> using Constant = std::integral_constant<bool, c>;
        using False = std::integral_constant<bool, false>::type;
        using True = std::integral_constant<bool, true>::type;

        Callable _callable;
        Adapter _adapter;

        template<typename Vector, typename Adapt, std::size_t... indices>
        Type _invoke(const Vector&, const Adapt&, False) const {
            throw AccessViolation{};
        }

        template<typename Vector, typename Adapt, std::size_t... indices>
        Type _invoke(const Vector& args, const Adapt& adapt, True) const {
            return const_cast<WrapperModel*>(this)
                ->_invoke(args, adapt, std::make_index_sequence<n>{});
        }

        template<std::size_t... indices>
        Type _invoke(
            const std::vector<Type>& args,
            False,
            std::index_sequence<indices...>
        ) {
            if (args.size() != n)
                throw ArgumentsMismatch{n, args.size()};
            return _callable(args[indices]...);
        }

        template<std::size_t... indices>
        Type _invoke(
            const std::vector<Source>& args,
            True,
            std::index_sequence<indices...>
        ) {
            if (args.size() != n)
                throw ArgumentsMismatch{n, args.size()};
            return _callable(_adapter(args[indices])...);
        }

    public:
        WrapperModel(Callable callable, Adapter adapter) :
                _callable{callable},
                _adapter{adapter}
        {}

        std::shared_ptr<WrapperConcept> copy() const noexcept override {
            return std::make_shared<WrapperModel>(*this);
        }

        std::size_t argc() const noexcept override { return n; }

        bool is_const() const noexcept override { return constant; }

        Type call(const std::vector<Type>& args) const override {
            return _invoke(args, False{}, Constant<constant>{});
        }

        Type call(const std::vector<Type>& args) override {
            return _invoke(args, False{}, std::make_index_sequence<n>{});
        }

        Type eval(const std::vector<Source>& args) const override {
            return _invoke(args, True{}, Constant<constant>{});
        }

        Type eval(const std::vector<Source>& args) override {
            return _invoke(args, True{}, std::make_index_sequence<n>{});
        }
    };

	template<typename Callable, typename Adapter>
	using ModelType = WrapperModel<
        Callable,
        Adapter,
        detail::Argc<Callable>::value,
        detail::IsConst<Callable>::value
    >;

    std::shared_ptr<WrapperConcept> _callable;

    Wrapper(std::shared_ptr<WrapperConcept>&& callable) :
            _callable{std::move(callable)}
    {}

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
            detail::IsConst<Adapter>::value,
            "Non constant adapter function"
        );
    }

    template<
        typename Callable,
        std::enable_if_t<Inspect<Callable>::not_me>* = nullptr,
        std::enable_if_t<!Inspect<Callable>::is_model>* = nullptr
    >
    Wrapper(Callable&& callable=[]() { return Type(); }) :
            Wrapper{
                std::forward<Callable>(callable),
                [](const Source& x) { return Type{x}; }
            }
    {}

    template<
        typename Callable,
        std::enable_if_t<Inspect<Callable>::is_model>* = nullptr
    >
    Wrapper(Callable&& callable) :
            _callable{
                std::make_shared<Callable>(
                    std::forward<Callable>(callable)
                )
            }
    {}

    Type call(const std::vector<Type>& args) const {
        return const_cast<const WrapperConcept*>(_callable.get())->call(args);
    }

    Type call(const std::vector<Type>& args) {
        return _callable->call(args);
    }

    template<typename... Args>
    Type call(Args&&... args) const {
        return const_cast<const WrapperConcept*>(_callable.get())
            ->call(std::vector<Type>{std::forward<Args>(args)...});
    }

    template<typename... Args>
    Type call(Args&&... args) {
        return _callable
            ->call(std::vector<Type>{std::forward<Args>(args)...});
    }

    Type operator()(const std::vector<Source>& args) const {
        return const_cast<const WrapperConcept*>(_callable.get())->eval(args);
    }

    Type operator()(const std::vector<Source>& args) {
        return _callable->eval(args);
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return const_cast<const WrapperConcept*>(_callable.get())
            ->eval(std::vector<Source>{std::forward<Args>(args)...});
    }

    template<typename... Args>
    Type operator()(Args&&... args) {
        return _callable
            ->eval(std::vector<Source>{std::forward<Args>(args)...});
    }

    bool operator==(const Wrapper& other) const noexcept {
        return _callable == other._callable;
    }

    Wrapper copy() const noexcept { return Wrapper{_callable->copy()}; }

    std::size_t argc() const noexcept { return _callable->argc(); }

    bool is_const() const noexcept { return _callable->is_const(); }
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