from fastapi import APIRouter, HTTPException, Depends, Query, Body
from typing import List, Optional
import re

from core.database import get_connection, query_specific_data
from core.models import ConfigData, MQTTMessage
from core.mqtt import publish_message
from core.config import CONFIG_FREQ_PATTERN, VALID_CONFIG_COMMANDS
from api.dependencies import get_api_key

router = APIRouter(tags=["Configuration"])

@router.get("/config", response_model=List[ConfigData], summary="Get configuration messages")
def get_config_data(
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get configuration messages
    """
    try:
        conn = get_connection()
        results = query_specific_data(
            conn, 
            "elfryd_config", 
            None, 
            None, 
            limit, 
            hours
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@router.post("/config/send", response_model=dict, summary="Send configuration command")
def send_config_command(
    message: MQTTMessage = Body(...),
    _: str = Depends(get_api_key),
):
    """
    Send a configuration command via MQTT
    """
    # Validate config command format
    if not message.message.strip():
        raise HTTPException(status_code=400, detail="Message cannot be empty")
    
    command_parts = message.message.strip().split()
    
    # Command format validation
    if len(command_parts) != 2:
        raise HTTPException(
            status_code=400, 
            detail="Command must be in format: [command_type] [frequency]"
        )
    
    command_type, frequency = command_parts
    
    # Validate command type
    if command_type not in VALID_CONFIG_COMMANDS:
        raise HTTPException(
            status_code=400, 
            detail=f"Invalid command type. Must be one of: {', '.join(VALID_CONFIG_COMMANDS)}"
        )
    
    # Validate frequency format
    if not re.match(CONFIG_FREQ_PATTERN, frequency):
        raise HTTPException(
            status_code=400, 
            detail="Frequency must be in format: freq[number]"
        )
    
    # Topic validation
    if not message.topic.strip():
        raise HTTPException(status_code=400, detail="Topic cannot be empty")
    
    # Publish to MQTT broker
    try:
        publish_message(message.topic, message.message)
        return {"success": True, "message": "Configuration command sent"}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Failed to publish message: {str(e)}"
        )