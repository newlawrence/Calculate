![Calculate](resource/calculate.svg)

| Version | 2.0.0rc4 |
| ------- | ---------|

---

[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/newlawrence/Calculate/blob/7f96b434dd77461f17a71f3fe3025c21b73ed0d0/copying)
[![Try online](https://img.shields.io/badge/try-online-blue.svg)](https://wandbox.org/permlink/hQct8Y1a6C5lUyVb)
[![Build Status](https://travis-ci.org/newlawrence/Calculate.svg?branch=master)](https://travis-ci.org/newlawrence/Calculate)
[![codecov](https://codecov.io/gh/newlawrence/Calculate/branch/master/graph/badge.svg)](https://codecov.io/gh/newlawrence/Calculate)

Header-only library written in modern **C++** aiming for flexibility and ease of use.

**Warning:** *Fully C++14 standard compliant compiler required!*

```c++
auto parser = calculate::DefaultParser{};
auto sum = parser.parse("x+y");

sum(1., 2.);  // returns 3.
```

**License:** MIT (see `copying`).
