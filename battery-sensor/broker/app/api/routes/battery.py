from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Optional

from core.database import get_connection, query_specific_data
from core.models import BatteryData
from api.dependencies import get_api_key

router = APIRouter(tags=["Battery"])

@router.get("/battery", response_model=List[BatteryData], summary="Get battery data")
def get_battery_data(
    battery_id: Optional[int] = Query(None, description="Filter by battery ID"),
    limit: int = Query(
        100, ge=1, le=1000, description="Maximum number of records to return"
    ),
    hours: Optional[float] = Query(
        24, ge=0, description="Get data from the last X hours"
    ),
    _: str = Depends(get_api_key),
):
    """
    Get battery data with optional filtering
    """
    try:
        conn = get_connection()
        results = query_specific_data(
            conn, 
            "elfryd_battery", 
            "battery_id", 
            battery_id, 
            limit, 
            hours
        )
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")