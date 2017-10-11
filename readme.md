# Calculate


### Little Math Expression Parser

[![Build Status](https://travis-ci.org/newlawrence/Calculate.svg?branch=master)](https://travis-ci.org/newlawrence/Calculate)
[![codecov](https://codecov.io/gh/newlawrence/Calculate/branch/master/graph/badge.svg)](https://codecov.io/gh/newlawrence/Calculate)
[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/newlawrence/Calculate/blob/7f96b434dd77461f17a71f3fe3025c21b73ed0d0/copying)

Written in modern **C++** for learning purposes, including bindings for **C**, **Fortran** and **Python**.

**Python** binary packages available using [**conda**](https://anaconda.org/newlawrence/calculate/files):

```
conda install calculate -c newlawrence
```


### Example of usage

Python version:

```python
>>> import calculate

# Automatic variables deduction
>>> hypot = calculate.parse('hypot(x, y)')
>>> hypot
Expression('hypot(x, y)', ['x', 'y'])
>>> hypot.variables()
['x', 'y']
>>> hypot(3, 4)
-5.0

# Manual variables specifiaction
>>> sin = calculate.Expression('sin(x)', ['x'])
>>> pi = calculate.Expression('pi')()
>>> pi
3.141592653589
>>> sin(pi / 2)
1.0
>>> sin(pi)
7.932657934721266e-13
```

### Requirements

*Version numbers listed below are the lowest supported tested*

* **CMake** *3.5* (*3.7* in the case of **Fortran**), to generate the build scripts .
* **C**/**C++** compiler, **C++14** compliant (**g++** *5.3*, **clang** *3.8*, **icpc** *17.0*, **MSVC** *14.0*).
* **Fortran** compiler (*optional*), with **2008** standard support (**gfortran** *6.3*, **ifort** *16.3*).
* **Python** *2.7* and *3.4* (*optional*), along with **setuptools** and **cffi** *1.0* libraries.
* **gcov** and **gcovr** tools (*optional*), to build the coverage reports.


### Building and testing

Just making the library and the bindings:

```
$ BINDING=True cmake .
$ make calculate
$ make calculate_fortran
$ make calculate_python
```

Bulding and running the tests:

```
$ TESTING=True cmake .
$ make unittests
```

Generating the coverage reports (only **GNU** and **Clang**):

```
$ COVERAGE=True cmake . -DCMAKE_BUILD_TYPE=Debug
$ make unittests
$ make coverage
```

Generating the **Python** binary distributions:

```
$ BINDING=True cmake . -DCMAKE_BUILD_TYPE=Release
$ make python_wheel
$ make conda_package-Py2.7

```

**License:** MIT (see `copying`).
