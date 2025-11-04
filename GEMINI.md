# GEMINI.md

This file provides context to Gemini when working with the `cryptosaba` and `jmarket1` repositories.

## `cryptosaba` Project Overview

This is a C++23 cryptocurrency market analysis and trading system. Its primary focus is on technical analysis and processing high-frequency trade data. The system is designed to read historical trade data, perform complex calculations, and simulate market order execution.

### Key Technologies
*   **C++23:** The core language used for the project.
*   **CMake:** The build system used to manage the project.
*   **Ninja:** Used with CMake for fast builds.
*   **Boost:** Utilized for data structures like circular buffers.
*   **Eigen:** A library for linear algebra, though it's currently commented out in the main `CMakeLists.txt`.

### Architecture
The project is architected around a reactive pipeline using a publish-subscribe model.
*   **Libraries (`src/libs`):** A collection of modules for handling trades, market simulation, technical analysis, and core utilities.
*   **Scripts (`src/scripts`):** Executables for running tests, analysis, and data import/export tasks.
*   **Data Flow:** A `pubsub` library is used for data flow. Components publish data to topics, and other components subscribe to these topics to perform their work. This creates a decoupled and reactive system.

## Building and Running `cryptosaba`

The project uses CMake and Ninja for building.

**Build Commands:**
```bash
mkdir -p build
cd build
cmake ..
ninja
```

**Running Tests:**
Tests are built as executables. To run a specific test, navigate to the `build` directory and execute the test binary. For example:
```bash
./src/scripts/tests/test1/lowess_test
```

## Development Conventions

*   **Coding Style:** C++23 is the standard. Headers use the `.hpp` extension.
*   **Method Chaining:** Many classes use method chaining for a fluent interface (e.g., `object->set_option(1)->another_option(2);`).
*   **Time Representation:** Time is represented as Unix timestamps in milliseconds (`size_t`).
*   **Data:** The system uses a configuration file at `data/config.conf` (relative to the home directory) to define data paths and other settings. Trade data is stored in a binary format.

## Related Project: `jmarket1`

There is a related JavaScript project, `jmarket1`, which is used for visualizing the data produced by `cryptosaba`.

*   **Location:** `/home/payam/projects/jmarket1/`
*   **Purpose:** A web-based visualization tool for market data charts and analysis from `cryptosaba`.
*   **Technology:**
    *   **SciChart:** A high-performance WebGL charting library.
    *   **ES Modules:** For modern JavaScript modules.
*   **Build Commands:**
    *   `npm run build`: For production builds.
    *   `npm run bw`: For development builds with watch mode.
*   **Data Integration:** `jmarket1` consumes binary data and CSV files that are exported from `cryptosaba` studies.
