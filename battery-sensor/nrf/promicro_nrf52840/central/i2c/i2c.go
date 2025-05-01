package i2c_target

import (
    "Kystlaget_central/ble"
    "fmt"
    "machine"
)

var (
    i2c      = machine.I2C0
    lastReg  byte
    registers map[byte]*Register
    sensorType = "null"
)

// Register describes a readable/writable register
type Register struct {
    ReadData     []byte
    WriteHandler func(data []byte)
}

// InitI2C sets up the I²C peripheral in target mode
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

// ConfigI2C sets the I²C address and initializes registers
func ConfigI2C(addr uint8) {
    if err := i2c.Listen(addr); err != nil {
        panic("I2C Listen failed: " + err.Error())
    }
    setupRegisters()
}

func setupRegisters() {
    // only called once
    registers = map[byte]*Register{
        0x00: {
            ReadData: []byte{0x01},
        },
        0x01: {
            ReadData: nil, // dynamic: handled in handleRequest
        },
    }
}

// PassiveListening loops forever handling I²C master events
func PassiveListening() error {
    if registers == nil {
        panic("registers not initialized; call ConfigI2C first")
    }
    buf := make([]byte, 64)
    for {
        fmt.Println("[I2C] Waiting for I²C event...")
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
            fmt.Println("[I2C] Event: Finish")
        default:
            fmt.Printf("[I2C] Unknown event: %v\n", evt)
        }
    }
}

func handleReceive(data []byte) {
    fmt.Printf("[I2C] Receive (%d): % X\n", len(data), data)
    if len(data) < 1 {
        return
    }
    reg := data[0]
    lastReg = reg

    r, ok := registers[reg]
    if !ok {
        fmt.Printf("[I2C] Unknown reg 0x%02X\n", reg)
        return
    }
    if len(data) > 1 {
        if r.WriteHandler != nil {
            r.WriteHandler(data[1:])
        } else {
            fmt.Printf("[I2C] Write to RO reg 0x%02X rejected\n", reg)
        }
    }
}

func handleRequest() {
    fmt.Println("[I2C] Master read request")

    if lastReg == 0x01 {
        batteryData := ble.GetBatteryArray()
        // pre-allocate a reasonable capacity
        reply := make([]byte, 0, len(batteryData)*8)

        for addr, msg := range batteryData {
            fmt.Printf("[I2C] Including device %s → New=%d ID=%d Payload len=%d\n",
                addr.String(), msg.New, msg.ID, len(msg.Payload),
            )

            // decide entry length
            var entryLen int
            switch sensorType {
            case "Gyro":
                entryLen = 19 // 1 byte New, 1 byte ID, 48 payload
            case "Temperature":
                entryLen = 6
            default:
                entryLen = 4
            }

            entry := make([]byte, entryLen)
            // pack New + ID
            entry[0] = byte(msg.New)
            cursed := 1
            switch sensorType {
            case "Battery":
                entry[1] = byte(msg.ID)
                cursed = 0
                
            }
            // copy as much of msg.Payload as fits
            copied := copy(entry[2-cursed:], msg.Payload)
            if copied < len(msg.Payload) {
                fmt.Printf("[I2C] Warning: payload truncated from %d to %d bytes\n",
                    len(msg.Payload), copied,
                )
            }

            reply = append(reply, entry...)
        }

        if len(reply) == 0 {
            fmt.Println("[I2C] No entries, sending a single zero entry")
            // match default entryLen for your sensor type
            zeroLen := 4
            if sensorType == "Gyro" {
                zeroLen = 24
            }
            reply = make([]byte, zeroLen)
        }

        fmt.Printf("[I2C] Replying all entries (%d bytes): % X\n",
            len(reply), reply,
        )
        i2c.Reply(reply)

        // clear the New flags
        for addr, msg := range batteryData {
            msg.New = 0
            ble.SetBatteryEntry(addr, msg)
        }
        return
    }
    // Static registers
    if r, ok := registers[lastReg]; ok && r.ReadData != nil {
        fmt.Printf("[I2C] Replying reg 0x%02X: % X\n", lastReg, r.ReadData)
        i2c.Reply(r.ReadData)
    } else {
        fmt.Printf("[I2C] Replying zeros for reg 0x%02X\n", lastReg)
        i2c.Reply(make([]byte, 5))
    }
}

