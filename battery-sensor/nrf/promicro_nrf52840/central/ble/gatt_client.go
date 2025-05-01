package ble

import (
	"fmt"
	"time"

	"tinygo.org/x/bluetooth"
)

var (
	devices_connected = 0
	sensorType = "null"
)


func InitGATT(sensorTypeMain string) error {
	if sensorTypeMain == "null"{
		fmt.Println("No sensor type choosen")
		devices_connected = 0
		return nil
	}
	sensorType = sensorTypeMain
	fmt.Println("[InitGATT] Initializing GATT profiles...")
	for addr, dev := range conns {
		fmt.Printf("[InitGATT] Discovering services for device: %s\n", addr.String())
		findSrvcChars(dev)		
	}
	devices_connected = len(conns)
	fmt.Printf("[InitGATT] Devices connected: %d\n", devices_connected)
	// Future work; add validation so IDs only corresponds to one address
	// for key, value := range addrIDArray{
	// fmt.Printf("[InitGATT] Addr and IDs: %s %d\n",key.String(), value)
	// }
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
	var readUUID bluetooth.UUID
	var buf []byte
	var bytes int
	switch sensorType {
	case "Battery":
		readUUID = bluetooth.NewUUID(voltageUUID)
		bytes = 2
		buf = make([]byte, bytes)
	case "Temperature":
		readUUID = bluetooth.NewUUID(tempUUID)
		bytes = 4
		buf = make([]byte, bytes)

	case "Gyro":
		readUUID = bluetooth.NewUUID(gyroUUID)
		bytes = 18
		buf = make([]byte, bytes)
	}

	for tick := range ticker.C {
		_ = tick
		for addr, dev := range conns {
		if dev.Active {
			fmt.Printf("[RunGATTClient] Reading characteristic from device %s...\n", addr.String())
			
			id := IDFor(addr)
			n, err := dev.Services[sensorType].Chars[readUUID].Read(buf[:bytes])
			if err != nil {
				fmt.Printf("[RunGATTClient] Error reading from device %s: %v\n", addr.String(), err)
				return err
			}

			if n != bytes {
				fmt.Printf("[RunGATTClient] Warning: expected %d bytes, got %d bytes\n",bytes , n)
			}
			fmt.Printf("[RunGATTClient] Device: %s Sensor: %s\n", addr.String(), sensorType)
			SetBatteryEntry(addr,BatteryMessage{
				New: 1,
				ID: id,
				Payload: buf,
			})
		} else {
			continue
		}
		}
	}
	return nil
}

func findSrvcChars(profile *GATTProfile) error {

	fmt.Println("[findSrvcChars] Discovering ID service")
	idsrvc := []bluetooth.UUID{idUUID}
	srvcs, err := profile.Device.DiscoverServices(idsrvc)
	if err != nil {
		profile.Active = false
		return nil
	}
	if len(srvcs) == 0 {
		fmt.Println("[findSrvcChars] No services found!")
		profile.Active = false
		return nil
	}

	profile.Services["ID"] = &ServiceClient{
		UUID: idUUID,
		Chars: make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	chars, err := srvcs[0].DiscoverCharacteristics(idsrvc)
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
		profile.Services["ID"].Chars[char.UUID()] = char
	}

	fmt.Println("[findSrvcChars] Discovering battery service...")
	var gattUUID bluetooth.UUID
	switch sensorType{
	case "Battery":
		gattUUID = bluetooth.NewUUID(batteryfind(profile.Address))
	
	case "Temperature":
		gattUUID = bluetooth.NewUUID(tempUUID)

	case "Gyro":
		gattUUID = bluetooth.NewUUID(gyroUUID)
	}
	srvcUUID := []bluetooth.UUID{gattUUID}
	srvcs, err = profile.Device.DiscoverServices(srvcUUID)
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
	

	profile.Services[sensorType] = &ServiceClient{
		UUID:  gattUUID,
		Chars: make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	chars, err = srvcs[0].DiscoverCharacteristics(srvcUUID)
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
		profile.Services[sensorType].Chars[char.UUID()] = char
	}

	return nil
}

func batteryfind(addr bluetooth.Address) [16]byte {
	switch addr.String() {
    case "D9:A8:EC:EA:72:6B":
        return enUUID
    case "EC:0A:B5:04:71:7B":
        return toUUID
    case "E9:46:77:D0:E3:05":
        return trerUUID
    case "CA:6A:4C:BD:7C:36":
        return fireUUID
    default:
		fmt.Println("fant ikke ajdpiajwdijawipdjaipwd")
		return [16]byte{}
    }
}



func IDFor(addr bluetooth.Address) int8 {
    switch addr.String() {
    case "D9:A8:EC:EA:72:6B":
        return 3
    case "EC:0A:B5:04:71:7B":
        return 2
    case "E9:46:77:D0:E3:05":
        return 4
    case "CA:6A:4C:BD:7C:36":
        return 1
    default:
		fmt.Println("fant ikke ajdpiajwdijawipdjaipwd")
        return 0
    }
}