# cdboost

A header-only C++23 implementation of Parallel DEVS (PDEVS) simulation.

## Overview

cdboost provides a lightweight, template-based PDEVS simulator. The core architecture follows the standard coordinator/simulator hierarchy:

- **Atomic models** inherit from `cdboost::pdevs::atomic<TIME, MSG>` and implement `internal()`, `external()`, `confluence()`, `out()`, and `advance()`.
- **Coupled models** compose atomic and coupled submodels via external input couplings (EIC), internal couplings (IC), and external output couplings (EOC).
- **Coordinators** manage the simulation hierarchy; `cdboost::pdevs::coordinator` handles both atomic simulators and coupled coordinators.
- **Runner** drives simulation to a given end time or until passivation.

Built-in basic models: `generator`, `processor`, `infinite_counter`, `event_stream`, `input_stream`.

## Requirements

- C++23 compiler (GCC 13+, Clang 16+)
- CMake 3.28+
- vcpkg (for dependencies)
- `boost-rational` (no C++23 equivalent for exact rational arithmetic)
- Catch2 3.x (tests only)

## Build

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build
ctest --test-dir build
```

## Usage

```cpp
#include <cdboost/pdevs/atomic.hpp>
#include <cdboost/pdevs/coupled.hpp>
#include <cdboost/pdevs/runner.hpp>
#include <cdboost/convenience.hpp>

// Define a TIME type that satisfies TIME::Inf() and arithmetic operators
// then define atomic models, compose into a coupled model, and run:

auto top = std::make_shared<cdboost::pdevs::coupled<Time, Msg>>(models, eic, ic, eoc);
cdboost::pdevs::runner<Time, Msg> r{top, Time{0}, std::cout, my_output_fn};
r.runUntilPassivate();
```

## License

BSD 2-Clause. Copyright (c) 2013-present, Damian Vicino.
