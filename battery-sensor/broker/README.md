# Elfryd MQTT TLS Docker Setup

A comprehensive IoT messaging platform with MQTT broker, TLS security, TimescaleDB storage, and a REST API.

## Overview

This repository provides an easy way to set up a secure MQTT infrastructure with:

- Eclipse Mosquitto MQTT broker with TLS support
- TimescaleDB for storing MQTT messages
- A Python bridge connecting MQTT to the database
- FastAPI REST API for interacting with stored data

The architecture ensures that only the necessary services are exposed externally while keeping the database secure.

## Setting up a New VM on Azure

### 1. Create an Ubuntu VM on Azure

1. Log in to the [Azure Portal](https://portal.azure.com)
2. Click "Create a resource" > "Compute" > "Virtual Machine"
3. Select "Ubuntu Server 22.04 LTS" or newer
4. Choose your VM size (at least 1GB RAM recommended)
5. Setup authentication (SSH key recommended)
6. Allow SSH (port 22)
7. Review and create

### 2. Configure Network Security

1. Go to your VM's "Networking" settings
2. Click "Add inbound port rule"
3. Add rules for:
   - MQTT (port 1883)
   - MQTT over TLS (port 8885)
   - API (port 8000)

### 3. Configure DNS Name

1. Go to your VM's "Overview" page
2. Click on the "Configure" link next to DNS name
3. Enter a DNS name prefix (this will be used for your TLS certificate)
4. Click "Save"
5. Note the full DNS name (e.g., `yourvm.westeurope.cloudapp.azure.com`)

### 4. Connect to the VM

```bash
ssh yourusername@your-vm-dns-name
```

### 5. Setup Git and Clone Relevant Code

1. Install Git:
```bash
sudo apt update && sudo apt install -y git
```

2. Generate SSH key for GitHub:
```bash
ssh-keygen -t ed25519 -C "your_email@example.com"
```

3. Copy the SSH public key:
```bash
cat ~/.ssh/id_ed25519.pub
```

4. Add the key to your GitHub account:
   - Go to GitHub > Settings > SSH and GPG keys
   - Click "New SSH key"
   - Paste your public key and save

5. Initialize empty repo:
```bash
git init elfryd
cd elfryd
```

6. Add the remote repository:
```bash
git remote add origin git@github.com:j-enk/IT2901-25-elfryd.git
```

7. Configure sparse checkout:
```bash
git config core.sparseCheckout true
echo "battery-sensor/broker" >> .git/info/sparse-checkout
```

8. Pull the `broker` directory:
```bash
git pull origin main
cd battery-sensor/broker
```

### 6. Run the Installation Script

```bash
sudo bash install.sh
```

The script will:

- Install Docker and required packages
- Generate TLS certificates for the MQTT server
- Set up Docker containers for all services
- Start all services and run tests
- Create a certificate package for clients

### 7. Download Client Certificates

From your local machine:

```bash
scp yourusername@your-vm-dns-name:~/elfryd/battery-sensor/broker/elfryd_client_certs.tar.gz .
tar -xzf elfryd_client_certs.tar.gz
```

## Testing the System

### Test MQTT Connection (using TLS certificate)

```bash
mosquitto_pub -h your-vm-dns-name -p 8885 --cafile ./client_certs/ca.crt -t test/topic -m "Secure Hello World"
```

### Test API Access 

Check system health:
```bash
curl http://your-vm-dns-name:8000/health
```

Get recent messages:
```bash
curl http://your-vm-dns-name:8000/messages
```

Publish a message via API:
```bash
curl -X POST http://your-vm-dns-name:8000/messages \
  -H "Content-Type: application/json" \
  -d '{"topic": "api/test", "message": "Hello from API"}'
```

## API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/health` | GET | Check system health status |
| `/messages` | GET | Get stored messages with optional filters |
| `/messages` | POST | Publish a message to MQTT |
| `/topics` | GET | Get list of unique topics |

## Repository Structure

```
azure_broker/
├── app/                          # Main application directory
│   ├── mqtt-broker/              # MQTT broker configuration
│   │   └── config/               # Broker config files
│   ├── api/                      # FastAPI application
│   │   └── app.py                # API code
│   ├── docker-compose.yml        # Container orchestration
│   ├── Dockerfile.bridge         # MQTT-DB bridge container
│   ├── Dockerfile.api            # API container
│   ├── mqtt_bridge.py            # MQTT-DB bridge code
│   └── requirements.txt          # Python dependencies
├── certs/                        # Generated TLS certificates
├── client_certs/                 # Certificates for clients
├── .gitignore                    # Git ignore file
├── cleanup.sh                    # Main cleanup script
├── elfryd_client_certs.tar.gz    # Client certificates package
├── install.sh                    # Main installation script
├── mqtt_monitor.sh               # Script to monitor MQTT messages
├── README.md                     # This file
└── restart.sh                    # Restart script
```

## Cleaning Up and Preserving Data

When you need to stop and clean up the services, you can use the `cleanup.sh` script which provides options to preserve important data:

```bash
sudo bash cleanup.sh
```

The cleanup script offers two preservation options:

1. **Database preservation** - Keep your TimescaleDB data volumes intact so all historical message data is preserved
2. **Certificate preservation** - Keep your TLS certificates so you don't need to regenerate them

This gives you three different cleanup/restart workflows:

### Complete Reinstallation
If you choose not to preserve anything during cleanup, you'll need to run the full installation script to restart:
```bash
sudo bash cleanup.sh    # Answer 'n' to both preservation questions
sudo bash install.sh    # Full reinstallation with new certificates
```

### Quick Restart with Preserved Certificates
If you preserve certificates during cleanup, you can use the restart script:
```bash
sudo bash cleanup.sh    # Answer 'y' to certificate preservation question
sudo bash restart.sh    # Quick restart using existing certificates
```

### Fresh Start with Preserved Data
If you preserve only the database, you'll get new certificates but keep all message history:
```bash
sudo bash cleanup.sh    # Answer 'y' to database preservation, 'n' to certificates
sudo bash install.sh    # Fresh installation with new certificates but preserved database
```

> **Note:** The restart script (`restart.sh`, described below) requires existing certificates to work. If you've removed certificates with cleanup, you must run the full `install.sh` script instead.

## Restarting Services

If you need to restart the services without reinstalling everything (for example, after a system reboot or after running `cleanup.sh` but keeping the certificates), you can use the restart script:

```bash
sudo bash restart.sh
```

This script will:
- Check for existing certificates
- Recreate any missing configuration files
- Start all Docker containers
- Verify that services are running properly
- Test the MQTT bridge connection to ensure messages flow through to the database

Unlike the full installation script, this script:
- Does not regenerate TLS certificates if they already exist
- Does not reinstall Docker or other dependencies

If the certificates or Docker installation are missing, you'll need to run the full `install.sh` script instead.


## Monitoring MQTT Messages

To monitor MQTT messages in real-time on the server, you can use the included monitoring script:

```bash
bash mqtt_monitor.sh
```

This script polls the API for new messages and displays them in a clear format, automatically refreshing when new messages arrive. 

The default display shows the last 10 messages with a 1-second refresh interval, but you can customize these settings with arguments:

```bash
bash mqtt_monitor.sh [lines] [interval]
```
- `lines` - Number of lines to display (default: 10)
- `interval` - Refresh interval in seconds (default: 1)

For example, to display 20 lines with a 2-second refresh interval:

```bash
bash mqtt_monitor.sh 20 2
```

The script requires jq for JSON processing, and you will be prompted to install it if it's missing.

## Troubleshooting

### Check container status
```bash
docker ps -a
```

### Check volumes
```bash
docker volume ls
```

### View container logs
```bash
docker logs mqtt-broker
docker logs mqtt-bridge
docker logs elfryd-api
docker logs timescaledb
```

### Check TLS connections
```bash
openssl s_client -connect localhost:8885
```

### Lingering processes on ports
If you get problems with the MQTT connection during the install script after cleaning up, you can try looking for processes on the TLS port and killing them:
```bash
sudo lsof -i :8885
sudo kill <PID>
```
Similarly, for the API, bridge and database ports:
```bash
sudo lsof -i :8000
sudo lsof -i :5432
sudo lsof -i :1883
sudo kill <PID>
```

## Docker Permissions

If you encounter permission issues with Docker, you can add your user to the `docker` group:

```bash
sudo usermod -aG docker $USER
```

To run Docker commands without `sudo`, you may also need to run the following:

```bash
sudo chmod 666 /var/run/docker.sock
``` 

## Connecting to the MQTT Broker from Clients

### Python Example

```python
import paho.mqtt.client as mqtt
import ssl

# Setup TLS
client = mqtt.Client()
client.tls_set(
    ca_certs="./ca.crt",  # Path to the downloaded CA certificate
    cert_reqs=ssl.CERT_REQUIRED,
    tls_version=ssl.PROTOCOL_TLSv1_2
)

# Connect and publish
client.connect("your-vm-dns-name", 8885, 60)
client.publish("test/topic", "Hello from IoT device")
client.disconnect()
```

Run `pip install paho-mqtt` to install the Paho MQTT client library if you don't have it already.

### Zephyr Example

See the [nRF9160 Feather docs](../nrf/hub/README.md) for a Zephyr-based MQTT client.