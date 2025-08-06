Small esp-idf application demonstrating [Tamp](https://github.com/BrianPugh/tamp) on the first 100KB of the enwik8 dataset.

[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/BrianPugh/esp-tamp-demo/main.svg)](https://results.pre-commit.ci/latest/github/BrianPugh/esp-tamp-demo/main)

# Usage

After activating your esp-idf environment, just run (replacing the port with your device's port):

```bash
idf.py -p /dev/tty.usbserial-0001 flash monitor
```

This will compress the first 100KB of the enwik8 dataset and print the compression/decompression performance.

# Performance

All benchmarks were performed with:

* Tamp v1.10.0
* esp-idf v5.4
* ESP32 hardware with default settings. Notably:
  * 160 MHz
  * DIO 2MB flash at 40MHz

The esp-optimizations can be enabled by running `idf.py config` and enabling/disabling `TAMP_ESP32`.

### Compression

With a 10bit window, the 100,000 byte file compresses to 51637 bytes.

|                 | Time (s) | Performance Boost |
|-----------------|----------|-------------------|
| Default         | 4.461    | Baseline          |
| ESP32-Optimized | 1.756    | 2.54x             |

### Decompression

|                 | Time (s) | Performance Boost |
|-----------------|----------|-------------------|
| Default         | 0.093    | Baseline          |

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
