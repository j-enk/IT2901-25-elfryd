// Package ble provides functionality for scanning, connecting, and reading
// data from BLE devices using TinyGo's Bluetooth API. It supports Battery,
// Temperature, and Gyro sensors with service discovery and characteristic reads.
package ble

import (
	"fmt"
	"time"

	"tinygo.org/x/bluetooth"
)

var (
	// devicesConnected holds the current count of active connected devices.
	devicesConnected = 0

	// sensorType indicates which type of sensor is active: "Battery", "Temperature", or "Gyro".
	sensorType = "null"
)

// InitGATT initializes GATT profiles for all connected devices.
// It discovers the ID service and reads a one-byte identifier from each device.
// sensorTypeMain must be one of "Battery", "Temperature", or "Gyro".
func InitGATT(sensorTypeMain string) error {
	// Validate sensor type selection
	if sensorTypeMain == "null" {
		return fmt.Errorf("no sensor type selected")
	}
	sensorType = sensorTypeMain

	// Buffer for reading the ID characteristic
	buf := make([]byte, 1)

	// Iterate over all connected devices and discover their services
	for addr, dev := range conns {
		// Discover ID service and characteristics
		if err := findSrvcChars(dev); err != nil {
			dev.Active = false
			continue
		}

		// Read one byte from the ID characteristic
		m, err := dev.Services["ID"].Chars[idUUID].Read(buf[:1])
		if err != nil || m != 1 {
			dev.Active = false
			continue
		}

		// Store the read identifier in the address-to-ID map
		addrIDArray[addr] = int8(buf[0])
	}

	// Update global count of connected devices
	devicesConnected = len(conns)
	return nil
}

// RunGATTClient starts periodic reads from the selected sensor characteristic
// on all active devices. It schedules reads every 1 second and pushes results
// to the message queue for further processing.
func RunGATTClient() error {
	// Exit early if no devices are connected
	if devicesConnected == 0 {
		return nil
	}

	// Prepare UUID and buffer length based on sensor type
	var (
		readUUID bluetooth.UUID
		nBytes   int
	)
	switch sensorType {
	case "Battery":
		readUUID = bluetooth.NewUUID(voltageUUID)
		nBytes = 2
	case "Temperature":
		readUUID = bluetooth.NewUUID(tempUUID)
		nBytes = 2
	case "Gyro":
		readUUID = bluetooth.NewUUID(gyroUUID)
		nBytes = 18
	default:
		return fmt.Errorf("unknown sensor type: %s", sensorType)
	}

	buf := make([]byte, nBytes)
		ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	// Periodic read loop
	for range ticker.C {
		for addr, dev := range conns {
			if !dev.Active {
				continue
			}

			// Perform characteristic read
			n, err := dev.Services[sensorType].Chars[readUUID].Read(buf)
			if err != nil {
				return fmt.Errorf("read error from %s: %w", addr.String(), err)
			}

			// Validate byte count
			if n != nBytes {
				fmt.Printf("warning: expected %d bytes, got %d bytes\n", nBytes, n)
			}

			// Construct and send the sensor message
			msg := BatteryMessage{
				New:     1,
				ID:      int8(addrIDArray[addr]),
				Payload: buf[:nBytes],
			}
			SetBatteryEntry(addr, msg)
		}
	}
	return nil
}

// findSrvcChars discovers the ID service and characteristics, as well as
// the sensor-specific service and characteristics for the given profile.
// It populates profile.Services with ServiceClient entries for "ID" and
// sensorType, each holding a map of characteristic UUID to DeviceCharacteristic.
func findSrvcChars(profile *GATTProfile) error {
	// Discover the ID service
	ids := []bluetooth.UUID{idUUID}
	srvcs, err := profile.Device.DiscoverServices(ids)
	if err != nil || len(srvcs) == 0 {
		profile.Active = false
		return fmt.Errorf("ID service discovery failed: %w", err)
	}

	// Initialize the ServiceClient for ID
	profile.Services["ID"] = &ServiceClient{
		UUID:  idUUID,
		Chars: make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	// Discover ID characteristics and populate map
	chars, err := srvcs[0].DiscoverCharacteristics(ids)
	if err != nil || len(chars) == 0 {
		return fmt.Errorf("ID characteristic discovery failed: %w", err)
	}
	for _, c := range chars {
		profile.Services["ID"].Chars[c.UUID()] = c
	}

	// Discover the sensor service based on global sensorType
	var svcUUID bluetooth.UUID
	switch sensorType {
	case "Battery":
		svcUUID = bluetooth.NewUUID(voltageUUID)
	case "Temperature":
		svcUUID = bluetooth.NewUUID(tempUUID)
	case "Gyro":
		svcUUID = bluetooth.NewUUID(gyroUUID)
	}

	srvcs, err = profile.Device.DiscoverServices([]bluetooth.UUID{svcUUID})
	if err != nil || len(srvcs) == 0 {
		profile.Active = false
		return fmt.Errorf("sensor service discovery failed: %w", err)
	}

	// Initialize and populate the ServiceClient for the sensor
	profile.Services[sensorType] = &ServiceClient{
		UUID:  svcUUID,
		Chars: make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	chars, err = srvcs[0].DiscoverCharacteristics([]bluetooth.UUID{svcUUID})
	if err != nil || len(chars) == 0 {
		return fmt.Errorf("sensor characteristic discovery failed: %w", err)
	}
	for _, c := range chars {
		profile.Services[sensorType].Chars[c.UUID()] = c
	}

	return nil
}
