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


@router.get(
    "/messages", response_model=List[StoredMessage], summary="Get stored messages"
)
def get_messages(
    topic: str = Query(..., description="Filter by topic (partial match)"),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    offset: int = Query(0, ge=0, description="Number of records to skip"),
    hours: Optional[float] = Query(
        24, ge=0, description="Get messages from the last X hours"
    ),
    time_offset: Optional[float] = Query(
        None,
        ge=0,
        description="Offset in hours from current time (e.g., 336 = start from 2 weeks ago)",
    ),
    _: str = Depends(get_api_key),
):
    """
    Retrieve general MQTT messages filtered by topic.

    This endpoint returns MQTT messages that don't fit into specialized categories
    (like battery, temperature, etc). A topic filter is required.

    ## Parameters
    - **topic**: Filter by topic name (required, partial matching supported)
    - **limit**: Maximum number of records to return (default: 100, max: 1000)
    - **offset**: Number of records to skip, useful for pagination (default: 0)
    - **hours**: Get data from the last X hours (default: 24)
    - **time_offset**: Offset in hours from current time (e.g., 336 = start from 2 weeks ago)

    ## Special Topics
    For specialized data topics, please use their dedicated endpoints:
    - For battery data: `/battery`
    - For temperature data: `/temperature`
    - For gyroscope data: `/gyro`
    - For configuration data: `/config`

    ## Response
    Returns an array of message records, each containing:
    - **id**: Unique record identifier
    - **topic**: The MQTT topic the message was sent to
    - **message**: The content of the message
    - **timestamp**: When the message was received

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
    Publishing to specialized data topics (battery, temperature, gyro, config)
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
