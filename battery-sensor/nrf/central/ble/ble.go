package ble

import (
	"tinygo.org/x/bluetooth"
	"bytes"
)

func InitBLE() error{
	return adapter.Enable()
}

func ScanStart() error{

	err := adapter.Scan(func(a *bluetooth.Adapter, device_found bluetooth.ScanResult){
		payload:=result.AdvertisementPayload.Bytes()
		if bytes.Equal(payload[5:21], uuidFilter[:]){
			addr := result.Address
			_, ok := conns[addr]
			if !ok {
				dev, err := adapter.Connect(addr, bluetooth.ConnectionParams{})
				if err!=null{
					log.Printf("[ERROR] Connect failed for %s: %v", addr.String(), err)
				}
				conns[addr] = {
					Device:		dev
					Services:	make(map[string]*ServiceClient)
				}
			}
		}
		//nothing
	})

}