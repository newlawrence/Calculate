#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#ifdef __cplusplus

#include "calculate/definitions.hpp"
#include "calculate/exceptions.hpp"
#include "calculate/symbols.hpp"


namespace calculate {

    using namespace calculate_definitions;
    using namespace calculate_symbols;
    using namespace calculate_exceptions;

    String version();
    String author();
    String date();

    vString constants();
    vString operators();
    vString functions();


    class Expression final {
        String _expression;
        vString _variables;
        pValue _values;
        String _infix;
        String _postfix;
        pEvaluable _tree;

        static vString _extract(const String &vars);
        static vString _validate(const vString &vars);

        qSymbol _tokenize(String expr);
        qEvaluable _shuntingYard(qSymbol &&infix);
        pEvaluable _buildTree(qEvaluable &&postfix);

        Expression() = delete;

    public:
        Expression(const Expression &other);
        Expression(Expression &&other);
        Expression(const String &expr, const String &vars);
        Expression(const String &expr, const vString &vars={});

        Expression& operator=(const Expression &other);
        Expression& operator=(Expression &&other);

        Value operator() (const vValue &values) const;
        template <typename... Args>
        Value operator() (Args... args) const {
            return this->operator() ({static_cast<Value>(args)...});
        };

        const String& expression() const noexcept { return _expression; }
        const vString& variables() const noexcept { return _variables; }
        const String& infix() const noexcept { return _infix; };
        const String& postfix() const noexcept { return _postfix; };
        String tree() const noexcept;
    };


    Expression parse(const String &expr);

}

#else

#include "calculate/binding.h"

#endif

#endif
