### ApexPad Ecosystem - Feature Expansion & Stability Update (v1.2.0)

This release introduces powerful new execution privileges for the host daemon, a migration to a highly resilient file system on the hardware, and critical anti-freezing optimizations across the entire serial communication stack.

#### New Features & Capabilities

- **Elevated Execution Control:** The Web Configurator and Python Daemon now support granular execution flags for Scripts and Applications. Users can natively trigger "Run as Administrator" (utilizing Windows UAC `ctypes` elevation) and "Run Headless" directly from the macropad.
- **Emergency Configuration Dump:** Added a failsafe "Force Dump to Desktop" option to the Daemon's system tray menu. This allows users to bypass schema validation and extract raw JSON data directly from the macropad to the desktop for debugging.
- **Verbose Hardware Monitoring:** The Python Daemon now actively listens for and routes `[EVENT]`, `[LOG]`, and `[SYSTEM]` outputs from the firmware directly to the UI Debug Console, providing real-time insight into hardware states and matrix scans.
- **Targeted JSON Error Reporting:** If a configuration file is malformed, the Python Daemon now intercepts the `JSONDecodeError`, logging the exact line number, column number, and a snippet of the problematic text to make manual debugging effortless.

#### Core Optimizations

- **LittleFS Migration:** The ESP32-S3 firmware has been completely migrated from the deprecated SPIFFS architecture to LittleFS, significantly improving file read/write speeds, flash wear-leveling, and protection against power-loss corruption.
- **Dynamic Serial Pacing:** The configuration upload protocol now dynamically calculates transmission delays based on payload size. Data is streamed line-by-line with a strict 5.0-second timeout, entirely eliminating serial buffer overflows during large configuration syncs.
- **Non-Blocking Hardware Event Queue:** The firmware's main polling loop has been refactored. The event queue now instantly bypasses unmapped keys and defers OLED I2C redraws until all rapid keystrokes are processed, eliminating microcontroller freezing during rapid multi-key presses.
- **Native USB CDC Unblocking:** The firmware now implements a strict 25ms TX timeout. If the host computer is busy, the ESP32 will instantly drop overflow logs rather than halting the entire processor waiting for the serial buffer to clear.

#### Bug Fixes

- **Path Sanitization:** The Web Configurator and Python Execution Handler now automatically strip literal surrounding quotation marks (often introduced via Windows "Copy as path") before interacting with the OS subprocess module, preventing execution crashes.
