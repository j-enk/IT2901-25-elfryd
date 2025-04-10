import time

import paho.mqtt.client as mqtt
from core.config import MQTT_CONFIG
from core.database import get_table_name
from core.mqtt import create_mqtt_client
from bridge.handlers import battery_handler, temperature_handler, gyro_handler, config_handler, default_handler

# Callback when a MQTT message is received
def on_message(msg: mqtt.MQTTMessage):
    """Handle incoming MQTT messages"""
    try:
        topic = msg.topic
        payload = msg.payload.decode('utf-8')
        print(f"Received message on topic {topic}: {payload}")
        
        # Determine table name based on topic
        table_name = get_table_name(topic)
        
        # Process message based on topic with match-case (Python 3.10+)
        match table_name:
            case "elfryd/battery":
                battery_handler.process_message(topic, payload)
            case "elfryd/temp":
                temperature_handler.process_message(topic, payload)
            case "elfryd/gyro":
                gyro_handler.process_message(topic, payload)
            case "elfryd/config":
                config_handler.process_message(topic, payload)
            case _:
                # Default case: use default handler
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