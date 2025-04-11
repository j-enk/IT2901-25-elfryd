import paho.mqtt.client as mqtt
from psycopg2 import sql
from core.config import MQTT_CONFIG
from core.database import get_table_name, get_connection
from core.mqtt import create_mqtt_client
from bridge.handlers import battery_handler, temperature_handler, gyro_handler, config_handler, default_handler

# Dictionary to track which tables have been created
created_tables = {}

# Function to create table if it doesn't exist
def ensure_table_exists(table_name):
    if table_name in created_tables:
        return

    try:
        conn = get_connection()
        cursor = conn.cursor()

        # Special handling for specific tables
        if table_name == "elfryd_battery":
            create_table_query = sql.SQL("""
            CREATE TABLE IF NOT EXISTS {} (
                id SERIAL PRIMARY KEY,
                battery_id INTEGER NOT NULL,
                voltage INTEGER NOT NULL,
                device_timestamp BIGINT NOT NULL,
                timestamp TIMESTAMPTZ DEFAULT NOW()
            );
            """).format(sql.Identifier(table_name))
        elif table_name == "elfryd_temp":
            create_table_query = sql.SQL("""
            CREATE TABLE IF NOT EXISTS {} (
                id SERIAL PRIMARY KEY,
                temperature INTEGER NOT NULL,
                device_timestamp BIGINT NOT NULL,
                timestamp TIMESTAMPTZ DEFAULT NOW()
            );
            """).format(sql.Identifier(table_name))
        elif table_name == "elfryd_gyro":
            create_table_query = sql.SQL("""
            CREATE TABLE IF NOT EXISTS {} (
                id SERIAL PRIMARY KEY,
                accel_x INTEGER NOT NULL,
                accel_y INTEGER NOT NULL,
                accel_z INTEGER NOT NULL,
                gyro_x INTEGER NOT NULL,
                gyro_y INTEGER NOT NULL,
                gyro_z INTEGER NOT NULL,
                device_timestamp BIGINT NOT NULL,
                timestamp TIMESTAMPTZ DEFAULT NOW()
            );
            """).format(sql.Identifier(table_name))
        elif table_name == "elfryd_config":
            create_table_query = sql.SQL("""
            CREATE TABLE IF NOT EXISTS {} (
                id SERIAL PRIMARY KEY,
                command TEXT NOT NULL,
                topic TEXT NOT NULL,
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
        conn.commit()
        cursor.close()
        conn.close()
        
        created_tables[table_name] = True
        print(f"Table {table_name} is ready")
    
    except Exception as e:
        print(f"Error creating table {table_name}: {str(e)}")

# Callback when a MQTT message is received
def on_message(_, __, msg: mqtt.MQTTMessage):
    """Handle incoming MQTT messages"""
    try:
        topic = msg.topic
        payload = msg.payload.decode('utf-8')
        print(f"Received message on topic {topic}: {payload}")
        
        # Determine table name based on topic
        table_name = get_table_name(topic)
        
        # Ensure the table exists before processing
        ensure_table_exists(table_name)
        
        # Process message based on topic with match-case (Python 3.10+)
        match table_name:
            case "elfryd_battery" | "elfryd_temp" | "elfryd_gyro" | "elfryd_config":
                # For specialized handlers, check if payload contains multiple datapoints
                datapoints = payload.split("|")
                for datapoint in datapoints:
                    if not datapoint.strip():
                        continue  # Skip empty datapoints
                    
                    print(f"Processing datapoint: {datapoint.strip()}")
                    
                    # Process with appropriate handler
                    if table_name == "elfryd_battery":
                        battery_handler.process_message(datapoint.strip())
                    elif table_name == "elfryd_temp":
                        temperature_handler.process_message(datapoint.strip())
                    elif table_name == "elfryd_gyro":
                        gyro_handler.process_message(datapoint.strip())
                    elif table_name == "elfryd_config":
                        config_handler.process_message(topic, datapoint.strip())
            case _:
                # Default case: use default handler (no splitting)
                default_handler.process_message(topic, payload)
    
    except Exception as e:
        print(f"Error processing message: {str(e)}")

def main():
    # Get MQTT client from core module
    client = create_mqtt_client("mqtt_bridge")
    
    # Set message callback
    client.on_message = on_message
    
    # Subscribe to all topics
    client.subscribe(MQTT_CONFIG["default_topic"], qos=2)
    print(f"Subscribed to topic: {MQTT_CONFIG['default_topic']}")
    
    # Start the MQTT loop
    client.loop_forever()

if __name__ == "__main__":
    main()