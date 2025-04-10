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

# Check for existing hostname in environment file
STORED_HOSTNAME=""
if [ -f "/etc/elfryd/elfryd.env" ]; then
  source /etc/elfryd/elfryd.env
  STORED_HOSTNAME=$ELFRYD_HOSTNAME
fi

# Get hostname for TLS certificate
SYSTEM_HOSTNAME=$(hostname -f)
DEFAULT_HOSTNAME=${STORED_HOSTNAME:-$SYSTEM_HOSTNAME}

read -p "Enter the VM's fully qualified domain name (press Enter to use $DEFAULT_HOSTNAME): " USER_HOSTNAME
if [ -n "$USER_HOSTNAME" ]; then
  CommonName=$USER_HOSTNAME
else
  CommonName=$DEFAULT_HOSTNAME
fi

echo "Using hostname: $CommonName for connections"

# Save the hostname to environment file for other scripts
print_section "Saving hostname to environment file"
mkdir -p /etc/elfryd
echo "ELFRYD_HOSTNAME=$CommonName" > /etc/elfryd/elfryd.env
chmod 644 /etc/elfryd/elfryd.env
echo "✅ Hostname saved to /etc/elfryd/elfryd.env"

# Check if certificate exists
BASE_DIR=$(pwd)
if [ ! -f "$BASE_DIR/certs/ca.crt" ] || [ ! -f "$BASE_DIR/certs/server.crt" ] || [ ! -f "$BASE_DIR/certs/server.key" ]; then
  print_error "TLS certificates not found. Please run install.sh first."
  echo "If you've previously run cleanup.sh, you'll need to run install.sh to recreate certificates."
  exit 1
fi

print_section "Generating API security"

# Check if API key exists
if [ -f "$BASE_DIR/app/.env" ]; then
  read -p "Do you want to generate a new API key? (y/n): " -n 1 -r GEN_NEW_KEY
  echo
  
  if [[ $GEN_NEW_KEY =~ ^[Yy]$ ]]; then
    API_KEY=$(openssl rand -hex 32)
    echo "API_KEY=$API_KEY" > $BASE_DIR/app/.env
    echo "✅ Generated new API key: $API_KEY"
    echo "⚠️  Warning: Previous API key is no longer valid"
  else
    source $BASE_DIR/app/.env
    echo "✅ Using existing API key"
  fi
else
  API_KEY=$(openssl rand -hex 32)
  echo "API_KEY=$API_KEY" > $BASE_DIR/app/.env
  echo "✅ Generated secure API key: $API_KEY"
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
echo "✅ Docker containers started successfully"
echo "Waiting to ensure all services are up and running..."
sleep 15

# Wait for services to be ready
print_section "Testing API readiness"
echo "Waiting for API to become available..."

# Try to access the API health endpoint with timeout and retries
max_retries=30
retry_count=0
while [ $retry_count -lt $max_retries ]; do
  if curl -k -s https://$CommonName:443/health | grep -q "healthy"; then
    echo "✅ API is ready!"
    break
  fi
  retry_count=$((retry_count + 1))
  echo "Waiting for API to become ready (attempt $retry_count/$max_retries)..."
  sleep 2
done

if [ $retry_count -eq $max_retries ]; then
  print_warning "Timed out waiting for API to become ready. Continuing anyway, but tests may fail."
  echo "Checking API logs:"
  docker logs elfryd-api | tail -n 20
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
  if curl -k -s "https://$CommonName:443/messages?topic=$TEST_TOPIC&limit=1" -H "X-API-Key: $API_KEY" | grep -q "$TEST_MESSAGE"; then
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
echo "  - FastAPI: port 443 (HTTPS)"
echo "  - TimescaleDB: internal only (only accessible through API)"
echo "  - MQTT-DB Bridge: running internally"
echo ""
echo "Available API endpoints:"
echo "  - GET  /health       - Check system health"
echo "  - GET  /messages     - Get stored messages with filters"
echo "  - POST /messages     - Publish MQTT message"
echo "  - GET  /topics       - Get list of unique topics"
echo ""
echo "To download client certificates to your local machine, run this command from your LOCAL machine:"
echo "  scp yourusername@$CommonName:$BASE_DIR/elfryd_client_certs.tar.gz ."
echo ""
echo "To test from this VM:"
echo "  - TLS MQTT: mosquitto_pub -h $CommonName -p 8885 --cafile $BASE_DIR/certs/ca.crt -t test/topic -m \"secure test message\""
echo "  - API (Public): curl -k -X GET https://$CommonName:443/health -w '\n'"
echo "  - API (Protected): curl -k -X GET https://$CommonName:443/messages -H \"X-API-Key: $API_KEY\" -w '\n'"
echo ""
echo "To stop and remove all containers and generated files, run: sudo bash cleanup.sh"
echo ""
echo -e "${GREEN}=================== ACTIVE API KEY ===================${NC}"
echo -e "${YELLOW}$API_KEY${NC}"
echo -e "${GREEN}=====================================================${NC}"
echo ""