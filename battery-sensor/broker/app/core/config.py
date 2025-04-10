import os

# Database connection parameters - only accessible within Docker network
DB_CONFIG = {
    "host": "timescaledb",
    "port": 5432,
    "dbname": "mqtt_data",
    "user": "myuser",
    "password": "mypassword"
}

# MQTT broker parameters
MQTT_CONFIG = {
    "broker": "mqtt-broker",
    "port": 1883,
    "tls_port": 8885,
    "use_tls": os.environ.get("USE_TLS", "false").lower() == "true",
    "default_topic": "#"
}

# Constants for table naming
DEFAULT_TABLE = "mqtt_messages"

# List of valid configuration commands
VALID_CONFIG_COMMANDS = ["battery", "temp", "gyro"]