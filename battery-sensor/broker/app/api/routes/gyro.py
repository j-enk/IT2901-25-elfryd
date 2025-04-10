from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Optional

from core.database import get_connection, query_specific_data
from core.models import GyroData
from dependencies import get_api_key

router = APIRouter(tags=["Gyroscope"])

@router.get("/gyro", response_model=List[GyroData], summary="Get gyroscope data")
def get_gyro_data(
    sensor_id: Optional[int] = Query(None, description="Filter by sensor ID"),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get gyroscope and accelerometer data with optional filtering
    """
    try:
        conn = get_connection()
        results = query_specific_data(
            conn, 
            "elfryd_gyro", 
            "sensor_id", 
            sensor_id, 
            limit, 
            hours
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")