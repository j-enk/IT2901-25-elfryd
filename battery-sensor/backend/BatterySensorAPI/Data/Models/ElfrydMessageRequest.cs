using System;
using System.Collections.Generic;

namespace BatterySensorAPI.Models
{
    public class ElfrydMessageRequest
    {
        public string Topic { get; set; }
        public string Message { get; set; }
    }
    
    // You can add more Elfryd-related models here as needed
    public class ElfrydMessage
    {
        public string Id { get; set; }
        public string Topic { get; set; }
        public string Message { get; set; }
        public DateTime Timestamp { get; set; }
    }
    
    public class ElfrydResponse
    {
        public bool Success { get; set; }
        public string Message { get; set; }
        public object Data { get; set; }
    }
}