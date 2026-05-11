# Agent instructions for this repository

These guidelines apply to any AI coding agent working on this repo. Keep responses concise and focused, and follow the standards below when proposing or making changes.

## Response format

- Keep answers short and impersonal.
- For change requests, format your reply as follows:
  1) Describe the solution step-by-step (briefly).
  2) Group changes by file; use the file path as the header.
  3) For each file, add a short summary, then a single code block with only the changes.
  4) Start the code block with four backticks and a language tag.
  5) First line inside the code block: a comment with the filepath.
  6) Use a single code block per file; if parts are unchanged, replace with `// ...existing code...`.
  7) Avoid heavy formatting; bullet lists are fine.
  8) Do not repeat large unchanged regions.

## C++ coding guidance (project-wide)

- Use C++23 features where appropriate.
- Use standard library features where possible.
- Avoid unnecessary dependencies; prefer the standard library and STL.
- Avoid adding new dependencies unless explicitly requested.
- Prefer `std::` over `using namespace std;`.
- Avoid `#define` unless absolutely necessary.
- Use `#pragma once` in headers.
- Prefer compile-time checks (`static_assert`, `constexpr`) over runtime checks where possible.
- Avoid global variables; prefer passing parameters or using class members.
- For code correctness, refer to the C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
- This is a C++ library for PDEVS simulation. For DEVS formalism correctness refer to Zeigler et al. "Theory of Modeling and Simulation" or the papers in `reading/`.
- Prefer STL algorithms and data structures over custom implementations.
- Prefer standard exceptions; document the exception-safety guarantee (strong, basic, no-throw) per function.
- Prefer descriptive variable and function names.
- Prefer `using` over `typedef`.
- Mark methods `const` when appropriate.
- Use `noexcept` on functions that cannot throw.

## Historical context

- CD-Boost is an original C++11 PDEVS simulator written by Damián Vicino (2013-present) at Carleton University / Université Nice Sophia Antipolis.
- An embedded port (Niyonkuru, 2015) was added and later removed; the desktop simulator is the canonical version.
- CD-Boost is the direct predecessor of the Cadmium PDEVS simulator and the CadmIA IA-DEVS simulator.
- This modernization effort updates the build system to CMake/vcpkg and the C++ standard to C++23, while preserving the PDEVS simulation semantics.

## Project constraints

- Respect existing BSD 2-Clause license headers.
- Only add new files when explicitly requested.
- Prefer standard C++23. Use `#pragma once` in headers.
- Use RAII, `constexpr`, and `static_assert` where appropriate; avoid macros when possible.
- No `using namespace` in headers. Keep includes ordered: standard library, Boost, project; group-separated.
- Keep lines ≤ 100 columns. Trim trailing whitespace. End files with a newline.
- Prefer `noexcept` on functions that cannot throw; mark methods `const` when appropriate.
- The project is a header-only library.

## Security and compliance

- Do not include copyrighted code you do not have rights to.
- Do not suggest unsafe operations or hidden network access.

## Testing and quality

- Use Catch2 3.x for all tests.
- Each test file compiles as a separate executable linked against `Catch2::Catch2WithMain`.
- Use `TEST_CASE` / `SECTION` or `SCENARIO` / `GIVEN` / `WHEN` / `THEN` style.
- Target high code coverage.
- Align to `.editorconfig` and `.clang-format` in this repo.
