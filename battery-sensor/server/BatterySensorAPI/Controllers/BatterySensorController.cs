// Controllers/BatterySensorController.cs
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using BatterySensorAPI.Models;
using BatterySensorAPI.Services;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;

namespace BatterySensorAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class BatterySensorController : ControllerBase
    {
        private readonly BatterySensorService _sensorService;
        private readonly ILogger<BatterySensorController> _logger;

        public BatterySensorController(
            BatterySensorService sensorService,
            ILogger<BatterySensorController> logger)
        {
            _sensorService = sensorService;
            _logger = logger;
        }

        // GET: api/BatterySensor/readings
        [HttpGet("recent")]
        public async Task<ActionResult<IEnumerable<BatteryReading>>> GetRecentReadings(
            [FromQuery] string sensorId = null,
            [FromQuery] int limit = 20)
        {
            try
            {
                var readings = await _sensorService.GetRecentReadings(sensorId, limit);
                return Ok(readings);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving recent battery readings");
                return StatusCode(500, "Error retrieving recent battery readings");
            }
        }

        // GET: api/BatterySensor/latest
        [HttpGet("latest")]
        public async Task<ActionResult<BatteryReading>> GetLatestReading(
            [FromQuery] string sensorId = null)
        {
            try
            {
                var reading = await _sensorService.GetLatestReading(sensorId);
                if (reading == null)
                {
                    return NotFound("No battery readings found");
                }
                return Ok(reading);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving latest battery reading");
                return StatusCode(500, "Error retrieving latest battery reading");
            }
        }

        // GET: api/BatterySensor/stats
        [HttpGet("stats")]
        public async Task<ActionResult<BatteryStats>> GetBatteryStats(
            [FromQuery] string sensorId = null,
            [FromQuery] int hours = 24)
        {
            try
            {
                var stats = await _sensorService.GetBatteryStats(sensorId, hours);
                return Ok(stats);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving battery statistics");
                return StatusCode(500, "Error retrieving battery statistics");
            }
        }

        // GET: api/BatterySensor/status
        [HttpGet("status")]
        public async Task<IActionResult> GetStatus()
        {
            try
            {
                var isApiAvailable = await _sensorService.CheckApiAvailability();
                var latestReading = await _sensorService.GetLatestReading();

                return Ok(new
                {
                    status = isApiAvailable ? "online" : "offline",
                    latestReading = latestReading != null ?
                        new
                        {
                            voltage = latestReading.Voltage,
                            timestamp = latestReading.Timestamp,
                            sensorId = latestReading.SensorId
                        } : null
                });
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error checking API status");
                return StatusCode(500, "Error checking API status");
            }
        }
    }
}