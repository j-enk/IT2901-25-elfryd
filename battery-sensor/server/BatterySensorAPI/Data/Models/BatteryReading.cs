// Models/BatteryReading.cs
using System;

namespace BatterySensorAPI.Models
{
    public class BatteryReading
    {
        public long id { get; set; }
        
        public double voltage { get; set; }
        
        public DateTime timestamp { get; set; }
        
        public string sensorId { get; set; }
        
        public double? temperature { get; set; }
        
        public double? stateOfCharge { get; set; }
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