import os
from fastapi import HTTPException, Security
from fastapi.security import APIKeyHeader

# API configuration for FastAPI app
API_CONFIG = {
    "title": "Elfryd MQTT API",
    "description": """
    API for the Elfryd IoT platform providing access to MQTT message data from device sensors.
    
    ## Features
    
    - **Sensor Data**: Access battery, temperature, and gyroscope readings
    - **Configuration**: Send and retrieve device configuration commands
    - **Messaging**: Access to all MQTT messages with topic filtering
    - **Security**: API key authentication for protected endpoints
    
    ## Authentication
    
    Most endpoints require an API key passed in the `X-API-Key` header.
    The API key is generated on API/broker installation and should be kept secret.
    
    ## Time-Based Querying
    
    All data endpoints support time-based querying with these parameters:
    - **hours**: Time window length in hours (e.g., 24 = last 24 hours of data)
    - **time_offset**: Start the time window this many hours in the past (e.g., 336 = 2 weeks ago)
    
    These can be combined to view historical data windows (e.g., hours=24, time_offset=336 to see data from exactly 2 weeks ago for a 24-hour period).
    """,
    "version": "1.0.0",
    "docs_url": "/docs",
    "redoc_url": "/redoc",
}

# Get API key from environment
API_KEY = os.getenv("API_KEY", "NO_KEY_SET")
api_key_header = APIKeyHeader(name="X-API-Key", auto_error=False)

def get_api_key(api_key: str = Security(api_key_header)):
    """
    Validate the API key from the request header.
    
    This dependency is used to protect sensitive endpoints from unauthorized access.
    The API key is compared against the one set during system installation.
    
    ## Parameters
    - **api_key**: The API key from the X-API-Key header
    
    ## Returns
    The API key if valid
    
    ## Raises
    - **401 Unauthorized**: If no API key is provided
    - **403 Forbidden**: If the provided API key is invalid
    """
    if not api_key:
        raise HTTPException(
            status_code=401,
            detail="API key is required in the X-API-Key header",
            headers={"WWW-Authenticate": "ApiKey"},
        )
    if api_key != API_KEY:
        raise HTTPException(
            status_code=403,
            detail="Invalid API key",
            headers={"WWW-Authenticate": "ApiKey"},
        )
    return api_key