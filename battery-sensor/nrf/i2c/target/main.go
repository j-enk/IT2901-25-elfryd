package main

import (
	"fmt"
	"machine"
	"time"
)

func main() {
	time.Sleep((time.Second * 5))
	fmt.Println("Starting I2C target device...")

	// Configure I2C in target mode with the desired address
	i2c := machine.I2C0
	config := machine.I2CConfig{
		Frequency: machine.KHz * 100,
		SDA:       machine.SDA_PIN,
		SCL:       machine.SCL_PIN,
		Mode:      machine.I2CModeTarget,
	}
	if err := i2c.Configure(config); err != nil {
		println("Failed to configure I2C:", err)
		return
	}

	// Set the I2C address for the target device
	err := i2c.Listen(0x69)
	if err != nil {
		panic(err)
	}

	for {
		register := 0
		buf := make([]byte, 64)
		fmt.Printf("Waiting for I2C event...\n")
		evt, n, err := i2c.WaitForEvent(buf)
		if err != nil {
			panic(err)
		}
		switch evt {
		case machine.I2CReceive:
			if n == 1 {
				register = int(buf[0])
				fmt.Printf("Received register address: %d\n", register)
				continue
			}
		case machine.I2CRequest:
			machine.I2C0.Reply([]byte{1, 2, 3, 4})
		}
	}
}
