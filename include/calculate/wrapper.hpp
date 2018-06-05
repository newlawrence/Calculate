/*
    Calculate - Version 2.1.1rc1
    Last modified 2018/06/05
    Released under MIT license
    Copyright (c) 2016-2018 Alberto Lorenzo <alorenzo.md@gmail.com>
*/


#ifndef __CALCULATE_WRAPPER_HPP__
#define __CALCULATE_WRAPPER_HPP__

#include <utility>

#include "util.hpp"


namespace calculate {

template<typename Type, typename Source>
struct WrapperConcept {
    virtual ~WrapperConcept() = default;
    virtual std::shared_ptr<WrapperConcept> clone() const noexcept = 0;
    virtual std::size_t argc() const noexcept = 0;
    virtual Type eval(const std::vector<Source>&) const = 0;
    virtual Type call(const std::vector<Type>&) const = 0;
};


template<typename Type, typename Source = Type>
class Wrapper {
    friend struct std::hash<Wrapper>;

    using WrapperConcept = calculate::WrapperConcept<Type, Source>;

    template<typename Callable, typename Adapter, std::size_t argcount>
    class WrapperModel final : public WrapperConcept {
        Callable _callable;
        Adapter _adapter;

        template<std::size_t... indices>
        Type _eval(
            const std::vector<Source>& args,
            std::index_sequence<indices...>
        ) const {
            if (args.size() != argcount)
                throw ArgumentsMismatch{argcount, args.size()};
            return _callable(_adapter(args[indices])...);
        }

        template<std::size_t... indices>
        Type _call(
            const std::vector<Type>& args,
            std::index_sequence<indices...>
        ) const {
            if (args.size() != argcount)
                throw ArgumentsMismatch{argcount, args.size()};
            return _callable(args[indices]...);
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

        Type eval(const std::vector<Source>& args) const override {
            return _eval(args, std::make_index_sequence<argcount>{});
        }

        Type call(const std::vector<Type>& args) const override {
            return _call(args, std::make_index_sequence<argcount>{});
        }
    };

	template<typename Callable, typename Adapter>
	using ModelType = WrapperModel<
        Callable,
        Adapter,
        util::argc_v<Callable>
    >;

    std::shared_ptr<WrapperConcept> _callable;

    Wrapper(std::shared_ptr<WrapperConcept>&& callable) : _callable{callable} {}

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
            util::is_copy_constructible_v<Callable>,
            "Non copy-constructible callable"
        );
        static_assert(
            util::is_same_v<
                util::args_tuple_t<Callable>,
                util::make_tuple_t<Type, util::argc_v<Callable>>
            >,
            "Wrong callable arguments types"
        );
        static_assert(
            util::is_same_v<util::result_t<Callable>, Type>,
            "Wrong callable return type"
        );
        static_assert(util::is_const_v<Callable>, "Non constant callable");

        static_assert(
            util::is_copy_constructible_v<Adapter>,
            "Non copy-constructible adapter"
        );
        static_assert(
            util::is_same_v<
                util::args_tuple_t<Adapter>,
                util::make_tuple_t<Source, 1>
            >,
            "Wrong adapter arguments types"
        );
        static_assert(
            util::is_same_v<util::result_t<Adapter>, Type>,
            "Wrong adapter return type"
        );
        static_assert(util::is_const_v<Adapter>, "Non constant adapter");
    }

    template<
        typename Callable,
        typename = std::enable_if_t<util::not_same_v<Callable, Wrapper>>,
        typename = std::enable_if_t<!util::is_base_of_v<WrapperConcept, Callable>>
    >
    Wrapper(Callable&& callable=[]() { return Type(); }) :
            Wrapper{
                std::forward<Callable>(callable),
                [](const Source& x) { return Type{x}; }
            }
    {}

    template<
        typename Callable,
        typename = std::enable_if_t<util::is_base_of_v<WrapperConcept, Callable>>
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
    Type eval(Args&&... args) const {
        return _callable->eval(util::to_vector<Source>(std::forward<Args>(args)...));
    }

    template<typename... Args>
    Type operator()(Args&&... args) const {
        return _callable->call(util::to_vector<Type>(std::forward<Args>(args)...));
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
