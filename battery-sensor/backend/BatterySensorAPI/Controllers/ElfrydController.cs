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
                // For a specific battery, just return the result as is
                var result = await _elfrydClient.GetBatteryDataAsync(battery_id, limit, hours, time_offset);
                
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

            // Return the array directly without wrapping it in a "data" object
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

        [HttpGet("gyro")]
        public async Task<IActionResult> GetGyroData(
            [FromQuery] int limit = 20,
            [FromQuery] double hours = 168,
            [FromQuery] double time_offset = 0
        )
        {
            var previousCulture = System.Threading.Thread.CurrentThread.CurrentCulture;
            System.Threading.Thread.CurrentThread.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
            try
            {
                var result = await _elfrydClient.GetGyroDataAsync(limit, hours, time_offset);
                return Content(result, "application/json");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving gyro data");
                return StatusCode(500, "Error retrieving gyro data from Elfryd API");
            }
        }

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