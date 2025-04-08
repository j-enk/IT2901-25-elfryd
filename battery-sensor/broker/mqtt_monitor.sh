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

# Default values
lines=${1:-10}
interval=${2:-1}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "jq not found. Installing..."
    sudo apt-get update && sudo apt-get install -y jq
fi

# Function to clear screen and display title and data
display_data() {
    clear
    echo "==============================="
    echo "  MQTT Message Monitor"
    echo "  Lines: $lines, Refresh: ${interval}s"
    echo "  Press 'q' to exit"
    echo "==============================="
    echo "Last check: $(date '+%H:%M:%S')"
    echo

    if [ -z "$1" ]; then
        echo "No data available yet..."
    else
        echo "$1" | jq -r '.[] | "\(.timestamp) | \(.topic) | \(.message)"' | head -n $lines
    fi
}

# Use an explicit flag to control the loop
running=true

# Signal handling for clean exit
trap 'running=false' SIGINT SIGTERM

last_data=""
display_data # Initial empty display

while $running; do
    current_data=$(curl -k -s -X GET "https://${ELFRYD_HOSTNAME}:443/messages" \
         -H "X-API-Key: $API_KEY")
    
    # Only update the display if data has changed
    if [ "$current_data" != "$last_data" ]; then
        display_data "$current_data"
        last_data=$current_data
    else
        # Just update the timestamp without affecting the rest of the display
        tput cup 5 12  # Position cursor at row 5, column 12 (after "Last check: ")
        echo -n "$(date '+%H:%M:%S')"
        tput cup $((lines + 8)) 0  # Move cursor to the bottom of the display area
    fi
    
    # Check for 'q' key press with a timeout
    read -t $interval -n 1 key
    if [[ $key == "q" || $key == "Q" ]]; then
        running=false
    fi
done

echo -e "\nExiting MQTT monitor. Goodbye!"
exit 0