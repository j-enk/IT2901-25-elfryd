package ble

import (
	"encoding/binary"
	"fmt"
	"time"

	"tinygo.org/x/bluetooth"
) 

func InitGATT() error{
	for _, dev := range conns { // Use 'range' to iterate over the slice
		return findSrvcChars(dev)
	}
	return nil
}

func RunGATTClient() error{
	ticker:= time.NewTicker(time.Second)
	defer ticker.Stop()

	for tick := range ticker.C {
		_ = tick
		for _, dev := range conns {
			buf := make([]byte, 4)
			n,err := dev.Services["Battery"].Chars[voltageUUID].Read(buf[:4])
			if err != nil{
				return err
			}
			val := int32(binary.LittleEndian.Uint32(buf[:n]))
			fmt.Println("value =", val)
		}
	}
	return nil
}

func findSrvcChars(profile *GATTProfile) error{
	batteryUUID := []bluetooth.UUID{voltageUUID}
	srvcs, err := profile.Device.DiscoverServices(batteryUUID)
	if err != nil{
		return err
	}
	if len(srvcs) == 0{
		// TODO: better error management
		return err
	}
	
	profile.Services["Battery"] = &ServiceClient{
		UUID:	voltageUUID,
		Chars:	make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic),
	}

	chars, err := srvcs[0].DiscoverCharacteristics(batteryUUID)
	if err != nil{
		return err
	}
	if len(chars)==0{
		return err
	}
	for _, char := range chars{
		profile.Services["Battery"].Chars[char.UUID()] = char
	}
	return nil
}