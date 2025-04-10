from pydantic import ValidationError
from psycopg2 import sql
from core.models import GyroData
from core.database import get_connection

def process_message(topic: str, payload: str):
    """Process and store gyroscope data from string format"""
    try:  
        # Parse payload - Format: "3/AccelX,AccelY,AccelZ/GyroX,GyroY,GyroZ/Timestamp"
        parts = payload.strip().split('/')
        if len(parts) != 4:
            print(f"Invalid gyro data format: {payload}")
            return
            
        try:
            # Parse accelerometer and gyroscope values
            accel_parts = parts[1].split(',')
            gyro_parts = parts[2].split(',')
            
            if len(accel_parts) != 3 or len(gyro_parts) != 3:
                print(f"Invalid gyro data components: {payload}")
                return
                
            # Create GyroData model
            gyro_data = GyroData(
                sensor_id=int(parts[0]),
                accel_x=int(accel_parts[0]),
                accel_y=int(accel_parts[1]),
                accel_z=int(accel_parts[2]),
                gyro_x=int(gyro_parts[0]),
                gyro_y=int(gyro_parts[1]),
                gyro_z=int(gyro_parts[2]),
                device_timestamp=int(parts[3]),
                topic=topic,
                raw_message=payload
            )
            
            # Store in database
            store_gyro_data(gyro_data)
            
        except (ValueError, IndexError) as e:
            print(f"Error parsing gyro data: {str(e)}")
            
    except ValidationError as e:
        print(f"Validation error in gyro message: {str(e)}")
    except Exception as e:
        print(f"Error processing gyro message: {str(e)}")

def store_gyro_data(data: GyroData):
    """Store validated gyro data in database"""
    try:
        conn = get_connection()
        cursor = conn.cursor()
        
        insert_query = sql.SQL("""
            INSERT INTO {} (sensor_id, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, 
                           device_timestamp, topic, raw_message)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """).format(sql.Identifier(f"elfryd_gyro"))
        
        cursor.execute(insert_query, (
            data.sensor_id,
            data.accel_x,
            data.accel_y,
            data.accel_z,
            data.gyro_x,
            data.gyro_y,
            data.gyro_z,
            data.device_timestamp,
            data.topic,
            data.raw_message
        ))
        conn.commit()
        cursor.close()
        conn.close()
        
    except Exception as e:
        print(f"Error storing gyro data: {str(e)}")