import time


class BLEDevice:
    def __init__(self, mac):
        self.mac = mac
        self.connected = False
        self.values = []
        # Additional device properties
    
    def update_value(self, value):
        self.values.append(value)
        self.last_value = value
        self.last_update = time.time()

class DeviceManager:
    def __init__(self):
        self.devices = {}
    
    def update_device(self, new_devices):
        for mac, new_device in new_devices.items():
            if mac not in self.devices:
                self.devices[mac] = new_device
            else:
                existing = self.devices[mac]
                existing.connected = new_device.connected
                if hasattr(new_device, 'last_value'):
                    existing.update_value(new_device.last_value)