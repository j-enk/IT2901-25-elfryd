# MQTT Bridge Documentation

This document explains the MQTT bridge component of the Elfryd system, which connects the MQTT broker to the database, processing and storing incoming messages.

## Bridge Overview

The MQTT bridge serves as a connector between the MQTT broker and the TimescaleDB database. It subscribes to all MQTT topics, processes incoming messages based on their topic and format, and stores them in the appropriate database tables.

## Architecture

The bridge follows these core principles:

1. **Message Processing Pipeline**: Subscribe → Parse → Transform → Store
2. **Topic-Based Routing**: Different topics are processed by specialized handlers
3. **Automatic Schema Creation**: Database tables are created dynamically as needed
4. **Efficient Data Storage**: Only relevant data is extracted and stored

## Topic Structure and Message Formats

The bridge recognizes the following topic patterns and their corresponding message formats:

### Battery Data (`elfryd/battery`)

**Format**: `{battery_id}/{voltage}/{timestamp}`

**Example**: `1/8432/1680123456`

**Parameters**:
- `battery_id`: Identifier of the battery (integer)
- `voltage`: Battery voltage in millivolts (integer)
- `timestamp`: Device timestamp in Unix seconds (integer)

**Storage**: Data is stored in the `elfryd_battery` table with fields:
- `id`: Auto-incrementing record ID
- `battery_id`: Battery identifier
- `voltage`: Battery voltage
- `device_timestamp`: Device timestamp
- `timestamp`: Server timestamp when received

### Temperature Data (`elfryd/temp`)

**Format**: `{temperature}/{timestamp}`

**Example**: `25/1680123456`

**Parameters**:
- `temperature`: Temperature reading in degrees Celsius (integer)
- `timestamp`: Device timestamp in Unix seconds (integer)

**Storage**: Data is stored in the `elfryd_temp` table.

### Gyroscope Data (`elfryd/gyro`)

**Format**: `{accel_x}/{accel_y}/{accel_z}/{gyro_x}/{gyro_y}/{gyro_z}/{timestamp}`

**Example**: `-4991017/-4984009/4979460/-239841/241869/-243303/1680123456`

**Parameters**:
- `accel_x`, `accel_y`, `accel_z`: Accelerometer readings
- `gyro_x`, `gyro_y`, `gyro_z`: Gyroscope readings
- `timestamp`: Device timestamp in Unix seconds

**Storage**: Data is stored in the `elfryd_gyro` table.

### Configuration Commands (`elfryd/config/send`)

**Format**: `{command}`

**Example**: `battery 10`

**Parameters**:
- `command`: Configuration command in the format `<sensor_type> [interval]`

**Valid commands**:
- `battery`, `temperature`, `gyro`: Force the device to send all available data for that sensor type
- `battery [interval]`, `temperature [interval]`, `gyro [interval]`: Set sampling interval in seconds (0 disables sampling)

**Confirmation Messages**:
Devices can respond to configuration commands by publishing to the `elfryd/config/confirm` topic with the same format as the original command. Confirmation messages are stored in the same table as the original command.

**Storage**: Commands are stored in the `elfryd_config` table with command and topic information.

### Other Topics

For any other topics, the bridge stores the entire message with minimal processing:
- `topic`: Full topic name
- `message`: Message payload
- `timestamp`: Server timestamp when received

## Batch Processing

The bridge supports processing multiple data points in a single message using the pipe (`|`) character as a separator:

```
1/8432/1680123456|2/7965/1680123456|3/8104/1680123456
```

This allows for more efficient data transmission from devices that need to send multiple readings at once.

## Database Table Structure

The bridge automatically creates the following tables as needed:

### elfryd_battery
```sql
CREATE TABLE elfryd_battery (
    id SERIAL PRIMARY KEY,
    battery_id INTEGER NOT NULL,
    voltage INTEGER NOT NULL,
    device_timestamp BIGINT NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW()
);
```

### elfryd_temp
```sql
CREATE TABLE elfryd_temp (
    id SERIAL PRIMARY KEY,
    temperature INTEGER NOT NULL,
    device_timestamp BIGINT NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW()
);
```

### elfryd_gyro
```sql
CREATE TABLE elfryd_gyro (
    id SERIAL PRIMARY KEY,
    accel_x INTEGER NOT NULL,
    accel_y INTEGER NOT NULL,
    accel_z INTEGER NOT NULL,
    gyro_x INTEGER NOT NULL,
    gyro_y INTEGER NOT NULL,
    gyro_z INTEGER NOT NULL,
    device_timestamp BIGINT NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW()
);
```

### elfryd_config
```sql
CREATE TABLE elfryd_config (
    id SERIAL PRIMARY KEY,
    command TEXT NOT NULL,
    topic TEXT NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW()
);
```

### Default table structure for other topics
```sql
CREATE TABLE topic_name (
    id SERIAL PRIMARY KEY,
    topic TEXT NOT NULL,
    message TEXT NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW()
);
```

## Error Handling

The bridge includes robust error handling to ensure continuous operation:

1. **Message Parsing Errors**: Invalid messages are logged but don't crash the bridge
2. **Database Connection Issues**: The bridge will log errors and continue trying to process messages
3. **Table Creation**: Tables are created automatically as needed

## Testing the Bridge

You can test the MQTT bridge by sending messages to the broker using the `mosquitto_pub` tool:

```bash
mosquitto_pub -h your-vm-dns-name -p 8885 --cafile ./client_certs/ca.crt -t elfryd/battery -m "1/8500/$(date +%s)"
```

After sending a message, you can verify it was processed by querying the API:

```bash
curl -k -X GET https://your-vm-dns-name:443/battery?limit=1 -H "X-API-Key: $API_KEY"
```

## Bridge Code Structure

The bridge code is organized into these components:

- `mqtt_bridge.py`: Main bridge application
- `handlers/`: Directory containing specialized handlers for different topics
  - `battery_handler.py`: Handles battery messages
  - `temperature_handler.py`: Handles temperature messages
  - `gyro_handler.py`: Handles gyroscope messages
  - `config_handler.py`: Handles configuration messages
  - `default_handler.py`: Handles all other messages

## Further Reading

For more information on how to interact with the data stored by the bridge, see the [API Documentation](api.md).