# Future Improvements and Extensibility

This document outlines potential future improvements to the Elfryd MQTT broker system and provides guidance on how to extend the system with new sensor types or capabilities.

## System Improvement Opportunities

### Security Enhancements

While the current system implements several security measures, the following improvements could further strengthen the overall security posture:

1. **Database Credential Management**

   - Current implementation: Database credentials are hardcoded in the Docker Compose file, and Docker secrets are not used
   - Improvement: Implement a secrets management solution like HashiCorp Vault or Docker secrets
   - Benefit: Better protection of database credentials, even within the Docker environment

2. **Certificate Rotation**

   - Current implementation: TLS certificates are generated during installation
   - Improvement: Implement automatic certificate rotation and renewal
   - Benefit: Reduced risk from compromised certificates and adherence to security best practices

3. **Role-Based Access Control (RBAC)**

   - Current implementation: Single API key for all API access
   - Improvement: Implement multiple API keys with different permission levels
   - Benefit: More granular control over who can access what data or functionality

4. **Rate Limiting and Throttling**
   - Current implementation: No request rate limiting
   - Improvement: Implement API rate limiting to prevent abuse (consider nginx or API gateway solutions)
   - Benefit: Protection against DoS attacks and API abuse

### Performance Optimizations

1. **Database Partitioning**

   - Current implementation: Standard TimescaleDB tables
   - Improvement: Implement TimescaleDB hypertables with automatic partitioning
   - Benefit: Better query performance for time-series data, especially as data volume grows

2. **Data Retention Policies**

   - Current implementation: No automatic data cleanup
   - Improvement: Implement data retention policies to automatically archive or delete old data
   - Benefit: Controlled database growth and improved performance

3. **Caching Layer**
   - Current implementation: Direct database queries for all requests
   - Improvement: Add Redis or similar caching for frequently accessed data
   - Benefit: Reduced database load and improved API response times

### Monitoring and Observability

1. **System Metrics**

   - Current implementation: Basic health check endpoint
   - Improvement: Implement comprehensive system metrics with Prometheus/Grafana
   - Benefit: Better visibility into system performance and early warning of issues

2. **Logging Infrastructure**

   - Current implementation: Console logging
   - Improvement: Implement structured logging with centralized log storage and analysis
   - Benefit: Easier troubleshooting and better visibility into system behavior

3. **Alerting System**
   - Current implementation: No automated alerting
   - Improvement: Set up alerts for system issues and anomalous data
   - Benefit: Faster response to problems and potential data anomalies

## Adding New Sensor Types

The Elfryd system is designed to be extensible, allowing for the addition of new sensor types as requirements evolve. This section outlines the process for adding support for a new sensor type, such as a current sensor measuring electricity in amperes.

### Current Data Flow

When a new sensor type is added to the Elfryd boat, the data can still be published to the MQTT broker in one of two ways:

1. **Via MQTT TLS Connection**

   - As long as the device has the correct TLS client certificates, it can publish to any topic
   - Data will be stored in a generic format in the database (topic, message, timestamp)
   - No specialized processing or interpretation will be done

2. **Via the API**
   - Using the `/messages` POST endpoint with a valid API key
   - Also stores data in generic format without specialized processing

### Extending the System for New Sensor Types

To fully support a new sensor type with specialized processing and API endpoints, the following steps are required:

#### 1. Define the Topic and Message Format

Define a standardized format for the new sensor data. For example, an ampere sensor might use:

**Topic**: `elfryd/ampere`  
**Message Format**: `{sensor_id}/{current_value}/{timestamp}`  
**Example**: `1/5600/1680123456` (Sensor 1, 5.6 amperes, Unix timestamp)

#### 2. Update the Bridge

1. **Create a Specialized Handler**

   ```bash
   # Create a new handler file
   touch app/bridge/handlers/ampere_handler.py
   ```

2. **Implement the Handler**
   The handler should:

   - Parse the message format
   - Validate the data
   - Store it in a specialized table

   Example implementation:

   ```python
   # app/bridge/handlers/ampere_handler.py
   from pydantic import ValidationError
   from psycopg2 import sql
   from core.models import AmpereData  # Needs to be created
   from core.database import get_connection

   def process_message(payload: str):
       """Process and store ampere data from string format"""
       try:
           # Parse payload: "sensor_id/current_value/timestamp"
           parts = payload.strip().split("/")
           if len(parts) != 3:
               print(f"Invalid ampere data format: {payload}")
               return

           # Create AmpereData model
           ampere_data = AmpereData(
               sensor_id=int(parts[0]),
               current=int(parts[1]),  # Current in milliamperes
               device_timestamp=int(parts[2])
           )

           # Store in database
           store_ampere_data(ampere_data)

       except Exception as e:
           print(f"Error processing ampere message: {str(e)}")

   def store_ampere_data(data: AmpereData):
       """Store validated ampere data in database"""
       try:
           conn = get_connection()
           cursor = conn.cursor()

           insert_query = sql.SQL(
               """
               INSERT INTO {} (sensor_id, current, device_timestamp)
               VALUES (%s, %s, %s)
               """
           ).format(sql.Identifier("elfryd_ampere"))

           cursor.execute(
               insert_query,
               (data.sensor_id, data.current, data.device_timestamp)
           )
           conn.commit()
           cursor.close()
           conn.close()

       except Exception as e:
           print(f"Error storing ampere data: {str(e)}")
   ```

3. **Update the Bridge Core**
   Modify `mqtt_bridge.py` to recognize the new topic and use the specialized handler:

   ```python
   # In mqtt_bridge.py, update imports
   from bridge.handlers import (
       battery_handler,
       temperature_handler,
       gyro_handler,
       config_handler,
       ampere_handler,  # Add the new handler
       default_handler,
   )

   # In the ensure_table_exists function, add:
   elif table_name == "elfryd_ampere":
       create_table_query = sql.SQL(
           """
       CREATE TABLE IF NOT EXISTS {} (
           id SERIAL PRIMARY KEY,
           sensor_id INTEGER NOT NULL,
           current INTEGER NOT NULL,
           device_timestamp BIGINT NOT NULL,
           timestamp TIMESTAMPTZ DEFAULT NOW()
       );
       """
       ).format(sql.Identifier(table_name))

   # In the on_message function's match-case statement, update:
   match table_name:
       case "elfryd_battery" | "elfryd_temp" | "elfryd_gyro" | "elfryd_config" | "elfryd_ampere":
           # Process with appropriate handler
           # ...
           elif table_name == "elfryd_ampere":
               ampere_handler.process_message(datapoint.strip())
   ```

4. **Update the Data Model**
   Add the new data model in `core/models.py`:

   ```python
   # In core/models.py
   class AmpereData(BaseModel):
       id: Optional[int] = None
       sensor_id: int
       current: int
       device_timestamp: int
       timestamp: Optional[datetime] = None

   # Also add a response model for the API:
   class AmpereDataResponse(BaseModel):
       id: Optional[int] = Field(None, description="Unique record identifier", example=123)
       sensor_id: int = Field(..., description="Identifier of the ampere sensor", example=1)
       current: int = Field(..., description="Current in milliamperes (mA)", example=5600)
       device_timestamp: int = Field(..., description="Timestamp of the measurement on the device (Unix timestamp)", example=1712841632)

       class Config:
           json_schema_extra = {
               "example": {
                   "id": 123,
                   "sensor_id": 1,
                   "current": 5600,
                   "device_timestamp": 1712841632
               }
           }
   ```

#### 3. Add API Support

1. **Create API Router**

   ```bash
   # Create a new API route file
   touch app/api/routes/ampere.py
   ```

2. **Implement the API Endpoint**

   ```python
   # app/api/routes/ampere.py
   from fastapi import APIRouter, Depends, Query
   from typing import List
   from core.database import get_connection
   from core.models import AmpereDataResponse
   from api.dependencies import get_api_key

   router = APIRouter()

   @router.get(
       "/ampere",
       response_model=List[AmpereDataResponse],
       summary="Get ampere readings",
       description="Retrieve current measurements from ampere sensors"
   )
   async def get_ampere_data(
       sensor_id: int = Query(None, description="Filter by specific sensor ID"),
       limit: int = Query(20, description="Maximum number of records to return"),
       hours: int = Query(168, description="Get data from the last X hours"),
       time_offset: int = Query(0, description="Hours offset from current time"),
       api_key: str = Depends(get_api_key)
   ):
       conn = get_connection()
       cursor = conn.cursor()

       query = """
           SELECT id, sensor_id, current, device_timestamp
           FROM elfryd_ampere
           WHERE timestamp >= NOW() - INTERVAL '%s hours' - INTERVAL '%s hours'
           AND timestamp <= NOW() - INTERVAL '%s hours'
       """

       params = [hours, time_offset, time_offset]

       if sensor_id is not None:
           query += " AND sensor_id = %s"
           params.append(sensor_id)

       query += " ORDER BY device_timestamp DESC LIMIT %s"
       params.append(limit)

       cursor.execute(query, params)
       results = cursor.fetchall()

       cursor.close()
       conn.close()

       return [
           AmpereDataResponse(
               id=row[0],
               sensor_id=row[1],
               current=row[2],
               device_timestamp=row[3]
           )
           for row in results
       ]
   ```

3. **Update the API Main File**
   Add the new router to `app.py`:

   ```python
   # In app.py
   from api.routes import ampere

   # Add to the app.include_router section:
   app.include_router(ampere.router, tags=["Ampere Sensors"])
   ```

#### 4. Update Documentation

1. **Update the Bridge Documentation**
   Add information about the new sensor type to `bridge.md`:

   ```markdown
   ### Ampere Data (`elfryd/ampere`)

   **Format**: `{sensor_id}/{current}/{timestamp}`

   **Example**: `1/5600/1680123456`

   **Parameters**:

   - `sensor_id`: Identifier of the current sensor (integer)
   - `current`: Electric current in milliamperes (integer)
   - `timestamp`: Device timestamp in Unix seconds (integer)

   **Storage**: Data is stored in the `elfryd_ampere` table.
   ```

2. **Update the API Documentation**
   Add information about the new endpoint to `api.md`:

   ````markdown
   ### Ampere Data

   ```
   GET /ampere
   ```

   Retrieves current measurements from ampere sensors.

   **Parameters:**

   - `sensor_id` (optional): Filter by specific sensor ID
   - `limit` (default: 20): Maximum number of records
   - `hours` (default: 168): Data from last X hours
   - `time_offset` (optional): Hours offset from current time

   **Response Example:**

   ```json
   [
     {
       "id": 123,
       "sensor_id": 1,
       "current": 5600,
       "device_timestamp": 1680123456
     }
   ]
   ```
   ````

### 5. Update the Scripts

Lastsly, the `seed.sh` and `db_monitor.sh` scripts should be updated to include the new sensor type. This will allow for easy testing and monitoring of the new functionality. The implementation will be similar to the existing scripts, ensuring that the new sensor type is included in the data generation and monitoring processes.

### Testing New Sensor Types

Once implemented, you can test the new sensor type using the following steps:

1. **Get the Latest Code**

   ```bash
   git pull origin main  # Or the specific branch
   ```

2. **Tear Down Current Setup**

   ```bash
   sudo bash cleanup.sh
   ```

3. **Update Docker Images**

   ```bash
   cd app
   docker compose build --no-cache mqtt-bridge api
   ```

4. **Start the System**

   ```bash
   cd ..
   sudo bash install.sh
   ```

5. **Send Test Data**

   ```bash
   mosquitto_pub -h your-hostname -p 8885 --cafile ./client_certs/ca.crt -t elfryd/ampere -m "1/5600/$(date +%s)"
   ```

6. **Verify Data Storage**

   ```bash
   # Check database directly
   bash db_monitor.sh ampere

   # Or via API
   curl -k -X GET "https://your-hostname:443/ampere?limit=5" -H "X-API-Key: your-api-key"
   ```

## Backward Compatibility

The Elfryd system is designed to maintain backward compatibility even as new features are added:

1. **Generic Message Storage**: All MQTT messages are stored, even if specialized processing is not available
2. **API Stability**: Existing API endpoints maintain their structure and functionality
3. **Graceful Handling**: Unrecognized message formats are logged but don't crash the system

> **Note**: Make sure that any new topic endpoints do not already have existing message data if the format is different. The system will not automatically convert or migrate existing data.

## Recommendations for Future Additions

When adding new sensor types or features to the Elfryd system, follow these best practices:

1. **Maintain Format Consistency**: Follow the established pattern of `id/value/timestamp` where possible
2. **Batch Processing Support**: Ensure new handlers support the pipe (`|`) separator for batch processing
3. **Error Handling**: Implement robust error handling to prevent system crashes
4. **Documentation**: Update all relevant documentation with the new functionality
5. **Testing**: Create a comprehensive test plan to validate the new functionality

## Conclusion

The Elfryd MQTT broker system is designed to be extendable while maintaining its core reliability and security features. By following the process outlined in this document, new sensor types can be seamlessly integrated into the existing infrastructure, allowing the system to grow alongside the boat monitoring requirements.
