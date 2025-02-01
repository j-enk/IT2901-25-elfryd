using Microsoft.EntityFrameworkCore;
using System.ComponentModel.DataAnnotations;
using System;
namespace backend.API.Data.Models 
{
    [Index(nameof(Name), nameof(BoatID), IsUnique = true)]
    public class BookingPeriod
    // Opening and closing times for a part of the year
    {
        [Key]
        public string Name { get; set; }
        public DateTime StartDate { get; set; }
        public DateTime EndDate { get; set; }
        public int BookingOpens { get; set; }
        // The earliest hour available for booking
        public int BookingCloses { get; set; }
        // The hour the latest booking must be finished
        public long BoatID { get; set; }
        public Boat Boat { get; set; }
    }
}