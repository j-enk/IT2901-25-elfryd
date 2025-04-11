import os
from fastapi import HTTPException, Security
from fastapi.security import APIKeyHeader
from pathlib import Path

# Get the current directory
current_dir = Path(__file__).parent

# API configuration for FastAPI app
API_CONFIG = {
    "title": "Elfryd MQTT Broker API",
    "description_path": str(current_dir / "description.md"),  # Use Markdown file path
    "version": "1.0.0",
    "docs_url": "/docs",
    "redoc_url": "/redoc",
}

# Rest of the file unchanged

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
