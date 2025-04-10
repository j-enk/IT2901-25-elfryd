import re
from pydantic import ValidationError
from core.models import ConfigData
from core.database import get_connection
from core.config import CONFIG_FREQ_PATTERN, VALID_CONFIG_COMMANDS

def process_message(topic: str, payload: str):
    """Process and store configuration commands"""
    try:
        # Validate command format
        command_parts = payload.strip().split()
        
        if len(command_parts) != 2:
            print(f"Invalid config command format: {payload}")
            return
        
        command_type, frequency = command_parts
        
        # Validate command type
        if command_type not in VALID_CONFIG_COMMANDS:
            print(f"Invalid config command type: {command_type}")
            return
        
        # Validate frequency format
        if not re.match(CONFIG_FREQ_PATTERN, frequency):
            print(f"Invalid frequency format: {frequency}")
            return
        
        # Create ConfigData model
        config_data = ConfigData(
            command=payload,
            topic=topic,
            raw_message=payload
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
        
        # Create table if it doesn't exist
        create_table_query = """
        CREATE TABLE IF NOT EXISTS elfryd_config (
            id SERIAL PRIMARY KEY,
            command TEXT NOT NULL,
            topic TEXT NOT NULL,
            raw_message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_elfryd_config_timestamp ON elfryd_config (timestamp DESC);
        """
        cursor.execute(create_table_query)
        conn.commit()
        
        # Insert data
        insert_query = """
        INSERT INTO elfryd_config (command, topic, raw_message)
        VALUES (%s, %s, %s)
        """
        cursor.execute(insert_query, (
            data.command,
            data.topic,
            data.raw_message
        ))
        conn.commit()
        cursor.close()
        conn.close()
        
    except Exception as e:
        print(f"Error storing config data: {str(e)}")