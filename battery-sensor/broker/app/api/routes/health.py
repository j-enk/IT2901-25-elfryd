from fastapi import APIRouter
from core.database import get_connection
from core.mqtt import create_mqtt_client
from core.models import HealthCheckResponse

router = APIRouter(tags=["Health"])


@router.get("/health", response_model=HealthCheckResponse, summary="Health check")
def health_check():
    """
    Check the health of the API and its connections to the database and MQTT broker.

    ## Response
    Returns a health status object containing:
    - **status**: Overall system health status (healthy/unhealthy)
    - **database**: Database connection status
    - **mqtt**: MQTT broker connection status

    ## Authentication
    This endpoint is public and does not require authentication.
    """
    health_status = {"status": "healthy", "database": "connected", "mqtt": "connected"}

    # Check database connection
    try:
        conn = get_connection()
        cursor = conn.cursor()
        cursor.execute("SELECT 1")
        cursor.close()
        conn.close()
    except Exception as db_error:
        health_status["database"] = f"error: {str(db_error)}"
        health_status["status"] = "unhealthy"

    # Check MQTT connection
    try:
        client = create_mqtt_client("health-check")
        client.disconnect()
    except Exception as mqtt_error:
        health_status["mqtt"] = f"error: {str(mqtt_error)}"
        health_status["status"] = "unhealthy"

    return health_status
