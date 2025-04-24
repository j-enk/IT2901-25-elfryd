// Models/BatteryReading.cs
using System;

namespace BatterySensorAPI.Models
{
    public class BatteryReading
    {
        public int id { get; set; }
        public int battery_id { get; set; }
        
        public double millivoltage { get; set; }
        
        public DateTime timestamp { get; set; }
    }
        

    public class BatteryStats
    {
        public double MinVoltage { get; set; }
        public double MaxVoltage { get; set; }
        public double AverageVoltage { get; set; }
        public int ReadingCount { get; set; }
        public int TimeWindowHours { get; set; }
        public BatteryReading LatestReading { get; set; }
    }
}