import os
from fastapi.security.api_key import APIKeyHeader

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

# API configuration
API_CONFIG = {
    "title": "Elfryd MQTT API",
    "description": "API for interacting with MQTT broker and retrieving stored messages",
    "version": "1.0.0",
    "openapi_tags": [
        {
            "name": "Authentication",
            "description": "Requires X-API-Key header for protected endpoints",
        }
    ]
}

# Authentication
API_KEY = os.getenv("API_KEY")
if not API_KEY:
    raise ValueError("API_KEY environment variable not set")

api_key_header = APIKeyHeader(name="X-API-Key", auto_error=False)

# Constants for table naming
DEFAULT_TABLE = "mqtt_messages"

# Regex patterns
CONFIG_FREQ_PATTERN = r"^freq\d+$"
VALID_CONFIG_COMMANDS = ["battery", "temp", "gyro"]