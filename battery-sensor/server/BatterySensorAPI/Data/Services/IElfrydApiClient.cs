namespace BatterySensorAPI.Services
{
    public interface IElfrydApiClient
    {
        Task<string> UpdateConfigAsync(string command);
        Task<string> GetConfigAsync(bool sendAll, int limit);
        Task<string> GetBatteryDataAsync(string battery_id = null, int limit = 20, int hours = 24);
    }
}