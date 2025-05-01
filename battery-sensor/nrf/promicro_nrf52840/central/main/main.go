package main

import (
	"Kystlaget_central/I2C"
	"Kystlaget_central/ble"
	"fmt"
	"time"
)

var sensorType string

func main() {
	time.Sleep(3 * time.Second)
	fmt.Printf("string: %s\n",sensorType)

	//init
	must("Init BLE", ble.InitBLE())
	must("Init I2C", i2c_target.InitI2C(sensorType))

	must("Scanning", ble.ScanStart(sensorType))
	fmt.Println("Finished scanning")
	must("Init Gatt client", ble.InitGATT(sensorType))
	//i2c go rutine must go first i think🤷‍♂️
	var address uint8
	switch sensorType {
	case "Battery":
		address = 0x10
	case "Temperature":
		address = 0x20
	case "Gyro":
		address = 0x30
	default:
		panic("feil sensortyper")
	}
	i2c_target.ConfigI2C(address)
	go func() {
		must("Runtime I2C", i2c_target.PassiveListening())
	}()
	go must("Runtime Gatt client", ble.RunGATTClient())
	select {}
}

func must(action string, err error) {
	if err != nil {
		fmt.Printf("%s failed: %v", action, err)
	}
}
