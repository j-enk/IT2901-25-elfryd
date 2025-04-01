import paho.mqtt.client as mqtt
import psycopg2
from psycopg2 import sql
import ssl
import time
import os

# MQTT Setup - use container names instead of localhost
MQTT_BROKER = "mqtt-broker"
MQTT_PORT = 1883
MQTT_TLS_PORT = 8885
MQTT_TOPIC = "#"
USE_TLS = os.environ.get('USE_TLS', 'false').lower() == 'true'

# PostgreSQL Setup - use container name
DB_HOST = "timescaledb"
DB_PORT = "5432"  # Default PostgreSQL port inside container
DB_NAME = "mqtt_data"
DB_USER = "myuser"
DB_PASSWORD = "mypassword"

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
        dbname=DB_NAME,
        user=DB_USER,
        password=DB_PASSWORD,
        host=DB_HOST,
        port=DB_PORT
    )

# Connect to database
conn = connect_with_retry(db_connect)
cur = conn.cursor()

# Create table if it doesn't exist
create_table_query = """
CREATE TABLE IF NOT EXISTS mqtt_messages (
    id SERIAL PRIMARY KEY,
    topic TEXT NOT NULL,
    message TEXT NOT NULL,
    timestamp TIMESTAMPTZ DEFAULT NOW()
);
"""
cur.execute(create_table_query)
conn.commit()

# MQTT callback functions
def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Successfully connected to MQTT broker")
        client.subscribe(MQTT_TOPIC)
        print(f"Subscribed to {MQTT_TOPIC}")
    else:
        print(f"Failed to connect to MQTT broker with result code {rc}")

def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic}: {msg.payload.decode()}")
    
    # Insert the message into the database
    insert_query = sql.SQL("INSERT INTO mqtt_messages (topic, message) VALUES (%s, %s)")
    cur.execute(insert_query, (msg.topic, msg.payload.decode()))
    conn.commit()

# Setup MQTT client with the newer callback API
client = mqtt.Client(client_id="", protocol=mqtt.MQTTv5, callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
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
        tls_version=ssl.PROTOCOL_TLSv1_2
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
