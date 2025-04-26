package main

import (
	"Kystlaget_central/ble"
	"Kystlaget_central/I2C"
	"time"
	"log"
)



func main(){
	time.Sleep(3*time.Second)

	//init
	must("Init BLE", ble.InitBLE())
	must("Init I2C", i2c_target.InitI2C())

	go func(){
		time.Sleep(1*time.Minute)
		adapter.Stopscan()
	}

	must("Scanning", ble.ScanStart())

	must("Init Gatt client", ble.InitGATT())
	
	go must("Runtime Gatt client", ble.RunGATTClient())
	go func(){
		must("Config I2C", 	i2c_target.ConfigI2C(0x69))
		must("Runtime I2C", i2c_target.PassiveListening())
	}()
	select{}
}


func must(action string, err error) {
	if err != nil {
		log.Printf("%s failed: %v", action, err)
	}
}