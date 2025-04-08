from fastapi import FastAPI, HTTPException, Depends, BackgroundTasks, Query
from fastapi.middleware.cors import CORSMiddleware
import paho.mqtt.client as mqtt
import psycopg2
from psycopg2 import sql
import os
import ssl
import time
from typing import List, Dict, Any, Optional
from datetime import datetime, timedelta
from pydantic import BaseModel

from fastapi.security.api_key import APIKeyHeader
from fastapi import Security, HTTPException, Depends
from starlette.status import HTTP_403_FORBIDDEN
import secrets

app = FastAPI(
    title="Elfryd MQTT API", 
    description="API for interacting with MQTT broker and retrieving stored messages",
    openapi_tags=[{"name": "Authentication", "description": "Requires X-API-Key header for protected endpoints"}]
)
# Enable CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In production, replace with specific origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Database connection parameters - only accessible within Docker network
DB_HOST = "timescaledb"
DB_PORT = 5432
DB_NAME = "mqtt_data"
DB_USER = "myuser"
DB_PASSWORD = "mypassword"

# MQTT broker parameters
MQTT_BROKER = "mqtt-broker"
MQTT_PORT = 1883
MQTT_TLS_PORT = 8885
USE_TLS = os.environ.get('USE_TLS', 'false').lower() == 'true'

# Pydantic models
class MQTTMessage(BaseModel):
    topic: str
    message: str

class StoredMessage(BaseModel):
    id: int
    topic: str
    message: str
    timestamp: datetime

# Database connection pool
def get_db_connection():
    try:
        conn = psycopg2.connect(
            host=DB_HOST,
            port=DB_PORT,
            dbname=DB_NAME,
            user=DB_USER,
            password=DB_PASSWORD
        )
        return conn
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database connection failed: {str(e)}")

# MQTT client setup
def get_mqtt_client():
    client = mqtt.Client(client_id="api-client", protocol=mqtt.MQTTv5, callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
    
    if USE_TLS:
        client.tls_set(
            ca_certs="/app/certs/ca.crt", 
            cert_reqs=ssl.CERT_REQUIRED,
            tls_version=ssl.PROTOCOL_TLSv1_2
        )
        client.tls_insecure_set(False)
        port = MQTT_TLS_PORT
    else:
        port = MQTT_PORT
        
    try:
        client.connect(MQTT_BROKER, port, 60)
        return client
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"MQTT connection failed: {str(e)}")
    
# Define API key header
API_KEY = os.environ.get("API_KEY", "your-secure-api-key")
api_key_header = APIKeyHeader(name="X-API-Key", auto_error=False)

# Create a dependency
async def get_api_key(api_key: str = Security(api_key_header)):
    if api_key != API_KEY:
        raise HTTPException(
            status_code=HTTP_403_FORBIDDEN,
            detail="Invalid API key",
        )
    return api_key

# API Endpoints
@app.get("/", summary="Root endpoint")
def read_root():
    """
    Root endpoint providing basic API information.
    """
    return {"message": "Elfryd MQTT API is running", "version": "1.0"}

@app.post("/messages", summary="Publish an MQTT message")
def publish_message(message: MQTTMessage):
    """
    Publish a message to the MQTT broker
    """
    try:
        client = get_mqtt_client()
        result = client.publish(message.topic, message.message, qos=1)
        client.disconnect()
        
        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            raise HTTPException(status_code=500, detail=f"Failed to publish message: {mqtt.error_string(result.rc)}")
            
        return {"status": "success", "message": "Message published successfully"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error publishing message: {str(e)}")

@app.get("/messages", response_model=List[StoredMessage], summary="Get stored messages")
def get_messages(
    api_key: str = Depends(get_api_key),
    topic: Optional[str] = Query(None, description="Filter by topic (supports SQL LIKE patterns)"),
    limit: int = Query(100, ge=1, le=1000, description="Maximum number of messages to return"),
    offset: int = Query(0, ge=0, description="Number of messages to skip"),
    hours: Optional[float] = Query(None, ge=0, description="Get messages from the last X hours")
):
    """
    Get messages stored in the database with optional filtering
    """
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        
        # Build the query based on parameters
        query = "SELECT id, topic, message, timestamp FROM mqtt_messages WHERE 1=1"
        params = []
        
        if topic:
            query += " AND topic LIKE %s"
            params.append(f"%{topic}%")
            
        if hours is not None:
            query += " AND timestamp > %s"
            params.append(datetime.now() - timedelta(hours=hours))
            
        query += " ORDER BY timestamp DESC LIMIT %s OFFSET %s"
        params.extend([limit, offset])
        
        cur.execute(query, params)
        results = [
            {
                "id": row[0],
                "topic": row[1],
                "message": row[2],
                "timestamp": row[3]
            }
            for row in cur.fetchall()
        ]
        
        cur.close()
        conn.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@app.get("/topics", summary="Get list of topics")
def get_topics():
    """
    Get a list of all unique topics in the database
    """
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        cur.execute("SELECT DISTINCT topic FROM mqtt_messages ORDER BY topic")
        topics = [row[0] for row in cur.fetchall()]
        cur.close()
        conn.close()
        return {"topics": topics}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

@app.get("/health", summary="Check API health")
def health_check():
    """
    Check the health of connections to MQTT broker and database
    """
    health_status = {"status": "healthy", "mqtt": False, "database": False}
    
    # Check DB connection
    try:
        conn = get_db_connection()
        cur = conn.cursor()
        cur.execute("SELECT 1")
        health_status["database"] = True
        cur.close()
        conn.close()
    except Exception as e:
        health_status["status"] = "degraded"
        health_status["database_error"] = str(e)
    
    # Check MQTT connection
    try:
        client = get_mqtt_client()
        client.disconnect()
        health_status["mqtt"] = True
    except Exception as e:
        health_status["status"] = "degraded"
        health_status["mqtt_error"] = str(e)
        
    if not health_status["mqtt"] or not health_status["database"]:
        health_status["status"] = "degraded"
        
    return health_status
