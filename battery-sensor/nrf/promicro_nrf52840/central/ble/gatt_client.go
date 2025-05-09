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
	devicesConnected = 0
	sensorType       = "null"
)

// InitGATT initializes GATT profiles for all connected devices.
func InitGATT(sensorTypeMain string) error {
	if sensorTypeMain == "null" {
		return fmt.Errorf("no sensor type selected")
	}
	sensorType = sensorTypeMain
	fmt.Println("InitGATT: Sensor type set to", sensorType)

	buf := make([]byte, 1)

	for addr, dev := range conns {
		fmt.Printf("InitGATT: Discovering services for device %s\n", addr.String())

		if err := findSrvcChars(dev); err != nil {
			fmt.Printf("InitGATT: Failed to find services for %s: %v\n", addr.String(), err)
			dev.Active = false
			continue
		}

		m, err := dev.Services["ID"].Chars[idUUID].Read(buf[:1])
		if err != nil || m != 1 {
			fmt.Printf("InitGATT: Failed to read ID from %s: %v (read %d bytes)\n", addr.String(), err, m)
			dev.Active = false
			continue
		}
		addrIDArray[addr] = int8(buf[0])
		fmt.Printf("InitGATT: Assigned ID %d to device %s\n", buf[0], addr.String())
	}

	devicesConnected = len(conns)
	fmt.Printf("InitGATT: %d devices connected\n", devicesConnected)
	return nil
}

// RunGATTClient performs periodic sensor reads
func RunGATTClient() error {
	if devicesConnected == 0 {
		fmt.Println("RunGATTClient: No devices connected, exiting")
		return nil
	}

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

	for range ticker.C {
		for addr, dev := range conns {
			if !dev.Active {
				continue
			}
			fmt.Printf("RunGATTClient: Reading %s from %s\n", sensorType, addr.String())

			n, err := dev.Services[sensorType].Chars[readUUID].Read(buf)
			if err != nil {
				fmt.Printf("RunGATTClient: Read error from %s: %v\n", addr.String(), err)
				continue
			}

			if n != nBytes {
				fmt.Printf("RunGATTClient: Warning - expected %d bytes, got %d bytes from %s\n", nBytes, n, addr.String())
			}

			msg := BatteryMessage{
				New:     1,
				ID:      int8(addrIDArray[addr]),
				Payload: buf[:nBytes],
			}
			fmt.Printf("RunGATTClient: Received data from %s: ID=%d, Payload=%x\n", addr.String(), msg.ID, msg.Payload)
			SetBatteryEntry(addr, msg)
		}
	}
	return nil
}

// findSrvcChars discovers required services/characteristics
func findSrvcChars(profile *GATTProfile) error {
	fmt.Println("findSrvcChars: Starting service discovery")

	ids := []bluetooth.UUID{idUUID}
	srvcs, err := profile.Device.DiscoverServices(ids)
	if err != nil || len(srvcs) == 0 {
		fmt.Println("findSrvcChars: ID service not found")
		profile.Active = false
		return fmt.Errorf("ID service discovery failed: %w", err)
	}
	fmt.Println("findSrvcChars: ID service found")

	profile.Services["ID"] = &ServiceClient{
		UUID:  idUUID,
		Chars: make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	chars, err := srvcs[0].DiscoverCharacteristics(ids)
	if err != nil || len(chars) == 0 {
		fmt.Println("findSrvcChars: ID characteristics not found")
		return fmt.Errorf("ID characteristic discovery failed: %w", err)
	}
	for _, c := range chars {
		profile.Services["ID"].Chars[c.UUID()] = c
	}
	fmt.Println("findSrvcChars: ID characteristics discovered")

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
		fmt.Printf("findSrvcChars: Sensor service (%s) not found\n", sensorType)
		profile.Active = false
		return fmt.Errorf("sensor service discovery failed: %w", err)
	}
	fmt.Printf("findSrvcChars: Sensor service (%s) found\n", sensorType)

	profile.Services[sensorType] = &ServiceClient{
		UUID:  svcUUID,
		Chars: make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	chars, err = srvcs[0].DiscoverCharacteristics([]bluetooth.UUID{svcUUID})
	if err != nil || len(chars) == 0 {
		fmt.Printf("findSrvcChars: Sensor characteristics (%s) not found\n", sensorType)
		return fmt.Errorf("sensor characteristic discovery failed: %w", err)
	}
	for _, c := range chars {
		profile.Services[sensorType].Chars[c.UUID()] = c
	}
	fmt.Printf("findSrvcChars: Sensor characteristics (%s) discovered\n", sensorType)

	return nil
}
