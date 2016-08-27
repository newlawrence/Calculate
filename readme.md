# Calculate


### Little Math Expression Parser

[![Build Status](https://travis-ci.org/newlawrence/Calculate.svg?branch=master)](https://travis-ci.org/newlawrence/Calculate)
[![codecov](https://codecov.io/gh/newlawrence/Calculate/branch/master/graph/badge.svg)](https://codecov.io/gh/newlawrence/Calculate)
[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/newlawrence/Calculate/blob/7f96b434dd77461f17a71f3fe3025c21b73ed0d0/copying)

Written in modern **C++** for learning purposes.

Bindings for **C**, **Fortran** and **Python**.


### Requirements

* **CMake** *>= 3.5*, to generate the makefiles and **Visual Studio** solutions.
* **C**/**C++** compiler, with **C++14** standard support (**GNU**, **Clang** and **MSVC** tested).
* **Fortran** compiler (*optional*), with **2008** standard support (**GNU** and **Intel** tested).
* **Python** *>= 3.5* (*optional*), along with **setuptools** and **cffi** *>=1.0* libraries.
* **gcov** and **gcovr** tools (*optional*), for generating coverage reports.


### Building and testing

Just making the library and the bindings:

```
$ BINDING=1 cmake .
$ make calculate
$ make calculate_fortran
$ make calculate_python
```

Bulding and running the tests:

```
$ TESTING=1 cmake .
$ make unittests
```

Generating the coverage reports (only **GNU** and **Clang**):

```
$ COVERAGE=1 cmake . -DCMAKE_BUILD_TYPE=Debug
$ make unittests
$ make coverage
```

**License:** MIT (see `copying`).
