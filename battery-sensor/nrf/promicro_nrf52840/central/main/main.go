package main

import (
	i2c_target "Kystlaget_central/I2C"
	"Kystlaget_central/ble"
	"fmt"
	"time"
)

func main() {
	time.Sleep(3 * time.Second)

	//init
	must("Init BLE", ble.InitBLE())
	must("Init I2C", i2c_target.InitI2C())

	must("Scanning", ble.ScanStart())
	fmt.Println("Finished scanning")
	must("Init Gatt client", ble.InitGATT())
	//i2c go rutine must go first i think🤷‍♂️
	go func() {
		i2c_target.ConfigI2C(0x69)
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
