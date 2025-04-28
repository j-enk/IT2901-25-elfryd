# Scripts Documentation

This document explains all utility scripts available in the Elfryd MQTT broker system, their purpose, usage, and options.

## Overview of Available Scripts

| Script                 | Description                                                                   |
| ---------------------- | ----------------------------------------------------------------------------- |
| `install.sh`           | Main installation script for setting up the entire broker system              |
| `cleanup.sh`           | Cleans up Docker containers and optionally preserves data and certificates    |
| `restart.sh`           | Restart services without full reinstallation, can also generate new API key   |
| `seed.sh`              | Populates the database with sample data (sensor readings and config messages) |
| `battery_generator.sh` | Continuously generates battery data with configurable parameters              |
| `db_monitor.sh`        | Monitors and displays database activity in real-time                          |

## install.sh

The main installation script that sets up the entire Elfryd MQTT broker infrastructure.

### Usage

```bash
sudo bash install.sh
```

### What it does

1. Installs necessary packages (Docker, Docker Compose, OpenSSL, etc.)
2. Creates TLS certificates for secure MQTT communication and SSL for the API
3. Generates a secure API key for protected endpoints
4. Sets up Docker containers for all services:
   - Mosquitto MQTT broker with TLS
   - TimescaleDB for data storage
   - MQTT-DB bridge service
   - FastAPI REST API with HTTPS
5. Runs tests to verify the installation
6. Creates client certificate package for devices

### Options

The script will prompt you for the hostname to use for TLS certificates. To be valid, this must match the DNS name of your VM. You can also choose to generate a new API key or use an existing one.

## cleanup.sh

Stops and removes Docker containers and optionally cleans up generated files.

### Usage

```bash
sudo bash cleanup.sh
```

### What it does

1. Stops all Docker containers
2. Optionally removes Docker volumes:
   - Database volume (contains all message history)
   - MQTT configuration and certificate volumes
3. Optionally removes generated files:
   - TLS certificates
   - API key
   - Client certificate package

### Options

The script will prompt you for preservation choices:

1. **Database preservation** - Keep your TimescaleDB data volumes intact
2. **Certificate preservation** - Keep your TLS certificates
3. **API key preservation** - Keep your API key so clients don't need to update

## restart.sh

Restart services without a full reinstallation. Useful after system reboots or after running `cleanup.sh`.

### Usage

```bash
sudo bash restart.sh
```

### What it does

1. Checks for existing certificates
2. Recreates any missing configuration files
3. Optionally generates a new API key or uses the existing one
4. Starts all Docker containers
5. Verifies that services are running properly

### Options

The script will prompt you if you want to generate a new API key (if one already exists).

## seed.sh

Sends sample data to the MQTT broker for testing purposes. This is useful for populating the database with initial readings.

### Usage

```bash
bash seed.sh
```

### What it does

Simulates sensor readings by sending messages of the correct format to the MQTT broker (see `bridge.md` for message formats). It also samples some configuration messages.

- Sends battery data to the `elfryd/battery` topic
- Sends temperature data to the `elfryd/temp` topic
- Sends gyroscope data to the `elfryd/gyro` topic
- Sends configuration messages to the `elfryd/config` topic

Assuming the bridge is running, this data will be stored in the TimescaleDB database, and you can view it via the API or directly in the database.

### Options

No command-line options, but you can modify the script to change the number of readings and voltage values.

## battery_generator.sh

Continuously generates battery data with configurable parameters, simulating real device output.

### Usage

```bash
bash battery_generator.sh [frequency] [num_batteries]
```

### Parameters

- `frequency` - How often to send data in seconds (default: 1)
- `num_batteries` - Number of battery cells to simulate (default: 4)

### Examples

```bash
# Send data for 4 batteries every 1 second
bash battery_generator.sh

# Send data for 10 batteries every 5 seconds
bash battery_generator.sh 5 10
```

> **Note**: The script simulates an average of 1 reading per second per battery, regardless of the transmission frequency.

### What it does

1. Creates sine-wave based battery voltage data (with some noise) to simulate realistic patterns
2. Sends data to the `elfryd/battery` topic in the format `{battery_id}/{voltage}/{timestamp}`
3. Runs continuously until stopped with by pressing 'q'

## db_monitor.sh

Monitors database contents in real-time by directly querying the TimescaleDB database, allowing you to see both device timestamps and server timestamps.

### Usage

```bash
bash db_monitor.sh <table> [lines] [interval]
bash db_monitor.sh messages <table_name> [lines] [interval]
```

### Parameters

- `table` - Required. Table type to monitor (`battery`, `temp`, `gyro`, `config`, or `messages`)
- `table_name` - Required when using the `messages` type. Specifies the exact table to query.
- `lines` - Number of rows to display (default: 10)
- `interval` - Refresh interval in seconds (default: 1)

### Examples

```bash
# Display last 10 battery readings, refresh every 1 second
bash db_monitor.sh battery

# Display last 20 temperature readings, refresh every 2 seconds
bash db_monitor.sh temp 20 2

# Display last 15 gyroscope readings, refresh every 5 seconds
bash db_monitor.sh gyro 15 5

# Display configuration commands
bash db_monitor.sh config

# Display messages from a specific table
bash db_monitor.sh messages test_verification 15 2
```

### What it does

1. Directly queries the TimescaleDB database using Docker exec commands
2. For sensor data, displays both device timestamps and server timestamps
3. Calculates and displays latency between device time and server time
4. Updates in real-time based on the specified interval
5. Displays data in a formatted table
6. Provides interactive monitoring (press 'q' to quit)

### Advantages over API-based monitoring

1. Shows raw database data without API processing
2. Displays server timestamps that are not exposed through the API
3. Calculates latency between device time and message arrival
4. Allows monitoring of any table in the database (with the `messages` type)

## Common Workflows

### Complete Reinstallation

If you choose not to preserve anything during cleanup, you'll need to run the full installation script to restart:

```bash
sudo bash cleanup.sh    # Answer 'n' to all preservation questions
sudo bash install.sh    # Full reinstallation with new certificates and API key
```

### Quick Restart with Preserved Certificates and API Key

If you preserve certificates and API key during cleanup, you can use the restart script:

```bash
sudo bash cleanup.sh    # Answer 'y' to certificate and API key preservation questions
sudo bash restart.sh    # Quick restart using existing certificates and API key
```

### Fresh Start with Preserved Data but New Security

If you preserve only the database, you'll get new certificates and API key but keep all message history:

```bash
sudo bash cleanup.sh    # Answer 'y' to database preservation, 'n' to others
sudo bash install.sh    # Fresh installation with new certificates and API key but preserved database
```

## Troubleshooting

If you encounter issues with the scripts:

### Check Docker Status

```bash
docker ps -a
docker volume ls
```

### View Container Logs

```bash
docker logs mqtt-broker
docker logs mqtt-bridge
docker logs elfryd-api
docker logs timescaledb
```

### Check for Port Conflicts

If ports are already in use, you may need to kill existing processes:

```bash
sudo lsof -i :8885  # Check TLS MQTT port
sudo lsof -i :1883  # Check internal MQTT port
sudo lsof -i :443   # Check API port
sudo kill <PID>     # Kill the process if needed
```

For other troubleshooting options, refer to [VM Setup Guide](vm_setup.md).
