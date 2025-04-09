from fastapi import FastAPI, HTTPException, Depends, BackgroundTasks, Query
from fastapi.middleware.cors import CORSMiddleware
import paho.mqtt.client as mqtt
import psycopg2
from psycopg2 import sql
import os
import ssl
import time
from typing import List, Dict, Any, Optional
from datetime import datetime, timedelta
from pydantic import BaseModel

from fastapi.security.api_key import APIKeyHeader
from fastapi import Security, HTTPException, Depends
from starlette.status import HTTP_403_FORBIDDEN
import secrets

app = FastAPI(
    title="Elfryd MQTT API",
    description="API for interacting with MQTT broker and retrieving stored messages",
    openapi_tags=[
        {
            "name": "Authentication",
            "description": "Requires X-API-Key header for protected endpoints",
        }
    ],
)
# Enable CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In production, replace with specific origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Database connection parameters - only accessible within Docker network
DB_HOST = "timescaledb"
DB_PORT = 5432
DB_NAME = "mqtt_data"
DB_USER = "myuser"
DB_PASSWORD = "mypassword"

# MQTT broker parameters
MQTT_BROKER = "mqtt-broker"
MQTT_PORT = 1883
MQTT_TLS_PORT = 8885
USE_TLS = os.environ.get("USE_TLS", "false").lower() == "true"

# Get API key from environment variable
API_KEY = os.getenv("API_KEY")
if not API_KEY:
    raise ValueError("API_KEY environment variable not set")
api_key_header = APIKeyHeader(name="X-API-Key", auto_error=False)


# Function to determine table name from topic
def get_table_name(topic):
    parts = topic.split('/')
    
    # Need at least two parts for a valid table name
    if len(parts) < 2:
        return "mqtt_messages"  # Default table for unrecognized formats
    
    # Use two parts of the topic to create a table name
    return f"{parts[0]}_{parts[1]}"


# Function to get all table names in the database that are message tables
def get_all_message_tables(conn):
    cursor = conn.cursor()
    cursor.execute("""
        SELECT table_name FROM information_schema.tables 
        WHERE table_schema = 'public' AND 
        (table_name LIKE '%_%' OR table_name = 'mqtt_messages')
    """)
    tables = [row[0] for row in cursor.fetchall()]
    cursor.close()
    return tables


# Pydantic models for various data types
class MQTTMessage(BaseModel):
    topic: str
    message: str

class StoredMessage(BaseModel):
    id: int
    topic: str
    message: str
    timestamp: datetime

class BatteryData(BaseModel):
    id: int
    battery_id: int
    voltage: int
    device_timestamp: int
    topic: str
    raw_message: str
    timestamp: datetime

class TemperatureData(BaseModel):
    id: int
    sensor_id: int
    temperature: int
    device_timestamp: int
    topic: str
    raw_message: str
    timestamp: datetime

class GyroData(BaseModel):
    id: int
    sensor_id: int
    accel_x: int
    accel_y: int
    accel_z: int
    gyro_x: int
    gyro_y: int
    gyro_z: int
    device_timestamp: int
    topic: str
    raw_message: str
    timestamp: datetime

class ConfigData(BaseModel):
    id: int
    command: str
    topic: str
    raw_message: str
    timestamp: datetime



# Database connection pool
def get_db_connection():
    try:
        conn = psycopg2.connect(
            host=DB_HOST,
            port=DB_PORT,
            dbname=DB_NAME,
            user=DB_USER,
            password=DB_PASSWORD,
        )
        return conn
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Database connection failed: {str(e)}"
        )


# MQTT client setup
def get_mqtt_client():
    client = mqtt.Client(
        client_id="api-client",
        protocol=mqtt.MQTTv5,
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    )

    if USE_TLS:
        client.tls_set(
            ca_certs="/app/certs/ca.crt",
            cert_reqs=ssl.CERT_REQUIRED,
            tls_version=ssl.PROTOCOL_TLSv1_2,
        )
        client.tls_insecure_set(False)
        port = MQTT_TLS_PORT
    else:
        port = MQTT_PORT

    try:
        client.connect(MQTT_BROKER, port, 60)
        return client
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"MQTT connection failed: {str(e)}")

# Create a dependency
async def get_api_key(api_key: str = Security(api_key_header)):
    if api_key != API_KEY:
        raise HTTPException(
            status_code=HTTP_403_FORBIDDEN,
            detail="Invalid API key",
        )
    return api_key

# API Endpoints
@app.get("/", summary="Root endpoint")
def read_root():
    """
    Root endpoint providing basic API information.
    """
    return {"message": "Elfryd MQTT API is running", "version": "1.0"}


@app.post("/messages", summary="Publish an MQTT message")
def publish_message(
    message: MQTTMessage,
    _: str = Depends(get_api_key),
):
    """
    Publish a message to the MQTT broker
    """
    try:
        client = get_mqtt_client()
        result = client.publish(message.topic, message.message, qos=1)
        client.disconnect()

        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            raise HTTPException(
                status_code=500,
                detail=f"Failed to publish message: {mqtt.error_string(result.rc)}",
            )

        return {"status": "success", "message": "Message published successfully"}
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Error publishing message: {str(e)}"
        )



@app.get("/messages", response_model=List[Union[StoredMessage, BatteryData, TemperatureData, GyroData, ConfigData]], 
         summary="Get stored messages")
def get_messages(
    topic: Optional[str] = Query(
        None, description="Filter by topic (supports SQL LIKE patterns)"
    ),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of messages to return"
    ),
    offset: int = Query(0, ge=0, description="Number of messages to skip"),
    hours: Optional[float] = Query(
        None, ge=0, description="Get messages from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get messages stored in the database with optional filtering.
    Returns specialized data structures for known topics.
    """
    try:
        conn = get_db_connection()
        all_results = []
        
        # Decide which tables to query
        if topic:
            # If a topic filter is provided, only query the relevant table
            tables = [get_table_name(topic)]
        else:
            # Without a specific topic, query all tables
            tables = get_all_message_tables(conn)
        
        # For each table, construct and execute a query
        for table in tables:
            try:
                cur = conn.cursor()
                
                # Build the query based on parameters and table structure
                if table == "elfryd_battery":
                    query = sql.SQL("""
                        SELECT id, battery_id, voltage, device_timestamp, topic, raw_message, timestamp 
                        FROM {} WHERE 1=1
                    """).format(sql.Identifier(table))
                elif table == "elfryd_temp":
                    query = sql.SQL("""
                        SELECT id, sensor_id, temperature, device_timestamp, topic, raw_message, timestamp 
                        FROM {} WHERE 1=1
                    """).format(sql.Identifier(table))
                elif table == "elfryd_gyro":
                    query = sql.SQL("""
                        SELECT id, sensor_id, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, 
                               device_timestamp, topic, raw_message, timestamp 
                        FROM {} WHERE 1=1
                    """).format(sql.Identifier(table))
                elif table == "elfryd_config":
                    query = sql.SQL("""
                        SELECT id, command, topic, raw_message, timestamp 
                        FROM {} WHERE 1=1
                    """).format(sql.Identifier(table))
                else:
                    # Default query for standard message tables
                    query = sql.SQL("SELECT id, topic, message, timestamp FROM {} WHERE 1=1").format(
                        sql.Identifier(table)
                    )
                
                params = []
                
                if topic:
                    query = sql.SQL("{} AND topic LIKE %s").format(query)
                    params.append(f"%{topic}%")
                
                if hours is not None:
                    query = sql.SQL("{} AND timestamp > %s").format(query)
                    params.append(datetime.now() - timedelta(hours=hours))
                
                query = sql.SQL("{} ORDER BY timestamp DESC").format(query)
                
                cur.execute(query, params)
                rows = cur.fetchall()
                
                # Convert rows to appropriate data model based on table
                if table == "elfryd_battery":
                    table_results = [
                        {
                            "id": row[0], 
                            "battery_id": row[1], 
                            "voltage": row[2], 
                            "device_timestamp": row[3], 
                            "topic": row[4], 
                            "raw_message": row[5], 
                            "timestamp": row[6]
                        } for row in rows
                    ]
                elif table == "elfryd_temp":
                    table_results = [
                        {
                            "id": row[0], 
                            "sensor_id": row[1], 
                            "temperature": row[2], 
                            "device_timestamp": row[3], 
                            "topic": row[4], 
                            "raw_message": row[5], 
                            "timestamp": row[6]
                        } for row in rows
                    ]
                elif table == "elfryd_gyro":
                    table_results = [
                        {
                            "id": row[0], 
                            "sensor_id": row[1], 
                            "accel_x": row[2], 
                            "accel_y": row[3], 
                            "accel_z": row[4], 
                            "gyro_x": row[5], 
                            "gyro_y": row[6], 
                            "gyro_z": row[7], 
                            "device_timestamp": row[8], 
                            "topic": row[9], 
                            "raw_message": row[10], 
                            "timestamp": row[11]
                        } for row in rows
                    ]
                elif table == "elfryd_config":
                    table_results = [
                        {
                            "id": row[0], 
                            "command": row[1], 
                            "topic": row[2], 
                            "raw_message": row[3], 
                            "timestamp": row[4]
                        } for row in rows
                    ]
                else:
                    # Standard message format
                    table_results = [
                        {"id": row[0], "topic": row[1], "message": row[2], "timestamp": row[3]}
                        for row in rows
                    ]
                
                all_results.extend(table_results)
                cur.close()
            except Exception as table_error:
                # If a specific table query fails, log it but continue with other tables
                print(f"Error querying table {table}: {str(table_error)}")
                continue
        
        # Sort combined results by timestamp (newest first)
        all_results.sort(key=lambda x: x["timestamp"], reverse=True)
        
        # Apply limit and offset to the combined results
        paginated_results = all_results[offset:offset+limit]
        
        conn.close()
        return paginated_results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

# Add new endpoints for specific data types
@app.get("/battery", response_model=List[BatteryData], summary="Get battery data")
def get_battery_data(
    battery_id: Optional[int] = Query(None, description="Filter by battery ID"),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get battery data with optional filtering
    """
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        
        query = sql.SQL("""
            SELECT id, battery_id, voltage, device_timestamp, topic, raw_message, timestamp 
            FROM elfryd_battery WHERE 1=1
        """)
        
        params = []
        
        if battery_id is not None:
            query = sql.SQL("{} AND battery_id = %s").format(query)
            params.append(battery_id)
        
        if hours is not None:
            query = sql.SQL("{} AND timestamp > %s").format(query)
            params.append(datetime.now() - timedelta(hours=hours))
        
        query = sql.SQL("{} ORDER BY timestamp DESC LIMIT %s").format(query)
        params.append(limit)
        
        cur.execute(query, params)
        
        results = [
            {
                "id": row[0], 
                "battery_id": row[1], 
                "voltage": row[2], 
                "device_timestamp": row[3], 
                "topic": row[4], 
                "raw_message": row[5], 
                "timestamp": row[6]
            } for row in cur.fetchall()
        ]
        
        cur.close()
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@app.get("/temperature", response_model=List[TemperatureData], summary="Get temperature data")
def get_temperature_data(
    sensor_id: Optional[int] = Query(None, description="Filter by sensor ID"),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get temperature data with optional filtering
    """
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        
        query = sql.SQL("""
            SELECT id, sensor_id, temperature, device_timestamp, topic, raw_message, timestamp 
            FROM elfryd_temp WHERE 1=1
        """)
        
        params = []
        
        if sensor_id is not None:
            query = sql.SQL("{} AND sensor_id = %s").format(query)
            params.append(sensor_id)
        
        if hours is not None:
            query = sql.SQL("{} AND timestamp > %s").format(query)
            params.append(datetime.now() - timedelta(hours=hours))
        
        query = sql.SQL("{} ORDER BY timestamp DESC LIMIT %s").format(query)
        params.append(limit)
        
        cur.execute(query, params)
        
        results = [
            {
                "id": row[0], 
                "sensor_id": row[1], 
                "temperature": row[2], 
                "device_timestamp": row[3], 
                "topic": row[4], 
                "raw_message": row[5], 
                "timestamp": row[6]
            } for row in cur.fetchall()
        ]
        
        cur.close()
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@app.get("/gyro", response_model=List[GyroData], summary="Get gyroscope data")
def get_gyro_data(
    sensor_id: Optional[int] = Query(None, description="Filter by sensor ID"),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get gyroscope data with optional filtering
    """
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        
        query = sql.SQL("""
            SELECT id, sensor_id, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, 
                   device_timestamp, topic, raw_message, timestamp 
            FROM elfryd_gyro WHERE 1=1
        """)
        
        params = []
        
        if sensor_id is not None:
            query = sql.SQL("{} AND sensor_id = %s").format(query)
            params.append(sensor_id)
        
        if hours is not None:
            query = sql.SQL("{} AND timestamp > %s").format(query)
            params.append(datetime.now() - timedelta(hours=hours))
        
        query = sql.SQL("{} ORDER BY timestamp DESC LIMIT %s").format(query)
        params.append(limit)
        
        cur.execute(query, params)
        
        results = [
            {
                "id": row[0], 
                "sensor_id": row[1], 
                "accel_x": row[2], 
                "accel_y": row[3], 
                "accel_z": row[4], 
                "gyro_x": row[5], 
                "gyro_y": row[6], 
                "gyro_z": row[7], 
                "device_timestamp": row[8], 
                "topic": row[9], 
                "raw_message": row[10], 
                "timestamp": row[11]
            } for row in cur.fetchall()
        ]
        
        cur.close()
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@app.post("/config", summary="Send a config command")
def send_config_command(
    command: str = Query(..., description="Config command to send"),
    _: str = Depends(get_api_key),
):
    """
    Send a configuration command to the MQTT broker
    """
    try:
        # Validate config command
        valid_commands = ["battery", "temp", "gyro"]
        freq_pattern = re.compile(r"^freq\d+$")
        
        if not (command in valid_commands or freq_pattern.match(command)):
            raise HTTPException(
                status_code=400, 
                detail=f"Invalid command. Must be one of {valid_commands} or 'freqX' where X is a number"
            )
        
        # Publish command to MQTT
        client = get_mqtt_client()
        result = client.publish("elfryd/config", command, qos=1)
        client.disconnect()

        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            raise HTTPException(
                status_code=500,
                detail=f"Failed to publish command: {mqtt.error_string(result.rc)}",
            )

        return {"status": "success", "message": f"Config command '{command}' sent successfully"}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Error sending config command: {str(e)}"
        )

@app.get("/topics", summary="Get list of topics")
def get_topics(
    _: str = Depends(get_api_key),
):
    """
    Get a list of all unique topics in the database from all message tables
    """
    try:
        conn = get_db_connection()
        all_topics = set()
        
        # Get all message tables
        tables = get_all_message_tables(conn)
        
        # Query each table for unique topics
        for table in tables:
            try:
                cur = conn.cursor()
                query = sql.SQL("SELECT DISTINCT topic FROM {} ORDER BY topic").format(
                    sql.Identifier(table)
                )
                cur.execute(query)
                table_topics = [row[0] for row in cur.fetchall()]
                all_topics.update(table_topics)
                cur.close()
            except Exception as table_error:
                # If a specific table query fails, log it but continue with other tables
                print(f"Error querying topics from table {table}: {str(table_error)}")
                continue
        
        conn.close()
        return {"topics": sorted(list(all_topics))}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")


@app.get("/health", summary="Check API health")
def health_check():
    """
    Check the health of connections to MQTT broker and database
    """
    health_status = {"status": "healthy", "mqtt": False, "database": False}

    # Check DB connection
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        cur.execute("SELECT 1")
        health_status["database"] = True
        cur.close()
        conn.close()
    except Exception as e:
        health_status["status"] = "degraded"
        health_status["database_error"] = str(e)

    # Check MQTT connection
    try:
        client = get_mqtt_client()
        client.disconnect()
        health_status["mqtt"] = True
    except Exception as e:
        health_status["status"] = "degraded"
        health_status["mqtt_error"] = str(e)

    if not health_status["mqtt"] or not health_status["database"]:
        health_status["status"] = "degraded"

    return health_status