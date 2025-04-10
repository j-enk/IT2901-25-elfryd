import paho.mqtt.client as mqtt
import ssl
from fastapi import HTTPException

from .config import MQTT_CONFIG

def create_mqtt_client(client_id: str):
    """
    Create and configure an MQTT client
    """
    client = mqtt.Client(
        client_id=client_id,
        protocol=mqtt.MQTTv5,
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    )

    if MQTT_CONFIG["use_tls"]:
        client.tls_set(
            ca_certs="/app/certs/ca.crt",
            cert_reqs=ssl.CERT_REQUIRED,
            tls_version=ssl.PROTOCOL_TLSv1_2,
        )
        client.tls_insecure_set(False)
        port = MQTT_CONFIG["tls_port"]
    else:
        port = MQTT_CONFIG["port"]

    try:
        client.connect(MQTT_CONFIG["broker"], port, 60)
        return client
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"MQTT connection failed: {str(e)}")

def publish_message(topic, message, qos=2):
    """
    Publish a message to the MQTT broker
    """
    try:
        client = create_mqtt_client("publisher")
        result = client.publish(topic, message, qos=qos)
        client.disconnect()

        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            raise HTTPException(
                status_code=500,
                detail=f"Failed to publish message: {mqtt.error_string(result.rc)}",
            )
            
        return True
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Error publishing message: {str(e)}"
        )