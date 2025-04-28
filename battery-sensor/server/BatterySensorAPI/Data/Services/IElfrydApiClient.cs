using BatterySensorAPI.Models;

namespace BatterySensorAPI.Services
{
    public interface IElfrydApiClient
    {
        Task<string> UpdateConfigAsync(string command);
        Task<string> GetConfigAsync(int limit = 20, double hours = 168, double time_offset = 0);
        Task<string> GetBatteryDataAsync(int battery_id = 0, int limit = 20, double hours = 24, double time_offset = 0);
        Task<string> GetGyroDataAsync(int limit = 20, double hours = 24, double time_offset = 0);
        Task<string> GetTempDataAsync(int limit = 20, double hours = 24, double time_offset = 0);
        Task<List<BatteryReading>> GetLatestPerBatteryAsync(int battery_id = 0);
    }
}