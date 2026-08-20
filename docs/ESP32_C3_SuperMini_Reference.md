# ESP32-C3 SuperMini Reference Guide

## Overview
The **ESP32-C3 SuperMini** is a highly compact development board based on the Espressif ESP32-C3 SoC. Built around a 32-bit RISC-V single-core processor, it offers excellent performance, built-in Wi-Fi and Bluetooth LE, and a versatile set of peripherals. Its minimal footprint makes it ideal for embedded applications with strict size constraints.

## Hardware Specifications
* **Processor:** ESP32-C3 (32-bit RISC-V single-core up to 160 MHz)
* **Memory:** 400 KB SRAM, 384 KB ROM, 8 KB RTC SRAM
* **Storage:** 4 MB external SPI Flash
* **Wireless Connectivity:** 2.4 GHz Wi-Fi (802.11 b/g/n) and Bluetooth 5 (LE)
* **Hardware Interfaces:** 
  * 1x I2C
  * 1x SPI
  * 2x UART
  * Up to 11x GPIOs
  * 4x 12-bit ADC channels
* **Power & Data:** USB Type-C (Internal USB Serial/JTAG controller)
* **Dimensions:** ~22.5mm x 18mm

## Zephyr OS Integration
The board is officially supported in the Zephyr Project ecosystem. The board identifier used for building applications is `esp32c3_supermini`.

### Common Build Commands
To compile an application for this board:
```bash
west build -b esp32c3_supermini app/
```

To flash the firmware via the USB Type-C connection:
```bash
west flash
```

## Reference Links
* **[Zephyr Official Documentation: ESP32-C3 SuperMini](https://docs.zephyrproject.org/latest/boards/others/esp32c3_supermini/doc/index.html)**
  The primary Zephyr resource containing board configurations, bootloader details, and supported features.
* **[ESP32-C3 Series Datasheet (PDF)](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)**
  Official Espressif datasheet detailing electrical characteristics, pin definitions, and hardware limits.
* **[ESP32-C3 Technical Reference Manual (PDF)](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf)**
  In-depth architecture guide covering registers, memory mapping, and detailed peripheral operation (I2C, SPI, Timers).
* **[Zephyr Hardware Abstraction Layer (HAL) for Espressif](https://github.com/zephyrproject-rtos/hal_espressif)**
  The repository containing the lower-level drivers and ESP-IDF modules integrated into Zephyr.
