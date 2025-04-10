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
    id: Optional[int] = None  # Optional for creating new records
    battery_id: int
    voltage: int
    device_timestamp: int
    topic: str
    raw_message: str
    timestamp: Optional[datetime] = None  # Optional for creating new records

class TemperatureData(BaseModel):
    id: Optional[int] = None
    sensor_id: int
    temperature: int
    device_timestamp: int
    topic: str
    raw_message: str
    timestamp: Optional[datetime] = None

class GyroData(BaseModel):
    id: Optional[int] = None
    sensor_id: int
    accel_x: int
    accel_y: int
    accel_z: int
    gyro_x: int
    gyro_y: int
    gyro_z: int
    device_timestamp: int
    topic: str
    raw_message: str
    timestamp: Optional[datetime] = None

class ConfigData(BaseModel):
    id: Optional[int] = None
    command: str
    topic: str
    raw_message: str
    timestamp: Optional[datetime] = None