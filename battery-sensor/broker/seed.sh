#!/bin/bash

# Configuration
HOST="elfryd.northeurope.cloudapp.azure.com"
PORT="8885"
CA_FILE="/home/elfryd-adm/elfryd-branch-19/battery-sensor/broker/certs/ca.crt"

# Check if CA file exists
if [ ! -f "$CA_FILE" ]; then
    echo "Error: CA certificate file not found at $CA_FILE"
    echo "Please update the CA_FILE path in the script."
    exit 1
fi

echo "=== Elfryd MQTT Test Data Generator ==="
echo "This script will send test data to your MQTT broker"
echo "Host: $HOST"
echo "CA file: $CA_FILE"
echo ""

# Function to get current timestamp in seconds since epoch
get_timestamp() {
    date +%s
}

# Function to send a battery reading
send_battery_reading() {
    local battery_id="$1"
    local voltage="$2"
    local timestamp="$3"
    local message="${battery_id}/${voltage}/${timestamp}"
    local topic="elfryd/battery"
    
    echo "Sending battery reading: $message"
    mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CA_FILE" -t "$topic" -m "$message"
    sleep 0.5
}

# Function to send a temperature reading
send_temperature_reading() {
    local sensor_id="$1"
    local temperature="$2"
    local timestamp="$3"
    local message="${sensor_id}/${temperature}/${timestamp}"
    local topic="elfryd/temp"
    
    echo "Sending temperature reading: $message"
    mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CA_FILE" -t "$topic" -m "$message"
    sleep 0.5
}

# Function to send a gyroscope reading
send_gyro_reading() {
    local sensor_id="$1"
    local accel_x="$2"
    local accel_y="$3"
    local accel_z="$4"
    local gyro_x="$5"
    local gyro_y="$6"
    local gyro_z="$7"
    local timestamp="$8"
    local message="${sensor_id}/${accel_x},${accel_y},${accel_z}/${gyro_x},${gyro_y},${gyro_z}/${timestamp}"
    local topic="elfryd/gyro"
    
    echo "Sending gyro reading: $message"
    mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CA_FILE" -t "$topic" -m "$message"
    sleep 0.5
}

# Function to send a configuration command
send_config_command() {
    local command="$1"
    local topic="elfryd/config"
    
    echo "Sending config command: $command"
    mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CA_FILE" -t "$topic" -m "$command"
    sleep 0.5
}

echo "Generating battery data..."
base_timestamp=$(get_timestamp)

# Generate battery data for multiple battery IDs
for battery_id in 11 12 13 14 15; do
    # Generate multiple readings per battery with slightly different timestamps and voltages
    for i in {1..3}; do
        ts=$((base_timestamp - i*300))
        voltage=$((15000 + RANDOM % 2000))
        send_battery_reading "$battery_id" "$voltage" "$ts"
    done
done

echo "Generating temperature data..."
# Generate temperature data for multiple sensors
for sensor_id in 1 2 3 4; do
    # Generate multiple readings per sensor
    for i in {1..2}; do
        ts=$((base_timestamp - i*600))
        temperature=$((20 + RANDOM % 15))
        send_temperature_reading "$sensor_id" "$temperature" "$ts"
    done
done

echo "Generating gyroscope data..."
# Generate gyro data for multiple sensors
for sensor_id in 1 2 3; do
    # Generate readings with realistic accelerometer and gyroscope values
    ts=$((base_timestamp - RANDOM % 500))
    accel_x=$((RANDOM % 10000000 - 5000000))
    accel_y=$((RANDOM % 10000000 - 5000000))
    accel_z=$((RANDOM % 10000000 - 5000000))
    gyro_x=$((RANDOM % 500000 - 250000))
    gyro_y=$((RANDOM % 500000 - 250000))
    gyro_z=$((RANDOM % 500000 - 250000))
    
    # Format gyro values to include leading zeros
    formatted_gyro_x=$(printf "%07d" ${gyro_x#-})
    if [[ $gyro_x -lt 0 ]]; then
        formatted_gyro_x="-$formatted_gyro_x"
    else
        formatted_gyro_x="0$formatted_gyro_x"
    fi
    
    formatted_gyro_y=$(printf "%07d" ${gyro_y#-})
    if [[ $gyro_y -lt 0 ]]; then
        formatted_gyro_y="-$formatted_gyro_y"
    else
        formatted_gyro_y="0$formatted_gyro_y"
    fi
    
    formatted_gyro_z=$(printf "%07d" ${gyro_z#-})
    if [[ $gyro_z -lt 0 ]]; then
        formatted_gyro_z="-$formatted_gyro_z"
    else
        formatted_gyro_z="0$formatted_gyro_z"
    fi
    
    send_gyro_reading "$sensor_id" "$accel_x" "$accel_y" "$accel_z" "$formatted_gyro_x" "$formatted_gyro_y" "$formatted_gyro_z" "$ts"
done

echo "Sending configuration commands..."
# Send various configuration commands
for freq in 10 30 60 100 120; do
    send_config_command "freq $freq"
done

# Send some other types of configuration commands
send_config_command "battery"
send_config_command "mode normal"
send_config_command "sleep 300"

echo "Sending test verification messages..."
# Send some generic test messages
for i in {1..3}; do
    message="Test message $i at $(date)"
    topic="test/verification"
    echo "Sending test message: $message"
    mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CA_FILE" -t "$topic" -m "$message"
    sleep 0.5
done

echo ""
echo "All test data has been sent successfully!"
echo "You can verify the data using the curl and database query commands."