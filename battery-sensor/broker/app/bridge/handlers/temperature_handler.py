from pydantic import ValidationError
from psycopg2 import sql
from core.models import TemperatureData
from core.database import get_connection

def process_message(topic: str, payload: str):
    """Process and store temperature data from string format"""
    try:
        # Parse payload - Format: "2/Temp/Timestamp"
        parts = payload.strip().split('/')
        if len(parts) != 3:
            print(f"Invalid temperature data format: {payload}")
            return
            
        try:
            # Create TemperatureData model
            temp_data = TemperatureData(
                sensor_id=int(parts[0]),
                temperature=int(parts[1]),
                device_timestamp=int(parts[2]),
                topic=topic,
                raw_message=payload
            )
            
            # Store in database
            store_temperature_data(temp_data)
            
        except (ValueError, IndexError) as e:
            print(f"Error parsing temperature data: {str(e)}")
            
    except ValidationError as e:
        print(f"Validation error in temperature message: {str(e)}")
    except Exception as e:
        print(f"Error processing temperature message: {str(e)}")

def store_temperature_data(data: TemperatureData):
    """Store validated temperature data in database"""
    try:
        conn = get_connection()
        cursor = conn.cursor()
        
        insert_query = sql.SQL("""
            INSERT INTO {} (sensor_id, temperature, device_timestamp, topic, raw_message)
            VALUES (%s, %s, %s, %s, %s)
        """).format(sql.Identifier(f"elfryd_temp"))
        
        cursor.execute(insert_query, (
            data.sensor_id,
            data.temperature,
            data.device_timestamp,
            data.topic,
            data.raw_message
        ))
        conn.commit()
        cursor.close()
        conn.close()
        
    except Exception as e:
        print(f"Error storing temperature data: {str(e)}")