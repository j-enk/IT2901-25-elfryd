from fastapi import APIRouter, HTTPException, Depends, Query, Body
from typing import List, Optional

from core.database import get_connection, query_specific_data
from core.models import ConfigData, MQTTMessage
from core.mqtt import publish_message
from core.config import VALID_CONFIG_COMMANDS
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
    command: str = Body(..., description="Configuration command to send", embed=True),
    _: str = Depends(get_api_key),
):
    """
    Send a configuration command via MQTT to the predefined config topic
    """
    # Validate config command format
    if not command.strip():
        raise HTTPException(status_code=400, detail="Command cannot be empty")
    
    command = command.strip()
    
    # Validate command format
    if command.startswith("freq "):
        # Handle frequency command
        parts = command.split()
        if len(parts) != 2 or not parts[1].isdigit():
            raise HTTPException(
                status_code=400, 
                detail="Frequency command must be in format: freq [number]"
            )
    else:
        # Handle sensor type commands
        if command not in VALID_CONFIG_COMMANDS:
            raise HTTPException(
                status_code=400, 
                detail=f"Invalid command. Must be one of: {', '.join(VALID_CONFIG_COMMANDS)} or 'freq [number]'"
            )
    
    # Set fixed topic for all configuration commands
    topic = "elfryd/config/send"
    
    # Publish to MQTT broker
    try:
        publish_message(topic, command)
        return {"success": True, "message": "Configuration command sent", "topic": topic}
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Failed to publish message: {str(e)}"
        )