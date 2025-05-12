// Package ble provides BLE scanning and connection functionality using TinyGo.
package ble

import (
	"bytes"
	"time"
	"tinygo.org/x/bluetooth"
)

func InitBLE() error{
	return Adapter.Enable()
}

// ScanStart scans for BLE peripherals advertising the specified sensorType and
// establishes connections to them. It filters advertisements based on the sensor's
// service UUID, collects unique devices for up to 5 seconds, and then attempts
// to connect to each discovered device, storing active connections in the global map.
//
// Parameters:
//   sensorType - one of "Battery", "Temperature", or "Gyro" to select the filter UUID.
//
// Returns:
//   error if scanning failed; otherwise nil.
func ScanStart(sensorType string) error {
	scanStartTime := time.Now()

	// Channel for discovered devices, buffered to limit concurrent connections
	foundDevices := make(chan bluetooth.ScanResult, 8)
	uniqueAddress := make(map[bluetooth.Address]bool)

	// Start scanning and collect unique devices advertising the selected UUID
	err := Adapter.Scan(func(a *bluetooth.Adapter, deviceFound bluetooth.ScanResult) {
		// Stop scanning after a 5-second window
		if time.Since(scanStartTime) > 5*time.Second {
			Adapter.StopScan()
			return
		}

		payload := deviceFound.AdvertisementPayload.Bytes()
		if len(payload) >= 21 && bytes.Equal(payload[5:21], SensorUUID[:]) {
			addr := deviceFound.Address
			if !uniqueAddress[addr] {
				select {
				case foundDevices <- deviceFound:
				default:
					// Buffer full: drop extra devices
				}
				uniqueAddress[addr] = true
			}
		}
	})
	if err != nil {
		return err
	}

	// Close channel to signal no more devices
	close(foundDevices)

	// Attempt connections for each discovered device
	for device := range foundDevices {
		addr := device.Address
		if _, exists := conns[addr]; exists {
			continue // skip if already connected
		}

		dev, err := Adapter.Connect(addr, bluetooth.ConnectionParams{})
		if err != nil {
			// Skip devices that fail to connect
			continue
		}

		// Store connected device profile
		conns[addr] = &GATTProfile{
			Device:   dev,
			Active:   true,
			Services: make(map[string]*ServiceClient),
		}
	}

	return nil
}