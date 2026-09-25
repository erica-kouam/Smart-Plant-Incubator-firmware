# Smart Plant Incubator - Non-Blocking Concurrent Firmware
An automated environmental control system built on the ESP32 platform using optimized C++ firmware. 

## 🧠 Technical Execution & Concurrency
The primary engineering challenge of this system was resolving synchronization loops between time-sensitive sensor polling and high-latency network tasks. 

* **The Problem:** Standard network API requests (SSL Handshakes, network handshakes) freeze microcontroller execution loops. High-latency network calls stall sensor collection paths.
* **The Solution:** Developed a structured, non-blocking network-layer communication stack. Implemented an asynchronous network client mechanism to ensure real-time sensor paths (I2C/SPI tracking soil moisture and temperature parameters via DHT22) execute deterministically without system blocks.

## 🛠️ Technology Stack
* **Hardware:** ESP32 Microcontroller (Tensilica Xtensa Dual-Core)
* **Firmware Language:** C++ (Object-Oriented Programming, state-machine layouts)
* **Peripherals Integration:** I2C, SPI Bus configurations
* **Network Layers:** Wi-Fi / BLE stack routing to a localized Telegram Bot API client
