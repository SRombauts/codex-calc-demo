# calc

`calc` is a small, dependency-free ISO C17 command-line expression calculator.
It is intentionally compact: the interesting part of the project is its
cross-platform engineering workflow, automated quality gates, and reproducible
release process.

The evaluator uses a recursive-descent parser with one layer per precedence
level. The public evaluator API is isolated from the CLI, while tests use the
vendored Unity test framework and CTest orchestration.

Implementation and usage documentation will be completed with the calculator
feature.

## Development

See [AGENTS.md](AGENTS.md) for repository policy and [PLAN.md](PLAN.md) for the
phased implementation task list.

## License

MIT

