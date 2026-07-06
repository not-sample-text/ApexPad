# Coming Features

### 1. Web UI & OLED Synchronization

- **The Goal:** Unify the visual ecosystem.
- **Action Plan:** Overhaul the layout of the Web Configurator so the digital interface accurately mirrors the layout and state of the physical OLED screen on the device.

### 2. Dynamic Profile Management

- **The Goal:** Support multiple independent workspaces on the device without requiring a PC sync.
- **Action Plan:** Implement a profile system storing 4 distinct JSON configurations in the ESP32's SPIFFS. Develop a two-way communication protocol where the hardware dynamically loads the requested layer and broadcasts the state change back to the host daemon.

### 3. Scheduled Macros (Anti-AFK)

- **The Goal:** Support automated, time-interval keypresses.
- **Action Plan:** Implement a non-blocking background timer loop on the firmware that triggers specific macros on a user-defined schedule independent of physical keypresses.

### 4. Wireless Bluetooth Connectivity (BLE NUS)

- **The Goal:** Cut the cord while maintaining high-speed serial communication.
- **Action Plan:** Transition from a standard USB serial connection to a custom Bluetooth Low Energy GATT server utilizing the Nordic UART Service (NUS) over NimBLE for low-latency wireless macro execution.

### 5. Addressable LED Integration (Hardware Revision)

- **The Goal:** Full RGB illumination.
- **Action Plan:** Route new trace pathways and power distribution for the LED data lines on the bottom PCB layout in KiCad, manufacture the revised board, and write the addressable lighting loop in the firmware.

### 6. Path Not Found Error Handling (Host Daemon)

- **The Goal:** Provide clear user feedback when an executed script or application path is invalid.
- **Action Plan:** Implement an error window/dialog in the Python script that visually alerts the user when a triggered application or script path cannot be found, preventing silent failures.
