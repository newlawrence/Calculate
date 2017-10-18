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
    static constexpr bool value = noexcept(std::declval<Function>()(std::declval<Args>()...));
};

template<typename Type, typename = void>
struct Traits : Traits<decltype(&Type::operator())> {};

template<typename Result, typename... Args>
struct Traits<std::function<Result(Args...)>, void> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(Args...) noexcept,
    std::enable_if_t<NoExcept<Result(*)(Args...) noexcept, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(Args...),
    std::enable_if_t<!NoExcept<Result(*)(Args...), Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(*)(Args...) noexcept,
    std::enable_if_t<NoExcept<Result(*)(Args...) noexcept, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = true;
};

template<typename Result, typename... Args>
struct Traits<
    Result(*)(Args...),
    std::enable_if_t<!NoExcept<Result(*)(Args...), Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = true;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) noexcept,
    std::enable_if_t<NoExcept<Type, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = false;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...),
    std::enable_if_t<!NoExcept<Type, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = false;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) const noexcept,
    std::enable_if_t<NoExcept<Type, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = true;
};

template<typename Type, typename Result, typename... Args>
struct Traits<
    Result(Type::*)(Args...) const,
    std::enable_if_t<!NoExcept<Type, Args...>::value>
> {
    using result = Result;
    using arguments = std::tuple<std::decay_t<Args>...>;
    static constexpr bool const_method = true;
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
    static constexpr bool value = Traits<Function>::const_method;
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


template<typename Type, typename Source = Type>
class Wrapper {
    friend struct std::hash<Wrapper>;

    struct Concept {
        virtual std::shared_ptr<Concept> copy() const noexcept = 0;
        virtual std::size_t argc() const noexcept = 0;
        virtual bool is_const() const noexcept = 0;
        virtual Type evaluate(const std::vector<Source>&) const = 0;
        virtual Type evaluate(const std::vector<Source>&) = 0;
        virtual ~Concept() {}
    };

    template<typename Callable, typename Adapter, std::size_t n, bool const_method>
    class Model final : public Concept {
        Callable _callable;
        Adapter _adapter;

        template<std::size_t... indices>
        Type _evaluate(
            std::integral_constant<bool, true>::type,
            const std::vector<Source>& args,
            std::index_sequence<indices...>
        ) const {
            if (args.size() != n)
                throw ArgumentsMismatch{n, args.size()};
            return _callable(_adapter(args[indices])...);
        }

        template<std::size_t... indices>
        Type _evaluate(
            std::integral_constant<bool, false>::type,
            const std::vector<Source>&,
            std::index_sequence<indices...>
        ) const { throw AccessViolation{}; }

        template<std::size_t... indices>
        Type _evaluate(const std::vector<Source>& args, std::index_sequence<indices...>) {
            if (args.size() != n)
                throw ArgumentsMismatch{n, args.size()};
            return _callable(_adapter(args[indices])...);
        }

    public:
        Model(Callable callable, Adapter adapter) : _callable{callable}, _adapter{adapter} {}

        virtual std::shared_ptr<Concept> copy() const noexcept override {
            return std::make_shared<Model>(*this);
        }

        virtual std::size_t argc() const noexcept override { return n; }

        virtual bool is_const() const noexcept override { return const_method; }

        virtual Type evaluate(const std::vector<Source>& args) const override {
            return _evaluate(
                std::integral_constant<bool, const_method>{},
                args,
                std::make_index_sequence<n>{}
            );
        }

        virtual Type evaluate(const std::vector<Source>& args) override {
            return _evaluate(args, std::make_index_sequence<n>{});
        }
    };

    std::shared_ptr<Concept> _callable;

    Wrapper(std::shared_ptr<Concept>&& callable) : _callable{std::move(callable)} {}

public:
    template<typename Callable, typename Adapter>
    Wrapper(Callable&& callable, Adapter&& adapter) :
            _callable{
                std::make_shared<Model<
                    Callable,
                    Adapter,
                    detail::Argc<Callable>::value,
                    detail::IsConst<Callable>::value
                >>(std::forward<Callable>(callable), std::forward<Adapter>(adapter))
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
            std::is_same<detail::Arguments<Adapter>, detail::Repeated<Source, 1>>::value,
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
        std::enable_if_t<detail::NotSame<Callable, Wrapper>::value>* = nullptr
    >
    Wrapper(Callable&& callable=[]() { return Type(); }) :
            Wrapper{std::forward<Callable>(callable), [](const Source& x) { return Type{x}; }}
    {}

    Wrapper copy() { return Wrapper{_callable->copy()}; }

    std::size_t argc() const noexcept { return _callable->argc(); }

    bool is_const() const noexcept { return _callable->is_const(); }

    Type operator()(const std::vector<Source>& args) const {
        return const_cast<const Concept*>(_callable.get())->evaluate(args);
    }

    Type operator()(const std::vector<Source>& args) { return _callable->evaluate(args); }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return const_cast<const Concept*>(_callable.get())
            ->evaluate(std::vector<Source>{std::forward<Args>(args)...});
    }

    template<typename... Args>
    Type operator()(Args&&... args) {
        return _callable->evaluate(std::vector<Source>{std::forward<Args>(args)...});
    }
};

}


namespace std {

template<typename Type, typename Source>
struct hash<calculate::Wrapper<Type, Source>> {
    size_t operator()(const calculate::Wrapper<Type, Source>& wrapper) const {
        return hash<std::shared_ptr<typename calculate::Wrapper<Type, Source>::Concept>>{}(
            wrapper._callable
        );
    }
};

}

#endif
