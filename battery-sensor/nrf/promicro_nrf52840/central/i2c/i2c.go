package i2c_target

import (
	"Kystlaget_central/ble"
	"fmt"
	"machine"
)

var (
	i2c        = machine.I2C0
	lastReg    byte
	registers  map[byte]*Register
	sensorType = "null"
)

type Register struct {
	ReadData     []byte
	WriteHandler func(data []byte)
}

func InitI2C(sensorTypeMain string) error {
	cfg := machine.I2CConfig{
		Frequency: machine.KHz * 100,
		SDA:       machine.SDA_PIN,
		SCL:       machine.SCL_PIN,
		Mode:      machine.I2CModeTarget,
	}
	sensorType = sensorTypeMain
	fmt.Printf("Initializing I2C with sensor type: %s\n", sensorType)
	return i2c.Configure(cfg)
}

func ConfigI2C(addr uint8) {
	fmt.Printf("Listening on I2C address: 0x%X\n", addr)
	if err := i2c.Listen(addr); err != nil {
		panic("I2C Listen failed: " + err.Error())
	}
	setupRegisters()
}

func setupRegisters() {
	registers = map[byte]*Register{
		0x00: {ReadData: []byte{0x01}},
		0x01: {ReadData: nil},
	}
	fmt.Println("I2C registers initialized")
}

func PassiveListening() error {
	if registers == nil {
		panic("registers not initialized; call ConfigI2C first")
	}
	buf := make([]byte, 64)
	fmt.Println("Starting I2C passive listening loop")
	for {
		evt, n, err := i2c.WaitForEvent(buf)
		if err != nil {
			fmt.Printf("I2C event error: %v\n", err)
			continue
		}
		switch evt {
		case machine.I2CReceive:
			fmt.Printf("Received data: %X\n", buf[:n])
			handleReceive(buf[:n])
		case machine.I2CRequest:
			fmt.Println("I2C read request")
			handleRequest()
		case machine.I2CFinish:
			fmt.Println("I2C transaction finished")
		default:
			fmt.Printf("Unknown I2C event: %v\n", evt)
		}
	}
}

func handleReceive(data []byte) {
	if len(data) < 1 {
		fmt.Println("No register specified in receive")
		return
	}
	lastReg = data[0]
	fmt.Printf("Selected register: 0x%X\n", lastReg)
	r, exists := registers[lastReg]
	if !exists {
		fmt.Println("Unknown register; ignoring")
		return
	}
	if len(data) > 1 && r.WriteHandler != nil {
		fmt.Printf("Writing to register 0x%X: %X\n", lastReg, data[1:])
		r.WriteHandler(data[1:])
	}
}

func handleRequest() {
	fmt.Printf("Handling read request for register 0x%X\n", lastReg)
	if lastReg == 0x01 {
		batteryData := ble.GetBatteryArray()
		reply := make([]byte, 0, len(batteryData)*8)
		for _, msg := range batteryData {
			entry := make([]byte, ble.I2CReplyLen)
			entry[0] = byte(msg.New)
			entry[1] = byte(msg.ID)
			copy(entry[2:], msg.Payload)
			reply = append(reply, entry...)
		}

		if len(reply) == 0 {
			reply = make([]byte, 2)
		}

		fmt.Printf("Replying with dynamic data: %X lenght=%d\n", reply,len(reply))
		i2c.Reply(reply)

		for addr, msg := range batteryData {
			msg.New = 0
			ble.SetBatteryEntry(addr, msg)
		}
		return
	}

	if r, exists := registers[lastReg]; exists && r.ReadData != nil {
		fmt.Printf("Replying with static data: %X\n", r.ReadData)
		i2c.Reply(r.ReadData)
	} else {
		fmt.Println("Replying with default zero data")
		i2c.Reply(make([]byte, 5))
	}
}
