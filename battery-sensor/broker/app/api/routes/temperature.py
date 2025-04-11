from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Optional

from core.database import get_connection, query_specific_data
from core.models import TemperatureData
from api.dependencies import get_api_key

router = APIRouter(tags=["Temperature"])

@router.get("/temperature", response_model=List[TemperatureData], summary="Get temperature data")
def get_temperature_data(
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Retrieve temperature sensor readings from connected devices.
    
    This endpoint returns temperature sensor data collected from the IoT devices
    in the Elfryd system. The temperature values are in degrees Celsius.
    
    ## Parameters
    - **limit**: Maximum number of records to return (default: 100, max: 1000)
    - **hours**: Get data from the last X hours (default: 24)
    
    ## Response
    Returns an array of temperature records, each containing:
    - **id**: Unique record identifier
    - **temperature**: Temperature reading in degrees Celsius
    - **device_timestamp**: Timestamp from the device (Unix timestamp)
    - **timestamp**: Server timestamp when the reading was received
    
    ## Authentication
    Requires API key in the X-API-Key header
    """
    try:
        conn = get_connection()
        results = query_specific_data(
            conn, 
            "elfryd_temp", 
            None, 
            None, 
            limit, 
            hours
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")