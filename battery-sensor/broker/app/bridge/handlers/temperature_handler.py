from pydantic import ValidationError
from psycopg2 import sql
from core.models import TemperatureData
from core.database import get_connection

def process_message(payload: str):
    """Process and store temperature data from string format"""
    try:
        # Parse payload: "Temp/Timestamp"
        parts = payload.strip().split('/')
        if len(parts) != 2:
            print(f"Invalid temperature data format: {payload}")
            return
            
        try:
            # Create TemperatureData model
            temp_data = TemperatureData(
                temperature=int(parts[0]),
                device_timestamp=int(parts[1])
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
            INSERT INTO {} (temperature, device_timestamp)
            VALUES (%s, %s)
        """).format(sql.Identifier(f"elfryd_temp"))
        
        cursor.execute(insert_query, (
            data.temperature,
            data.device_timestamp
        ))
        conn.commit()
        cursor.close()
        conn.close()
        
    except Exception as e:
        print(f"Error storing temperature data: {str(e)}")