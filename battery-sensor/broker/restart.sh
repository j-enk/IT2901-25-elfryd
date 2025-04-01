#!/bin/bash

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_section() {
  echo -e "${GREEN}\n==== $1 ====${NC}\n"
}

print_warning() {
  echo -e "${YELLOW}WARNING: $1${NC}"
}

print_error() {
  echo -e "${RED}ERROR: $1${NC}"
}

# Check if script is run as root
if [ "$EUID" -ne 0 ]; then
  print_error "Please run as root or with sudo"
  exit 1
fi

print_section "Elfryd MQTT/TLS Service Restart"
echo "This script will restart the MQTT broker, database, and API services without recreating certificates."

# After line 28, add this section to get the hostname for TLS certificate
HOSTNAME=$(hostname -f)
read -p "Enter the VM's fully qualified domain name (press Enter to use $HOSTNAME): " USER_HOSTNAME
if [ -n "$USER_HOSTNAME" ]; then
  CommonName=$USER_HOSTNAME
else
  CommonName=$HOSTNAME
fi

echo "Using hostname: $CommonName for connections"

# Check if certificate exists
BASE_DIR=$(pwd)
if [ ! -f "$BASE_DIR/certs/ca.crt" ] || [ ! -f "$BASE_DIR/certs/server.crt" ] || [ ! -f "$BASE_DIR/certs/server.key" ]; then
  print_error "TLS certificates not found. Please run install.sh first."
  echo "If you've previously run cleanup.sh, you'll need to run install.sh to recreate certificates."
  exit 1
fi

# Check if Mosquitto configuration exists
if [ ! -f "$BASE_DIR/app/mqtt-broker/config/mosquitto.conf" ]; then
  print_section "Creating Mosquitto configuration"
  
  # Create directories for configuration
  mkdir -p $BASE_DIR/app/mqtt-broker/config
  
  # Create Mosquitto configuration file
  cat > $BASE_DIR/app/mqtt-broker/config/mosquitto.conf << EOL
# TLS-secured listener for IoT devices
listener 8885 0.0.0.0
allow_anonymous true
cafile /mosquitto/certs/ca.crt
keyfile /mosquitto/certs/server.key
certfile /mosquitto/certs/server.crt

# Enable internal listener for bridge
listener 1883 0.0.0.0
allow_anonymous true
EOL
fi

# Create client certificates package if it doesn't exist
if [ ! -f "$BASE_DIR/elfryd_client_certs.tar.gz" ]; then
  print_section "Creating client certificates package"
  mkdir -p $BASE_DIR/client_certs
  cp $BASE_DIR/certs/ca.crt $BASE_DIR/client_certs/
  cd $BASE_DIR
  tar -czf elfryd_client_certs.tar.gz client_certs
fi

# Start the Docker containers
print_section "Starting Docker containers"
cd $BASE_DIR/app
docker compose down
docker compose up -d --force-recreate

# Wait for services to be ready
print_section "Testing API readiness"
echo "Waiting for API to become available..."

# Try to access the API health endpoint with timeout and retries
max_retries=30
retry_count=0
while [ $retry_count -lt $max_retries ]; do
  if curl -s http://localhost:8000/health | grep -q "healthy"; then
    echo "✅ API is ready!"
    break
  fi
  retry_count=$((retry_count + 1))
  echo "Waiting for API to become ready (attempt $retry_count/$max_retries)..."
  sleep 2
done

if [ $retry_count -eq $max_retries ]; then
  print_warning "Timed out waiting for API to become ready."
fi

# After the API readiness check, add MQTT broker and bridge checks
print_section "Testing MQTT connection"
echo "Testing TLS MQTT connection..."

# Unique test topic and message to easily identify in database
TEST_TOPIC="test/verification/restart"
TEST_MESSAGE="Bridge test message $(date +%s)"

# Test TLS MQTT connection
if mosquitto_pub -h $CommonName -p 8885 --cafile $BASE_DIR/certs/ca.crt -t "$TEST_TOPIC" -m "$TEST_MESSAGE" 2>/dev/null; then
  echo "✅ TLS MQTT connection successful!"
else
  print_warning "TLS MQTT connection test failed"
fi

# Give the bridge a moment to process
print_section "Testing MQTT bridge"
echo "Checking if message reaches the database..."
sleep 5

# Check if message made it to the database through API
BRIDGE_WORKING=false
max_retries=10
retry_count=0

while [ $retry_count -lt $max_retries ]; do
  if curl -s "http://localhost:8000/messages?topic=$TEST_TOPIC&limit=1" | grep -q "$TEST_MESSAGE"; then
    echo "✅ MQTT bridge is working correctly! Message successfully stored in database."
    BRIDGE_WORKING=true
    break
  fi
  retry_count=$((retry_count + 1))
  echo "Waiting for message to appear in database (attempt $retry_count/$max_retries)..."
  sleep 2
done

if [ "$BRIDGE_WORKING" = false ]; then
  print_warning "Could not verify MQTT bridge functionality. Messages may not be stored in the database."
  echo "Checking MQTT bridge logs:"
  docker logs mqtt-bridge | tail -n 20
fi

# Get hostname for testing
HOSTNAME=$(hostname -f)

# Print completion message
print_section "Restart complete!"
echo "Services have been restarted with the following endpoints:"
echo "  - MQTT Broker: port 8885 (TLS-secured)"
echo "  - FastAPI: port 8000"
echo "  - TimescaleDB: internal only (not accessible externally)"
echo "  - MQTT-DB Bridge: running internally"
echo ""
echo "To test from this VM:"
echo "  - TLS MQTT: mosquitto_pub -h $CommonName -p 8885 --cafile $BASE_DIR/certs/ca.crt -t test/topic -m \"secure test message\""
echo "  - API: curl http://localhost:8000/health"
echo ""