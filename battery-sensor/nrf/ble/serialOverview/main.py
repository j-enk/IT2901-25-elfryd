import curses
from ui.dashboard import Dashboard
from utils.serial_reader import SerialReader
from models.device import DeviceManager
from parsers.ble_parser import BLEParser

def main(stdscr):
    curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_RED, curses.COLOR_BLACK)

    device_manager = DeviceManager()
    serial_reader = SerialReader(device_manager)
    dashboard = Dashboard(device_manager)
    parser = BLEParser()
    
    try:
        while True:
            line = serial_reader.process_data()
            print(f"Serial: {line}")  # DEBUG
            if line:
                new_data = parser.parse_line(line)
                print(f"Parsed: {new_data}")  # DEBUG
                if new_data:
                    device_manager.update_device(new_data)
                    dashboard.update(stdscr)
    except KeyboardInterrupt:
        dashboard.cleanup()

if __name__ == "__main__":
    curses.wrapper(main)