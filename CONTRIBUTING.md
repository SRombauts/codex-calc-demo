# Contributing

## Workflow

1. Create a short-lived `feature/*`, `fix/*`, or `hardening/*` branch from `main`.
2. Keep commits atomic and add a regression test for behavior changes.
3. Run the developer build and tests, then the checks relevant to the change.
4. Open a pull request with motivation, validation evidence, and risks.
5. Address the independent read-only review and wait for every required CI check before squash merge.

The detailed coding, review, CI, Git, and release rules are normative in [AGENTS.md](AGENTS.md).

## Local checks

```console
cmake --preset dev && cmake --build --preset dev
ctest --preset dev --output-on-failure
clang-format --dry-run --Werror include/*.h src/*.c tests/test_calc.c
cppcheck --error-exitcode=1 --enable=warning,style,performance,portability \
  --std=c17 --language=c --inline-suppr -I include include src tests/test_calc.c
```

Sanitizer presets are `asan` and `ubsan`. Coverage uses the `coverage` preset followed by gcovr with
`--filter 'src/calc.c' --fail-under-line 90`.
