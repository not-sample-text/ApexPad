# Current Issues

### 1. Fix Headless Execution for GUI Applications

- **The Issue:** Apps triggered by the macropad are launching invisibly in the background.
- **Action Plan:** Refactor the `execution_handler.py` logic to strip the `STARTUPINFO` and `DEVNULL` kwargs from the `_launch_app` method, restoring normal window creation for standard applications while keeping script execution fully silent.

### 2. Daemon RAM & Thread Optimization

- **The Issue:** The daemon idles at 20MB of RAM because the Tkinter debug UI thread spins up simultaneously with the main listener loop.
- **Action Plan:** Defer the instantiation of the `DebugWindow` class. Tie the thread creation strictly to the user clicking the system tray icon, dropping the background footprint down to ~1MB.

### 3. Dynamic Execution Controls (Web UI & Host Integration)

- **The Issue:** The current JSON payload doesn't support granular execution permissions.
- **Action Plan:** Update the Web Configurator schema to include boolean toggles for "Run as Admin (UAC)" and "Silent Execution." Update the Python execution handler to parse these flags and conditionally apply `runas` elevation or terminal suppression per script.

### 4. System Stability & Crash Logging

- **The Issue:** The Python app occasionally crashes silently without leaving a traceback.
- **Action Plan:** Implement a persistent file-logging system (`apexpad.log`) that captures `stderr` and application state right alongside the main execution loop to trap and isolate the random crashes.
