from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from api.dependencies import API_CONFIG
from api.routes import routers

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
@app.get("/", summary="Root endpoint")
def read_root():
    """
    Root endpoint providing basic API information.
    """
    return {"message": "Elfryd MQTT API is running", "version": "1.0"}