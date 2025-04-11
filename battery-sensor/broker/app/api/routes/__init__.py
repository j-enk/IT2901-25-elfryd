# Import all route modules
from api.routes import battery, temperature, gyro, config, messages, topics, health

# Create a list of all routers to be included
routers = [
    battery.router,
    temperature.router,
    gyro.router,
    config.router,
    messages.router,
    topics.router,
    health.router,
]

# This allows 'from api.routes import routers' pattern
__all__ = [
    "routers",
    "battery",
    "temperature",
    "gyro",
    "config",
    "messages",
    "topics",
    "health",
]
