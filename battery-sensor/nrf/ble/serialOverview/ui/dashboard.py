import curses
import time

class Dashboard:
    def __init__(self, device_manager):
        self.device_manager = device_manager
        
    def update(self,stdscr):
        devices = self.device_manager.devices
        stdscr.clear()
        h, w = stdscr.getmaxyx()
        
        # Header
        stdscr.addstr(0, 0, "BLE Device Dashboard".center(w), curses.A_BOLD)
        stdscr.addstr(1, 0, "-"*w)
        
        # Device sections
        row = 2
        if not devices:
            stdscr.addstr(row, 2, "No devices found yet...")
            stdscr.refresh()
            return
        for i, (mac, device) in enumerate(devices.items()):
            status = "CONNECTED" if device.connected else "PENDING"
            color = curses.color_pair(1) if device.connected else curses.color_pair(2)
            
            # Device header
            stdscr.addstr(row, 0, f"Device {i+1}: {mac}", color)
            stdscr.addstr(row, 30, f"[{status}]", color)
            row += 1
            
            # Value display
            if device.connected:
                if device.last_value is not None:
                    stdscr.addstr(row, 2, f"Current Value: {device.last_value}")
                    stdscr.addstr(row+1, 2, f"Updated: {time.ctime(device.last_update)[11:19]}")
                else:
                    stdscr.addstr(row, 2, "Waiting for first reading...")
                row += 2
            else:
                stdscr.addstr(row, 2, "Not connected - scanning...")
                row += 1
            
            stdscr.addstr(row, 0, "-"*w)
            row += 1
        
        stdscr.refresh()
        
    def cleanup(self):
        curses.endwin()