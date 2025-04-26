package ble

import (
	"fmt"
	"log"
	"time"
) 

func InitGATT(){
	for _, dev := conns {
		findSrvcChars(dev)
	}
}

func RunGATTClient() error{
	ticker:= time.NewTicker(time.Second)
	defer ticker.Stop()

	for tick := range ticker.C{
		for _, dev := conns {
			n,err := dev.Services["Battery"].Chars[voltageUUID].Read(buf[:4])
			if err != nil{
				return err
			}
			val := int32(binary.LittleEndian.Uint32(buf[:nyu]))
			fmt.Println("value =", val)
		}
	}
}

func findSrvcChars(profile GATTProfile) error{
	srvcs, err := profile.Device.DiscoverServices(voltageUUID)
	if err != nil{
		return err
	}
	if len(srvcs) == 0{
		// TODO: better error management
		return err
	}
	
	profile.Services["Battery"] = &ServiceClient{
		UUID:	voltageUUID
		Chars:	make(map[bluetooth.UUID]bluetooth.DeviceCharacteristic)
	}

	chars, err := srvcs[0].DiscoverCharacteristics([]bluetooth.UUID{voltageUUID})
	if err != nil{
		return err
	}
	if len(chars)==0{
		return err
	}
	for _, char := range chars{
	profile.Services["Battery"].Chars[char.UUID()] = char
	}
}