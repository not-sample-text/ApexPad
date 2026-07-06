### ApexPad Host Daemon - Core Execution Engine Update (v1.1.1)

This release introduces a critical stability architecture patch for the host-side daemon when managing external script and application processes.

#### Core Optimization

- **PyInstaller Headless Environment Fix:** Resolved a critical runtime `OSError: [WinError 6] The handle is invalid` crash that occurred when executing external applications or Python/Tkinter GUIs from a compiled, console-less (`--noconsole`) binary.
- **Advanced Process Sandboxing:** The execution engine now utilizes an abstracted `STARTUPINFO` layout (`STARTF_USESHOWWINDOW` with `SW_HIDE`) combined with complete standard stream redirection to `subprocess.DEVNULL`. This explicitly forces Windows to cleanly decouple child processes from the parent GUI environment without throwing stream assignment faults.
- **Seamless Background Execution:** Maintained complete stealth execution; macro-mapped scripts, shell commands, and background binaries execute perfectly in the background without creating visible window flashes or taskbar interruptions.
