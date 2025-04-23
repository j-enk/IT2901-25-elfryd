#!/bin/bash

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

# Function to print help
print_help() {
    echo "Database Monitor Usage:"
    echo "  bash db_monitor.sh <table> [lines] [interval]"
    echo "  bash db_monitor.sh messages <table_name> [lines] [interval]"
    echo ""
    echo "Parameters:"
    echo "  table     - Table type to query (REQUIRED): battery, temp, gyro, config, or messages"
    echo "              For 'messages' type, you must specify the table name as the second argument"
    echo "  lines     - Number of rows to display (default: 10)"
    echo "  interval  - Refresh interval in seconds (default: 1)"
    echo ""
    echo "Examples:"
    echo "  bash db_monitor.sh battery              - Show battery data, 10 lines, refresh every 1s"
    echo "  bash db_monitor.sh temp 20 2            - Show temperature data, 20 lines, refresh every 2s"
    echo "  bash db_monitor.sh gyro 15 5            - Show gyroscope data, 15 lines, refresh every 5s"
    echo "  bash db_monitor.sh config 10 1          - Show configuration data, 10 lines, refresh every 1s"
    echo "  bash db_monitor.sh messages test_verification 15 2  - Show messages from test_verification table"
    echo ""
    exit 0
}

# Check for help flag
if [[ "$1" == "-h" || "$1" == "--help" || -z "$1" ]]; then
    print_help
fi

# Parse arguments
table_type=$1
shift # Remove first argument

# Handle the 'messages' special case
if [[ "$table_type" == "messages" ]]; then
    if [[ -z "$1" ]]; then
        print_error "When using 'messages' type, you must specify the table name as the second argument."
        print_help
    fi

    table="$1"
    shift # Remove the table name from arguments
else
    # Map table_type to actual table name for predefined types
    case "$table_type" in
    "battery")
        table="elfryd_battery"
        ;;
    "temp")
        table="elfryd_temp"
        ;;
    "gyro")
        table="elfryd_gyro"
        ;;
    "config")
        table="elfryd_config"
        ;;
    *)
        print_error "Unknown table type: $table_type"
        echo "Valid options are: battery, temp, gyro, config, or messages <table_name>"
        print_help
        ;;
    esac
fi

# Parse remaining arguments
lines=${1:-10}
interval=${2:-1}
topic_filter=${3:-""}

# Check if Docker is running and timescaledb container exists
if ! docker ps | grep -q timescaledb; then
    print_error "TimescaleDB container is not running. Please start the system first."
    exit 1
fi

# Function to check if a table exists in the database
check_table_exists() {
    local table_name=$1

    # Run SQL query to check if table exists
    result=$(docker exec -it timescaledb psql -U myuser -d mqtt_data -c "SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_schema = 'public' AND table_name = '$table_name');" -t | tr -d '[:space:]')

    if [[ "$result" == "t" ]]; then
        return 0 # Table exists
    else
        return 1 # Table doesn't exist
    fi
}

# Check if specified table exists
if ! check_table_exists "$table"; then
    print_error "Table '$table' does not exist in the database."
    echo "Available tables in the database:"
    docker exec -it timescaledb psql -U myuser -d mqtt_data -c "SELECT table_name FROM information_schema.tables WHERE table_schema = 'public' ORDER BY table_name;"
    exit 1
fi

# Function to get the query based on table type
get_query() {
    local table=$1
    local limit=$2
    local topic_filter=$3

    # Detect table schema to determine appropriate query
    local columns=$(docker exec -it timescaledb psql -U myuser -d mqtt_data -c "\d $table" | grep -E "[a-z_]+ +\|" | awk '{print $1}' | tr '\n' ' ')

    # Check for known schemas first
    case "$table" in
    "elfryd_battery")
        echo "SELECT battery_id, voltage, device_timestamp, timestamp, 
                  extract(epoch from timestamp) as server_epoch,
                  extract(epoch from timestamp) - device_timestamp::numeric as latency
                  FROM $table
                  ORDER BY timestamp DESC
                  LIMIT $limit;"
        ;;
    "elfryd_temp")
        echo "SELECT temperature, device_timestamp, timestamp, 
                  extract(epoch from timestamp) as server_epoch,
                  extract(epoch from timestamp) - device_timestamp::numeric as latency
                  FROM $table
                  ORDER BY timestamp DESC
                  LIMIT $limit;"
        ;;
    "elfryd_gyro")
        echo "SELECT accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, 
                  device_timestamp, timestamp,
                  extract(epoch from timestamp) as server_epoch,
                  extract(epoch from timestamp) - device_timestamp::numeric as latency 
                  FROM $table
                  ORDER BY timestamp DESC
                  LIMIT $limit;"
        ;;
    "elfryd_config")
        echo "SELECT command, topic, timestamp FROM $table
                  ORDER BY timestamp DESC
                  LIMIT $limit;"
        ;;
    *)
        # Query for a specific messages table
        # Check if the table has device_timestamp and timestamp
        if echo "$columns" | grep -qw "device_timestamp" && echo "$columns" | grep -qw "timestamp"; then
            echo "SELECT *, extract(epoch from timestamp) as server_epoch, 
                      extract(epoch from timestamp) - device_timestamp::numeric as latency 
                      FROM $table 
                      ORDER BY timestamp DESC 
                      LIMIT $limit;"
        # If it only has timestamp (like mqtt_messages)
        elif echo "$columns" | grep -qw "timestamp"; then
            if [ -n "$topic_filter" ]; then
                echo "SELECT * FROM $table 
                          WHERE topic LIKE '%$topic_filter%' 
                          ORDER BY timestamp DESC 
                          LIMIT $limit;"
            else
                echo "SELECT * FROM $table 
                          ORDER BY timestamp DESC 
                          LIMIT $limit;"
            fi
        # For any other schema
        else
            echo "SELECT * FROM $table 
                      LIMIT $limit;"
        fi
        ;;
    esac
}

# Function to get the appropriate header based on table
get_header() {
    local table=$1

    case "$table" in
    "elfryd_battery")
        echo -e "Battery ID\tVoltage (mV)\tDevice Time\tServer Time\t\t\tLatency (s)"
        echo -e "-----------\t------------\t-----------\t-------------------------\t----------"
        ;;
    "elfryd_temp")
        echo -e "Temperature (°C)\tDevice Time\tServer Time\t\t\tLatency (s)"
        echo -e "----------------\t-----------\t-------------------------\t----------"
        ;;
    "elfryd_gyro")
        echo -e "Accel [X,Y,Z]\t\tGyro [X,Y,Z]\t\t\tDevice Time\tServer Time\t\t\tLatency (s)"
        echo -e "----------------\t-------------------------\t-----------\t-------------------------\t----------"
        ;;
    "elfryd_config")
        echo -e "Command\t\t\tTopic\t\t\tTimestamp"
        echo -e "-------\t\t\t-----\t\t\t---------"
        ;;
    *)
        # Print column names from database for the specific messages table
        echo -e "$(docker exec -it timescaledb psql -U myuser -d mqtt_data -c "\\d $table" | grep -E "[a-z_]+ +\\|" | awk '{print $1}' | tr '\n' '\t')"
        echo -e "$(printf -- '-%.0s' {1..80})"
        ;;
    esac
}

# Function to get the table display name
get_table_display_name() {
    local table=$1
    local table_type=$2

    case "$table" in
    "elfryd_battery")
        echo "Battery Readings (with Server Timestamps)"
        ;;
    "elfryd_temp")
        echo "Temperature Readings (with Server Timestamps)"
        ;;
    "elfryd_gyro")
        echo "Gyroscope Readings (with Server Timestamps)"
        ;;
    "elfryd_config")
        echo "Configuration Commands"
        ;;
    *)
        if [[ "$table_type" == "messages" ]]; then
            echo "Messages Table: $table"
        else
            echo "Table: $table"
        fi
        ;;
    esac
}

# Function to clear screen and display data
display_data() {
    clear
    local table_display=$(get_table_display_name "$table" "$table_type")

    echo -e "${GREEN}===============================${NC}"
    echo -e "${GREEN}DB Monitor: $table_display${NC}"
    echo -e "${GREEN}Lines: $lines, Refresh: ${interval}s${NC}"
    echo -e "${GREEN}Press 'q' to exit${NC}"
    echo -e "${GREEN}===============================${NC}"
    echo -e "${YELLOW}Last check: $(date '+%H:%M:%S')${NC}"
    echo

    # Display appropriate header based on table type
    get_header "$table"

    if [ -z "$1" ]; then
        echo "No data available yet..."
        displayed_lines=1
    else
        echo -e "$1"
        # Count displayed lines (assuming data already formatted properly)
        displayed_lines=$(echo "$1" | wc -l)
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

while $running; do
    # Get the query based on table type
    query=$(get_query "$table" "$lines" "$topic_filter")

    # Execute the query in the database container
    current_data=$(docker exec -it timescaledb psql -U myuser -d mqtt_data -c "$query" -t)

    # Only update the display if data has changed
    if [ "$current_data" != "$last_data" ]; then
        display_data "$current_data"
        last_data=$current_data
    else
        # Just update the timestamp without affecting the rest of the display
        timestamp_row=5
        tput cup $timestamp_row 12 # Position cursor after "Last check: "
        echo -n "$(date '+%H:%M:%S')"

        # Calculate the appropriate cursor position based on actual displayed lines
        base_position=8 # Header lines before data
        cursor_position=$((base_position + displayed_lines + 1))
        tput cup $cursor_position 0
    fi

    # Check for 'q' key press with a timeout
    read -t $interval -n 1 key 2>/dev/null || true
    if [[ $key == "q" || $key == "Q" ]]; then
        running=false
    fi
done

# Restore terminal settings
stty "$old_settings"

echo -e "\nExiting database monitor. Goodbye!"
exit 0
