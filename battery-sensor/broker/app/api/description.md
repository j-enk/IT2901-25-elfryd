API for the Elfryd IoT platform providing access to MQTT message data from device sensors.
    
## Features
    
- **Sensor Data**: Access battery, temperature, and gyroscope readings
- **Configuration**: Send and retrieve device configuration commands
- **Messaging**: Access to all MQTT messages with topic filtering
- **Security**: API key authentication for protected endpoints
    
## Authentication
    
Most endpoints require an API key passed in the `X-API-Key` header.
The API key is generated on API/broker installation and should be kept secret.
    
## Time-Based Querying
    
All data endpoints support time-based querying with these parameters:
- **hours**: Time window length in hours (e.g., 24 = last 24 hours of data)
- **time_offset**: Start the time window this many hours in the past (e.g., 336 = 2 weeks ago)
    
These can be combined to view historical data windows (e.g., hours=24, time_offset=336 to see data from exactly 2 weeks ago for a 24-hour period).