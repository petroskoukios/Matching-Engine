# Matching-Engine

[![CI](https://github.com/petroskoukios/Matching-Engine/actions/workflows/ci.yml/badge.svg)](https://github.com/petroskoukios/Matching-Engine/actions/workflows/ci.yml)

A limit order book in C++20, designed for low-latency operation.

**Implemented:** price-time priority order book (add, cancel, best bid/ask,
level aggregation), validated at the API boundary, unit-tested with
sanitizers in CI.

**In progress:** matching engine (limit and market orders, partial fills),
randomized differential testing against a reference matcher, benchmarks.

**Status:** in active development (August 2026). Core matching engine in
progress.

## Building

CMake-based build with separate correctness and performance
configurations.

## License

MIT