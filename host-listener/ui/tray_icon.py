import pystray
from pystray import MenuItem as item
from PIL import Image, ImageDraw
from ui.debug_window import DebugWindow
from utils.logger import get_logger

logger = get_logger("TrayIcon")

class ApexTrayIcon:
    def __init__(self, serial_mgr, config_mgr):
        self.serial_mgr = serial_mgr
        self.config_mgr = config_mgr
        self.debug_window = DebugWindow()
        self.icon = None

    def _create_image(self):
        # Generates a dynamic 64x64 blue icon with a white 'A'
        image = Image.new('RGB', (64, 64), color=(30, 144, 255))
        dc = ImageDraw.Draw(image)
        dc.rectangle(
            (8, 8, 56, 56), 
            outline=(255, 255, 255), 
            width=4
        )
        return image

    def _on_upload(self, icon, item):
        success = self.config_mgr.pick_and_copy_config()
        if success:
            self.serial_mgr.trigger_config_upload()

    def _on_download(self, icon, item):
        self.serial_mgr.trigger_config_download()

    def _on_force_dump(self, icon, item):
        logger.warning("Triggering raw emergency configuration dump...")
        self.serial_mgr.trigger_force_dump()

    def _on_debug(self, icon, item):
        self.debug_window.show()

    def _on_exit(self, icon, item):
        logger.info("Shutting down daemon...")
        self.serial_mgr.stop_daemon()
        self.icon.stop()

    def run(self):
        menu = pystray.Menu(
            item('ApexPad Daemon', None, enabled=False),
            pystray.Menu.SEPARATOR,
            item('Upload Config to Pad', self._on_upload),
            item('Recover Config from Pad', self._on_download),
            item('Force Dump to Desktop', self._on_force_dump),
            pystray.Menu.SEPARATOR,
            item('Show Debug Console', self._on_debug),
            item('Exit', self._on_exit)
        )
        
        self.icon = pystray.Icon("ApexPad", self._create_image(), "ApexPad Server", menu)
        # This call blocks the main thread, keeping the Python app alive
        self.icon.run()
