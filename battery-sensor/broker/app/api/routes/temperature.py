from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Optional

from core.database import get_connection, query_sensor_data
from core.models import TemperatureData
from api.dependencies import get_api_key

router = APIRouter(tags=["Temperature"])

@router.get("/temperature", response_model=List[TemperatureData], summary="Get temperature data")
def get_temperature_data(
    limit: int = Query(
        20, ge=1, le=10000, description="Maximum number of records to return"
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
    Retrieve temperature measurements from connected devices.
    
    ## Parameters
    - **limit**: Maximum number of records to return (default: 20, max: 10000)
    - **hours**: Get data from the last X hours (default: 168)
    - **time_offset**: Offset in hours from current time (e.g., 336 = start from 2 weeks ago)
    
    ## Response
    Returns an array of temperature records, each containing:
    - **id**: Unique record identifier
    - **temperature**: Temperature reading in degrees Celsius
    - **device_timestamp**: Timestamp of the measurement on the device (Unix timestamp)
    
    ## Authentication
    Requires API key in the X-API-Key header
    """
    try:
        conn = get_connection()
        results = query_sensor_data(
            conn, 
            "elfryd_temp", 
            None, 
            None, 
            limit, 
            hours,
            time_offset
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")