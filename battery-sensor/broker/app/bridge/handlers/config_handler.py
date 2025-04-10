from pydantic import ValidationError
from psycopg2 import sql
from core.models import ConfigData
from core.database import get_connection
from core.config import VALID_CONFIG_COMMANDS

def process_message(topic: str, payload: str):
    """Process and store configuration commands"""
    try:
        payload = payload.strip()
        
        # Handle two types of messages:
        # 1. Simple commands: "battery", "temp", "gyro"
        # 2. Frequency commands: "freq XXX" where XXX is a number
        
        if payload.startswith("freq "):
            # Handle frequency command
            parts = payload.split()
            if len(parts) != 2 or not parts[1].isdigit():
                print(f"Invalid frequency command format: {payload}")
                return
        else:
            # Handle sensor type commands
            if payload not in VALID_CONFIG_COMMANDS:
                print(f"Invalid config command: {payload}")
                return
        
        # Create ConfigData model
        config_data = ConfigData(
            command=payload,
            topic=topic
        )
        
        # Store in database
        store_config_data(config_data)
        
    except ValidationError as e:
        print(f"Validation error in config message: {str(e)}")
    except Exception as e:
        print(f"Error processing config message: {str(e)}")

def store_config_data(data: ConfigData):
    """Store validated config data in database"""
    try:
        conn = get_connection()
        cursor = conn.cursor()
        
        insert_query = sql.SQL("""
            INSERT INTO {} (command, topic)
            VALUES (%s, %s)
        """).format(sql.Identifier(f"elfryd_config"))
        
        cursor.execute(insert_query, (
            data.command,
            data.topic,
        ))
        conn.commit()
        cursor.close()
        conn.close()
        
    except Exception as e:
        print(f"Error storing config data: {str(e)}")