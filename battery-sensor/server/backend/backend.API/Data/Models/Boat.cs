using Microsoft.EntityFrameworkCore;
using System;
using System.Collections.Generic;

namespace backend.API.Data.Models
{
    [Index(nameof(BoatID), IsUnique = true)]
    public class Boat
    {
        public long BoatID { get; set; }
        public string Name { get; set; }
        public int ChargingTime { get; set; } // Percentage of the booking length needed to charge
        public float WeatherLimit { get; set; } // m/s of maximum allowed wind.
        public int AdvanceBookingLimit { get; set; } // How many days into the future you can book
        public int MaxBookingLimit { get; set; } // Maximum length in hours for a booking
        public long LockBoxID { get; set; }
        public LockBox LockBox { get; set; }
        public List<Booking> Bookings { get; set; }
        public List<BookingPeriod> BookingPeriods { get; set; }
    }
}