# Matching-Engine

[![CI](https://github.com/petroskoukios/Matching-Engine/actions/workflows/ci.yml/badge.svg)](https://github.com/petroskoukios/Matching-Engine/actions/workflows/ci.yml)

A limit order book and matching engine in C++20, built correctness-first as
the foundation for low-latency work: every optimization will be measured
against this tested baseline.

## What's implemented

- **Limit order book** - price-time priority, O(1) best bid/ask, cancel via
  an id index into iterator-stable level queues, strict validation at the API
  boundary (`[[nodiscard]]` error codes, `std::optional` for empty-book
  queries - no sentinel values)
- **Matching engine** - limit and market orders, partial fills, multi-level
  sweeps; trades execute at the resting order's price; market-order
  remainders are discarded, never rested
- **Differential testing** - a deliberately naive reference matcher (unsorted
  vectors, every query computed by linear scan) serves as an oracle: 100
  seeded random order streams x 200 operations are applied to both engines,
  comparing returned error codes, emitted trades, best bid/ask, and per-level
  quantities after every operation, plus a book-never-crossed invariant.
  Failures print their seed and operation index for exact replay.
- **Fault-injection check** - four deliberate bugs (wrong trade price, FIFO
  inversion, order-index leak, crossing off-by-one) were injected into the
  engine; the differential suite caught all four.

## In progress

- Benchmark harness (Google Benchmark) - throughput and latency percentiles
  for the current `std::map` baseline, then measured optimization passes
- NASDAQ ITCH 5.0 replay
- Lock-free SPSC order intake

## Design notes

- Prices are scaled integer ticks (`int64_t`) - never floating point
- Price levels: `std::map` per side (bids descending, asks ascending), FIFO
  `std::list` per level; time priority is queue position, so orders carry no
  timestamps
- Order lookup: `unordered_map` from id to list iterator - cancels do not
  scan
- Two build configurations, kept strictly separate: a **correctness** build
  (`-Werror`, AddressSanitizer + UBSan) used for all tests and CI, and a
  **perf** build (`-O3`, no sanitizers) used only for benchmarks

## Building

Requires CMake >= 3.21 and a C++20 compiler (developed with g++ 13).

```bash
cmake --preset correctness && cmake --build --preset correctness
ctest --preset correctness
```

Perf build: `cmake --preset perf && cmake --build --preset perf`

## Scope

A learning-driven systems project, not production exchange software: single
instrument, no networking, no persistence, plain limit/market orders only.

## License

[MIT](LICENSE)