from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from api.dependencies import API_CONFIG
from api.routes import routers
from core.models import RootResponse

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

# Include all routers from the routes package
for router in routers:
    app.include_router(router)

# Root endpoint
@app.get("/", response_model=RootResponse, summary="Root endpoint")
def read_root():
    """
    Root endpoint providing basic API information.
    
    This endpoint serves as a simple health check and API version indicator.
    It's useful for verifying that the API is up and running.
    For more detailed health checks, please use the `/health` endpoint.
    
    ## Response
    Returns a JSON object containing:
    - **message**: Confirmation that the API is running
    - **version**: Current API version
    
    ## Authentication
    This endpoint is public and does not require authentication.
    """
    return {"message": "Elfryd MQTT API is running", "version": "1.0"}