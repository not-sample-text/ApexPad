import serial
import serial.tools.list_ports
import time
import json
from utils.logger import get_logger
from handlers.execution_handler import ExecutionHandler

logger = get_logger("SerialManager")

class SerialManager:
    def __init__(self, config_manager):
        self.config_manager = config_manager
        self.serial_conn = None
        self.is_running = False
        
        self._wants_upload = False
        self._wants_download = False
        self._force_dump = False

    def auto_connect(self):
        while self.is_running:
            logger.info("Scanning COM ports for ApexPad...")
            ports = serial.tools.list_ports.comports()
            for port in ports:
                try:
                    s = serial.Serial(port.device, 115200, timeout=1)
                    time.sleep(0.1)
                    s.write(b"[PING]\n")
                    time.sleep(0.1)
                    
                    response = s.read_all().decode('utf-8', errors='ignore')
                    if "[PONG:APEXPAD]" in response:
                        logger.info(f"ApexPad established on {port.device}!")
                        self.serial_conn = s
                        return
                    s.close()
                except Exception:
                    pass
            
            time.sleep(3)

    def start_daemon(self):
        self.is_running = True
        
        while self.is_running:
            if not self.serial_conn or not self.serial_conn.is_open:
                self.auto_connect()

            if self._wants_upload:
                self._wants_upload = False
                self._execute_upload()
                continue
                
            if self._wants_download:
                self._wants_download = False
                logger.info("Requesting hardware config...")
                self.serial_conn.write(b"[CFG_READ_REQ]\n")

            try:
                line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue
                
                if line.startswith("[CMD:") and line.endswith("]"):
                    hex_str = line[5:-1]
                    self._handle_cmd(hex_str)
                
                elif "[CFG_READ_START]" in line:
                    self._read_downloaded_config()
                    
                # Catch all other verbose hardware logs and route them to the debug screen
                elif line.startswith("[EVENT]") or line.startswith("[LOG]") or line.startswith("[SYSTEM]"):
                    logger.debug(f"Hardware: {line}")
                    
                else:
                    # Catch any unformatted strings just in case
                    logger.debug(f"Hardware (Raw): {line}")

            except serial.SerialException:
                logger.error("Serial connection lost. Reverting to discovery mode...")
                if self.serial_conn:
                    self.serial_conn.close()
            except Exception as e:
                logger.error(f"Listener loop exception: {e}")

    def stop_daemon(self):
        self.is_running = False
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()

    def _handle_cmd(self, hex_str):
        try:
            packed = int(hex_str, 16)
            layer = packed >> 4
            key_index = (packed & 0x0F) - 1
            col = key_index // 4
            row = key_index % 4
            
            logger.debug(f"Intercepted Command -> Layer: {layer}, Row: {row}, Col: {col}")

            action = self.config_manager.get_action(layer, row, col)
            if action:
                ExecutionHandler.execute(action)
            else:
                logger.warning(f"No valid action mapped for Key C{col}R{row} on Layer {layer}.")
        except ValueError:
            pass

    def trigger_config_upload(self):
        self._wants_upload = True

    def trigger_config_download(self):
        self._wants_download = True
        self._force_dump = False

    def trigger_force_dump(self):
        self._wants_download = True
        self._force_dump = True

    def _execute_upload(self):
        if not self.serial_conn or not self.serial_conn.is_open:
            logger.error("Upload aborted: No active connection.")
            return

        logger.info("Initiating upload protocol...")
        self.serial_conn.write(b"[CFG_WRITE_REQ]\n")
        
        ack_wait_start = time.time()
        ack_received = False
        while time.time() - ack_wait_start < 3:
            line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
            if "[CFG_WRITE_ACK]" in line:
                ack_received = True
                break

        if not ack_received:
            logger.error("Timeout: Hardware failed to ACK.")
            return

        json_str = self.config_manager.get_raw_json_string()
        lines = json_str.splitlines()
        
        # Dynamic delay calculation to ensure we stay well under the 5-second limit
        delay = min(0.01, 4.0 / len(lines)) if lines else 0.01
        
        upload_start = time.time()
        for line in lines:
            if time.time() - upload_start > 5.0:
                logger.error("Upload aborted: Exceeded strict 5-second time limit.")
                self.serial_conn.close()
                return

            self.serial_conn.write((line + "\n").encode('utf-8'))
            time.sleep(delay) 
            
        self.serial_conn.write(b"\n[CFG_WRITE_EOF]\n")

        logger.info("Payload sent. Awaiting hardware reboot...")
        
        self.serial_conn.close()

    def _read_downloaded_config(self):
        logger.info("Hardware initiated config recovery stream...")
        buffer = []
        timeout_start = time.time()
        max_lines = 1000 # Memory protection
        
        while time.time() - timeout_start < 5 and len(buffer) < max_lines:
            line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
            if "[CFG_READ_END]" in line:
                break
            if line:
                buffer.append(line)
        
        if not buffer:
            logger.warning("Hardware returned an empty configuration.")
            return
        
        json_str = "\n".join(buffer)

        if self._force_dump:
            logger.info("Emergency Dump flag active. Bypassing schema validation...")
            self.config_manager.dump_config_to_desktop(json_str)
            self._force_dump = False
            return

        try:
            parsed = json.loads(json_str)
            if "layers" not in parsed or not isinstance(parsed["layers"], list):
                logger.error("Hardware provided invalid config schema. Rejecting.")
                return
            
            self.config_manager.save_recovered_config(json_str)
        except json.JSONDecodeError as e:
            logger.error("Hardware provided malformed JSON. Rejecting.")
            logger.error(f"Parse Error Details: {e.msg} at Line {e.lineno}, Column {e.colno}")
            
            lines = json_str.splitlines()
            if 0 <= e.lineno - 1 < len(lines):
                problem_line = lines[e.lineno - 1]
                logger.error(f"Problematic line: {problem_line.strip()}")
