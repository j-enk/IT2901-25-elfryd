using System;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Threading.Tasks;
using System.Text.Json;

public class ElfrydApiClient
{
    private readonly HttpClient _client;
    private readonly string _apiKey;

    public ElfrydApiClient(string baseUrl, string apiKey, HttpClientHandler handler = null)
    {
        _apiKey = apiKey;
        _client = handler != null ? new HttpClient(handler) : new HttpClient();
        _client.BaseAddress = new Uri(baseUrl);
    }

    public async Task<string> GetMessagesAsync(string topic = null, int limit = 100)
    {
        var requestUri = $"messages?limit={limit}";
        if (!string.IsNullOrEmpty(topic))
        {
            requestUri += $"&topic={Uri.EscapeDataString(topic)}";
        }

        var request = new HttpRequestMessage(HttpMethod.Get, requestUri);
        request.Headers.Add("X-API-Key", _apiKey);

        var response = await _client.SendAsync(request);
        response.EnsureSuccessStatusCode();
        
        return await response.Content.ReadAsStringAsync();
    }

    public async Task<string> PublishMessageAsync(string topic, string message)
    {
        var content = new StringContent(
            JsonSerializer.Serialize(new { topic, message }), 
            System.Text.Encoding.UTF8, 
            "application/json");

        var request = new HttpRequestMessage(HttpMethod.Post, "messages");
        request.Headers.Add("X-API-Key", _apiKey);
        request.Content = content;

        var response = await _client.SendAsync(request);
        response.EnsureSuccessStatusCode();
        
        return await response.Content.ReadAsStringAsync();
    }

    public async Task<string> UpdateFrequencyAsync( string command)
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

    // public async Task<string> SendSensorConfigAsync(string topic, string message)
    // {
    //     var content = new StringContent(
    //         JsonSerializer.Serialize(new { topic, message }), 
    //         System.Text.Encoding.UTF8, 
    //         "application/json");

    //     var request = new HttpRequestMessage(HttpMethod.Post, "config/send");
    //     request.Headers.Add("X-API-Key", _apiKey);
    //     request.Content = content;

    //     var response = await _client.SendAsync(request);
    //     response.EnsureSuccessStatusCode();
        
    //     return await response.Content.ReadAsStringAsync();
    // }

    public async Task<string> GetConfigAsync()
    {
        var request = new HttpRequestMessage(HttpMethod.Get, "config");
        request.Headers.Add("X-API-Key", _apiKey);

        var response = await _client.SendAsync(request);
        response.EnsureSuccessStatusCode();
        
        return await response.Content.ReadAsStringAsync();
    }

    public async Task<string> GetBatteryDataAsync(string battery_id = null, int limit = 20){
        var requestUri = $"battery?limit={limit}";

        if (!string.IsNullOrEmpty(battery_id))
        {
            requestUri += $"&battery_id={Uri.EscapeDataString(battery_id)}";
        }
        
        
        var request = new HttpRequestMessage(HttpMethod.Get, requestUri);
        request.Headers.Add("X-API-Key", _apiKey);

        var response = await _client.SendAsync(request);
        response.EnsureSuccessStatusCode();
        
        return await response.Content.ReadAsStringAsync();
    }
}