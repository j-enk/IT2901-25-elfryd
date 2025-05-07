package ble

import (
	"bytes"
	"fmt"
	"time"

	"tinygo.org/x/bluetooth"
)

func InitBLE() error{
	return Adapter.Enable()
}

func ScanStart(sensorType string) error{

	scanStartTime := time.Now()

	foundDevices := make(chan bluetooth.ScanResult, 8)
	uniqueAddress := make(map[bluetooth.Address]bool)
	var uuidFilter [16]byte
	switch sensorType {
	case "Battery":
		uuidFilter = voltageUUID
	case "Temperature":
		uuidFilter = tempUUID
	case "Gyro":
		uuidFilter = gyroUUID
	}
	err := Adapter.Scan(func(a *bluetooth.Adapter, device_found bluetooth.ScanResult){
		//Får ikke skanne etter at den har connected?
		if time.Since(scanStartTime) > 5*time.Second {
			Adapter.StopScan()
			return
		}
		payload:= device_found.AdvertisementPayload.Bytes()
		if bytes.Equal(payload[5:21], uuidFilter[:]){
			if !uniqueAddress[device_found.Address]{
				select {
				case foundDevices <- device_found:
				default:
					fmt.Println("Channel full, no more connections allowed")
				}
				uniqueAddress[device_found.Address] = true
			}
		}
		//nothing
	})
	if err != nil{
		return err
	}
	close(foundDevices)

	for device:=range foundDevices{
		addr:=device.Address
		if _, exists := conns[addr]; exists{
			continue
		}
		dev, err := Adapter.Connect(addr, bluetooth.ConnectionParams{})
		if err != nil{
			fmt.Printf("[ERROR] Connect failed for %s: %v\n", addr.String(), err)
			continue
		}
		conns[addr] = &GATTProfile{
			Device: 	dev,
			Active: 	true,	
			Address:  addr,
			// ID:			0,
			Services: 	make(map[string]*ServiceClient),
		}
	}
	fmt.Printf("Devices connected: %d\n", len(conns))
	return nil
}