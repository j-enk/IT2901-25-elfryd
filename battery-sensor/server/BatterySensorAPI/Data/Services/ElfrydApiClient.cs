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

    public async Task<string> UpdateConfigAsync( string command)
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

    public async Task<string> GetConfigAsync(bool sendAll, int limit = 10)
    {

        var requestUri = "config";
        if (!sendAll)
        {
            requestUri += $"?limit={limit}";
        }else{
            Console.WriteLine("Sending request without limit parameter (sendAll=true)");

        }
        var request = new HttpRequestMessage(HttpMethod.Get, requestUri);
        request.Headers.Add("X-API-Key", _apiKey);

        var response = await _client.SendAsync(request);
        response.EnsureSuccessStatusCode();
        
        return await response.Content.ReadAsStringAsync();
    }

    public async Task<string> GetBatteryDataAsync(string battery_id = null, int limit = 20, int hours = 24){
        var requestUri = $"battery?limit={limit}";
        requestUri += $"&hours={hours}";

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