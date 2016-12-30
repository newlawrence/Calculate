# Calculate


### Little Math Expression Parser

[![Build Status](https://travis-ci.org/newlawrence/Calculate.svg?branch=master)](https://travis-ci.org/newlawrence/Calculate)
[![codecov](https://codecov.io/gh/newlawrence/Calculate/branch/master/graph/badge.svg)](https://codecov.io/gh/newlawrence/Calculate)
[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/newlawrence/Calculate/blob/7f96b434dd77461f17a71f3fe3025c21b73ed0d0/copying)

Written in modern **C++** for learning purposes.

With bindings for **C**, **Fortran** and **Python**.


### Requirements

*Version numbers listed below are the lowest supported tested*

* **CMake** *3.5* (*3.7* in the case of **Fortran**), to generate the build scripts .
* **C**/**C++** compiler, **C++14** compliant (**g++** *5.3*, **clang** *3.8*, **icpc** *17.0*, **MSVC** *19.0*).
* **Fortran** compiler (*optional*), with **2008** standard support (**ifort** *16.3*).
* **Python** *2.7* (*optional*), along with **setuptools** and **cffi** *1.0* libraries.
* **gcov** and **gcovr** tools (*optional*), to build the coverage reports.


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

Generating the **Python** binary distributions:

```
$ BINDING=1 cmake . -DCMAKE_BUILD_TYPE=Release
$ make python_wheel
$ make conda_package-Py2.7

```

**License:** MIT (see `copying`).
