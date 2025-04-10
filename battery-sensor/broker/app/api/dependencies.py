from fastapi import Security, HTTPException
from starlette.status import HTTP_403_FORBIDDEN

from core.config import api_key_header, API_KEY

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