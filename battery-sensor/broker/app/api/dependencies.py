import os
from fastapi import Security, HTTPException
from fastapi.security import APIKeyHeader
from starlette.status import HTTP_403_FORBIDDEN

# API configuration
API_CONFIG = {
    "title": "Elfryd MQTT API",
    "description": "API for interacting with MQTT broker and retrieving stored messages",
    "version": "1.0.0",
    "openapi_tags": [
        {
            "name": "Authentication",
            "description": "Requires X-API-Key header for protected endpoints",
        }
    ]
}

# Authentication
API_KEY = os.getenv("API_KEY")
if not API_KEY:
    raise ValueError("API_KEY environment variable not set")

api_key_header = APIKeyHeader(name="X-API-Key", auto_error=False)

async def get_api_key(api_key: str = Security(api_key_header)):
    """
    Dependency to validate API key for protected endpoints
    """
    if api_key != API_KEY:
        raise HTTPException(
            status_code=HTTP_403_FORBIDDEN,
            detail="Invalid API key",
        )
    return api_key