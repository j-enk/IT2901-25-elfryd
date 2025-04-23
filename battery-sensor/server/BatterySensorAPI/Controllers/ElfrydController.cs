using System;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using BatterySensorAPI.Services;
using BatterySensorAPI.Models;

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
        [FromQuery] bool sendAll = false,
        [FromQuery] int limit = 10
        )
        {
            try
            {
                var config = await _elfrydClient.GetConfigAsync(sendAll, limit);
                return Content(config, "application/json");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving Elfryd config");
                return StatusCode(500, "Error retrieving config from Elfryd API");
            }
        }

        [HttpGet("battery")]
        public async Task<IActionResult> GetBatteryData(
            [FromQuery] string battery_id = null,
            [FromQuery] int limit = 20,
            [FromQuery] int hours = 24
        )
        {
             try
    {
        var result = await _elfrydClient.GetBatteryDataAsync(battery_id, limit, hours);
        
        // Verify the response matches query id
        if (!string.IsNullOrEmpty(battery_id))
        {
            // Parse the JSON
            var jsonDoc = System.Text.Json.JsonDocument.Parse(result);
            var dataArray = jsonDoc.RootElement.GetProperty("data");
            
            if (dataArray.GetArrayLength() == 0)
            {
                return Content(result, "application/json");
            }
            
            // Verify battery_id
            bool allMatch = true;
            int batteryIdAsInt;
            
            if (int.TryParse(battery_id, out batteryIdAsInt))
            {
                foreach (var item in dataArray.EnumerateArray())
                {
                    if (item.TryGetProperty("battery_id", out var batteryIdProperty))
                    {
                        if (batteryIdProperty.GetInt32() != batteryIdAsInt)
                        {
                            allMatch = false;
                            break;
                        }
                    }
                    else
                    {
                        // If any item doesn't have battery_id property
                        allMatch = false;
                        break;
                    }
                }
                
                if (!allMatch)
                {
                    _logger.LogWarning("FastAPI returned unfiltered data despite battery_id filter");
                    return StatusCode(500, "API returned inconsistent data that didn't match filter criteria");
                }
            }
        }
        
        return Content(result, "application/json");
    }
    catch (Exception ex)
    {
        _logger.LogError(ex, "Error retrieving battery data");
        return StatusCode(500, "Error retrieving battery data from Elfryd API");
    }
}
    }
}