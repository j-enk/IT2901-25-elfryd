from pydantic import BaseModel
from datetime import datetime
from typing import Optional

# Base message model for MQTT publishing
class MQTTMessage(BaseModel):
    topic: str
    message: str

# Base stored message model
class StoredMessage(BaseModel):
    id: int
    topic: str
    message: str
    timestamp: datetime

# Specialized data models
class BatteryData(BaseModel):
    id: Optional[int] = None
    battery_id: int
    voltage: int
    device_timestamp: int
    timestamp: Optional[datetime] = None

class TemperatureData(BaseModel):
    id: Optional[int] = None
    temperature: int
    device_timestamp: int
    timestamp: Optional[datetime] = None

class GyroData(BaseModel):
    id: Optional[int] = None
    accel_x: int
    accel_y: int
    accel_z: int
    gyro_x: int
    gyro_y: int
    gyro_z: int
    device_timestamp: int
    timestamp: Optional[datetime] = None

class ConfigData(BaseModel):
    id: Optional[int] = None
    command: str
    topic: str
    timestamp: Optional[datetime] = None