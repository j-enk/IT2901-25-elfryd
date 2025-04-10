from fastapi import APIRouter, HTTPException
import psycopg2

from core.database import get_connection
from core.mqtt import create_mqtt_client

router = APIRouter(tags=["Health"])

@router.get("/health", response_model=dict, summary="Health check")
def health_check():
    """
    Check the health of the API and its connections to the database and MQTT broker
    """
    health_status = {
        "status": "healthy",
        "database": "connected",
        "mqtt": "connected"
    }
    
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