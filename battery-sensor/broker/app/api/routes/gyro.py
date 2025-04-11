from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Optional

from core.database import get_connection, query_specific_data
from core.models import GyroData
from api.dependencies import get_api_key

router = APIRouter(tags=["Gyroscope"])

@router.get("/gyro", response_model=List[GyroData], summary="Get gyroscope data")
def get_gyro_data(
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Retrieve gyroscope and accelerometer data from connected devices.
    
    This endpoint returns motion sensor data collected from the IoT devices
    in the Elfryd system, including both accelerometer and gyroscope values.
    
    ## Parameters
    - **limit**: Maximum number of records to return (default: 100, max: 1000)
    - **hours**: Get data from the last X hours (default: 24)
    
    ## Response
    Returns an array of motion sensor records, each containing:
    - **id**: Unique record identifier
    - **accel_x**: Accelerometer X-axis reading
    - **accel_y**: Accelerometer Y-axis reading
    - **accel_z**: Accelerometer Z-axis reading
    - **gyro_x**: Gyroscope X-axis reading (angular velocity)
    - **gyro_y**: Gyroscope Y-axis reading (angular velocity)
    - **gyro_z**: Gyroscope Z-axis reading (angular velocity)
    - **device_timestamp**: Timestamp from the device (Unix timestamp)
    - **timestamp**: Server timestamp when the reading was received
    
    ## Authentication
    Requires API key in the X-API-Key header
    """
    try:
        conn = get_connection()
        results = query_specific_data(
            conn, 
            "elfryd_gyro", 
            None, 
            None, 
            limit, 
            hours
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")