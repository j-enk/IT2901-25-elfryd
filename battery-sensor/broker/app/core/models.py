from pydantic import BaseModel, Field
from datetime import datetime
from typing import Optional, List


# Specialized data models (for internal use)
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


# Base message model for MQTT publishing
class MQTTMessage(BaseModel):
    topic: str = Field(
        ..., description="The MQTT topic to publish to", example="random/topic"
    )
    message: str = Field(
        ..., description="The message payload to publish", example="Hello, world!"
    )

    class Config:
        schema_extra = {
            "example": {"topic": "random/topic", "message": "Hello, world!"}
        }


# Base stored message model
class StoredMessage(BaseModel):
    id: int = Field(..., description="Unique identifier for the message", example=42)
    topic: str = Field(
        ...,
        description="The MQTT topic the message was published to",
        example="test/topic",
    )
    message: str = Field(
        ..., description="The message payload", example="This is a test message"
    )
    timestamp: datetime = Field(
        ...,
        description="Server timestamp when the message was received",
        example="2025-04-10T14:23:45.123456",
    )

    class Config:
        schema_extra = {
            "example": {
                "id": 42,
                "topic": "test/topic",
                "message": "This is a test message",
                "timestamp": "2025-04-10T14:23:45.123456",
            }
        }


# API response models with examples
class BatteryDataResponse(BaseModel):
    id: Optional[int] = Field(None, description="Unique record identifier", example=123)
    battery_id: int = Field(..., description="Identifier of the battery", example=1)
    voltage: int = Field(
        ..., description="Battery voltage in millivolts (mV)", example=3824
    )
    device_timestamp: int = Field(
        ...,
        description="Timestamp of the measurement on the device (Unix timestamp)",
        example=1712841632,
    )

    class Config:
        schema_extra = {
            "example": {
                "id": 123,
                "battery_id": 1,
                "voltage": 3824,
                "device_timestamp": 1712841632,
            }
        }


class TemperatureDataResponse(BaseModel):
    id: Optional[int] = Field(None, description="Unique record identifier", example=456)
    temperature: int = Field(
        ..., description="Temperature reading in degrees Celsius", example=25
    )
    device_timestamp: int = Field(
        ...,
        description="Timestamp of the measurement on the device (Unix timestamp)",
        example=1712841730,
    )

    class Config:
        schema_extra = {
            "example": {"id": 456, "temperature": 25, "device_timestamp": 1712841730}
        }


class GyroDataResponse(BaseModel):
    id: Optional[int] = Field(None, description="Unique record identifier", example=789)
    accel_x: int = Field(
        ..., description="Accelerometer X-axis reading", example=-4991017
    )
    accel_y: int = Field(
        ..., description="Accelerometer Y-axis reading", example=-4984009
    )
    accel_z: int = Field(
        ..., description="Accelerometer Z-axis reading", example=4979460
    )
    gyro_x: int = Field(..., description="Gyroscope X-axis reading", example=-239841)
    gyro_y: int = Field(..., description="Gyroscope Y-axis reading", example=241869)
    gyro_z: int = Field(..., description="Gyroscope Z-axis reading", example=-243303)
    device_timestamp: int = Field(
        ...,
        description="Timestamp of the measurement on the device (Unix timestamp)",
        example=1712841803,
    )

    class Config:
        schema_extra = {
            "example": {
                "id": 789,
                "accel_x": -4991017,
                "accel_y": -4984009,
                "accel_z": 4979460,
                "gyro_x": -239841,
                "gyro_y": 241869,
                "gyro_z": -243303,
                "device_timestamp": 1712841803,
            }
        }


class ConfigDataResponse(BaseModel):
    id: Optional[int] = Field(
        None, description="Unique identifier for the command", example=1
    )
    command: str = Field(
        ..., description="Configuration command to be sent", example="battery 10"
    )
    topic: str = Field(
        ..., description="MQTT topic for the command", example="elfryd/config/send"
    )
    timestamp: Optional[datetime] = Field(
        None,
        description="Server timestamp when the command was sent",
        example="2025-04-10T14:23:45.123456",
    )

    class Config:
        schema_extra = {
            "example": {
                "id": 1,
                "command": "battery 10",
                "topic": "elfryd/config/send",
                "timestamp": "2025-04-10T14:23:45.123456",
            }
        }


class ConfigCommandResponse(BaseModel):
    success: bool = Field(
        ...,
        description="Indicates if the commands were sent successfully",
        example=True,
    )
    message: str = Field(
        ..., description="Status message", example="Configuration commands sent"
    )
    topic: str = Field(
        ...,
        description="The topic the commands were sent to",
        example="elfryd/config/send",
    )
    commands: List[str] = Field(
        ...,
        description="List of commands that were sent",
        example=["battery 10", "temp 30"],
    )

    class Config:
        schema_extra = {
            "example": {
                "success": True,
                "message": "Configuration commands sent",
                "topic": "elfryd/config/send",
                "commands": ["battery 10", "temp 30", "gyro 0"],
            }
        }


class HealthCheckResponse(BaseModel):
    status: str = Field(
        ..., description="Overall system health status", example="healthy"
    )
    database: str = Field(
        ..., description="Database connection status", example="connected"
    )
    mqtt: str = Field(
        ..., description="MQTT broker connection status", example="connected"
    )

    class Config:
        schema_extra = {
            "example": {
                "status": "healthy",
                "database": "connected",
                "mqtt": "connected",
            }
        }


class TopicListResponse(BaseModel):
    topics: List[str] = Field(
        ...,
        description="List of unique topics in the database",
        example=[
            "elfryd/battery",
            "elfryd/temp",
            "test/verification",
            "elfryd/config/send",
            "random/topic",
        ],
    )

    class Config:
        schema_extra = {
            "example": {
                "topics": [
                    "elfryd/battery",
                    "elfryd/temp",
                    "test/verification",
                    "elfryd/config/send",
                    "random/topic",
                ]
            }
        }


class MessagePublishResponse(BaseModel):
    success: bool = Field(
        ...,
        description="Indicates if the message was published successfully",
        example=True,
    )
    topic: str = Field(
        ...,
        description="The topic the message was published to",
        example="test/message",
    )

    class Config:
        schema_extra = {"example": {"success": True, "topic": "test/message"}}


class RootResponse(BaseModel):
    message: str = Field(
        ..., 
        description="Confirmation message that the API is running",
        example="Elfryd MQTT API is running"
    )
    version: str = Field(
        ..., 
        description="Current API version",
        example="1.0"
    )
    
    class Config:
        schema_extra = {
            "example": {
                "message": "Elfryd MQTT API is running",
                "version": "1.0"
            }
        }