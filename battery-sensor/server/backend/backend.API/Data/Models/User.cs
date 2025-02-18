using Microsoft.AspNetCore.Identity;
using System.Collections.Generic;
namespace backend.API.Data.Models
{
    public class User : IdentityUser
    {

        public string Name { get; set; }
        public bool isActive {get; set;}
        public List<Booking> Bookings { get; set; }
    }
}