from fastapi import APIRouter, HTTPException, Depends
from core.database import get_connection, get_topics
from core.models import TopicListResponse
from api.dependencies import get_api_key

router = APIRouter(tags=["Topics"])


@router.get("/topics", response_model=TopicListResponse, summary="Get unique topics")
def get_all_topics(
    _: str = Depends(get_api_key),
):
    """
    Get a list of all unique topics in the database.

    This endpoint retrieves all unique MQTT topics that have been recorded in the
    system, useful for discovering what data is available.

    ## Response
    Returns a list of unique topic strings.

    ## Authentication
    Requires API key in the X-API-Key header.
    """
    try:
        conn = get_connection()
        topic_list = get_topics(conn)
        conn.close()
        return {"topics": topic_list}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")
