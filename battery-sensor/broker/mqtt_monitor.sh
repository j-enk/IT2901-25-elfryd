#!/bin/bash

# Load API key from environment file
if [ -f "./app/.env" ]; then
  source ./app/.env
else
  echo "Error: API key file not found. Please run install.sh first."
  exit 1
fi

# Function to print help
print_help() {
  echo "MQTT Monitor Usage:"
  echo "  bash mqtt_monitor.sh <topic> [lines] [interval]"
  echo ""
  echo "Parameters:"
  echo "  topic     - Topic to filter messages by (REQUIRED)"
  echo "  lines     - Number of messages to display (default: 10)"
  echo "  interval  - Refresh interval in seconds (default: 1)"
  echo ""
  echo "Examples:"
  echo "  bash mqtt_monitor.sh elfryd/battery      - Show battery data, 10 lines, refresh every 1s"
  echo "  bash mqtt_monitor.sh elfryd/temp 20 2    - Show temperature data, 20 lines, refresh every 2s"
  echo "  bash mqtt_monitor.sh elfryd/gyro 15 5    - Show gyroscope data, 15 lines, refresh every 5s"
  echo "  bash mqtt_monitor.sh elfryd/config 10 1  - Show configuration data, 10 lines, refresh every 1s"
  echo "  bash mqtt_monitor.sh test/verification 5 3 - Show general messages from test/verification topic"
  echo ""
  exit 0
}

# Check for help flag
if [[ "$1" == "-h" || "$1" == "--help" || -z "$1" ]]; then
  print_help
fi

# Parse arguments (topic is now required)
topic_filter=$1
lines=${2:-10}
interval=${3:-1}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "jq not found. Installing..."
    sudo apt-get update && sudo apt-get install -y jq
fi

# Function to determine the endpoint based on the topic
get_endpoint() {
    local topic=$1
    
    case "$topic" in
        *"elfryd/battery"*)
            echo "/battery"
            ;;
        *"elfryd/temp"*)
            echo "/temperature"
            ;;
        *"elfryd/gyro"*)
            echo "/gyro"
            ;;
        *"elfryd/config"*)
            echo "/config"
            ;;
        *)
            echo "/messages?topic=$topic"
            ;;
    esac
}

# Function to format data based on the topic type
format_data() {
    local data=$1
    local topic=$2
    
    case "$topic" in
        *"elfryd/battery"*)
            # Format battery data
            echo "$data" | jq -r '.[] | " \(.timestamp) | Battery ID: \(.battery_id) | Voltage: \(.voltage)mV | Device TS: \(.device_timestamp)"'
            ;;
        *"elfryd/temp"*)
            # Format temperature data
            echo "$data" | jq -r '.[] | " \(.timestamp) | Sensor ID: \(.sensor_id) | Temp: \(.temperature)°C | Device TS: \(.device_timestamp)"'
            ;;
        *"elfryd/gyro"*)
            # Format gyroscope data
            echo "$data" | jq -r '.[] | " \(.timestamp) | Sensor ID: \(.sensor_id) | Accel: [\(.accel_x),\(.accel_y),\(.accel_z)] | Gyro: [\(.gyro_x),\(.gyro_y),\(.gyro_z)] | Device TS: \(.device_timestamp)"'
            ;;
        *"elfryd/config"*)
            # Format configuration data
            echo "$data" | jq -r '.[] | " \(.timestamp) | Command: \(.command) | Topic: \(.topic)"'
            ;;
        *)
            # Format general messages
            echo "$data" | jq -r '.[] | " \(.timestamp) | \(.topic) | \(.message)"'
            ;;
    esac
}

# Function to display the appropriate header based on topic
get_header() {
    local topic=$1
    
    case "$topic" in
        *"elfryd/battery"*)
            echo " Timestamp | Battery ID | Voltage | Device Timestamp"
            echo " -------------------------------------------------"
            ;;
        *"elfryd/temp"*)
            echo " Timestamp | Sensor ID | Temperature | Device Timestamp"
            echo " ----------------------------------------------------"
            ;;
        *"elfryd/gyro"*)
            echo " Timestamp | Sensor ID | Accelerometer | Gyroscope | Device Timestamp"
            echo " ----------------------------------------------------------------"
            ;;
        *"elfryd/config"*)
            echo " Timestamp | Command | Topic"
            echo " ----------------------------"
            ;;
        *)
            echo " Timestamp | Topic | Message"
            echo " ---------------------------"
            ;;
    esac
}

# Function to get the topic display name
get_topic_display_name() {
    local topic=$1
    
    case "$topic" in
        *"elfryd/battery"*)
            echo "Battery Readings"
            ;;
        *"elfryd/temp"*)
            echo "Temperature Readings"
            ;;
        *"elfryd/gyro"*)
            echo "Gyroscope Readings"
            ;;
        *"elfryd/config"*)
            echo "Configuration Commands"
            ;;
        *)
            echo "Messages: $topic"
            ;;
    esac
}

# Function to clear screen and display title and data
display_data() {
    clear
    local topic_display=$(get_topic_display_name "$topic_filter")
    
    echo " ==============================="
    echo " MQTT Monitor: $topic_display"
    echo " Lines: $lines, Refresh: ${interval}s"
    echo " Press 'q' to exit"
    echo " ==============================="
    echo " Last check: $(date '+%H:%M:%S')"
    echo
    
    # Display appropriate header based on topic type
    get_header "$topic_filter"

    if [ -z "$1" ]; then
        echo "No data available yet..."
        # Store that we displayed 1 line (the "No data" message)
        displayed_lines=1
    else
        # Parse and display the data, counting actual lines displayed
        data_lines=$(format_data "$1" "$topic_filter")
        if [ -z "$data_lines" ]; then
            echo "No matching messages found."
            displayed_lines=1
        else
            echo "$data_lines" | head -n $lines
            # Count how many lines were actually displayed
            displayed_lines=$(echo "$data_lines" | wc -l)
            # If displayed_lines exceeds our requested lines, cap it
            if [ "$displayed_lines" -gt "$lines" ]; then
                displayed_lines=$lines
            fi
        fi
    fi
    
    # Store the displayed_lines as a global variable for later cursor positioning
    export displayed_lines
}

# Use an explicit flag to control the loop
running=true

# Signal handling for clean exit
trap 'running=false' SIGINT SIGTERM

# Save current terminal settings and disable echo
old_settings=$(stty -g)
stty -echo

last_data=""
display_data # Initial empty display

# Determine which API endpoint to use based on the topic
endpoint=$(get_endpoint "$topic_filter")

while $running; do
    # Construct the appropriate API URL
    if [[ "$endpoint" == "/messages"* ]]; then
        # Already contains the query parameter
        api_url="https://${ELFRYD_HOSTNAME}:443${endpoint}&limit=$lines"
    else
        # Specialized endpoint
        api_url="https://${ELFRYD_HOSTNAME}:443${endpoint}?limit=$lines"
    fi
    
    current_data=$(curl -k -s -X GET "$api_url" -H "X-API-Key: $API_KEY")
    
    # Only update the display if data has changed
    if [ "$current_data" != "$last_data" ]; then
        display_data "$current_data"
        last_data=$current_data
    else
        # Just update the timestamp without affecting the rest of the display
        timestamp_row=5
        tput cup $timestamp_row 13  # Position cursor after "Last check: "
        echo -n "$(date '+%H:%M:%S')"
        
        # Calculate the appropriate cursor position based on actual displayed lines
        # Base position (headers) + displayed lines + 1 (for good measure)
        base_position=9  # Header lines before data
        cursor_position=$((base_position + displayed_lines + 1))
        tput cup $cursor_position 0
    fi
    
    # Check for 'q' key press with a timeout
    read -t $interval -n 1 key
    if [[ $key == "q" || $key == "Q" ]]; then
        running=false
    fi
done

# Restore terminal settings
stty "$old_settings"

echo -e "\nExiting MQTT monitor. Goodbye!"
exit 0