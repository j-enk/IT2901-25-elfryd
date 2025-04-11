from fastapi import APIRouter, HTTPException, Depends, Query, Body
from typing import List, Optional
import re

from core.database import get_connection, query_config_data
from core.models import ConfigData
from core.mqtt import publish_message
from core.config import BASE_CONFIG_COMMANDS, COMMAND_PATTERNS
from api.dependencies import get_api_key

router = APIRouter(tags=["Configuration"])

@router.get("/config", response_model=List[ConfigData], summary="Get configuration data")
def get_config_data(
    limit: int = Query(
        20, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        168, ge=0, description="Get data from the last X hours"
    ),
    time_offset: Optional[float] = Query(
        None, ge=0, description="Offset in hours from current time (e.g., 336 = start from 2 weeks ago)"
    ),
    _: str = Depends(get_api_key),
):
    """
    Retrieve configuration commands sent to devices.
    
    ## Parameters
    - **limit**: Maximum number of records to return (default: 20, max: 1000)
    - **hours**: Get data from the last X hours (default: 168)
    - **time_offset**: Offset in hours from current time (e.g., 336 = start from 2 weeks ago)
    
    ## Response
    Returns an array of configuration records, each containing:
    - **id**: Unique record identifier
    - **command**: Configuration command sent
    - **topic**: Topic the command was sent to
    - **timestamp**: Server timestamp of when the command was processed
    
    ## Authentication
    Requires API key in the X-API-Key header
    """
    try:
        conn = get_connection()
        results = query_config_data(
            conn,
            limit, 
            hours,
            time_offset
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
    Send a configuration command to connected devices via MQTT.
    
    ## Command Formats
    The following command formats are supported:
    
    ### Sensor Configuration
    - **battery**: Enable battery reading with default interval
    - **battery X**: Enable battery reading with X interval (X=0 disables)
    - **temp**: Enable temperature reading with default interval
    - **temp X**: Enable temperature reading with X interval (X=0 disables)
    - **gyro**: Enable gyroscope reading with default interval
    - **gyro X**: Enable gyroscope reading with X interval (X=0 disables)
    
    ### Multiple Commands
    Multiple commands can be sent at once by separating them with the "|" character.
    
    ## Examples
    - `battery` - Enable battery readings with default interval
    - `temp 30` - Enable temperature readings every 30 seconds
    - `gyro 0` - Disable gyroscope readings
    - `battery 10|temp 30|gyro 0` - Configure multiple sensors in one request
    
    ## Request Body
    - **command**: The configuration command string
    
    ## Response
    - JSON object with success status and details
    
    ## Authentication
    Requires API key in the X-API-Key header
    """
    # Validate config command format
    if not command.strip():
        raise HTTPException(status_code=400, detail="Command cannot be empty")
    
    command = command.strip()
    
    # Split into multiple commands if needed
    commands = command.split("|")
    invalid_commands = []
    
    # Validate each command
    for cmd in commands:
        cmd = cmd.strip()
        if not cmd:
            continue
        
        # Check if command matches any valid pattern
        is_valid = False
        for pattern in COMMAND_PATTERNS.values():
            if re.match(pattern, cmd):
                is_valid = True
                break
        
        if not is_valid:
            invalid_commands.append(cmd)
    
    # If any commands are invalid, return error
    if invalid_commands:
        valid_formats = [
            f"{base}" for base in BASE_CONFIG_COMMANDS
        ] + [
            f"{base} [number]" for base in BASE_CONFIG_COMMANDS
        ] + ["freq [number]"]
        
        raise HTTPException(
            status_code=400,
            detail=f"Invalid commands: {', '.join(invalid_commands)}. Valid formats: {', '.join(valid_formats)}"
        )
    
    # Set fixed topic for all configuration commands
    topic = "elfryd/config/send"
    
    # Publish to MQTT broker
    try:
        publish_message(topic, command)
        return {
            "success": True, 
            "message": "Configuration commands sent", 
            "topic": topic,
            "commands": [cmd.strip() for cmd in commands if cmd.strip()]
        }
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Failed to publish message: {str(e)}"
        )