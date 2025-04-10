import paho.mqtt.client as mqtt
import ssl
from fastapi import HTTPException

from .config import MQTT_CONFIG

def create_mqtt_client(client_id: str, use_tls: bool = False) -> mqtt.Client:
    """
    Create and configure an MQTT client
    """
    client = mqtt.Client(
        client_id=client_id,
        protocol=mqtt.MQTTv5,
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    )

    if use_tls:
        # Set TLS settings
        client.tls_set(
            ca_certs="/app/certs/ca.crt",
            cert_reqs=ssl.CERT_REQUIRED,
            tls_version=ssl.PROTOCOL_TLSv1_2,
        )
        client.tls_insecure_set(False)
        host = MQTT_CONFIG["tls_broker"]
        port = MQTT_CONFIG["tls_port"]
    else:
        host = MQTT_CONFIG["broker"]
        port = MQTT_CONFIG["port"]

    try:
        client.connect(host, port, 60)
        return client
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"MQTT connection failed: {str(e)}")

def publish_message(topic, message, qos=2):
    """
    Publish a message to the MQTT broker
    """
    try:
        print(f"Attempting to publish to {topic}: {message}")
        client = create_mqtt_client("mqtt_publisher", use_tls=True)
        print(f"MQTT client created successfully")
        print(f"Using broker: {MQTT_CONFIG['tls_broker']}:{MQTT_CONFIG['tls_port']}")
        result = client.publish(topic, message, qos=qos)
        print(f"Publish result code: {result.rc}, message: {mqtt.error_string(result.rc)}")
        
        # Add a short wait before disconnecting
        client.loop_start()
        import time
        time.sleep(2)  # Give time for the publish to complete
        client.loop_stop()
        client.disconnect()
        
        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            raise HTTPException(
                status_code=500,
                detail=f"Failed to publish message: {mqtt.error_string(result.rc)}",
            )
        
        return True
    except Exception as e:
        print(f"MQTT publish error: {str(e)}")
        # Re-raise the exception
        raise