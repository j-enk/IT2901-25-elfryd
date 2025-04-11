from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Optional

from core.database import get_connection, query_specific_data
from core.models import BatteryData
from api.dependencies import get_api_key

router = APIRouter(tags=["Battery"])

@router.get("/battery", response_model=List[BatteryData], summary="Get battery data")
def get_battery_data(
    battery_id: Optional[int] = Query(
        None, description="Filter by battery ID"
    ),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Retrieve battery voltage measurements from connected devices.
    
    This endpoint returns battery voltage data collected from the IoT devices
    in the Elfryd system. The voltage values are in millivolts (mV).
    
    ## Parameters
    - **battery_id**: Filter by specific battery identifier (optional)
    - **limit**: Maximum number of records to return (default: 100, max: 1000)
    - **hours**: Get data from the last X hours (default: 24)
    
    ## Response
    Returns an array of battery records, each containing:
    - **id**: Unique record identifier
    - **battery_id**: Identifier of the battery
    - **voltage**: Battery voltage in millivolts (mV)
    - **device_timestamp**: Timestamp from the device (Unix timestamp)
    - **timestamp**: Server timestamp when the reading was received
    
    ## Authentication
    Requires API key in the X-API-Key header
    """
    try:
        conn = get_connection()
        filter_column = "battery_id" if battery_id is not None else None
        filter_value = battery_id if battery_id is not None else None
        
        results = query_specific_data(
            conn, 
            "elfryd_battery", 
            filter_column, 
            filter_value, 
            limit, 
            hours
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")