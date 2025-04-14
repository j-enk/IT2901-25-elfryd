from psycopg2 import sql
from core.database import get_connection, get_table_name


def process_message(topic, payload):
    """Process and store a generic message"""
    store_generic_message(topic, payload)


def store_generic_message(topic, message):
    """Store a generic message in the database"""
    try:
        conn = get_connection()
        cursor = conn.cursor()

        table_name = get_table_name(topic)

        insert_query = sql.SQL(
            """
            INSERT INTO {} (topic, message) 
            VALUES (%s, %s)
        """
        ).format(sql.Identifier(table_name))

        cursor.execute(insert_query, (topic, message))
        conn.commit()
        cursor.close()
        conn.close()

    except Exception as e:
        print(f"Error storing generic message: {str(e)}")
