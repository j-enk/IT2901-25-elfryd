from pydantic import ValidationError
from core.models import BatteryData
from core.database import get_connection

def process_message(topic: str, payload: str):
    """Process and store battery data from string format"""
    try:  
        # Parse payload - Format: "1X/Voltage/Timestamp"
        parts = payload.strip().split('/')
        if len(parts) != 3:
            print(f"Invalid battery data format: {payload}")
            return
            
        try:
            # Create BatteryData model
            battery_data = BatteryData(
                battery_id=int(parts[0]),
                voltage=int(parts[1]),
                device_timestamp=int(parts[2]),
                topic=topic,
                raw_message=payload
            )
            
            # Store in database
            store_battery_data(battery_data)
            
        except (ValueError, IndexError) as e:
            print(f"Error parsing battery data: {str(e)}")
            
    except ValidationError as e:
        print(f"Validation error in battery message: {str(e)}")
    except Exception as e:
        print(f"Error processing battery message: {str(e)}")

def store_battery_data(data: BatteryData):
    """Store validated battery data in database"""
    try:
        conn = get_connection()
        cursor = conn.cursor()
        
        # Create table if it doesn't exist
        create_table_query = """
        CREATE TABLE IF NOT EXISTS elfryd_battery (
            id SERIAL PRIMARY KEY,
            battery_id INTEGER NOT NULL,
            voltage INTEGER NOT NULL,
            device_timestamp BIGINT NOT NULL,
            topic TEXT NOT NULL,
            raw_message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_elfryd_battery_timestamp ON elfryd_battery (timestamp DESC);
        CREATE INDEX IF NOT EXISTS idx_elfryd_battery_battery_id ON elfryd_battery (battery_id);
        """
        cursor.execute(create_table_query)
        conn.commit()
        
        # Insert data
        insert_query = """
        INSERT INTO elfryd_battery (battery_id, voltage, device_timestamp, topic, raw_message)
        VALUES (%s, %s, %s, %s, %s)
        """
        cursor.execute(insert_query, (
            data.battery_id,
            data.voltage,
            data.device_timestamp,
            data.topic,
            data.raw_message
        ))
        conn.commit()
        cursor.close()
        conn.close()
        
    except Exception as e:
        print(f"Error storing battery data: {str(e)}")