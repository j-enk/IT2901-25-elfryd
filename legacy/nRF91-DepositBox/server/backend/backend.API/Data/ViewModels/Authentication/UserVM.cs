using System.Collections.Generic;

namespace backend.API.Data.ViewModels
{
    public class UserVM
    {
        public string Name { get; set; }
        public string Phone { get; set; }
        public string Email { get; set; }
        public string Role { get; set; }
        public List<long> BookingIDs { get; set; }
        public bool isActive{get; set;} 

    }

}