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
        client = create_mqtt_client("mqtt_publisher", use_tls=True)
        # Start the network loop
        client.loop_start()
        result = client.publish(topic, message, qos=qos)
        # Wait for the message to be published
        result.wait_for_publish()
        # Stop the network loop and disconnect
        client.loop_stop()
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
