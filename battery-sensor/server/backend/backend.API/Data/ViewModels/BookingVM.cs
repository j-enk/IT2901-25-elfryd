using System;
using backend.API.Data.Models;
namespace backend.API.Data.ViewModels
{
    public class BookingVM
    {
        public long BookingID { get; set; }
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
        public char Status { get; set; }
        public string Comment { get; set; }
        public string UserID { get; set; }
        public long BoatID { get; set; }
        public bool isOfficial { get; set; }

    }

    public class BookingReadableVM
    {
        public long BookingID { get; set; }
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
        public DateTime ChargingDone { get; set; } // Time when charging after use is finished
        public DateTime? KeyTakenTime { get; set; }
        public DateTime? KeyReturnedTime { get; set; }
        public char Status { get; set; }
        public string Comment { get; set; }
        public string BookingOwnerId { get; set; }
        public string BookingOwner { get; set; }
        public string BookingOwnerPhone { get; set; }
        public string BookingOwnerEmail { get; set; }
        public string BookedBoat { get; set; }
        public bool isOfficial { get; set; }
    }

    public class BookingCalenderViewVM
    {
        public long BookingID { get; set; }
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
        public DateTime ChargingDone { get; set; }
        public char Status { get; set; }
        public string Comment { get; set; }
        public string BookingOwner { get; set; }
        public string BookingOwnerNumber { get; set; }
        public bool isOfficial { get; set; }
        public string BoatName { get; set; }
    }

    public class HasBookedVM
    {
        public long BookingID { get; set; }
        public string BookingOwner { get; set; }
    }

}