package ble

import (
	"tinygo.org/x/bluetooth"
	"sync"
)

var(
	Adapter 	= bluetooth.DefaultAdapter
	conns		= make(map[bluetooth.Address]*GATTProfile)
	mu = sync.Mutex{}
	uuidFilter	= 	[16]byte{
						0xCD, 0xEE, 0x3D, 0x67,
						0x35, 0xCD, 0x3A, 0x94,
						0x1D, 0x45, 0xBD, 0xB7,
						0x5E, 0x67, 0x70, 0xBF,
					}
	voltageUUID	=	bluetooth.NewUUID([16]byte{
						0x00, 0x00, 0x2B, 0x18,
						0x00, 0x00,
						0x10, 0x00,
						0x80, 0x00,
						0x00, 0x80,
						0x5F, 0x9B, 0x34, 0xFB,
					})
	ScanStop =		false
	BatteryArray = 	make(map[bluetooth.Address]BatteryMessage)
	MessageBus = make(chan Message, 4)
	sensorUUID = [16]byte{
		0x69, 0x8b, 0x5a, 0x33,
		0xec, 0x96,
		0x40, 0x9a,
		0xa2, 0x70,
		0xe7, 0x43, 0xa9, 0x73, 0x37, 0x5b,
	}
)

type GATTProfile struct{
	Device			bluetooth.Device
	Active			bool
	// ID				int8
	Services		map[string]*ServiceClient
}

type ServiceClient struct{
	UUID	bluetooth.UUID
	Chars	map[bluetooth.UUID]bluetooth.DeviceCharacteristic
}

type BatteryMessage struct{
	New			int8	//Flag for if data has been updated since last read (0: old, 1: new)
	ID			int8
	Payload 	int32
}

// GetBatteryArray safely returns a copy of BatteryArray
func GetBatteryArray() map[bluetooth.Address]BatteryMessage {
	mu.Lock()
	defer mu.Unlock()

	// Return a *copy* to avoid race conditions if caller modifies it
	copy := make(map[bluetooth.Address]BatteryMessage, len(batteryArray))
	for addr, msg := range batteryArray {
		copy[addr] = msg
	}
	return copy
}

// SetBatteryEntry safely sets or updates a BatteryMessage for a device
func SetBatteryEntry(addr bluetooth.Address, msg BatteryMessage) {
	mu.Lock()
	defer mu.Unlock()

	batteryArray[addr] = msg
}