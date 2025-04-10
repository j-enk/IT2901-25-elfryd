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
        private readonly ElfrydApiClient _elfrydClient;
        private readonly ILogger<ElfrydController> _logger;

        public ElfrydController(
            ElfrydApiClient elfrydClient,
            ILogger<ElfrydController> logger)
        {
            _elfrydClient = elfrydClient;
            _logger = logger;
        }

        [HttpGet("messages")]
        public async Task<IActionResult> GetMessages(
            [FromQuery] string topic = null,
            [FromQuery] int limit = 20)
        {
            try
            {
                var messages = await _elfrydClient.GetMessagesAsync(topic, limit);
                return Content(messages, "application/json");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving Elfryd messages");
                return StatusCode(500, "Error retrieving messages from Elfryd API");
            }
        }

        [HttpPost("messages")]
        public async Task<IActionResult> PublishMessage([FromBody] ElfrydMessageRequest request)
        {
            if (string.IsNullOrEmpty(request.Topic) || string.IsNullOrEmpty(request.Message))
            {
                return BadRequest("Topic and message are required");
            }
            
            try
            {
                var result = await _elfrydClient.PublishMessageAsync(request.Topic, request.Message);
                return Ok(new { success = true, result = result });
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error publishing message to Elfryd API");
                return StatusCode(500, "Error publishing message to Elfryd API");
            }
        }

        [HttpPost("config/frequency")]
public async Task<IActionResult> UpdateConfigFrequency([FromBody] ElfrydConfigRequest request)
{
    if (request.Command == null) 
    {
        return BadRequest("Config frequency is required");
    }
    
    try 
    {
        var result = await _elfrydClient.UpdateFrequencyAsync(request.Command);
        return Ok(new { success = true, result = result });
    }
    catch (Exception ex) 
    {
        _logger.LogError(ex, "Error updating frequency");
        return StatusCode(500, "Error updating frequency configuration");
    }
}

        [HttpGet("config")]
        public async Task<IActionResult> GetConfig()
        {
            try
            {
                var config = await _elfrydClient.GetConfigAsync();
                return Content(config, "application/json");
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error retrieving Elfryd config");
                return StatusCode(500, "Error retrieving config from Elfryd API");
            }
        }

        [HttpGet("battery")]
        public async Task<IActionResult> GetBatteryData(){
            try{

            var result = await _elfrydClient.GetBatteryDataAsync();
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