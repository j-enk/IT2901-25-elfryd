using Microsoft.EntityFrameworkCore;
using System;
namespace backend.API.Data.Models
{

    [Index(nameof(BookingID), IsUnique = true)]
    public class Booking
    {
        public long BookingID { get; set; }
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
        public DateTime ChargingDone { get; set; } // Time when charging after use is finished
        public DateTime? KeyTakenTime { get; set; }
        public DateTime? KeyReturnedTime { get; set; }
        public char Status { get; set; }
        public string Comment { get; set; }
        public string UserID { get; set; }
        public User User { get; set; }
        public long BoatID { get; set; }
        public Boat Boat { get; set; }
        public bool isOfficial { get; set; }
    }
}