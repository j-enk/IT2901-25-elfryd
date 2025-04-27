package ble

import (
	"tinygo.org/x/bluetooth"
)

var(
	Adapter 	= bluetooth.DefaultAdapter
	conns		= make(map[bluetooth.Address]*GATTProfile)
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
	//Not implemented yet
	// idUUID		=	
)

type GATTProfile struct{
	Device			bluetooth.Device
	// ID				int8
	Services		map[string]*ServiceClient
}

type ServiceClient struct{
	UUID	bluetooth.UUID
	Chars	map[bluetooth.UUID]bluetooth.DeviceCharacteristic
}

type Message struct{
	ID			int8
	Payload 	int32
}