from fastapi import APIRouter, HTTPException, Depends, Query, Body
from typing import List, Optional

from core.database import get_connection, query_messages
from core.models import StoredMessage, MQTTMessage
from core.mqtt import publish_message
from api.dependencies import get_api_key

router = APIRouter(tags=["Messages"])

@router.get("/messages", response_model=List[StoredMessage], summary="Get stored messages")
def get_messages(
    topic: Optional[str] = Query(None, description="Filter by topic (partial match)"),
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
    Get stored MQTT messages with optional filtering
    """
    try:
        conn = get_connection()
        results = query_messages(conn, topic, limit, offset, hours)
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@router.post("/messages/publish", response_model=dict, summary="Publish MQTT message")
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
    
    try:
        publish_message(message.topic, message.message)
        return {"success": True, "message": "Message published"}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Failed to publish message: {str(e)}"
        )