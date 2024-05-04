Small esp-idf application demonstrating [Tamp](https://github.com/BrianPugh/tamp) on the first 100KB of the enwik8 dataset.

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
