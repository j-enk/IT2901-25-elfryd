from pydantic import ValidationError
from psycopg2 import sql
from core.models import BatteryData
from core.database import get_connection


def process_message(payload: str):
    """Process and store battery data from string format"""
    try:
        # Parse payload: "X/Voltage/Timestamp"
        parts = payload.split("/")
        if len(parts) != 3:
            print(f"Invalid battery data format: {payload}")
            return

        try:
            # Create BatteryData model
            battery_data = BatteryData(
                battery_id=int(parts[0]),
                voltage=int(parts[1]),
                device_timestamp=int(parts[2]),
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

        insert_query = sql.SQL(
            """
            INSERT INTO {} (battery_id, voltage, device_timestamp)
            VALUES (%s, %s, %s)
        """
        ).format(sql.Identifier(f"elfryd_battery"))

        cursor.execute(
            insert_query, (data.battery_id, data.voltage, data.device_timestamp)
        )
        conn.commit()
        cursor.close()
        conn.close()

    except Exception as e:
        print(f"Error storing battery data: {str(e)}")
