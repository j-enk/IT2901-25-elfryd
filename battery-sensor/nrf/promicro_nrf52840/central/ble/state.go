package ble

import (
	"errors"
	"sync"
	"tinygo.org/x/bluetooth"
)

var(
	Adapter 	= bluetooth.DefaultAdapter
	conns		= make(map[bluetooth.Address]*GATTProfile)
	mu = sync.Mutex{}
	idUUID = 		bluetooth.NewUUID([16]byte{
						0x00, 0x00, 0x2C, 0x05,
						0x00, 0x00,
						0x10, 0x00,
						0x80, 0x00,
						0x00, 0x80,
						0x5F, 0x9B, 0x34, 0xFB,
					})
	SensorUUID [16]byte
	GattReadSize int
	I2CReplyLen int
	BatteryArray = 	make(map[bluetooth.Address]BatteryMessage)
	addrIDArray = 	make(map[bluetooth.Address]int8)
)

type GATTProfile struct{
	Device			bluetooth.Device
	Active			bool
	// ID				int8 //flyttet til addrIDArray
	Services		map[string]*ServiceClient
}

type ServiceClient struct{
	UUID	bluetooth.UUID
	Chars	map[bluetooth.UUID]bluetooth.DeviceCharacteristic
}

type BatteryMessage struct{
	New			int8	//Flag for if data has been updated since last read (0: old, 1: new)
	ID			int8
	Payload 	[]byte
}

func InitState(sensorType string) error{
	switch sensorType {
	case "Battery":
		SensorUUID = [16]byte{
			0x00, 0x00, 0x2B, 0x18,
			0x00, 0x00,
			0x10, 0x00,
			0x80, 0x00,
			0x00, 0x80,
			0x5F, 0x9B, 0x34, 0xFB,
		}
		GattReadSize = 2
		I2CReplyLen = 4
	case "Temperature":
		SensorUUID = [16]byte{
			0x00, 0x00, 0x2A, 0x6E,
			0x00, 0x00,
			0x10, 0x00,
			0x80, 0x00,
			0x00, 0x80,
			0x5F, 0x9B, 0x34, 0xFB,
		}
		GattReadSize = 2
		I2CReplyLen = 4 //Ikke implemetert id i backend
	case "Gyro":
		SensorUUID = [16]byte{
			0x00, 0x00, 0x2F, 0x01,
			0x00, 0x00,
			0x10, 0x00,
			0x80, 0x00,
			0x00, 0x80,
			0x5F, 0x9B, 0x34, 0xFB,
		}
		GattReadSize = 18
		I2CReplyLen = 20 //Ikke implemetert id i backend
	default:
		return errors.New("invalid sensor type")
	}
	return nil
}

// GetBatteryArray safely returns a copy of BatteryArray
func GetBatteryArray() map[bluetooth.Address]BatteryMessage{
	mu.Lock()
	defer mu.Unlock()

	// Return a *copy* to avoid race conditions if caller modifies it
	copy := make(map[bluetooth.Address]BatteryMessage, len(BatteryArray))
	for addr, msg := range BatteryArray {
		copy[addr] = msg
	}
	return copy
}

// SetBatteryEntry safely sets or updates a BatteryMessage for a device
func SetBatteryEntry(addr bluetooth.Address, msg BatteryMessage) {
    mu.Lock()
    defer mu.Unlock()

    // Create a copy of the Payload slice to avoid shared references
    payloadCopy := make([]byte, len(msg.Payload))
    copy(payloadCopy, msg.Payload)
    msg.Payload = payloadCopy

    BatteryArray[addr] = msg
}