import serial

class SerialReader:
    def __init__(self, device_manager, port='/dev/ttyACM0', baud=115200):
        self.ser = serial.Serial(port, baud)
        self.device_manager = device_manager
        
    def process_data(self):
        return self.ser.readline().decode('utf-8').strip()