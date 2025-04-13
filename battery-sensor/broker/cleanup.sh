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

print_section "Elfryd MQTT/TLS Cleanup"
echo "This script will stop and remove all Docker containers and generated files."

# Ask about database volume
read -p "Do you want to preserve the database volume to keep your data? (y/n): " -n 1 -r KEEP_DB_DATA
echo
REMOVE_DB_VOLUME=true
if [[ $KEEP_DB_DATA =~ ^[Yy]$ ]]; then
    REMOVE_DB_VOLUME=false
    echo "Database volume will be preserved."
fi

# Ask about certificates
read -p "Do you want to preserve the TLS certificates? (y/n): " -n 1 -r KEEP_CERTS
echo
REMOVE_CERTS=true
if [[ $KEEP_CERTS =~ ^[Yy]$ ]]; then
    REMOVE_CERTS=false
    echo "TLS certificates will be preserved."
fi

# Ask about API key
read -p "Do you want to preserve the API key? (y/n): " -n 1 -r KEEP_API_KEY
echo
REMOVE_API_KEY=true
if [[ $KEEP_API_KEY =~ ^[Yy]$ ]]; then
    REMOVE_API_KEY=false
    echo "API key will be preserved."
fi

# Get confirmation for everything else
read -p "This will remove all containers and selected files. Continue? (y/n): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    print_warning "Operation cancelled."
    exit 1
fi

# Stop and remove Docker containers
print_section "Stopping Docker containers"
cd $BASE_DIR/app
docker compose down

# Remove Docker volumes
print_section "Managing Docker volumes"
if [ "$REMOVE_DB_VOLUME" = true ]; then
    echo "Removing all Docker volumes including database..."
    docker volume rm elfryd-timescaledb-data elfryd-mqtt-config elfryd-mqtt-certs elfryd-mqtt-data elfryd-mqtt-log elfryd-bridge-certs elfryd-api-certs app_timescaledb-data 2>/dev/null || echo "Some volumes could not be removed"
else
    echo "Preserving database volumes..."
    # Remove only non-database volumes
    docker volume rm elfryd-mqtt-config elfryd-mqtt-certs elfryd-mqtt-data elfryd-mqtt-log elfryd-bridge-certs elfryd-api-certs 2>/dev/null || echo "Some volumes could not be removed"
    echo "Database volumes 'elfryd-timescaledb-data' and 'app_timescaledb-data' have been preserved."
    # Show preserved volumes
    echo "Preserved volumes:"
    docker volume ls | grep -E "elfryd-timescaledb-data|app_timescaledb-data" || echo "No database volumes found"
fi

# Remove anonymous Docker volumes - BUT SKIP IF PRESERVING DATABASE
if [ "$REMOVE_DB_VOLUME" = true ]; then
    print_section "Removing anonymous volumes"
    anonymous_volumes=$(docker volume ls -qf "dangling=true")
    if [ -n "$anonymous_volumes" ]; then
        echo "Removing anonymous volumes:"
        echo "$anonymous_volumes"
        docker volume rm $anonymous_volumes
    else
        echo "No anonymous volumes to remove"
    fi
else
    print_section "Skipping anonymous volume removal to avoid accidental database loss"
    echo "You may need to manually remove unwanted anonymous volumes later with:"
    echo "docker volume ls -qf \"dangling=true\" | xargs -r docker volume rm"
fi

# Remove Docker network
print_section "Cleaning up Docker networks"
docker network rm app_elfryd-network elfryd-network 2>/dev/null || true

# Remove certificates and generated files
print_section "Managing certificates and generated files"
if [ "$REMOVE_CERTS" = true ]; then
    echo "Removing all certificates and generated files..."
    rm -rf $BASE_DIR/certs
    rm -rf $BASE_DIR/client_certs
    rm -f $BASE_DIR/elfryd_client_certs.tar.gz
    
    # Ask about Let's Encrypt certificates
    read -p "Do you also want to remove Let's Encrypt certificates? (y/n): " -n 1 -r REMOVE_LETSENCRYPT
    echo
    if [[ $REMOVE_LETSENCRYPT =~ ^[Yy]$ ]]; then
      if [ -f "$BASE_DIR/app/.env" ]; then
        HOSTNAME=$(grep ELFRYD_HOSTNAME "$BASE_DIR/app/.env" | cut -d'=' -f2)
        if [ -n "$HOSTNAME" ]; then
          echo "Removing Let's Encrypt certificates for $HOSTNAME..."
          
          # Try to remove with acme.sh if it exists
          if [ -f "$HOME/.acme.sh/acme.sh" ]; then
            source "$HOME/.acme.sh/acme.sh.env"
            ~/.acme.sh/acme.sh --remove -d $HOSTNAME --force
            echo "Removed certificates from acme.sh management"
          fi
          
          # Also remove the actual certificate files
          if [ -d "/etc/letsencrypt/live/$HOSTNAME" ]; then
            rm -rf /etc/letsencrypt/live/$HOSTNAME 2>/dev/null || true
            rm -rf /etc/letsencrypt/archive/$HOSTNAME 2>/dev/null || true
            rm -rf /etc/letsencrypt/renewal/$HOSTNAME.conf 2>/dev/null || true
            echo "Removed Let's Encrypt certificate files"
          fi
        fi
      fi
    else
      echo "Let's Encrypt certificates will be preserved."
    fi
else
    echo "Preserving TLS certificates for reuse..."
    # Just remove the MQTT broker configuration, keep certificates
    echo "Preserved certificates in $BASE_DIR/certs/"
fi

# Handle API key while preserving hostname
print_section "Managing environment configuration"
if [ "$REMOVE_API_KEY" = true ]; then
    echo "Removing API key but preserving hostname..."
    if [ -f "$BASE_DIR/app/.env" ]; then
        # Extract hostname if present
        HOSTNAME_ENTRY=$(grep "ELFRYD_HOSTNAME=" "$BASE_DIR/app/.env" || echo "")

        # Remove the old .env file
        rm -f "$BASE_DIR/app/.env"

        # If hostname was present, create a new .env with just the hostname
        if [ -n "$HOSTNAME_ENTRY" ]; then
            echo "$HOSTNAME_ENTRY" >"$BASE_DIR/app/.env"
        fi
    else
        echo "No .env file found."
    fi
else
    echo "Preserving API key for reuse..."
fi

# Always remove the MQTT broker configuration to ensure clean restart
rm -rf $BASE_DIR/app/mqtt-broker/

# Remove any dangling Docker images
print_section "Cleaning up Docker images"
docker image prune -f

# Remove the global environment file if it exists (legacy file)
if [ -f "/etc/elfryd/elfryd.env" ]; then
    echo "Removing old environment file format..."
    rm -f /etc/elfryd/elfryd.env
fi

print_section "Cleanup Complete!"
echo -n "All containers"
if [ "$REMOVE_DB_VOLUME" = false ]; then
    echo -n " have been removed, but the database volumes were preserved."
    echo "Your data will still be available when you reinstall."
    # Show preserved volumes again
    echo "Preserved volumes:"
    docker volume ls | grep -E "elfryd-timescaledb-data|app_timescaledb-data" || echo "No database volumes found"
else
    echo -n " and database volumes"
fi

if [ "$REMOVE_CERTS" = false ]; then
    echo " have been removed, but the TLS certificates were preserved."
    echo "You can use 'restart.sh' to quickly bring the services back up with the same certificates."
else
    echo " and certificates have been removed."
    echo "You'll need to run 'install.sh' to generate new certificates and restart services."
fi
echo "To reinstall, run: sudo bash install.sh"
