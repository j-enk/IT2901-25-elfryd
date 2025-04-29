package ble

import (
	"encoding/binary"
	"fmt"
	"time"

	"tinygo.org/x/bluetooth"
)

var devices_connected = 0

func InitGATT() error {
	fmt.Println("[InitGATT] Initializing GATT profiles...")
	for addr, dev := range conns {
		fmt.Printf("[InitGATT] Discovering services for device: %s\n", addr.String())
		err := findSrvcChars(dev)
		if err != nil {
			fmt.Printf("[InitGATT] Failed to find services for %s: %v\n", addr.String(), err)
			return err
		}
	}
	devices_connected = len(conns)
	fmt.Printf("[InitGATT] Devices connected: %d\n", devices_connected)
	return nil
}

func RunGATTClient() error {
	fmt.Println("[RunGATTClient] Starting GATT client...")
	ticker := time.NewTicker(time.Second)
	defer ticker.Stop()

	if devices_connected == 0 {
		fmt.Println("[RunGATTClient] No connected devices. Exiting.")
		return nil
	}

	for tick := range ticker.C {
		_ = tick
		for addr, dev := range conns {
		if dev.Active {
			fmt.Printf("[RunGATTClient] Reading characteristic from device %s...\n", addr.String())

			buf := make([]byte, 4)
			n, err := dev.Services["Battery"].Chars[voltageUUID].Read(buf[:4])
			if err != nil {
				fmt.Printf("[RunGATTClient] Error reading from device %s: %v\n", addr.String(), err)
				return err
			}

			if n != 4 {
				fmt.Printf("[RunGATTClient] Warning: expected 4 bytes, got %d bytes\n", n)
			}

			val := int32(binary.LittleEndian.Uint32(buf[:n]))
			fmt.Printf("[RunGATTClient] Device %s voltage value = %d\n", addr.String(), val)
			SetBatteryEntry(addr,&BatteryMessage{
				New: 1,
				ID: 1,
				Payload: val
			})
		} else {
			continue
		}
		}
	}
	return nil
}

func findSrvcChars(profile *GATTProfile) error {
	fmt.Println("[findSrvcChars] Discovering battery service...")

	batteryUUID := []bluetooth.UUID{voltageUUID}
	srvcs, err := profile.Device.DiscoverServices(batteryUUID)
	if err != nil {
		fmt.Printf("[findSrvcChars] Failed to discover services: %v\n", err)
		profile.Active = false
		return nil
	}
	if len(srvcs) == 0 {
		fmt.Println("[findSrvcChars] No services found!")
		profile.Active = false
		return nil
	}
	fmt.Printf("[findSrvcChars] Found %d service(s)\n", len(srvcs))

	profile.Services["Battery"] = &ServiceClient{
		UUID:  voltageUUID,
		Chars: make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	chars, err := srvcs[0].DiscoverCharacteristics(batteryUUID)
	if err != nil {
		fmt.Printf("[findSrvcChars] Failed to discover characteristics: %v\n", err)
		return err
	}
	if len(chars) == 0 {
		fmt.Println("[findSrvcChars] No characteristics found!")
		return fmt.Errorf("no characteristics found")
	}

	for _, char := range chars {
		fmt.Printf("[findSrvcChars] Found characteristic: %s\n", char.UUID().String())
		profile.Services["Battery"].Chars[char.UUID()] = char
	}

	return nil
}