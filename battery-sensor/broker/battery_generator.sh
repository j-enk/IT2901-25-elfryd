#!/bin/bash

BASE_DIR=$(pwd)

# Colors for terminal output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
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

print_help() {
    echo "Battery Data Generator Usage:"
    echo "  bash battery_generator.sh [frequency] [num_batteries]"
    echo ""
    echo "Parameters:"
    echo "  frequency      - How often to send data in seconds (default: 1)"
    echo "  num_batteries  - Number of battery cells to simulate (default: 4)"
    echo ""
    echo "Examples:"
    echo "  bash battery_generator.sh            - Send data for 4 batteries every 1 second"
    echo "  bash battery_generator.sh 5 10       - Send data for 10 batteries every 5 seconds"
    echo ""
    exit 0
}

# Check for help flag
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    print_help
fi

# Parse arguments
FREQUENCY=${1:-1}
NUM_BATTERIES=${2:-4}

# Validate inputs
if ! [[ "$FREQUENCY" =~ ^[0-9]+$ ]] || [ "$FREQUENCY" -lt 1 ]; then
    print_error "Frequency must be a positive integer"
    print_help
fi

if ! [[ "$NUM_BATTERIES" =~ ^[0-9]+$ ]] || [ "$NUM_BATTERIES" -lt 1 ]; then
    print_error "Number of batteries must be a positive integer"
    print_help
fi

# Get hostname from environment file or system
if [ -f "$BASE_DIR/app/.env" ]; then
    source "$BASE_DIR/app/.env"
    HOST=${ELFRYD_HOSTNAME}
else
    # Default to system hostname if .env file not found
    HOST=$(hostname -f)
    print_warning "No .env file found. Using system hostname: $HOST"
fi

# Configure TLS port
PORT="8885"

# Set CA certificate location - check both possible locations
if [ -f "$BASE_DIR/certs/ca.crt" ]; then
    CA_FILE="$BASE_DIR/certs/ca.crt"
elif [ -f "$BASE_DIR/client_certs/ca.crt" ]; then
    CA_FILE="$BASE_DIR/client_certs/ca.crt"
else
    print_error "CA certificate file not found."
    echo "Please run this script from the broker directory where certificates were created."
    echo "Or update the CA_FILE path in the script manually."
    exit 1
fi

print_section "Continuous Battery Data Generator"
echo "Host: $HOST"
echo "CA file: $CA_FILE"
echo "Frequency: Every $FREQUENCY seconds"
echo "Number of battery cells: $NUM_BATTERIES"
echo "Average data rate: $NUM_BATTERIES readings per second"
echo ""
echo "Press Ctrl+C to stop the data generation"
echo ""

# Function to get current timestamp in seconds since epoch
get_timestamp() {
    date +%s
}

# Function to send battery readings
send_battery_readings() {
    local messages=("$@")
    local topic="elfryd/battery"
    local combined_message=$(IFS='|'; echo "${messages[*]}")
    
    echo "Sending battery readings: $combined_message"
    mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CA_FILE" -t "$topic" -m "$combined_message"
}

# Initialize sine wave parameters for each battery
declare -a amplitude
declare -a period
declare -a phase
declare -a baseline

# Set different parameters for each battery to create variety
for ((i=1; i<=$NUM_BATTERIES; i++)); do
    # Random amplitude between 0.5V and 1.5V (in millivolts)
    amplitude[$i]=$(( (RANDOM % 1000 + 500) ))
    
    # Random period between 60 and 180 seconds
    period[$i]=$(( (RANDOM % 120 + 60) ))
    
    # Random phase offset between 0 and 2π
    phase[$i]=$(( (RANDOM % 628) / 100 ))
    
    # Random baseline voltage between 8V and 10V (in millivolts)
    baseline[$i]=$(( (RANDOM % 2000 + 8000) ))
done

# Setup trap to handle Ctrl+C gracefully
trap 'echo -e "\nStopping data generation. Goodbye!"; exit 0' SIGINT SIGTERM

# Calculate how many data points to send per batch
DATA_POINTS_PER_BATCH=$((FREQUENCY * NUM_BATTERIES))

# Calculate reference time for consistent sine wave
START_TIME=$(get_timestamp)
time_offset=0

print_section "Starting continuous data generation..."

while true; do
    messages=()
    
    # Generate data points for the current time window
    for ((t=0; t<FREQUENCY; t++)); do
        current_timestamp=$(($(get_timestamp) + t))
        
        for ((b=1; b<=$NUM_BATTERIES; b++)); do
            # Calculate elapsed time for sine wave
            elapsed_time=$((time_offset + t))
            
            # Calculate voltage based on sine wave + noise
            sine_value=$(echo "s($elapsed_time / ${period[$b]} * 6.28 + ${phase[$b]})" | bc -l)
            voltage_base=$(echo "${baseline[$b]} + ${amplitude[$b]} * $sine_value" | bc)
            
            # Add random noise (±200mV)
            noise=$(( (RANDOM % 400) - 200 ))
            voltage=$(( ${voltage_base%.*} + noise ))
            
            # Ensure voltage stays within bounds (3000mV to 13000mV)
            if [ "$voltage" -lt 3000 ]; then
                voltage=3000
            elif [ "$voltage" -gt 13000 ]; then
                voltage=13000
            fi
            
            # Create message in format: battery_id/voltage/timestamp
            message="${b}/${voltage}/${current_timestamp}"
            messages+=("$message")
        done
    done
    
    # Send all messages
    send_battery_readings "${messages[@]}"
    
    # Update time offset for next batch
    time_offset=$((time_offset + FREQUENCY))
    
    # Sleep until next batch
    sleep $FREQUENCY
done