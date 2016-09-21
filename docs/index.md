---
layout: default
title: "ReaverLib documentation"
---

### What is ReaverLib?

ReaverLib is a set of helpful libraries written for use in the Reaver Project. It contains various utilities that are either missing from the standard library
and from Boost, or their standard library or Boost versions were not satisfying the needs and design of the Reaver Project.

### Getting started

To use ReaverLib, you'll need a fairly recent C++ compiler with a C++17 mode. Currently, Clang 3.7+ and GCC 6.1+ are supported, together with Boost 1.60.0+.

ReaverLib is not a standalone library, since a lot of it requires some Boost features, but it is header only (if you discount the need to sometimes link some
Boost libraries). To install ReaverLib, simply run

    sudo make install

All the usual environment variables (`PREFIX` and so on) modifying the behavior of `make install` are supported.

