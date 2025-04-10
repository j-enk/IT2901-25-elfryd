from fastapi import APIRouter, HTTPException, Depends

from core.database import get_connection, get_topics
from api.dependencies import get_api_key

router = APIRouter(tags=["Topics"])

@router.get("/topics", response_model=list, summary="Get unique topics")
def get_all_topics(
    _: str = Depends(get_api_key),
):
    """
    Get a list of all unique topics in the database
    """
    try:
        conn = get_connection()
        topic_list = get_topics(conn)
        conn.close()
        return topic_list
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")