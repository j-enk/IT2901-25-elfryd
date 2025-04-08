import re
from models.device import BLEDevice

class BLEParser:
    CONN_PATTERN = r"Connected: ([0-9A-Fa-f:]+)"
    READ_PATTERN = r"int32_t: (-?\d+)"
    
    @classmethod
    def parse_line(line):
        # Connection pattern
        conn_pattern = r"Connected: ([0-9A-Fa-f:]+) \(random\)"
        # GATT read pattern
        read_pattern = r"\[GATT READ SUCCESS\] Device: ([0-9A-Fa-f:]+).*?int32_t: (-?\d+)"
        
        devices = {}
        
        # Check for connection events
        conn_match = re.search(conn_pattern, line)
        if conn_match:
            mac = conn_match.group(1)
            devices[mac] = BLEDevice(mac)
            devices[mac].connected = True
            return devices
        
        # Check for GATT reads
        read_match = re.search(read_pattern, line, re.DOTALL)
        if read_match:
            mac = read_match.group(1)
            value = int(read_match.group(2))
            if mac not in devices:
                devices[mac] = BLEDevice(mac)
            devices[mac].update_value(value)
            return devices
        
        return None