Small esp-idf application demonstrating [Tamp](https://github.com/BrianPugh/tamp) on the first 100KB of the enwik8 dataset.

[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/BrianPugh/esp-tamp-demo/main.svg)](https://results.pre-commit.ci/latest/github/BrianPugh/esp-tamp-demo/main)

# Usage

After activating your esp-idf environment, just run (replacing the port with your device's port):

```bash
idf.py -p /dev/tty.usbserial-0001 flash monitor
```

This will compress the first 100KB of the enwik8 dataset and print the compression/decompression performance.

# Performance

All benchmarks were performed on three hardware targets:

* ESP32 (Xtensa LX6)
* ESP32-S3 (Xtensa LX7)
* ESP32-C3 (RISC-V)

All targets used identical settings:

* Tamp v2.3.0
* esp-idf v6.0.2
* CPU at 160 MHz
* DIO 2MB flash at 80MHz
* Compiler set to `-O2` (`COMPILER_OPTIMIZATION_PERF=y`)

The esp-optimizations can be enabled by running `idf.py config` and toggling `CONFIG_TAMP_ESP32`. It applies a rewritten match-search algorithm on all targets, plus a target-specific inner loop:

* ESP32-S3: SIMD (PIE) vector instructions (largest speedup)
* ESP32 (LX6): hand-tuned Xtensa scalar assembly
* ESP32-C3 (RISC-V): no target-specific inner loop, so its speedup comes only from the shared algorithm

### Compression

With a 10bit window, the 100,000 byte file compresses to 51637 bytes.

|                 | ESP32           | ESP32-S3        | ESP32-C3        |
|-----------------|-----------------|-----------------|-----------------|
| Default         | 1.756 s         | 1.487 s         | 1.413 s         |
| ESP32-Optimized | 1.708 s (1.03x) | 0.254 s (5.85x) | 0.961 s (1.47x) |

### Decompression

|                 | ESP32           | ESP32-S3        | ESP32-C3        |
|-----------------|-----------------|-----------------|-----------------|
| Default         | 0.068 s         | 0.053 s         | 0.040 s         |
| ESP32-Optimized | 0.067 s (1.01x) | 0.050 s (1.06x) | 0.036 s (1.11x) |

# Local Development

To use a local copy of the `tamp` component:

1. go into `tamp/espidf/tamp` and run `make`
2. Modify `main/idf_component.yml` to point at that folder; e.g.:

   ```yaml
   brianpugh/tamp:
     path: ../../tamp/espidf/tamp
   ```

3. Build and flash. You will have to run `make` in the `tamp/espidf/tamp` directory each time you make changes to copy the modified files into the "staged" esp-idf component.

### Acknowledgement

All esp32-specific optimizations were implemented by [@BitsForPeople](https://github.com/BitsForPeople) in his [esp-tamp fork](https://github.com/BitsForPeople/esp-tamp/tree/main).
