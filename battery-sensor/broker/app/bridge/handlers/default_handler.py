from core.database import get_connection, get_table_name

def process_message(topic, payload):
    """Process and store a generic message"""
    store_generic_message(topic, payload)

def store_generic_message(topic, message):
    """Store a generic message in the database"""
    try:
        table_name = get_table_name(topic)
        conn = get_connection()
        cursor = conn.cursor()
        
        # Create table if it doesn't exist
        create_table_query = f"""
        CREATE TABLE IF NOT EXISTS {table_name} (
            id SERIAL PRIMARY KEY,
            topic TEXT NOT NULL,
            message TEXT NOT NULL,
            timestamp TIMESTAMPTZ DEFAULT NOW()
        );
        CREATE INDEX IF NOT EXISTS idx_{table_name}_timestamp ON {table_name} (timestamp DESC);
        """
        cursor.execute(create_table_query)
        conn.commit()
        
        # Insert message
        insert_query = f"""
        INSERT INTO {table_name} (topic, message)
        VALUES (%s, %s)
        """
        cursor.execute(insert_query, (topic, message))
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Error storing message in database: {str(e)}")