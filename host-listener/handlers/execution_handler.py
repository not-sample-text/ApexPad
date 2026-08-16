import subprocess
import os
import sys
import webbrowser
import shlex

from utils.logger import get_logger

logger = get_logger("ExecutionHandler")

class ExecutionHandler:
    @staticmethod
    def _get_silent_kwargs():
        """
        Generates subprocess kwargs that survive PyInstaller --noconsole environments.
        Forces the OS to safely detach standard I/O streams without crashing.
        """
        kwargs = {
            'stdin': subprocess.DEVNULL,
            'stdout': subprocess.DEVNULL,
            'stderr': subprocess.DEVNULL
        }
        
        if sys.platform == 'win32':
            # PyInstaller specific workaround to prevent WinError 6
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            startupinfo.wShowWindow = 0 # SW_HIDE
            
            kwargs['startupinfo'] = startupinfo
            kwargs['creationflags'] = subprocess.CREATE_NO_WINDOW
            
        return kwargs

    @staticmethod
    def execute(action):
        if not action or not isinstance(action, dict):
            return
        
        action_type = action.get("type")
        value = action.get("value")
        label = action.get("label", "Unknown")
        run_admin = action.get("runAsAdmin", False)
        run_headless = action.get("runHeadless", False)

        if not value:
            return

        if action_type == "APP":
            ExecutionHandler._launch_app(value, label, run_admin, run_headless)
        elif action_type == "SCRIPT":
            ExecutionHandler._run_script(value, label, run_admin, run_headless)
        elif action_type in ("KEY", "SHORTCUT"):
            pass
        else:
            logger.warning(f"Unknown action type '{action_type}' for '{label}'")

    @staticmethod
    def _launch_app(path, label, run_admin=False, run_headless=False):
        # Strip literal quotes that might have been pasted from Windows
        path = path.strip('"').strip("'")
        logger.info(f"Launching APP [{label}]: {path} (Admin: {run_admin}, Headless: {run_headless})")
        
        try:
            if sys.platform == 'win32':
                if run_admin:
                    import ctypes
                    # show_cmd: 0 = SW_HIDE, 1 = SW_SHOWNORMAL
                    show_cmd = 0 if run_headless else 1
                    ctypes.windll.shell32.ShellExecuteW(None, "runas", path, "", None, show_cmd)
                else:
                    kwargs = ExecutionHandler._get_silent_kwargs() if run_headless else {}
                    subprocess.Popen(path, **kwargs)
            else:
                kwargs = ExecutionHandler._get_silent_kwargs() if run_headless else {}
                subprocess.Popen(shlex.split(path), **kwargs)
        except Exception as e:
            logger.error(f"Failed to launch APP '{path}': {e}")

    @staticmethod
    def _run_script(path, label, run_admin=False, run_headless=False):
        # Strip literal quotes that might have been pasted from Windows
        path = path.strip('"').strip("'")
        logger.info(f"Running SCRIPT [{label}]: {path} (Admin: {run_admin}, Headless: {run_headless})")
        
        if path.startswith("http://") or path.startswith("https://"):
            webbrowser.open(path)
            return

        if not os.path.exists(path):
            logger.error(f"Script path does not exist: {path}")
            return

        ext = os.path.splitext(path)[1].lower()
        cmd = []

        if ext == ".py":
            script_dir = os.path.dirname(path)
            venv_exe = None
            
            for venv_name in ["venv", ".venv", "env", ".env"]:
                if sys.platform == 'win32':
                    possible_exe = os.path.join(script_dir, venv_name, "Scripts", "python.exe")
                else:
                    possible_exe = os.path.join(script_dir, venv_name, "bin", "python3")
                
                if os.path.exists(possible_exe):
                    venv_exe = possible_exe
                    break

            if venv_exe:
                logger.info(f"Detected local venv. Routing through: {venv_exe}")
                cmd = [venv_exe, path]
            else:
                logger.info("No local venv found. Using global python.")
                cmd = ["python", path] if sys.platform == 'win32' else ["python3", path]

        elif ext in (".sh", ".bash"):
            cmd = ["bash", path]
        elif ext in (".bat", ".cmd"):
            cmd = ["cmd.exe", "/c", path]
        elif ext == ".ps1":
            cmd = ["powershell", "-ExecutionPolicy", "Bypass", "-File", path]
        elif ext == ".js":
            cmd = ["node", path]
        elif ext == ".ahk":
            cmd = ["AutoHotkey.exe", path]
        elif ext == ".scpt":
            cmd = ["osascript", path]
        else:
            if sys.platform == 'win32':
                cmd = ["cmd.exe", "/c", "start", '""', path]
            elif sys.platform == 'darwin':
                cmd = ["open", path]
            else:
                cmd = ["xdg-open", path]
        
        try:
            if sys.platform == 'win32':
                if run_admin:
                    import ctypes
                    show_cmd = 0 if run_headless else 1
                    # ShellExecuteW requires the executable and arguments to be split
                    executable = cmd[0]
                    arguments = " ".join(cmd[1:]) if len(cmd) > 1 else ""
                    ctypes.windll.shell32.ShellExecuteW(None, "runas", executable, arguments, None, show_cmd)
                else:
                    kwargs = ExecutionHandler._get_silent_kwargs() if run_headless else {}
                    subprocess.Popen(cmd, **kwargs)
            else:
                kwargs = ExecutionHandler._get_silent_kwargs() if run_headless else {}
                subprocess.Popen(cmd, **kwargs)
        except Exception as e:
            logger.error(f"Failed to run SCRIPT '{path}': {e}")
