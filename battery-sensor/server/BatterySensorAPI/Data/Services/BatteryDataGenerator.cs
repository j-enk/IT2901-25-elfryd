using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using BatterySensorAPI.Models;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace BatterySensorAPI.Services
{
    public class BatteryDataGenerator : BackgroundService
    {
        private readonly ILogger<BatteryDataGenerator> _logger;
        private readonly BatterySensorService _sensorService;
        private readonly List<BatteryReading> _latestReadings = new List<BatteryReading>();
        private readonly object _lockObject = new object();
        private readonly string _sensorId = "battery001";
        private readonly Random _random = new Random();
        
        // Store the latest mock readings for streaming
        public List<BatteryReading> GetLatestReadings(int count = 20)
        {
            lock (_lockObject)
            {
                return _latestReadings.Count > count 
                    ? _latestReadings.GetRange(0, count) 
                    : new List<BatteryReading>(_latestReadings);
            }
        }

        public BatteryDataGenerator(
            ILogger<BatteryDataGenerator> logger,
            BatterySensorService sensorService)
        {
            _logger = logger;
            _sensorService = sensorService;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            _logger.LogInformation("Battery Data Generator started");
            
            // Initialize with some readings
            GenerateInitialReadings();

            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    // Generate a new reading at each interval
                    GenerateNewReading();
                    
                    // Wait for 1 minute before generating the next reading
                    await Task.Delay(TimeSpan.FromMinutes(1), stoppingToken);
                }
                catch (OperationCanceledException)
                {
                    // Normal during shutdown, no need to log
                    break;
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Error generating mock battery data");
                    await Task.Delay(TimeSpan.FromSeconds(10), stoppingToken);
                }
            }
            
            _logger.LogInformation("Battery Data Generator stopped");
        }
        
        private void GenerateInitialReadings()
        {
            // Use the existing GenerateMockReadings to get initial data
            var initialReadings = _sensorService.GenerateMockReadings(20, _sensorId);
            
            lock (_lockObject)
            {
                _latestReadings.Clear();
                _latestReadings.AddRange(initialReadings);
            }
            
            _logger.LogInformation($"Generated {initialReadings.Count} initial readings");
        }
        
        private void GenerateNewReading()
        {
            lock (_lockObject)
            {
                // Get the latest voltage from existing readings as a starting point
                double lastVoltage = _latestReadings.Count > 0 
                    ? _latestReadings[0].voltage 
                    : 16.0;
                
                // Create slight variations around the last reading for realism
                double dischargeAmount = _random.NextDouble() * 0.2 - 0.05; // Between -0.05 and +0.15
                double newVoltage = Math.Min(20.0, Math.Max(12.0, lastVoltage + dischargeAmount));
                
                // Calculate state of charge
                double soc = ((newVoltage - 12.0) / 8.0) * 100.0;
                
                // Create a new reading
                var newReading = new BatteryReading
                {
                    id = _latestReadings.Count > 0 ? _latestReadings[0].id + 1 : 1,
                    sensorId = _sensorId,
                    timestamp = DateTime.UtcNow,
                    voltage = newVoltage,
                    temperature = 20 + _random.NextDouble() * 10,  // 20-30°C
                    stateOfCharge = soc
                };
                
                // Add to the beginning (most recent)
                _latestReadings.Insert(0, newReading);
                
                // Trim the list if it gets too large
                if (_latestReadings.Count > 100)
                {
                    _latestReadings.RemoveRange(100, _latestReadings.Count - 100);
                }
                
                _logger.LogInformation($"Generated new reading: {newVoltage:F2}V at {DateTime.UtcNow}");
            }
        }
    }
}