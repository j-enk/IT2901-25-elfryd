package i2c_target

import (
	"fmt"
	"machine"
)

var i2c = machine.I2C0

func InitI2C() error {

	fmt.Println("heloooooo")
	config := machine.I2CConfig{
		Frequency: machine.KHz * 100,
		SDA:       machine.SDA_PIN,
		SCL:       machine.SCL_PIN,
		Mode:      machine.I2CModeTarget,
	}

	if err := i2c.Configure(config); err != nil {
		return err
	}
	return nil
}

func ConfigI2C(address uint8) {
	err := i2c.Listen(address)
	if err != nil {
		panic(err)
	}

}

func PassiveListening() error{
	for {
		register := 0
		buf := make([]byte, 64)
		fmt.Printf("Waiting for I2C event...\n")
		evt, n, err := i2c.WaitForEvent(buf)
		if err != nil {
			return err
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
