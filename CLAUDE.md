# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C++23 cryptocurrency market analysis and trading system ("cryptosaba") focused on technical analysis and trade data processing. The codebase processes historical trade data, performs various technical analysis calculations, and supports simulated market order execution.

## Build System

**CMake + Ninja** with LLD linker (lld-21)
- C++23 standard required
- Compiler optimization: `-O3`
- Build commands:
  ```bash
  mkdir -p build
  cd build
  cmake ..
  ninja
  ```
- Single test execution: Navigate to build directory and run the executable directly, e.g., `./src/scripts/tests/test1/lowess_test`

## Project Architecture

### Core Library Structure

The codebase is organized into **libraries** (`src/libs/`) and **scripts** (`src/scripts/`):

**Core Infrastructure:**
- `libs/core/config/` - Singleton configuration manager that loads from `data/config.conf` (relative to home path). Provides typed getters for config values.
- `libs/core/pubsub/` - Publish-subscribe pattern implementation (Meyers singleton). Used extensively for reactive data pipelines where components subscribe to topics and receive callbacks when data is published.
- `libs/utils/` - Utility functions for data manipulation, file operations, string processing, datetime conversion, and random number generation

**Trade Data:**
- `libs/trade/trade.hpp` - Core `Trade` struct (price, volume, quantity, timestamp, is_buyer_maker) and `TradeReader` for reading binary trade files by symbol
- `libs/trade/tradecache.hpp` - High-performance circular buffers for caching trades with time-indexed access (`TradeCache`, `TradeCache2`, `TradeCacheSimple`). Maintains indexes at multiple timeframes (1s, 5s, 10s, 30s, 1m, 5m, 10m, 30m, 1h)

**Market Simulation:**
- `libs/market/market.hpp` - `Market` class that simulates order execution. Manages pending, active, and completed orders. Supports market and limit orders with stop-loss, take-profit, activation/cancel prices. Generates `MarketReport` with win rates, profit stats, order durations.

**Technical Analysis (`libs/ta/`):**
- `candles/` - OHLCV candle aggregation from trades. `Candles` (circular buffer) and `CandlesVector` support building higher timeframes from 1s candles
- `zigzag/` - ZigZag indicator implementations (`ZigZag`, `ZigZagEnhanced`) that identify swing highs/lows based on price delta thresholds
- `lowess/` - Locally weighted regression (LOWESS) smoothing for candle data, weighted by volume
- `linear_regression/` - Linear regression calculations for trend analysis
- `logscale/` - Logarithmic scale transformations
- `frames/` - Time-based frames for aggregating trade statistics (VWAP, volume, highs/lows) at specified millisecond intervals
- `stepper/` - Tools for stepping through time-series data
- `volumebox/` - Volume-based analysis structures
- `trendlines/up_trend_line_zigzag/` - Trend line calculation from zigzag points
- `vnode/` - Volume node analysis (commented out in main CMakeLists.txt)

**Statistics:**
- `libs/statistics/` - `LiveStats` and `LiveStatsFixed` for online statistical calculations (mean, variance, stddev, min/max) without storing all data points

### Data Flow Pattern

The system uses a **reactive pipeline** architecture via PubSub:
1. Trade data is published to "trades" topic
2. Components subscribe to topics and react to new data
3. Example: `TradeReader::pubsub_trades()` → `Candles::subscribe_to_pubsub_trades()` → `ZigZag::subscribe_to_pubsub()` → `Market::subscribe_to_pubsub()`
4. Many classes have `subscribe_to_pubsub()` methods that return `this` for method chaining

### Scripts

**Tests** (`src/scripts/tests/`):
- `test1/` - Utility tests, Eigen library tests (commented), LOWESS tests
- `candles_test/` - Candle aggregation validation
- `trendline_test/` - Trend line calculation tests
- `scene_test/` - Scene-based testing scenarios (`scene1`, `scene_info`)
- `trade_tests/` - Trade data validation (timestamp ordering, quantity/volume integrity)

**Analysis & Import:**
- `market_anal/` - Market analysis scripts producing chart data
- `trade_file_importer/` - Import and process raw trade files

### External Dependencies

- **Boost** - Circular buffers extensively used throughout (`boost::circular_buffer`)
- **Eigen** - Matrix operations (currently commented out in CMakeLists.txt)
- External headers in `/usr/local/include`

## Development Notes

### File Extensions
- Headers use `.hpp` extension (not `.h`)
- Implementation files use `.cpp`

### Data Files
- Configuration file: `data/config.conf` (relative to home path, gitignored)
- Trade data stored as binary files organized by symbol
- The system reads config to get symbols list and data paths

### Memory Management
- Extensive use of circular buffers for bounded memory consumption
- Singleton pattern for shared resources (Config, PubSub, TradeCaches)
- Iterators stored in circular buffers for efficient time-indexed access

### Common Patterns
- **Method chaining**: Many initialization methods return `this*` to allow fluent interface (e.g., `market->set_commision(0.001)->subscribe_to_pubsub()`)
- **Time representation**: Unix timestamps in milliseconds (size_t)
- **Price representation**: Double precision floats

### Testing Strategy
Tests validate:
- Trade data integrity (no negative values, correct price = quantity/volume relationship)
- Candle aggregation correctness across timeframes
- Technical indicator calculations
- Market simulation order execution logic

## Related Projects

### jmarket1 - Visualization Library

**Location**: `../jmarket1` (relative to cryptosaba root)

**Purpose**: JavaScript/TypeScript visualization library for rendering market data charts and analysis results from cryptosaba

**Tech Stack**:
- **SciChart** - High-performance WebGL charting library (v4.0.890)
- **esbuild** - Module bundler for development and production builds
- **ES Modules** - Modern JavaScript module system

**Project Structure**:
- `index.html` - Main HTML file (root of project)
- `src/index.js` - JavaScript entry point
- `src/` - Source code
  - `chart/` - Chart components and configurations
  - `elements/` - UI elements
  - `lib/` - Library utilities
  - `scenarios/` - Different visualization scenarios
  - `config.js` - Configuration
- `js/` - Build output directory (bundled files)
- `css/` - Stylesheets
- `images/` - Image assets

**Build Commands**:
- `npm run build` - Build for production
- `npm run bw` - Build and watch for development (with sourcemaps)

**Data Integration**:
- Consumes binary data exported from cryptosaba studies (e.g., snapshot_analyze)
- Reads candle data, normalized volumes, and scaled prices
- Metadata from CSV files

**Development Notes**:
- When working on both projects, use absolute paths for jmarket1: `/home/payam/projects/jmarket1/`
- Binary data files are written to cryptosaba's `config.files_path` directory
- jmarket1 reads these files for visualization
