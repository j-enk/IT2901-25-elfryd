#!/bin/bash

# Load API key from environment file
if [ -f "./app/.env" ]; then
  source ./app/.env
else
  echo "Error: API key file not found. Please run install.sh first."
  exit 1
fi

# Default values
LINES=${1:-10}
INTERVAL=${2:-1}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "jq not found. Installing..."
    sudo apt-get update && sudo apt-get install -y jq
fi


# Function to clear screen and display title
display_title() {
    clear
    echo "==============================="
    echo "  MQTT Message Monitor"
    echo "  Lines: $LINES, Refresh: ${INTERVAL}s"
    echo "  Press 'q' to exit"
    echo "==============================="
    echo
}

# last_data=""
# while true; do 
#   current_data=$(curl -s "http://localhost:443/messages")
#   if [ "$current_data" != "$last_data" ]; then 
#     clear
#     echo "MQTT message monitor started at $(date '+%H:%M:%S'). Press 'q' to exit."
#     echo "Showing up to $lines messages every $interval seconds."
#     echo "$current_data" | jq -r '.[] | "\(.timestamp) | \(.topic) | \(.message)"' | head -n $lines
#     last_data=$current_data
#   else 
#     echo -ne "\rLast check: $(date '+%H:%M:%S') - No changes"
#   fi
  
#   # Check for 'q' key press with a timeout
#   read -t $interval -n 1 key
#   if [[ $key == "q" || $key == "Q" ]]; then
#     running=false
#   fi
# done

# Main monitoring loop
while true; do
    display_title
    
    # Get last messages using API with API key
    curl -k -s "https://localhost:443/messages?limit=$LINES" \
         -H "X-API-Key: $API_KEY" | \
    jq -r '.[] | "\(.timestamp) - \(.topic): \(.payload)"' | \
    sort -r
    
    sleep $INTERVAL
done

echo -e "\nExiting MQTT monitor. Goodbye!"
exit 0