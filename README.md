Small esp-idf application demonstrating [Tamp](https://github.com/BrianPugh/tamp) on the first 100KB of the enwik8 dataset.

[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/BrianPugh/esp-tamp-demo/main.svg)](https://results.pre-commit.ci/latest/github/BrianPugh/esp-tamp-demo/main)

# Usage
After activating your esp-idf environment, just run (replacing the port with your device's port):
```bash
idf.py -p /dev/tty.usbserial-0001 flash monitor
```
This will compress the first 100KB of the enwik8 dataset and print the compression/decompression performance.

# Performance
All benchmarks were performed with esp-idf v5.4 on an ESP32 with default settings. Notably:

* 160 MHz
* DIO 2MB flash at 40MHz

### Compression

|                 | Time (s) | Performance Boost |
|-----------------|----------|-------------------|
| Default         | 4.461    | Baseline          |
| ESP32-Optimized | 2.045    | 2.18x             |

### Decompression

|                 | Time (s) | Performance Boost |
|-----------------|----------|-------------------|
| Default         | 0.093    | Baseline          |


### Acknowledgement
All esp32-specific optimizations were implemented by [@BitsForPeople](https://github.com/BitsForPeople) in his [esp-tamp fork](https://github.com/BitsForPeople/esp-tamp/tree/main).
