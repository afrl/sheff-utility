# SHEFF Utility (`sheff-utility`)

`sheff-utility` is the foundational C++ utility library of the **Scalable Hierarchical Enrichment Finite-element Framework (SHEFF)**. It provides low-overhead containers, memory views, binary/JSON serialization, MPI communication helpers, profiling timers, filesystem abstractions, and exception/diagnostic macros shared across every SHEFF module.

## Capabilities

- **Compact Containers & Non-Owning Views**:
  - `DynamicArray`, `DynamicMatrix`, `VectorSet` (sorted flat set), `Bitmap`, `Bitset`, and `DynamicBitset`.
  - Non-owning array/range abstractions (`Slice`, `SliceList`, `UniquePtrSlice`, `IteratorRange`, `Range`, `Permutation`, `ContainerSearch`, `EraseByIndex`) for zero-copy traversal of connectivity and field buffers.
- **Binary & JSON Serialization (`utility/serialization/`)**:
  - Uniform serialization framework (`Serialization.h`, `NamedValue.h`) supporting compact binary streams (`OutputSerializer`, `InputDeserializer`, `LocatableBinarySerializer`), byte-size pre-counting (`SerialByteCounter`), and human-readable JSON inspection (`JSONSerializer`).
- **MPI & Distributed Utilities (`utility/MPIUtils.h`)**:
  - Optional MPI wrappers (`SHEFF_USE_MPI`) for rank/size queries, collective reductions, barriers, and serialized object exchange across ranks.
- **Diagnostics, Profiling & System Helpers**:
  - Rich exception types and source-location macros (`Exceptions.h`), hierarchical runtime profiling (`Profile.h`), binary/ASCII stream helpers (`IOUtils.h`, `ASCIIStreamHelpers.h`), string manipulation and natural sorting (`StringUtils.h`, `NaturalSort.h`), hashing (`Hash.h`), and cross-platform filesystem/OS queries (`Filesystem.h`, `SystemInfo.h`, `CrossplatformHelpers.h`).

## Project Structure

- `include/utility/`: Public headers for containers, slices, MPI helpers, profiling, exceptions, and `serialization/`.
- `source/`: Compiled implementations for profiling, filesystem, string, exception, and MPI utilities.
- `unit_tests/`: Comprehensive unit test suite covering containers, slices, serialization, and MPI helpers.
- **CMake Package / Target**: `Utility` / `Utility::utility` (static library).

## Dependencies

- **Build System**: [`sheff-build-tools`](https://github.com/afrl/sheff-build-tools) (`SHEFF_TOOLS`), CMake 3.17+, C++17/C++20 compiler.
- **Third-Party Libraries**: [`nlohmann/json`](https://github.com/nlohmann/json) (`nlohmann_json::nlohmann_json`).
- **Optional External Libraries**: MPI (`--use-mpi` / `-DSHEFF_USE_MPI=ON`).

## Building

When building within [SPAWC](https://github.com/afrl/spawc), `sheff-utility` is built automatically via the top-level `make.py`.

To build standalone (with `SHEFF_TOOLS` set to [`sheff-build-tools`](https://github.com/afrl/sheff-build-tools)):

```bash
python3 make.py --config --build --third-party
python3 make.py --config --build --use-mpi --make-tests --run-tests
```

## Public Release Information

Distribution Statement A. Approved for public release: distribution is unlimited. Case #: AFRL-2026-4513
