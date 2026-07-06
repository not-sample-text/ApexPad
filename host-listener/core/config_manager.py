import os
import json
import pathlib
import shutil
import tkinter as tk
from tkinter import filedialog
from utils.logger import get_logger

logger = get_logger("ConfigManager")

class ConfigManager:
    def __init__(self):
        self.config_data = None
        self.local_config_path = os.path.join(os.getcwd(), "config.json")

    def locate_and_load(self):
        local_exists = os.path.exists(self.local_config_path)

        if local_exists:
            self._load_json(self.local_config_path)
        else:
            logger.warning("No local config.json found. Waiting for user action...")
            self.config_data = {}

    def _backup_local(self):
        """Helper method to rename the existing local config to config_old.json"""
        if os.path.exists(self.local_config_path):
            old_path = os.path.join(os.getcwd(), "config_old.json")
            try:
                if os.path.exists(old_path):
                    os.remove(old_path)
                os.rename(self.local_config_path, old_path)
                logger.info("Backed up previous local config to config_old.json")
            except Exception as e:
                logger.error(f"Failed to backup old config: {e}")

    def pick_and_copy_config(self):
        """Used for the Upload Button: Picks file, checks paths, copies locally, reloads."""
        root = tk.Tk()
        root.withdraw()
        
        # Ensures the dialog stays on top of other windows
        root.attributes('-topmost', True) 
        
        file_path = filedialog.askopenfilename(
            title="ApexPad: Select Config to Upload",
            filetypes=[("JSON Files", "*.json")]
        )
        root.destroy()

        if file_path and os.path.exists(file_path):
            try:
                if os.path.abspath(file_path) == os.path.abspath(self.local_config_path):
                    logger.info("Selected local config directly. Skipping copy.")
                else:
                    self._backup_local()
                    shutil.copy2(file_path, self.local_config_path)
                    logger.info(f"Copied {file_path} to local directory as master.")
                
                self._load_json(self.local_config_path)
                return True
            except Exception as e:
                logger.error(f"Failed to copy config: {e}")
                return False
        return False

    def save_recovered_config(self, json_str):
        """Used for the Download Button: Backs up local, saves new locally & in Downloads."""
        self._backup_local()

        # 1. Save new locally (This becomes the highest priority master)
        try:
            with open(self.local_config_path, 'w', encoding='utf-8') as f:
                f.write(json_str)
            logger.info("Saved recovered config to local directory.")
        except Exception as e:
            logger.error(f"Failed to write local config: {e}")

        # 2. Save a duplicate to OS Downloads folder
        downloads_path = os.path.join(pathlib.Path.home(), "Downloads", "config.json")
        try:
            with open(downloads_path, 'w', encoding='utf-8') as f:
                f.write(json_str)
            logger.info(f"Saved duplicate to: {downloads_path}")
        except Exception as e:
            logger.error(f"Failed to write to Downloads: {e}")

        # 3. Reload into memory
        self._load_json(self.local_config_path)

    def _load_json(self, path):
        try:
            with open(path, 'r', encoding='utf-8') as f:
                self.config_data = json.load(f)
            logger.info("Configuration successfully loaded into memory.")
        except Exception as e:
            logger.error(f"Failed to parse JSON: {e}")

    def get_raw_json_string(self):
        if not os.path.exists(self.local_config_path):
            return "{}"
        with open(self.local_config_path, 'r', encoding='utf-8') as f:
            return f.read()

    def get_action(self, layer_id, row, col):
        if not self.config_data or "layers" not in self.config_data:
            return None
        key_id = f"C{col}R{row}"
        for layer in self.config_data["layers"]:
            if layer.get("id") == layer_id:
                keys = layer.get("keys", {})
                return keys.get(key_id)
        return None
