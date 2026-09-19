# Project engineering rules

These rules apply to the entire repository.

## Scope and design

- Build a compact ISO C17 command-line expression calculator.
- Keep the parser dependency-free and use recursive descent with the grammar
  `expression -> term ((+|-) term)*`, `term -> unary ((*|/) unary)*`,
  `unary -> (+|-) unary | primary`, and `primary -> number | (expression)`.
- Separate the reusable evaluator from CLI argument handling. Avoid speculative
  abstractions and platform-specific code.
- Treat non-finite or out-of-range numeric literals/results as evaluation
  errors. Division by positive or negative zero is a dedicated error.

## Coding style and comments

- Compile as ISO C17 with extensions disabled.
- Format C/CMake files with the repository `.clang-format` configuration.
- Use four spaces, braces on the same line, `snake_case` functions/variables,
  and `UPPER_SNAKE_CASE` constants. Public names use the `calc_` prefix.
- Prefer small functions, immutable inputs, explicit ownership, and no global
  mutable state. Check all fallible operations.
- Comments explain intent, invariants, or non-obvious edge cases; never narrate
  straightforward code. Public API declarations receive concise documentation.
- Builds must be warning-clean under the strict warning sets configured by
  CMake for GCC, Clang/AppleClang, and MSVC.

## Tests and local validation

- Every behavior change needs focused Unity unit tests. Regression fixes add a
  test that fails before the fix.
- Tests cover precedence, grouping, unary signs, whitespace, decimals,
  malformed input, trailing tokens, division by zero, and numeric range errors.
- Before requesting review, run:
  `cmake --preset dev`, `cmake --build --preset dev`, and
  `ctest --preset dev --output-on-failure`.
- Also run formatting verification, cppcheck, ASan/UBSan, and coverage when the
  change can affect them. CI is the cross-platform source of truth.
- Do not weaken warnings, analyzers, sanitizer settings, or coverage thresholds
  merely to make a check pass.

## Git workflow, pull requests, and review

- `main` is always releasable. A minimal bootstrap may be committed directly;
  substantive work uses short-lived `feature/*` or `hardening/*` branches.
- Commits are atomic, imperative, and scoped to one coherent concern. Do not
  rewrite shared history or force-push unless recovery explicitly requires it.
- Each substantive branch is pushed and opened as a PR with motivation, change
  summary, validation evidence, and risk notes.
- After local validation, assign exactly one read-only reviewer subagent to
  inspect the complete PR diff for correctness, portability, safety, tests,
  maintainability, and compliance with this file. The implementer triages every
  finding, fixes relevant issues in atomic commits, reruns affected checks, and
  records the outcome in the PR before merge.
- Merge only after required CI checks succeed. Use squash merge for a compact
  public history, then delete the remote feature branch.

## CI

- GitHub Actions must exercise Linux x86-64 and ARM64, Windows x86-64, and
  macOS ARM64 with GCC, Clang/AppleClang, and MSVC where applicable.
- Quality automation includes strict warnings, clang-format, cppcheck,
  AddressSanitizer, UndefinedBehaviorSanitizer, CodeQL, and line coverage.
- Coverage uses GCC/gcov/gcovr and enforces at least 90% line coverage for the
  evaluator library. CI uploads useful logs/reports even when practical.
- Pin third-party Actions to immutable commit SHAs. Grant each workflow only the
  minimum permissions it needs.

## Release procedure

- A release candidate must be green on `main`, documented, and versioned in
  CMake. Create and push an annotated `vMAJOR.MINOR.PATCH` tag.
- The release workflow builds tested archives for Linux x86-64, Linux ARM64,
  Windows x86-64, and macOS ARM64, publishes SHA-256 checksums, and creates the
  matching GitHub Release automatically with generated notes.
- Verify the tag, release page, artifacts, and checksums after publication.

