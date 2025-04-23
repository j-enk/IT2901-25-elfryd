import os

# Database connection parameters - only accessible within Docker network
DB_CONFIG = {
    "host": "timescaledb",
    "port": 5432,
    "dbname": "mqtt_data",
    "user": "myuser",
    "password": "mypassword",
}

# MQTT broker parameters
MQTT_CONFIG = {
    "broker": "mqtt-broker",
    "tls_broker": os.environ.get("ELFRYD_HOSTNAME", "mqtt-broker"),
    "port": 1883,
    "tls_port": 8885,
    "default_topic": "#",
}

# Base configuration commands
BASE_CONFIG_COMMANDS = ["battery", "temp", "gyro"]

# Regex patterns for valid command formats
COMMAND_PATTERNS = {
    "basic": r"^(battery|temp|gyro)$",  # basic commands: battery, temp, gyro
    "interval": r"^(battery|temp|gyro) \d+$",  # interval commands: battery 10, temp 5, gyro 20
}
