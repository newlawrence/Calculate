![Calculate](resource/calculate.svg)

| Version | 2.1.1rc10 |
| ------- | -------- |

---

[![Download](https://api.bintray.com/packages/newlawrence/calculate/Calculate%3Anewlawrence/images/download.svg)](https://bintray.com/newlawrence/calculate/Calculate%3Anewlawrence/_latestVersion)
[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/newlawrence/Calculate/blob/7f96b434dd77461f17a71f3fe3025c21b73ed0d0/copying)
[![Try online](https://img.shields.io/badge/try-online-blue.svg)](https://wandbox.org/permlink/vVyBsIhckIRuvNP9)
[![Build Status](https://travis-ci.org/newlawrence/Calculate.svg?branch=master)](https://travis-ci.org/newlawrence/Calculate)
[![codecov](https://codecov.io/gh/newlawrence/Calculate/branch/master/graph/badge.svg)](https://codecov.io/gh/newlawrence/Calculate)

Header-only library written in modern **C++** aiming for flexibility and ease of use. **Calculate** is not only a mathematical expressions parser but an engine built on top of the [Shunting Yard algorithm](https://en.wikipedia.org/wiki/Shunting-yard_algorithm).

The main objective of the library is to offer a clean and intuitive interface, where the expressions act and feel like regular functions. Another objective is to be completely configurable; from the underlying data type to the tokenizing logic, the library is in fact a custom parser factory.

```c++
auto parser = calculate::Parser{};
auto sum = parser.parse("x+y");

sum(1., 2.);  // returns 3.
```

**Calculate** is available as a [conan package](https://bintray.com/newlawrence/calculate/Calculate%3Anewlawrence):

```bash
# Append calculate to Conan's repositories list
conan remote add calculate https://api.bintray.com/conan/newlawrence/calculate
```

### Features

* Generic. `double` and `std::complex<double>` parsers included by default.
* User defined constants, functions, and prefix, suffix and binary operators.
* Infix and postfix notations supported.
* Regex-based customizable lexers.
* Header-only.

### Build and test

**Calculate** doesn't have any third party dependencies, the library should work with any compiler fully compatible with the **C++14** standard. Currently it has being tested under **gcc (5.2+)**,  **clang (3.7+)**, **msvc (19.10+)** and **intel (18.0+)**.

The examples and tests need [CMake](https://cmake.org/) to be built. [Conan](https://conan.io/) can be used to handle the dependencies:

```bash
# Build the example (Boost libraries needed)
conan install example --install-folder build/example
cmake -H. -Bbuild -DCALCULATE_BUILD_EXAMPLES=ON
cmake --build build --target example

# Build and run the tests (Catch2 library needed)
conan install test --install-folder build/test
cmake -H. -Bbuild -DCALCULATE_BUILD_TESTS=ON
cmake --build build --target make_test  # build
cmake --build build --target test       # run
```

### User guide

Want to try? Check out **Calculate**'s [wiki](https://github.com/newlawrence/Calculate/wiki) to get started.

**License:** MIT (see `copying`).
