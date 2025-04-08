#!/bin/bash

# Use an explicit flag to control the loop
running=true

# Signal handling for clean exit
trap 'running=false' SIGINT SIGTERM

# Get command line arguments or use defaults
lines=${1:-10}
interval=${2:-1}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "Error: jq is required but not installed. Please install it first."
    echo "You can install it with: sudo apt-get install jq"
    exit 1
fi

echo "Starting MQTT message monitor. Press 'q' to exit."
echo "Showing up to $lines messages every $interval seconds."

last_data=""
while $running; do 
  current_data=$(curl -s "http://localhost:443/messages")
  if [ "$current_data" != "$last_data" ]; then 
    clear
    echo "MQTT message monitor started at $(date '+%H:%M:%S'). Press 'q' to exit."
    echo "Showing up to $lines messages every $interval seconds."
    echo "$current_data" | jq -r '.[] | "\(.timestamp) | \(.topic) | \(.message)"' | head -n $lines
    last_data=$current_data
  else 
    echo -ne "\rLast check: $(date '+%H:%M:%S') - No changes"
  fi
  
  # Check for 'q' key press with a timeout
  read -t $interval -n 1 key
  if [[ $key == "q" || $key == "Q" ]]; then
    running=false
  fi
done

echo -e "\nExiting MQTT monitor. Goodbye!"
exit 0