using System.Collections.Generic;

namespace backend.API.Data.ViewModels.Authentication
{
    public class UserReadableVM
    {
        public string ID { get; set; }
        public string Name { get; set; }
        public string Phone { get; set; }
        public string Email { get; set; }
        public List<string> Roles { get; set; }
        public int NumberOfBookings { get; set; }
        public bool isActive {get; set;}
    }
}