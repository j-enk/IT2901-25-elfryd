package i2c_target

import (
	"Kystlaget_central/ble"
	"encoding/binary"
	"fmt"
	"machine"
    "sync"
)

var (
	i2c      = machine.I2C0
	lastReg  byte
	registers map[byte]*Register
)

// Register describes a readable/writable register
type Register struct {
	// For a read, what bytes to return
	ReadData []byte
	// For a write, a hook to handle incoming data (nil → read-only)
	WriteHandler func(data []byte)
}

// InitI2C sets up the I2C peripheral
func InitI2C() error {
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

// ConfigI2C sets the device I2C address to listen on
func ConfigI2C(address uint8) {
	err := i2c.Listen(address)
	if err != nil {
		panic("I2C Listen failed: " + err.Error())
	}
    SetupRegisters()
}

// SetupRegisters initializes the available register map
func SetupRegisters() {
	registers = map[byte]*Register{
		0x00: {
            ReadData: []byte{0x01},     // Register which to pull the battery voltage from
		},
		0x01: {
			ReadData: ble.BatteryArray, // Dynamic battery array from BLE side
		},
	}
}

// PassiveListening waits for incoming I2C events forever
func PassiveListening() error {
	if registers == nil {
		panic("registers map is not initialized, call SetupRegisters() first")
	}

	buf := make([]byte, 64)

	for {
		fmt.Println("[I2C] Waiting for I2C event...")
		evt, n, err := i2c.WaitForEvent(buf)
		if err != nil {
			fmt.Printf("[I2C] WaitForEvent error: %v\n", err)
			continue
		}

		switch evt {
		case machine.I2CReceive:
			handleReceive(buf[:n])

		case machine.I2CRequest:
			handleRequest()

		case machine.I2CFinish:
			fmt.Println("[I2C] Event: Finish transaction")

		default:
			fmt.Printf("[I2C] Unknown event type: %v\n", evt)
		}
	}
}

// handleReceive processes incoming data written from the master
func handleReceive(data []byte) {
	fmt.Printf("[I2C] Event: Receive (%d bytes) data=% X\n", len(data), data)
	if len(data) < 1 {
		fmt.Println("[I2C] Warning: empty receive event")
		return
	}

	reg := data[0]
	lastReg = reg

	r, exists := registers[reg]
	if !exists {
		fmt.Printf("[I2C] Unknown register 0x%02X — will reject on read\n", reg)
		return
	}

	// Handle write to register if there is extra data
	if len(data) > 1 {
		if r.WriteHandler != nil {
			fmt.Printf("[I2C] Writing to reg 0x%02X: % X\n", reg, data[1:])
			r.WriteHandler(data[1:])
		} else {
			fmt.Printf("[I2C] Write attempted to read-only reg 0x%02X — rejected\n", reg)
		}
	} else {
		fmt.Printf("[I2C] Register 0x%02X selected for read\n", reg)
	}
}

// handleRequest sends data back to the master when requested
func handleRequest() {
	fmt.Println("[I2C] Event: Master read request")
	r, exists := registers[lastReg]
	if exists {
		// Special dynamic handling example:
		if lastReg == 0x01 {
			select {
			case msg := <-ble.MessageBus:
				fmt.Printf("[I2C] BLE Message ID=%d Payload=%d\n", msg.ID, msg.Payload)

				// Build a 5-byte response
				data := make([]byte, 5)
				data[0] = byte(msg.ID)
				binary.LittleEndian.PutUint32(data[1:], uint32(msg.Payload))

				fmt.Printf("[I2C] Replying dynamic BLE data: % X\n", data)
				i2c.Reply(data)

			default:
				// No new BLE message, reply zeros
				fmt.Println("[I2C] No BLE message available, sending zero response")
				i2c.Reply(make([]byte, 5))
			}
		} else {
			// Normal static register
			fmt.Printf("[I2C] Replying % X from register 0x%02X\n", r.ReadData, lastReg)
			i2c.Reply(r.ReadData)
		}
	} else {
		// Unknown register: reply with zeros
		fmt.Printf("[I2C] Unknown register 0x%02X, replying zeros\n", lastReg)
		i2c.Reply(make([]byte, 5))
	}
}
