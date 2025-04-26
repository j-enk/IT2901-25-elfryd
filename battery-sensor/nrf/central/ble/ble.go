package ble

import (
	"tinygo.org/x/bluetooth"
	"bytes"
	"log"
)

func InitBLE() error{
	return Adapter.Enable()
}

func ScanStart() error{

	err := Adapter.Scan(func(a *bluetooth.Adapter, device_found bluetooth.ScanResult){
		payload:=device_found.AdvertisementPayload.Bytes()
		if bytes.Equal(payload[5:21], uuidFilter[:]){
			addr := device_found.Address
			_, ok := conns[addr]
			if !ok {
				dev, err := Adapter.Connect(addr, bluetooth.ConnectionParams{})
				if err!=nil{
					log.Printf("[ERROR] Connect failed for %s: %v", addr.String(), err)
				}
				conns[addr] = &GATTProfile{
					Device:		dev,
					Services:	make(map[string]*ServiceClient),
				}
			}
		}
		//nothing
	})
	if err!=nil{
		return err
	}
	return nil
}