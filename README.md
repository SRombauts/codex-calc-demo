# calc

[![Build and test](https://github.com/SRombauts/codex-calc-demo/actions/workflows/build.yml/badge.svg)](https://github.com/SRombauts/codex-calc-demo/actions/workflows/build.yml)
[![Quality](https://github.com/SRombauts/codex-calc-demo/actions/workflows/quality.yml/badge.svg)](https://github.com/SRombauts/codex-calc-demo/actions/workflows/quality.yml)
[![CodeQL](https://github.com/SRombauts/codex-calc-demo/actions/workflows/codeql.yml/badge.svg)](https://github.com/SRombauts/codex-calc-demo/actions/workflows/codeql.yml)
[![Coverage](https://img.shields.io/badge/coverage-99.0%25-brightgreen)](https://github.com/SRombauts/codex-calc-demo/actions/workflows/quality.yml)
[![Release](https://img.shields.io/github/v/release/SRombauts/codex-calc-demo)](https://github.com/SRombauts/codex-calc-demo/releases)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A small, dependency-free ISO C17 command-line expression calculator. The code is deliberately
compact; the project primarily demonstrates a production-style, cross-platform engineering
workflow.

```console
$ calc "2 + 3 * 4"
14
$ calc "(2 + 3) * 4"
20
$ calc "-12.5 / (2 + 3)"
-2.5
```

## Build

CMake 3.20 or newer and a C17 compiler are required. Ninja is used by the developer presets.

```console
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure
```

Without presets, a conventional build also works:

```console
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Install with `cmake --install build --config Release --prefix <destination>`. Unity 2.6.1 is
vendored solely for tests; the calculator binary has no third-party runtime dependency.

## Expression language

The evaluator accepts decimal floating-point literals, optional decimal exponents, ASCII operators,
parentheses, and C whitespace. Hexadecimal floats, functions, variables, and implicit multiplication
are intentionally outside the grammar.

```text
expression = term, { ("+" | "-"), term } ;
term       = unary, { ("*" | "/"), unary } ;
unary      = ("+" | "-"), unary | primary ;
primary    = decimal | "(", expression, ")" ;
```

Operators of equal precedence associate from the left. Unary signs bind more tightly than
multiplication. Nesting is capped at 128 unary signs/parentheses to keep malformed input bounded.
Overflow, underflow-to-zero, and division by positive or negative zero are errors.

## CLI contract

Exactly one expression argument is required (shell quoting is needed only when it contains spaces or
metacharacters). Successful results are printed with enough precision to round-trip a `double`.
Evaluation diagnostics go to standard error and include a zero-based byte offset.

| Exit code | Meaning |
| ---: | --- |
| 0 | Successful evaluation |
| 2 | Invalid CLI usage |
| 3 | Syntax error |
| 4 | Division by zero |
| 5 | Numeric range error |
| 6 | Output/write error |

## Architecture

`src/calc.c` implements a locale-independent recursive-descent evaluator exposed by
`include/calc.h`. Each parser function corresponds to one precedence level and returns a typed
`calc_result`; no mutable global state or allocation is used. `src/main.c` is a thin adapter that
maps the API result to output and process exit codes.

Unity unit tests exercise the evaluator directly. A CMake script drives black-box CLI assertions,
and CTest orchestrates both suites.

## Quality and supported CI targets

Every pull request runs strict warning-as-error builds and tests on:

| Platform | Architecture | Compiler |
| --- | --- | --- |
| Linux | x86-64 | GCC and Clang |
| Linux | ARM64 | GCC |
| Windows | x86-64 | MSVC |
| macOS | ARM64 | AppleClang |

Separate jobs enforce clang-format, cppcheck, AddressSanitizer, UndefinedBehaviorSanitizer, CodeQL,
and a 90% line-coverage floor specifically for the evaluator (`99.0%` at v0.1.0). Workflow actions
are pinned to immutable commits and run with minimal permissions.

## Releases

Pushing an annotated semantic-version tag builds and tests archives for Linux x86-64, Linux ARM64,
Windows x86-64, and macOS ARM64. GitHub Actions publishes the archives, generated release notes,
and `SHA256SUMS.txt` on the matching GitHub Release.

See [CONTRIBUTING.md](CONTRIBUTING.md) for the development workflow, [AGENTS.md](AGENTS.md) for the
complete engineering policy, and [CHANGELOG.md](CHANGELOG.md) for release notes.

## License

MIT. The vendored Unity test framework retains its own MIT license in `tests/unity/LICENSE.txt`.
