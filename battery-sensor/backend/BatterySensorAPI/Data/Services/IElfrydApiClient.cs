using BatterySensorAPI.Models;

namespace BatterySensorAPI.Services
{
    public interface IElfrydApiClient
    {
        Task<string> UpdateConfigAsync(string command);
        Task<string> GetConfigAsync(int limit = 20, int hours = 168, int time_offset = 0);
        Task<string> GetBatteryDataAsync(string battery_id = null, int limit = 20, int hours = 24, int time_offset = 0);
        Task<string> GetGyroDataAsync(int limit = 20, int hours = 24, int time_offset = 0);
        Task<string> GetTempDataAsync(int limit = 20, int hours = 24, int time_offset = 0);
        Task<List<BatteryReading>> GetLatestPerBatteryAsync(string battery_id = null);
    }
}