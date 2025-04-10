import psycopg2
from psycopg2 import sql
from psycopg2.extensions import connection as Connection

from datetime import datetime, timedelta
from typing import List, Optional, Dict, Any, Union
from fastapi import HTTPException

from .config import DB_CONFIG, DEFAULT_TABLE
from .models import BatteryData, TemperatureData, GyroData, ConfigData, StoredMessage


def get_connection():
    """
    Create and return a database connection.
    """
    try:
        conn = psycopg2.connect(
            host=DB_CONFIG["host"],
            port=DB_CONFIG["port"],
            dbname=DB_CONFIG["dbname"],
            user=DB_CONFIG["user"],
            password=DB_CONFIG["password"],
        )
        return conn
    except Exception as e:
        raise HTTPException(
            status_code=500, detail=f"Database connection failed: {str(e)}"
        )
    

def get_table_name(topic: str) -> str:
    """
    Determine table name from topic.
    """
    parts = topic.split('/')
    
    # Need at least two parts for a valid table name
    if len(parts) < 2:
        return DEFAULT_TABLE  # Default table for unrecognized formats
    
    # Use two parts of the topic to create a table name
    return f"{parts[0]}_{parts[1]}"

def get_all_message_tables(conn: Connection) -> List[str]:                    
    """
    Get all table names in the database that are message tables.
    """
    cursor = conn.cursor()
    query = sql.SQL("""
        SELECT table_name FROM information_schema.tables 
        WHERE table_schema = 'public' AND 
        (table_name LIKE '%_%' OR table_name = 'mqtt_messages')
    """)
    cursor.execute(query)
    tables = [row[0] for row in cursor.fetchall()]
    cursor.close()
    return tables

def get_table_query(table: str) -> sql.Composed:
    """
    Return the appropriate query SQL and column mapping for a given table.
    """
    match table:
        case "elfryd_battery":
            return sql.SQL("""
                SELECT id, battery_id, voltage, device_timestamp, topic, raw_message, timestamp 
                FROM {} WHERE 1=1
            """)
        case "elfryd_temp":
            return sql.SQL("""
                SELECT id, sensor_id, temperature, device_timestamp, topic, raw_message, timestamp 
                FROM {} WHERE 1=1
            """)
        case "elfryd_gyro":
            return sql.SQL("""
                SELECT id, sensor_id, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, 
                       device_timestamp, topic, raw_message, timestamp 
                FROM {} WHERE 1=1
            """)
        case "elfryd_config":
            return sql.SQL("""
                SELECT id, command, topic, raw_message, timestamp 
                FROM {} WHERE 1=1
            """)
        case _:
            # Default query for standard message tables
            return sql.SQL("SELECT id, topic, message, timestamp FROM {} WHERE 1=1")

def row_to_model(table: str, row: tuple) -> Union[BatteryData, TemperatureData, GyroData, ConfigData, StoredMessage]:
    """
    Convert a database row to the appropriate model instance based on table name.
    """
    match table:
        case "elfryd_battery":
            return BatteryData(
                id=row[0],
                battery_id=row[1],
                voltage=row[2],
                device_timestamp=row[3],
                topic=row[4],
                raw_message=row[5],
                timestamp=row[6]
            )
        case "elfryd_temp":
            return TemperatureData(
                id=row[0],
                sensor_id=row[1],
                temperature=row[2],
                device_timestamp=row[3],
                topic=row[4],
                raw_message=row[5],
                timestamp=row[6]
            )
        case "elfryd_gyro":
            return GyroData(
                id=row[0],
                sensor_id=row[1],
                accel_x=row[2],
                accel_y=row[3],
                accel_z=row[4],
                gyro_x=row[5],
                gyro_y=row[6],
                gyro_z=row[7],
                device_timestamp=row[8],
                topic=row[9],
                raw_message=row[10],
                timestamp=row[11]
            )
        case "elfryd_config":
            return ConfigData(
                id=row[0],
                command=row[1],
                topic=row[2],
                raw_message=row[3],
                timestamp=row[4]
            )
        case _:
            # Default message format
            return StoredMessage(
                id=row[0],
                topic=row[1],
                message=row[2],
                timestamp=row[3]
            )

def query_messages(
    conn: Connection,
    topic: Optional[str] = None,
    limit: int = 100,
    offset: int = 0,
    hours: Optional[float] = None
) -> List[Dict[str, Any]]:
    """
    Query messages from the database with optional filtering.
    """
    all_results = []
    
    # Exit early if no topic is provided
    if not topic:
        raise HTTPException(status_code=400, detail="Topic is required")
    
    table = get_table_name(topic)
    
    try:
        cur = conn.cursor()
        
        # Build the query based on parameters and table structure
        query = get_table_query(table).format(sql.Identifier(table))
        
        params = []
        
        if topic:
            query = sql.SQL("{} AND topic LIKE %s").format(query)
            params.append(f"%{topic}%")
        
        if hours is not None:
            query = sql.SQL("{} AND timestamp > %s").format(query)
            params.append(datetime.now() - timedelta(hours=hours))
        
        query = sql.SQL("{} ORDER BY timestamp DESC").format(query)
        
        cur.execute(query, params)
        rows = cur.fetchall()
        
        # Convert rows to model instances and then to dictionaries
        table_results = []
        for row in rows:
            model = row_to_model(table, row)
            # Convert model to dictionary for API response
            table_results.append(model.model_dump())
        
        all_results.extend(table_results)
        cur.close()

    except Exception as table_error:
        print(f"Error querying table {table}: {str(table_error)}")
    
    # Sort combined results by timestamp (newest first)
    all_results.sort(key=lambda x: x["timestamp"], reverse=True)
    
    # Apply limit and offset to the combined results
    paginated_results = all_results[offset:offset+limit]
    
    return paginated_results

def query_specific_data(
    conn: Connection,
    table_name: str,
    id_column: Optional[str] = None,
    id_value: Optional[int] = None,
    limit: int = 100,
    hours: Optional[float] = None
) -> List[Dict[str, Any]]:
    """
    Query specific data type (battery, temperature, gyro) with optional filtering
    """
    try:
        cur = conn.cursor()
        
        # Get appropriate query based on table name
        base_query = get_table_query(table_name)
        
        # If invalid table name, raise error immediately
        if not base_query and table_name not in ["elfryd_battery", "elfryd_temp", "elfryd_gyro", "elfryd_config"]:
            raise ValueError(f"Unsupported table name: {table_name}")
        
        # Format the query with the actual table name
        query = base_query.format(sql.Identifier(table_name))
        params = []
        
        if id_column and id_value is not None:
            query = sql.SQL("{} AND {} = %s").format(query, sql.Identifier(id_column))
            params.append(id_value)
        
        if hours is not None:
            query = sql.SQL("{} AND timestamp > %s").format(query)
            params.append(datetime.now() - timedelta(hours=hours))
        
        query = sql.SQL("{} ORDER BY timestamp DESC LIMIT %s").format(query)
        params.append(limit)
        
        cur.execute(query, params)
        rows = cur.fetchall()
        
        # Convert rows to model instances and then to dictionaries
        results = []
        for row in rows:
            model = row_to_model(table_name, row)
            # Convert model to dictionary for API response
            results.append(model.model_dump())
        
        cur.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")

def get_topics(conn: Connection) -> List[str]:
    """
    Get a list of all unique topics in the database from all message tables
    """
    all_topics = set()
    
    # Get all message tables
    tables = get_all_message_tables(conn)
    
    # Query each table for unique topics
    for table in tables:
        try:
            cur = conn.cursor()
            query = sql.SQL("SELECT DISTINCT topic FROM {} ORDER BY topic").format(
                sql.Identifier(table)
            )
            cur.execute(query)
            table_topics = [row[0] for row in cur.fetchall()]
            all_topics.update(table_topics)
            cur.close()
        except Exception as table_error:
            # If a specific table query fails, log it but continue with other tables
            print(f"Error querying topics from table {table}: {str(table_error)}")
            continue
    
    return sorted(list(all_topics))