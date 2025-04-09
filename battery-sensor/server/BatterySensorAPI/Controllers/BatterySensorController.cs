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
    private readonly BatteryDataGenerator _dataGenerator;
    private readonly ILogger<BatterySensorController> _logger;

    public BatterySensorController(
        BatterySensorService sensorService,
        BatteryDataGenerator dataGenerator,
        ILogger<BatterySensorController> logger)
    {
        _sensorService = sensorService;
        _dataGenerator = dataGenerator;
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
                            voltage = latestReading.voltage,
                            timestamp = latestReading.timestamp,
                            sensorId = latestReading.sensorId
                        } : null
                });
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error checking API status");
                return StatusCode(500, "Error checking API status");
            }
        }
        [HttpGet("stream")]
public async Task GetBatteryStream(CancellationToken cancellationToken)
{
    Response.Headers.Add("Content-Type", "text/event-stream");
    Response.Headers.Add("Cache-Control", "no-cache");
    Response.Headers.Add("Connection", "keep-alive");
    Response.Headers.Add("Access-Control-Allow-Origin", "*");

    _logger.LogInformation("SSE stream requested");
    
    // Track last sent data timestamp to avoid duplicates
    DateTime lastSentTimestamp = DateTime.MinValue;

    try
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            // Get the latest data from the generator
            var readings = _dataGenerator.GetLatestReadings(20);
            
            if (readings.Count > 0)
            {
                var newestTimestamp = readings[0].timestamp;
                
                // Only send if we have new data
                if (newestTimestamp > lastSentTimestamp)
                {
                    _logger.LogInformation(readings[9].ToString());
                    _logger.LogInformation($"Sending {readings.Count} readings via SSE");
                    lastSentTimestamp = newestTimestamp;
                    
                    var json = System.Text.Json.JsonSerializer.Serialize(readings);
                    await Response.WriteAsync($"data: {json}\n\n", cancellationToken);
                    await Response.Body.FlushAsync(cancellationToken);
                }
                else
                {
                    // Send heartbeat to keep connection alive
                    await Response.WriteAsync(": heartbeat\n\n", cancellationToken);
                    await Response.Body.FlushAsync(cancellationToken);
                    _logger.LogDebug("Sent SSE heartbeat");
                }
            }
            else
            {
                _logger.LogWarning("No readings available for SSE");
                await Response.WriteAsync(": no data\n\n", cancellationToken);
                await Response.Body.FlushAsync(cancellationToken);
            }
            
            // Check for new data every 5 seconds
            await Task.Delay(5000, cancellationToken);
        }
    }
    catch (OperationCanceledException)
    {
        _logger.LogInformation("SSE connection cancelled");
    }
    catch (Exception ex)
    {
        _logger.LogError(ex, "Error in SSE stream");
    }
    
    _logger.LogInformation("SSE stream ended");
}

    // Helper method to get your battery readings
    private async Task<List<BatteryReading>> GetLatestBatteryReadings()
    {
        // Replace with your actual data retrieval logic
        // This would likely query your database for the most recent readings
        return await _sensorService.GetRecentReadings();
    }
    }
}
