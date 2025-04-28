package i2c_target

import (
	"Kystlaget_central/ble"
	"encoding/binary"
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

func PassiveListening() error {
    for {
        // 1) Wait for an I2C event
        register := 0
        buf := make([]byte, 64)
        fmt.Println("[I2C] Waiting for I2C event...")
        evt, n, err := i2c.WaitForEvent(buf)
        if err != nil {
            fmt.Printf("[I2C] WaitForEvent error: %v\n", err)
            return err
        }

        // 2) Handle the event type
        switch evt {
        case machine.I2CReceive:
            fmt.Printf("[I2C] Event: Receive (%d bytes)\n", n)
            if n == 1 {
                register = int(buf[0])
                fmt.Printf("[I2C] Received register address: %d\n", register)
            } else {
                fmt.Printf("[I2C] Unexpected receive length %d (want 1)\n", n)
            }

        case machine.I2CRequest:
            fmt.Println("[I2C] Event: Request from master")
            select {
            case msg := <-ble.MessageBus:
                fmt.Printf("[I2C] Got message from BLE: ID=%d, Payload=%d\n", msg.ID, msg.Payload)

                // Build the 5-byte reply
                data := make([]byte, 5)
                data[0] = byte(msg.ID)
                binary.LittleEndian.PutUint32(data[1:], uint32(msg.Payload))
                fmt.Printf("[I2C] Replying with bytes: %v\n", data)

                machine.I2C0.Reply(data)
                fmt.Println("[I2C] Reply sent")

            default:
                // No BLE data available
                fmt.Println("[I2C] No message in channel; sending zero response")
                zero := make([]byte, 5)
                fmt.Printf("[I2C] Replying with zeros: %v\n", zero)
                machine.I2C0.Reply(zero)
            }

        default:
            fmt.Printf("[I2C] Unknown event type: %v\n", evt)
        }
    }
}
