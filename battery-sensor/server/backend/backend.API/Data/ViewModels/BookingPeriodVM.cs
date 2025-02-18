using System;

namespace backend.API.Data.ViewModels
{
    public class BookingPeriodVM
    {
        public string Name { get; set; }
        public DateTime StartDate { get; set; }
        public DateTime EndDate { get; set; }
        public int BookingOpens { get; set; }
        // The earliest hour available for booking
        public int BookingCloses { get; set; }
        // The hour the latest booking must be finished
        public long BoatID { get; set; }
    }

    public class BookingPeriodReadableVM
    {
        public string Name { get; set; }
        public DateTime StartDate { get; set; }
        public DateTime EndDate { get; set; }
        public int BookingOpens { get; set; }
        // The earliest hour available for booking
        public int BookingCloses { get; set; }
        // The hour the latest booking must be finished
        public string BoatName { get; set; }
        public long BoatID { get; set; }
    }
}