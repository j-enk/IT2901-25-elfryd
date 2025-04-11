from pydantic import ValidationError
from psycopg2 import sql
import re
from core.models import ConfigData
from core.database import get_connection
from core.config import COMMAND_PATTERNS


def process_message(topic: str, payload: str):
    """Process and store configuration commands"""
    try:
        payload = payload.strip()

        # Check if the message format is valid
        valid_format = False
        for pattern in COMMAND_PATTERNS.values():
            if re.match(pattern, payload):
                valid_format = True
                break

        if not valid_format:
            print(f"Invalid command format: {payload}")
            return

        # Create ConfigData model
        config_data = ConfigData(command=payload, topic=topic)

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

        insert_query = sql.SQL(
            """
            INSERT INTO {} (command, topic)
            VALUES (%s, %s)
        """
        ).format(sql.Identifier("elfryd_config"))

        cursor.execute(
            insert_query,
            (
                data.command,
                data.topic,
            ),
        )
        conn.commit()
        cursor.close()
        conn.close()

    except Exception as e:
        print(f"Error storing config data: {str(e)}")
