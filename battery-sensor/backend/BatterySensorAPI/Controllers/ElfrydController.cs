using System;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using BatterySensorAPI.Services;
using BatterySensorAPI.Models;
using System.Text.Json;

namespace BatterySensorAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class ElfrydController : ControllerBase
    {
        private readonly IElfrydApiClient _elfrydClient;
        private readonly ILogger<ElfrydController> _logger;


        public ElfrydController(
     IElfrydApiClient elfrydClient,
     ILogger<ElfrydController> logger)
        {
            _elfrydClient = elfrydClient;
            _logger = logger;
        }

        /// <summary>
        /// Updates device configuration settings
        /// </summary>
        /// <remarks>
        /// Sends configuration commands to Elfryd IoT devices.
        /// 
        /// ## Parameters
        /// - **command**: Command string to send to devices. Valid formats:
        ///   - Single command: "battery", "temp 30", "gyro 0"
        ///   - Multiple commands: "battery 10|temp 30|gyro 0"
        ///   
        /// ## Supported Commands
        /// - **battery**: Configure battery monitoring
        /// - **temp**: Configure temperature monitoring, with optional sampling rate
        /// - **gyro**: Configure gyroscope monitoring, with optional sampling rate
        /// 
        /// ## Authentication
        /// Requires API key in the X-API-Key header when forwarded to Elfryd API
        /// </remarks>
        [HttpPost("config/update")]
        public async Task<IActionResult> UpdateConfig([FromBody] ElfrydUpdateConfigRequest request)
        {
            if (request.Command == null)
            {
                return BadRequest("Config command is required");
            }

            string command = request.Command.Trim();

            if (string.IsNullOrWhiteSpace(command))
            {
                return BadRequest("Config command cannot be empty");
            }

            // Split the commands if multiple were sent
            string[] commands = command.Split('|');
            List<string> invalidCommands = new List<string>();

            HashSet<string> allowedCommandTypes = new HashSet<string>
            {
                "battery", "temp", "gyro"
            };

            foreach (string cmd in commands)
            {
                string trimmedCmd = cmd.Trim();
                if (string.IsNullOrWhiteSpace(trimmedCmd))
                {
                    invalidCommands.Add("''");
                    continue;
                }

                // Match either "command" or "command value" 
                var match = System.Text.RegularExpressions.Regex.Match(
                    trimmedCmd,
                    @"^(\w+)(?:\s+(\d+))?$"
                );

                if (!match.Success)
                {
                    invalidCommands.Add(trimmedCmd);
                    continue;
                }

                // Check if command type is in the allowed list
                string commandType = match.Groups[1].Value;
                if (!allowedCommandTypes.Contains(commandType))
                {
                    invalidCommands.Add(trimmedCmd);
                }
            }

            if (invalidCommands.Count > 0)
            {
                // Example of valid commands
                var examples = new List<string>
                {
                    "battery",
                    "temp 30",
                    "gyro 0",
                    "battery 10|temp 30|gyro 0"
                };

                return BadRequest(
                    $"Invalid command: {string.Join(", ", invalidCommands)}. " +
                    $"Examples of valid formats:\n{string.Join("\n", examples)}"
                );
            }

            try
            {
                var result = await _elfrydClient.UpdateConfigAsync(request.Command);
                return Ok(new { success = true, result = result });
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error updating config");
                return StatusCode(500, "Error updating configuration");
            }
        }

        /// <summary>
        /// Retrieve configuration history from connected devices
        /// </summary>
        /// <remarks>
        /// This endpoint returns configuration commands that have been sent to 
        /// IoT devices in the Elfryd system.
        /// 
        /// ## Parameters
        /// - **limit**: Maximum number of records to return (default: 20)
        /// - **hours**: Get data from the last X hours (default: 168)
        /// - **time_offset**: Offset in hours from current time
        /// 
        /// ## Response
        /// Returns an array of configuration records, each containing:
        /// - **id**: Unique record identifier
        /// - **device_id**: Identifier of the device
        /// - **command**: Configuration command sent to the device
        /// - **device_timestamp**: Timestamp of the command on the device (Unix timestamp)
        /// 
        /// 
        /// ## Authentication
        /// Requires API key in the X-API-Key header when forwarded to Elfryd API
        /// </remarks>
        [HttpGet("config")]
        public async Task<IActionResult> GetConfig(
        [FromQuery] int limit = 20,
        [FromQuery] double hours = 168,
        [FromQuery] double time_offset = 0
        )
        {
            var previousCulture = System.Threading.Thread.CurrentThread.CurrentCulture;
            System.Threading.Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
            try
            {
                var config = await _elfrydClient.GetConfigAsync(limit, hours, time_offset);
                return Content(config, "application/json");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving Elfryd config");
                return StatusCode(500, "Error retrieving config from Elfryd API");
            }
        }

        /// <summary>
        /// Retrieve battery voltage measurements from connected devices
        /// </summary>
        /// <remarks>
        /// This endpoint returns battery voltage data collected from the IoT devices
        /// in the Elfryd system. The voltage values are in millivolts (mV).
        ///
        /// ## Parameters
        /// - **battery_id**: Filter by specific battery identifier (0 = all batteries)
        /// - **limit**: Maximum number of records to return (default: 20)
        /// - **hours**: Get data from the last X hours (default: 168)
        /// - **time_offset**: Offset in hours from current time
        ///
        /// ## Response
        /// Returns an array of battery records, each containing:
        /// - **id**: Unique record identifier
        /// - **battery_id**: Identifier of the battery
        /// - **voltage**: Battery voltage in millivolts (mV)
        /// - **device_timestamp**: Timestamp of the measurement on the device (Unix timestamp)
        ///
        /// ## Authentication
        /// Requires API key in the X-API-Key header when forwarded to Elfryd API
        /// </remarks>
        [HttpGet("battery", Order = 0)]
        public async Task<IActionResult> GetBatteryData(
            [FromQuery] int battery_id = 0,
            [FromQuery] int limit = 20,
            [FromQuery] double hours = 168,
            [FromQuery] double time_offset = 0
        )
        {
            try
            {
                // This guarantees that the doubles are parsed as dot (.).
                var previousCulture = System.Threading.Thread.CurrentThread.CurrentCulture;
                System.Threading.Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;

                try
                {
                    if (battery_id != 0)
                    {
                        // Retrieve data for a specific battery
                        var result = await _elfrydClient.GetBatteryDataAsync(battery_id, limit, hours, time_offset);

                        // Validate that all entries have the requested battery_id
                        try
                        {
                            var jsonDoc = System.Text.Json.JsonDocument.Parse(result);

                            // Check if an entry contains illegal battery_id
                            foreach (var item in jsonDoc.RootElement.EnumerateArray())
                            {
                                if (item.TryGetProperty("battery_id", out var idProp) &&
                                    idProp.TryGetInt32(out var id) &&
                                    id != battery_id)
                                {
                                    _logger.LogWarning("Inconsistent data received from Elfryd API: requested battery_id={RequestedId}, but found battery_id={FoundId}",
                                        battery_id, id);

                                    return new ObjectResult($"Inconsistent battery data: requested battery_id={battery_id}, but found entries with different IDs")
                                    {
                                        StatusCode = StatusCodes.Status500InternalServerError
                                    };
                                }
                            }
                        }
                        catch (System.Text.Json.JsonException ex)
                        {
                            _logger.LogError(ex, "Error parsing JSON from Elfryd API");
                            return new ObjectResult("Error parsing battery data from Elfryd API")
                            {
                                StatusCode = StatusCodes.Status500InternalServerError
                            };
                        }

                        return Content(result, "application/json");
                    }

                    // Collect all data points for batteries 1-8
                    var allEntries = new List<(int batteryId, long timestamp, JsonElement raw)>();

                    for (int id = 1; id <= 8; id++)
                    {
                        var result = await _elfrydClient.GetBatteryDataAsync(id, limit, hours, time_offset);

                        var jsonDoc = System.Text.Json.JsonDocument.Parse(result);
                        var dataArray = jsonDoc.RootElement.ValueKind == System.Text.Json.JsonValueKind.Object &&
                                        jsonDoc.RootElement.TryGetProperty("data", out var dataElement)
                            ? dataElement
                            : jsonDoc.RootElement;

                        foreach (var item in dataArray.EnumerateArray())
                        {
                            if (item.TryGetProperty("device_timestamp", out var timestampProp) &&
                                timestampProp.TryGetInt64(out var timestamp))
                            {
                                allEntries.Add((id, timestamp, item));
                            }
                        }
                    }

                    if (allEntries.Count == 0)
                    {
                        return new JsonResult(new object[0]);
                    }

                    // Sort all entries by timestamp ascending (oldest first)
                    var sortedData = allEntries
                        .OrderBy(e => e.timestamp)
                        .Select(e => e.raw)
                        .ToList();

                    return new JsonResult(sortedData);
                }
                finally
                {
                    // Restore the original culture
                    System.Threading.Thread.CurrentThread.CurrentCulture = previousCulture;
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving battery data: {ErrorMessage}", ex.ToString());
                return StatusCode(500, "Error retrieving battery data from Elfryd API");
            }
        }

        /// <summary>
        /// Retrieve gyroscope data
        /// </summary>
        /// <remarks>
        /// This endpoint returns gyroscope data collected from the Elfryd system.
        /// 
        /// ## Parameters
        /// - **limit**: Maximum number of records to return (default: 20)
        /// - **hours**: Get data from the last X hours (default: 168)
        /// - **time_offset**: Offset in hours from current time
        /// 
        /// ## Response
        /// Returns an array of gyroscope sensor readings.
        /// 
        /// ## Authentication
        /// Requires API key in the X-API-Key header when forwarded to Elfryd API
        /// </remarks>
        [HttpGet("gyro")]
        public async Task<IActionResult> GetGyroData(
            [FromQuery] int limit = 20,
            [FromQuery] double hours = 168,
            [FromQuery] double time_offset = 0
        )
        {
            var prevCulture = System.Threading.Thread.CurrentThread.CurrentCulture;
            System.Threading.Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;

            try
            {
                // pull raw JSON from the API client
                var rawJson = await _elfrydClient.GetGyroDataAsync(limit, hours, time_offset);

                // deserialize to RawGyroRow[]
                var raw = JsonSerializer.Deserialize<RawGyroRow[]>(rawJson)!;

                const double µg = 1_000_000.0;
                const double mdeg = 1_000.0;
                const double rad2deg = 180.0 / Math.PI;

                // project into MotionRow[]
                var motion = raw.Select(r =>
                {
                    // convert to g
                    var ax = r.accel_x / µg;
                    var ay = r.accel_y / µg;
                    var az = r.accel_z / µg;
                    // convert to °/s
                    var gx = r.gyro_x / mdeg;
                    var gy = r.gyro_y / mdeg;
                    var gz = r.gyro_z / mdeg;

                    // compute orientation & heave
                    var roll = Math.Atan2(ay, az) * rad2deg;
                    var pitch = Math.Atan2(-ax, Math.Sqrt(ay * ay + az * az)) * rad2deg;
                    var yawRate = gz;
                    var heave = Math.Sqrt(ax * ax + ay * ay + az * az) - 1.0;

                    return new MotionRow
                    {
                        t = DateTimeOffset.FromUnixTimeSeconds(r.device_timestamp).UtcDateTime,
                        ax = ax,
                        ay = ay,
                        az = az,
                        gx = gx,
                        gy = gy,
                        gz = gz,
                        roll = roll,
                        pitch = pitch,
                        yawRate = yawRate,
                        heave = heave
                    };
                }).ToArray();

                return Ok(motion);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving gyro data");
                return StatusCode(500, "Error retrieving gyro data from Elfryd API");
            }
            finally
            {
                Thread.CurrentThread.CurrentCulture = prevCulture;
            }
        }

        /// <summary>
        /// Retrieve temperature data
        /// </summary>
        /// <remarks>
        /// This endpoint returns temperature data collected from the Elfryd system.
        /// 
        /// ## Parameters
        /// - **limit**: Maximum number of records to return (default: 20)
        /// - **hours**: Get data from the last X hours (default: 168)
        /// - **time_offset**: Offset in hours from current time
        /// 
        /// ## Response
        /// Returns an array of temperature readings.
        /// 
        /// ## Authentication
        /// Requires API key in the X-API-Key header when forwarded to Elfryd API
        /// </remarks>
        [HttpGet("temp")]
        public async Task<IActionResult> GetTempData(
            [FromQuery] int limit = 20,
            [FromQuery] double hours = 168,
            [FromQuery] double time_offset = 0
        )
        {
            var previousCulture = System.Threading.Thread.CurrentThread.CurrentCulture;
            System.Threading.Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
            try
            {
                var result = await _elfrydClient.GetTempDataAsync(limit, hours, time_offset);
                return Content(result, "application/json");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving temperature data");
                return StatusCode(500, "Error retrieving temperature data from Elfryd API");
            }
        }

        /// <summary>
        /// Get overall system state of charge
        /// </summary>
        /// <remarks>
        /// This endpoint calculates the average state of charge (SoC) across all batteries
        /// in the Elfryd system, returning a single percentage value.
        /// 
        /// ## Response
        /// Returns a single integer representing the system-wide average state of charge (0-100%).
        /// The SoC is estimated from battery voltage levels:
        /// - 12000mV = 0% (fully discharged)
        /// - 15000mV = 100% (fully charged)
        /// 
        /// ## Authentication
        /// Requires API key in the X-API-Key header for the underlying battery data fetch
        /// </remarks>
        [HttpGet("battery/system/soc", Order = 1)]
        public async Task<IActionResult> GetSystemSocVoltage()
        {
            var latestReadings = await _elfrydClient.GetLatestPerBatteryAsync();

            if (!latestReadings.Any())
                return StatusCode(500, "No battery readings available");

            var socValues = latestReadings
                .Select(r => VoltageToSoC(r.millivoltage))
                .ToList();

            var systemSoc = socValues.Average();
            return Ok(Math.Round(systemSoc));
        }

        /// <summary>
        /// Get state of charge for individual batteries
        /// </summary>
        /// <remarks>
        /// This endpoint retrieves the state of charge (SoC) for individual batteries
        /// in the Elfryd system.
        /// 
        /// ## Parameters
        /// - **battery_id**: Filter by specific battery identifier (0 = all batteries)
        /// 
        /// ## Response
        /// Returns an array of battery SoC values, each containing:
        /// - **battery_id**: Identifier of the battery
        /// - **Soc**: State of charge as a percentage (0-100%)
        /// 
        /// The SoC is estimated from battery voltage levels:
        /// - 12000mV = 0% (fully discharged)
        /// - 15000mV = 100% (fully charged)
        /// 
        /// ## Authentication
        /// Requires API key in the X-API-Key header for the underlying battery data fetch
        /// </remarks>
        [HttpGet("battery/individual/soc", Order = 1)]
        public async Task<IActionResult> GetIndividualSocVoltage(int battery_id = 0)
        {

            var latestReadings = await _elfrydClient.GetLatestPerBatteryAsync(battery_id);

            if (!latestReadings.Any())
                return StatusCode(500, "No battery readings available");

            var socValues = latestReadings
                .Select(r => new { r.battery_id, Soc = VoltageToSoC(r.millivoltage) })
                .ToList();

            return Ok(socValues);
        }


        private double VoltageToSoC(double millivolts)
        {
            const double minVoltage = 12000.0; // Fully discharged
            const double maxVoltage = 15000.0; //Fully charged

            // Clamp voltage within range
            double clampedVoltage = Math.Clamp(millivolts, minVoltage, maxVoltage);

            // Calculate percentage
            double soc = ((clampedVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;

            return soc;
        }
    }
}