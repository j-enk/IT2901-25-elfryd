from fastapi import APIRouter, HTTPException, Depends, Query, Body
from typing import List, Optional

from core.database import get_connection, query_messages
from core.models import StoredMessage, MQTTMessage, MessagePublishResponse
from core.mqtt import publish_message
from api.dependencies import get_api_key

router = APIRouter(tags=["Messages"])

# Define special topics that have dedicated endpoints
SPECIAL_TOPICS = {
    "elfryd/battery": "/battery",
    "elfryd/temp": "/temperature",
    "elfryd/gyro": "/gyro",
    "elfryd/config": "/config",
}


@router.get("/messages", response_model=List[StoredMessage], summary="Get stored messages")
def get_messages(
    topic: str = Query(..., description="Filter by topic (partial match)"),
    limit: int = Query(
        100, ge=0, le=10000, description="Maximum number of records to return (0 for no limit)"
    ),
    offset: int = Query(0, ge=0, description="Number of records to skip"),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    time_offset: Optional[float] = Query(
        None,
        ge=0,
        description="Offset in hours from current time (e.g., 336 = start from 2 weeks ago)",
    ),
    _: str = Depends(get_api_key),
):
    """
    Retrieve stored MQTT messages filtered by topic.

    Get messages from the database that match the specified topic pattern.
    Results can be paginated using the limit and offset parameters.
    Time filtering can be applied using hours and time_offset.

    ## Parameters
    - **topic**: Filter by topic (partial match)
    - **limit**: Maximum number of records to return (default: 100, max: 10000). When used with the hours parameter, 
      data points will be evenly distributed across the time range instead of just returning the newest records.
      Set to 0 to disable limiting and return all data points in the time range.
    - **offset**: Number of records to skip (for pagination)
    - **hours**: Get data from the last X hours (default: 24)
    - **time_offset**: Offset in hours from current time (e.g., 336 = start from 2 weeks ago)

    ## Response
    Returns an array of message records, each containing:
    - **id**: Unique message identifier
    - **topic**: The MQTT topic
    - **message**: The message content (as string)
    - **timestamp**: Timestamp when the message was received

    ## Authentication
    Requires API key in the X-API-Key header
    """
    # Check if topic matches any special topics that have dedicated endpoints
    for special_topic, endpoint in SPECIAL_TOPICS.items():
        if special_topic in topic.lower():
            raise HTTPException(
                status_code=400,
                detail=f"This topic contains specialized data. Please use the dedicated endpoint: {endpoint}",
            )

    try:
        conn = get_connection()
        results = query_messages(conn, topic, limit, offset, hours, time_offset)
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")


@router.post(
    "/messages", response_model=MessagePublishResponse, summary="Publish MQTT message"
)
def publish_mqtt_message(
    message: MQTTMessage = Body(...),
    _: str = Depends(get_api_key),
):
    """
    Publish a message to the MQTT broker.

    This endpoint allows you to send a message to any non-specialized MQTT topic.

    ## Request Body
    - **topic**: The MQTT topic to publish to (required)
    - **message**: The message content to publish (required)

    ## Restrictions
    Publishing to specialized data topics (elfryd/battery, elfryd/temp, elfryd/gyro, elfryd/config)
    is not allowed via the API for security reasons. Use MQTT with TLS for these topics.

    ## Response
    - JSON object with success status and topic that was published to

    ## Authentication
    Requires API key in the X-API-Key header
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
                detail="Publishing to specialized data topics via API is not allowed. Use MQTT with TLS for secure publishing.",
            )

    try:
        publish_message(message.topic, message.message)
        return {"success": True, "topic": message.topic}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Failed to publish message: {str(e)}"
        )
