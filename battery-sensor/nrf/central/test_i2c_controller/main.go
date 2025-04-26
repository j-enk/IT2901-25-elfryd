package main

import (
	"fmt"
	"machine"
	"time"
)

func main() {
	time.Sleep(3 * time.Second) // Allow USB init

	fmt.Println("Starting I2C controller...")

	i2c := machine.I2C0
	err := i2c.Configure(machine.I2CConfig{
		SDA:       machine.SDA_PIN,
		SCL:       machine.SCL_PIN,
		Frequency: machine.KHz * 100,
		Mode:      machine.I2CModeController,
	})
	if err != nil {
		panic(err)
	}

	targetAddr := uint16(0x69) // Match the target's address

	for {
		// Step 1: Write register address to target
		fmt.Println("Writing register address 0x01...")
		err := i2c.Tx(targetAddr, []byte{0x01}, nil)
		if err != nil {
			fmt.Println("Error sending register:", err)
			time.Sleep(time.Second)
			continue
		}

		// Step 2: Read response from target
		buf := make([]byte, 4)
		err = i2c.Tx(targetAddr, nil, buf)
		if err != nil {
			fmt.Println("Error reading response:", err)
		} else {
			fmt.Printf("Received response: %v\n", buf)
		}

		time.Sleep(2 * time.Second)
	}
}
