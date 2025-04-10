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
  echo "  ./mqtt_monitor.sh [lines] [interval] [topic]"
  echo ""
  echo "Parameters:"
  echo "  lines     - Number of messages to display (default: 10)"
  echo "  interval  - Refresh interval in seconds (default: 1)"
  echo "  topic     - Filter messages by topic (optional)"
  echo ""
  echo "Examples:"
  echo "  ./mqtt_monitor.sh                  - Show last 10 messages, refresh every 1s"
  echo "  ./mqtt_monitor.sh 20 2             - Show last 20 messages, refresh every 2s"
  echo "  ./mqtt_monitor.sh 10 1 sensor/temp - Show only messages from sensor/temp topic"
  echo "  ./mqtt_monitor.sh 20 2 device/     - Show messages whose topic starts with device/"
  echo ""
  exit 0
}

# Check for help flag
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
  print_help
fi

# Default values
lines=${1:-10}
interval=${2:-1}
topic_filter=${3:-""}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "jq not found. Installing..."
    sudo apt-get update && sudo apt-get install -y jq
fi

# Function to clear screen and display title and data
display_data() {
    clear
    echo " ==============================="
    echo " MQTT Message Monitor"
    echo " Lines: $lines, Refresh: ${interval}s"
    if [ -n "$topic_filter" ]; then
        echo " Topic filter: $topic_filter"
    fi
    echo " Press 'q' to exit"
    echo " ==============================="
    echo " Last check: $(date '+%H:%M:%S')"
    echo
    echo " ---------------------------"
    echo " Timestamp | Topic | Message"
    echo " ---------------------------"

    if [ -z "$1" ]; then
        echo "No data available yet..."
        # Store that we displayed 1 line (the "No data" message)
        displayed_lines=1
    else
        # Parse and display the data, counting actual lines displayed
        data_lines=$(echo "$1" | jq -r '.[] | " \(.timestamp) | \(.topic) | \(.message)"')
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

# Calculate base header offset (add 1 if we have a topic filter)
header_offset=0
if [ -n "$topic_filter" ]; then
    header_offset=1
fi

while $running; do
    # Construct API URL with optional topic filter
    api_url="https://${ELFRYD_HOSTNAME}:443/messages?limit=$lines"
    if [ -n "$topic_filter" ]; then
        api_url="${api_url}&topic=${topic_filter}"
    fi
    
    current_data=$(curl -k -s -X GET "$api_url" -H "X-API-Key: $API_KEY")
    
    # Only update the display if data has changed
    if [ "$current_data" != "$last_data" ]; then
        display_data "$current_data"
        last_data=$current_data
    else
        # Just update the timestamp without affecting the rest of the display
        # Adjust the row position based on whether we have a topic filter
        timestamp_row=$((5 + header_offset))
        tput cup $timestamp_row 13  # Position cursor after "Last check: "
        echo -n "$(date '+%H:%M:%S')"
        
        # Calculate the appropriate cursor position based on actual displayed lines
        # Base position (headers) + displayed lines + 1 (for good measure)
        base_position=$((9 + header_offset))  # Header lines before data
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