from fastapi import APIRouter, HTTPException, Depends, Query, Body
from typing import List, Optional

from core.database import get_connection, query_messages
from core.models import StoredMessage, MQTTMessage
from core.mqtt import publish_message
from api.dependencies import get_api_key

router = APIRouter(tags=["Messages"])

# Define special topics that have dedicated endpoints
SPECIAL_TOPICS = {
    "elfryd/battery": "/battery",
    "elfryd/temp": "/temperature",
    "elfryd/gyro": "/gyro",
    "elfryd/config": "/config"
}

@router.get("/messages", response_model=List[StoredMessage], summary="Get stored messages")
def get_messages(
    topic: str = Query(..., description="Filter by topic (partial match)"), 
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    offset: int = Query(
        0, ge=0, description="Number of records to skip"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get messages from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get stored MQTT messages with filtering by topic (required)
    """
    # Check if topic matches any special topics that have dedicated endpoints
    for special_topic, endpoint in SPECIAL_TOPICS.items():
        if special_topic in topic.lower():
            raise HTTPException(
                status_code=400, 
                detail=f"This topic contains specialized data. Please use the dedicated endpoint: {endpoint}"
            )
    
    try:
        conn = get_connection()
        results = query_messages(conn, topic, limit, offset, hours)
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@router.post("/messages", response_model=dict, summary="Publish MQTT message")
def publish_mqtt_message(
    message: MQTTMessage = Body(...),
    _: str = Depends(get_api_key),
):
    """
    Publish a message to the MQTT broker
    """
    if not message.topic.strip():
        raise HTTPException(status_code=400, detail="Topic cannot be empty")
    
    if not message.message.strip():
        raise HTTPException(status_code=400, detail="Message cannot be empty")
    
    # Check if posting to special topics
    for special_topic in SPECIAL_TOPICS:
        if special_topic in message.topic.lower():
            raise HTTPException(
                status_code=403, 
                detail="Publishing to specialized data topics via API is not allowed. Use MQTT with TLS for secure publishing."
            )
    
    try:
        publish_message(message.topic, message.message)
        return {"success": True, "message": "Message published: " + message.message + " to topic: " + message.topic}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Failed to publish message: {str(e)}"
        )