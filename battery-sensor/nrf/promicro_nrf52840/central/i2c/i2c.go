// Package i2c_target implements an I²C target (slave) interface
// that exposes BLE sensor data to an I²C master. It uses TinyGo machine.I2C
// in target mode to listen for read/write requests on predefined registers.
package i2c_target

import (
	"Kystlaget_central/ble"
	"machine"
)

var (
	// i2c is the I²C peripheral in target mode
	i2c = machine.I2C0

	// lastReg stores the last register address selected by the master
	lastReg byte

	// registers maps register addresses to Register descriptors
	registers map[byte]*Register

	// sensorType indicates which BLE sensor data to expose via register 0x01
	sensorType = "null"
)

// Register describes a readable/writable I²C register
// ReadData: static bytes to return on read requests
// WriteHandler: optional function to handle writes to this register
type Register struct {
	ReadData     []byte
	WriteHandler func(data []byte)
}

// InitI2C configures the I²C peripheral in target mode and stores
// a sensor type identifier for dynamic data reads.
// sensorTypeMain must be one of "Battery", "Temperature", or "Gyro".
func InitI2C(sensorTypeMain string) error {
	cfg := machine.I2CConfig{
		Frequency: machine.KHz * 100,
		SDA:       machine.SDA_PIN,
		SCL:       machine.SCL_PIN,
		Mode:      machine.I2CModeTarget,
	}
	sensorType = sensorTypeMain
	return i2c.Configure(cfg)
}

// ConfigI2C sets the I²C target address and initializes the register map.
// Must be called after InitI2C and before PassiveListening.
func ConfigI2C(addr uint8) {
	if err := i2c.Listen(addr); err != nil {
		panic("I2C Listen failed: " + err.Error())
	}
	setupRegisters()
}

// setupRegisters populates the initial set of registers with
// static responses and placeholders for dynamic registers.
func setupRegisters() {
	registers = map[byte]*Register{
		0x00: { ReadData: []byte{0x01} }, // Fixed version or ID register
		0x01: { ReadData: nil },         // Dynamic data: BLE sensor values
	}
}

// PassiveListening loops indefinitely, handling I²C master events.
// It dispatches receive (register selection) and request (read data)
// events to the appropriate handlers.
func PassiveListening() error {
	if registers == nil {
		panic("registers not initialized; call ConfigI2C first")
	}
	buf := make([]byte, 64)
	for {
		evt, n, err := i2c.WaitForEvent(buf)
		if err != nil {
			continue // Ignore and wait for next event
		}
		switch evt {
		case machine.I2CReceive:
			handleReceive(buf[:n])
		case machine.I2CRequest:
			handleRequest()
		case machine.I2CFinish:
			// Transaction finished; no action needed
		default:
			// Unhandled event type
		}
	}
}

// handleReceive processes I²C write data from the master.
// The first byte selects the register; subsequent bytes invoke a write handler.
func handleReceive(data []byte) {
	if len(data) < 1 {
		return // No register specified
	}
	lastReg = data[0]
	r, exists := registers[lastReg]
	if !exists {
		return // Unknown register; ignore
	}
	if len(data) > 1 && r.WriteHandler != nil {
		r.WriteHandler(data[1:])
	}
}

// handleRequest responds to an I²C read request based on the last selected register.
// For register 0x01, it fetches BLE sensor data; for others, it returns static data.
func handleRequest() {
	// Dynamic register: BLE sensor payloads
	if lastReg == 0x01 {
		batteryData := ble.GetBatteryArray()
		// Preallocate reply buffer based on number of devices and sensor type
		reply := make([]byte, 0, len(batteryData)*8)
		for _, msg := range batteryData {
			var entryLen int
            var id int = 0
			switch sensorType {
			case "Gyro":
				entryLen = 1 + len(msg.Payload)
			case "Temperature":
				entryLen = 1 + len(msg.Payload)
			default: // "Battery"
				entryLen = 1 + 1 + len(msg.Payload)
                id = 1
			}

			entry := make([]byte, entryLen)
			entry[0] = byte(msg.New)
			if id == 1 {entry[1] = byte(msg.ID)}
			copy(entry[1+id:], msg.Payload)
			reply = append(reply, entry...)
		}

		// If no data, send a zero-filled default entry
		if len(reply) == 0 {
			zeroLen := 1 + 1 + 0
			reply = make([]byte, zeroLen)
		}

		i2c.Reply(reply)
		// Clear "New" flags for all entries
		for addr, msg := range batteryData {
			msg.New = 0
			ble.SetBatteryEntry(addr, msg)
		}
		return
	}

	// Static register: return preconfigured bytes or zeros
	if r, exists := registers[lastReg]; exists && r.ReadData != nil {
		i2c.Reply(r.ReadData)
	} else {
		i2c.Reply(make([]byte, 5))
	}
}
