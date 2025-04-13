# API Documentation

This document explains the FastAPI-based REST API that provides access to the MQTT message data stored in the TimescaleDB database.

## API Overview

The Elfryd broker API provides a secure HTTPS interface to access sensor data, configuration commands, and MQTT messages stored in the system. It's designed for:

- Backend applications to retrieve sensor data
- Administration interfaces to monitor the system
- Development tools to test and debug the IoT infrastructure
- Sending configuration commands to devices

## Authentication

Most API endpoints are protected with an API key that is generated during installation (see [VM Setup](vm_setup.md) and [Scripts](scripts.md) documentation). The API key is required for all endpoints except the health check (`/health`), which is publicly accessible to verify system status. To access protected endpoints, include the API key in the `X-API-Key` header with your requests:

```bash
curl -k -X GET https://your-vm-dns-name:443/battery -H "X-API-Key: YOUR_API_KEY"
```

The API key is stored in the `.env` file inside the app directory on the VM, and is also shown at the end of the installation process.

## Interactive Documentation

Full interactive API documentation is available at:

- **Swagger UI**: `https://your-vm-dns-name:443/docs`
- **ReDoc**: `https://your-vm-dns-name:443/redoc`

These interfaces provide a complete reference of all endpoints, parameters, request/response schemas, and example usage. As long as you have the API key, you can test the endpoints directly from the documentation interface.

## API Endpoints

### Core Endpoints

| Endpoint    | Method | Description                  | Authentication |
|-------------|--------|------------------------------|----------------|
| `/health`   | GET    | Check system health status   | Public         |
| `/messages` | GET    | Get stored messages by topic | API Key        |
| `/messages` | POST   | Publish a message to MQTT    | API Key        |
| `/topics`   | GET    | Get list of unique topics    | API Key        |

### Sensor Data Endpoints

| Endpoint       | Method | Description                      | Authentication |
|----------------|--------|----------------------------------|---------------|
| `/battery`     | GET    | Retrieve battery sensor data     | API Key       |
| `/temperature` | GET    | Retrieve temperature sensor data | API Key       |
| `/gyro`        | GET    | Retrieve gyroscope sensor data   | API Key       |

### Configuration Endpoints

| Endpoint       | Method | Description                | Authentication |
|----------------|--------|----------------------------|---------------|
| `/config`      | GET    | Get configuration messages | API Key       |
| `/config/send` | POST   | Send configuration command | API Key       |

## Common Query Parameters

Most data retrieval endpoints support these common parameters:

- `limit`: Maximum number of records to return
- `hours`: Get data from the last X hours
- `time_offset`: Offset in hours from current time

These parameters can be combined to create specific time windows. For example, setting `hours=24` and `time_offset=336` would retrieve data from exactly 2 weeks ago for a 24-hour period.

## Endpoint Details

### Health Check

```
GET /health
```

Returns the current health status of the system components.

**Response Example:**
```json
{
  "status": "healthy",
  "database": "connected",
  "mqtt": "connected"
}
```

### Battery Data

```
GET /battery
```

Retrieves battery voltage measurements.

**Parameters:**
- `battery_id` (optional): Filter by specific battery ID
- `limit` (default: 20): Maximum number of records
- `hours` (default: 168): Data from last X hours
- `time_offset` (optional): Hours offset from current time

**Response Example:**
```json
[
  {
    "id": 123,
    "battery_id": 1,
    "voltage": 8432,
    "device_timestamp": 1680123456
  }
]
```

### Temperature Data

```
GET /temperature
```

Retrieves temperature measurements.

**Parameters:**
- `limit` (default: 20): Maximum number of records
- `hours` (default: 168): Data from last X hours
- `time_offset` (optional): Hours offset from current time

**Response Example:**
```json
[
  {
    "id": 456,
    "temperature": 25,
    "device_timestamp": 1680123730
  }
]
```

### Gyroscope Data

```
GET /gyro
```

Retrieves gyroscope and accelerometer measurements.

**Parameters:**
- `limit` (default: 20): Maximum number of records
- `hours` (default: 168): Data from last X hours
- `time_offset` (optional): Hours offset from current time

**Response Example:**
```json
[
  {
    "id": 789,
    "accel_x": -4991017,
    "accel_y": -4984009,
    "accel_z": 4979460,
    "gyro_x": -239841,
    "gyro_y": 241869,
    "gyro_z": -243303,
    "device_timestamp": 1680123803
  }
]
```

### Configuration Data

```
GET /config
```

Retrieves configuration commands sent to devices.

**Parameters:**
- `limit` (default: 20): Maximum number of records
- `hours` (default: 168): Data from last X hours
- `time_offset` (optional): Hours offset from current time

**Response Example:**
```json
[
  {
    "id": 1,
    "command": "battery 10",
    "topic": "elfryd/config/send",
    "timestamp": "2025-04-10T14:23:45.123456"
  }
]
```

### Send Configuration Command

```
POST /config/send
```

Sends a configuration command to connected devices.

**Request Body:**
```json
{
  "command": "battery 10"
}
```

**Response Example:**
```json
{
  "success": true,
  "message": "Configuration commands sent",
  "topic": "elfryd/config/send",
  "commands": ["battery 10"]
}
```

### Get Messages

```
GET /messages?topic=test/topic
```

Retrieves general MQTT messages filtered by topic.

**Parameters:**
- `topic` (required): Filter by topic (partial match)
- `limit` (default: 100): Maximum number of records
- `offset` (default: 0): Number of records to skip
- `hours` (default: 24): Data from last X hours
- `time_offset` (optional): Hours offset from current time

**Response Example:**
```json
[
  {
    "id": 42,
    "topic": "test/topic",
    "message": "This is a test message",
    "timestamp": "2025-04-10T14:23:45.123456"
  }
]
```

### Publish Message

```
POST /messages
```

Publishes a message to a specific MQTT topic. 

> **Note**: Sensor data topics (elfryd/{sensor-type}) are blacklisted and will not be accepted.

**Request Body:**
```json
{
  "topic": "test/topic",
  "message": "Hello, world!"
}
```

**Response Example:**
```json
{
  "success": true,
  "topic": "test/topic"
}
```

### Get Topics

```
GET /topics
```

Returns a list of all unique topics in the database.

**Response Example:**
```json
{
  "topics": [
    "elfryd/battery",
    "elfryd/temp",
    "test/verification",
    "elfryd/config/send"
  ]
}
```

## Code Structure

The API code follows a modular structure:

- `app.py`: Main application entry point
- `dependencies.py`: Shared dependencies and configuration
- `routes/`: Directory containing endpoint modules by feature
  - `battery.py`: Battery data endpoints
  - `temperature.py`: Temperature data endpoints
  - `gyro.py`: Gyroscope data endpoints
  - `config.py`: Configuration endpoints
  - `messages.py`: General message endpoints
  - `topics.py`: Topic listing endpoints
  - `health.py`: System health endpoints

## Error Handling

The API returns standard HTTP error codes:

- **400**: Bad Request - Invalid parameters or request format
- **401**: Unauthorized - Missing API key
- **403**: Forbidden - Invalid API key
- **404**: Not Found - Resource not found
- **500**: Internal Server Error - Server-side issue

Error responses include a detail message explaining the issue.

## Usage Examples

### Python Client Example

```python
import requests

API_URL = "https://your-vm-hostname:443"
API_KEY = "your-api-key"

headers = {
    "X-API-Key": API_KEY
}

# Get battery data
response = requests.get(
    f"{API_URL}/battery",
    headers=headers,
    params={"hours": 24, "limit": 5},
    verify=False  # Skip TLS verification
)

if response.status_code == 200:
    batteries = response.json()
    for battery in batteries:
        print(f"Battery {battery['battery_id']}: {battery['voltage']} mV")
else:
    print(f"Error: {response.status_code}, {response.text}")
```

### Curl Examples

Get latest battery readings:
```bash
curl -k -X GET "https://your-vm-hostname:443/battery?limit=5" -H "X-API-Key: your-api-key"
```

Send configuration command:
```bash
curl -k -X POST "https://your-vm-hostname:443/config/send" \
  -H "Content-Type: application/json" \
  -H "X-API-Key: your-api-key" \
  -d '{"command": "battery 10"}'
```

Get system health:
```bash
curl -k -X GET "https://your-vm-hostname:443/health"
```

List all available topics:
```bash
curl -k -X GET "https://your-vm-hostname:443/topics" -H "X-API-Key: your-api-key"
```

## Further Reading

For more details on the data format and topic structure, see the [Bridge Documentation](bridge.md).