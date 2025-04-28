using System;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Threading.Tasks;
using System.Text.Json;
using BatterySensorAPI.Models;


namespace BatterySensorAPI.Services
{

    public class ElfrydApiClient : IElfrydApiClient
    {
        private readonly HttpClient _client;
        private readonly string _apiKey;

        public ElfrydApiClient(string baseUrl, string apiKey, HttpClientHandler handler = null)
        {
            _apiKey = apiKey;
            _client = handler != null ? new HttpClient(handler) : new HttpClient();
            _client.BaseAddress = new Uri(baseUrl);
        }

        public async Task<string> UpdateConfigAsync(string command)
        {
            var content = new StringContent(
                JsonSerializer.Serialize(new { command }),
                System.Text.Encoding.UTF8,
                "application/json");

            var request = new HttpRequestMessage(HttpMethod.Post, "config/send");
            request.Headers.Add("X-API-Key", _apiKey);
            request.Content = content;

            var response = await _client.SendAsync(request);
            response.EnsureSuccessStatusCode();

            return await response.Content.ReadAsStringAsync();
        }

        public async Task<string> GetConfigAsync(int limit = 20, double hours = 168, double time_offset = 0)
        {

            var requestUri = $"config?limit={limit}";
            requestUri += $"&hours={hours}";
            requestUri += $"&time_offset={time_offset}";

            var request = new HttpRequestMessage(HttpMethod.Get, requestUri);
            request.Headers.Add("X-API-Key", _apiKey);

            var response = await _client.SendAsync(request);
            response.EnsureSuccessStatusCode();

            return await response.Content.ReadAsStringAsync();
        }

        public async Task<string> GetBatteryDataAsync(int battery_id = 0, int limit = 20, double hours = 24, double time_offset = 0)
        {
            var requestUri = $"battery?limit={limit}";
            requestUri += $"&hours={hours}";
            requestUri += $"&time_offset={time_offset}";

            if (battery_id!=0)
            {
                requestUri += $"&battery_id={battery_id}";
            }

            Console.WriteLine("Request URI: {0}", requestUri);
            var request = new HttpRequestMessage(HttpMethod.Get, requestUri);
            request.Headers.Add("X-API-Key", _apiKey);

            var response = await _client.SendAsync(request);
            response.EnsureSuccessStatusCode();

            return await response.Content.ReadAsStringAsync();
        }

        public async Task<string> GetTempDataAsync(int limit = 20, double hours = 168, double time_offset = 0)
        {
            var requestUri = $"temperature?limit={limit}";
            requestUri += $"&hours={hours}";
            requestUri += $"&time_offset={time_offset}";
            var request = new HttpRequestMessage(HttpMethod.Get, requestUri);
            request.Headers.Add("X-API-Key", _apiKey);

            var response = await _client.SendAsync(request);
            response.EnsureSuccessStatusCode();

            return await response.Content.ReadAsStringAsync();
        }

        public async Task<string> GetGyroDataAsync(int limit = 20, double hours = 168, double time_offset = 0)
        {
            var requestUri = $"gyro?limit={limit}";
            requestUri += $"&hours={hours}";
            requestUri += $"&time_offset={time_offset}";
            var request = new HttpRequestMessage(HttpMethod.Get, requestUri);
            request.Headers.Add("X-API-Key", _apiKey);

            var response = await _client.SendAsync(request);
            response.EnsureSuccessStatusCode();

            return await response.Content.ReadAsStringAsync();
        }

        public async Task<List<BatteryReading>> GetLatestPerBatteryAsync(int battery_id = 0)
        {
            var json = await this.GetBatteryDataAsync(
                battery_id: battery_id,
                limit: 0,
                hours: 10,
                time_offset: 0
            );

            using var doc = JsonDocument.Parse(json);

            var dataArray = doc.RootElement.ValueKind == JsonValueKind.Object && doc.RootElement.TryGetProperty("data", out var dataElement)
                ? dataElement
                : doc.RootElement;

            List<BatteryReading> readings = new List<BatteryReading>();

            foreach (var item in dataArray.EnumerateArray())
            {
                try
                {

                    if (item.TryGetProperty("battery_id", out var batteryIdElement) &&
                        item.TryGetProperty("voltage", out var voltageElement))
                    {
                        int batteryId = batteryIdElement.GetInt32();
                        double voltage = voltageElement.GetDouble();

                        DateTime timestamp = DateTime.UtcNow;
                        if (item.TryGetProperty("device_timestamp", out var timestampElement))
                        {
                            timestamp = DateTimeOffset.FromUnixTimeSeconds(timestampElement.GetInt64()).UtcDateTime;
                        }

                        readings.Add(new BatteryReading
                        {
                            battery_id = batteryId,
                            millivoltage = voltage,
                            timestamp = timestamp
                        });
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error parsing battery reading: {ex.Message}");
                }
            }

            // Group by battery_id and get the most recent reading for each
            var latestPerBattery = readings
                .GroupBy(r => r.battery_id)
                .Select(g => g.OrderByDescending(r => r.timestamp).First())
                .ToList();

            return latestPerBattery;
        }
    }
}