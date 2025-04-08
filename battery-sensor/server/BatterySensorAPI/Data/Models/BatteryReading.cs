// Models/BatteryReading.cs
using System;

namespace BatterySensorAPI.Models
{
    public class BatteryReading
    {
        public long Id { get; set; }
        
        public double Voltage { get; set; }
        
        public DateTime Timestamp { get; set; }
        
        public string SensorId { get; set; }
        
        public double? Temperature { get; set; }
        
        public double? StateOfCharge { get; set; }
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