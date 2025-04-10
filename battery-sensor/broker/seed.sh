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
    local temperature="$1"
    local timestamp="$2"
    local message="${temperature}/${timestamp}"
    local topic="elfryd/temp"
    
    echo "Sending temperature reading: $message"
    mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CA_FILE" -t "$topic" -m "$message"
    sleep 0.5
}

# Function to send a gyroscope reading
send_gyro_reading() {
    local accel_x="$1"
    local accel_y="$2"
    local accel_z="$3"
    local gyro_x="$4"
    local gyro_y="$5"
    local gyro_z="$6"
    local timestamp="$7"
    local message="${accel_x},${accel_y},${accel_z}/${gyro_x},${gyro_y},${gyro_z}/${timestamp}"
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
for battery_id in 1 2 3 4; do
    # Generate multiple readings per battery with slightly different timestamps and voltages
    for i in {1..4}; do
        ts=$((base_timestamp - i*300))
        voltage=$((7000 + RANDOM % 6000))
        send_battery_reading "$battery_id" "$voltage" "$ts"
    done
done

echo "Generating temperature data..."
# Generate temperature data for one sensor
for i in {1..12}; do
    ts=$((base_timestamp - i*600))
    temperature=$((20 + RANDOM % 15))
    send_temperature_reading "$temperature" "$ts"
done

echo "Generating gyroscope data..."
# Generate multiple gyroscope readings (at least 10)
for i in {1..12}; do
    ts=$((base_timestamp - i*180))
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
        formatted_gyro_x="$formatted_gyro_x"
    fi

    formatted_gyro_y=$(printf "%07d" ${gyro_y#-})
    if [[ $gyro_y -lt 0 ]]; then
        formatted_gyro_y="-$formatted_gyro_y"
    else
        formatted_gyro_y="$formatted_gyro_y"
    fi

    formatted_gyro_z=$(printf "%07d" ${gyro_z#-})
    if [[ $gyro_z -lt 0 ]]; then
        formatted_gyro_z="-$formatted_gyro_z"
    else
        formatted_gyro_z="$formatted_gyro_z"
    fi

    send_gyro_reading "$accel_x" "$accel_y" "$accel_z" "$formatted_gyro_x" "$formatted_gyro_y" "$formatted_gyro_z" "$ts"
done

echo "Sending configuration commands..."
# Send various configuration commands
for freq in 10 30 60 100 120; do
    send_config_command "freq $freq"
done

# Send some other types of configuration commands
send_config_command "battery"
send_config_command "temp"
send_config_command "battery"
send_config_command "gyro"

echo ""
echo "All test data has been sent successfully!"
echo "You can verify the data using these commands:"
echo "docker exec -it timescaledb psql -U myuser -d mqtt_data -c \"SELECT * FROM elfryd_battery LIMIT 10;\""
echo "docker exec -it timescaledb psql -U myuser -d mqtt_data -c \"SELECT * FROM elfryd_temp LIMIT 10;\""
echo "docker exec -it timescaledb psql -U myuser -d mqtt_data -c \"SELECT * FROM elfryd_gyro LIMIT 10;\""
echo "docker exec -it timescaledb psql -U myuser -d mqtt_data -c \"SELECT * FROM elfryd_config LIMIT 10;\""