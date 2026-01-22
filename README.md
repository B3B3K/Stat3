# PC Stats Display on ST7735 TFT (Arduino + Python)

This project displays real-time system performance metrics from a computer on a small 160×80 ST7735-based TFT display connected to an Arduino-compatible microcontroller. System data is collected on the host PC using Python and transmitted over serial to the Arduino, which renders the information using an optimized, low-flicker drawing routine.

## Features

- CPU usage (%)
- RAM usage (GB, including swap on Linux)
- GPU usage (% approximate)
- VRAM usage (GB, NVIDIA only)
- Disk read/write speeds (MB/s)
- Network upload/download speeds (MB/s)
- Scrolling history graphs for CPU, RAM, GPU, and VRAM
- Differential screen updates for minimal flicker and good performance

## Hardware Requirements

- High Frequency microcontroller (e.g. ESP family)
- 0.96" ST7735 TFT display (160×80 resolution)
- USB CDC (ESPc3)

### Pin Connections (example for Pro Micro / Leonardo style boards)

| TFT Pin   | Microcontroller Pin |
|-----------|----------------------|
| CS        | 0                    |
| RST       | 2                    |
| DC        | 1                    |
| MOSI      | 21 (hardware SPI)    |
| SCLK      | 20 (hardware SPI)    |
| VCC       | 3.3V or 5V (display dependent) |
| GND       | GND                  |

**Note:** ESPC3-OLED and ST7735s boards are very compatible with each other, keep pinouts in eye while shopping for this project.
**Note:** CPP compiled program ready to use or u can compile it via 
**g++ -o server.exe server.cpp -lpdh -static-libgcc -static-libstdc++ -O2**
