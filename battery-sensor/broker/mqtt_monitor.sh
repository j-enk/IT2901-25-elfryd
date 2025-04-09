#!/bin/bash

# Load API key from environment file
if [ -f "./app/.env" ]; then
  source ./app/.env
else
  echo "Error: API key file not found. Please run install.sh first."
  exit 1
fi

# Get hostname from environment file
if [ -f /etc/elfryd/elfryd.env ]; then
  source /etc/elfryd/elfryd.env
else
  echo "Warning: Environment file not found, using default hostname"
  ELFRYD_HOSTNAME="localhost"  # Fallback
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
    else
        echo "$1" | jq -r '.[] | " \(.timestamp) | \(.topic) | \(.message)"' | head -n $lines
    fi
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
        tput cup 5 13  # Position cursor at row 5, column 12 (after "Last check: ")
        echo -n "$(date '+%H:%M:%S')"
        tput cup $((lines + 11)) 0  # Move cursor to the bottom of the display area
    fi
    
    # Check for 'q' key press with a timeout
    read -t $interval -n 1 key
    if [[ $key == "q" || $key == "Q" ]]; then
        running=false
    fi
done

# Restore terminal settings
stty "$old_settings"

echo -e "Exiting MQTT monitor. Goodbye!"
exit 0