# Matching-Engine

[![CI](https://github.com/petroskoukios/Matching-Engine/actions/workflows/ci.yml/badge.svg)](https://github.com/petroskoukios/Matching-Engine/actions/workflows/ci.yml)

A limit order book and matching engine written in C++20, designed
for low-latency operation. It implements price–time priority matching with
support for limit and market orders, partial fills, and order
cancellation/modification.

Built as a systems project with a trading application: correctness first
(invariant and randomized differential testing), then measured
performance, benchmark results will be published here as the project
develops.

**Status:** in active development (August 2026). Core matching engine in
progress.

## Building

CMake-based build with separate correctness and performance
configurations.

## License

MIT