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

        # Build the query based on parameters and table structure
        query = sql.SQL(
            "SELECT id, topic, message, timestamp FROM {} WHERE 1=1"
        ).format(sql.Identifier(table))

        params = []

        if topic:
            query = sql.SQL("{} AND topic LIKE %s").format(query)
            params.append(f"%{topic}%")

        # Apply time-based filtering using server timestamp
        if hours is not None:
            if time_offset is not None:
                # Calculate the time range with offset
                end_time = datetime.now() - timedelta(hours=time_offset)
                start_time = end_time - timedelta(hours=hours)

                query = sql.SQL("{} AND timestamp >= %s AND timestamp <= %s").format(
                    query
                )
                params.extend([start_time, end_time])
            else:
                # Standard time window from current time
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
    paginated_results = all_results[offset : offset + limit]

    return paginated_results


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
        query = sql.SQL("SELECT {} FROM {} WHERE 1=1").format(
            sql.SQL(columns), sql.Identifier(table_name)
        )
        params = []

        # Apply ID filtering if provided
        if id_column and id_value is not None:
            query = sql.SQL("{} AND {} = %s").format(query, sql.Identifier(id_column))
            params.append(id_value)

        # Apply time-based filtering using device_timestamp
        if hours is not None:
            # Convert hours to seconds
            seconds_duration = int(hours * 3600)

            # Calculate the current time in unix timestamp
            current_unix = int(datetime.now().timestamp())

            # Apply time offset if provided
            if time_offset is not None:
                seconds_offset = int(time_offset * 3600)
                start_time = current_unix - seconds_offset
            else:
                start_time = current_unix

            # Calculate the end time based on the duration
            end_time = start_time
            start_time = end_time - seconds_duration

            # Add the time range filter to the query
            query = sql.SQL(
                "{} AND device_timestamp >= %s AND device_timestamp <= %s"
            ).format(query)
            params.extend([start_time, end_time])

        # Order by device_timestamp
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
        query = sql.SQL(
            "SELECT id, command, topic, timestamp FROM {} WHERE 1=1"
        ).format(sql.Identifier(table_name))
        params = []

        # Apply time-based filtering using server timestamp
        if hours is not None:
            if time_offset is not None:
                # Calculate the time range with offset
                end_time = datetime.now() - timedelta(hours=time_offset)
                start_time = end_time - timedelta(hours=hours)

                query = sql.SQL("{} AND timestamp >= %s AND timestamp <= %s").format(
                    query
                )
                params.extend([start_time, end_time])
            else:
                # Standard time window from current time
                query = sql.SQL("{} AND timestamp > %s").format(query)
                params.append(datetime.now() - timedelta(hours=hours))

        # Order by server timestamp
        query = sql.SQL("{} ORDER BY timestamp DESC LIMIT %s").format(query)
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
