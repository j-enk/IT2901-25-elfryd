import psycopg2
from psycopg2 import sql
from psycopg2.extensions import connection as Connection

from datetime import datetime, timedelta
from typing import List, Optional, Union
from fastapi import HTTPException

from .config import DB_CONFIG
from .models import (
    BatteryDataResponse,
    ConfigDataResponse,
    GyroDataResponse,
    StoredMessage,
    TemperatureDataResponse,
)


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
    parts = topic.split("/")

    # Need at least two parts for a valid table name
    if len(parts) < 2:
        return parts[0]

    # Use two parts of the topic to create a table name
    return f"{parts[0]}_{parts[1]}"


def get_all_message_tables(conn: Connection) -> List[str]:
    """
    Get all table names in the database that are message tables.
    """
    cursor = conn.cursor()
    query = sql.SQL(
        """
        SELECT table_name FROM information_schema.tables 
        WHERE table_schema = 'public'
    """
    )
    cursor.execute(query)
    tables = [row[0] for row in cursor.fetchall()]
    cursor.close()
    return tables


def row_to_model(table: str, row: tuple) -> Union[
    BatteryDataResponse,
    TemperatureDataResponse,
    GyroDataResponse,
    ConfigDataResponse,
    StoredMessage,
]:
    """
    Convert a database row to the appropriate model instance based on table name.
    """
    match table:
        case "elfryd_battery":
            return BatteryDataResponse(
                id=row[0], battery_id=row[1], voltage=row[2], device_timestamp=row[3]
            )
        case "elfryd_temp":
            return TemperatureDataResponse(
                id=row[0], temperature=row[1], device_timestamp=row[2]
            )
        case "elfryd_gyro":
            return GyroDataResponse(
                id=row[0],
                accel_x=row[1],
                accel_y=row[2],
                accel_z=row[3],
                gyro_x=row[4],
                gyro_y=row[5],
                gyro_z=row[6],
                device_timestamp=row[7],
            )
        case "elfryd_config":
            return ConfigDataResponse(
                id=row[0], command=row[1], topic=row[2], timestamp=row[3]
            )
        case _:
            # Default message format
            return StoredMessage(
                id=row[0], topic=row[1], message=row[2], timestamp=row[3]
            )


def query_messages(
    conn: Connection,
    topic: Optional[str] = None,
    limit: int = 20,
    offset: int = 0,
    hours: Optional[float] = None,
    time_offset: Optional[float] = None,
) -> List[StoredMessage]:
    """
    Query messages from the database with optional filtering.
    When limit is specified alongside hours, data points are distributed evenly across
    the time range instead of just returning the newest points.

    Parameters:
    - conn: Database connection
    - topic: Topic filter (partial match)
    - limit: Maximum number of records to return
    - offset: Number of records to skip
    - hours: Optional time window in hours to filter data
    - time_offset: Optional offset in hours from current time (e.g., 24 = start from yesterday)
    """
    all_results = []

    # Exit early if no topic is provided
    if not topic:
        raise HTTPException(status_code=400, detail="Topic is required")

    table = get_table_name(topic)

    try:
        cur = conn.cursor()

        # Apply time-based filtering using server timestamp
        if hours is not None:
            # Calculate the time range
            end_time = datetime.now()
            if time_offset is not None:
                end_time = end_time - timedelta(hours=time_offset)
            start_time = end_time - timedelta(hours=hours)
            
            # If we have both a time range and a limit, use the sampling approach
            if limit > 0:
                # First, check how many records we have in the time range
                count_query = sql.SQL(
                    """
                    SELECT COUNT(*) FROM {} 
                    WHERE topic LIKE %s
                    AND timestamp >= %s AND timestamp <= %s
                    """
                ).format(sql.Identifier(table))
                
                count_params = [f"%{topic}%", start_time, end_time]
                cur.execute(count_query, count_params)
                total_count = cur.fetchone()[0]
                
                if total_count <= limit:
                    # If we have fewer points than the limit, just return all of them
                    query = sql.SQL(
                        """
                        SELECT id, topic, message, timestamp 
                        FROM {} 
                        WHERE topic LIKE %s
                        AND timestamp >= %s AND timestamp <= %s
                        ORDER BY timestamp ASC
                        """
                    ).format(sql.Identifier(table))
                    params = [f"%{topic}%", start_time, end_time]
                else:
                    # Use window functions to divide the time range into equal buckets
                    # and select one representative point from each bucket
                    query = sql.SQL(
                        """
                        WITH bucketed_data AS (
                            SELECT 
                                id, topic, message, timestamp,
                                WIDTH_BUCKET(EXTRACT(EPOCH FROM timestamp), 
                                            EXTRACT(EPOCH FROM %s::timestamp), 
                                            EXTRACT(EPOCH FROM %s::timestamp), 
                                            %s) AS bucket
                            FROM {} 
                            WHERE topic LIKE %s
                            AND timestamp >= %s AND timestamp <= %s
                        ),
                        ranked_data AS (
                            SELECT 
                                *,
                                ROW_NUMBER() OVER (PARTITION BY bucket ORDER BY timestamp) AS row_num
                            FROM bucketed_data
                        )
                        SELECT id, topic, message, timestamp
                        FROM ranked_data
                        WHERE row_num = 1
                        ORDER BY timestamp ASC
                        """
                    ).format(sql.Identifier(table))
                    params = [start_time, end_time, limit, f"%{topic}%", start_time, end_time]
            else:
                # No limit specified, use regular time filtering
                query = sql.SQL(
                    """
                    SELECT id, topic, message, timestamp 
                    FROM {} 
                    WHERE topic LIKE %s
                    AND timestamp >= %s AND timestamp <= %s
                    ORDER BY timestamp ASC
                    """
                ).format(sql.Identifier(table))
                params = [f"%{topic}%", start_time, end_time]
        else:
            # No time filtering, just apply regular limit with newest first
            query = sql.SQL(
                """
                SELECT id, topic, message, timestamp 
                FROM {} 
                WHERE topic LIKE %s
                ORDER BY timestamp DESC 
                LIMIT %s
                OFFSET %s
                """
            ).format(sql.Identifier(table))
            params = [f"%{topic}%", limit, offset]

        cur.execute(query, params)
        rows = cur.fetchall()

        # Convert rows to model instances and then to dictionaries
        for row in rows:
            model = row_to_model(table, row)
            # Convert model to dictionary for API response
            all_results.append(model.model_dump())

        cur.close()

    except Exception as table_error:
        print(f"Error querying table {table}: {str(table_error)}")

    return all_results


def query_sensor_data(
    conn: Connection,
    table_name: str,
    id_column: Optional[str] = None,
    id_value: Optional[int] = None,
    limit: int = 20,
    hours: Optional[float] = None,
    time_offset: Optional[float] = None,
) -> List[Union[BatteryDataResponse, TemperatureDataResponse, GyroDataResponse]]:
    """
    Query sensor-specific data (battery, temperature, gyro) with optional filtering.
    Uses device_timestamp for time-based filtering and excludes server timestamp from results.
    
    When limit is specified alongside hours, data points are distributed evenly across
    the time range instead of just returning the newest points.

    Parameters:
    - conn: Database connection
    - table_name: Name of the sensor table to query
    - id_column: Optional column name for ID filtering
    - id_value: Optional value for ID filtering
    - limit: Maximum number of records to return
    - hours: Optional time window in hours to filter data
    - time_offset: Optional offset in hours from current time (e.g., 24 = start from yesterday)
    """
    try:
        cur = conn.cursor()

        # Map table names to their columns (excluding server timestamp)
        table_columns = {
            "elfryd_battery": "id, battery_id, voltage, device_timestamp",
            "elfryd_temp": "id, temperature, device_timestamp",
            "elfryd_gyro": "id, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, device_timestamp",
        }

        # Validate table name
        if table_name not in table_columns:
            raise ValueError(f"Unsupported sensor table: {table_name}")

        # Build query with specific columns (excluding timestamp)
        columns = table_columns[table_name]
        
        # Apply time-based filtering and determine sampling approach
        if hours is not None:
            # Convert hours to seconds
            seconds_duration = int(hours * 3600)

            # Calculate the current time in unix timestamp
            current_unix = int(datetime.now().timestamp())

            # Apply time offset if provided
            if time_offset is not None:
                seconds_offset = int(time_offset * 3600)
                end_time = current_unix - seconds_offset
            else:
                end_time = current_unix

            # Calculate the start time based on the duration
            start_time = end_time - seconds_duration
            
            # If we have both a time range and a limit, use a more sophisticated sampling approach
            if limit > 0:
                # Option 1: Time-bucket based approach using window functions
                # This creates evenly distributed time buckets and selects one point from each bucket
                
                # First, build a query to determine the count of records in the time range
                count_query = sql.SQL(
                    """
                    SELECT COUNT(*) FROM {} 
                    WHERE device_timestamp >= %s AND device_timestamp <= %s
                    """
                ).format(sql.Identifier(table_name))
                
                count_params = [start_time, end_time]
                
                # Add ID filtering if provided
                if id_column and id_value is not None:
                    count_query = sql.SQL("{} AND {} = %s").format(
                        count_query, sql.Identifier(id_column)
                    )
                    count_params.append(id_value)
                
                cur.execute(count_query, count_params)
                total_count = cur.fetchone()[0]
                
                if total_count <= limit:
                    # If we have fewer points than the limit, just return all of them
                    query = sql.SQL("SELECT {} FROM {} WHERE device_timestamp >= %s AND device_timestamp <= %s").format(
                        sql.SQL(columns), sql.Identifier(table_name)
                    )
                    params = [start_time, end_time]
                    
                    # Add ID filtering if provided
                    if id_column and id_value is not None:
                        query = sql.SQL("{} AND {} = %s").format(query, sql.Identifier(id_column))
                        params.append(id_value)
                    
                    query = sql.SQL("{} ORDER BY device_timestamp ASC").format(query)
                    
                else:
                    # Use window functions to divide the time range into equal buckets
                    # and select one representative point from each bucket
                    
                    # We need to include the device_timestamp in the output columns
                    # to construct the time buckets properly
                    time_bucket_size = seconds_duration / limit
                    
                    # Build the window function query to sample evenly across time
                    query = sql.SQL(
                        """
                        WITH bucketed_data AS (
                            SELECT 
                                {},
                                WIDTH_BUCKET(device_timestamp, %s, %s, %s) AS bucket
                            FROM {} 
                            WHERE device_timestamp >= %s AND device_timestamp <= %s
                        """).format(
                            sql.SQL(columns), 
                            sql.Identifier(table_name)
                        )
                    
                    params = [start_time, end_time, limit, start_time, end_time]
                    
                    # Add ID filtering if provided
                    if id_column and id_value is not None:
                        query = sql.SQL("{} AND {} = %s").format(query, sql.Identifier(id_column))
                        params.append(id_value)
                    
                    query = sql.SQL(
                        """
                        {}
                        ),
                        ranked_data AS (
                            SELECT 
                                *,
                                ROW_NUMBER() OVER (PARTITION BY bucket ORDER BY device_timestamp) AS row_num
                            FROM bucketed_data
                        )
                        SELECT {}
                        FROM ranked_data
                        WHERE row_num = 1
                        ORDER BY device_timestamp ASC
                        """
                    ).format(query, sql.SQL(columns))
            else:
                # No limit specified, use regular time filtering
                query = sql.SQL("SELECT {} FROM {} WHERE device_timestamp >= %s AND device_timestamp <= %s").format(
                    sql.SQL(columns), sql.Identifier(table_name)
                )
                params = [start_time, end_time]
                
                # Add ID filtering if provided
                if id_column and id_value is not None:
                    query = sql.SQL("{} AND {} = %s").format(query, sql.Identifier(id_column))
                    params.append(id_value)
                
                query = sql.SQL("{} ORDER BY device_timestamp ASC").format(query)
        else:
            # No time filtering, just apply regular limit
            query = sql.SQL("SELECT {} FROM {} WHERE 1=1").format(
                sql.SQL(columns), sql.Identifier(table_name)
            )
            params = []
            
            # Apply ID filtering if provided
            if id_column and id_value is not None:
                query = sql.SQL("{} AND {} = %s").format(query, sql.Identifier(id_column))
                params.append(id_value)
            
            query = sql.SQL("{} ORDER BY device_timestamp DESC LIMIT %s").format(query)
            params.append(limit)

        cur.execute(query, params)
        rows = cur.fetchall()

        # Convert rows to model instances and then to dictionaries
        results = []
        for row in rows:
            model = row_to_model(table_name, row)
            results.append(model.model_dump())

        cur.close()
        return results
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Database query failed: {str(e)}")


def query_config_data(
    conn: Connection,
    limit: int = 20,
    hours: Optional[float] = None,
    time_offset: Optional[float] = None,
) -> List[ConfigDataResponse]:
    """
    Query configuration data with optional filtering.
    Uses server timestamp for time-based filtering.
    
    When limit is specified alongside hours, data points are distributed evenly across
    the time range instead of just returning the newest points.

    Parameters:
    - conn: Database connection
    - limit: Maximum number of records to return
    - hours: Optional time window in hours to filter data
    - time_offset: Optional offset in hours from current time (e.g., 24 = start from yesterday)
    """
    try:
        cur = conn.cursor()
        table_name = "elfryd_config"

        # Build query with all columns including timestamp
        if hours is not None:
            # Calculate the time range
            end_time = datetime.now()
            if time_offset is not None:
                end_time = end_time - timedelta(hours=time_offset)
            start_time = end_time - timedelta(hours=hours)
            
            # If we have both a time range and a limit, use the sampling approach
            if limit > 0:
                # First, check how many records we have in the time range
                count_query = sql.SQL(
                    """
                    SELECT COUNT(*) FROM {} 
                    WHERE timestamp >= %s AND timestamp <= %s
                    """
                ).format(sql.Identifier(table_name))
                
                count_params = [start_time, end_time]
                cur.execute(count_query, count_params)
                total_count = cur.fetchone()[0]
                
                if total_count <= limit:
                    # If we have fewer points than the limit, just return all of them
                    query = sql.SQL(
                        """
                        SELECT id, command, topic, timestamp 
                        FROM {} 
                        WHERE timestamp >= %s AND timestamp <= %s
                        ORDER BY timestamp ASC
                        """
                    ).format(sql.Identifier(table_name))
                    params = [start_time, end_time]
                else:
                    # Use window functions to divide the time range into equal buckets
                    # and select one representative point from each bucket
                    query = sql.SQL(
                        """
                        WITH bucketed_data AS (
                            SELECT 
                                id, command, topic, timestamp,
                                WIDTH_BUCKET(EXTRACT(EPOCH FROM timestamp), 
                                            EXTRACT(EPOCH FROM %s::timestamp), 
                                            EXTRACT(EPOCH FROM %s::timestamp), 
                                            %s) AS bucket
                            FROM {} 
                            WHERE timestamp >= %s AND timestamp <= %s
                        ),
                        ranked_data AS (
                            SELECT 
                                *,
                                ROW_NUMBER() OVER (PARTITION BY bucket ORDER BY timestamp) AS row_num
                            FROM bucketed_data
                        )
                        SELECT id, command, topic, timestamp
                        FROM ranked_data
                        WHERE row_num = 1
                        ORDER BY timestamp ASC
                        """
                    ).format(sql.Identifier(table_name))
                    params = [start_time, end_time, limit, start_time, end_time]
            else:
                # No limit specified or limit is zero, use regular time filtering
                query = sql.SQL(
                    """
                    SELECT id, command, topic, timestamp 
                    FROM {} 
                    WHERE timestamp >= %s AND timestamp <= %s
                    ORDER BY timestamp ASC
                    """
                ).format(sql.Identifier(table_name))
                params = [start_time, end_time]
        else:
            # No time filtering, just apply regular limit with newest first
            query = sql.SQL(
                """
                SELECT id, command, topic, timestamp 
                FROM {} 
                ORDER BY timestamp DESC 
                LIMIT %s
                """
            ).format(sql.Identifier(table_name))
            params = [limit]

        cur.execute(query, params)
        rows = cur.fetchall()

        # Convert rows to model instances and then to dictionaries
        results = []
        for row in rows:
            model = row_to_model(table_name, row)
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
