import paho.mqtt.client as mqtt
import psycopg2
from psycopg2 import sql
import ssl
import time
import os
import re
from datetime import datetime

# MQTT Setup - use container names instead of localhost
MQTT_BROKER = "mqtt-broker"
MQTT_PORT = 1883
MQTT_TLS_PORT = 8885
MQTT_TOPIC = "#"
USE_TLS = os.environ.get("USE_TLS", "false").lower() == "true"

# PostgreSQL Setup - use container name
DB_HOST = "timescaledb"
DB_PORT = "5432"  # Default PostgreSQL port inside container
DB_NAME = "mqtt_data"
DB_USER = "myuser"
DB_PASSWORD = "mypassword"

# Dictionary to track which tables have been created
created_tables = {}

# Function to determine table name from topic
def get_table_name(topic):
    parts = topic.split('/')
    
    # Need at least two parts for a valid table name
    if len(parts) < 2:
        return "mqtt_messages"  # Default table for unrecognized formats
    
    # Use two parts of the topic to create a table name
    return f"{parts[0]}_{parts[1]}"

# Function to create table if it doesn't exist
def ensure_table_exists(table_name, cursor, connection):
    if table_name in created_tables:
        return

    # Special handling for specific tables
    if table_name == "elfryd_battery":
        create_table_query = sql.SQL("""
        CREATE TABLE IF NOT EXISTS {} (
            id SERIAL PRIMARY KEY,
            battery_id INTEGER NOT NULL,
            voltage INTEGER NOT NULL,
            device_timestamp BIGINT NOT NULL,
            topic TEXT NOT NULL,
            raw_message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        """).format(sql.Identifier(table_name))
    elif table_name == "elfryd_temp":
        create_table_query = sql.SQL("""
        CREATE TABLE IF NOT EXISTS {} (
            id SERIAL PRIMARY KEY,
            sensor_id INTEGER NOT NULL,
            temperature INTEGER NOT NULL,
            device_timestamp BIGINT NOT NULL,
            topic TEXT NOT NULL,
            raw_message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        """).format(sql.Identifier(table_name))
    elif table_name == "elfryd_gyro":
        create_table_query = sql.SQL("""
        CREATE TABLE IF NOT EXISTS {} (
            id SERIAL PRIMARY KEY,
            sensor_id INTEGER NOT NULL,
            accel_x INTEGER NOT NULL,
            accel_y INTEGER NOT NULL,
            accel_z INTEGER NOT NULL,
            gyro_x INTEGER NOT NULL,
            gyro_y INTEGER NOT NULL,
            gyro_z INTEGER NOT NULL,
            device_timestamp BIGINT NOT NULL,
            topic TEXT NOT NULL,
            raw_message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        """).format(sql.Identifier(table_name))
    elif table_name == "elfryd_config":
        create_table_query = sql.SQL("""
        CREATE TABLE IF NOT EXISTS {} (
            id SERIAL PRIMARY KEY,
            command TEXT NOT NULL,
            topic TEXT NOT NULL,
            raw_message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        """).format(sql.Identifier(table_name))
    else:
        # Default table structure for unrecognized formats
        create_table_query = sql.SQL("""
        CREATE TABLE IF NOT EXISTS {} (
            id SERIAL PRIMARY KEY,
            topic TEXT NOT NULL,
            message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        """).format(sql.Identifier(table_name))
    
    cursor.execute(create_table_query)
    connection.commit()
    created_tables[table_name] = True
    print(f"Table {table_name} is ready")

# Parse message based on topic and format
def parse_message(topic, message):
    if topic == "elfryd/battery":
        # Format: "1X/Voltage/Timestamp"
        try:
            parts = message.split('/')
            if len(parts) == 3:
                battery_id = int(parts[0])
                voltage = int(parts[1])
                device_timestamp = int(parts[2])
                return {
                    "battery_id": battery_id,
                    "voltage": voltage,
                    "device_timestamp": device_timestamp
                }
        except (ValueError, IndexError) as e:
            print(f"Error parsing battery message: {e}")
    
    elif topic == "elfryd/temp":
        # Format: "2/Temp/Timestamp"
        try:
            parts = message.split('/')
            if len(parts) == 3:
                sensor_id = int(parts[0])
                temperature = int(parts[1])
                device_timestamp = int(parts[2])
                return {
                    "sensor_id": sensor_id,
                    "temperature": temperature,
                    "device_timestamp": device_timestamp
                }
        except (ValueError, IndexError) as e:
            print(f"Error parsing temperature message: {e}")
    
    elif topic == "elfryd/gyro":
        # Format: "3/AccelX,AccelY,AccelZ/GyroX,GyroY,GyroZ/Timestamp"
        try:
            parts = message.split('/')
            if len(parts) == 4:
                sensor_id = int(parts[0])
                accel_parts = parts[1].split(',')
                gyro_parts = parts[2].split(',')
                device_timestamp = int(parts[3])
                
                if len(accel_parts) == 3 and len(gyro_parts) == 3:
                    return {
                        "sensor_id": sensor_id,
                        "accel_x": int(accel_parts[0]),
                        "accel_y": int(accel_parts[1]),
                        "accel_z": int(accel_parts[2]),
                        "gyro_x": int(gyro_parts[0]),
                        "gyro_y": int(gyro_parts[1]),
                        "gyro_z": int(gyro_parts[2]),
                        "device_timestamp": device_timestamp
                    }
        except (ValueError, IndexError) as e:
            print(f"Error parsing gyro message: {e}")
    
    elif topic == "elfryd/config":
        # Format could be "freqX", "battery", "temp", or "gyro"
        return {
            "command": message
        }
    
    # Return None if parsing fails or for unrecognized formats
    return None

# Retry connection to services
def connect_with_retry(connect_func, max_retries=30, delay=2):
    retries = 0
    while retries < max_retries:
        try:
            return connect_func()
        except Exception as e:
            print(f"Connection failed: {e}. Retrying in {delay} seconds...")
            retries += 1
            time.sleep(delay)
    raise Exception("Max retries reached, cannot connect")

# Connect to PostgreSQL with retry
def db_connect():
    return psycopg2.connect(
        dbname=DB_NAME, user=DB_USER, password=DB_PASSWORD, host=DB_HOST, port=DB_PORT
    )

# Connect to database
conn = connect_with_retry(db_connect)
cur = conn.cursor()

# Create default table for unrecognized message formats
ensure_table_exists("mqtt_messages", cur, conn)

# MQTT callback functions
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Successfully connected to MQTT broker")
        client.subscribe(MQTT_TOPIC)
        print(f"Subscribed to {MQTT_TOPIC}")
    else:
        print(f"Failed to connect to MQTT broker with result code {rc}")

def on_message(client, userdata, msg):
    topic = msg.topic
    message = msg.payload.decode()
    
    print(f"Message received on topic {topic}: {message}")

    # Determine which table to use
    table_name = get_table_name(topic)
    
    # Ensure the table exists
    ensure_table_exists(table_name, cur, conn)
    
    # Parse message based on topic
    parsed_data = parse_message(topic, message)
    
    # Insert the message into the appropriate table
    if parsed_data and table_name == "elfryd_battery":
        insert_query = sql.SQL("""
            INSERT INTO {} (battery_id, voltage, device_timestamp, topic, raw_message)
            VALUES (%s, %s, %s, %s, %s)
        """).format(sql.Identifier(table_name))
        
        cur.execute(insert_query, (
            parsed_data["battery_id"],
            parsed_data["voltage"],
            parsed_data["device_timestamp"],
            topic,
            message
        ))
    elif parsed_data and table_name == "elfryd_temp":
        insert_query = sql.SQL("""
            INSERT INTO {} (sensor_id, temperature, device_timestamp, topic, raw_message)
            VALUES (%s, %s, %s, %s, %s)
        """).format(sql.Identifier(table_name))
        
        cur.execute(insert_query, (
            parsed_data["sensor_id"],
            parsed_data["temperature"],
            parsed_data["device_timestamp"],
            topic,
            message
        ))
    elif parsed_data and table_name == "elfryd_gyro":
        insert_query = sql.SQL("""
            INSERT INTO {} (sensor_id, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, 
                           device_timestamp, topic, raw_message)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """).format(sql.Identifier(table_name))
        
        cur.execute(insert_query, (
            parsed_data["sensor_id"],
            parsed_data["accel_x"],
            parsed_data["accel_y"],
            parsed_data["accel_z"],
            parsed_data["gyro_x"],
            parsed_data["gyro_y"],
            parsed_data["gyro_z"],
            parsed_data["device_timestamp"],
            topic,
            message
        ))
    elif parsed_data and table_name == "elfryd_config":
        insert_query = sql.SQL("""
            INSERT INTO {} (command, topic, raw_message)
            VALUES (%s, %s, %s)
        """).format(sql.Identifier(table_name))
        
        cur.execute(insert_query, (
            parsed_data["command"],
            topic,
            message
        ))
    else:
        # Default insertion for unparsed or unrecognized messages
        insert_query = sql.SQL("INSERT INTO {} (topic, message) VALUES (%s, %s)").format(
            sql.Identifier(table_name)
        )
        
        cur.execute(insert_query, (topic, message))
    
    conn.commit()

# Setup MQTT client with the newer callback API
client = mqtt.Client(
    client_id="",
    protocol=mqtt.MQTTv5,
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
)
client.on_connect = on_connect
client.on_message = on_message

# Setup TLS if enabled
if USE_TLS:
    print("Using TLS connection...")
    client.tls_set(
        ca_certs="/app/certs/ca.crt",
        certfile=None,
        keyfile=None,
        cert_reqs=ssl.CERT_REQUIRED,
        tls_version=ssl.PROTOCOL_TLSv1_2,
    )
    client.tls_insecure_set(False)
    MQTT_PORT = MQTT_TLS_PORT

# Connect to MQTT broker with retry
def mqtt_connect():
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    return True

connect_with_retry(mqtt_connect)

# Loop forever to listen for messages
try:
    print("MQTT bridge is running. Press Ctrl+C to exit.")
    client.loop_forever()
except KeyboardInterrupt:
    print("Disconnecting from broker...")
    client.disconnect()
    conn.close()
except Exception as e:
    print(f"Error: {e}")
    conn.close()