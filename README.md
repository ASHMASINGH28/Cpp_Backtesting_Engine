# Event-Driven Backtesting Engine (C++17)

A minimal but architecturally "real" backtesting engine, built the same way
production trading systems are structured: discrete events (Market → Signal →
Order → Fill) flowing through a queue, decoupling data, strategy, portfolio,
and execution so any one piece can be swapped without touching the others.

## Architecture

```
DataHandler --MarketEvent--> Strategy --SignalEvent--> Portfolio
     ^                                                      |
     |                                                 OrderEvent
     |                                                      v
     +------------------------------------------- ExecutionHandler
                         FillEvent
```

- **DataHandler** — reads OHLCV bars from CSV, reveals them one date at a
  time (never exposes future bars) across a shared multi-symbol calendar.
- **Strategy** — consumes `MarketEvent`s, emits `SignalEvent`s. Two are
  included: `SMACrossoverStrategy` (dual moving-average crossover, long-only)
  and `BuyAndHoldStrategy` (used as the benchmark).
- **Portfolio** — turns signals into sized `OrderEvent`s, tracks cash and
  positions, marks-to-market every bar to build the equity curve.
- **ExecutionHandler** — simulates fills with slippage (bps) and a flat
  commission per trade.
- **PerformanceMetrics** — total return, CAGR, Sharpe ratio, max drawdown.

## Build

Requires a C++17 compiler. No external dependencies for the core engine.

```bash
g++ -std=c++17 -O2 -Iinclude src/*.cpp -o backtester
g++ -std=c++17 -O2 -Iinclude tools/generate_sample_data.cpp -o generate_sample_data
```

(A `CMakeLists.txt` is included too, if you have cmake installed:
`mkdir build && cd build && cmake .. && make`.)

## Run

**1. Get data.** Fastest path — generate synthetic random-walk data locally,
no internet required:

```bash
./generate_sample_data data 750
```

This writes `data/AAPL.csv` and `data/SPY.csv` in the same column format
real data uses (`Date,Open,High,Low,Close,Adj Close,Volume`), so swapping in
real data later is a drop-in replacement.

To use **real** historical data instead:
- Download manually from Stooq: `curl "https://stooq.com/q/d/l/?s=aapl.us&i=d" -o data/AAPL.csv`
- Or build the optional fetcher (`tools/fetch_data.cpp`, needs libcurl):
  `g++ -std=c++17 tools/fetch_data.cpp -o fetch_data -lcurl && ./fetch_data AAPL data`
- Or use Python's `yfinance` to export CSVs into the `data/` folder.

**2. Run the backtest:**

```bash
./backtester data AAPL 100000
```

Args: `<dataDir> <symbol> <initialCapital>`. Prints a performance report for
both the SMA crossover strategy and the buy-and-hold benchmark, and writes
`output_<name>_equity.csv` for each so you can plot the equity curve.

## Design notes / known simplifications

- **Fill timing**: orders fill at the *same* bar's adjusted close (plus
  slippage), not the next bar's open. This is a common simplification for
  daily-bar backtests but is technically a small amount of lookahead. The
  natural "v2" improvement is delaying fills to the next bar's open in
  `ExecutionHandler`.
- **Long-only, single position per symbol**: no shorting, no position
  scaling in/out. Extending `Portfolio`/`Strategy` to support these is a good
  next step.
- **Adjusted close**: signals and PnL use `Adj Close`, not raw `Close`, so
  stock splits/dividends don't create fake price jumps in the signal.
- **No survivorship-bias correction**: like most simple backtests, this only
  tests symbols that exist today. A more rigorous version would use a
  point-in-time universe including delisted names.

## Possible extensions

- Walk-forward parameter optimization (train SMA windows on one window, test
  on the next) to guard against overfitting.
- Multithreaded grid search over strategy parameters (`std::thread` / thread
  pool) — good for demonstrating concurrency skills.
- Short selling and margin modeling.
- Intraday/minute-bar support (swap the CSV source for Alpha Vantage
  intraday data).
- Google Test unit tests for the SMA calculation and performance metrics.
