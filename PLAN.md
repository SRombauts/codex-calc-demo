# Implementation plan

- [x] Phase 1 — Bootstrap Git/GitHub, record engineering rules, add the CMake
      skeleton, and document the design.
- [x] Phase 2 — Add cross-platform build, quality, security, and coverage CI;
      validate it through a reviewed pull request.
- [x] Phase 3 — Implement the C17 evaluator and CLI with Unity/CTest unit and
      integration coverage; review, fix findings, and merge only when green.
- [x] Phase 4 — Harden edge cases and portability, complete documentation, run
      full local/remote validation, review the final diff, and merge.
- [ ] Phase 5 — Annotate and push `v0.1.0`; let GitHub Actions build release
      artifacts and publish the GitHub Release, then verify all deliverables.

## Definition of done

`calc` evaluates the documented grammar, produces stable diagnostics and exit
codes, passes strict builds/tests/analyzers/sanitizers on supported platforms,
meets the 90% evaluator line-coverage gate, and has a reproducible tagged
release containing all four platform/architecture artifacts and checksums.
