// Services/BatterySensorService.cs
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using BatterySensorAPI.Models;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;

namespace BatterySensorAPI.Services
{
    public class BatterySensorService
    {
        private readonly HttpClient _httpClient;
        private readonly ILogger<BatterySensorService> _logger;
        private readonly string _fastApiBaseUrl;
        private readonly bool _useMockData;
        private static readonly Random _random = new Random();

        // Cache for readings when API is unavailable
        private List<BatteryReading> _cachedReadings = new List<BatteryReading>();
        private DateTime _lastCacheUpdate = DateTime.MinValue;

        public BatterySensorService(
        HttpClient httpClient,
        IConfiguration configuration,
        ILogger<BatterySensorService> logger)
    {
        _httpClient = httpClient;
        _logger = logger;
        _fastApiBaseUrl = configuration["BatterySensor:FastApiUrl"];
        _useMockData = configuration.GetValue<bool>("BatterySensor:UseMockData", false);
        
        // Ensure the FastAPI URL has http:// prefix if not present
        if (!string.IsNullOrEmpty(_fastApiBaseUrl) && !_fastApiBaseUrl.StartsWith("http"))
        {
            _fastApiBaseUrl = "http://" + _fastApiBaseUrl;
        }
    }

        public async Task<List<BatteryReading>> GetRecentReadings(string sensorId = null, int limit = 20)
        {
            if (_useMockData)
            {
                return GenerateMockReadings(limit, sensorId);
            }

            try
            {
                // Build the query URL with parameters
                var url = $"{_fastApiBaseUrl}/readings/recent?limit={limit}";
                if (!string.IsNullOrEmpty(sensorId))
                {
                    url += $"&sensor_id={sensorId}";
                }

                _logger.LogInformation($"Fetching readings from: {url}");
                var response = await _httpClient.GetAsync(url);
                response.EnsureSuccessStatusCode();

                var content = await response.Content.ReadAsStringAsync();
                var options = new JsonSerializerOptions
                {
                    PropertyNameCaseInsensitive = true
                };

                var readings = JsonSerializer.Deserialize<List<BatteryReading>>(content, options);
                
                // Update cache if we got data
                if (readings != null && readings.Count > 0)
                {
                    _cachedReadings = readings;
                    _lastCacheUpdate = DateTime.UtcNow;
                }
                
                return readings ?? new List<BatteryReading>();
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error fetching battery sensor readings from FastAPI");
                
                // Use cache if it's less than 30 minutes old
                if (_cachedReadings.Count > 0 && DateTime.UtcNow.Subtract(_lastCacheUpdate).TotalMinutes < 30)
                {
                    _logger.LogWarning("Using cached battery readings due to FastAPI error");
                    return _cachedReadings;
                }
                
                // Fall back to mock data
                return GenerateMockReadings(limit, sensorId);
            }
        }

        public async Task<BatteryReading> GetLatestReading(string sensorId = null)
        {
            if (_useMockData)
            {
                return GenerateMockReadings(1, sensorId).FirstOrDefault();
            }

            try
            {
                // Build the query URL with parameters
                var url = $"{_fastApiBaseUrl}/readings/latest";
                if (!string.IsNullOrEmpty(sensorId))
                {
                    url += $"?sensor_id={sensorId}";
                }

                var response = await _httpClient.GetAsync(url);
                response.EnsureSuccessStatusCode();

                var content = await response.Content.ReadAsStringAsync();
                var options = new JsonSerializerOptions
                {
                    PropertyNameCaseInsensitive = true
                };

                return JsonSerializer.Deserialize<BatteryReading>(content, options);
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error fetching latest battery reading from FastAPI");
                
                // Use most recent cached reading if available
                if (_cachedReadings.Count > 0 && DateTime.UtcNow.Subtract(_lastCacheUpdate).TotalMinutes < 30)
                {
                    return _cachedReadings.OrderByDescending(r => r.timestamp).FirstOrDefault();
                }
                
                // Fall back to mock data
                return GenerateMockReadings(1, sensorId).FirstOrDefault();
            }
        }

        public async Task<BatteryStats> GetBatteryStats(string sensorId = null, int hours = 24)
        {
            var readings = await GetRecentReadings(sensorId, 1000); // Get a larger sample
            
            if (readings.Count == 0)
            {
                return new BatteryStats();
            }
            
            // Filter readings to the specified time window
            var cutoffTime = DateTime.UtcNow.AddHours(-hours);
            var filteredReadings = readings
                .Where(r => r.timestamp >= cutoffTime)
                .ToList();
            
            if (filteredReadings.Count == 0)
            {
                return new BatteryStats();
            }
            
            // Calculate statistics
            return new BatteryStats
            {
                MinVoltage = filteredReadings.Min(r => r.voltage),
                MaxVoltage = filteredReadings.Max(r => r.voltage),
                AverageVoltage = filteredReadings.Average(r => r.voltage),
                ReadingCount = filteredReadings.Count,
                TimeWindowHours = hours,
                LatestReading = filteredReadings.OrderByDescending(r => r.timestamp).First()
            };
        }

        // Check if FastAPI is accessible
        public async Task<bool> CheckApiAvailability()
        {
            if (_useMockData)
            {
                return true; // Always report available when using mock data
            }

            try
            {
                var response = await _httpClient.GetAsync($"{_fastApiBaseUrl}/health");
                return response.IsSuccessStatusCode;
            }
            catch
            {
                return false;
            }
        }

        // Generate mock data for development without FastAPI access
        public List<BatteryReading> GenerateMockReadings(int count, string sensorId = null)
{
    var readings = new List<BatteryReading>();
    var now = DateTime.UtcNow;
    
    // If sensorId isn't specified, use a default
    if (string.IsNullOrEmpty(sensorId))
    {
        sensorId = "battery001";
    }

    // Create a more realistic battery discharge curve
    // Batteries typically don't discharge linearly, so we'll simulate a slight curve
    double baseVoltage = 16.0; // Starting at a mid-range voltage
    double dischargeRate = 0.05; // Small discharge per reading
    double randomnessFactor = 0.3; // Add some randomness to voltage readings

    for (int i = 0; i < count; i++)
    {
        // Calculate voltage with a slight decay curve and some randomness
        double hoursAgo = i * (5.0 / 60.0); // Convert 5-minute intervals to hours
        double voltageDecay = Math.Pow(hoursAgo, 1.2) * dischargeRate; // Non-linear decay
        
        // Add some randomness to simulate real-world variations
        double randomVariation = (_random.NextDouble() * 2 - 1) * randomnessFactor;
        
        // Calculate voltage between 12V-20V range
        double voltage = Math.Min(20.0, Math.Max(12.0, baseVoltage - voltageDecay + randomVariation));
        
        // State of charge is roughly proportional to voltage
        // 20V = 100%, 12V = 0%
        double soc = ((voltage - 12.0) / 8.0) * 100.0;
        
        // Generate readings going backward in time
        var reading = new BatteryReading
        {
            id = i + 1,
            sensorId = sensorId,
            timestamp = now.AddMinutes(-5 * i),  // 5-minute intervals
            voltage = voltage,
            temperature = 20 + _random.NextDouble() * 10,  // 20-30°C
            stateOfCharge = soc
        };
        
        readings.Add(reading);
    }

    return readings;
}
    }
}