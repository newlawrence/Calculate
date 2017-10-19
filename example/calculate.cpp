#include <exception>
#include <chrono>
#include <limits>
#include <iostream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "calculate.hpp"


namespace fs = boost::filesystem;
namespace po = boost::program_options;

template<typename Parser>
void run(
    std::string,
    std::vector<std::string>,
    const std::vector<std::string>&,
    const po::variables_map&
);


int main(int argc, char *argv[]) {
    fs::path program{argv[0]};
    po::variables_map arguments;
    std::string string;
    std::vector<std::string> variables;
    std::vector<std::string> substitutions;

    po::options_description named_args("Options");
    named_args.add_options()
        (
            "expression,e",
            po::value<std::string>(&string)->required(),
            "Expression to parse"
        )
        ("help,h", "Show this help message")
        (
            "var,v",
            po::value<std::vector<std::string>>(&variables),
            "Add variable (var:value)"
        )
        ("postfix,p", "Use postfix parser")
        ("complex,c", "Use complex parser")
        ("optimize,o", "Optimize expression")
        (
            "sub,s",
            po::value<std::vector<std::string>>(&substitutions),
            "Substitute variable by value (var:value)"
        )
        (
            "iter,i",
            po::value<std::size_t>()->default_value(1000),
            "Performance iterations"
        )
        ("analysis,a", "Print analysis")
    ;
    po::positional_options_description positional_args;
    positional_args.add("expression", 1);

    try {
        po::store(
            po::command_line_parser(argc, argv)
                .options(named_args)
                .positional(positional_args)
                .run(),
            arguments
        );

        if (arguments.count("help")) {
            std::cout << "Usage: " << program.filename() << " [options] {-e [ --expression ]}"
                " <expression>" << std::endl;
            std::cout << named_args << std::endl;
            return 0;
        }
        po::notify(arguments);

        if (arguments.count("complex"))
            run<calculate::DefaultComplexParser>(string, variables, substitutions, arguments);
        else
            run<calculate::DefaultParser>(string, variables, substitutions, arguments);
    }
    catch (const po::error& error) {
        std::cerr << "Command line error: " << error.what() << std::endl;
        return 1;
    }
    catch (const calculate::BaseError& error) {
        std::cerr << error.what() << std::endl;
        return 2;
    }
    return 0;
}


template<typename Parser>
void run(
    std::string expression,
    std::vector<std::string> variables,
    const std::vector<std::string>& substitutions,
    const po::variables_map& arguments
) {
    using Type = typename Parser::Type;
    Parser parser{};

    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point::rep build_time, opt_time, eval_time;
    std::size_t iterations = arguments["iter"].as<std::size_t>();
    Type result;

    std::vector<Type> values;
    if (arguments.count("var")) {
        for (auto& var : variables) {
            auto sep = var.find(":");
            if (sep == 0 || sep > var.size() - 2)
                throw po::error("bad variable input '" + var + "'");
            values.push_back(parser.cast(var.substr(sep + 1, var.size())));
            var = var.substr(0, sep);
        }
    }

    auto parse = arguments.count("postfix") ? &Parser::from_postfix : &Parser::from_infix;
    auto whole_set = variables;
    if (arguments.count("sub")) {
        for (auto& sub : substitutions) {
            auto sep = sub.find(":");
            if (sep == 0 || sep > sub.size() - 2)
                throw po::error("bad substitution input '" + sub + "'");
            auto var = sub.substr(0, sep);
            if (std::find(variables.begin(), variables.end(), var) == variables.end())
                whole_set.push_back(var);
        }
    }

    auto function = (parser.*parse)(expression, whole_set);
    if (arguments.count("analysis")) {
        begin = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < iterations; i++)
            (parser.*parse)(expression, whole_set);
        build_time = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - begin
        ).count() / iterations;
    }

    if (arguments.count("sub")) {
        for (auto& sub : substitutions) {
            auto sep = sub.find(":");
            function = parser.substitute(
                function,
                sub.substr(0, sep),
                parser.cast(sub.substr(sep + 1, sub.size()))
            );
        }
    }

    if (arguments.count("optimize")) {
        function = parser.optimize(function);
        if (arguments.count("analysis")) {
            begin = std::chrono::steady_clock::now();
            for (std::size_t i = 0; i < iterations; i++)
                parser.optimize(function);
            opt_time = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - begin
            ).count() / iterations;
        }
    }

    result = function(values);
    if (arguments.count("analysis")) {
        begin = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < iterations; i++)
            function(values);
        eval_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - begin
        ).count() / iterations;
    }

    if (arguments.count("analysis")) {
        std::cout << "Infix notation:    " << function.infix() << std::endl;
        std::cout << "Postfix notation:  " << function.postfix() << std::endl;
        if (arguments.count("var")) {
            std::cout << "Variables:         ";
            for (const auto& var : variables)
                std::cout << var << " ";
            std::cout << std::endl;
            std::cout << "Values:            ";
            for (const auto& val : values)
                std::cout << parser.to_string(val) << " ";
            std::cout << std::endl;
        }
        std::cout << "Result:            " << parser.to_string(result) << std::endl;
        std::cout << "Iterations:        " << iterations << std::endl;
        std::cout << "Building time:     " << build_time << " us" << std::endl;
        if (arguments.count("optimize"))
            std::cout << "Optimization time: " << opt_time << " us" << std::endl;
        std::cout << "Evaluation time:   " << eval_time << " ns" << std::endl;
    }
    else
        std::cout << parser.to_string(result) << std::endl;
}
