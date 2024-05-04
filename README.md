Small esp-idf application demonstrating [Tamp](https://github.com/BrianPugh/tamp) on the first 100KB of the enwik8 dataset.

[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/BrianPugh/esp-tamp-demo/main.svg)](https://results.pre-commit.ci/latest/github/BrianPugh/esp-tamp-demo/main)

# Performance
All esp-idf

### Compression

|                 | Time (s) | Performance Boost |
|-----------------|----------|-------------------|
| Default         | 5.750    | Baseline          |
| ESP32-Optimized | 2.246    | 2.56x             |

### Decompression


### Acknowledgement
All esp32-specific optimizations were implemented by @BitsForPeople in his [esp-tamp fork](https://github.com/BitsForPeople/esp-tamp/tree/main).
