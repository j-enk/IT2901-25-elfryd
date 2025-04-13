#!/bin/bash

BASE_DIR=$(pwd)

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

# Check for existing hostname in environment file
STORED_HOSTNAME=""
if [ -f "$BASE_DIR/app/.env" ]; then
  source $BASE_DIR/app/.env
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

echo "Using hostname: $CommonName for TLS certificates"

# Save the hostname to environment file for other scripts
if [ -f "$BASE_DIR/app/.env" ]; then
  # Append hostname to existing .env file if it exists
  grep -q "ELFRYD_HOSTNAME=" "$BASE_DIR/app/.env" &&
    sed -i "s/ELFRYD_HOSTNAME=.*/ELFRYD_HOSTNAME=$CommonName/" "$BASE_DIR/app/.env" ||
    echo "ELFRYD_HOSTNAME=$CommonName" >>"$BASE_DIR/app/.env"
else
  # Create new .env file with hostname
  echo "ELFRYD_HOSTNAME=$CommonName" >"$BASE_DIR/app/.env"
fi
echo "✅ Hostname saved to app/.env file"

# Install required packages
print_section "Installing required packages"
apt-get update
apt-get install -y ca-certificates curl gnupg openssl net-tools python3-certbot

# Install Docker
print_section "Installing Docker"
install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
chmod a+r /etc/apt/keyrings/docker.gpg
echo "deb [arch="$(dpkg --print-architecture)" signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu "$(. /etc/os-release && echo "$VERSION_CODENAME")" stable" | tee /etc/apt/sources.list.d/docker.list >/dev/null

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

# Create Let's Encrypt certificates for API
print_section "Setting up Let's Encrypt certificates for API"
echo "Checking if we can obtain Let's Encrypt certificates..."

# Create folder for Let's Encrypt certificate storage
mkdir -p /etc/letsencrypt/live/$CommonName

# Check if port 443 is available for the certbot standalone server
PORT_443_STATUS=$(netstat -tuln | grep ":443 " || echo "Available")
if [[ "$PORT_443_STATUS" != "Available" ]]; then
  print_warning "Port 443 appears to be in use. Will create a temporary self-signed certificate for startup."
  print_warning "To get proper Let's Encrypt certificates later, run: certbot certonly --standalone -d $CommonName"
  
  # Create directory for Let's Encrypt certificates with self-signed fallback
  mkdir -p /etc/letsencrypt/live/$CommonName
  
  # Generate a temporary self-signed certificate for the API
  print_section "Generating temporary self-signed certificate for API"
  # Copy from existing OpenSSL certs as a fallback
  cp $BASE_DIR/certs/server.key /etc/letsencrypt/live/$CommonName/privkey.pem
  cp $BASE_DIR/certs/server.crt /etc/letsencrypt/live/$CommonName/fullchain.pem
  
  # Add to .env file for later renewal
  echo "NEEDS_LETSENCRYPT=true" >> "$BASE_DIR/app/.env"
  echo "Temporary certificates created. Will use the same as MQTT broker for now."
else
  # Try to get Let's Encrypt certificate
  if certbot certonly --standalone --non-interactive --agree-tos --register-unsafely-without-email -d $CommonName; then
    print_section "Let's Encrypt certificate obtained successfully"
    echo "✅ Certificates stored in /etc/letsencrypt/live/$CommonName/"
    
    # Verify the certificates exist
    if [ -f "/etc/letsencrypt/live/$CommonName/privkey.pem" ] && [ -f "/etc/letsencrypt/live/$CommonName/fullchain.pem" ]; then
      echo "✅ Certificate files verified"
      echo "NEEDS_LETSENCRYPT=false" >> "$BASE_DIR/app/.env"
    else
      print_warning "Let's Encrypt certificates were not found at the expected location"
      print_warning "Using self-signed certificates as fallback"
      
      # Copy from existing OpenSSL certs as a fallback
      cp $BASE_DIR/certs/server.key /etc/letsencrypt/live/$CommonName/privkey.pem
      cp $BASE_DIR/certs/server.crt /etc/letsencrypt/live/$CommonName/fullchain.pem
      
      echo "NEEDS_LETSENCRYPT=true" >> "$BASE_DIR/app/.env"
    fi
  else
    print_warning "Could not obtain Let's Encrypt certificate. Using self-signed certificate instead."
    print_warning "To get proper Let's Encrypt certificates later, run: certbot certonly --standalone -d $CommonName"
    
    # Copy from existing OpenSSL certs as a fallback
    cp $BASE_DIR/certs/server.key /etc/letsencrypt/live/$CommonName/privkey.pem
    cp $BASE_DIR/certs/server.crt /etc/letsencrypt/live/$CommonName/fullchain.pem
    
    echo "NEEDS_LETSENCRYPT=true" >> "$BASE_DIR/app/.env"
  fi
fi

# Add certificate paths to .env file for the API container
echo "SSL_CERT_PATH=/etc/letsencrypt/live/$CommonName/fullchain.pem" >> "$BASE_DIR/app/.env"
echo "SSL_KEY_PATH=/etc/letsencrypt/live/$CommonName/privkey.pem" >> "$BASE_DIR/app/.env"

print_section "Generating API security"

# Generate a random API key if it doesn't exist
if [ -f "$BASE_DIR/app/.env" ]; then
  read -p "Do you want to generate a new API key? (y/n): " -n 1 -r GEN_NEW_KEY
  echo

  if [[ $GEN_NEW_KEY =~ ^[Yy]$ ]]; then
    API_KEY=$(openssl rand -hex 32)
    # Update API key in existing .env file
    grep -q "API_KEY=" "$BASE_DIR/app/.env" &&
      sed -i "s/API_KEY=.*/API_KEY=$API_KEY/" "$BASE_DIR/app/.env" ||
      echo "API_KEY=$API_KEY" >>"$BASE_DIR/app/.env"
    echo "✅ Generated new API key: $API_KEY"
    echo "⚠️  Warning: Previous API key is no longer valid"
  else
    # Load existing API key
    API_KEY=$(grep API_KEY "$BASE_DIR/app/.env" | cut -d'=' -f2)
    echo "✅ Using existing API key"
  fi
else
  # This condition should never be reached since we created the file earlier
  API_KEY=$(openssl rand -hex 32)
  echo "API_KEY=$API_KEY" >>"$BASE_DIR/app/.env"
  echo "✅ Generated secure API key: $API_KEY"
fi

# Create Mosquitto configuration
print_section "Configuring MQTT broker"

# Create directories for configuration only
mkdir -p $BASE_DIR/app/mqtt-broker/config

# Create Mosquitto configuration file
cat >$BASE_DIR/app/mqtt-broker/config/mosquitto.conf <<EOL
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
docker compose down
docker compose up -d --force-recreate
echo ""
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

# Test TLS MQTT connection
print_section "Testing MQTT broker"
echo "Testing TLS MQTT connection..."

# Unique test topic and message to easily identify in database
TEST_TOPIC="test/verification/install"
TEST_MESSAGE="Bridge test message $(date +%s)"

# Test TLS MQTT connection
if mosquitto_pub -h $CommonName -p 8885 --cafile $BASE_DIR/certs/ca.crt -t "$TEST_TOPIC" -m "$TEST_MESSAGE" 2>/dev/null; then
  echo "✅ TLS MQTT connection successful!"
else
  print_warning "TLS MQTT connection test failed"
fi

# Test internal bridge functionality
print_section "Testing MQTT bridge"
echo "Checking if message reaches the database..."

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

# Print completion message
print_section "Setup complete!"
echo "System setup is completed with the following services:"
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
echo "  - GET  /battery      - Get battery sensor data"
echo "  - GET  /temperature  - Get temperature sensor data"
echo "  - GET  /gyro         - Get gyroscope sensor data"
echo "  - GET  /config       - Get configuration messages"
echo "  - POST /config/send  - Send configuration command"
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
