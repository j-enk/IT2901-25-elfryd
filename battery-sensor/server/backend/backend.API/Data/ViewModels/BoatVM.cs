using System.Collections.Generic;

namespace backend.API.Data.ViewModels
{
    public class BoatVM
    {
        public long BoatID { get; set; }
        public string Name { get; set; }
        public int ChargingTime { get; set; } // Percentage of the booking length needed to charge
        public float WeatherLimit { get; set; } // Wind speed limit in m/s which if crossed should disable booking; not implemented
        public int AdvanceBookingLimit { get; set; } // How many days into the future you can book
        public int MaxBookingLimit { get; set; } // Maximum length in hours for a booking

        public long LockBoxID { get; set; }
        public List<long> BookingsIds { get; set; }
        public List<long> BookingPeriodsIds { get; set; }

    }

    public class BoatReadableVM 
    {
        public long BoatID { get; set; }
        public string Name { get; set; }
        public int ChargingTime { get; set; } // Percentage of the booking length needed to charge
        public float WeatherLimit { get; set; } // Wind speed limit in m/s which if crossed should disable booking; not implemented
        public float AdvanceBookingLimit { get; set; } // How many days into the future you can book
        public int MaxBookingLimit { get; set; } // Maximum length in hours for a booking

        public long LockBoxID { get; set; }
        public List<long> BookingsIds { get; set; }
        public List<string> BookingPeriodsNames { get; set; }
        
    }

    public class BoatListVM
    {
        public long BoatID { get; set; }
        public string Name { get; set; }
        public int ChargingTime { get; set; } // Percentage of the booking length needed to charge
        public float WeatherLimit { get; set; }
        public float AdvanceBookingLimit { get; set; } // How many days into the future you can book
        public int MaxBookingLimit { get; set; } 
        public long LockBoxID { get; set; }
    }


}