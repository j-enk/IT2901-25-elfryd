from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Optional

from core.database import get_connection, query_sensor_data
from core.models import TemperatureDataResponse
from api.dependencies import get_api_key

router = APIRouter(tags=["Temperature"])


@router.get(
    "/temperature", response_model=List[TemperatureDataResponse], summary="Get temperature data"
)
def get_temperature_data(
    limit: int = Query(
        20, ge=0, le=1000000, description="Maximum number of records to return (0 for no limit)"
    ),
    hours: Optional[float] = Query(
        168, ge=0, description="Get data from the last X hours"
    ),
    time_offset: Optional[float] = Query(
        None,
        ge=0,
        description="Offset in hours from current time (e.g., 336 = start from 2 weeks ago)",
    ),
    _: str = Depends(get_api_key),
):
    """
    Retrieve temperature measurements from connected devices.

    This endpoint returns temperature data collected from IoT devices
    in the Elfryd system. Temperature values are in degrees Celsius.

    ## Parameters
    - **limit**: Maximum number of records to return (default: 20, max: 1000000). When used with the hours parameter, 
      data points will be evenly distributed across the time range instead of just returning the newest records.
      Set to 0 to disable limiting and return all data points in the time range.
    - **hours**: Get data from the last X hours (default: 168)
    - **time_offset**: Offset in hours from current time (e.g., 336 = start from 2 weeks ago)

    ## Response
    Returns an array of temperature records, each containing:
    - **id**: Unique record identifier
    - **temperature**: Temperature in degrees Celsius
    - **device_timestamp**: Timestamp of the measurement on the device (Unix timestamp)

    ## Authentication
    Requires API key in the X-API-Key header
    """
    try:
        conn = get_connection()
        results = query_sensor_data(
            conn, "elfryd_temp", None, None, limit, hours, time_offset
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")
