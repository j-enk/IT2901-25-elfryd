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

print_section "Elfryd MQTT/TLS Setup"
echo "This script will set up a secure MQTT broker with TLS, database, and API on this VM."

# Get hostname for TLS certificate
HOSTNAME=$(hostname -f)
read -p "Enter the VM's fully qualified domain name (press Enter to use $HOSTNAME): " USER_HOSTNAME
if [ -n "$USER_HOSTNAME" ]; then
  CommonName=$USER_HOSTNAME
else
  CommonName=$HOSTNAME
fi

echo "Using hostname: $CommonName for TLS certificates"

# Install required packages
print_section "Installing required packages"
apt-get update
apt-get install -y ca-certificates curl gnupg openssl net-tools

# Install Docker
print_section "Installing Docker"
install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
chmod a+r /etc/apt/keyrings/docker.gpg
echo "deb [arch="$(dpkg --print-architecture)" signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu "$(. /etc/os-release && echo "$VERSION_CODENAME")" stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null

apt-get update
apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# Set up Docker permissions for non-root usage
print_section "Setting up Docker permissions"
usermod -aG docker $SUDO_USER
chmod 666 /var/run/docker.sock
echo "Docker permissions have been configured for non-root usage"

# Install Mosquitto clients for testing
apt-get install -y mosquitto-clients

# Setup directory structure 
print_section "Setting up directories"
BASE_DIR=$(pwd)

# Generate TLS certificates
print_section "Generating TLS certificates"
mkdir -p $BASE_DIR/certs
cd $BASE_DIR/certs

openssl req -new -x509 -days 730 -nodes -extensions v3_ca -keyout ca.key -out ca.crt \
  -subj "/CN=$CommonName/O=MQTT TLS/C=NO" \
  -addext "subjectAltName = DNS:$CommonName"

openssl genrsa -out server.key 2048
openssl req -new -out server.csr -key server.key -subj "/CN=$CommonName"
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 730
openssl rsa -in server.key -out server.key

# Set permissions
chmod a+r *.key

# Create Mosquitto configuration
print_section "Configuring MQTT broker"

# Create directories for configuration only
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

# Create a client certificate package
mkdir -p $BASE_DIR/client_certs
cp $BASE_DIR/certs/ca.crt $BASE_DIR/client_certs/
cd $BASE_DIR
tar -czf elfryd_client_certs.tar.gz client_certs

# Start the Docker containers
print_section "Starting Docker containers"
cd $BASE_DIR/app
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
  print_warning "Timed out waiting for API to become ready. Continuing anyway, but tests may fail."
fi

# Test MQTT bridge functionality (using localhost for bridge to connect to broker)
print_section "Testing MQTT bridge"
echo "Publishing a test message and verifying it reaches the database..."

# Unique test topic and message to easily identify in database
TEST_TOPIC="test/verification/install"
TEST_MESSAGE="Bridge test message $(date +%s)"

# Test TLS MQTT connection
echo "Testing TLS MQTT connection..."
if mosquitto_pub -h $CommonName -p 8885 --cafile $BASE_DIR/certs/ca.crt -t "$TEST_TOPIC" -m "$TEST_MESSAGE" 2>/dev/null; then
  echo "✅ TLS MQTT connection successful!"
else
  print_warning "TLS MQTT connection test failed"
fi

# Give the bridge a moment to process
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

# Print completion message
print_section "Setup complete!"
echo "System setup is completed with the following services:"
echo "  - MQTT Broker: port 8885 (TLS-secured)"
echo "  - FastAPI: port 8000"
echo "  - TimescaleDB: internal only (not accessible externally)"
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
echo "  - API: curl http://localhost:8000/health"
echo ""
echo "To stop and remove all containers and generated files, run: sudo bash cleanup.sh"
echo ""