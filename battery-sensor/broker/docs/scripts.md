# Scripts Documentation

This document explains all utility scripts available in the Elfryd MQTT broker system, their purpose, usage, and options.

## Overview of Available Scripts

| Script | Description |
|--------|-------------|
| `install.sh` | Main installation script for setting up the entire broker system |
| `cleanup.sh` | Cleans up Docker containers and optionally preserves data and certificates |
| `restart.sh` | Restart services without full reinstallation |
| `seed.sh` | Sends test battery data to the MQTT broker |
| `battery_generator.sh` | Continuously generates battery data with configurable parameters |
| `mqtt_monitor.sh` | Monitors MQTT messages in real-time on the server |

## install.sh

The main installation script that sets up the entire Elfryd MQTT broker infrastructure.

### Usage

```bash
sudo bash install.sh
```

### What it does

1. Installs necessary packages (Docker, Docker Compose, OpenSSL, etc.)
2. Creates TLS certificates for secure MQTT communication
3. Generates a secure API key for protected endpoints
4. Sets up Docker containers for all services:
   - Mosquitto MQTT broker with TLS
   - TimescaleDB for data storage
   - MQTT-DB bridge service
   - FastAPI REST API
5. Runs tests to verify the installation
6. Creates client certificate package for devices

### Options

The script will prompt you for the hostname to use for TLS certificates. You can either:
- Use the VM's DNS name (recommended for production)
- Use another hostname (for development or testing)

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

Sends sample battery data to the MQTT broker for testing.

### Usage

```bash
bash seed.sh
```

### What it does

Simulates battery readings by sending messages to the `elfryd/battery` topic with the format:
```
{battery_id}/{voltage}/{timestamp}
```

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

### What it does

1. Creates sine-wave based battery voltage data to simulate realistic patterns
2. Sends data to the `elfryd/battery` topic in the format `{battery_id}/{voltage}/{timestamp}`
3. Runs continuously until stopped with Ctrl+C or by pressing 'q'

## mqtt_monitor.sh

Monitors MQTT messages in real-time on the server, allowing you to see incoming data.

### Usage

```bash
bash mqtt_monitor.sh [lines] [interval]
```

### Parameters

- `lines` - Number of lines to display (default: 10)
- `interval` - Refresh interval in seconds (default: 1)

### Examples

```bash
# Display last 10 messages, refresh every 1 second
bash mqtt_monitor.sh

# Display last 20 messages, refresh every 2 seconds
bash mqtt_monitor.sh 20 2
```

### What it does

1. Polls the API for new messages
2. Displays them in a clear, formatted output
3. Automatically refreshes when new messages arrive
4. Uses the API key stored in the `.env` file for authentication

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

For more detailed setup information, refer to [VM Setup Guide](vm_setup.md).