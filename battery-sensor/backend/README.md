# Elfryd Battery Sensor API

The Elfryd Battery Sensor API is a .NET-based REST API that provides a simplified interface for accessing the Elfryd MQTT broker data. It serves as a middleware layer between client applications and the underlying MQTT data infrastructure, providing well-defined endpoints for battery voltage, temperature, gyroscope, and system configuration.

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#overview">Overview</a></li>
    <li><a href="#system-architecture">System Architecture</a></li>
    <li><a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#environment-setup">Environment Setup</a></li>
        <li><a href="#building-and-running">Building and Running</a></li>
      </ul>
    </li>
    <li><a href="#api-documentation">API Documentation</a>
      <ul>
        <li><a href="#battery-endpoints">Battery Endpoints</a></li>
        <li><a href="#temperature-endpoints">Temperature Endpoints</a></li>
        <li><a href="#gyroscope-endpoints">Gyroscope Endpoints</a></li>
        <li><a href="#configuration-endpoints">Configuration Endpoints</a></li>
      </ul>
    </li>
    <li><a href="#authentication">Authentication</a></li>
    <li><a href="#error-handling">Error Handling</a></li>
    <li><a href="#testing">Testing</a></li>
    <li><a href="#support-and-contribution">Support and Contribution</a></li>
  </ol>
</details>

## Overview

The BatterySensorAPI is designed to simplify integration with the Elfryd MQTT broker system for client applications and completing necessary calculations for the frontend display. 

Key features include:
- Abstraction of MQTT communication details
- Standardized REST interface
- Support for filtering and querying data
- Battery state-of-charge calculation
- Centralized error handling and logging

## System Architecture

The BatterySensorAPI is built using ASP.NET Core and follows a clean architecture pattern:

```
BatterySensorAPI/
├── Controllers/         # API endpoints handling HTTP requests
├── Data/                # Data access and manipulation logic
│   ├── Models/          # Domain models representing business entities
│   └── Services/        # Services implementing business logic
├── Properties/          # Project settings and launch configurations
└── Program.cs          # Application entry point and dependency configuration
```

The API communicates with the [Elfryd MQTT broker system](../broker/README.md) through HTTP requests, retrieving data from and sending commands to the broker.

## Getting Started

### Prerequisites

- [.NET 9.0 SDK](https://dotnet.microsoft.com/download/dotnet/9.0)
- Access to the Elfryd MQTT broker system
- API key for the Elfryd MQTT broker. [Retrieved here](../broker/docs/api.md#authentication)

### Environment Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/j-enk/IT2901-25-elfryd.git
   cd IT2901-25-elfryd/battery-sensor/backend
   ```

2. Duplicate the `.env.example` file in the BatterySensorAPI directory. Rename it to `.env`  and insert the necessary information according to the following configuration:
   ```
   HOST_NAME=your-broker-hostname
   API_KEY=your-api-key
   ```

### Building and Running

1. Build the project:
   ```bash
   cd BatterySensorAPI
   dotnet build
   ```

2. Run the API:
   ```bash
   dotnet run
   ```

3. The API will be available at:
   - HTTP: http://localhost:5196

4. Access Swagger documentation at http://localhost:5196/swagger/index.html when running in development mode.

## API Documentation

The BatterySensorAPI provides endpoints to access battery data, temperature readings, gyroscope data, and configuration settings from the Elfryd MQTT broker.

### Battery Endpoints

#### Get Battery Data

```
GET /api/elfryd/battery
```

Retrieves battery voltage measurements from connected devices.

**Parameters:**
- `battery_id` (optional): Filter by specific battery ID, 0 to retrieve all.
- `limit` (default: 20, max: 1000000): Maximum number of records to return. When used with the hours parameter, data points will be evenly distributed across the time range instead of just returning the newest records. Set to 0 to disable limiting and return all data points in the time range.
- `hours` (default: 168): Get data from the last X hours
- `time_offset` (default: 0): Offset in hours from current time (e.g., 336 = start from 2 weeks ago)

**Response Example:**
```json
[
  {
    "id": 123,
    "battery_id": 1,
    "voltage": 12524,
    "device_timestamp": 1712841632
  }
]
```

#### Get System State of Charge

```
GET /api/elfryd/battery/system/soc
```

Calculates and returns the average state-of-charge across all batteries in the system.

**Parameters:**
- `minVoltage` (optional, default: 12000): Minimum voltage value in millivolts representing 0% charge
- `maxVoltage` (optional, default: 15000): Maximum voltage value in millivolts representing 100% charge

**Response Example:**
```json
75
```

#### Get Individual Battery State of Charge

```
GET /api/elfryd/battery/individual/soc
```

**Parameters:**
- `battery_id` (optional, default: 0): Filter by specific battery ID
- `minVoltage` (optional, default: 12000): Minimum voltage value in millivolts representing 0% charge
- `maxVoltage` (optional, default: 15000): Maximum voltage value in millivolts representing 100% charge

**Response Example:**
```json
[
  {
    "battery_id": 1,
    "soc": 84
  },
  {
    "battery_id": 2,
    "soc": 76
  }
]
```

### Temperature Endpoints

#### Get Temperature Data

```
GET /api/elfryd/temp
```

Retrieves temperature measurements.

**Parameters:**
- `limit` (default: 20, max: 1000000): Maximum number of records to return (default: 100, max: 10000). When used with the hours parameter, data points will be evenly distributed across the time range instead of just returning the newest records. Set to 0 to disable limiting and return all data points in the time range.
- `hours` (default: 168): Get data from the last X hours
- `time_offset` (default: 0): Offset in hours from current time (e.g., 336 = start from 2 weeks ago)

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

### Gyroscope Endpoints

#### Get Gyroscope Data

```
GET /api/elfryd/gyro
```

Retrieves gyroscope and accelerometer measurements.

**Parameters:**
- `limit` (default: 20, max: 1000000): Maximum number of records to return . When used with the hours parameter, data points will be evenly distributed across the time range instead of just returning the newest records. Set to 0 to disable limiting and return all data points in the time range.
- `hours` (default: 168): Get data from the last X hours
- `time_offset` (default: 0): Offset in hours from current time (e.g., 336 = start from 2 weeks ago)

**Response Example:**
```json
[
  {
    "device_timestamp": 1745514713,
    "ax": 2.027653,
    "ay": -1.789282,
    "az": -4.599922,
    "gx": -15.489,
    "gy": -120.072,
    "gz": 155.29,
    "roll": -158.74491316601845,
    "pitch": -22.33366139553019,
    "yawRate": 155.29,
    "heave": 4.335933767394138,
    "id": 828
  }
]
```

### Configuration Endpoints

#### Get Configuration Data

```
GET /api/elfryd/config
```

Retrieves configuration commands sent to devices.

**Parameters:**
- `limit` (default: 20, max: 10000): Maximum number of records to return. When used with the hours parameter, data points will be evenly distributed across the time range instead of just returning the newest records. Set to 0 to disable limiting and return all data points in the time range.
- `hours` (default: 168): Get data from the last X hours
- `time_offset` (default: 0): Offset in hours from current time (e.g., 336 = start from 2 weeks ago)

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

#### Send Configuration Command

```
POST /api/elfryd/config/update
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
  "result": "{\"success\":true,\"message\":\"Configuration commands sent\",\"topic\":\"elfryd/config/send\",\"commands\":[\"battery 10\"]}"
}
```

## Authentication

The BatterySensorAPI itself does not implement authentication. However, it does require an API key to communicate with the Elfryd MQTT broker, which is provided through the `API_KEY` environment variable.

## Error Handling

The API implements a consistent error handling approach:

- **400 Bad Request**: Invalid parameters or request format
- **500 Internal Server Error**: Server-side issue or communication problem with the Elfryd broker

Error responses include a descriptive message explaining the issue.

## Testing

The BatterySensorAPI includes a suite of unit tests in the `BatterySensorAPI.Tests` project. These tests verify the functionality of the controllers and ensure that the API behaves correctly under various conditions.

To run the tests:

```bash
cd BatterySensorAPI.Tests
dotnet test
```

The tests use Moq for mocking dependencies, allowing for isolated testing of controllers without requiring a connection to the actual Elfryd MQTT broker.

## Support and Contribution

For questions or issues, please contact the contributors of the backend system:

- Magnus Hansen Åsheim (main developer)
- Sigve Fehn Kulsrud (contributor)
- Martin Vågseter Jakobsen (contributor)
