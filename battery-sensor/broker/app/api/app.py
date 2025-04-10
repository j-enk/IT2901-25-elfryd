# Modified app.py that would use dependencies.py
from fastapi import FastAPI, Depends
from fastapi.middleware.cors import CORSMiddleware

from core.config import API_CONFIG
from api.dependencies import get_api_key  # Import the dependency instead of defining it

# Create FastAPI app
app = FastAPI(**API_CONFIG)

# Enable CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Import all route modules to register endpoints
from api.routes import battery, temperature, gyro, config, messages, topics, health

# Root endpoint
@app.get("/", summary="Root endpoint")
def read_root():
    """
    Root endpoint providing basic API information.
    """
    return {"message": "Elfryd MQTT API is running", "version": "1.0"}